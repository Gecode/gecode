"""Owning LP basis factories and explicit submission semantics across the ABI."""
import ctypes as C
from dataclasses import FrozenInstanceError
import math
import unittest
from gecode_optimize import (ApiError, Backend, Cancellation, Guarantee, Library, Model,
    Options, Row, Session, Termination as T, VariableType, load_library, LpObservationOptions as LO,
    LpBasis, LpBasisOrigin as Origin, LpBasisStatus as B, LpBasisSubmissionState as S)
from gecode_optimize import binding as b

class LpBasisTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library();cls.available=cls.library.lp_observation_capabilities().available
    def model(self):
        m=Model(self.library);x=m.add_variable();y=m.add_variable();dead=m.add_variable();m.remove(dead)
        deleted=m.add_row();m.remove(deleted);row=m.add_row([(x,1),(y,1)],lower=4)
        m.set_objective([(x,2),(y,3)],offset=7)
        return m,x,y,row,dead,deleted
    def basis(self,m):return LpBasis.from_model(m,[B.BASIC,B.LOWER,None],[None,B.LOWER])
    def test_factories_copies_and_history(self):
        m,x,y,row,dead,deleted=self.model()
        with m,Session(self.library) as session,self.basis(m) as basis:
            self.assertEqual(basis.info.origin,Origin.CALLER)
            self.assertEqual(basis.columns,(B.BASIC,B.LOWER,None));self.assertEqual(basis.rows,(None,B.LOWER))
            self.assertEqual(basis.column(x),B.BASIC);self.assertEqual(basis.row(row),B.LOWER)
            with self.assertRaises(FrozenInstanceError):basis.info.origin=Origin.OBSERVATIONS
            for bad in (deleted,Row(row.model_id+1,row.slot,self.library)):
                with self.assertRaises(ApiError):basis.row(bad)
            with self.assertRaises(ApiError):basis.column(dead)
            with self.assertRaises(TypeError):basis.row(x)
            with session.solve_lp_with_basis(m,basis) as solved:
                self.assertEqual(solved.termination,T.OPTIMAL if self.available else T.UNSUPPORTED)
                self.assertEqual(solved.submission.state,S.ACCEPTED if self.available else S.NOT_ATTEMPTED)
                self.assertEqual(solved.submission.statuses_changed,False if self.available else None)
                copied=solved.copy_basis();observed=solved.copy_observed();ordinary=observed.copy_result()
                if self.available:
                    with LpBasis.from_observed(observed) as exported:
                        self.assertEqual(exported.info.origin,Origin.OBSERVATIONS)
                        with m.solve_lp_with_basis(exported) as second:
                            self.assertEqual(second.termination,T.OPTIMAL)
                else:
                    with self.assertRaises(ApiError):LpBasis.from_observed(observed)
            revision=copied.info.revision;m.set_bounds(row,5,math.inf)
            with session.solve_lp_with_basis(m,copied) as stale:
                self.assertEqual(stale.termination,T.INVALID_MODEL);self.assertFalse(stale.submission.backend_attempted)
                self.assertEqual(stale.info['requested_revision'],revision)
                with stale.copy_basis() as retained:self.assertEqual(retained.info.revision,revision)
            with session.solve_lp_observed(m) as fresh:
                if self.available:
                    with fresh.copy_result() as result:self.assertEqual(result.objective,17)
        with copied,observed,ordinary:
            self.assertEqual(copied.column(x),B.BASIC)
            self.assertEqual(copied.info.revision,revision)
            if self.available:self.assertEqual(ordinary.objective,15);self.assertEqual(ordinary.value(x),4)
        with self.assertRaises(RuntimeError):copied.column(x)
    def test_source_library_and_options(self):
        m,*_=self.model();other,*_=self.model()
        with m,other,self.basis(m) as basis,Session(self.library) as session:
            with other.solve_lp_with_basis(basis) as foreign:self.assertEqual(foreign.termination,T.INVALID_MODEL)
            for solve in (Options(backend=Backend.NATIVE),Options(guarantee=Guarantee.EXACT),Options(guarantee=Guarantee.CERTIFIED)):
                with m.solve_lp_with_basis(basis,LO(solve=solve)) as out:
                    self.assertEqual(out.termination,T.UNSUPPORTED);self.assertFalse(out.submission.backend_attempted)
            for invalid in (Options(),object()):
                with self.assertRaises(TypeError):m.solve_lp_with_basis(basis,invalid)
            with self.assertRaises(TypeError):m.solve_lp_with_basis(object())
            another=Library(self.library.path)
            with Model(another) as alien:
                with self.assertRaises(TypeError):alien.solve_lp_with_basis(basis)
                with self.assertRaises(TypeError):session.solve_lp_with_basis(alien,basis)
            with self.assertRaises(TypeError):LpBasis.from_observed(object())
    def test_counts_statuses_and_c_records(self):
        m,x,*_=self.model()
        with m:
            for columns,rows in (([],[]),([None,B.LOWER,None],[None,B.BASIC]),([B.BASIC,B.LOWER,B.LOWER],[None,B.LOWER]),([B.LOWER,B.LOWER,None],[None,B.LOWER])):
                with self.assertRaises(ApiError):LpBasis.from_model(m,columns,rows)
            for invalid in (True,0.5,object()):
                with self.assertRaises(TypeError):LpBasis.from_model(m,[invalid],[])
            for invalid in (-1,99):
                with self.assertRaises(ValueError):LpBasis.from_model(m,[invalid],[])
            with self.basis(m) as basis:
                for mutate in (lambda o:setattr(o,'struct_size',C.sizeof(o)-1),lambda o:setattr(o,'reserved',1),
                               lambda o:setattr(o,'basis',2),lambda o:setattr(o.solve,'reserved',1)):
                    options=LO()._marshal(self.library);mutate(options);out=b.U64(99)
                    with self.assertRaises(ApiError) as error:self.library.call('solve_lp_with_basis',m._open(),basis._open(),C.byref(options),C.byref(out))
                    self.assertEqual(error.exception.code,1);self.assertEqual(out.value,0)
                with self.assertRaises(ApiError):m.solve_lp_with_basis(basis,LO(solve=Options(primal_start=[(x,4)])))
                out=b._BasisInfo()
                with self.assertRaises(ApiError):self.library.call('basis_info',basis._open(),C.byref(out),C.sizeof(out)-1)
                values=(b.I32*2)(77,88);needed=b.U64()
                with self.assertRaises(ApiError) as error:self.library.call('basis_statuses',basis._open(),1,values,2,C.byref(needed))
                self.assertEqual(error.exception.code,6);self.assertEqual(tuple(values),(77,88));self.assertEqual(needed.value,3)
                with self.assertRaises(ApiError):self.library.call('basis_statuses',basis._open(),0,None,0,C.byref(needed))
    def test_cancellation_limits_and_reset(self):
        m,*_=self.model()
        with m,self.basis(m) as basis,Session(self.library) as session,Cancellation(self.library) as cancel:
            cancel.cancel()
            for options,expected in ((Options(time_limit_seconds=0),T.TIME_LIMIT),(Options(cancellation=cancel),T.CANCELLED)):
                with session.solve_lp_with_basis(m,basis,LO(solve=options)) as out:
                    self.assertEqual(out.termination,expected if self.available else T.UNSUPPORTED)
                    self.assertFalse(out.submission.backend_attempted)
                    with out.copy_observed() as observed:self.assertFalse(observed.has_solution)
            with session.solve_lp_with_basis(m,basis) as out:self.assertEqual(out.termination,T.OPTIMAL if self.available else T.UNSUPPORTED)
    def test_singular_repair_and_final_basis_distinction(self):
        with Model(self.library) as m:
            x=m.add_variable();y=m.add_variable();m.add_row([(x,1),(y,1)],1,1);m.add_row([(x,2),(y,2)],2,2)
            m.set_objective([(x,1),(y,2)])
            with LpBasis.from_model(m,[B.BASIC,B.BASIC],[B.LOWER,B.LOWER]) as basis,m.solve_lp_with_basis(basis) as out:
                self.assertEqual(out.submission.state,S.REPAIRED if self.available else S.NOT_ATTEMPTED)
                self.assertEqual(out.submission.statuses_changed,True if self.available else None)
                if self.available:
                    with out.copy_observed() as observed,observed.copy_result() as ordinary:self.assertEqual(ordinary.objective,1)
        m,*_=self.model()
        with m,LpBasis.from_model(m,[B.LOWER,B.LOWER,None],[None,B.BASIC]) as logical,m.solve_lp_with_basis(logical) as out:
            if self.available:
                self.assertEqual(out.submission.state,S.ACCEPTED);self.assertFalse(out.submission.statuses_changed)
                with out.copy_observed() as observed:self.assertEqual(observed.observations.columns[0].basis,B.BASIC)
    def test_requested_basis_survives_unrequested_export(self):
        m,*_=self.model()
        with m,self.basis(m) as basis,m.solve_lp_with_basis(basis,LO(basis=False,duals=False)) as out:
            with out.copy_basis() as copy:self.assertEqual(copy.columns,basis.columns)
            with out.copy_observed() as observed:
                with self.assertRaises(ApiError) as error:LpBasis.from_observed(observed)
                self.assertEqual(error.exception.code,3)
                with observed.copy_result() as result:
                    self.assertEqual(result.termination,T.OPTIMAL if self.available else T.UNSUPPORTED)

    def test_empty_and_zero_row_max(self):
        with Model(self.library) as m,LpBasis.from_model(m,[],[]) as basis,m.solve_lp_with_basis(basis) as out:
            self.assertEqual(basis.columns,());self.assertEqual(basis.rows,());self.assertFalse(out.submission.backend_attempted)
            self.assertEqual(out.termination,T.OPTIMAL if self.available else T.UNSUPPORTED)
        with Model(self.library) as m:
            x=m.add_variable(lower=0,upper=5);m.set_objective([(x,2)],maximize=True,offset=7)
            with LpBasis.from_model(m,[B.UPPER],[]) as basis,m.solve_lp_with_basis(basis) as out:
                if self.available:
                    with out.copy_observed() as observed,observed.copy_result() as ordinary:self.assertEqual(ordinary.objective,17)
        with Model(self.library) as m:
            m.add_variable(VariableType.INTEGER,0,1)
            with self.assertRaises(ApiError):LpBasis.from_model(m,[B.LOWER],[])

if __name__=='__main__':unittest.main()
