#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# dependencies = []
# ///
"""Frozen serial CADDS/cursor/CABS study; one writer per named run, cwd paths."""
import argparse
from fractions import Fraction
import importlib.util
import itertools
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / 'examples/data/tsptw'
SPEC = importlib.util.spec_from_file_location('tsptw_benchmark', Path(__file__).with_name('tsptw-benchmark.py'))
benchmark = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(benchmark)
checker = benchmark.checker
DEVELOPMENT = ('Dumas/n20w20.001.txt', 'GendreauDumasExtended/n20w120.001.txt',
               'SolomonPotvinBengio/rc_201.1.txt')
CONFIGURATIONS = (('cadds-b1', 'cadds', 1), ('cadds-b4', 'cadds', 4),
                  ('cursor-b1', 'cursor', 1), ('cursor-b4', 'cursor', 4),
                  ('cabs', 'cabs', 1))
LEDGER = Path('/private/tmp/open-search-030-budget.jsonl')
HEADER = dict(event='budget', scope='task', task_id='open-search-030', total_seconds=300,
              verification_reserve_seconds=120, cleanup_reserve_seconds=20)


def cases():
    manifest = json.loads((DATA / 'manifest.json').read_text())
    selected = []
    for identity in DEVELOPMENT:
        item = next(row for row in manifest['cases'] if row['id'] == identity)
        if item['split'] != 'development':
            raise ValueError('case is not development: '+identity)
        path = DATA / 'published' / identity
        digest = benchmark.digest(path)
        if digest != item['sha256']:
            raise ValueError('manifest content mismatch: '+identity)
        selected.append(dict(id=identity, path=str(path.relative_to(ROOT)), sha256=digest))
    return selected


def experiment(args):
    binary = args.binary.resolve()
    frozen_cases = cases()  # Validate and freeze development before any suite launch.
    rows = []
    if args.suite == 'witness':
        rows.append(dict(id='row000', argv=[str(binary), 'witness'], cap_seconds=5,
                         config='semantic-witness', case=None, mode='witness', repetition=0))
    elif args.suite == 'tiny':
        for path in sorted((DATA / 'tiny').glob('*.txt')):
            for name, engine, width in CONFIGURATIONS:
                mode = 'opt' if engine == 'cabs' else 'coverage'
                rows.append(dict(id=f'row{len(rows):03}', config=name, repetition=0,
                    case=dict(id='tiny/'+path.name, path=str(path.relative_to(ROOT)), sha256=benchmark.digest(path)),
                    mode=mode, cap_seconds=1,
                    argv=[str(binary), engine, mode, str(width), '0', str(path.relative_to(ROOT))]))
    else:
        for repetition in range(2):
            configs = CONFIGURATIONS if repetition == 0 else tuple(reversed(CONFIGURATIONS))
            for case in frozen_cases:
                for name, engine, width in configs:
                    rows.append(dict(id=f'row{len(rows):03}', config=name, repetition=repetition,
                        case=case, mode='opt', cap_seconds=2,
                        argv=[str(binary), engine, 'opt', str(width), '1500', case['path']]))
    return dict(schema_version=1, suite=args.suite, binary=str(binary), binary_sha256=benchmark.digest(binary),
        source_sha256=benchmark.digest(Path(__file__).with_suffix('.cpp')),
        runner_sha256=benchmark.digest(__file__), development_cases=frozen_cases,
        configurations=[dict(name=n, engine=e, width=w) for n, e, w in CONFIGURATIONS],
        repetitions=2, ordinary_stop_ms=1500, development_outer_cap_seconds=2,
        development_rows=30, development_nominal_seconds=60,
        model=dict(representation='lazy', bound='MIN_ARCS', rank='bound', residual_propagation='OFF',
                   state_pruning='OFF', branching='ascending native visits', seed=None, threads=1),
        rss_bytes=1024**3, sample_interval_seconds=0.05, rows=rows)


def feasible_tours(data):
    tours = set()
    for tour in itertools.permutations(range(1, len(data[0]))):
        try:
            checker.check_tour(data, tour)
        except ValueError:
            continue
        tours.add(tour)
    return tours


def checked_output(path, row, capture):
    records = []
    lines = path.read_text().splitlines()
    for index, line in enumerate(lines):
        try:
            records.append(json.loads(line))
        except json.JSONDecodeError:
            if capture['status'] in ('timeout', 'rss-limit') and index == len(lines)-1:
                break  # A killed process can leave one unfinished record.
            raise ValueError('malformed native JSON output')
    if row['mode'] == 'witness':
        if capture['status'] != 'ok' or len(records) != 1 or records[0].get('kind') != 'witness':
            raise ValueError('semantic witness did not pass')
        return dict(witness=records[0], trace=[], summary=None, checked_publications=0)
    data = checker.read_instance(ROOT / row['case']['path'])
    trace, summary, tours = [], None, set()
    previous_time = -1
    for record in records:
        if record.get('kind') in ('solution', 'coverage'):
            if summary is not None:
                raise ValueError('publication after summary')
            expected = 'coverage' if row['mode'] == 'coverage' else 'solution'
            if record['kind'] != expected:
                raise ValueError('incorrect publication kind')
            cost, elapsed = record['scaled_cost'], record['elapsed_ms']
            if not benchmark.nonnegative_number(cost, integer=True) or not benchmark.nonnegative_number(elapsed):
                raise ValueError('invalid publication metric')
            result = checker.check_tour(data, record['tour'], Fraction(cost, data[2]))
            if elapsed < previous_time:
                raise ValueError('native publication clock decreased')
            if row['mode'] == 'opt' and trace and cost >= trace[-1]['scaled_cost']:
                raise ValueError('incumbents do not strictly improve')
            previous_time = elapsed
            tour = tuple(record['tour'])
            if row['mode'] == 'coverage' and tour in tours:
                raise ValueError('duplicate coverage tour')
            tours.add(tour)
            trace.append(dict(scaled_cost=cost, elapsed_ms=elapsed, tour=record['tour'], exact_cost=result['cost']))
        elif record.get('kind') == 'summary' and summary is None:
            summary = record
        else:
            raise ValueError('unexpected native record')
    if capture['status'] == 'ok' and summary is None:
        raise ValueError('successful run has no summary')
    if summary:
        best = min((item['scaled_cost'] for item in trace), default=None)
        if (summary.get('best_scaled_cost') != best or summary.get('publications') != len(trace) or
                summary.get('scale') != data[2] or summary.get('elapsed_ms', -1) < previous_time):
            raise ValueError('summary disagrees with publications')
        termination = summary.get('termination')
        if termination not in ('optimal', 'infeasible', 'stopped', 'exhausted'):
            raise ValueError('unknown termination')
        if (termination == 'optimal' and best is None) or (termination == 'infeasible' and best is not None):
            raise ValueError('contradictory proof')
        if row['case']['id'].startswith('tiny/'):
            expected = checker.enumerate_tours(data)
            optimum = expected['optimum']['scaled_cost'] if expected['optimum'] else None
            if termination == 'stopped' or best != optimum:
                raise ValueError('tiny optimum/exhaustion mismatch')
            if row['mode'] == 'coverage' and (termination != 'exhausted' or tours != feasible_tours(data)):
                raise ValueError('tiny complete tour identities mismatch')
    return dict(trace=trace, summary=summary, checked_publications=len(trace))


def load_sample(samples, row):
    path = samples / (row['id']+'.json')
    if not path.exists():
        return None
    record = json.loads(path.read_text())
    if record.get('row') != row or record.get('capture', {}).get('status') not in benchmark.TERMINAL:
        raise ValueError('invalid saved terminal sample: '+str(path))
    return record


def run(args):
    config = experiment(args)
    if args.dry_run:
        print(json.dumps(dict(config=config, nominal_seconds=sum(r['cap_seconds'] for r in config['rows']),
            ledger=str(LEDGER), budget=HEADER), indent=2))
        return
    directory = args.results_root / args.name
    directory.mkdir(parents=True, exist_ok=True)
    saved = directory / 'config.json'
    if saved.exists() and json.loads(saved.read_text()) != config:
        raise ValueError('material configuration changed; use a new experiment name')
    if not saved.exists():
        benchmark.write_json(saved, config)
    if not LEDGER.exists():
        with LEDGER.open('x') as output:
            output.write(json.dumps(HEADER)+'\n')
    if benchmark.budget_header(LEDGER) != HEADER:
        raise ValueError('unexpected task budget header')
    samples = directory / 'samples'
    samples.mkdir(exist_ok=True)
    benchmark.require_reaped(samples)
    selected = args.only.split(',') if args.only else None
    if selected and any(identity not in {row['id'] for row in config['rows']} for identity in selected):
        raise ValueError('unknown selected row')
    launched = 0
    for index, row in enumerate(config['rows']):
        if selected and row['id'] not in selected:
            continue
        if load_sample(samples, row):
            continue
        if args.limit is not None and launched >= args.limit:
            break
        remaining = sum(other['cap_seconds'] for other in config['rows'][index+1:]
                        if (not selected or other['id'] in selected) and not load_sample(samples, other))
        # The budget must cover every unlaunched frozen development run.
        if args.phase == 'correctness':
            remaining += 60
        attempt = 1
        destination = samples / row['id']
        while destination.with_suffix('.stdout').exists() or destination.with_suffix('.stderr').exists():
            attempt += 1
            destination = samples / (row['id']+f'-attempt{attempt}')
        capture = benchmark.capture(row['argv'], destination, row['cap_seconds'],
            config['rss_bytes'], config['sample_interval_seconds'],
            budget=dict(ledger=str(LEDGER), phase=args.phase), remaining_nominal_seconds=remaining)
        record = dict(row=row, capture=capture, stream=destination.name)
        try:
            record['checked'] = checked_output(destination.with_suffix('.stdout'), row, capture)
        except ValueError as error:
            record['capture'] = dict(capture, status='invalid-output', validation_error=str(error))
        benchmark.write_json(samples / (row['id']+'.json'), record)
        launched += 1
        print(json.dumps(dict(id=row['id'], status=record['capture']['status'], wall_seconds=capture['wall_seconds'])), flush=True)


def analyze(args):
    directory = args.results_root / args.name
    config = json.loads((directory / 'config.json').read_text())
    rows, missing = [], []
    for row in config['rows']:
        record = load_sample(directory / 'samples', row)
        if record is None:
            missing.append(row['id'])
            continue
        # Recheck raw publications independently on every offline analysis.
        checked = checked_output(directory / 'samples' / (record['stream']+'.stdout'), row, record['capture'])
        capture, trace, summary = record['capture'], checked['trace'], checked['summary']
        best = min(trace, key=lambda item: item['scaled_cost']) if trace else None
        rows.append(dict(id=row['id'], config=row['config'], case=row['case'], repetition=row['repetition'],
            argv=row['argv'], status=capture['status'], summary=summary, witness=checked.get('witness'),
            checked_publications=len(trace), first_publication_ms=trace[0]['elapsed_ms'] if trace else None,
            best_publication_ms=best['elapsed_ms'] if best else None, best=best,
            incumbent_trace=[dict(scaled_cost=t['scaled_cost'], elapsed_ms=t['elapsed_ms']) for t in trace],
            end_to_end_wall_seconds=capture['wall_seconds'], sampled_peak_rss_bytes=capture['peak_rss_bytes'],
            rss_samples=capture['rss_samples'], maximum_sample_gap_seconds=capture['maximum_sample_gap_seconds'],
            rss_probe_seconds=capture['rss_probe_seconds'], termination_reap_seconds=capture['termination_reap_seconds'],
            wall_cap_overshoot_seconds=capture['wall_cap_overshoot_seconds'], rss_overshoot_bytes=capture['rss_overshoot_bytes']))
    result = dict(schema_version=1, experiment=args.name, configuration=config, expected_rows=len(config['rows']),
                  completed_rows=len(rows), missing_rows=missing, rows=rows)
    output = args.output or directory / 'analysis.json'
    benchmark.write_json(output, result)
    print(json.dumps(dict(output=str(output), completed=len(rows), missing=len(missing),
        checked_publications=sum(r['checked_publications'] for r in rows))))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    for name in ('run', 'analyze', 'recover'):
        sub = commands.add_parser(name)
        sub.add_argument('--name', required=True)
        sub.add_argument('--results-root', type=Path, default=Path('/private/tmp/open-search-030-study/results'))
        if name == 'run':
            sub.add_argument('--binary', type=Path, required=True)
            sub.add_argument('--suite', choices=('witness', 'tiny', 'development'), default='development')
            sub.add_argument('--phase', choices=('correctness', 'development', 'verification'), default='development')
            sub.add_argument('--dry-run', action='store_true')
            sub.add_argument('--limit', type=int)
            sub.add_argument('--only', help='comma-separated frozen row IDs for independent verification')
        if name == 'analyze':
            sub.add_argument('--output', type=Path)
    args = parser.parse_args()
    if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9_.-]*', args.name):
        parser.error('name must be one safe path component')
    try:
        if args.command == 'run':
            run(args)
        elif args.command == 'analyze':
            analyze(args)
        else:
            benchmark.recover(args.results_root / args.name / 'samples')
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
