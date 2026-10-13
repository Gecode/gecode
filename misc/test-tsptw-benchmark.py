"""Focused checks for record reuse and false proof/parser hazards."""
import importlib.util
import copy
import contextlib
import io
from unittest.mock import Mock, patch
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('benchmark', Path(__file__).with_name('tsptw-benchmark.py'))
benchmark = importlib.util.module_from_spec(spec)
spec.loader.exec_module(benchmark)


class Records(unittest.TestCase):
    def test_case_selector_cannot_leak_held_out_cases_into_development(self):
        case = 'Dumas/n20w20.001.txt'
        selected = benchmark.select_cases('development', Path('/tmp/selected'), case)
        self.assertEqual([row['id'] for row in selected], [case])
        for split, names in (('held-out', case), ('tiny', case),
                ('development', case+','+case), ('development', 'Dumas/n40w20.002.txt')):
            with self.subTest(split=split, names=names), self.assertRaises(ValueError):
                benchmark.select_cases(split, Path('/tmp/selected'), names)
        selected = benchmark.select_cases('development', Path('/tmp/selected'), case+',tiny/late-return.txt')
        self.assertEqual([row['id'] for row in selected], [case, 'tiny/late-return.txt'])
        self.assertEqual(len(benchmark.select_cases('development', Path('/tmp/selected'))), 12)
        with self.assertRaises(ValueError):
            benchmark.select_cases('development', Path('/tmp/selected'), 'tiny/../late-return.txt')

    def test_task_budget_preserves_verification_and_cleanup(self):
        with tempfile.TemporaryDirectory() as directory:
            ledger = Path(directory)/'budget.jsonl'
            header = dict(event='budget', scope='task', task_id='open-search-028',
                total_seconds=300, verification_reserve_seconds=60, cleanup_reserve_seconds=20)
            ledger.write_text(json.dumps(header)+'\n')
            attempt = benchmark.admit_budget(ledger, 'correctness', 15, 171)
            with self.assertRaisesRegex(ValueError, 'unreaped'):
                benchmark.admit_budget(ledger, 'development', 2, 0)
            benchmark.budget_event(ledger, dict(event='reap', id=attempt, wall_seconds=200))
            with self.assertRaisesRegex(ValueError, 'task allowance'):
                benchmark.admit_budget(ledger, 'development', 21, 0)
            with self.assertRaisesRegex(ValueError, 'excludes'):
                benchmark.admit_budget(ledger, 'held-out', 1, 0)
            with self.assertRaisesRegex(ValueError, 'verification allowance'):
                benchmark.admit_budget(ledger, 'verification', 61, 0)
            attempt = benchmark.admit_budget(ledger, 'verification', 15, 0)
            benchmark.budget_event(ledger, dict(event='reap', id=attempt, wall_seconds=15))
            self.assertEqual(benchmark.budget_state(ledger), (dict(correctness=200, verification=15), {}))
            attempt = benchmark.admit_budget(ledger, 'verification', 45, 0)
            benchmark.budget_event(ledger, dict(event='unresolved', id=attempt, wall_seconds=3))
            with self.assertRaisesRegex(ValueError, 'unreaped'):
                benchmark.admit_budget(ledger, 'verification', 1, 0)

    def test_summary_uses_saved_exact_transition_arguments(self):
        # The historical display name deliberately differs from the frozen arguments.
        row = dict(config='transition-astar', arguments=benchmark.CONFIGS['transition-astar-lazy-min-arcs'])
        summary = dict(type='summary', engine='astar', formulation='transition', bound='min-arcs',
            residual_propagation='off', state_pruning='off', termination='infeasible', incumbent=None,
            elapsed_ms=2, preprocessing_ms=1, search_ms=1, statistics=dict(node=1))
        benchmark.validate_summary(summary, [], row)
        for field, wrong in (('bound', 'prefix'), ('residual_propagation', 'unary'), ('state_pruning', 'pareto')):
            with self.subTest(field=field), self.assertRaisesRegex(ValueError, field):
                benchmark.validate_summary(dict(summary, **{field:wrong}), [], row)
        column = dict(summary, engine='column', bound='prefix', initial_quota=1, quota_growth=1, frontier_limit=0)
        with self.assertRaisesRegex(ValueError, 'initial_quota'):
            benchmark.validate_summary(dict(column, initial_quota=2), [], dict(config='transition-column'))

    def test_malformed_terminal_record_is_unfinished(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'row.json'
            path.write_text(json.dumps(dict(row={}, status='ok', wall_seconds=float('nan'), incumbents=[], diagnostic='')))
            self.assertIsNone(benchmark.completed(path, {}))
            path.write_text('{')
            self.assertIsNone(benchmark.completed(path, {}))

    def test_malformed_saved_contents_do_not_reuse_or_count_as_proof(self):
        checked = benchmark.checker.check_tour(
            benchmark.checker.read_instance(benchmark.DATA/'tiny/waiting.txt'), [1, 2])
        incumbent = dict(tour=[1, 2], **{k: checked[k] for k in ('cost', 'scaled_cost', 'scale', 'return_time')})
        trace = dict(incumbent, type='solution', checked=True, elapsed_ms=1)
        summary = dict(type='summary', engine='bab', formulation='closed', termination='optimal',
            incumbent=incumbent, elapsed_ms=2, preprocessing_ms=0.5, search_ms=1.5, statistics=dict(node=3))
        metrics = dict(preprocessing_ms=0.5, search_ms=1.5, startup_and_teardown_ms=1,
            expansions=3, descriptor_count=None, descriptor_bytes=None, allocation=None,
            frontier_bytes=None, clone_bytes=None)
        row = dict(config='closed-bab', argv=['solver', 'waiting.txt'])
        valid = dict(row=row, status='ok', wall_seconds=0.003, diagnostic='', incumbents=[trace],
            final_objective=5, metrics=metrics, proof='optimal', summary=summary,
            stop_reason='optimal', returncode=0, peak_rss_bytes=None, argv=row['argv'])
        mutations = [
            ('incumbents', [42]), ('summary', {}), ('final_objective', 'not-an-objective'),
            ('metrics', dict(metrics, search_ms='not-a-measurement')),
            ('metrics', dict(metrics, search_ms=float('nan'))), ('final_objective', 6),
            ('proof', 'infeasible'), ('summary', dict(summary, incumbent=dict(incumbent, tour=[2, 1]))),
            ('argv', ['solver', 'different.txt']),
        ]
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'row.json'
            path.write_text(json.dumps(valid))
            self.assertIsNotNone(benchmark.completed(path, row))
            for field, replacement in mutations:
                with self.subTest(field=field, replacement=replacement):
                    value = copy.deepcopy(valid)
                    value[field] = replacement
                    path.write_text(json.dumps(value))
                    self.assertIsNone(benchmark.completed(path, row))

    def test_malformed_live_measurements_are_reusable_terminal_failures(self):
        instance = benchmark.DATA/'tiny/waiting.txt'
        checked = benchmark.checker.check_tour(benchmark.checker.read_instance(instance), [1, 2])
        incumbent = dict(tour=[1, 2], **{k: checked[k] for k in ('cost', 'scaled_cost', 'scale', 'return_time')})
        solution = dict(incumbent, type='solution', elapsed_ms=1)
        summary = dict(type='summary', engine='astar', formulation='transition', termination='optimal',
            incumbent=incumbent, elapsed_ms=2, preprocessing_ms=0.5, search_ms=1.5,
            statistics=dict(node=3, state_descriptors=0, state_bytes=0),
            allocation=dict(space_bytes=1, intvar_handle_bytes=1, root_positions=0,
                prepared_root_positions=1, solution_positions=3),
            measurements=dict(bound_calls=1, bound_ms=0.1, inference_calls=0, inference_ms=0, propagation_ms=0))
        row = dict(id='sample', config='transition-astar', case=dict(path=str(instance), sha256=benchmark.digest(instance)))
        config = dict(configs=benchmark.CONFIGS, native=dict(path='unused'), seconds=1,
            rss_bytes=1024**3, sample_interval_seconds=0.05)
        for section, field in (('statistics', 'node'), ('statistics', 'state_bytes'),
                ('allocation', 'space_bytes'), ('measurements', 'bound_ms')):
            with self.subTest(section=section, field=field), tempfile.TemporaryDirectory() as directory:
                malformed = copy.deepcopy(summary)
                malformed[section][field] = 'not-a-measurement'
                def capture(argv, destination, *limits):
                    destination.with_suffix('.stdout').write_text(json.dumps(solution)+'\n'+json.dumps(malformed)+'\n')
                    destination.with_suffix('.stderr').write_text('')
                    return dict(status='ok', returncode=0, wall_seconds=0.003, diagnostic='', peak_rss_bytes=None)
                samples = Path(directory)
                with patch.object(benchmark, 'capture', side_effect=capture), contextlib.redirect_stdout(io.StringIO()):
                    benchmark.collect(config, row, samples)
                result = benchmark.completed(samples/'sample.json', row)
                self.assertIsNotNone(result)
                self.assertEqual(result['status'], 'invalid-output')
                self.assertIsNone(result['proof'])
                self.assertEqual(result['final_objective'], 5)
                self.assertEqual(len(result['incumbents']), 1)
                self.assertIsNone(result['summary'])
                self.assertIn(field, result['diagnostic'])

    def test_failed_cleanup_preserves_error_streams_and_blocks_resume(self):
        for failure in ('kill', 'wait'):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as directory:
                samples = Path(directory)
                ledger = samples/'budget.jsonl'
                ledger.write_text(json.dumps(dict(event='budget', total_seconds=7200))+'\n')
                process = Mock(pid=321, returncode=None)
                process.poll.return_value = None
                original = RuntimeError('original probe failure')
                cleanup = PermissionError('cleanup permission denied')
                def spawn(*args, **kwargs):
                    kwargs['stdout'].write(b'partial output\n')
                    kwargs['stderr'].write(b'partial diagnostic\n')
                    return process
                process.wait.side_effect = cleanup if failure == 'wait' else None
                with patch.object(benchmark.subprocess, 'Popen', side_effect=spawn), \
                        patch.object(benchmark.subprocess, 'run', side_effect=original), \
                        patch.object(benchmark.os, 'killpg', side_effect=cleanup if failure == 'kill' else None), \
                        patch.object(benchmark.time, 'perf_counter', side_effect=range(20)):
                    with self.assertRaises(RuntimeError) as raised:
                        benchmark.capture(['solver'], samples/'sample', 100, 1000, 0.05,
                            budget=dict(ledger=str(ledger), phase='correctness'), remaining_nominal_seconds=0)
                self.assertIs(raised.exception, original)
                self.assertIn('cleanup permission denied', original.__notes__[0])
                self.assertEqual((samples/'sample.stdout').read_text(), 'partial output\n')
                self.assertEqual((samples/'sample.stderr').read_text(), 'partial diagnostic\n')
                used, pending = benchmark.budget_state(ledger)
                self.assertEqual(len(pending), 1)
                launch = json.loads((samples/'sample.launch.json').read_text())
                self.assertEqual(used['correctness'], launch['wall_seconds'])
                self.assertGreater(used['correctness'], 0)
                self.assertEqual(next(iter(pending.values()))['pid'], 321)
                self.assertNotIn('"event": "reap"', ledger.read_text())
                with patch.object(benchmark.subprocess, 'Popen') as spawn:
                    with self.assertRaisesRegex(ValueError, '321'):
                        benchmark.capture(['solver'], samples/'other', 1, 1000, 0.05)
                    with self.assertRaisesRegex(ValueError, '321'):
                        benchmark.collect({}, {}, samples)
                    spawn.assert_not_called()
                with self.assertRaisesRegex(ValueError, '321'):
                    benchmark.admit_budget(ledger, 'correctness', 1, 0)

    def test_confirmed_exit_and_failed_spawn_close_launch(self):
        for mode in ('exit', 'kill', 'spawn-failure'):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as directory:
                samples = Path(directory)
                ledger = samples/'budget.jsonl'
                ledger.write_text(json.dumps(dict(event='budget', total_seconds=7200))+'\n')
                process = Mock(pid=321, returncode=0)
                process.poll.return_value = None if mode == 'kill' else 0
                with patch.object(benchmark.subprocess, 'Popen',
                        side_effect=OSError('spawn failed') if mode == 'spawn-failure' else None,
                        return_value=process), patch.object(benchmark.os, 'killpg'), \
                        patch.object(benchmark.time, 'perf_counter', side_effect=range(20)):
                    result = benchmark.capture(['solver'], samples/'sample', 0.1, 1000, 0.05,
                        budget=dict(ledger=str(ledger), phase='correctness'), remaining_nominal_seconds=0)
                self.assertEqual(result['status'], 'execution-error' if mode == 'spawn-failure' else
                    'timeout' if mode == 'kill' else 'ok')
                self.assertFalse((samples/'sample.launch.json').exists())
                used, pending = benchmark.budget_state(ledger)
                self.assertFalse(pending)
                self.assertGreater(used['correctness'], 0)

    def test_recovery_requires_absence_and_keeps_measured_accounting(self):
        with tempfile.TemporaryDirectory() as directory:
            samples = Path(directory)
            ledger = samples/'budget.jsonl'
            ledger.write_text('\n'.join(json.dumps(row) for row in (
                dict(event='budget', total_seconds=7200),
                dict(event='launch', id='attempt', phase='correctness'),
                dict(event='unresolved', id='attempt', wall_seconds=5)))+'\n')
            marker = samples/'sample.launch.json'
            launch = dict(id='attempt', pid=321, pgid=321, argv=['solver'], wall_seconds=5, started_at=100,
                budget=dict(ledger=str(ledger), phase='correctness'))
            benchmark.write_json(marker, launch)
            for error in (None, PermissionError('cannot inspect PID')):
                with patch.object(benchmark.os, 'kill', side_effect=error) as probe, \
                        patch.object(benchmark.os, 'killpg') as signal_group:
                    with self.assertRaises((ValueError, PermissionError)):
                        benchmark.recover(samples)
                    probe.assert_called_once_with(321, 0)
                    signal_group.assert_not_called()
                    self.assertTrue(marker.exists())
            with patch.object(benchmark.os, 'kill', side_effect=ProcessLookupError), \
                    patch.object(benchmark.time, 'time', return_value=110), \
                    patch.object(benchmark.os, 'killpg') as signal_group:
                benchmark.recover(samples)
                signal_group.assert_not_called()
            self.assertFalse(marker.exists())
            used, pending = benchmark.budget_state(ledger)
            self.assertEqual(used, dict(correctness=10))
            recovery = json.loads(ledger.read_text().splitlines()[-1])
            self.assertEqual(recovery['observed_wall_seconds'], 5)
            self.assertIn('exit time unobserved', recovery['timing'])
            self.assertFalse(pending)
            benchmark.require_reaped(samples)
            # Historical reaps retain their original accounting interpretation.
            ledger.write_text('\n'.join(json.dumps(row) for row in (
                dict(event='budget', total_seconds=7200),
                dict(event='launch', id='old', phase='correctness'),
                dict(event='reap', id='old', wall_seconds=7)))+'\n')
            self.assertEqual(benchmark.budget_state(ledger), (dict(correctness=7), {}))

    def test_infeasibility_proof_rejects_any_checked_tour_for_same_case(self):
        case = dict(id='waiting.txt')
        config = dict(cases=[case], seconds=1, configs={})
        infeasible = dict(row=dict(case=case), proof='infeasible', incumbents=[], final_objective=None)
        for status in ('ok', 'timeout', 'rss-limit', 'invalid-output'):
            with self.subTest(status=status):
                feasible = dict(row=dict(case=case), proof=None, status=status,
                    incumbents=[dict(checked=True, scaled_cost=5)], final_objective=5)
                with self.assertRaisesRegex(ValueError, 'infeasibility proof for waiting.txt'):
                    benchmark.comparison_report(config, [infeasible, feasible])

    def test_checker_keeps_valid_trace_before_rejecting_false_proof(self):
        data = benchmark.checker.read_instance(benchmark.DATA/'tiny/waiting.txt')
        solution = dict(type='solution', elapsed_ms=1, tour=[1, 2], **{k: v for k, v in
            benchmark.checker.check_tour(data, [1, 2]).items() if k != 'visits'})
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'output'
            path.write_text(json.dumps(solution)+'\n'+json.dumps(dict(type='summary', termination='infeasible', incumbent=None))+'\n')
            trace, summary, errors = benchmark.checked_output(path, data)
            self.assertEqual(len(trace), 1)
            self.assertTrue(errors)

    def test_checks_later_incumbents_after_an_invalid_line(self):
        data = benchmark.checker.read_instance(benchmark.DATA/'tiny/waiting.txt')
        value = benchmark.checker.check_tour(data, [1, 2])
        solution = dict(type='solution', elapsed_ms=1, tour=[1, 2], **{k: v for k, v in value.items() if k != 'visits'})
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'output'
            path.write_text('bad json\n'+json.dumps(solution)+'\n')
            trace, _, errors = benchmark.checked_output(path, data)
            self.assertTrue(errors)
            self.assertEqual(len(trace), 1)
            self.assertTrue(trace[0]['checked'])


if __name__ == '__main__':
    unittest.main()
