"""C ABI-backed pool/repair conformance; no timing/performance claims."""
import concurrent.futures
from dataclasses import FrozenInstanceError
import math
import threading
import time
import unittest

from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Model, Options,
    PoolCompletion, PoolOptions, RelaxationSelection, RelaxationSide, RepairOptions,
    Termination, VariableType, load_library)


class Workflows(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib=load_library()
        cls.highs=cls.lib.capabilities(Backend.HIGHS)["available"]
        cls.native=cls.lib.capabilities(Backend.NATIVE)["available"]

    def test_ranked_projection_recourse_and_independent_result_lifetime(self):
        with Model(self.lib) as model:
            x=model.add_variable(VariableType.INTEGER,-1,1)
            y=model.add_variable(upper=4)
            dead=model.add_variable();model.remove(dead)
            model.add_row({x:1,y:1},lower=2)
            model.set_objective({x:2,y:1},offset=5)
            identity=model.identity
            with model.solve_pool(PoolOptions(Options(backend=Backend.HIGHS),5,[x])) as pool:
                self.assertEqual((pool.info["model_id"],pool.info["revision"]),identity)
                self.assertEqual(pool.projection,(x,))
                if not self.highs:
                    self.assertEqual(pool.termination,Termination.UNSUPPORTED)
                    self.assertEqual(pool.info["entry_count"],0)
                    self.assertFalse(pool.exhausted)
                    with self.assertRaises(ApiError): pool.entry_result(0)
                    return
                self.assertEqual(pool.completion,PoolCompletion.EXHAUSTED)
                self.assertEqual(pool.ranked_prefix,3)
                self.assertEqual(pool.info["attempt_count"],4)
                for i in range(3):
                    record=pool.entry(i)
                    self.assertEqual(record.projection_values,(i-1,))
                    self.assertTrue(record.rank_established)
                    with self.assertRaises(FrozenInstanceError): record.rank_established=False
                    with pool.entry_result(i) as result:
                        self.assertEqual(result.termination,Termination.UNKNOWN)
                        self.assertIsNone(result.best_bound)
                        self.assertAlmostEqual(result.value(y),3-i)
                        self.assertAlmostEqual(result.objective,6+i)
                        self.assertEqual(result.values[dead.slot],{"active":False,"present":False,"value":None})
                self.assertEqual(pool.attempt(3).termination,Termination.INFEASIBLE)
                self.assertFalse(pool.attempt(3).candidate_accepted)
                retained=pool.entry_result(0)
            model.set_bounds(x,1,1)
        with retained:
            self.assertEqual(retained.info["model_id"],identity[0])
            self.assertAlmostEqual(retained.value(x),-1)
            self.assertAlmostEqual(retained.objective,6)
        with self.assertRaises(RuntimeError): pool.entry(0)

    def test_pool_requested_limit_maximize_ties_and_explicit_native(self):
        for native in (False,True):
            available=self.native if native else self.highs
            options=Options(backend=Backend.NATIVE if native else Backend.HIGHS,
                            guarantee=Guarantee.EXACT if native else Guarantee.NUMERICAL)
            with self.subTest(native=native),Model(self.lib) as model:
                x=model.add_variable(VariableType.INTEGER,-2,0)
                y=model.add_variable(VariableType.BINARY)
                model.set_objective({x:2},maximize=True,offset=-3)
                with model.solve_pool(PoolOptions(options,2,[x,y])) as pool:
                    self.assertEqual(pool.termination,Termination.SOLUTION_LIMIT if available else Termination.UNSUPPORTED)
                    self.assertFalse(pool.exhausted)
                    if available:
                        self.assertEqual(pool.ranked_prefix,2)
                        self.assertNotEqual(pool.entry(0).projection_values,pool.entry(1).projection_values)
                        for i in (0,1):
                            with pool.entry_result(i) as result: self.assertEqual(result.objective,-3)

    def test_empty_models_zero_items_and_projection_presence(self):
        with Model(self.lib) as model:
            model.set_objective(offset=-7)
            with model.solve_pool(PoolOptions(Options(backend=Backend.HIGHS),5,[])) as pool:
                self.assertEqual(pool.projection,())
                if self.highs:
                    self.assertTrue(pool.exhausted)
                    self.assertEqual(pool.info["entry_count"],1)
                    self.assertEqual(pool.entry(0).projection_values,())
                    with pool.entry_result(0) as result:
                        self.assertEqual(result.values,[])
                        self.assertEqual(result.objective,-7)
                else: self.assertEqual(pool.termination,Termination.UNSUPPORTED)
            with model.relax_feasibility(RepairOptions(Options(backend=Backend.HIGHS),(),True)) as repair:
                self.assertEqual(repair.variable_map,())
                self.assertEqual(repair.original_values,())
                self.assertEqual(repair.info["item_count"],0)
                if self.highs:
                    self.assertTrue(repair.has_repair)
                    self.assertTrue(repair.original_validation.valid)
                    self.assertEqual(repair.weighted_violation,0)
                    self.assertEqual(repair.original_objective,-7)
                else: self.assertEqual(repair.termination,Termination.UNSUPPORTED)
            x=model.add_variable(VariableType.INTEGER,0,1)
            with model.solve_pool(PoolOptions(projection=[])) as pool:
                self.assertEqual(pool.termination,Termination.UNSUPPORTED)
            with model.solve_pool(PoolOptions(max_solutions=0)) as pool:
                self.assertEqual(pool.termination,Termination.INVALID_MODEL)

    def test_repair_original_private_mapping_stages_and_history(self):
        with Model(self.lib) as model:
            dead=model.add_variable();model.remove(dead)
            x=model.add_variable(VariableType.INTEGER,0,2)
            y=model.add_variable(upper=1)
            demand=model.add_row({x:1,y:1},lower=4,name="demand λ")
            model.set_objective({x:2,y:1},offset=-9)
            identity=model.identity
            with model.relax_feasibility(RepairOptions(Options(backend=Backend.HIGHS),
                    [RelaxationSelection(demand,RelaxationSide.LOWER,2)],True)) as repair:
                self.assertEqual((repair.info["source_model_id"],repair.info["source_revision"]),identity)
                mapping=repair.variable_map
                self.assertFalse(mapping[dead.slot].active)
                self.assertEqual(mapping[x.slot].source,x)
                self.assertNotEqual(mapping[x.slot].private.model_id,x.model_id)
                item=repair.item(0)
                self.assertEqual(item.source,demand)
                self.assertEqual(item.name,"demand λ")
                self.assertEqual(item.slack.model_id,mapping[x.slot].private.model_id)
                if not self.highs:
                    self.assertEqual(repair.termination,Termination.UNSUPPORTED)
                    self.assertFalse(repair.has_repair)
                    self.assertIsNone(item.activity)
                    self.assertFalse(repair.original_values[x.slot]["present"])
                    with self.assertRaises(ApiError) as error: repair.final_result()
                    self.assertEqual(error.exception.code,7)
                    return
                self.assertEqual(repair.termination,Termination.OPTIMAL)
                self.assertTrue(repair.has_repair)
                self.assertTrue(repair.info["minimum_violation_established"])
                self.assertTrue(repair.info["original_objective_optimized"])
                self.assertAlmostEqual(repair.minimum_weighted_violation,2)
                self.assertAlmostEqual(repair.weighted_violation,2)
                self.assertAlmostEqual(repair.original_objective,-4)
                self.assertAlmostEqual(item.violation,1)
                self.assertAlmostEqual(item.activity,3)
                self.assertAlmostEqual(item.weighted_violation,2)
                self.assertFalse(repair.original_validation.valid)
                self.assertTrue(repair.original_validation.model_valid)
                self.assertAlmostEqual(repair.original_validation.max_row_violation,1)
                self.assertAlmostEqual(repair.original_value(x),2)
                self.assertEqual(repair.original_values[dead.slot],{"active":False,"present":False,"value":None})
                self.assertEqual(repair.info["completed_stages"],2)
                self.assertTrue(repair.stage(0).completed)
                self.assertAlmostEqual(repair.stage(0).retention_bound,2)
                self.assertEqual(len(repair.objective_values),2)
                self.assertIsNotNone(repair.violation_lock)
                with self.assertRaises(ApiError): repair.original_value(mapping[x.slot].private)
                with self.assertRaises(ApiError): repair.original_value(dead)
                retained=repair.final_result();stage=repair.stage_result(0)
            model.set_bounds(x,0,1)
        with retained,stage:
            self.assertEqual(retained.termination,Termination.UNKNOWN)
            self.assertIsNone(retained.best_bound)
            self.assertAlmostEqual(retained.value(mapping[x.slot].private),2)
            self.assertAlmostEqual(retained.value(item.slack),1)
            with self.assertRaises(ApiError): retained.value(x)
            self.assertEqual(stage.info["model_id"],mapping[x.slot].private.model_id)
        with self.assertRaises(RuntimeError): repair.original_value(x)

    def test_repair_binary_bounds_remain_intrinsic_and_semis_are_explicit(self):
        with Model(self.lib) as model:
            x=model.add_variable(VariableType.BINARY,.25,.75)
            model.set_objective({x:1})
            selections=[RelaxationSelection(x,side) for side in RelaxationSide]
            with model.relax_feasibility(RepairOptions(Options(backend=Backend.HIGHS),selections,True)) as repair:
                if self.highs:
                    self.assertTrue(repair.has_repair)
                    self.assertAlmostEqual(repair.minimum_weighted_violation,.25)
                    self.assertAlmostEqual(repair.original_value(x),0)
                    self.assertAlmostEqual(repair.original_validation.max_bound_violation,.25)
                else: self.assertEqual(repair.termination,Termination.UNSUPPORTED)
        with Model(self.lib) as model:
            semi=model.add_variable(VariableType.SEMI_INTEGER,2,4)
            with model.relax_feasibility(RepairOptions(selections=[RelaxationSelection(semi)])) as repair:
                self.assertEqual(repair.termination,Termination.UNSUPPORTED)
                self.assertFalse(repair.has_repair)

    def test_invalid_inputs_status_and_ownership(self):
        with Model(self.lib) as model,Model(self.lib) as foreign:
            x=model.add_variable(VariableType.INTEGER,0,1)
            other=foreign.add_variable(VariableType.INTEGER,0,1)
            dead=model.add_variable(VariableType.INTEGER,0,1);model.remove(dead)
            row=model.add_row({x:1},lower=1)
            identity=model.identity
            for projection in ([other],[dead],[x,x]):
                with model.solve_pool(PoolOptions(projection=projection)) as pool:
                    self.assertEqual(pool.termination,Termination.INVALID_MODEL)
            with self.assertRaises(TypeError): model.solve_pool(PoolOptions(projection=[row]))
            with self.assertRaises(OverflowError): model.solve_pool(PoolOptions(max_solutions=2**64))
            for source in (other,dead):
                with model.relax_feasibility(RepairOptions(selections=[RelaxationSelection(source)])) as repair:
                    self.assertEqual(repair.termination,Termination.INVALID_MODEL)
            for penalty in (0,-1,math.nan,math.inf):
                with self.assertRaises(ApiError):
                    model.relax_feasibility(RepairOptions(selections=[RelaxationSelection(row,penalty=penalty)]))
            with self.assertRaises(ValueError): model.relax_feasibility(RepairOptions(selections=[RelaxationSelection(row,9)]))
            with self.assertRaises(TypeError): model.relax_feasibility(RepairOptions(optimize_original_objective=1))
            with self.assertRaises(TypeError): model.relax_feasibility(Options())
            with self.assertRaises(TypeError): model.solve_pool(Options())
            self.assertEqual(model.identity,identity)

    def test_repair_failed_refinement_preserves_only_established_evidence(self):
        with Model(self.lib) as model:
            x=model.add_variable()
            model.set_objective({x:1},maximize=True)
            with model.relax_feasibility(RepairOptions(Options(backend=Backend.HIGHS),(),True)) as repair:
                if not self.highs:
                    self.assertEqual(repair.termination,Termination.UNSUPPORTED)
                else:
                    self.assertEqual(repair.termination,Termination.UNBOUNDED)
                    self.assertTrue(repair.info["minimum_violation_established"])
                    self.assertFalse(repair.info["original_objective_optimized"])
                    self.assertEqual(repair.info["completed_stages"],1)
                    self.assertTrue(repair.has_repair)
                    self.assertEqual(repair.minimum_weighted_violation,0)
                    self.assertTrue(repair.stage(0).completed)
                    self.assertFalse(repair.stage(1).completed)
                    with repair.stage_result(1) as stage:
                        self.assertEqual(stage.termination,Termination.UNBOUNDED)
                    with repair.final_result() as retained:
                        self.assertEqual(retained.termination,Termination.UNKNOWN)
                        self.assertIsNone(retained.best_bound)
        with Model(self.lib) as model:
            x=model.add_variable(VariableType.INTEGER,.25,.75)
            with model.relax_feasibility(RepairOptions(Options(backend=Backend.HIGHS))) as repair:
                self.assertEqual(repair.termination,Termination.INFEASIBLE if self.highs else Termination.UNSUPPORTED)
                self.assertFalse(repair.has_repair)
                self.assertFalse(repair.info["minimum_violation_established"])
                self.assertIsNone(repair.minimum_weighted_violation)

    def test_cancellation_limits_missing_capabilities_and_no_fallback(self):
        with Model(self.lib) as model,Cancellation(self.lib) as token:
            x=model.add_variable(VariableType.INTEGER,0,1)
            token.cancel()
            with model.solve_pool(PoolOptions(Options(cancellation=token))) as pool:
                self.assertEqual(pool.termination,Termination.CANCELLED)
                self.assertEqual(pool.info["entry_count"],0)
                self.assertIsNotNone(pool.message)
            with model.relax_feasibility(RepairOptions(Options(cancellation=token))) as repair:
                self.assertEqual(repair.termination,Termination.CANCELLED)
                self.assertFalse(repair.has_repair)
                self.assertEqual(repair.variable_map[0].source,x)
                self.assertIsNone(repair.variable_map[0].private)
            for method,wrapper in ((model.solve_pool,PoolOptions),(model.relax_feasibility,RepairOptions)):
                with method(wrapper(Options(time_limit_seconds=0))) as result:
                    self.assertEqual(result.termination,Termination.TIME_LIMIT)
                with method(wrapper(Options(guarantee=Guarantee.CERTIFIED))) as result:
                    self.assertEqual(result.termination,Termination.UNSUPPORTED)
            with model.solve_pool(PoolOptions(Options(node_limit=1),max_solutions=2)) as pool:
                self.assertEqual(pool.termination,Termination.UNSUPPORTED)

    def test_cancel_and_destroy_source_during_pool_call(self):
        # Many requested classes prevent a vacuous already-completed oracle.
        # A backend-disabled build still checks its owning Unsupported outcome.
        model=Model(self.lib)
        for _ in range(16): model.add_variable(VariableType.BINARY)
        identity=model.identity
        with Cancellation(self.lib) as token,concurrent.futures.ThreadPoolExecutor(max_workers=1) as executor:
            entered=threading.Event()
            function=self.lib._functions["pool_solve"]
            def observed(*args):
                entered.set()
                return function(*args)
            self.lib._functions["pool_solve"]=observed
            try:
                future=executor.submit(model.solve_pool,PoolOptions(Options(backend=Backend.HIGHS,
                    cancellation=token,time_limit_seconds=3),max_solutions=10000))
                self.assertTrue(entered.wait(1))
                time.sleep(.02)
                if self.highs: self.assertFalse(future.done(),"fixture completed before testing in-flight destruction")
                token.cancel();model.close()
                with future.result(timeout=4) as pool:
                    self.assertEqual((pool.info["model_id"],pool.info["revision"]),identity)
                    self.assertEqual(pool.termination,Termination.CANCELLED if self.highs else Termination.UNSUPPORTED)
                    self.assertFalse(pool.exhausted)
            finally:
                self.lib._functions["pool_solve"]=function
                model.close()


if __name__=="__main__":
    unittest.main()
