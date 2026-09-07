"""Independent finite-word oracles for typed Regular C/Python semantics."""
import ctypes as C
from dataclasses import FrozenInstanceError
from itertools import product
import math
import unittest
from gecode_optimize import (ApiError,Backend,Guarantee,Library,Model,Options,
    RegularTransition as Edge,Session,Termination as T,VariableType as V,load_library)
from gecode_optimize import binding as b


def language(length,initial,edges,finals):
    # Enumerate paths first, independently of a proposed assignment's transition lookup.
    frontier={(initial,())}
    for _ in range(length):
        frontier={(e.to_state,word+(e.symbol,)) for state,word in frontier for e in edges if e.from_state==state}
    return {word for state,word in frontier if state in finals}

class RegularTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.library=load_library();cls.native=cls.library.capabilities(Backend.NATIVE)['available']
    def test_automaton_word_oracle_both_senses_aliases_and_routing(self):
        cases=[(2,0,[Edge(0,-1,0),Edge(0,1,1),Edge(1,-1,1),Edge(1,1,0)],[0,0]),
               (3,0,[Edge(0,-1,1),Edge(1,1,2)],[2]),
               (1,0,[],[0]),(1,0,[],[]),
               ((1<<64)-1,(1<<64)-2,[Edge((1<<64)-2,-1,(1<<64)-2)],[(1<<64)-2])]
        for states,initial,edges,finals in cases:
            for positions in ((),(0,),(0,1),(0,1,0)):
                for maximize in (False,True):
                    with self.subTest(states=states,positions=positions,maximize=maximize),Model(self.library) as m:
                        variables=[m.add_variable(V.INTEGER,-1,1) for _ in range(max(positions,default=-1)+1)]
                        word=[variables[i] for i in positions]
                        m.add_regular(word,states,initial,edges,finals)
                        coefficients=[2,-3][:len(variables)]
                        m.set_objective(list(zip(variables,coefficients)),maximize=maximize,offset=-4)
                        accepted=language(len(word),initial,edges,finals)
                        candidates=[(sum(c*x for c,x in zip(coefficients,values))-4,values)
                            for values in product((-1,0,1),repeat=len(variables)) if tuple(values[i] for i in positions) in accepted]
                        options=Options(backend=Backend.AUTO,guarantee=Guarantee.EXACT)
                        with m.solve(options) as out:
                            self.assertEqual(out.termination,(T.OPTIMAL if candidates else T.INFEASIBLE) if self.native else T.UNSUPPORTED)
                            if candidates and self.native:
                                best=(max if maximize else min)(value for value,_ in candidates)
                                self.assertEqual(out.objective,best);self.assertEqual(out.best_bound,best)
                                actual=tuple(out.value(v) for v in variables)
                                self.assertIn((best,actual),candidates)
                        with m.solve(Options(backend=Backend.HIGHS)) as out:
                            self.assertEqual(out.termination,T.UNSUPPORTED);self.assertFalse(out.has_solution)
    def test_mixed_globals_rows_removal_and_historical_result(self):
        edges=[Edge(0,-1,0),Edge(0,1,1),Edge(1,-1,1),Edge(1,1,0)]
        for maximize,expected in ((False,7),(True,13)):
            with Model(self.library) as m:
                x,y,z=[m.add_variable(V.INTEGER,-1,1) for _ in range(3)]
                regular=m.add_regular([x,y,z],2,0,edges,[0,0],name='even π')
                m.add_all_different([x,y]);m.add_row([(x,1),(y,1),(z,1)],lower=-1)
                m.set_objective([(x,2),(y,-1),(z,3)],maximize=maximize,offset=7)
                before=m.identity[1];m.set_name(regular,'renamed');self.assertEqual(m.identity[1],before+1)
                with self.assertRaises(ApiError):m.remove(z)
                result=m.solve(Options(backend=Backend.NATIVE,guarantee=Guarantee.EXACT))
                with Session(self.library) as session,session.solve(m) as unsupported:self.assertEqual(unsupported.termination,T.UNSUPPORTED)
                m.remove(regular)
                with self.assertRaises(ApiError):m.remove(regular)
            with result:
                if self.native:
                    self.assertEqual(result.termination,T.OPTIMAL);self.assertEqual(result.objective,expected)
                    self.assertEqual((result.value(x),result.value(y),result.value(z)),(-1,1,1) if not maximize else (1,-1,1))
                else:self.assertEqual(result.termination,T.UNSUPPORTED)
    def test_atomic_invalid_model_data_and_python_integer_checks(self):
        with Model(self.library) as m,Model(self.library) as other:
            x=m.add_variable(V.INTEGER,-1,1);foreign=other.add_variable(V.INTEGER,-1,1)
            dead=m.add_variable(V.INTEGER,-1,1);m.remove(dead);continuous=m.add_variable()
            valid=[Edge(0,-1,1)]
            invalid=[([foreign],2,0,valid,[1]),([dead],2,0,valid,[1]),([continuous],2,0,valid,[1]),
                     ([x],0,0,[],[]),([x],2,2,[],[]),([x],2,0,valid,[2]),
                     ([x],2,0,[Edge(2,-1,1)],[1]),([x],2,0,[Edge(0,-1,2)],[1]),
                     ([x],2,0,valid+valid,[1]),([x],2,0,valid+[Edge(0,-1,0)],[1]),
                     ([x],2,0,[Edge(0,(1<<53)+1,1)],[1])]
            revision=m.identity[1]
            for args in invalid:
                with self.assertRaises(ApiError):m.add_regular(*args)
                self.assertEqual(m.identity[1],revision)
            for value in (True,0.5,'1'):
                with self.assertRaises(TypeError):m.add_regular([x],value,0,valid,[1])
                with self.assertRaises(TypeError):m.add_regular([x],2,0,[Edge(0,value,1)],[1])
            for value in (-1,1<<64):
                with self.assertRaises(OverflowError):m.add_regular([x],value,0,valid,[1])
            with self.assertRaises(TypeError):m.add_regular([x],2,0,[(0,-1,1)],[1])
            with self.assertRaises(FrozenInstanceError):valid[0].symbol=1
            with self.assertRaises(ValueError):m.add_regular([x],2,0,valid,[1],name='bad\0name')
            another=Library(self.library.path)
            with Model(another) as foreign_library:
                with self.assertRaises(ValueError):foreign_library.add_regular([x],2,0,valid,[1])
            g=m.add_regular([x],2,0,valid,[1]);self.assertEqual(m.identity[1],revision+1);m.remove(g)
    def test_c_layout_size_reserved_counts_and_no_partial_mutation(self):
        with Model(self.library) as m:
            x=m.add_variable(V.INTEGER,-1,1);ids=(b._Id*1)(x._id());edges=(b._RegularTransition*2)()
            for e in edges:e.struct_size=C.sizeof(e);e.from_state=0;e.symbol=-1;e.to_state=1
            edges[1].symbol=1;finals=(b.U64*1)(1);revision=m.identity[1]
            def call(variables=ids,n=1,transitions=edges,count=2,size=C.sizeof(b._RegularTransition),states=2,final=finals,nfinal=1,name=b'ok'):
                out=b._Id(99,99,99,99)
                try:self.library.call('model_add_regular',m._open(),variables,n,states,0,transitions,count,size,final,nfinal,name,C.byref(out))
                finally:
                    self.assertEqual(bytes(out),bytes(b._Id()));self.assertEqual(m.identity[1],revision)
            for kwargs in ({'size':C.sizeof(b._RegularTransition)-1},{'variables':None}, {'n':(1<<64)-1},
                           {'transitions':None},{'count':(1<<64)-1},{'final':None},{'nfinal':(1<<64)-1},{'name':b'\xc0\x80'}):
                with self.assertRaises(ApiError) as error:call(**kwargs)
                self.assertEqual(error.exception.code,1)
            for field,value in (('struct_size',0),('reserved',1)):
                original=getattr(edges[1],field);setattr(edges[1],field,value)
                with self.assertRaises(ApiError) as error:call()
                self.assertEqual(error.exception.code,1);setattr(edges[1],field,original)
            edges[1].to_state=2
            with self.assertRaises(ApiError) as error:call()
            self.assertEqual(error.exception.code,3)
            # No edge arrays are required for an empty deterministic language.
            g=m.add_regular([],1,0,[],[0]);m.remove(g)
    def test_exact_symbol_endpoints_and_semicontinuous_scope(self):
        with Model(self.library) as m:
            x=m.add_variable(V.INTEGER,-1,1)
            for symbol in (-(1<<53),1<<53):
                g=m.add_regular([x],1,0,[Edge(0,symbol,0)],[0]);m.remove(g)
        with Model(self.library) as m:
            x=m.add_variable(V.SEMI_INTEGER,2,3);m.add_regular([x],1,0,[Edge(0,0,0),Edge(0,3,0)],[0]);m.set_objective([(x,1)],maximize=True,offset=-2)
            with m.solve(Options(backend=Backend.AUTO,guarantee=Guarantee.EXACT)) as out:
                self.assertEqual(out.termination,T.OPTIMAL if self.native else T.UNSUPPORTED)
                if self.native:self.assertEqual(out.objective,1);self.assertEqual(out.value(x),3)

if __name__=='__main__':unittest.main()
