"""Run with PYTHONPATH=python GECODE_OPTIMIZE_LIBRARY=/path/to/library python3 -m unittest discover -s python/tests -v."""
import concurrent.futures
import math
import tempfile
import threading
import unittest
from pathlib import Path
from unittest.mock import patch

from gecode_optimize import (ApiError, Backend, Cancellation, GlobalConstraint, Guarantee, Indicator, Model,
                             Options, Session, Termination, VariableType, load_library)


class Conformance(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib = load_library()
        cls.highs = cls.lib.capabilities(Backend.HIGHS)["available"]
        cls.native = cls.lib.capabilities(Backend.NATIVE)["available"]

    def assert_solution(self, result, objective, values=()):
        if not self.highs:
            self.assertEqual(result.termination, Termination.UNSUPPORTED)
            self.assertFalse(result.has_solution)
            self.assertIsNone(result.objective)
            self.assertIsNone(result.best_bound)
            self.assertTrue(result.message)
            return
        self.assertEqual(result.termination, Termination.OPTIMAL, result.message)
        self.assertTrue(result.info["solution_validated"])
        self.assertAlmostEqual(result.objective, objective, places=6)
        for variable, expected in values:
            self.assertAlmostEqual(result.value(variable), expected, places=6)

    def test_lp_continuous_recourse_and_result_lifetime(self):
        # Analytic optimum: x + y >= 2.5, x integer, y >= 0,
        # min 2x + 3y + 0.25 => x=2, y=0.5, objective=5.75.
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, upper=5)
            y = model.add_variable(upper=5)
            dead = model.add_variable(VariableType.BINARY)
            model.remove(dead)
            model.add_row({x: 1, y: 1}, lower=2.5)
            model.set_objective({x: 2, y: 3}, offset=.25)
            result = model.solve(Options(backend=Backend.HIGHS))
            identity = model.identity
        self.assertTrue(model.closed)
        with result:
            self.assert_solution(result, 5.75, [(x, 2), (y, .5)])
            self.assertEqual((result.info["model_id"], result.info["revision"]), identity)
            self.assertEqual(result.backend, "HiGHS")
            records = result.values
            self.assertEqual(records[2], {"active": False, "present": False, "value": None})
            self.assertEqual(records[0]["present"], self.highs)
            if self.highs:
                self.assertTrue(result.backend_version)
                with self.assertRaises(ApiError):
                    result.value(dead)
            else:
                with self.assertRaises(ApiError) as error:
                    result.value(x)
                self.assertEqual(error.exception.code, 7)
        result.close()  # sequential close is idempotent
        with self.assertRaises(RuntimeError):
            result.info

    def test_lp_fractional_and_maximum_offset(self):
        with Model(self.lib) as model:
            x = model.add_variable(upper=10)
            model.add_row({x: 2}, lower=3, upper=5)
            model.set_objective({x: 4}, maximize=True, offset=-7)
            with model.solve(Options(backend=Backend.HIGHS)) as result:
                self.assert_solution(result, 3, [(x, 2.5)])

    def test_domains_and_semi_zero_exception(self):
        # Independent one-variable optima exercise every public type.
        cases = [(VariableType.CONTINUOUS, 0, 3, 1.5, 1.5),
                 (VariableType.INTEGER, 0, 3, 1.5, 2),
                 (VariableType.BINARY, 0, 1, .5, 1),
                 (VariableType.SEMI_CONTINUOUS, 2, 5, 1, 2),
                 (VariableType.SEMI_INTEGER, 2.5, 5, 1, 3),
                 (VariableType.SEMI_CONTINUOUS, 2, 5, 0, 0),
                 (VariableType.SEMI_INTEGER, 2.5, 5, 0, 0)]
        for kind, lower, upper, demand, optimum in cases:
            with self.subTest(kind=kind, demand=demand), Model(self.lib) as model:
                x = model.add_variable(kind, lower, upper)
                model.add_row({x: 1}, lower=demand)
                model.set_objective({x: 1})
                with model.solve(Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, optimum, [(x, optimum)])

    def test_session_edits_and_snapshot_history(self):
        with Model(self.lib) as model, Session(self.lib) as session:
            x = model.add_variable(VariableType.INTEGER, upper=8)
            row = model.add_row({x: 1}, lower=2)
            model.set_objective({x: 1}, offset=3)
            first = session.solve(model, Options(backend=Backend.HIGHS))
            with first:
                self.assert_solution(first, 5, [(x, 2)])
                model.set_bounds(row, 3, math.inf)
                model.set_bounds(x, 3, 8)
                model.set_objective_coefficient(x, 2)
                model.set_objective_offset(-1)
                model.set_name(x, "quantity")
                model.set_name(row, "demand")
                with session.solve(model, Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, 5, [(x, 3)])
                    if self.highs:
                        self.assertEqual(result.info["revision"], model.identity[1])
                        self.assertLess(first.info["revision"], result.info["revision"])
                        self.assertEqual(first.value(x), 2)
                        self.assertEqual(session.statistics["model_loads"], 1)
                        self.assertEqual(session.statistics["incremental_updates"], 1)
                model.set_coefficient(row, x, 2)
                with session.solve(model, Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, 5, [(x, 3)])
                    if self.highs:
                        self.assertEqual(session.statistics["model_loads"], 2)
                session.reset()
                with session.solve(model, Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, 5)
            model.remove(row)
            model.set_objective()
            model.remove(x)
            with model.solve(Options(backend=Backend.HIGHS)) as result:
                self.assert_solution(result, 0)

    def test_api_errors_and_safe_integer_conversion(self):
        with Model(self.lib) as model, Model(self.lib) as foreign:
            x = model.add_variable(upper=3)
            y = foreign.add_variable()
            with self.assertRaises(ApiError):
                model.add_row({y: 1})
            row = model.add_row({x: 1}, lower=1)
            with self.assertRaises(TypeError):
                model.set_coefficient(x, row, 1)
            with self.assertRaises(ApiError):
                model.remove(x)
            with self.assertRaises(ApiError):
                model.set_bounds(x, math.nan, 1)
            with self.assertRaises(ApiError):
                model.set_objective({x: math.inf})
            with self.assertRaises(ValueError):
                model.add_variable(name="bad\0name")
            with self.assertRaises(ValueError):
                model.add_variable(type=99)
            for options, error in [(Options(node_limit=-1), OverflowError),
                                   (Options(node_limit=2**64), OverflowError),
                                   (Options(threads=2**31), OverflowError),
                                   (Options(random_seed=.5), TypeError),
                                   (Options(threads=True), TypeError),
                                   (Options(time_limit_seconds=math.nan), ApiError),
                                   (Options(integrality_tolerance=.5), ApiError),
                                   (Options(primal_start={x: math.inf}), ApiError)]:
                with self.subTest(options=options), self.assertRaises(error):
                    model.solve(options)
            model.remove(row)
            model.remove(x)
            with self.assertRaises(ApiError):
                model.set_bounds(x, 0, 1)
        with self.assertRaises(RuntimeError):
            model.add_variable()

    def test_starts_cancellation_options_and_guarantees(self):
        with Model(self.lib) as model, Session(self.lib) as session, Cancellation(self.lib) as token:
            x = model.add_variable(VariableType.INTEGER, upper=8)
            model.set_objective({x: 1}, maximize=True)
            with model.solve(Options(backend=Backend.HIGHS, primal_start={x: 6})) as result:
                self.assert_solution(result, 8)
                self.assertEqual(result.info["start_submitted"], self.highs)
            token.cancel()
            with session.solve(model, Options(backend=Backend.HIGHS, cancellation=token)) as result:
                self.assertEqual(result.termination, Termination.CANCELLED)
                self.assertFalse(result.has_solution)
                self.assertIsNone(result.objective)
            with session.solve(model, Options(backend=Backend.HIGHS)) as result:
                self.assert_solution(result, 8)
            with model.solve(Options(backend=Backend.HIGHS, guarantee=Guarantee.CERTIFIED)) as result:
                self.assertEqual(result.termination, Termination.UNSUPPORTED)
            with model.solve(Options(backend=Backend.HIGHS, time_limit_seconds=0)) as result:
                self.assertEqual(result.termination, Termination.TIME_LIMIT)
            token.close()
            with self.assertRaises(RuntimeError):
                model.solve(Options(cancellation=token))

    def test_infeasible_unbounded_and_explicit_native(self):
        with Model(self.lib) as model:
            x = model.add_variable(upper=1)
            model.add_row({x: 1}, lower=2)
            with model.solve(Options(backend=Backend.HIGHS)) as result:
                self.assertEqual(result.termination, Termination.INFEASIBLE if self.highs else Termination.UNSUPPORTED)
                self.assertIsNone(result.objective)
        with Model(self.lib) as model:
            x = model.add_variable()
            model.set_objective({x: -1})
            with model.solve(Options(backend=Backend.HIGHS)) as result:
                expected = (Termination.UNBOUNDED, Termination.INFEASIBLE_OR_UNBOUNDED) if self.highs else (Termination.UNSUPPORTED,)
                self.assertIn(result.termination, expected)
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, upper=4)
            model.set_objective({x: 1}, maximize=True, offset=-1)
            with model.solve(Options(backend=Backend.NATIVE, guarantee=Guarantee.EXACT)) as result:
                native = self.lib.capabilities(Backend.NATIVE)["available"]
                self.assertEqual(result.termination, Termination.OPTIMAL if native else Termination.UNSUPPORTED)
                if native:
                    self.assertEqual(result.objective, 3)
                    self.assertEqual(result.value(x), 4)

    def test_read_prepares_owner_before_allocating_c_model(self):
        class AllocationFailure(Model):
            def __new__(cls):
                raise MemoryError("owner allocation failed")

        class InitializationFailure(Model):
            def _initialize(self, library, handle=None):
                raise MemoryError("owner initialization failed")

        for owner in (AllocationFailure, InitializationFailure):
            with self.subTest(owner=owner.__name__):
                with patch.object(self.lib, "call", wraps=self.lib.call) as call:
                    with self.assertRaises(MemoryError):
                        owner.read("not-read.lp", self.lib)
                    call.assert_not_called()

    def test_io_roundtrip_and_failure_preserves_destination(self):
        with tempfile.TemporaryDirectory() as directory, Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, upper=8, name="quantity")
            model.add_row({x: 2}, lower=3, upper=9, name="demand")
            model.set_objective({x: 3}, maximize=True, offset=-1.25)
            for extension in ("lp", "mps"):
                path = Path(directory) / ("model." + extension)
                model.write(path)
                with Model.read(path, self.lib) as imported, imported.solve(Options(backend=Backend.HIGHS)) as result:
                    self.assertNotEqual(imported.identity[0], model.identity[0])
                    self.assert_solution(result, 10.75)
                    if self.highs:
                        with self.assertRaises(ApiError):
                            result.value(x)
                path.write_text("preserve me", encoding="utf8")
                with self.assertRaises(ApiError):
                    Model.read(path, self.lib)
                self.assertEqual(path.read_text(), "preserve me")
            bad = Path(directory) / "model.unsupported"
            bad.write_text("unchanged", encoding="utf8")
            with self.assertRaises(ApiError):
                model.write(bad)
            self.assertEqual(bad.read_text(), "unchanged")

    def test_global_builders_auto_routing_and_sessions(self):
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, 0, 2)
            y = model.add_variable(VariableType.INTEGER, 0, 2)
            distinct = model.add_all_different([x, y], name="distinct")
            self.assertIsInstance(distinct, GlobalConstraint)
            table = model.add_table([x, y], [(0, 1), (2, 0)])
            # Index/result/array aliases retain their exact meaning: x=2 is
            # outside this element's indices, so the only solution is (0,1).
            model.add_element(x, [x, y], x)
            model.add_cumulative([x, x], [0, 1], [100, 0], capacity=0)
            successors = [model.add_variable(VariableType.INTEGER, v, v) for v in [2, 3, 1]]
            model.add_circuit(successors, index_base=1)
            model.set_objective({x: 1, y: 2})
            model.set_name(distinct, "renamed")
            for backend in (Backend.AUTO, Backend.NATIVE):
                with model.solve(Options(backend=backend)) as result:
                    self.assertEqual(result.termination, Termination.OPTIMAL if self.native else Termination.UNSUPPORTED)
                    if self.native:
                        self.assertEqual(result.objective, 2)
                        self.assertEqual((result.value(x), result.value(y)), (0, 1))
            with model.solve(Options(backend=Backend.HIGHS)) as result:
                self.assertEqual(result.termination, Termination.UNSUPPORTED)
            # The persistent numerical session keeps its declared route.
            with Session(self.lib) as session:
                with session.solve(model) as result:
                    self.assertEqual(result.termination, Termination.UNSUPPORTED)
                with session.solve(model, Options(backend=Backend.NATIVE)) as result:
                    # A persistent numerical session never changes backend;
                    # explicit Native is available on Model.solve instead.
                    self.assertEqual(result.termination, Termination.UNSUPPORTED)
            with self.assertRaises(ApiError):
                model.remove(x)
            with self.assertRaises(TypeError):
                model.set_bounds(distinct, 0, 1)
            with tempfile.TemporaryDirectory() as directory:
                with self.assertRaises(ApiError):
                    model.write(Path(directory) / "globals.lp")
            model.remove(table)
            with self.assertRaises(ApiError):
                model.remove(table)
            with self.assertRaises(ApiError):
                model.set_name(table, "deleted")

    def test_global_aliases_bases_and_zero_cases(self):
        def check(model, feasible):
            with model.solve(Options(backend=Backend.NATIVE)) as result:
                expected = Termination.OPTIMAL if feasible else Termination.INFEASIBLE
                self.assertEqual(result.termination, expected if self.native else Termination.UNSUPPORTED)
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, 0, 2)
            model.add_all_different([x, x])
            check(model, False)
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, 0, 2)
            model.add_table([x, x], [(0, 1)])
            check(model, False)
        with Model(self.lib) as model:
            index = model.add_variable(VariableType.INTEGER, -2, -2)
            model.add_element(index, [index], index, index_base=-2)
            check(model, True)
        for tuples, feasible in [([], False), ([()], True)]:
            with Model(self.lib) as model:
                model.add_table([], tuples)
                model.add_all_different([])
                model.add_cumulative([], [], [], 0)
                check(model, feasible)
        with Model(self.lib) as model:
            x = model.add_variable(VariableType.INTEGER, 1, 2)
            model.add_circuit([x, x], index_base=1)
            check(model, False)

    def test_global_errors_are_atomic(self):
        with Model(self.lib) as model, Model(self.lib) as foreign:
            x = model.add_variable(VariableType.INTEGER, 0, 2)
            y = foreign.add_variable(VariableType.INTEGER, 0, 2)
            continuous = model.add_variable(upper=2)
            dead = model.add_variable(VariableType.INTEGER, 0, 2)
            model.remove(dead)
            before = model.identity
            failures = [
                (lambda: model.add_all_different([y]), ApiError),
                (lambda: model.add_all_different([dead]), ApiError),
                (lambda: model.add_all_different([continuous]), ApiError),
                (lambda: model.add_table([x], [(1, 2)]), ValueError),
                (lambda: model.add_table([x], [(2**63,)]), OverflowError),
                (lambda: model.add_table([x], [(2**53+1,)]), ApiError),
                (lambda: model.add_table([x], [(1.0,)]), TypeError),
                (lambda: model.add_element(x, [x], x, index_base=2**63), OverflowError),
                (lambda: model.add_element(x, [x], x, index_base=True), TypeError),
                (lambda: model.add_cumulative([x], [], [1], 1), ApiError),
                (lambda: model.add_cumulative([x], [-1], [1], 1), ApiError),
                (lambda: model.add_cumulative([x], [1], [1], -1), ApiError),
                (lambda: model.add_circuit([]), ApiError),
                (lambda: model.add_circuit([x], index_base=2**53+1), ApiError),
            ]
            for action, error in failures:
                with self.assertRaises(error):
                    action()
                self.assertEqual(model.identity, before)
            global_id = model.add_all_different([x])
            with self.assertRaises(ApiError):
                foreign.remove(global_id)
            model.remove(global_id)
            model.remove(x)

    def test_indicator_guards_removal_and_boolean_semantics(self):
        for active in (False, True):
            with Model(self.lib) as model:
                b = model.add_variable(VariableType.BINARY, int(active), int(active))
                x = model.add_variable(VariableType.INTEGER, 0, 4)
                model.set_objective({x: 1})
                before = model.identity
                for action, error in [
                    (lambda: model.add_indicator(b, 1, {x: 1}, lower=3), TypeError),
                    (lambda: model.add_indicator(x, active, {x: 1}, lower=3), ApiError),
                    (lambda: model.add_indicator(b, active, {x: math.inf}, lower=3), ApiError),
                    (lambda: model.add_boolean_and(b, [x]), ApiError),
                ]:
                    with self.assertRaises(error):
                        action()
                    self.assertEqual(model.identity, before)
                indicator = model.add_indicator(b, active, {x: 1}, lower=3)
                self.assertIsInstance(indicator, Indicator)
                self.assertIsNotNone(indicator.inactive_gate)
                with self.assertRaises(ApiError):
                    model.set_bounds(x, 0, 5)
                with model.solve(Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, 3, [(x, 3)])
                with tempfile.TemporaryDirectory() as directory:
                    with self.assertRaises(ApiError):
                        model.write(Path(directory) / "indicator.lp")
                model.remove(indicator)
                with self.assertRaises(ApiError):
                    model.remove(indicator)
                model.remove(indicator.inactive_gate)
                with model.solve(Options(backend=Backend.HIGHS)) as result:
                    self.assert_solution(result, 0, [(x, 0)])
                tautology = model.add_indicator(b, active)
                self.assertIsNone(tautology.inactive_gate)
                model.remove(tautology)
        for left in (0, 1):
            for right in (0, 1):
                with Model(self.lib) as model:
                    a = model.add_variable(VariableType.BINARY, left, left)
                    b = model.add_variable(VariableType.BINARY, right, right)
                    conjunction = model.add_variable(VariableType.BINARY)
                    disjunction = model.add_variable(VariableType.BINARY)
                    true = model.add_variable(VariableType.BINARY)
                    false = model.add_variable(VariableType.BINARY)
                    self.assertIsNone(model.add_boolean_and(conjunction, [a, b, a]))
                    model.add_boolean_or(disjunction, [a, b, b])
                    model.add_boolean_and(true)
                    model.add_boolean_or(false)
                    model.add_boolean_and(a, [a, a])  # aliased output/input
                    with model.solve(Options(backend=Backend.HIGHS)) as result:
                        self.assert_solution(result, 0, [(conjunction, left & right),
                                                         (disjunction, left | right),
                                                         (true, 1), (false, 0)])

    def test_thread_local_errors_and_independent_solves(self):
        barrier = threading.Barrier(2)
        def error_worker(which):
            if which == 0:
                code = self.lib._functions["model_destroy"](0)
            else:
                code = self.lib._functions["model_create"](None)
            barrier.wait(timeout=5)
            text = self.lib._dll.gecode_opt_v1_last_error().decode()
            return code, text
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
            a, b = list(executor.map(error_worker, range(2)))
        self.assertEqual(a[0], 2)
        self.assertIn("handle", a[1])
        self.assertEqual(b[0], 1)
        self.assertIn("pointer", b[1])
        def solve_worker(lower):
            with Model(self.lib) as model, Session(self.lib) as session:
                x = model.add_variable(VariableType.INTEGER, lower, 20)
                model.set_objective({x: 1})
                with session.solve(model, Options(backend=Backend.HIGHS)) as result:
                    return result.termination, result.objective
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
            results = list(executor.map(solve_worker, [2, 7]))
        expected = [(Termination.OPTIMAL, 2), (Termination.OPTIMAL, 7)] if self.highs else [(Termination.UNSUPPORTED, None)] * 2
        self.assertEqual(results, expected)


if __name__ == "__main__":
    unittest.main()
