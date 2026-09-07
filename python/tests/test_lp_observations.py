"""Original-coordinate LP observations and owning C/Python boundary tests."""
import ctypes as C
from dataclasses import FrozenInstanceError
import math
import unittest

from gecode_optimize import (
    ApiError, Backend, Cancellation, Guarantee, Library, Model, Options, Row,
    Session, Termination, VariableType, load_library, LpObservationOptions,
    LpObservationState as State, LpObservationReason as Reason, LpBasisStatus,
    LpDualSource, QuadraticModel)
from gecode_optimize import binding as b


class LpObservationsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library()
        cls.available=cls.library.lp_observation_capabilities().available

    def model(self, maximize=False, offset=7):
        m=Model(self.library)
        x=m.add_variable(lower=0,upper=10);y=m.add_variable(lower=0,upper=10)
        dead=m.add_variable(lower=0,upper=10);m.remove(dead)
        removed=m.add_row();m.remove(removed)
        row=m.add_row([(x,1),(y,1)],lower=-math.inf if maximize else 3,
                      upper=3 if maximize else math.inf)
        objective=[(x,4 if maximize else 1),(y,1 if maximize else 2)]
        m.set_objective(objective,maximize=maximize,offset=offset)
        return m,x,y,row,dead,removed

    def test_analytic_signs_offsets_and_copied_history(self):
        for maximize in (False,True):
            for offset in (0,7,1e16):
                with self.subTest(maximize=maximize,offset=offset):
                    m,x,y,row,dead,removed=self.model(maximize,offset)
                    with m,Session(self.library) as session:
                        observed=session.solve_lp_observed(m)
                        data=observed.observations
                        ordinary=observed.copy_result()
                        self.assertEqual(data.model_id,x.model_id)
                        self.assertEqual(len(data.rows),2);self.assertEqual(len(data.columns),3)
                        self.assertFalse(data.rows[0].active);self.assertIsNone(data.rows[0].dual)
                        with self.assertRaises(ValueError):data.row(removed)
                        with self.assertRaises(ValueError):data.column(dead)
                        with self.assertRaises(ValueError):data.row(Row(row.model_id+1,row.slot,self.library))
                        with self.assertRaises(TypeError):data.row(x)
                        with self.assertRaises(FrozenInstanceError):data.rows[1].dual=99
                        if self.available:
                            self.assertEqual(observed.termination,Termination.OPTIMAL)
                            self.assertEqual(data.dual_point.state,State.AVAILABLE)
                            self.assertTrue(data.checks.accepted)
                            self.assertAlmostEqual(data.row(row).dual,4 if maximize else 1)
                            self.assertAlmostEqual(data.column(y).reduced_cost,-3 if maximize else 1)
                            self.assertEqual(data.column(y).basis,LpBasisStatus.LOWER)
                            self.assertAlmostEqual(data.row(row).activity,3)
                            self.assertAlmostEqual(data.checks.normalized_gap,0)
                            self.assertIsNone(data.row(row).lower_slack if maximize else data.row(row).upper_slack)
                            self.assertAlmostEqual(data.row(row).upper_slack if maximize else data.row(row).lower_slack,0)
                            self.assertEqual(ordinary.value(x),3)
                            self.assertIsNotNone(data.metadata.backend_dual_tolerance)
                        else:
                            self.assertEqual(observed.termination,Termination.UNSUPPORTED)
                            self.assertEqual(data.dual_point.reason,Reason.UNSUPPORTED)
                            self.assertIsNone(data.row(row).dual);self.assertIsNone(data.checks.normalized_gap)
                        m.set_bounds(row,-math.inf if maximize else 4,4 if maximize else math.inf)
                        observed.close()
                    # Frozen data and separately copied Result own their history.
                    self.assertEqual(data.row(row).activity,3 if self.available else None)
                    if self.available:self.assertEqual(ordinary.value(x),3)
                    ordinary.close()
                    with self.assertRaises(RuntimeError):observed.copy_result()

    def test_constant_row_and_request_groups(self):
        m,x,y,row,_,_=self.model()
        with m:
            constant=m.add_row(lower=-1,upper=1)
            for duals,basis in ((True,True),(False,True),(True,False),(False,False)):
                with m.solve_lp_observed(LpObservationOptions(duals=duals,basis=basis)) as result:
                    data=result.observations
                    if not duals:
                        self.assertEqual(data.dual_point.state,State.NOT_REQUESTED)
                        self.assertIsNone(data.row(row).dual);self.assertIsNone(data.column(y).reduced_cost)
                        self.assertIsNone(data.checks.max_stationarity)
                    elif self.available:
                        self.assertEqual(data.row(constant).dual,0)
                        self.assertEqual(data.row(constant).dual_source,LpDualSource.DERIVED_CONSTANT_ROW)
                    if not basis:self.assertEqual(data.basis.state,State.NOT_REQUESTED)
                    elif self.available:
                        self.assertEqual(data.basis.state,State.UNAVAILABLE)
                        self.assertEqual(data.basis.reason,Reason.ELIDED_CONSTANT_ROWS)
                    self.assertIsNone(data.row(constant).basis)

    def test_session_edits_interruption_no_stale_data(self):
        m,x,_,row,_,_=self.model()
        with m,Session(self.library) as session,Cancellation(self.library) as cancellation:
            with session.solve_lp_observed(m) as old:
                original=old.observations
                m.set_bounds(row,4,math.inf)
                with session.solve_lp_observed(m) as changed:
                    if self.available:self.assertEqual(changed.observations.row(row).activity,4)
                    self.assertNotEqual(changed.observations.revision,original.revision)
                cancellation.cancel()
                for options in (Options(time_limit_seconds=0),Options(cancellation=cancellation)):
                    with session.solve_lp_observed(m,LpObservationOptions(solve=options)) as stopped:
                        self.assertEqual(stopped.termination,
                            (Termination.CANCELLED if options.cancellation else Termination.TIME_LIMIT) if self.available else Termination.UNSUPPORTED)
                        data=stopped.observations
                        self.assertIsNone(data.row(row).activity);self.assertIsNone(data.row(row).dual)
                        self.assertIsNone(data.column(x).basis);self.assertFalse(data.checks.accepted)
                        self.assertIsNone(data.checks.max_stationarity)
                self.assertEqual(original.row(row).activity,3 if self.available else None)

    def test_unsupported_scope_and_types(self):
        m,x,_,row,_,_=self.model()
        with m:
            for solve in (Options(backend=Backend.NATIVE),Options(guarantee=Guarantee.EXACT),Options(guarantee=Guarantee.CERTIFIED)):
                with m.solve_lp_observed(LpObservationOptions(solve=solve)) as out:
                    self.assertEqual(out.termination,Termination.UNSUPPORTED)
                    self.assertFalse(out.has_solution);self.assertIsNone(out.observations.row(row).dual)
            global_id=m.add_all_different([])
            with m.solve_lp_observed() as out:self.assertEqual(out.termination,Termination.UNSUPPORTED)
            m.remove(global_id);m.add_variable(VariableType.INTEGER,0,0)
            with m.solve_lp_observed() as out:self.assertEqual(out.termination,Termination.UNSUPPORTED)
            for options in (Options(),LpObservationOptions(duals=1),LpObservationOptions(solve=object())):
                with self.assertRaises(TypeError):m.solve_lp_observed(options)
            for value in (-1,math.inf,math.nan):
                with self.assertRaises(ApiError):m.solve_lp_observed(LpObservationOptions(stationarity=value))
            with Session(self.library) as session,QuadraticModel(self.library) as qp:
                with self.assertRaises(TypeError):session.solve_lp_observed(qp)

    def test_c_sizes_flags_buffers_and_handle_kinds(self):
        m,x,y,row,_,_=self.model()
        with m,m.solve_lp_observed() as result:
            library=self.library
            for mutate in (lambda o:setattr(o,"struct_size",C.sizeof(o)-1),lambda o:setattr(o,"reserved",1),
                           lambda o:setattr(o,"duals",2),lambda o:setattr(o,"basis",-1),
                           lambda o:setattr(o.solve,"struct_size",C.sizeof(o.solve)-1),lambda o:setattr(o.solve,"reserved",1)):
                native=LpObservationOptions()._marshal(library);mutate(native);output=b.U64(99)
                with self.assertRaises(ApiError) as caught:library.call("solve_lp_observed",m._open(),C.byref(native),C.byref(output))
                self.assertEqual(caught.exception.code,1);self.assertEqual(output.value,0)
            out=b._LpInfo()
            with self.assertRaises(ApiError):library.call("lp_observed_result_info",result._open(),C.byref(out),C.sizeof(out)-1)
            with self.assertRaises(ApiError) as caught:library.call("lp_observed_result_info",m._open(),C.byref(out),C.sizeof(out))
            self.assertEqual(caught.exception.code,2)
            with self.assertRaises(ApiError):library.call("result_info",result._open(),C.byref(b._Info()),C.sizeof(b._Info))
            buffer=(b._LpRow*1)();C.memset(buffer,0x5a,C.sizeof(buffer));before=bytes(buffer);needed=b.U64()
            with self.assertRaises(ApiError) as caught:library.call("lp_observed_result_rows",result._open(),buffer,C.sizeof(b._LpRow),1,C.byref(needed))
            self.assertEqual(caught.exception.code,6);self.assertEqual(needed.value,2);self.assertEqual(bytes(buffer),before)
            record=b._LpRow();library.call("lp_observed_result_row",result._open(),row._id(),C.byref(record),C.sizeof(record))
            self.assertEqual(record.struct_size,C.sizeof(record));self.assertEqual(record.reserved,0)
            self.assertEqual(record.dual.reserved,0)
            if not self.available:self.assertEqual((record.dual.present,record.dual.value),(0,0))
            count=b.U64();library.call("lp_observed_result_text",result._open(),0,None,0,C.byref(count))
            text=C.create_string_buffer(count.value);library.call("lp_observed_result_text",result._open(),0,text,count.value,C.byref(count))
            self.assertEqual(text.raw[-1],0)

    def test_empty_available_rows_are_distinct_from_unavailable_duals(self):
        with Model(self.library) as model,model.solve_lp_observed() as result:
            data=result.observations
            self.assertEqual(data.rows,());self.assertEqual(data.columns,())
            self.assertEqual(data.primal_rows.state,State.AVAILABLE if self.available else State.UNAVAILABLE)
            self.assertEqual(data.dual_point.state,State.UNAVAILABLE)
            self.assertIsNone(data.checks.normalized_gap)
            with result.copy_result() as ordinary:
                self.assertEqual(ordinary.termination,Termination.OPTIMAL if self.available else Termination.UNSUPPORTED)

    def test_capabilities_are_copied_immutable(self):
        caps=self.library.lp_observation_capabilities()
        self.assertEqual(caps.available,caps.duals);self.assertEqual(caps.available,caps.basis_export)
        self.assertTrue(caps.limitations);self.assertIsInstance(caps.limitations,tuple)
        with self.assertRaises(FrozenInstanceError):caps.available=False


if __name__=="__main__":unittest.main()
