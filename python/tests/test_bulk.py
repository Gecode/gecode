"""Atomic bulk C ABI/Python conformance; no optional numerical dependencies."""
import ctypes as C
import gc
import math
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from gecode_optimize import (ApiError, Backend, Library, Model, Options, RowSpec,
                             SparseRowBatch, Termination, VariableSpec, VariableType, load_library)
from gecode_optimize import binding as B


class BulkConformance(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.lib=load_library()
        cls.highs=cls.lib.capabilities(Backend.HIGHS)["available"]

    def check_solution(self, result, optimum):
        self.assertEqual(result.termination, Termination.OPTIMAL if self.highs else Termination.UNSUPPORTED, result.message)
        if self.highs:
            self.assertAlmostEqual(result.objective,optimum)

    def test_scalar_equivalence_names_and_history(self):
        specs=[VariableSpec(VariableType.INTEGER,-3,7,"production π"),
               VariableSpec(upper=10,name="recourse 😀")]
        with Model(self.lib) as bulk, Model(self.lib) as scalar, tempfile.TemporaryDirectory() as directory:
            dead=bulk.add_variable();bulk.remove(dead)
            before=bulk.identity
            variables=bulk.add_variables(specs)
            self.assertEqual(bulk.identity,(before[0],before[1]+1))
            self.assertEqual([v.slot for v in variables],[1,2])
            sx=tuple(scalar.add_variable(s.type,s.lower,s.upper,s.name) for s in specs)
            # Reversed mapping; coefficients +1e16+1-1e16 coalesce to exactly 1.
            batch=SparseRowBatch([variables[1],variables[0]],[0,4,5],[1,1,1,0,0],
                                 [1e16,1,-1e16,1,1],[2.5,0],[math.inf,4],["demand λ",""])
            before=bulk.identity
            rows=bulk.add_rows_sparse(batch)
            self.assertEqual(bulk.identity,(before[0],before[1]+1))
            scalar.add_row({sx[0]:1,sx[1]:1},lower=2.5,name="demand λ")
            scalar.add_row({sx[1]:1},lower=0,upper=4)
            bulk.set_objective({variables[0]:2,variables[1]:3},offset=.25)
            scalar.set_objective({sx[0]:2,sx[1]:3},offset=.25)
            # Generated file comments retain exact UTF-8 names, independent of
            # input Python objects and temporary C-string/term arrays.
            del specs,batch;gc.collect()
            a=Path(directory)/'a.lp';b=Path(directory)/'b.lp';bulk.write(a);scalar.write(b)
            self.assertEqual(a.read_bytes(),b.read_bytes())
            with bulk.solve(Options(backend=Backend.HIGHS)) as historical, scalar.solve(Options(backend=Backend.HIGHS)) as cold:
                self.check_solution(historical,5.75);self.check_solution(cold,5.75)
                identity=bulk.identity
                more=bulk.add_rows([RowSpec({variables[0]:1},lower=4,name="edit")])
                self.assertEqual(bulk.identity,(identity[0],identity[1]+1))
                self.assertEqual(more[0].slot,rows[-1].slot+1)
                with bulk.solve(Options(backend=Backend.HIGHS)) as current:
                    self.check_solution(current,8.25)
                new=bulk.add_variables([VariableSpec(upper=1)])
                bulk.close()
                self.assertEqual((historical.info['model_id'],historical.info['revision']),identity)
                if self.highs:
                    self.assertEqual(historical.value(variables[0]),2)
                    with self.assertRaises(ApiError):historical.value(new[0])
                    with self.assertRaises(ApiError):historical.value(dead)

    def test_all_types_row_sequence_and_single_c_call(self):
        specs=[VariableSpec(t,lo,hi) for t,lo,hi in [
            (VariableType.CONTINUOUS,0,3),(VariableType.INTEGER,0,3),
            (VariableType.BINARY,0,None),(VariableType.SEMI_CONTINUOUS,2,5),
            (VariableType.SEMI_INTEGER,2.5,5)]]
        with Model(self.lib) as m:
            calls=[];original=self.lib.call
            def count_calls(name,*args):
                calls.append(name);return original(name,*args)
            with patch.object(self.lib,'call',count_calls):
                x=m.add_variables(specs)
            self.assertEqual(calls,['model_add_variables'])
            calls.clear();before=m.identity
            with patch.object(self.lib,'call',count_calls):
                rows=m.add_rows([RowSpec([(v,1)],lower=demand) for v,demand in zip(x,[1.5,1.5,.5,1,1])])
            self.assertEqual(calls,['model_add_rows']);self.assertEqual(m.identity[1],before[1]+1)
            m.set_objective([(v,1) for v in x])
            with m.solve(Options(backend=Backend.HIGHS)) as result:self.check_solution(result,9.5)
            calls.clear()
            with patch.object(self.lib,'call',count_calls):
                m.add_rows_sparse(SparseRowBatch(x,[0,5],list(range(5)),[1]*5,[0],[math.inf]))
            self.assertEqual(calls,['model_add_rows_sparse'])
            self.assertEqual(len(rows),5)

    def test_late_failures_are_atomic(self):
        with Model(self.lib) as m, Model(self.lib) as foreign:
            x=m.add_variable(upper=5);dead=m.add_variable();m.remove(dead);other=foreign.add_variable()
            before=m.identity
            for specs in ([VariableSpec(),VariableSpec(lower=math.nan)],
                          [VariableSpec(),VariableSpec(name='bad\0name')],
                          [VariableSpec(),VariableSpec(name='\ud800')],
                          [VariableSpec(),VariableSpec(type=999)],
                          [VariableSpec(),object()]):
                with self.subTest(specs=specs),self.assertRaises((ApiError,ValueError,TypeError)):
                    m.add_variables(specs)
                self.assertEqual(m.identity,before)
            for bad in (RowSpec({other:1}),RowSpec({dead:1}),RowSpec({x:math.inf}),
                        RowSpec({x:1},lower=2,upper=1),RowSpec(name='\0')):
                with self.subTest(bad=bad),self.assertRaises((ApiError,ValueError)):
                    m.add_rows([RowSpec({x:1}),bad])
                self.assertEqual(m.identity,before)
            fresh=m.add_variables([VariableSpec()])[0]
            self.assertEqual(fresh.slot,dead.slot+1)
            # Wrong library instances are rejected in Python before entering C.
            alias=Library(self.lib.path)
            with Model(alias) as other_model:
                alien=other_model.add_variable()
                before=m.identity
                with self.assertRaises(ValueError):m.add_rows([RowSpec({x:1}),RowSpec({alien:1})])
                with self.assertRaises(ValueError):m.add_rows_sparse(SparseRowBatch([alien]))
                self.assertEqual(m.identity,before)

    def test_csr_malformed_dimensions_indices_and_unused_columns(self):
        with Model(self.lib) as m, Model(self.lib) as foreign:
            x=m.add_variable();dead=m.add_variable();m.remove(dead);other=foreign.add_variable()
            bad=[SparseRowBatch([x],[],[],[],[],[]),
                 SparseRowBatch([x],[0,1],[0],[1],[0],[]),
                 SparseRowBatch([x],[0,1],[0],[],[0],[1]),
                 SparseRowBatch([x],[0,1],[0],[1],[0],[1],["a","b"]),
                 SparseRowBatch([x],[1,1],[0],[1],[0],[1]),
                 SparseRowBatch([x],[0,2,1],[0],[1],[0,0],[1,1]),
                 SparseRowBatch([x],[0,1],[1],[1],[0],[1]),
                 SparseRowBatch([x],[0,1],[0],[math.nan],[0],[1]),
                 SparseRowBatch([x,x]),SparseRowBatch([dead]),SparseRowBatch([other])]
            before=m.identity
            for batch in bad:
                with self.subTest(batch=batch),self.assertRaises(ApiError):m.add_rows_sparse(batch)
                self.assertEqual(m.identity,before)
            for value in (-1,2**64,True,1.0):
                with self.subTest(value=value),self.assertRaises((OverflowError,TypeError)):
                    m.add_rows_sparse(SparseRowBatch([x],[0,1],[value],[1],[0],[1]))
                self.assertEqual(m.identity,before)
            for batch in (SparseRowBatch(),SparseRowBatch([x])):
                self.assertEqual(m.add_rows_sparse(batch),())
            self.assertEqual(m.add_variables([]),());self.assertEqual(m.add_rows([]),())
            self.assertEqual(m.identity,before)
            m.close()
            with self.assertRaises(RuntimeError):m.add_variables([])
            with self.assertRaises(RuntimeError):m.add_rows([])
            with self.assertRaises(RuntimeError):m.add_rows_sparse(SparseRowBatch())

    def test_sparse_many_nonzeros_and_constant_contradiction(self):
        # Large enough to catch scalar dispatch but small enough for ordinary CI.
        with Model(self.lib) as m:
            x=m.add_variables([VariableSpec(upper=2)]*1024)
            calls=[];original=self.lib.call
            def record(name,*args):calls.append(name);return original(name,*args)
            with patch.object(self.lib,'call',record):
                rows=m.add_rows_sparse(SparseRowBatch(x,list(range(1025)),list(range(1024)),
                                                      [1]*1024,[1]*1024,[2]*1024))
            self.assertEqual(calls,['model_add_rows_sparse']);self.assertEqual(len(rows),1024)
            # Empty contradictory row must not disappear during batch conversion.
            m.add_rows([RowSpec(lower=1)])
            with m.solve(Options(backend=Backend.HIGHS)) as result:
                self.assertEqual(result.termination,Termination.INFEASIBLE if self.highs else Termination.UNSUPPORTED)

    def test_c_name_copy_and_output_preflight(self):
        with Model(self.lib) as m, tempfile.TemporaryDirectory() as directory:
            name=C.create_string_buffer('original π'.encode())
            spec=B._VariableSpec(C.sizeof(B._VariableSpec),int(VariableType.CONTINUOUS),0,0,1,C.cast(name,C.c_char_p))
            before=m.identity;out=(B._Id*1)();out[0].slot=123
            for capacity,output,error in ((0,out,6),(0,None,6),(1,None,1)):
                with self.assertRaises(ApiError) as failure:
                    self.lib.call('model_add_variables',m._open(),C.byref(spec),1,output,capacity)
                self.assertEqual(failure.exception.code,error);self.assertEqual(out[0].slot,123)
                self.assertEqual(m.identity,before)
            self.lib.call('model_add_variables',m._open(),C.byref(spec),1,out,1)
            name.value=b'changed';del name,spec;gc.collect()
            file=Path(directory)/'names.lp';m.write(file)
            self.assertIn('original π'.encode().hex(),file.read_text())
