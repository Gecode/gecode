"""Scenario boundaries, independent hand/exhaustive oracles, and owning history."""
import ctypes as C
from dataclasses import FrozenInstanceError
import math
import unittest
from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Library, Model,
    Options, Result, Row, Session, Termination as T, Variable, VariableType as V, load_library,
    ScenarioDefinition as D, ScenarioVariableBounds as VB, ScenarioRowBounds as RB,
    ScenarioOptions as SO, ScenarioReuse as Reuse, ScenarioBatchCompletion as Completion,
    ScenarioRunState as State, ScenarioId)
from gecode_optimize import binding as b

class ScenarioTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library()
    def model(self):
        m=Model(self.library);x=m.add_variable(V.INTEGER,0,8);y=m.add_variable(V.INTEGER,0,8)
        dead=m.add_variable();m.remove(dead);gone=m.add_row();m.remove(gone)
        row=m.add_row([(x,1),(y,1)],lower=4);m.set_objective([(x,2),(y,3)],offset=7)
        return m,x,y,row,dead,gone
    def available(self,backend):return self.library.capabilities(backend)['available']
    def test_analytic_history_and_cold_reuse(self):
        for backend in (Backend.HIGHS,Backend.NATIVE):
            for reuse in (Reuse.AUTOMATIC,Reuse.COLD):
                m,x,y,row,dead,gone=self.model()
                defs=[D('base'),D('cap',variable_bounds=[VB(x,upper=1)]),
                      D('cost π',[(x,4),(y,1)],-10),D('rhs',row_bounds=[RB(row,lower=6)]),
                      D('infeasible',variable_bounds=[VB(x,upper=1),VB(y,upper=1)])]
                original=m.identity
                with m,m.solve_scenarios(defs,SO(solve=Options(backend=backend,guarantee=Guarantee.EXACT if backend==Backend.NATIVE else Guarantee.NUMERICAL),reuse=reuse)) as batch:
                    ids=[batch.scenario(i) for i in range(5)];info=batch.info
                    self.assertEqual((info.model_id,info.revision),original);self.assertNotEqual(info.batch_id,original[0])
                    self.assertEqual(m.identity,original)
                    saved=tuple(batch.definition(id) for id in ids);self.assertEqual(saved,tuple(defs))
                    with self.assertRaises(FrozenInstanceError):info.work=1
                    with self.assertRaises(FrozenInstanceError):saved[0].name='changed'
                    private=batch.map(x);private_row=batch.map(row)
                    self.assertEqual(private.model_id,info.batch_id);self.assertEqual(private_row.slot,row.slot)
                    for bad in (dead,gone,Variable(x.model_id+1,x.slot,self.library)):
                        with self.assertRaises(ApiError):batch.map(bad)
                    with self.assertRaises(TypeError):batch.map(ids[0])
                    if self.available(backend):
                        self.assertTrue(info.all_resolved);self.assertEqual(info.completion,Completion.COMPLETE)
                        self.assertIsNone(info.stop_reason);self.assertEqual(info.attempted,5);self.assertEqual(info.resolved,5)
                        for id,obj,values in zip(ids,[15,18,-6,19],[(4,0),(1,3),(0,4),(6,0)]):
                            outcome=batch.outcome(id);self.assertEqual(outcome.result.termination,T.OPTIMAL)
                            self.assertEqual(outcome.result.revision,id.index+1);self.assertTrue(outcome.check.candidate_examined)
                            self.assertTrue(outcome.check.validation.valid);self.assertTrue(outcome.check.objective_matches)
                            self.assertEqual((batch.value(id,x),batch.value(id,y)),values)
                            with batch.copy_result(id) as child:
                                self.assertEqual(child.objective,obj);self.assertEqual(child.value(private),values[0])
                                with self.assertRaises(ApiError):child.value(x)
                        bad=batch.outcome(ids[-1]);self.assertEqual(bad.result.termination,T.INFEASIBLE)
                        self.assertFalse(bad.check.candidate_examined);self.assertIsNone(bad.check.validation)
                        with self.assertRaises(ApiError):batch.value(ids[-1],x)
                        child=batch.copy_result(ids[0]);metadata=batch.outcome(ids[0])
                        if backend==Backend.HIGHS and reuse==Reuse.AUTOMATIC:
                            self.assertEqual(info.reuse_statistics.solve_calls,5)
                            self.assertGreater(info.reuse_statistics.incremental_updates,0)
                        else:self.assertEqual(info.reuse_statistics.solve_calls,0)
                    else:
                        self.assertEqual(info.stop_reason,T.UNSUPPORTED);self.assertFalse(info.all_resolved)
                        self.assertEqual(batch.outcome(ids[0]).result.termination,T.UNSUPPORTED)
                        self.assertEqual(batch.outcome(ids[1]).state,State.NOT_STARTED)
                        self.assertIsNone(batch.outcome(ids[1]).result);self.assertIsNone(batch.check(ids[1]))
                        with self.assertRaises(ApiError):batch.copy_result(ids[1])
                        child=batch.copy_result(ids[0]);metadata=batch.outcome(ids[0])
                    m.set_bounds(row,7,math.inf);m.set_objective([(y,3)]);m.remove(row);m.remove(x)
                    self.assertEqual(batch.definition(ids[1]),defs[1]);self.assertEqual(batch.map(x),private)
                    with m.solve_scenarios([D()]) as later:
                        if later.info.batch_id is not None:
                            with self.assertRaises(ApiError):later.outcome(ids[0])
                with child:
                    self.assertEqual(child.info['model_id'],private.model_id)
                    if self.available(backend):self.assertEqual(child.value(private),4);self.assertEqual(child.objective,15)
                self.assertEqual(saved[2].name,'cost π');self.assertEqual(metadata.scenario,ids[0])
                with self.assertRaises(RuntimeError):batch.outcome(ids[0])
    def test_small_exhaustive_min_max(self):
        for backend in (Backend.HIGHS,Backend.NATIVE):
            if not self.available(backend):continue
            for maximize in (False,True):
                with Model(self.library) as m:
                    x=m.add_variable(V.INTEGER,-2,2);y=m.add_variable(V.INTEGER,-2,2)
                    row=m.add_row([(x,2),(y,-1)],lower=-2,upper=2);m.set_objective([(x,1),(y,2)],maximize=maximize,offset=5)
                    defs=[];expected=[]
                    for lower in (-2,0,1):
                        for c in (-3,0,4):
                            defs.append(D(objective_coefficients=[(x,c)],objective_offset=-7,variable_bounds=[VB(y,lower=lower)]))
                            choices=[(c*a+2*d-7,a,d) for a in range(-2,3) for d in range(lower,3) if -2<=2*a-d<=2]
                            expected.append((max if maximize else min)(v[0] for v in choices))
                    with m.solve_scenarios(defs,SO(solve=Options(backend=backend))) as result:
                        self.assertTrue(result.info.all_resolved)
                        for i,obj in enumerate(expected):
                            with result.copy_result(result.scenario(i)) as child:self.assertEqual(child.objective,obj)
    def test_continuous_recourse_and_unbounded_status(self):
        with Model(self.library) as m:
            x=m.add_variable(V.CONTINUOUS,0,math.inf);y=m.add_variable(V.CONTINUOUS,0,10)
            row=m.add_row([(x,1),(y,1)],lower=3);m.set_objective([(x,2),(y,3)],offset=7)
            defs=[D(),D(variable_bounds=[VB(x,upper=1)]),
                  D(objective_coefficients=[(x,4)]),D(row_bounds=[RB(row,lower=4)])]
            with m.solve_scenarios(defs,SO(solve=Options(backend=Backend.HIGHS))) as batch:
                if self.available(Backend.HIGHS):
                    self.assertTrue(batch.info.all_resolved)
                    for i,objective in enumerate((13,15,16,15)):
                        with batch.copy_result(batch.scenario(i)) as child:self.assertAlmostEqual(child.objective,objective)
                else:self.assertEqual(batch.info.stop_reason,T.UNSUPPORTED)
            with m.solve_scenarios([D()],SO(solve=Options(backend=Backend.NATIVE))) as batch:
                self.assertEqual(batch.info.stop_reason,T.UNSUPPORTED)
        with Model(self.library) as m:
            x=m.add_variable(V.CONTINUOUS,0,math.inf);m.set_objective([(x,-1)])
            with m.solve_scenarios([D(),D(variable_bounds=[VB(x,upper=2)])],SO(solve=Options(backend=Backend.HIGHS))) as batch:
                if self.available(Backend.HIGHS):
                    self.assertTrue(batch.info.all_resolved)
                    first=batch.outcome(batch.scenario(0));self.assertEqual(first.result.termination,T.UNBOUNDED)
                    with batch.copy_result(batch.scenario(0)) as unbounded:
                        # Unbounded can retain a finite feasible point; that is
                        # independent of the status and not an attained optimum.
                        if unbounded.has_solution:
                            self.assertTrue(first.check.candidate_examined);self.assertTrue(first.check.validation.valid)
                            self.assertEqual(unbounded.objective,-batch.value(batch.scenario(0),x))
                        else:self.assertIsNone(unbounded.objective)
                        self.assertIsNone(unbounded.relative_gap)
                    with batch.copy_result(batch.scenario(1)) as finite:self.assertEqual(finite.objective,-2)
                else:self.assertEqual(batch.info.stop_reason,T.UNSUPPORTED)
    def test_empty_and_limits(self):
        m,x,y,row,*_=self.model()
        with m,Cancellation(self.library) as cancel:
            with m.solve_scenarios([]) as empty:
                self.assertTrue(empty.info.all_resolved);self.assertEqual(empty.info.attempted,0)
                with self.assertRaises(ApiError):empty.scenario(0)
            cancel.cancel()
            cases=[(SO(solve=Options(time_limit_seconds=0)),T.TIME_LIMIT),
                   (SO(solve=Options(cancellation=cancel)),T.CANCELLED),
                   (SO(solve=Options(time_limit_seconds=0),max_scenarios=0),T.TIME_LIMIT),
                   (SO(solve=Options(cancellation=cancel),max_scenarios=0),T.CANCELLED),
                   (SO(solve=Options(node_limit=0)),T.NODE_LIMIT),
                   (SO(solve=Options(node_limit=1)),T.UNSUPPORTED),
                   (SO(max_scenarios=1),T.MEMORY_LIMIT),(SO(max_patch_entries=0),T.MEMORY_LIMIT),
                   (SO(max_saved_value_slots=1),T.MEMORY_LIMIT),(SO(max_work=0),T.ITERATION_LIMIT)]
            for options,reason in cases:
                with m.solve_scenarios([D(objective_coefficients=[(x,2)]),D()],options) as result:
                    self.assertEqual(result.info.stop_reason,reason);self.assertFalse(result.info.all_resolved)
                    self.assertEqual(result.info.attempted,0);self.assertIsNone(result.info.batch_id)
                    self.assertTrue(result.message)
            if self.available(Backend.NATIVE):
                with m.solve_scenarios([D()],SO(solve=Options(backend=Backend.NATIVE,node_limit=100000))) as result:
                    self.assertTrue(result.info.all_resolved)
    def test_rejected_semantics_and_kinds(self):
        m,x,y,row,dead,gone=self.model()
        with m:
            bad=[D(objective_coefficients=[(x,1),(x,2)]),D(objective_coefficients=[(dead,1)]),
                 D(objective_coefficients=[(Variable(x.model_id+10,x.slot,self.library),1)]),
                 D(objective_coefficients=[(x,math.nan)]),D(objective_offset=math.inf),
                 D(variable_bounds=[VB(x,lower=9)]),D(variable_bounds=[VB(x,lower=math.nan)]),
                 D(variable_bounds=[VB(x,lower=1),VB(x,upper=2)]),D(row_bounds=[RB(gone,lower=1)]),
                 D(row_bounds=[RB(row,upper=1),RB(row,lower=0)])]
            for definition in bad:
                with m.solve_scenarios([D(),definition]) as result:
                    self.assertEqual(result.info.completion,Completion.REJECTED);self.assertEqual(result.info.stop_reason,T.INVALID_MODEL)
                    self.assertEqual(result.info.offending_scenario,1);self.assertEqual(result.info.attempted,0)
            for options in (SO(solve=Options(primal_start=[(x,4),(y,0)])),SO(solve=Options(guarantee=Guarantee.CERTIFIED))):
                with m.solve_scenarios([D()],options) as result:self.assertEqual(result.info.stop_reason,T.UNSUPPORTED)
            with m.solve_scenarios([D()]) as result:
                id=result.scenario(0)
                for wrong in (ScenarioId(id.batch_id+1,0,self.library),ScenarioId(id.batch_id,9,self.library)):
                    for fn in (result.outcome,result.check,result.definition,result.copy_result):
                        with self.assertRaises(ApiError):fn(wrong)
                with self.assertRaises(TypeError):result.outcome(x)
                with Session(self.library) as session:
                    info=b._ScenarioInfo()
                    with self.assertRaises(ApiError):self.library.call('scenario_batch_info',session._open(),C.byref(info),C.sizeof(info))
        for kind in ('semi','indicator','global','deleted_global'):
            with Model(self.library) as m:
                x=m.add_variable(V.SEMI_INTEGER if kind=='semi' else V.INTEGER,1,2)
                if kind=='indicator':
                    gate=m.add_variable(V.BINARY);m.add_indicator(gate,True,[(x,1)],lower=1)
                if kind in ('global','deleted_global'):
                    g=m.add_all_different([x])
                    if kind=='deleted_global':m.remove(g)
                with m.solve_scenarios([D()]) as result:self.assertEqual(result.info.stop_reason,T.UNSUPPORTED)
    def test_record_sizes_arrays_presence_and_copy(self):
        m,x,y,row,*_=self.model()
        with m:
            base=SO()._marshal(self.library);definition=b._scenario_definitions([D('saved',[(x,0)],0,[VB(x,upper=7)],[RB(row,lower=2)])],self.library)
            for mutate in (lambda o:setattr(o,'struct_size',C.sizeof(o)-1),lambda o:setattr(o,'reserved',1),
                           lambda o:setattr(o,'reserved_flags',1),lambda o:setattr(o,'reuse',2),
                           lambda o:setattr(o.solve,'struct_size',0),lambda o:setattr(o.solve,'reserved',1)):
                native=SO()._marshal(self.library);mutate(native);out=b.U64(99)
                with self.assertRaises(ApiError):self.library.call('solve_scenarios',m._open(),definition,1,C.sizeof(b._ScenarioDefinition),C.byref(native),C.byref(out))
                self.assertEqual(out.value,0)
            for key,val in (('struct_size',1),('reserved',1)):
                data=b._scenario_definitions([D()],self.library);setattr(data[0],key,val);out=b.U64(99)
                with self.assertRaises(ApiError):self.library.call('solve_scenarios',m._open(),data,1,C.sizeof(b._ScenarioDefinition),C.byref(base),C.byref(out))
                self.assertEqual(out.value,0)
            for mutate in (lambda d:setattr(d[0].objective_offset,'present',2),lambda d:setattr(d[0].objective_offset,'reserved',1),
                           lambda d:setattr(d[0].variable_bounds[0],'struct_size',0),lambda d:setattr(d[0].variable_bounds[0],'has_upper',2),
                           lambda d:setattr(d[0].row_bounds[0],'reserved',1)):
                data=b._scenario_definitions([D(variable_bounds=[VB(x,upper=7)],row_bounds=[RB(row,lower=2)])],self.library);mutate(data);out=b.U64()
                with self.assertRaises(ApiError):self.library.call('solve_scenarios',m._open(),data,1,C.sizeof(b._ScenarioDefinition),C.byref(base),C.byref(out))
            token=b.U64();self.library.call('solve_scenarios',m._open(),definition,1,C.sizeof(b._ScenarioDefinition),C.byref(base),C.byref(token))
            definition[0].objective_offset.value=99;definition[0].variable_bounds[0].upper=1
            with b.ScenarioBatchResult(self.library,token.value) as result:
                id=result.scenario(0);copied=result.definition(id);self.assertEqual(copied.objective_offset,0);self.assertEqual(copied.variable_bounds[0].upper,7)
                info=b._ScenarioInfo()
                with self.assertRaises(ApiError):self.library.call('scenario_batch_info',result._open(),C.byref(info),C.sizeof(info)-1)
                needed=b.U64();raw=(b._ScenarioBounds*1)();raw[0].upper=123
                with self.assertRaises(ApiError):self.library.call('scenario_batch_bounds',result._open(),id._id(),1,raw,C.sizeof(b._ScenarioBounds)-1,1,C.byref(needed))
                self.assertEqual(raw[0].upper,123)
                self.library.call('scenario_batch_bounds',result._open(),id._id(),1,raw,C.sizeof(b._ScenarioBounds),1,C.byref(needed))
                self.assertEqual(raw[0].reserved,0);self.assertEqual(raw[0].has_lower,0);self.assertEqual(raw[0].lower,0);self.assertEqual(raw[0].upper,7)
                with self.assertRaises(ApiError):self.library.call('scenario_batch_text',result._open(),id._id(),2,None,0,C.byref(needed))
    def test_python_type_library_and_immutability(self):
        m,x,*_=self.model()
        with m:
            terms=[(x,1)];bounds=[VB(x,upper=3)];definition=D(objective_coefficients=terms,variable_bounds=bounds)
            terms.clear();bounds.clear();self.assertEqual(len(definition.objective_coefficients),1);self.assertEqual(len(definition.variable_bounds),1)
            for definitions in ([object()],[D(name='bad\0name')],[D(variable_bounds=[object()])],[D(row_bounds=[VB(x,upper=1)])]):
                with self.assertRaises((TypeError,ValueError)):m.solve_scenarios(definitions)
            for option in (Options(),object()):
                with self.assertRaises(TypeError):m.solve_scenarios([],option)
            for value in (True,0.5):
                with self.assertRaises(TypeError):m.solve_scenarios([],SO(max_work=value))
            other=Library(self.library.path)
            with Model(other) as foreign:
                alien=foreign.add_variable()
                with self.assertRaises(ValueError):m.solve_scenarios([D(objective_coefficients=[(alien,1)])])
                with m.solve_scenarios([D()]) as result:
                    with self.assertRaises(ValueError):result.outcome(ScenarioId(result.info.batch_id,0,other))

if __name__=='__main__':unittest.main()
