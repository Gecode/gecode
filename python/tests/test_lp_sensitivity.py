"""Original-basis numerical ranges through owning C/Python records."""
import ctypes as C
from dataclasses import FrozenInstanceError, replace
import math
import unittest
from unittest.mock import patch
from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Library, Model, Options, Row, Variable,
    VariableType, load_library, LpObservationOptions, LpBasisStatus, LpSensitivityOptions as SO,
    LpSensitivityLimits as Limits, LpSensitivityTolerances as Checks, LpObjectiveParameter as Obj,
    LpEqualityRhsParameter as Rhs, LpSensitivityCompletion as Complete, LpSensitivityReason as Reason,
    LpSensitivityState as State, LpRangeEndKind as End, LpRangeEnd, Termination)
from gecode_optimize import binding as b

class LpSensitivityTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library();cls.available=cls.library.lp_observation_capabilities().available
    def model(self,maximize=False):
        m=Model(self.library);dead=m.add_variable();m.remove(dead);x=m.add_variable();y=m.add_variable();z=m.add_variable(upper=1)
        gone=m.add_row();m.remove(gone);row=m.add_row([(x,1),(y,1)],3,3)
        m.set_objective([(x,-2 if maximize else 2),(y,-1 if maximize else 1)],maximize=maximize,offset=7)
        return m,x,y,z,row,dead,gone
    def test_analytic_endpoints_and_history(self):
        for maximize in (False,True):
            m,x,y,z,row,dead,gone=self.model(maximize)
            with m,m.solve_lp_observed() as source:
                original=source.info
                with source.analyze_sensitivity(SO(parameters=(Obj(x),Obj(y),Rhs(row)))) as result:
                    self.assertEqual(result.info.model_id,x.model_id);self.assertEqual(result.info.guarantee,Guarantee.NUMERICAL)
                    self.assertEqual(result.work.preparation_visits,3);copied=result.copy_source_observed()
                    if not self.available:
                        self.assertEqual(result.info.completion,Complete.REJECTED);self.assertIsNone(result.entries)
                        self.assertFalse(result.work.factor_setup_attempted)
                        with self.assertRaises(ApiError) as error:result.copy_basis()
                        self.assertEqual(error.exception.code,9)
                        copied.close();continue
                    self.assertEqual(result.info.completion,Complete.COMPLETE);self.assertIsNone(result.info.stop_reason)
                    a,c,r=result.entries;self.assertTrue(all(e.group.state is State.AVAILABLE and e.interval.checks.accepted for e in result.entries))
                    if maximize:
                        self.assertEqual(a.interval.lower.kind,End.NEGATIVE_INFINITY);self.assertEqual(a.interval.upper.value,-1)
                        self.assertEqual(c.interval.lower.value,-2);self.assertEqual(c.interval.upper.kind,End.POSITIVE_INFINITY)
                    else:
                        self.assertEqual(a.interval.lower.value,1);self.assertEqual(a.interval.upper.kind,End.POSITIVE_INFINITY)
                        self.assertEqual(c.interval.lower.kind,End.NEGATIVE_INFINITY);self.assertEqual(c.interval.upper.value,2)
                    self.assertEqual(r.interval.anchor,3);self.assertEqual(r.interval.lower.value,0)
                    self.assertEqual(r.interval.upper.kind,End.POSITIVE_INFINITY);self.assertIsNone(r.interval.upper.value)
                    self.assertEqual(r.interval.objective_slope,-1 if maximize else 1)
                    self.assertEqual(result.objective(x),a);self.assertEqual(result.equality_rhs(row),r);self.assertIsNone(result.objective(z))
                    self.assertEqual(result.factor_order,(y,));self.assertEqual(result.active_slots(Variable),(False,True,True,True))
                    self.assertEqual(result.active_slots(Row),(False,True));self.assertTrue(result.reference_checks.primal.valid)
                    self.assertTrue(result.reference_checks.kkt.accepted);self.assertTrue(result.reference_checks.basis_point_matches)
                    self.assertIsNotNone(result.reference_checks.max_system_residual)
                    work=result.work;result.entries;result.reference_checks;result.factor_order;self.assertEqual(work,result.work)
                    basis=result.copy_basis();saved=result.entries;checks=result.reference_checks
                    for bad in (dead,Variable(x.model_id+1,x.slot,self.library)):
                        with self.assertRaises(ApiError):result.objective(bad)
                    with self.assertRaises(ApiError):result.equality_rhs(gone)
                    with self.assertRaises(TypeError):result.objective(row)
                    with self.assertRaises(FrozenInstanceError):r.interval.anchor=99
                    m.set_objective_offset(999)
                with copied,basis,copied.copy_result() as ordinary:
                    self.assertEqual(copied.info,original);self.assertEqual(ordinary.objective,4 if maximize else 10)
                    self.assertEqual(basis.column(y),LpBasisStatus.BASIC)
                self.assertTrue(saved[0].interval.checks.accepted);self.assertTrue(checks.kkt.accepted)
                with self.assertRaises(RuntimeError):result.entry(0)
    def test_singletons_zero_coefficients_and_factor_entities(self):
        with Model(self.library) as m:
            x=m.add_variable(upper=2);z=m.add_variable(upper=1);r=m.add_row([(x,1)],1,1);q=m.add_row([(x,2)],2,2)
            m.set_objective([(x,1)])
            with m.solve_lp_observed() as source,source.analyze_sensitivity(SO(parameters=(Rhs(r),Rhs(q),Obj(z)))) as result:
                if not self.available:self.assertEqual(result.info.completion,Complete.REJECTED);return
                self.assertEqual(result.info.completion,Complete.COMPLETE)
                for entry,anchor in zip(result.entries[:2],(1,2)):
                    self.assertEqual(entry.interval.lower.value,anchor);self.assertEqual(entry.interval.upper.value,anchor)
                self.assertEqual(result.objective(z).interval.anchor,0)
                self.assertEqual(len(result.factor_order),2);self.assertTrue(any(isinstance(v,Row) for v in result.factor_order))
    def test_rejections_and_source_preservation(self):
        m,x,_,_,row,*_=self.model()
        with m,m.solve_lp_observed() as source:
            for options in (SO(parameters=(Obj(x),),backend=Backend.NATIVE),SO(parameters=(Obj(x),Obj(x))),
                            SO(parameters=(Obj(Variable(x.model_id+1,x.slot,self.library)),))):
                with source.analyze_sensitivity(options) as result,result.copy_source_observed() as copied:
                    self.assertEqual(result.info.completion,Complete.REJECTED);self.assertEqual(copied.info,source.info)
                    self.assertFalse(result.work.factor_setup_attempted)
            for observation in (LpObservationOptions(basis=False),LpObservationOptions(duals=False),LpObservationOptions(solve=Options(time_limit_seconds=0))):
                with m.solve_lp_observed(observation) as weak,weak.analyze_sensitivity(SO(parameters=(Obj(x),))) as result:
                    self.assertEqual(result.info.completion,Complete.REJECTED);self.assertFalse(result.work.factor_setup_attempted)
            m.set_bounds(row,0,4)
            with m.solve_lp_observed() as ranged,ranged.analyze_sensitivity(SO(parameters=(Rhs(row),))) as result:
                self.assertEqual(result.info.completion,Complete.REJECTED)
        for kind in (VariableType.INTEGER,VariableType.SEMI_CONTINUOUS):
            with Model(self.library) as m:
                x=m.add_variable(kind,1,2);m.add_row([(x,1)],1,2)
                with m.solve_lp_observed() as source,source.analyze_sensitivity(SO(parameters=(Obj(x),))) as result:
                    self.assertEqual(result.info.completion,Complete.REJECTED)
    def test_quotas_cancel_and_invalid_precedence(self):
        m,x,*_=self.model()
        with m,m.solve_lp_observed() as source,Cancellation(self.library) as token:
            self.assertFalse(token.cancelled)
            cases=((SO(parameters=(Obj(x),),time_limit_seconds=0),Termination.TIME_LIMIT),
                   (SO(parameters=(Obj(x),),limits=Limits(max_work=0)),Termination.ITERATION_LIMIT),
                   (SO(parameters=(Obj(x),),limits=Limits(max_requests=0)),Termination.MEMORY_LIMIT))
            for options,stop in cases:
                with source.analyze_sensitivity(options) as result:
                    self.assertEqual(result.info.completion,Complete.INTERRUPTED);self.assertEqual(result.info.stop_reason,stop)
                    self.assertEqual(result.work.preparation_visits,0);self.assertFalse(result.work.factor_setup_attempted)
            token.cancel();self.assertTrue(token.cancelled)
            with source.analyze_sensitivity(SO(parameters=(Obj(x),),cancellation=token)) as result:self.assertEqual(result.info.stop_reason,Termination.CANCELLED)
            for options in (SO(),SO(parameters=(Obj(x),),time_limit_seconds=-1,cancellation=token),
                            SO(parameters=(Obj(x),),checks=Checks(stationarity=math.nan),cancellation=token)):
                with source.analyze_sensitivity(options) as result:
                    self.assertEqual(result.info.completion,Complete.REJECTED);self.assertEqual(result.info.reason,Reason.INVALID_SOURCE)
                    self.assertIsNone(result.info.stop_reason)
            if self.available:
                with source.analyze_sensitivity(SO(parameters=(Obj(x),),limits=Limits(max_work=1))) as result:
                    self.assertEqual(result.work.preparation_visits,1);self.assertEqual(result.work.coordinator_visits,0)
                    self.assertEqual(result.info.reason,Reason.RESOURCE_LIMIT);self.assertFalse(result.work.factor_setup_attempted)
    def test_python_final_cleanup_clear(self):
        m,x,y,z,row,*_=self.model()
        with m,m.solve_lp_observed() as source,Cancellation(self.library) as token:
            if not self.available:return
            for cancel in (False,True):
                # C++ uses its real monotonic clock. Only the outer Python clock
                # advances after a successful C analysis, without a timing race.
                calls=iter((0.0,0.0,0.0 if cancel else 2.0));own_call=b._own_call
                def finish(*args):
                    result=own_call(*args)
                    if cancel and args[2]=='analyze_lp_sensitivity':token.cancel()
                    return result
                with patch.object(b.time,'monotonic',side_effect=lambda:next(calls)),patch.object(b,'_own_call',side_effect=finish):
                    result=source.analyze_sensitivity(SO(parameters=(Obj(x),Rhs(row)),time_limit_seconds=1,cancellation=token))
                with result:
                    self.assertEqual(result.info.completion,Complete.INTERRUPTED);self.assertEqual(result.info.reason,Reason.STOPPED)
                    self.assertEqual(result.info.stop_reason,Termination.CANCELLED if cancel else Termination.TIME_LIMIT)
                    self.assertTrue(result.info.has_sensitivity)
                    for entry in (*result.entries,result.entry(0),result.objective(x),result.equality_rhs(row)):
                        self.assertIsNone(entry.interval);self.assertEqual(entry.group.state,State.UNAVAILABLE)
                    self.assertIsNone(result.objective(z));self.assertTrue(result.reference_checks.kkt.accepted)
                    with result.copy_source_observed() as copied:self.assertEqual(copied.info,source.info)
                    with result.copy_basis() as basis:self.assertEqual(basis.column(y),LpBasisStatus.BASIC)
    def test_abi_records_buffers_and_types(self):
        m,x,*_=self.model()
        with m,m.solve_lp_observed() as source:
            for mutate in (lambda o:setattr(o,'struct_size',0),lambda o:setattr(o,'reserved',1),
                           lambda o:setattr(o.checks,'struct_size',0),lambda o:setattr(o.limits,'reserved',1),
                           lambda o:setattr(o.requests[0],'struct_size',0),lambda o:setattr(o.requests[0],'reserved_flags',1)):
                o=SO(parameters=(Obj(x),))._marshal(self.library);mutate(o);h=b.U64(99)
                with self.assertRaises(ApiError) as error:self.library.call('analyze_lp_sensitivity',source._open(),C.byref(o),C.byref(h))
                self.assertEqual(error.exception.code,1);self.assertEqual(h.value,0)
            for invalid in (Options(),object()):
                with self.assertRaises(TypeError):source.analyze_sensitivity(invalid)
            for params in ((object(),),(Obj(1),),(Obj(x),Rhs(x))):
                with self.assertRaises(TypeError):source.analyze_sensitivity(SO(parameters=params))
            another=Library(self.library.path)
            with Model(another) as other:
                alien=other.add_variable()
                with self.assertRaises(ValueError):source.analyze_sensitivity(SO(parameters=(Obj(alien),)))
            with source.analyze_sensitivity(SO(parameters=(Obj(x),))) as result:
                info=b._SensitivityInfo()
                with self.assertRaises(ApiError):self.library.call('sensitivity_info',result._open(),C.byref(info),C.sizeof(info)-1)
                if self.available:
                    needed=b.U64();entry=b._SensitivityEntry();C.memset(C.byref(entry),0xA5,C.sizeof(entry))
                    with self.assertRaises(ApiError) as error:self.library.call('sensitivity_entries',result._open(),C.byref(entry),C.sizeof(entry),0,C.byref(needed))
                    self.assertEqual(error.exception.code,6);self.assertEqual(entry.struct_size,0xa5a5a5a5a5a5a5a5)
            for kind,value in ((End.FINITE,None),(End.FINITE,math.inf),(End.POSITIVE_INFINITY,0)):
                with self.assertRaises(ValueError):LpRangeEnd(kind,value)
    def test_real_partial_interval_is_not_complete(self):
        with Model(self.library) as m:
            y=m.add_variable(lower=-math.inf);w=m.add_variable(upper=1)
            m.add_row([(y,1),(w,1e14)],1,1);m.set_objective([(y,100),(w,1e16-2)])
            with m.solve_lp_observed() as source,source.analyze_sensitivity(SO(parameters=(Obj(y),Obj(w)))) as result:
                if not self.available:self.assertEqual(result.info.completion,Complete.REJECTED);return
                with source.copy_result() as ordinary:self.assertEqual(ordinary.objective,98)
                self.assertEqual(result.info.completion,Complete.PARTIAL)
                rejected,accepted=result.entries
                self.assertEqual(rejected.group.state,State.REJECTED);self.assertEqual(rejected.group.reason,Reason.INTERVAL_CHECKS)
                self.assertIsNone(rejected.interval);self.assertEqual(accepted.group.state,State.AVAILABLE)
                self.assertEqual(accepted.interval.upper.value,1e16);self.assertEqual(result.entry(1),accepted)
                self.assertEqual(result.objective(y),rejected);self.assertTrue(result.reference_checks.kkt.accepted)
    def test_cancellation_owner_close_and_shared_copy(self):
        with Cancellation(self.library) as token,token.copy() as copied:
            self.assertFalse(copied.cancelled);token.cancel();token.close();self.assertTrue(copied.cancelled)
            with copied.copy() as second:copied.close();self.assertTrue(second.cancelled)
        m,x,*_=self.model()
        with m,m.solve_lp_observed() as source:
            if not self.available:return
            for cancel in (False,True):
                with Cancellation(self.library) as token:
                    own_call=b._own_call
                    def finish(*args):
                        result=own_call(*args)
                        if args[2]=='analyze_lp_sensitivity':
                            if cancel:token.cancel()
                            token.close()
                        return result
                    with patch.object(b,'_own_call',side_effect=finish):
                        result=source.analyze_sensitivity(SO(parameters=(Obj(x),),cancellation=token))
                    with result:
                        self.assertTrue(result.info.has_sensitivity)
                        self.assertEqual(result.info.completion,Complete.INTERRUPTED if cancel else Complete.COMPLETE)
                        self.assertEqual(result.info.stop_reason,Termination.CANCELLED if cancel else None)
                        self.assertEqual(result.entry(0).group.state,State.UNAVAILABLE if cancel else State.AVAILABLE)
                        self.assertTrue(result.reference_checks.kkt.accepted)
if __name__=='__main__':unittest.main()
