"""Distinct finite-box QP binding conformance, with analytic original oracles."""
import ctypes as C
from dataclasses import FrozenInstanceError
import math
import unittest

from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Library,
    Model, Options, QuadraticModel, QuadraticOptions, QuadraticResult, Result,
    Session, Termination, WeightedSquare, load_library)


class Quadratic(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib=load_library()
        cls.available=cls.lib.quadratic_capabilities()["available"]

    def assert_completed(self, result):
        self.assertIsInstance(result,QuadraticResult)
        self.assertNotIsInstance(result,Result)
        self.assertEqual(result.termination,Termination.OPTIMAL if self.available else Termination.UNSUPPORTED)
        self.assertEqual(result.has_solution,self.available)
        self.assertEqual(result.info["guarantee"],Guarantee.NUMERICAL)
        checks=result.checks
        if self.available:
            self.assertTrue(checks.primal_valid and checks.objective_valid and checks.kkt_available)
            self.assertTrue(checks.kkt_valid and checks.bound_valid)
            self.assertLessEqual(checks.gap_upper_bound,1e-6)
            self.assertIsNotNone(checks.max_stationarity)
            self.assertIsNotNone(checks.max_complementarity)
        else:
            self.assertFalse(checks.primal_valid or checks.objective_valid or checks.kkt_available)
            self.assertFalse(checks.kkt_valid or checks.bound_valid)
            for field in ("original_objective","normalized_lower_bound","gap_upper_bound","max_stationarity","max_complementarity"):
                self.assertIsNone(getattr(checks,field))
            for field in ("objective","best_bound","absolute_gap","relative_gap","vendor_objective","vendor_dual_estimate",
                          "original_gradient","square_residuals"):
                self.assertIsNone(getattr(result,field))

    def test_both_senses_original_objective_gradient_bounds_and_offsets(self):
        # Independent closed form: x*=2-l/(2*w), f=c+l*x+w*(x-2)^2.
        for maximize in (False,True):
            for weight in (0.5,1,2):
                for linear in (-1,0,1):
                    with self.subTest(maximize=maximize,weight=weight,linear=linear),QuadraticModel(self.lib) as model:
                        x=model.add_continuous(-4,4,name="decision λ")
                        square=WeightedSquare({x:1},-2,weight,"residual π")
                        sign=-1 if maximize else 1
                        setter=model.maximize_concave_squares if maximize else model.minimize_squares
                        setter([square],{x:sign*linear},offset=-13)
                        with model.solve() as result:
                            self.assert_completed(result)
                            if not self.available:
                                continue
                            optimum=2-linear/(2*weight)
                            objective=-13+sign*(linear*optimum+weight*(optimum-2)**2)
                            self.assertAlmostEqual(result.value(x),optimum,places=6)
                            self.assertAlmostEqual(result.objective,objective,places=6)
                            self.assertAlmostEqual(result.checks.original_objective,objective,places=6)
                            self.assertAlmostEqual(result.square_residuals[0],optimum-2,places=6)
                            self.assertAlmostEqual(result.original_gradient[x.slot],0,places=6)
                            self.assertLessEqual(sign*result.best_bound,sign*objective+1e-12)
                            self.assertLessEqual(result.checks.normalized_lower_bound,sign*objective+1e-12)
                            self.assertEqual(result.info["regularization"],0)
                            self.assertIsInstance(result.message,str)
                            self.assertTrue(result.backend_version)
                            with self.assertRaises(FrozenInstanceError): result.checks.bound_valid=False

    def test_rows_mutations_tombstones_and_historical_ownership(self):
        with QuadraticModel(self.lib) as model:
            dead=model.add_continuous(0,1);model.remove(dead)
            x=model.add_continuous(-4,4)
            square=WeightedSquare({x:1},-2,2)
            model.minimize_squares([square],{x:4},offset=-7)
            identity=model.identity
            retained=model.solve()
            self.assert_completed(retained)
            self.assertEqual((retained.info["model_id"],retained.info["revision"]),identity)
            square.offset=99;square.terms={};square.weight=99 # Inputs were copied.
            row=model.add_row({x:1},lower=2)
            model.set_coefficient(row,x,2);model.set_bounds(row,4,math.inf)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:
                    self.assertAlmostEqual(result.value(x),2)
                    self.assertAlmostEqual(result.objective,1)
            model.remove(row);model.set_bounds(x,3,4)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:self.assertAlmostEqual(result.value(x),3)
            model.minimize_squares();model.remove(x)
        with retained:
            self.assertEqual(retained.values[dead.slot],{"active":False,"present":False,"value":None})
            if self.available:
                self.assertAlmostEqual(retained.value(x),1)
                self.assertAlmostEqual(retained.objective,-1)
                self.assertEqual(retained.original_gradient[dead.slot],0)
                with self.assertRaises(ApiError):retained.value(dead)
            else:
                with self.assertRaises(ApiError) as error:retained.value(x)
                self.assertEqual(error.exception.code,7)
        with self.assertRaises(RuntimeError):retained.checks

    def test_empty_and_constant_square_and_huge_offset(self):
        with QuadraticModel(self.lib) as model:
            model.minimize_squares([WeightedSquare((),3,2)],offset=-7)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:
                    self.assertEqual(result.values,())
                    self.assertEqual(result.original_gradient,())
                    self.assertEqual(result.square_residuals,(3,))
                    self.assertEqual(result.objective,11)
        with QuadraticModel(self.lib) as model:
            x=model.add_continuous(0,2)
            model.minimize_squares([WeightedSquare({x:1},-1)],linear={x:2},offset=1e16)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:
                    self.assertLessEqual(result.checks.gap_upper_bound,1e-6)
                    self.assertGreater(result.absolute_gap,result.checks.gap_upper_bound)
                    self.assertEqual(result.objective,1e16)

    def test_coupled_squares_and_equality_original_gradient(self):
        with QuadraticModel(self.lib) as model:
            x=model.add_continuous(-2,2);y=model.add_continuous(-2,2)
            # Under x+y=1, minimize 2(x-y)^2 + (x+y-2)^2.
            model.add_row({x:1,y:1},lower=1,upper=1)
            model.minimize_squares([WeightedSquare([(x,2),(x,-1),(y,-1)],weight=2),
                                   WeightedSquare({x:1,y:1},offset=-2)],offset=7)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:
                    self.assertAlmostEqual(result.value(x),0.5)
                    self.assertAlmostEqual(result.value(y),0.5)
                    self.assertAlmostEqual(result.objective,8)
                    self.assertEqual(len(result.square_residuals),2)
                    self.assertAlmostEqual(result.square_residuals[0],0)
                    self.assertAlmostEqual(result.square_residuals[1],-1)
                    # A constrained optimum has a nonzero original gradient.
                    self.assertAlmostEqual(result.original_gradient[x.slot],-2)
                    self.assertAlmostEqual(result.original_gradient[y.slot],-2)

    def test_rejected_mutations_preserve_objective_and_revision(self):
        with QuadraticModel(self.lib) as model,QuadraticModel(self.lib) as foreign:
            x=model.add_continuous(-4,4);y=foreign.add_continuous(-4,4)
            model.minimize_squares([WeightedSquare({x:1},-2)])
            revision=model.identity
            bad=[WeightedSquare({y:1}),WeightedSquare({x:math.nan}),WeightedSquare({x:1},weight=-1),
                 WeightedSquare({x:1},weight=0),WeightedSquare({x:1},offset=math.inf)]
            for square in bad:
                with self.assertRaises(ApiError):model.minimize_squares([square])
                self.assertEqual(model.identity,revision)
            with self.assertRaises(ApiError):model.minimize_squares([WeightedSquare({x:1})],{y:1})
            with self.assertRaises(ApiError):model.remove(x)
            with self.assertRaises(ApiError):model.set_bounds(x,0,math.inf)
            with self.assertRaises(ApiError):model.add_continuous(math.nan,1)
            with self.assertRaises(ValueError):model.minimize_squares([WeightedSquare({x:1},name="bad\0name")])
            with self.assertRaises(TypeError):model.minimize_squares([object()])
            self.assertEqual(model.identity,revision)
            with model.solve() as result:
                self.assert_completed(result)
                if self.available:
                    self.assertAlmostEqual(result.value(x),2)
                    with self.assertRaises(ApiError):result.value(y)
            model.minimize_squares();model.remove(x)
            with self.assertRaises(ApiError):model.add_row({x:1})

    def test_distinct_classes_and_library_ownership(self):
        with QuadraticModel(self.lib) as model,Model(self.lib) as linear,Session(self.lib) as session:
            self.assertNotIsInstance(model,Model)
            with self.assertRaises(TypeError):session.solve(model)
            with self.assertRaises(TypeError):model.solve(Options())
            x=model.add_continuous(0,1)
            with self.assertRaises(ApiError):linear.add_row({x:1})
            second=Library(self.lib.path)
            with QuadraticModel(second) as foreign:
                y=foreign.add_continuous(0,1)
                with self.assertRaises(ValueError):model.add_row({y:1})
                with self.assertRaises(ValueError):model.minimize_squares([WeightedSquare({y:1})])
            # Even direct C misuse cannot erase the quadratic objective.
            out=C.c_uint64()
            with self.assertRaises(ApiError) as error:self.lib.call("solve",model._open(),None,C.byref(out))
            self.assertEqual(error.exception.code,2)
            self.assertEqual(out.value,0)

    def test_limits_cancellation_unsupported_and_bad_option_fields(self):
        with QuadraticModel(self.lib) as model,Cancellation(self.lib) as cancel:
            x=model.add_continuous(-4,4);model.minimize_squares([WeightedSquare({x:1},-2)])
            unsupported=[Options(backend=Backend.NATIVE),Options(guarantee=Guarantee.EXACT),
                         Options(guarantee=Guarantee.CERTIFIED),Options(threads=2),Options(random_seed=1),Options(node_limit=1),
                         Options(primal_start={x:2})]
            for solve in unsupported:
                with model.solve(QuadraticOptions(solve)) as result:
                    self.assertEqual(result.termination,Termination.UNSUPPORTED)
                    self.assertFalse(result.has_solution)
                    self.assertIsNone(result.checks.gap_upper_bound)
            for solve in (Options(time_limit_seconds=0),Options(cancellation=cancel)):
                if solve.cancellation:cancel.cancel()
                with model.solve(QuadraticOptions(solve)) as result:
                    self.assertEqual(result.termination,Termination.CANCELLED if solve.cancellation else Termination.TIME_LIMIT)
                    self.assertFalse(result.has_solution)
                    self.assertIsNone(result.objective)
                    self.assertFalse(result.checks.kkt_available)
                    self.assertIsNone(result.checks.max_stationarity)
                    self.assertIsNone(result.checks.max_complementarity)
            for options in (QuadraticOptions(optimality_tolerance=math.nan),QuadraticOptions(stationarity_tolerance=-1),
                            QuadraticOptions(complementarity_tolerance=math.inf)):
                with self.assertRaises(ApiError):model.solve(options)
            for options in (QuadraticOptions(iteration_limit=-1),QuadraticOptions(max_auxiliary_variables=2**64)):
                with self.assertRaises(OverflowError):model.solve(options)
            with self.assertRaises(TypeError):model.solve(QuadraticOptions(iteration_limit=True))
            with self.assertRaises(TypeError):model.solve(QuadraticOptions(solve=object()))
            with model.solve(QuadraticOptions(max_auxiliary_variables=0)) as result:
                self.assertEqual(result.termination,Termination.UNSUPPORTED)
                self.assertFalse(result.has_solution)

    def test_iteration_limit_keeps_candidate_separate_from_optimality(self):
        with QuadraticModel(self.lib) as model:
            x=model.add_continuous(-4,4)
            model.minimize_squares([WeightedSquare({x:1},-2)])
            with model.solve(QuadraticOptions(iteration_limit=0)) as result:
                self.assertEqual(result.termination,Termination.ITERATION_LIMIT if self.available else Termination.UNSUPPORTED)
                self.assertEqual(result.info["qp_iterations"],0)
                if result.has_solution:
                    self.assertTrue(result.checks.primal_valid and result.checks.objective_valid)
                    self.assertAlmostEqual(result.objective,(result.value(x)-2)**2)
                    self.assertFalse(result.checks.kkt_valid)
                    self.assertGreater(result.checks.gap_upper_bound,1e-6)

    def test_infeasible_rows_have_no_successful_original_checks(self):
        with QuadraticModel(self.lib) as model:
            x=model.add_continuous(0,1)
            model.minimize_squares([WeightedSquare({x:1})])
            model.add_row({x:1},lower=2)
            with model.solve() as result:
                self.assertEqual(result.termination,Termination.INFEASIBLE if self.available else Termination.UNSUPPORTED)
                self.assertFalse(result.has_solution)
                self.assertFalse(result.checks.primal_valid or result.checks.objective_valid or result.checks.kkt_valid)
                self.assertIsNone(result.objective)
                self.assertIsNone(result.checks.gap_upper_bound)


if __name__=="__main__":
    unittest.main()
