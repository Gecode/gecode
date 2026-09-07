"""Native complete starts through the existing owning C/Python boundary."""
import unittest

from gecode_optimize import (Backend, Cancellation, Guarantee, Model, Options,
                             Termination, VariableType, load_library)


class NativeStarts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib = load_library()
        cls.native = cls.lib.capabilities(Backend.NATIVE)["available"]

    def options(self, start, **kwargs):
        return Options(backend=Backend.NATIVE, guarantee=Guarantee.EXACT,
                       primal_start=start, **kwargs)

    def test_both_senses_activation_values_and_owning_history(self):
        for active in (False, True):
            for maximize in (False, True):
                with self.subTest(active=active, maximize=maximize), Model(self.lib) as model:
                    b = model.add_variable(VariableType.BINARY)
                    x = model.add_variable(VariableType.INTEGER, 0, 4)
                    gate = model.add_indicator(b, active, {x: 1}, lower=3).inactive_gate
                    model.set_objective({x: 1, b: -10}, maximize=maximize, offset=3)
                    feasible = [(q, v) for q in (0, 1) for v in range(5)
                                if q != int(active) or v >= 3]
                    key = lambda pair: pair[1] - 10*pair[0] + 3
                    best = (max if maximize else min)(feasible, key=key)
                    poor = (min if maximize else max)(feasible, key=key)
                    start = {b: poor[0], x: poor[1]}  # omit live derived gate
                    result = model.solve(self.options(start))
                    identity = model.identity
                    start.clear()
                    model.set_objective({}, offset=91)
                with result:
                    self.assertEqual(result.termination, Termination.OPTIMAL if self.native else Termination.UNSUPPORTED)
                    self.assertEqual(result.info["start_submitted"], self.native)
                    self.assertEqual((result.info["model_id"], result.info["revision"]), identity)
                    if self.native:
                        self.assertTrue(result.info["solution_validated"])
                        self.assertEqual(result.objective, key(best))
                        self.assertEqual(result.best_bound, key(best))
                        self.assertEqual((result.value(b), result.value(x)), best)
                        self.assertEqual(result.value(gate), int(best[0] != int(active)))

    def test_exact_input_errors_partial_starts_and_zero_budget(self):
        with Model(self.lib) as model, Cancellation(self.lib) as token:
            b = model.add_variable(VariableType.BINARY)
            x = model.add_variable(VariableType.INTEGER, 0, 4)
            gate = model.add_indicator(b, True, {x: 1}, lower=3).inactive_gate
            # Each rejected complete point violates a different original rule.
            cases = [({b: 0, x: 4, gate: 0}, Termination.INVALID_MODEL),
                     ({b: 1-1e-12, x: 3}, Termination.INVALID_MODEL),
                     ({b: 1, x: 2}, Termination.INVALID_MODEL),
                     ({b: 0}, Termination.UNSUPPORTED)]
            for start, expected in cases:
                with self.subTest(start=start), model.solve(self.options(start)) as result:
                    self.assertEqual(result.termination, expected if self.native else Termination.UNSUPPORTED)
                    self.assertFalse(result.has_solution)
                    self.assertFalse(result.info["start_submitted"])
            if self.native:
                token.cancel()
                for changes, expected in [({"time_limit_seconds": 0}, Termination.TIME_LIMIT),
                                          ({"node_limit": 0}, Termination.NODE_LIMIT),
                                          ({"cancellation": token}, Termination.CANCELLED)]:
                    with model.solve(self.options({b: 0, x: 4}, **changes)) as result:
                        self.assertEqual(result.termination, expected)
                        self.assertFalse(result.has_solution)
                        self.assertFalse(result.info["start_submitted"])

    def test_removed_gate_requires_explicit_value_and_is_not_rederived(self):
        with Model(self.lib) as model:
            b = model.add_variable(VariableType.BINARY)
            x = model.add_variable(VariableType.INTEGER, 0, 4)
            indicator = model.add_indicator(b, True, {x: 1}, lower=3)
            gate = indicator.inactive_gate
            model.remove(indicator)
            model.set_objective({}, offset=11)
            with model.solve(self.options({b: 0, x: 4})) as incomplete:
                self.assertEqual(incomplete.termination, Termination.UNSUPPORTED)
                self.assertFalse(incomplete.has_solution)
            # This gate=0 contradicts its old, now removed equation at b=0.
            with model.solve(self.options({b: 0, x: 4, gate: 0})) as complete:
                self.assertEqual(complete.termination, Termination.OPTIMAL if self.native else Termination.UNSUPPORTED)
                if self.native:
                    self.assertTrue(complete.info["start_submitted"])
                    self.assertEqual(complete.objective, 11)
                    self.assertEqual(complete.value(gate), 0)
                    self.assertEqual(complete.value(x), 4)
