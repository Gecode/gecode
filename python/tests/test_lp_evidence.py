"""Numerical LP evidence boundaries, private raw stages, and historical ownership."""
import ctypes as C
from dataclasses import FrozenInstanceError
import math
import unittest
from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Library, Model,
    Options, Row, Session, Termination as T, Variable, VariableType as V, load_library,
    LpEvidenceOptions as EO, LpEvidenceRequest as Request, LpEvidenceState as State,
    LpEvidenceReason as Reason, LpEvidenceCompletion as Completion, LpEvidencePhase as Phase,
    LpEvidenceSide as Side, LpEvidenceColumnKind as Kind)
from gecode_optimize import binding as b

class LpEvidenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library();cls.available=cls.library.capabilities(Backend.HIGHS)['available']
    def ray_model(self):
        m=Model(self.library);dead=m.add_variable();m.remove(dead)
        x=m.add_variable();y=m.add_variable(lower=-math.inf)
        gone=m.add_row();m.remove(gone);row=m.add_row([(x,1),(y,-1)],0,0)
        m.set_objective([(x,-1)],offset=7)
        return m,x,y,row,dead,gone
    def test_ray_groups_and_owning_raw_stage(self):
        m,x,y,row,dead,gone=self.ray_model();identity=m.identity
        with m,m.analyze_lp_evidence(EO(request=Request.BOTH)) as result:
            info=result.info;self.assertTrue(info.has_evidence);self.assertEqual((info.model_id,info.revision),identity)
            self.assertEqual(info.stage_count,3);self.assertEqual((info.column_slots,info.row_slots),(3,2))
            self.assertEqual(m.identity,identity)
            diag=result.diagnostics;self.assertEqual(len(diag.columns),3);self.assertFalse(diag.columns[0].active)
            self.assertIsNone(diag.columns[0].base_value);self.assertIsNone(diag.columns[0].direction)
            self.assertFalse(diag.rows[0].active);self.assertEqual(diag.columns[x.slot].source,x)
            with self.assertRaises(FrozenInstanceError):info.work=0
            with self.assertRaises(FrozenInstanceError):diag.columns[0].direction=1
            if self.available:
                self.assertEqual(info.completion,Completion.COMPLETE);self.assertIsNone(info.stop_reason);self.assertEqual(info.attempted_calls,3)
                self.assertEqual(result.primal_ray.state,State.AVAILABLE);self.assertEqual(result.farkas.state,State.UNAVAILABLE)
                self.assertEqual((result.direction_value(x),result.direction_value(y)),(1,1));self.assertEqual(result.base_value(x),0)
                self.assertEqual(diag.primal.normalized_objective_slope,-1);self.assertTrue(diag.primal.base_check.valid)
                self.assertEqual(result.slot(row).direction,0)
            else:
                self.assertEqual(info.stop_reason,T.UNSUPPORTED);self.assertEqual(info.attempted_calls,0)
                self.assertEqual(result.primal_ray.state,State.REJECTED);self.assertIsNone(diag.primal.base_check)
                self.assertIsNone(diag.primal.normalized_objective_slope)
                with self.assertRaises(ApiError):result.direction_value(x)
            for invalid in (dead,gone,Variable(x.model_id+1,x.slot,self.library)):
                with self.assertRaises(ApiError):result.slot(invalid)
            with self.assertRaises(TypeError):result.base_value(row)
            with self.assertRaises(ApiError):result.multiplier(row)
            child=result.copy_stage(1);stage=child.info;columns=child.columns;values=child.raw_values
            self.assertEqual(stage.phase,Phase.RECESSION);self.assertNotEqual(stage.private_model_id,identity[0]);self.assertEqual(stage.column_count,2)
            self.assertEqual(columns[0].source,x);self.assertEqual(columns[1].source,y)
            self.assertEqual(columns[0].kind,Kind.SOURCE_VARIABLE);self.assertIsNone(columns[0].side)
            self.assertEqual(columns[0].private_variable.model_id,stage.private_model_id)
            self.assertFalse(hasattr(child,'has_solution'));self.assertFalse(hasattr(child,'copy_result'))
            if self.available:
                self.assertTrue(stage.attempted);self.assertTrue(stage.candidate_examined);self.assertTrue(stage.check.valid)
                self.assertTrue(stage.raw_result.reported_solution_validated);self.assertEqual(stage.raw_result.termination_code,int(T.OPTIMAL))
                self.assertEqual(stage.raw_result.objective,-1);self.assertEqual(stage.raw_result.model_id,stage.private_model_id)
                self.assertEqual(values[0].reported_value,1);self.assertEqual(values[0].reported_mask,1)
            else:
                self.assertFalse(stage.attempted);self.assertIsNone(stage.raw_result);self.assertIsNone(stage.check);self.assertEqual(values,())
            m.set_objective([(y,1)]);m.remove(row);m.remove(x)
            self.assertEqual(result.slot(x),diag.columns[x.slot])
        with child:
            self.assertEqual(child.info,stage);self.assertEqual(child.columns,columns);self.assertEqual(child.raw_values,values)
            self.assertEqual(child.raw_backend,'HiGHS' if self.available else '')
        self.assertEqual(diag.metadata.primal_tolerance,1e-7)
        with self.assertRaises(RuntimeError):child.info
        with self.assertRaises(RuntimeError):result.info
    def test_requests_and_public_solve_counts(self):
        for request,count in ((Request.AUTOMATIC,2),(Request.PRIMAL_RAY,2),(Request.FARKAS,1),(Request.BOTH,3)):
            with Model(self.library) as m:
                x=m.add_variable();m.set_objective([(x,-1)])
                with m.analyze_lp_evidence(EO(request=request)) as result:
                    self.assertEqual(result.info.attempted_calls,count if self.available else 0)
                    if request==Request.FARKAS:self.assertEqual(result.primal_ray.state,State.NOT_REQUESTED)
                    if request==Request.PRIMAL_RAY:self.assertEqual(result.farkas.state,State.NOT_REQUESTED)
                    if request==Request.AUTOMATIC:
                        with result.copy_stage(2) as unused:
                            self.assertFalse(unused.info.attempted);self.assertIsNone(unused.info.raw_result)
        with Model(self.library) as empty,empty.analyze_lp_evidence(EO(request=Request.BOTH)) as result:
            # Includes local empty/constant solve decisions, not raw vendor runs.
            self.assertEqual(result.info.attempted_calls,3 if self.available else 0)
            if self.available:
                self.assertEqual(result.primal_ray.state,State.UNAVAILABLE);self.assertEqual(result.farkas.state,State.UNAVAILABLE)
                self.assertEqual(result.diagnostics.columns,());self.assertTrue(result.diagnostics.primal.base_check.valid)
    def test_farkas_signs_sides_and_auxiliary_coordinates(self):
        for upper in (False,True):
            for maximize in (False,True):
                with Model(self.library) as m:
                    x=m.add_variable(lower=0 if upper else -math.inf,upper=math.inf if upper else 0)
                    row=m.add_row([(x,1)],lower=-math.inf if upper else 1,upper=-1 if upper else math.inf)
                    m.set_objective([(x,3)],maximize=maximize,offset=1e16)
                    with m.analyze_lp_evidence(EO(request=Request.FARKAS)) as result:
                        self.assertEqual(result.primal_ray.state,State.NOT_REQUESTED)
                        if self.available:
                            self.assertEqual(result.farkas.state,State.AVAILABLE)
                            yr=result.multiplier(row);zc=result.multiplier(x)
                            self.assertEqual(yr.multiplier,-1 if upper else 1);self.assertEqual(zc.multiplier,1 if upper else -1)
                            self.assertEqual(yr.side,Side.UPPER if upper else Side.LOWER)
                            self.assertEqual(zc.side,Side.LOWER if upper else Side.UPPER)
                            self.assertEqual(yr.contribution+zc.contribution,1);self.assertEqual(zc.selected_bound,0)
                            self.assertEqual(result.diagnostics.farkas.contradiction_margin,1)
                        else:self.assertIsNone(result.diagnostics.farkas.contradiction_margin)
                        with result.copy_stage(0) as stage:
                            maps=stage.columns;self.assertEqual(len(maps),2)
                            self.assertEqual(maps[0].source,row);self.assertEqual(maps[0].kind,Kind.ROW_SIDE)
                            self.assertEqual(maps[1].source,x);self.assertEqual(maps[1].kind,Kind.VARIABLE_SIDE)
                            if self.available:self.assertAlmostEqual(stage.info.raw_result.objective,0.5)
        with Model(self.library) as m:
            x=m.add_variable(lower=-math.inf);lower=m.add_row([(x,1)],lower=1);upper=m.add_row([(x,1)],upper=0)
            with m.analyze_lp_evidence(EO(request=Request.FARKAS)) as result:
                if self.available:
                    zero=result.multiplier(x);self.assertEqual(zero.multiplier,0);self.assertIsNone(zero.side);self.assertIsNone(zero.selected_bound)
                    self.assertEqual(zero.contribution,0);self.assertEqual(result.multiplier(lower).multiplier,1);self.assertEqual(result.multiplier(upper).multiplier,-1)
        with Model(self.library) as m:
            row=m.add_row(upper=-2)
            with m.analyze_lp_evidence() as result:
                if self.available:self.assertEqual(result.multiplier(row).multiplier,-1);self.assertEqual(result.diagnostics.farkas.contradiction_margin,2)
                self.assertEqual(result.diagnostics.columns,())
    def test_direction_sense_offset_and_unavailable_bounded(self):
        for upper in (False,True):
            for maximize in (False,True):
                with Model(self.library) as m:
                    x=m.add_variable(lower=-math.inf if upper else 0,upper=0 if upper else math.inf)
                    coefficient=(1 if upper else -1)*(-1 if maximize else 1)
                    m.set_objective([(x,coefficient)],maximize=maximize,offset=1e16)
                    with m.analyze_lp_evidence() as result:
                        if self.available:self.assertEqual(result.direction_value(x),-1 if upper else 1);self.assertEqual(result.diagnostics.primal.normalized_objective_slope,-1)
        with Model(self.library) as m:
            x=m.add_variable(upper=1);m.set_objective([(x,1)])
            with m.analyze_lp_evidence(EO(request=Request.BOTH)) as result:
                if self.available:
                    self.assertEqual(result.primal_ray.state,State.UNAVAILABLE);self.assertEqual(result.farkas.state,State.UNAVAILABLE)
                    self.assertTrue(result.diagnostics.primal.base_check.valid)
                    with self.assertRaises(ApiError):result.base_value(x)
                    with self.assertRaises(ApiError):result.direction_value(x)
    def test_limits_cancel_and_unsupported_scope(self):
        with Model(self.library) as m,Cancellation(self.library) as cancel:
            x=m.add_variable();m.set_objective([(x,-1)]);cancel.cancel()
            for options,reason in ((EO(solve=Options(time_limit_seconds=0)),T.TIME_LIMIT),
                (EO(solve=Options(cancellation=cancel)),T.CANCELLED),(EO(solve=Options(node_limit=0)),T.NODE_LIMIT),
                (EO(max_work=0),T.ITERATION_LIMIT),(EO(max_auxiliary_variables=0),T.MEMORY_LIMIT),
                (EO(max_auxiliary_rows=0),T.MEMORY_LIMIT),(EO(max_auxiliary_nonzeros=0),T.MEMORY_LIMIT),
                (EO(max_retained_slots=0),T.MEMORY_LIMIT),(EO(max_auxiliary_solves=0),T.ITERATION_LIMIT if self.available else T.UNSUPPORTED)):
                with m.analyze_lp_evidence(options) as result:
                    self.assertEqual(result.info.stop_reason,reason);self.assertEqual(result.info.attempted_calls,0)
                    if not result.info.has_evidence:self.assertIsNone(result.primal_ray);self.assertIsNone(result.farkas);self.assertIsNone(result.diagnostics)
            with m.analyze_lp_evidence(EO(max_auxiliary_solves=1)) as result:
                self.assertEqual(result.info.attempted_calls,1 if self.available else 0)
                self.assertNotEqual(result.primal_ray.state,State.AVAILABLE)
            for solve in (Options(backend=Backend.NATIVE),Options(guarantee=Guarantee.EXACT),Options(guarantee=Guarantee.CERTIFIED),Options(primal_start=[(x,0)])):
                with m.analyze_lp_evidence(EO(solve=solve)) as result:
                    self.assertEqual(result.info.stop_reason,T.UNSUPPORTED);self.assertEqual(result.info.attempted_calls,0)
        for kind in ('fixed_integer','semi','global','indicator'):
            with Model(self.library) as m:
                x=m.add_variable(V.INTEGER if kind=='fixed_integer' else V.SEMI_CONTINUOUS if kind=='semi' else V.CONTINUOUS,1,1)
                if kind=='global':m.add_all_different([])
                if kind=='indicator':
                    gate=m.add_variable(V.BINARY);m.add_indicator(gate,True,[(x,1)],lower=1)
                with m.analyze_lp_evidence() as result:self.assertEqual(result.info.stop_reason,T.UNSUPPORTED)
    def test_c_layout_buffers_wrong_kind_and_python_types(self):
        m,x,*_=self.ray_model()
        with m:
            for mutate in (lambda o:setattr(o,'struct_size',1),lambda o:setattr(o,'reserved',1),lambda o:setattr(o,'reserved_flags',1),
                           lambda o:setattr(o,'request',99),lambda o:setattr(o.solve,'struct_size',1),lambda o:setattr(o.solve,'reserved',1)):
                native=EO()._marshal(self.library);mutate(native);out=b.U64(99)
                with self.assertRaises(ApiError):self.library.call('analyze_lp_evidence',m._open(),C.byref(native),C.byref(out))
                self.assertEqual(out.value,0)
            for opts in (EO(stationarity=-1),EO(recession=math.inf),EO(minimum_contradiction=math.nan)):
                with self.assertRaises(ApiError):m.analyze_lp_evidence(opts)
            for opts in (Options(),object()):
                with self.assertRaises(TypeError):m.analyze_lp_evidence(opts)
            for value in (True,0.5):
                with self.assertRaises(TypeError):m.analyze_lp_evidence(EO(max_auxiliary_solves=value))
            other=Library(self.library.path)
            with m.analyze_lp_evidence() as result,result.copy_stage(0) as child:
                for owner in (result,child):
                    ordinary=b._Info()
                    with self.assertRaises(ApiError):self.library.call('result_info',owner._open(),C.byref(ordinary),C.sizeof(ordinary))
                out=b._EvidenceStage()
                with self.assertRaises(ApiError):self.library.call('lp_evidence_stage_info',child._open(),C.byref(out),C.sizeof(out)-1)
                with self.assertRaises(ApiError):self.library.call('lp_evidence_stage_info',result._open(),C.byref(out),C.sizeof(out))
                needed=b.U64();slots=(b._EvidenceSlot*1)();slots[0].active=77
                with self.assertRaises(ApiError) as error:self.library.call('lp_evidence_slots',result._open(),1,slots,C.sizeof(b._EvidenceSlot),1,C.byref(needed))
                self.assertEqual(error.exception.code,6);self.assertEqual(slots[0].active,77);self.assertEqual(needed.value,3)
                with self.assertRaises(ApiError):self.library.call('lp_evidence_slots',result._open(),1,None,C.sizeof(b._EvidenceSlot)-1,0,C.byref(needed))
                with self.assertRaises(ApiError):self.library.call('lp_evidence_stage_text',child._open(),99,None,0,C.byref(needed))
                with self.assertRaises(ApiError):result.copy_stage(99)
                with self.assertRaises(ValueError):result.slot(Variable(x.model_id,x.slot,other))
                with self.assertRaises(TypeError):result.slot(object())
                with self.assertRaises(FrozenInstanceError):child.info.phase=Phase.FARKAS
                raw=b._EvidenceRawValue();self.library.call('lp_evidence_stage_raw_values',child._open(),None,C.sizeof(raw),0,C.byref(needed))
                if self.available:self.assertGreater(needed.value,0)

if __name__=='__main__':unittest.main()
