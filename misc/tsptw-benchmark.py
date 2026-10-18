#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Serial, named TSPTW comparisons. Paths resolve from cwd; one writer per run."""
import argparse
from fractions import Fraction
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import platform
import re
import signal
import statistics
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / 'examples/data/tsptw'
ADAPTER = Path(__file__).with_name('tsptw-didppy.py').resolve()
SPEC = importlib.util.spec_from_file_location('tsptw_checker', Path(__file__).with_name('tsptw-check.py'))
checker = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(checker)
CONFIGS = {
    'closed-bab': ['--formulation', 'closed', '--engine', 'bab'],
    'closed-cabs': ['--formulation', 'closed', '--engine', 'cabs'],
    'transition-bab': ['--formulation', 'transition', '--engine', 'bab'],
    'transition-astar': ['--formulation', 'transition', '--engine', 'astar'],
    'transition-cabs': ['--formulation', 'transition', '--engine', 'cabs'],
    'didppy-astar': ['--engine', 'astar'],
    'didppy-cabs': ['--engine', 'cabs'],
}
DEFAULT_CONFIGS = tuple(CONFIGS)
CONFIGS.update({
    'closed-val-bab': ['--formulation', 'closed', '--engine', 'bab', '--circuit-propagation', 'val'],
    'closed-size-bab': ['--formulation', 'closed', '--engine', 'bab', '--cost-branching', 'size'],
    'closed-rbs-regret': ['--formulation', 'closed', '--engine', 'rbs'],
    'closed-rbs-size': ['--formulation', 'closed', '--engine', 'rbs', '--cost-branching', 'size'],
    'closed-dom-size-bab': ['--formulation', 'closed', '--engine', 'bab', '--circuit-propagation', 'dom', '--cost-branching', 'size'],
    'closed-dom-size-cabs': ['--formulation', 'closed', '--engine', 'cabs', '--circuit-propagation', 'dom', '--cost-branching', 'size'],
})
for name, formulation, bound, residual in (
        ('lazy-prefix', 'transition', 'prefix', 'off'),
        ('eager-prefix', 'eager', 'prefix', 'off'),
        ('lazy-min-arcs', 'transition', 'min-arcs', 'off'),
        ('lazy-unary', 'transition', 'prefix', 'unary')):
    CONFIGS['transition-astar-'+name] = ['--formulation', formulation, '--engine', 'astar',
        '--bound', bound, '--residual-propagation', residual]
for engine in ('bab', 'cabs'):
    CONFIGS['transition-'+engine+'-lazy-unary'] = ['--formulation', 'transition', '--engine', engine,
        '--bound', 'prefix', '--residual-propagation', 'unary']
for engine in ('bab', 'cabs'):
    CONFIGS['transition-'+engine+'-lazy-cheap'] = ['--formulation', 'transition', '--engine', engine,
        '--bound', 'prefix', '--residual-propagation', 'cheap']
TERMINAL = {'ok', 'timeout', 'rss-limit', 'execution-error', 'invalid-output', 'measurement-error'}
QUALITY_STATUSES = {'ok', 'timeout', 'rss-limit'}
CHECKPOINTS_MS = (100, 1000, 5000, 15000, 30000)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_json(path, value):
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + '\n')
    temporary.replace(path)


def budget_header(path):
    header = json.loads(Path(path).read_text().splitlines()[0])
    if header.get('scope') == 'task':
        if (header.get('event') != 'budget' or not header.get('task_id') or
                any(not nonnegative_number(header.get(field)) for field in
                    ('total_seconds', 'verification_reserve_seconds', 'cleanup_reserve_seconds')) or
                header['total_seconds'] <= header['verification_reserve_seconds']+header['cleanup_reserve_seconds']):
            raise ValueError('invalid task budget header')
    elif header.get('event') != 'budget' or header.get('total_seconds') != 7200:
        raise ValueError('expected the saved 7200-second evaluation ledger')
    return header


def budget_state(path):
    budget_header(path)
    rows = [json.loads(line) for line in Path(path).read_text().splitlines()]
    used, pending, charged = {}, {}, {}
    for row in rows[1:]:
        if row['event'] == 'launch':
            pending[row['id']] = row
        elif row['event'] == 'process':
            pending[row['id']].update(row['process'])
        elif row['event'] in ('unresolved', 'reap'):
            launch = pending[row['id']]
            elapsed = row['wall_seconds']
            used[launch['phase']] = used.get(launch['phase'], 0) + elapsed-charged.get(row['id'], 0)
            charged[row['id']] = elapsed
            if row['event'] == 'reap':
                pending.pop(row['id'])
    return used, pending


def budget_event(path, row):
    with Path(path).open('a') as output:
        output.write(json.dumps(row, allow_nan=False)+'\n')
        output.flush()
        os.fsync(output.fileno())


def admit_budget(path, phase, seconds, remaining_nominal_seconds):
    header = budget_header(path)
    used, pending = budget_state(path)
    if pending:
        raise ValueError('unreaped budget entry requires inspection: '+json.dumps(pending, sort_keys=True))
    if header.get('scope') == 'task':
        if phase not in ('correctness', 'development', 'verification'):
            raise ValueError('task solver budget excludes compilation and held-out work')
        allowance = header['verification_reserve_seconds']
        if phase == 'verification' and used.get(phase, 0)+seconds > allowance:
            raise ValueError('task verification allowance would be exceeded')
        reserve = max(0, allowance-used.get('verification', 0)-(seconds if phase == 'verification' else 0))
        if sum(used.values())+seconds+remaining_nominal_seconds+reserve+header['cleanup_reserve_seconds'] > header['total_seconds']:
            raise ValueError('remaining nominal work and reserves exceed the task allowance')
        identity = str(time.time_ns())
        budget_event(path, dict(event='launch', id=identity, phase=phase,
            cap_seconds=seconds, remaining_nominal_seconds=remaining_nominal_seconds))
        return identity
    pre_freeze = sum(value for name, value in used.items() if name in ('compilation', 'correctness', 'development'))
    if phase in ('compilation', 'correctness', 'development') and pre_freeze+seconds > 400:
        raise ValueError('pre-freeze measured work would exceed 400 seconds')
    if phase == 'verification' and used.get(phase, 0)+seconds > 300:
        raise ValueError('verification allowance would exceed 300 seconds')
    verification_reserve = max(0, 300-used.get('verification', 0)-(seconds if phase == 'verification' else 0))
    if sum(used.values())+seconds+remaining_nominal_seconds+verification_reserve+600 > 7200:
        raise ValueError('remaining nominal work and reserves exceed the shared allowance')
    identity = str(time.time_ns())
    budget_event(path, dict(event='launch', id=identity, phase=phase,
        cap_seconds=seconds, remaining_nominal_seconds=remaining_nominal_seconds))
    return identity


def select_cases(split, selected_root, case_ids=None):
    if case_ids and split != 'development':
        raise ValueError('case-ids is a development-only selector')
    if split == 'tiny':
        return [dict(id=p.name, path=str(p), sha256=digest(p)) for p in sorted((DATA/'tiny').glob('*.txt'))]
    manifest = json.loads((DATA/'manifest.json').read_text())
    cases = [dict(id=c['id'], path=str((selected_root/c['path']).resolve()), sha256=c['sha256'],
                 family=c['family'], variant=c.get('variant'))
            for c in manifest['cases'] if c['split'] == split]
    if case_ids:
        names = case_ids.split(',')
        controls = {f'tiny/{p.name}': p for p in (DATA/'tiny').glob('*.txt')}
        cases.extend(dict(id=name, path=str(controls[name]), sha256=digest(controls[name]),
            family='tiny', variant='tiny control') for name in names if name in controls)
        if len(names) != len(set(names)) or not set(names) <= {c['id'] for c in cases}:
            raise ValueError('case-ids must name distinct development cases or existing tiny controls')
        cases = [c for c in cases if c['id'] in names]
    return cases


def experiment(args):
    names = args.configs.split(',') if args.configs else list(DEFAULT_CONFIGS[:5] if args.split == 'development' else DEFAULT_CONFIGS)
    if len(set(names)) != len(names) or any(n not in CONFIGS for n in names):
        raise ValueError('configs must be distinct names from: ' + ', '.join(CONFIGS))
    repetitions = args.repetitions if args.repetitions is not None else (2 if args.split == 'development' else 1)
    seconds = args.seconds if args.seconds is not None else (30 if args.split == 'held-out' else 15 if args.split == 'development' else 5)
    if repetitions < 1 or not math.isfinite(seconds) or seconds <= 0:
        raise ValueError('repetitions and seconds must be positive')
    if args.split == 'development' and (len(names) > 6 or repetitions > 2 or seconds > 15):
        raise ValueError('development is bounded to six configurations, two repeats, 15 seconds')
    if args.split == 'held-out' and (repetitions != 1 or seconds > 30):
        raise ValueError('held-out is bounded to one repeat and 30 seconds')
    if args.compilation_seconds is not None and (not math.isfinite(args.compilation_seconds) or args.compilation_seconds < 0):
        raise ValueError('compilation seconds must be finite and nonnegative')
    if args.command == 'run' and args.max_samples is not None and args.max_samples < 0:
        raise ValueError('max-samples must be nonnegative')
    solver = args.solver.resolve()
    config = dict(schema_version=1, split=args.split, configs={n: CONFIGS[n] for n in names},
        cases=select_cases(args.split, args.selected_root.resolve(), args.case_ids), repetitions=repetitions,
        seconds=seconds, rss_bytes=1024**3, sample_interval_seconds=0.05,
        threads=1, native=dict(path=str(solver), sha256=digest(solver),
            libraries={str(p.resolve()): digest(p) for p in sorted(solver.parent.parent.glob('libgecode*.6.4.0.dylib'))}),
        didppy=dict(version='0.11.1', adapter_sha256=digest(ADAPTER), launcher=args.uv),
        runner_sha256=digest(__file__), checker_sha256=digest(SPEC.origin),
        machine=dict(system=platform.platform(), machine=platform.machine(), processor=platform.processor()),
        sources={str(p.relative_to(ROOT)): digest(p) for p in
            [ROOT/'examples/tsptw.cpp', ROOT/'examples/tsptw-transition.hh', ROOT/'examples/tsptw-data.hh',
             ROOT/'gecode/search.hh', *sorted((ROOT/'gecode/search').rglob('*.cpp')),
             *sorted((ROOT/'gecode/search').rglob('*.hpp'))]},
        manifest_sha256=digest(DATA/'manifest.json'),
        compilation=dict(status='prebuilt; measured builds belong to the shared ledger when supplied', seconds=args.compilation_seconds),
        budget=dict(ledger=str(args.budget_ledger.resolve()), phase=args.budget_phase or
            ('held-out' if args.split == 'held-out' else 'development' if args.split == 'development' else 'correctness'))
            if args.budget_ledger else None,
        timing='launch through reap; checks after timing; watchdog includes startup and preprocessing')
    if config['budget'] and budget_header(config['budget']['ledger']).get('scope') == 'task':
        if config['budget']['phase'] not in ('correctness', 'development', 'verification') or args.split == 'held-out':
            raise ValueError('task solver budget excludes held-out work')
        config['compilation']['status'] = 'prebuilt; measured builds reported separately from task solver budget'
    rows = []
    for repetition in range(repetitions):
        order = names if repetition % 2 == 0 else names[::-1]
        for case_index, case in enumerate(config['cases']):
            for name in order:
                row = dict(id=f'{repetition:02d}-{case_index:03d}-{name}', case=case,
                    config=name, repetition=repetition, arguments=CONFIGS[name])
                row['argv'] = command(config, row)
                rows.append(row)
    config['rows'] = rows
    return config


def preview(config):
    seconds = len(config['rows'])*config['seconds']
    value = dict(samples=len(config['rows']), solver_budget_seconds=seconds,
        compilation_seconds=config['compilation']['seconds'],
        development_ceiling_seconds=2700, held_out_ceiling_seconds=7200,
        full_campaign=dict(development_samples=5*12*2, development_solver_seconds=1800,
            held_out_samples=7*24, held_out_solver_seconds=5040,
            held_out_remaining_for_ablations_seconds=2160,
            full_development_plus_held_out_seconds=6840,
            budget_scope='development tuning has its own 45-minute ceiling; evaluation has a separate 120-minute ceiling including its development ablations and compilation; held-out leaves 36 minutes inside evaluation'),
        configurations=list(config['configs']), cases=[c['id'] for c in config['cases']],
        timing=config['timing'])
    if config.get('budget'):
        value['budget'] = budget_header(config['budget']['ledger'])
        if value['budget'].get('scope') == 'task':
            for key in ('development_ceiling_seconds', 'held_out_ceiling_seconds', 'full_campaign'):
                value.pop(key)
    print(json.dumps(value, indent=2))


def nonnegative_number(value, *, integer=False):
    return (not isinstance(value, bool) and isinstance(value, int if integer else (int, float))
            and math.isfinite(value) and value >= 0)


def validate_incumbent(value):
    """Check a saved incumbent's own fields without needing the original input."""
    if not isinstance(value, dict):
        raise ValueError('invalid incumbent record')
    tour = value['tour']
    if (not isinstance(tour, list) or not tour or
            any(not nonnegative_number(v, integer=True) or v == 0 for v in tour) or
            set(tour) != set(range(1, len(tour)+1))):
        raise ValueError('invalid saved tour')
    if not nonnegative_number(value['scaled_cost'], integer=True):
        raise ValueError('invalid saved scaled cost')
    scale = value['scale']
    if not nonnegative_number(scale, integer=True) or scale == 0:
        raise ValueError('invalid saved scale')
    for key in ('cost', 'return_time'):
        if not isinstance(value[key], str) or not re.fullmatch(r'[0-9]+(?:\.[0-9]+)?', value[key]):
            raise ValueError('invalid saved ' + key)
    if Fraction(value['cost'])*scale != value['scaled_cost']:
        raise ValueError('saved cost and scaling differ')


def validate_saved_trace(trace):
    if not isinstance(trace, list):
        raise ValueError('invalid incumbent trace')
    previous_cost, previous_time = None, -1
    for incumbent in trace:
        validate_incumbent(incumbent)
        if incumbent['type'] != 'solution' or incumbent['checked'] is not True:
            raise ValueError('incumbent lacks independent check')
        elapsed = incumbent['elapsed_ms']
        if not nonnegative_number(elapsed) or elapsed < previous_time:
            raise ValueError('invalid saved incumbent time')
        if previous_cost is not None and incumbent['scaled_cost'] >= previous_cost:
            raise ValueError('nonimproving saved incumbent')
        previous_cost, previous_time = incumbent['scaled_cost'], elapsed


def validate_allocation(allocation):
    if not isinstance(allocation, dict):
        raise ValueError('invalid allocation measurements')
    for key in ('space_bytes', 'intvar_handle_bytes', 'root_positions',
                'prepared_root_positions', 'solution_positions'):
        if not nonnegative_number(allocation[key], integer=True):
            raise ValueError('invalid allocation measurement ' + key)


def validate_summary(summary, trace, row):
    if not isinstance(summary, dict) or summary['type'] != 'summary':
        raise ValueError('invalid summary')
    options = row['arguments'] if 'arguments' in row else CONFIGS[row['config']]
    engine = options[options.index('--engine')+1]
    formulation = 'didppy' if row['config'].startswith('didppy-') else options[options.index('--formulation')+1]
    if summary['engine'] != engine or summary['formulation'] != formulation:
        raise ValueError('summary engine/formulation differs')
    if formulation != 'didppy':
        for option, field, default in (('--bound', 'bound', 'cp-total' if formulation == 'closed' else 'prefix'),
                ('--residual-propagation', 'residual_propagation', 'off')):
            requested = options[options.index(option)+1] if option in options else default
            if summary.get(field, default if row['config'] in DEFAULT_CONFIGS else None) != requested:
                raise ValueError('summary differs from requested ' + field)
    if formulation == 'closed':
        for option, field, default in (('--circuit-propagation', 'circuit_propagation', 'dom'),
                                       ('--cost-branching', 'cost_branching', 'regret')):
            requested = options[options.index(option)+1] if option in options else default
            # Older default records predate these fields; explicit candidates require them.
            if summary.get(field, default if row['config'] in DEFAULT_CONFIGS else None) != requested:
                raise ValueError('summary differs from requested ' + field)
        if engine == 'rbs' and any(summary.get(k) != v for k, v in
                (('restart_cutoff', 'luby'), ('restart_scale', 1000), ('nogoods_limit', 128))):
            raise ValueError('summary differs from fixed restart settings')
    if formulation == 'didppy' and summary['version'] != '0.11.1':
        raise ValueError('DIDPPy version differs')
    termination = summary['termination']
    if termination not in ('optimal', 'infeasible', 'stopped'):
        raise ValueError('invalid termination')
    incumbent = summary['incumbent']
    if bool(trace) != (incumbent is not None):
        raise ValueError('summary and trace differ')
    if incumbent is not None:
        validate_incumbent(incumbent)
        if any(incumbent[k] != trace[-1][k] for k in ('tour', 'cost', 'scaled_cost', 'scale', 'return_time')):
            raise ValueError('summary differs from final incumbent')
    if termination == 'optimal' and not trace or termination == 'infeasible' and trace:
        raise ValueError('proof contradicts incumbent')
    for key in ('elapsed_ms', 'preprocessing_ms', 'search_ms'):
        if not nonnegative_number(summary[key]):
            raise ValueError('invalid summary timing ' + key)
    if trace and summary['elapsed_ms'] < trace[-1]['elapsed_ms']:
        raise ValueError('summary precedes final incumbent')
    statistics = summary['statistics']
    if not isinstance(statistics, dict) or 'node' not in statistics:
        raise ValueError('missing expansion statistics')
    for key, value in statistics.items():
        if not nonnegative_number(value, integer=True):
            raise ValueError('invalid summary statistic ' + key)
    if 'allocation' in summary:
        validate_allocation(summary['allocation'])
    if 'measurements' in summary:
        measurements = summary['measurements']
        if not isinstance(measurements, dict):
            raise ValueError('invalid summary measurements')
        for key in ('bound_calls', 'bound_ms', 'inference_calls', 'inference_ms', 'propagation_ms'):
            if not nonnegative_number(measurements[key], integer=key.endswith('_calls')):
                raise ValueError('invalid summary measurement ' + key)


def completed(path, row):
    if not path.exists():
        return None
    try:
        value = json.loads(path.read_text())
        if value['row'] != row or value['status'] not in TERMINAL:
            raise ValueError('record identity/status differs')
        if 'argv' in row and value['argv'] != row['argv']:
            raise ValueError('saved command differs from the frozen row')
        if not nonnegative_number(value['wall_seconds']):
            raise ValueError('invalid wall time')
        if not isinstance(value['diagnostic'], str):
            raise ValueError('invalid diagnostic')
        trace = value['incumbents']
        validate_saved_trace(trace)
        objective = value['final_objective']
        if objective is not None and not nonnegative_number(objective, integer=True):
            raise ValueError('invalid final objective')
        if objective != (trace[-1]['scaled_cost'] if trace else None):
            raise ValueError('final objective differs from saved trace')
        metrics = value['metrics']
        if not isinstance(metrics, dict):
            raise ValueError('invalid metrics')
        for key in ('preprocessing_ms', 'search_ms', 'startup_and_teardown_ms', 'expansions',
                    'frontier_bytes', 'clone_bytes'):
            metric = metrics[key]
            integer = key not in ('preprocessing_ms', 'search_ms', 'startup_and_teardown_ms')
            if metric is not None and not nonnegative_number(metric, integer=integer):
                raise ValueError('invalid saved metric ' + key)
        allocation = metrics['allocation']
        if allocation is not None:
            validate_allocation(allocation)
        proof = value['proof']
        if proof not in (None, 'optimal', 'infeasible'):
            raise ValueError('invalid proof')
        summary = value['summary']
        if summary is not None:
            validate_summary(summary, trace, row)
            for metric, field in (('preprocessing_ms', 'preprocessing_ms'), ('search_ms', 'search_ms')):
                if metrics[metric] != summary[field]:
                    raise ValueError('metric differs from saved summary ' + metric)
            if metrics['expansions'] != summary['statistics'].get('node'):
                raise ValueError('metric differs from summary statistic node')
            if metrics['allocation'] != summary.get('allocation'):
                raise ValueError('allocation differs from summary')
        if value['status'] == 'ok':
            if summary is None or value['returncode'] != 0:
                raise ValueError('successful row lacks successful summary/exit')
            expected_proof = summary['termination'] if summary['termination'] in ('optimal', 'infeasible') else None
            if proof != expected_proof or value['stop_reason'] != summary['termination']:
                raise ValueError('proof/stop reason differs from saved summary')
        elif proof is not None or value['stop_reason'] != value['status']:
            raise ValueError('failed row claims proof or wrong stop reason')
        if value['returncode'] is not None and (isinstance(value['returncode'], bool) or not isinstance(value['returncode'], int)):
            raise ValueError('invalid exit code')
        rss = value['peak_rss_bytes']
        if rss is not None and (not nonnegative_number(rss, integer=True) or rss == 0):
            raise ValueError('invalid RSS')
        return value
    except (ValueError, KeyError, TypeError, OverflowError) as error:
        print(f'unfinished record {path}: {error}', file=sys.stderr)
        return None


def command(config, row):
    options = config['configs'][row['config']]
    if row['config'].startswith('didppy-'):
        argv = [config['didppy']['launcher'], 'run', '--script', str(ADAPTER)]
    else:
        argv = [config['native']['path']]
    return argv + [row['case']['path']] + options + ['--time-ms', str(round(config['seconds']*1000))]


def kill_group(process):
    try:
        os.killpg(process.pid, signal.SIGKILL)
    except ProcessLookupError:
        pass
    process.wait()


def require_reaped(samples):
    pending = sorted(samples.glob('*.launch.json'))
    if pending:
        raise ValueError('unconfirmed child exit; use recover after inspecting '+
            '; '.join(str(path)+': '+path.read_text().strip() for path in pending))


def recover(samples):
    """Confirm the child is gone and count time until inspection; its exit time is unknown."""
    for path in sorted(samples.glob('*.launch.json')):
        launch = json.loads(path.read_text())
        pid = launch.get('pid')
        if not isinstance(pid, int) or pid <= 0:
            raise ValueError('launch has no confirmed child identity: '+str(path))
        try:
            os.kill(pid, 0)
        except ProcessLookupError:
            pass
        else:
            raise ValueError('child PID still exists; inspect before recovery: '+json.dumps(launch))
        budget = launch.get('budget')
        if budget:
            _, pending = budget_state(budget['ledger'])
            if launch['id'] in pending:
                budget_event(budget['ledger'], dict(event='reap', id=launch['id'],
                    wall_seconds=max(launch['wall_seconds'], time.time()-launch['started_at']),
                    observed_wall_seconds=launch['wall_seconds'], returncode=None,
                    status='recovered-absent', timing='launch-through-absence upper bound; exit time unobserved'))
        path.unlink()


def capture(argv, destination, seconds, rss_limit, interval, *, budget=None,
            remaining_nominal_seconds=5040):
    require_reaped(destination.parent)
    identity = None
    if budget:
        identity = admit_budget(budget['ledger'], budget['phase'], seconds, remaining_nominal_seconds)
        budget_event(budget['ledger'], dict(event='command', id=identity, argv=argv))
    started = time.perf_counter()
    marker = destination.with_suffix('.launch.json')
    launch = dict(id=identity, argv=argv, pid=None, pgid=None, budget=budget,
        started_at=time.time(), wall_seconds=0)
    write_json(marker, launch)
    peak, process, reason, exited = None, None, None, False
    observations, max_gap, probe_seconds, termination_seconds = 0, 0.0, 0.0, 0.0
    previous = started
    try:
        with destination.with_suffix('.stdout').open('wb') as stdout, destination.with_suffix('.stderr').open('wb') as stderr:
            try:
                environment = dict(os.environ, OMP_NUM_THREADS='1', OPENBLAS_NUM_THREADS='1', MKL_NUM_THREADS='1')
                process = subprocess.Popen(argv, stdout=stdout, stderr=stderr, cwd=ROOT,
                    start_new_session=True, env=environment)
            except OSError as error:
                reason = 'execution-error'
                return dict(returncode=None, status=reason, diagnostic=str(error),
                    wall_seconds=time.perf_counter()-started, peak_rss_bytes=None,
                    rss_samples=0, maximum_sample_gap_seconds=None, rss_probe_seconds=0,
                    termination_reap_seconds=0, wall_cap_overshoot_seconds=0, rss_overshoot_bytes=None)
            try:
                launch.update(pid=process.pid, pgid=process.pid)
                write_json(marker, launch)
                if budget:
                    budget_event(budget['ledger'], dict(event='process', id=identity,
                        process=dict(pid=process.pid, pgid=process.pid, argv=argv)))
                while process.poll() is None:
                    now = time.perf_counter()
                    if now-started >= seconds:
                        reason = 'timeout'
                    else:
                        # Include uv's interpreter child; launcher RSS alone is misleading.
                        probe_start = time.perf_counter()
                        probe = subprocess.run(['ps', '-axo', 'pid=,pgid=,rss='], capture_output=True, text=True)
                        probe_seconds += time.perf_counter()-probe_start
                        if probe.returncode:
                            reason = 'measurement-error'
                        else:
                            rss = sum(int(fields[2])*1024 for line in probe.stdout.splitlines()
                                if len(fields := line.split()) == 3 and int(fields[1]) == process.pid)
                            if rss:
                                peak = max(peak or 0, rss)
                            observations += 1
                            max_gap = max(max_gap, now-previous)
                            previous = now
                            if rss > rss_limit:
                                reason = 'rss-limit'
                    if reason:
                        termination_start = time.perf_counter()
                        kill_group(process)
                        termination_seconds += time.perf_counter()-termination_start
                        exited = True
                        break
                    time.sleep(min(interval, max(0, seconds-(time.perf_counter()-started))))
                if not exited:
                    termination_start = time.perf_counter()
                    process.wait()
                    termination_seconds += time.perf_counter()-termination_start
                    exited = True
            except BaseException as error:
                try:
                    kill_group(process)
                    exited = True
                except BaseException as cleanup_error:
                    error.add_note('child cleanup failed: '+str(cleanup_error)+
                        '; owned process: '+json.dumps(launch))
                raise
        elapsed = time.perf_counter()-started
        return dict(returncode=process.returncode, status=reason or ('ok' if process.returncode == 0 else 'execution-error'),
            diagnostic=reason or '', wall_seconds=elapsed, peak_rss_bytes=peak,
            rss_samples=observations, maximum_sample_gap_seconds=max_gap if observations else None,
            rss_probe_seconds=probe_seconds, termination_reap_seconds=termination_seconds,
            wall_cap_overshoot_seconds=max(0, elapsed-seconds),
            rss_overshoot_bytes=max(0, peak-rss_limit) if peak is not None else None)
    finally:
        launch['wall_seconds'] = time.perf_counter()-started
        # No child was created, or wait confirmed exit. A failed kill/wait never closes work.
        closed = process is None or exited
        write_json(marker, launch)
        if budget:
            budget_event(budget['ledger'], dict(event='reap' if closed else 'unresolved', id=identity,
                wall_seconds=launch['wall_seconds'],
                returncode=process.returncode if exited else None,
                status='interrupted' if sys.exc_info()[0] else reason or
                    ('ok' if process and process.returncode == 0 else 'execution-error')))
        if closed:
            marker.unlink()


def checked_output(path, data):
    trace = []
    summary = None
    errors = []
    previous_cost = None
    previous_time = -1
    for line_number, line in enumerate(path.read_text().splitlines(), 1):
        try:
            event = json.loads(line)
            if summary is not None:
                errors.append('output after final summary')
            if event['type'] == 'solution':
                value = checker.check_tour(data, event['tour'], event['cost'])
                for key in ('scaled_cost', 'scale', 'return_time'):
                    if value[key] != event[key]:
                        raise ValueError('incorrect solution ' + key)
                elapsed = event['elapsed_ms']
                if isinstance(elapsed, bool) or not isinstance(elapsed, (int, float)) or not math.isfinite(elapsed) or elapsed < previous_time:
                    raise ValueError('invalid incumbent time')
                if previous_cost is not None and value['scaled_cost'] >= previous_cost:
                    raise ValueError('nonimproving incumbent')
                candidate = dict(event, checked=True)
                validate_saved_trace([candidate])
                previous_cost, previous_time = value['scaled_cost'], elapsed
                trace.append(candidate)
            elif event['type'] == 'summary':
                if summary is not None:
                    raise ValueError('duplicate summary')
                summary = event
                if event['termination'] not in ('optimal', 'infeasible', 'stopped'):
                    raise ValueError('unknown solver termination')
                incumbent = event['incumbent']
                if bool(incumbent) != bool(trace):
                    raise ValueError('summary does not match published incumbent')
                if incumbent:
                    checker.check_tour(data, incumbent['tour'], incumbent['cost'])
                    if any(incumbent[k] != trace[-1][k] for k in ('tour', 'cost', 'scaled_cost', 'scale', 'return_time')):
                        raise ValueError('summary differs from final incumbent')
                if event['termination'] == 'optimal' and not trace or event['termination'] == 'infeasible' and trace:
                    raise ValueError('proof contradicts incumbent')
                for key in ('elapsed_ms', 'preprocessing_ms', 'search_ms'):
                    value = event.get(key)
                    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
                        raise ValueError('invalid summary timing ' + key)
            else:
                raise ValueError('unknown output event')
        except (ValueError, KeyError, TypeError) as error:
            errors.append(f'line {line_number}: {error}')
    return trace, summary, errors


def collect(config, row, samples, remaining_nominal_seconds=5040):
    require_reaped(samples)
    case_path = Path(row['case']['path'])
    if digest(case_path) != row['case']['sha256']:
        raise ValueError('input digest mismatch: ' + str(case_path))
    data = checker.read_instance(case_path)
    destination = samples/row['id']
    # Partial streams survive retries. Completed streams are never touched.
    for suffix in ('.stdout', '.stderr', '.json'):
        path = destination.with_suffix(suffix)
        if path.exists():
            path.rename(path.with_name(path.name + f'.partial-{time.time_ns()}'))
    argv = command(config, row)
    budget_arguments = dict(budget=config['budget'], remaining_nominal_seconds=remaining_nominal_seconds) if config.get('budget') else {}
    result = capture(argv, destination, config['seconds'], config['rss_bytes'], config['sample_interval_seconds'], **budget_arguments)
    trace, summary = [], None
    result['process_termination'] = result['status']
    try:
        trace, summary, errors = checked_output(destination.with_suffix('.stdout'), data)
        if errors:
            raise ValueError('; '.join(errors))
        if result['status'] == 'ok' and summary is None:
            raise ValueError('missing final summary')
        if summary is not None:
            validate_summary(summary, trace, row)
    except (ValueError, KeyError, TypeError, OverflowError) as error:
        result['status'] = 'invalid-output'
        result['diagnostic'] = str(error)
    valid = result['status'] == 'ok'
    if result['status'] == 'invalid-output':
        summary = None
    result.update(row=row, argv=argv, incumbents=trace, summary=summary,
        proof=summary['termination'] if valid and summary['termination'] in ('optimal', 'infeasible') else None,
        final_objective=trace[-1]['scaled_cost'] if trace else None,
        stop_reason=result['status'] if not valid else summary['termination'],
        metrics=dict(preprocessing_ms=summary.get('preprocessing_ms') if summary else None,
            search_ms=summary.get('search_ms') if summary else None,
            startup_and_teardown_ms=max(0, result['wall_seconds']*1000-summary['elapsed_ms']) if summary else None,
            expansions=summary.get('statistics', {}).get('node') if summary else None,
            allocation=summary.get('allocation') if summary else None,
            frontier_bytes=None, clone_bytes=None),
        unavailable='frontier and clone footprint are not instrumented; allocation sizeof excludes buffers; phases unavailable after kill; RSS is sampled process-group sum with overshoot')
    write_json(destination.with_suffix('.json'), result)
    print(row['id'], result['status'], result['proof'], result['final_objective'], flush=True)


def paired_sample(record, checkpoints):
    if record is None:
        return dict(status='missing', quality_usable=False)
    usable = record['status'] in QUALITY_STATUSES
    trace = record['incumbents'] if usable else []
    quality = {str(ms): next((v['scaled_cost'] for v in reversed(trace)
                             if v['elapsed_ms'] <= ms), None) for ms in checkpoints}
    return dict(status=record['status'], quality_usable=usable,
        final_objective=trace[-1]['scaled_cost'] if trace else None,
        scale=trace[-1]['scale'] if trace else None,
        first_incumbent_reported_ms=trace[0]['elapsed_ms'] if trace else None,
        quality_at_reported_ms=quality, proof=record['proof'], termination=record['stop_reason'],
        proof_reported_ms=record['summary']['elapsed_ms'] if record['proof'] else None,
        end_to_end_wall_seconds=record['wall_seconds'], sampled_peak_rss_bytes=record['peak_rss_bytes'])


def compare_pair(baseline, candidate, field, checkpoint=None):
    if 'missing' in (baseline['status'], candidate['status']):
        return 'missing'
    if not baseline['quality_usable'] or not candidate['quality_usable']:
        return 'failed'
    left, right = baseline[field], candidate[field]
    if checkpoint is not None:
        left, right = left[checkpoint], right[checkpoint]
    if left is None and right is None:
        return 'unavailable'
    if field in ('final_objective', 'quality_at_reported_ms'):
        if left is None:
            return 'win'
        if right is None:
            return 'loss'
    elif left is None or right is None:
        return 'unavailable'
    return 'win' if right < left else 'loss' if right > left else 'tie'


def paired_analysis(config, records, baseline_name=None, candidates=None):
    baseline_name = baseline_name or next((name for name in ('closed-dom-size-bab', 'closed-size-bab', 'closed-bab')
                                          if name in config['configs']), None)
    if baseline_name is None:
        return None
    checkpoints = [ms for ms in CHECKPOINTS_MS if ms <= config['seconds']*1000]
    by_key = {(row['case']['id'], row['repetition'], row['config']): record
              for row, record in zip(config['rows'], records)}
    pairs, counts = [], {}
    fields = ('final_objective', 'first_incumbent_reported_ms', 'proof_reported_ms',
              'end_to_end_wall_seconds', 'sampled_peak_rss_bytes')
    for name in candidates if candidates is not None else config['configs']:
        if name == baseline_name:
            continue
        tallies = {field: dict.fromkeys(('win', 'tie', 'loss', 'missing', 'failed', 'unavailable'), 0)
                   for field in (*fields, *(f'quality_at_{ms}_reported_ms' for ms in checkpoints))}
        for case in config['cases']:
            for repetition in range(config['repetitions']):
                baseline = paired_sample(by_key.get((case['id'], repetition, baseline_name)), checkpoints)
                candidate = paired_sample(by_key.get((case['id'], repetition, name)), checkpoints)
                comparisons = {field: compare_pair(baseline, candidate, field) for field in fields}
                for ms in checkpoints:
                    comparisons[f'quality_at_{ms}_reported_ms'] = compare_pair(
                        baseline, candidate, 'quality_at_reported_ms', str(ms))
                for field, outcome in comparisons.items():
                    tallies[field][outcome] += 1
                pairs.append(dict(case=case, repetition=repetition, candidate=name,
                    baseline=baseline, measured=candidate, comparisons=comparisons))
        counts[name] = tallies
    return dict(baseline=baseline_name, reported_time_checkpoints_ms=checkpoints, counts=counts, pairs=pairs,
        note='Costs are paired only within the same case and scale. Checked timeout/RSS incumbents count as feasible quality, never proof. Native reported clocks start at main entry and include preprocessing; outer wall clocks measure launch through reap. Missing metrics are unavailable; lower values win. Cases mark canonical data and deterministic window variants separately.')


def comparison_report(config, records):
    checkpoints = [ms for ms in CHECKPOINTS_MS if ms <= config['seconds']*1000]
    cases = []
    for case in config['cases']:
        selected = [r for r in records if r and r['row']['case']['id'] == case['id']]
        if any(r['proof'] == 'infeasible' for r in selected) and any(r['incumbents'] for r in selected):
            raise ValueError('checked tour contradicts an infeasibility proof for '+case['id'])
        proven = {r['final_objective'] for r in selected if r['proof'] == 'optimal'}
        if len(proven) > 1:
            raise ValueError('conflicting optimality proofs for '+case['id'])
        reference = next(iter(proven), None)
        if reference is not None and any(incumbent['scaled_cost'] < reference
                for record in selected for incumbent in record['incumbents']):
            raise ValueError('checked candidate contradicts an optimality proof')
        measured = []
        for record in selected:
            sample = paired_sample(record, checkpoints)
            objective = sample['final_objective']
            sample.update(config=record['row']['config'], repetition=record['row']['repetition'],
                gap_to_proved_optimum=float(Fraction(objective-reference, reference))
                    if objective is not None and reference else 0 if objective == reference == 0 else None,
                metrics=record['metrics'], measurements=record['summary'].get('measurements') if record['summary'] else None,
                rss_samples=record.get('rss_samples'), maximum_sample_gap_seconds=record.get('maximum_sample_gap_seconds'),
                rss_probe_seconds=record.get('rss_probe_seconds'), termination_reap_seconds=record.get('termination_reap_seconds'),
                wall_cap_overshoot_seconds=record.get('wall_cap_overshoot_seconds'), rss_overshoot_bytes=record.get('rss_overshoot_bytes'))
            measured.append(sample)
        cases.append(dict(case=case, reference_scaled_cost=reference,
            reference_kind='matching campaign optimality proof' if reference is not None else 'unavailable; campaign best is not an optimum',
            measurements=measured))
    same_model = {}
    for formulation, preferred in (('closed', ('closed-dom-size-bab', 'closed-size-bab', 'closed-bab')),
            ('transition', tuple(n for n in config['configs'] if n.startswith('transition-bab'))),
            ('didppy', ('didppy-astar',))):
        names = [name for name, arguments in config['configs'].items()
                 if ('didppy' if name.startswith('didppy-') else arguments[arguments.index('--formulation')+1]) == formulation]
        baseline = next((name for name in preferred if name in names), None)
        if baseline:
            paired = paired_analysis(config, records, baseline, names)
            same_model[formulation] = {key: value for key, value in paired.items() if key != 'pairs'}
    formulation_contrasts = {}
    for engine in ('bab', 'cabs'):
        names = {arguments[arguments.index('--formulation')+1]: name
                 for name, arguments in config['configs'].items()
                 if not name.startswith('didppy-') and arguments[arguments.index('--engine')+1] == engine}
        if 'closed' in names and 'transition' in names:
            paired = paired_analysis(config, records, names['closed'], [names['transition']])
            formulation_contrasts[engine] = {key: value for key, value in paired.items() if key != 'pairs'}
    family_groups = {}
    for case in cases:
        key = case['case'].get('family', 'tiny')+'/'+('variant' if case['case'].get('variant') else 'canonical')
        group = family_groups.setdefault(key, {})
        for record in case['measurements']:
            counts = group.setdefault(record['config'], dict(samples=0, feasible=0, optimal=0, infeasible=0, resource_stops=0, failures=0))
            counts['samples'] += 1
            counts['feasible'] += record['final_objective'] is not None
            counts['optimal'] += record['proof'] == 'optimal'
            counts['infeasible'] += record['proof'] == 'infeasible'
            counts['resource_stops'] += record['status'] in ('timeout', 'rss-limit')
            counts['failures'] += record['status'] not in QUALITY_STATUSES
    return dict(same_model=same_model, formulation_contrasts=formulation_contrasts,
        family_groups=family_groups, cases=cases,
        clocks='publication checkpoints use solver-reported milliseconds; launch-inclusive publication timestamps are unavailable',
        missing_metrics='frontier/clone buffers and phases after kill are unavailable; allocation sizeof excludes dynamic storage')


def analyze(directory, output=None):
    config = json.loads((directory/'config.json').read_text())
    records = [completed(directory/'samples'/f"{row['id']}.json", row) for row in config['rows']]
    groups = {}
    for name in config['configs']:
        selected = [r for r in records if r and r['row']['config'] == name]
        good = [r for r in selected if r['status'] == 'ok']
        groups[name] = dict(completed=len(selected), failed=len(selected)-len(good),
            status_counts={status:sum(r['status'] == status for r in selected) for status in sorted(TERMINAL)},
            feasible=sum(bool(r['incumbents']) and r['status'] in QUALITY_STATUSES for r in selected),
            resource_terminated=sum(r['status'] in ('timeout', 'rss-limit') for r in selected),
            proved=sum(r['proof'] is not None for r in good),
            optimal=sum(r['proof'] == 'optimal' for r in good),
            infeasible=sum(r['proof'] == 'infeasible' for r in good),
            ordinary_stopped=sum(r['stop_reason'] == 'stopped' for r in good),
            actual_launch_wall_seconds=sum(r['wall_seconds'] for r in selected),
            median_wall_seconds=statistics.median(r['wall_seconds'] for r in good) if good else None,
            sampled_peak_rss_bytes=max((r['peak_rss_bytes'] for r in selected if r['peak_rss_bytes'] is not None), default=None))
    report = dict(expected=len(records), completed=sum(r is not None for r in records),
        unfinished=sum(r is None for r in records), groups=groups, paired=paired_analysis(config, records),
        comparisons=comparison_report(config, records),
        note='medians include only valid process runs; incomplete/failed work is counted; objectives and checked traces remain in individual rows')
    write_json(directory/'analysis.json', report)
    if output:
        compact = dict(schema_version=1, campaign=directory.name, freeze_sha256=digest(directory/'config.json'),
            expected=report['expected'], completed=report['completed'], unfinished=report['unfinished'],
            configurations=config['configs'], limits=dict(seconds=config['seconds'], threads=config['threads'],
                rss_bytes=config['rss_bytes'], sample_interval_seconds=config['sample_interval_seconds']),
            machine=config['machine'], identities={key:config.get(key) for key in
                ('native', 'didppy', 'runner_sha256', 'checker_sha256', 'manifest_sha256', 'sources')},
            groups=groups, comparisons=report['comparisons'])
        write_json(output, compact)
    print(json.dumps(report, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    for name in ('preview', 'run'):
        sub = commands.add_parser(name)
        sub.add_argument('--split', choices=['tiny', 'development', 'held-out'], default='tiny')
        sub.add_argument('--configs', help='comma-separated fixed configuration names')
        sub.add_argument('--case-ids', help='comma-separated development IDs or tiny/<name>.txt controls; held-out always uses the full split')
        sub.add_argument('--budget-ledger', type=Path, help='shared evaluation or task solver ledger')
        sub.add_argument('--budget-phase', choices=['development', 'correctness', 'held-out', 'verification'])
        sub.add_argument('--solver', type=Path, default=ROOT/'build/open-search/bin/tsptw')
        sub.add_argument('--uv', default='uv')
        sub.add_argument('--selected-root', type=Path, default=DATA/'selected')
        sub.add_argument('--repetitions', type=int)
        sub.add_argument('--seconds', type=float)
        sub.add_argument('--compilation-seconds', type=float, default=None, help='separately measured prebuild time; omitted means unavailable')
        if name == 'run':
            sub.add_argument('--name', required=True)
            sub.add_argument('--results-root', type=Path, default=ROOT/'build/tsptw-results')
            sub.add_argument('--max-samples', type=int, help='collect at most this many missing rows; full config stays fixed')
    sub = commands.add_parser('recover', help='confirm recorded child absence before resuming; never signals a PID')
    sub.add_argument('--name', required=True)
    sub.add_argument('--results-root', type=Path, default=ROOT/'build/tsptw-results')
    sub = commands.add_parser('analyze')
    sub.add_argument('--name', required=True)
    sub.add_argument('--results-root', type=Path, default=ROOT/'build/tsptw-results')
    sub.add_argument('--output', type=Path, help='also write the compact reproducible comparison')
    args = parser.parse_args()
    try:
        if args.command != 'preview' and not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_.-]*', args.name):
            raise ValueError('name must be a single safe path component')
        if args.command == 'recover':
            recover(args.results_root/args.name/'samples')
            return
        if args.command == 'analyze':
            analyze(args.results_root/args.name, args.output)
            return
        config = experiment(args)
        preview(config)
        if args.command == 'preview':
            return
        directory = args.results_root/args.name
        config_path = directory/'config.json'
        if config_path.exists() and json.loads(config_path.read_text()) != config:
            raise ValueError('configuration differs; choose a new run name')
        directory.mkdir(parents=True, exist_ok=True)
        samples = directory/'samples'
        samples.mkdir(exist_ok=True)
        if not config_path.exists():
            write_json(config_path, config)
        remaining = sum(completed(samples/f"{row['id']}.json", row) is None for row in config['rows'])
        count = 0
        for row in config['rows']:
            if completed(samples/f"{row['id']}.json", row):
                continue
            if args.max_samples is not None and count >= args.max_samples:
                break
            budget = config.get('budget') or {}
            task_budget = budget and budget_header(budget['ledger']).get('scope') == 'task'
            nominal = max(0, (remaining-1)*config['seconds']) if task_budget or budget.get('phase') == 'held-out' else (0 if budget.get('phase') == 'verification' else 5040)
            collect(config, row, samples, nominal)
            count += 1
            remaining -= 1
    except (OSError, ValueError) as error:
        parser.exit(1, '\n'.join([str(error), *getattr(error, '__notes__', [])])+'\n')


if __name__ == '__main__':
    main()
