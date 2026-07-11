/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Zayenz Lagerkvist, 2007
 *     Christian Schulte, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "test/int.hh"

#include <gecode/minimodel.hh>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>

namespace Test { namespace Int {

   /// %Tests for extensional (relation) constraints
   namespace Extensional {

     /**
      * \defgroup TaskTestIntExtensional Extensional (relation) constraints
      * \ingroup TaskTestInt
      */
     //@{
     std::string
     extensional_kind_name(Gecode::ExtensionalPropKind epk) {
       switch (epk) {
       case Gecode::EPK_DENSE:
         return "Dense";
       case Gecode::EPK_SPARSE:
         return "Sparse";
       case Gecode::EPK_DENSE_COMPRESSED:
         return "DenseCompressed";
       case Gecode::EPK_AUTO:
         return "Auto";
       default:
         GECODE_NEVER;
         return "Unknown";
       }
     }

     /// %Test with simple regular expression
     class RegSimpleA : public Test {
     public:
       /// Create and register test
       RegSimpleA(void) : Test("Extensional::Reg::Simple::A",4,2,2) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     (REG(0) | REG(2)) +
                     (REG(-1) | REG(1)) +
                     (REG(7) | REG(0) | REG(1)) +
                     (REG(0) | REG(1)));
       }
     };

     /// %Test with simple regular expression
     class RegSimpleB : public Test {
     public:
       /// Create and register test
       RegSimpleB(void) : Test("Extensional::Reg::Simple::B",4,2,2) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]<x[1]) && (x[1]<x[2]) && (x[2]<x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     (REG(-2) + REG(-1) + REG(0) + REG(1)) |
                     (REG(-2) + REG(-1) + REG(0) + REG(2)) |
                     (REG(-2) + REG(-1) + REG(1) + REG(2)) |
                     (REG(-2) + REG(0) + REG(1) + REG(2)) |
                     (REG(-1) + REG(0) + REG(1) + REG(2)));
         }
     };

     /// %Test with simple regular expression
     class RegSimpleC : public Test {
     public:
       /// Create and register test
       RegSimpleC(void) : Test("Extensional::Reg::Simple::C",6,0,1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int pos = 0;
         int s = x.size();

         while (pos < s && x[pos] == 0) ++pos;
         if (pos + 4 > s) return false;

         for (int i = 0; i < 2; ++i, ++pos)
           if (x[pos] != 1) return false;
         if (pos + 2 > s) return false;

         for (int i = 0; i < 1; ++i, ++pos)
           if (x[pos] != 0) return false;
         while (pos < s && x[pos] == 0) ++pos;
         if (pos + 1 > s) return false;

         for (int i = 0; i < 1; ++i, ++pos)
           if (x[pos] != 1) return false;
         while (pos < s) if (x[pos++] != 0) return false;
         return true;

       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     *REG(0) + REG(1)(2,2) + +REG(0) + REG(1)(1,1) + *REG(0));
       }
     };

     /// %Test with regular expression for distinct constraint
     class RegDistinct : public Test {
     public:
       /// Create and register test
       RegDistinct(void) : Test("Extensional::Reg::Distinct",4,-1,4) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++) {
           if ((x[i] < 0) || (x[i] > 3))
             return false;
           for (int j=i+1; j<x.size(); j++)
             if (x[i]==x[j])
               return false;
         }
         return true;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     (REG(0)+REG(1)+REG(2)+REG(3)) |
                     (REG(0)+REG(1)+REG(3)+REG(2)) |
                     (REG(0)+REG(2)+REG(1)+REG(3)) |
                     (REG(0)+REG(2)+REG(3)+REG(1)) |
                     (REG(0)+REG(3)+REG(1)+REG(2)) |
                     (REG(0)+REG(3)+REG(2)+REG(1)) |
                     (REG(1)+REG(0)+REG(2)+REG(3)) |
                     (REG(1)+REG(0)+REG(3)+REG(2)) |
                     (REG(1)+REG(2)+REG(0)+REG(3)) |
                     (REG(1)+REG(2)+REG(3)+REG(0)) |
                     (REG(1)+REG(3)+REG(0)+REG(2)) |
                     (REG(1)+REG(3)+REG(2)+REG(0)) |
                     (REG(2)+REG(0)+REG(1)+REG(3)) |
                     (REG(2)+REG(0)+REG(3)+REG(1)) |
                     (REG(2)+REG(1)+REG(0)+REG(3)) |
                     (REG(2)+REG(1)+REG(3)+REG(0)) |
                     (REG(2)+REG(3)+REG(0)+REG(1)) |
                     (REG(2)+REG(3)+REG(1)+REG(0)) |
                     (REG(3)+REG(0)+REG(1)+REG(2)) |
                     (REG(3)+REG(0)+REG(2)+REG(1)) |
                     (REG(3)+REG(1)+REG(0)+REG(2)) |
                     (REG(3)+REG(1)+REG(2)+REG(0)) |
                     (REG(3)+REG(2)+REG(0)+REG(1)) |
                     (REG(3)+REG(2)+REG(1)+REG(0)));
       }
     };

     /// %Test with simple regular expression from Roland Yap
     class RegRoland : public Test {
     public:
       /// Create and register test
       RegRoland(int n)
         : Test("Extensional::Reg::Roland::"+str(n),n,0,1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n = x.size();
         return
           ((n > 1) && (x[n-2] == 0)) ||
           ((n > 0) && (x[n-1] == 0));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         REG r0(0), r1(1);
         REG r01 = r0 | r1;
         extensional(home, x, *r01 + r0 + r01(0,1));
       }
     };

     /// %Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedA : public Test {
     public:
       /// Create and register test
       RegSharedA(void) : Test("Extensional::Reg::Shared::A",4,2,2) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVarArgs y(8);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=x[i];
         unshare(home,y);
         extensional(home, y,
                     ((REG(0) | REG(2)) +
                      (REG(-1) | REG(1)) +
                      (REG(7) | REG(0) | REG(1)) +
                      (REG(0) | REG(1)))(2,2));
       }
     };

     /// %Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedB : public Test {
     public:
       /// Create and register test
       RegSharedB(void) : Test("Extensional::Reg::Shared::B",4,2,2) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVarArgs y(12);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=y[i+8]=x[i];
         unshare(home,y);
         extensional(home, y,
                     ((REG(0) | REG(2)) +
                      (REG(-1) | REG(1)) +
                      (REG(7) | REG(0) | REG(1)) +
                      (REG(0) | REG(1)))(3,3));
       }
     };

     /// %Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedC : public Test {
     public:
       /// Create and register test
       RegSharedC(void) : Test("Extensional::Reg::Shared::C",4,0,1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1]==1) && (x[2]==0) && (x[3]==1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolVarArgs y(8);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=channel(home,x[i]);
         unshare(home,y);
         extensional(home,y,
                     ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(2,2));
       }
     };

     /// %Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedD : public Test {
     public:
       /// Create and register test
       RegSharedD(void) : Test("Extensional::Reg::Shared::D",4,0,1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1]==1) && (x[2]==0) && (x[3]==1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolVarArgs y(12);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=y[i+8]=channel(home,x[i]);
         unshare(home, y);
         extensional(home, y,
                     ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(3,3));
       }
     };

     /// %Test for empty DFA
     class RegEmptyDFA : public Test {
     public:
       /// Create and register test
       RegEmptyDFA(void) : Test("Extensional::Reg::Empty::DFA",1,0,0) {
         testsearch = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         (void)x;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::DFA d;
         Gecode::extensional(home, x, d);
       }
     };

     /// %Test for empty regular expression
     class RegEmptyREG : public Test {
     public:
       /// Create and register test
       RegEmptyREG(void) : Test("Extensional::Reg::Empty::REG",1,0,0) {
         testsearch = false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         (void)x;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::REG r;
         Gecode::extensional(home, x, r);
       }
     };

     /// %Test for optimizations
     class RegOpt : public Test {
     protected:
       /// DFA size characteristic
       int n;
     public:
       /// Create and register test
       RegOpt(int n0)
         : Test("Extensional::Reg::Opt::"+str(n0),1,0,15), n(n0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0] < n) && ((x[0] & 1) == 0);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         DFA::Transition* t = new DFA::Transition[n+1];
         DFA::Transition* ti = t;
         int* f = new int[n+1];
         int* fi = f;
         for (int i=0; i<n; i++) {
           ti->i_state = 0;
           ti->symbol  = i;
           ti->o_state = i+1;
           ti++;
           if ((i & 1) == 0) {
             *fi = i+1; fi++;
           }
         }
         ti->i_state = -1;
         *fi = -1;
         DFA d(0, t, f, false);
         delete [] t;
         delete [] f;
         extensional(home, x, d);
       }

     };

     ///% Transform a TupleSet into a DFA
     Gecode::DFA tupleset2dfa(Gecode::TupleSet ts) {
       using namespace Gecode;
       REG expression;
       for (int i = 0; i<ts.tuples(); i++) {
         REG r;
         for (int j = 0; j<ts.arity(); j++) {
           r += REG(ts[i][j]);
         }
         expression |= r;
       }
       DFA dfa(expression);
       return dfa;
     }

     /// %Test with tuple set
     class TupleSetBase : public Test {
     protected:
       /// Simple test tupleset
       Gecode::TupleSet t;
       /// Whether the table is positive or negative
       bool pos;
       /// Support representation
       Gecode::ExtensionalPropKind epk;
     public:
       /// Create and register test
       TupleSetBase(bool p, Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::" + extensional_kind_name(epk0) +
                "::" + str(p) + "::Base",
                4,1,5,true,Gecode::IPL_DOM),
           t(4), pos(p), epk(epk0) {
         using namespace Gecode;
         IntArgs t1({2, 1, 2, 4});
         IntArgs t2({2, 2, 1, 4});
         IntArgs t3({4, 3, 4, 1});
         IntArgs t4({1, 3, 2, 3});
         IntArgs t5({3, 3, 3, 2});
         t.add(t1).add(t1).add(t2).add(t2)
          .add(t3).add(t3).add(t4).add(t4)
          .add(t5).add(t5).add(t5).add(t5)
          .add(t5).add(t5).add(t5).add(t5)
          .add(t1).add(t1).add(t2).add(t2)
          .add(t3).add(t3).add(t4).add(t4)
          .add(t5).add(t5).add(t5).add(t5)
          .add(t5).add(t5).add(t5).add(t5)
          .finalize(epk);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return pos == ((x[0] == 1 && x[1] == 3 && x[2] == 2 && x[3] == 3) ||
                        (x[0] == 2 && x[1] == 1 && x[2] == 2 && x[3] == 4) ||
                        (x[0] == 2 && x[1] == 2 && x[2] == 1 && x[3] == 4) ||
                        (x[0] == 3 && x[1] == 3 && x[2] == 3 && x[3] == 2) ||
                        (x[0] == 4 && x[1] == 3 && x[2] == 4 && x[3] == 1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         TupleSet ts = TupleSet(t.arity(),tupleset2dfa(t),epk);
         assert(t == ts);
         assert(ts.representation() == epk);
         extensional(home, x, t, pos, ipl);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         extensional(home, x, t, pos, r, ipl);
       }
     };

     /// %Test with tuple set
     class TupleSetTest : public Test {
     protected:
       /// Whether the table is positive or negative
       bool pos;
       /// Support representation
       Gecode::ExtensionalPropKind epk;
       /// The tuple set to use
       Gecode::TupleSet ts;
       /// Whether to validate dfa2tupleset
       bool toDFA;
     public:
       /// Create and register test
       TupleSetTest(const std::string& s, bool p,
                    Gecode::IntSet d0, Gecode::TupleSet ts0, bool td,
                    Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::" + extensional_kind_name(epk0) +
                "::" + str(p) + "::" + s,
                ts0.arity(),d0,true,Gecode::IPL_DOM),
           pos(p), epk(epk0), ts(ts0), toDFA(td) {
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         using namespace Gecode;
         for (int i=ts.tuples(); i--; ) {
           TupleSet::Tuple t = ts[i];
           bool same = true;
           for (int j=0; (j < ts.arity()) && same; j++)
             if (t[j] != x[j])
               same = false;
           if (same)
             return pos;
         }
         return !pos;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (toDFA) {
           TupleSet t = TupleSet(ts.arity(),tupleset2dfa(ts),epk);
           assert(ts == t);
           assert(t.representation() == epk);
         }
         extensional(home, x, ts, pos, ipl);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         using namespace Gecode;
         extensional(home, x, ts, pos, r, ipl);
       }
     };

     class RandomTupleSetTest : public TupleSetTest {
     public:
       /// Create and register test
       RandomTupleSetTest(const std::string& s, bool p,
                          Gecode::IntSet d0, Gecode::TupleSet ts0,
                          Gecode::ExtensionalPropKind epk0)
         : TupleSetTest(s,p,d0,ts0,false,epk0) {
         testsearch = false;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         using namespace Gecode;
         return new RandomAssignment(arity, dom, 1000, _rand);
       }
     };

     /// Sparse smoke test for very low-density unary tuple sets
     class SparseTupleSetUnary : public ::Test::Base {
     public:
       SparseTupleSetUnary(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::Unary") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int n = 2000;
         TupleSet ts(1);
         for (int i=0; i<n; i++)
           ts.add(IntArgs({i}));
         ts.finalize(EPK_SPARSE);

         if (ts.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: TupleSet did not select sparse support"
                     << std::endl;
           return false;
         }

         class SparseUnarySpace : public Space {
         public:
           IntVarArray x;
           SparseUnarySpace(const TupleSet& t, int n0)
             : x(*this,1,0,n0-1) {
             extensional(*this, x, t, true, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseUnarySpace(SparseUnarySpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseUnarySpace(*this);
           }
         };

         SparseUnarySpace* root = new SparseUnarySpace(ts,n);
         DFS<SparseUnarySpace> e(root);
         delete root;

         SparseUnarySpace* sol = e.next();
         if (sol == nullptr)
           return false;
         const bool ok = sol->x[0].assigned() && (sol->x[0].val() == 0);
         delete sol;
         return ok;
       }
     };

     /// Sparse smoke test for low-density ternary tuple sets
     class SparseTupleSetTernary : public ::Test::Base {
     public:
       SparseTupleSetTernary(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::Ternary") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int n = 1500;
         TupleSet ts(3);
         for (int i=0; i<n; i++)
           ts.add(IntArgs({i, (i*7) % n, (i*11) % n}));
         ts.finalize(EPK_SPARSE);

         if (ts.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: Ternary TupleSet did not select sparse support"
                     << std::endl;
           return false;
         }

         class SparseTernarySpace : public Space {
         public:
           IntVarArray x;
           SparseTernarySpace(const TupleSet& t, int n0)
             : x(*this,3,0,n0-1) {
             extensional(*this, x, t, true, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseTernarySpace(SparseTernarySpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseTernarySpace(*this);
           }
         };

         SparseTernarySpace* root = new SparseTernarySpace(ts,n);
         DFS<SparseTernarySpace> e(root);
         delete root;

         SparseTernarySpace* sol = e.next();
         if (sol == nullptr)
           return false;
         const int a = sol->x[0].val();
         const int b = sol->x[1].val();
         const int c = sol->x[2].val();
         delete sol;
         return (b == ((a*7) % n)) && (c == ((a*11) % n));
       }
     };

     /// Sparse smoke test for low-density higher-arity tuple sets
     class SparseTupleSetHighArity : public ::Test::Base {
     public:
       SparseTupleSetHighArity(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::HighArity") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int n = 1000;
         TupleSet ts(6);
         for (int i=0; i<n; i++)
           ts.add(IntArgs({i, (i*3) % n, (i*5) % n,
                           (i*7) % n, (i*11) % n, (i*13) % n}));
         ts.finalize(EPK_SPARSE);

         if (ts.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: High-arity TupleSet did not select sparse support"
                     << std::endl;
           return false;
         }

         class SparseHighAritySpace : public Space {
         public:
           IntVarArray x;
           SparseHighAritySpace(const TupleSet& t, int n0)
             : x(*this,6,0,n0-1) {
             extensional(*this, x, t, true, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseHighAritySpace(SparseHighAritySpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseHighAritySpace(*this);
           }
         };

         SparseHighAritySpace* root = new SparseHighAritySpace(ts,n);
         DFS<SparseHighAritySpace> e(root);
         delete root;

         SparseHighAritySpace* sol = e.next();
         if (sol == nullptr)
           return false;
         const int a = sol->x[0].val();
         const int b = sol->x[1].val();
         const int c = sol->x[2].val();
         const int d = sol->x[3].val();
         const int e0 = sol->x[4].val();
         const int f = sol->x[5].val();
         delete sol;
         return (b == ((a*3) % n)) &&
                (c == ((a*5) % n)) &&
                (d == ((a*7) % n)) &&
                (e0 == ((a*11) % n)) &&
                (f == ((a*13) % n));
       }
     };

     /// Sparse smoke test for nullary tuple sets
     class SparseTupleSetNullary : public ::Test::Base {
     public:
       SparseTupleSetNullary(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::Nullary") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseNullarySpace : public Space {
         public:
           IntVarArray x;
           SparseNullarySpace(const TupleSet& t)
             : x(*this,0,0,0) {
             extensional(*this, x, t, true, IPL_DOM);
           }
           SparseNullarySpace(SparseNullarySpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseNullarySpace(*this);
           }
         };

         TupleSet sat(0);
         sat.add(IntArgs(0));
         sat.finalize(EPK_SPARSE);
         if (sat.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: Nullary sat table not sparse" << std::endl;
           return false;
         }
         SparseNullarySpace* sat_root = new SparseNullarySpace(sat);
         DFS<SparseNullarySpace> sat_engine(sat_root);
         delete sat_root;
         SparseNullarySpace* sat_sol = sat_engine.next();
         if (sat_sol == nullptr) {
           std::cerr << "ERROR: Nullary sat table produced no solution"
                     << std::endl;
           return false;
         }
         delete sat_sol;

         TupleSet unsat(0);
         unsat.finalize(EPK_SPARSE);
         SparseNullarySpace* unsat_root = new SparseNullarySpace(unsat);
         DFS<SparseNullarySpace> unsat_engine(unsat_root);
         delete unsat_root;
         SparseNullarySpace* unsat_sol = unsat_engine.next();
         const bool ok = (unsat_sol == nullptr);
         if (!ok)
           std::cerr << "ERROR: Nullary empty table unexpectedly satisfiable"
                     << std::endl;
         delete unsat_sol;
         return ok;
       }
     };

     /// Sparse incremental smoke test for repeated and mixed delta updates
     class SparseTupleSetIncrementalDelta : public ::Test::Base {
     public:
       SparseTupleSetIncrementalDelta(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::IncrementalDelta") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseDeltaSpace : public Space {
         public:
           IntVarArray x;
           SparseDeltaSpace(const TupleSet& t)
             : x(*this,2,0,3) {
             extensional(*this, x, t, true, IPL_DOM);
             rel(*this, x[0], IRT_NQ, 0);
             rel(*this, x[0], IRT_NQ, 1);
             rel(*this, x[1], IRT_NQ, 3);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseDeltaSpace(SparseDeltaSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseDeltaSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({1,1}))
           .add(IntArgs({2,2})).add(IntArgs({3,3}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;

         SparseDeltaSpace* root = new SparseDeltaSpace(ts);
         DFS<SparseDeltaSpace> e(root);
         delete root;

         SparseDeltaSpace* sol = e.next();
         if (sol == nullptr)
           return false;
         SparseDeltaSpace* extra = e.next();
         const bool ok = sol->x[0].assigned() && sol->x[1].assigned() &&
                         (sol->x[0].val() == 2) && (sol->x[1].val() == 2) &&
                         (extra == nullptr);
         delete sol;
         delete extra;
         return ok;
       }
     };

     /// Sparse incremental test for assigned-variable advisor updates
     class SparseTupleSetIncrementalAssign : public ::Test::Base {
     public:
       SparseTupleSetIncrementalAssign(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::IncrementalAssign") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseAssignSpace : public Space {
         public:
           IntVarArray x;
           SparseAssignSpace(const TupleSet& t)
             : x(*this,2,0,3) {
             extensional(*this, x, t, true, IPL_DOM);
             rel(*this, x[0], IRT_EQ, 2);
             rel(*this, x[1], IRT_NQ, 1);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseAssignSpace(SparseAssignSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseAssignSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({1,1}))
           .add(IntArgs({2,1})).add(IntArgs({3,3}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;

         SparseAssignSpace* root = new SparseAssignSpace(ts);
         DFS<SparseAssignSpace> e(root);
         delete root;
         SparseAssignSpace* sol = e.next();
         const bool ok = (sol == nullptr);
         delete sol;
         return ok;
       }
     };

     /// Sparse incremental test for BoolView specialization
     class SparseTupleSetIncrementalBool : public ::Test::Base {
     public:
       SparseTupleSetIncrementalBool(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::IncrementalBool") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseBoolSpace : public Space {
         public:
           BoolVarArray x;
           SparseBoolSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this, x, t, true, IPL_DOM);
             rel(*this, x[0], IRT_NQ, 0);
             branch(*this, x, BOOL_VAR_NONE(), BOOL_VAL_MIN());
           }
           SparseBoolSpace(SparseBoolSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseBoolSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,1})).add(IntArgs({1,0}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;

         SparseBoolSpace* root = new SparseBoolSpace(ts);
         DFS<SparseBoolSpace> e(root);
         delete root;

         SparseBoolSpace* sol = e.next();
         if (sol == nullptr)
           return false;
         SparseBoolSpace* extra = e.next();
         const bool ok = sol->x[0].assigned() && sol->x[1].assigned() &&
                         (sol->x[0].val() == 1) && (sol->x[1].val() == 0) &&
                         (extra == nullptr);
         delete sol;
         delete extra;
         return ok;
       }
     };

     /// Disabled sparse propagators must defer failure until re-enabled
     class SparseTupleSetDisabledFailure : public ::Test::Base {
     public:
       SparseTupleSetDisabledFailure(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::DisabledFailure") {}

       virtual bool run(void) {
         using namespace Gecode;

         class DisabledSpace : public Space {
         public:
           IntVarArray x;
           DisabledSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this,x,t,true,IPL_DOM);
           }
           DisabledSpace(DisabledSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new DisabledSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({1,1}));
         ts.finalize(EPK_SPARSE);

         DisabledSpace* s = new DisabledSpace(ts);
         if (s->status() == SS_FAILED) {
           delete s;
           return false;
         }
         PropagatorGroup::all.disable(*s);
         rel(*s,s->x[0],IRT_EQ,0);
         rel(*s,s->x[1],IRT_EQ,1);
         if (s->status() == SS_FAILED) {
           delete s;
           return false;
         }

         PropagatorGroup::all.enable(*s);
         const bool failed = (s->status() == SS_FAILED);
         delete s;
         return failed;
       }
     };

     /// Sparse delta processing must depend on supports, not numeric width
     class SparseTupleSetWideDelta : public ::Test::Base {
     public:
       SparseTupleSetWideDelta(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::WideDelta") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int gap = 1000000000;
         const int lower = gap / 2;
         TupleSet ts(1);
         ts.add(IntArgs({0})).add(IntArgs({gap}));
         ts.finalize(EPK_SPARSE);

         class NegativeSpace : public Space {
         public:
           IntVar x;
           NegativeSpace(const TupleSet& t, int gap0, int lower0)
             : x(*this,0,gap0) {
             extensional(*this,IntVarArgs({x}),t,false,IPL_DOM);
             rel(*this,x,IRT_GQ,lower0);
           }
           NegativeSpace(NegativeSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new NegativeSpace(*this);
           }
         };

         NegativeSpace* n = new NegativeSpace(ts,gap,lower);
         if ((n->status() == SS_FAILED) ||
             (n->x.min() != lower) || (n->x.max() != gap-1)) {
           delete n;
           return false;
         }
         delete n;

         class ReifiedSpace : public Space {
         public:
           IntVar x;
           BoolVar b;
           ReifiedSpace(const TupleSet& t, int gap0, int lower0)
             : x(*this,0,gap0), b(*this,0,1) {
             extensional(*this,IntVarArgs({x}),t,true,
                         Reify(b,RM_EQV),IPL_DOM);
             rel(*this,x,IRT_GQ,lower0);
           }
           ReifiedSpace(ReifiedSpace& s)
             : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space* copy(void) {
             return new ReifiedSpace(*this);
           }
         };

         ReifiedSpace* r = new ReifiedSpace(ts,gap,lower);
         const bool ok = (r->status() != SS_FAILED) &&
           (r->x.min() == lower) && (r->x.max() == gap) &&
           !r->b.assigned();
         delete r;
         return ok;
       }
     };

     /// Sparse smoke test for negative tuple-set posting
     class SparseTupleSetNegative : public ::Test::Base {
     public:
       SparseTupleSetNegative(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::Negative") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseNegativeSpace : public Space {
         public:
           IntVarArray x;
           SparseNegativeSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this, x, t, false, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseNegativeSpace(SparseNegativeSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space*
           copy(void) {
             return new SparseNegativeSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({1,1}));
         ts.finalize(EPK_SPARSE);

         SparseNegativeSpace* root = new SparseNegativeSpace(ts);
         DFS<SparseNegativeSpace> e(root);
         delete root;

         int n = 0;
         while (SparseNegativeSpace* sol = e.next()) {
           if (sol->x[0].val() == sol->x[1].val()) {
             delete sol;
             return false;
           }
           n++;
           delete sol;
         }
         return n == 2;
       }
     };

     /// Sparse smoke test for reified tuple-set posting
     class SparseTupleSetReified : public ::Test::Base {
     public:
       SparseTupleSetReified(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::Reified") {}

       virtual bool run(void) {
         using namespace Gecode;

         class SparseReifiedSpace : public Space {
         public:
           IntVarArray x;
           BoolVar b;
           SparseReifiedSpace(const TupleSet& t)
             : x(*this,2,0,1), b(*this,0,1) {
             extensional(*this, x, t, true, Reify(b,RM_EQV), IPL_DOM);
             rel(*this, b, IRT_EQ, 1);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           SparseReifiedSpace(SparseReifiedSpace& s)
             : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space*
           copy(void) {
             return new SparseReifiedSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({1,1}));
         ts.finalize(EPK_SPARSE);

         SparseReifiedSpace* root = new SparseReifiedSpace(ts);
         DFS<SparseReifiedSpace> e(root);
         delete root;

         int n = 0;
         while (SparseReifiedSpace* sol = e.next()) {
           if (sol->x[0].val() != sol->x[1].val()) {
             delete sol;
             return false;
           }
           n++;
           delete sol;
         }
         return n == 2;
       }
     };

     /// Sparse/compressed tuplesets should materialize only requested support
     class TupleSetSingleRepresentation : public ::Test::Base {
     public:
       TupleSetSingleRepresentation(void)
         : ::Test::Base("Int::Extensional::TupleSet::Support::SingleRepresentation") {}

       virtual bool run(void) {
         using namespace Gecode;
         TupleSet ts(2);
         for (int i=0; i<100; i++)
           ts.add(IntArgs({i, (i*3) % 100}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: Sparse support not available" << std::endl;
           return false;
         }
         if (ts.fst(0)->supports(ts.words(),ts.fst(0)->min) != nullptr) {
           std::cerr << "ERROR: Sparse range exposed dense support"
                     << std::endl;
           return false;
         }

         TupleSet tc(2);
         for (int i=0; i<100; i++)
           tc.add(IntArgs({i, (i*7) % 100}));
         tc.finalize(EPK_DENSE_COMPRESSED);
         if (tc.representation() != EPK_DENSE_COMPRESSED) {
           std::cerr << "ERROR: Compressed support not available" << std::endl;
           return false;
         }
         if (tc.fst(0)->supports(tc.words(),tc.fst(0)->min) != nullptr) {
           std::cerr << "ERROR: Compressed range exposed dense support"
                     << std::endl;
           return false;
         }

         TupleSet td(2);
         for (int i=0; i<100; i++)
           td.add(IntArgs({i, (i*11) % 100}));
         td.finalize();
         if (td.representation() != EPK_DENSE) {
           std::cerr << "ERROR: Default finalize did not keep dense support"
                     << std::endl;
           return false;
         }
         if (td.fst(0)->supports(td.words(),td.fst(0)->min) == nullptr) {
           std::cerr << "ERROR: Dense range lost support data" << std::endl;
           return false;
         }

         TupleSet empty_sparse(2);
         empty_sparse.finalize(EPK_SPARSE);
         if (empty_sparse.representation() != EPK_SPARSE) {
           std::cerr << "ERROR: Empty sparse table forgot representation"
                     << std::endl;
           return false;
         }
         TupleSet empty_compressed(2);
         empty_compressed.finalize(EPK_DENSE_COMPRESSED);
         if (empty_compressed.representation() != EPK_DENSE_COMPRESSED) {
           std::cerr << "ERROR: Empty compressed table forgot representation"
                     << std::endl;
           return false;
         }
         return true;
       }
     };

     /// Sparse offset arrays need a representable terminal entry
     class TupleSetSupportOffsetBoundary : public ::Test::Base {
     public:
       TupleSetSupportOffsetBoundary(void)
         : ::Test::Base(
             "Int::Extensional::TupleSet::Support::OffsetBoundary") {}

       virtual bool run(void) {
         using Gecode::Int::Extensional::support_offsets_size;
         const unsigned long long max =
           static_cast<unsigned long long>
           (std::numeric_limits<unsigned int>::max());
         unsigned int n_offsets = 0U;
         if (!support_offsets_size(max-1ULL,n_offsets) ||
             (n_offsets != std::numeric_limits<unsigned int>::max()))
           return false;
         if (support_offsets_size(max,n_offsets))
           return false;
         return !support_offsets_size(max+1ULL,n_offsets);
       }
     };

     /// Failed finalization is terminal and cannot expose partial support data
     class TupleSetTerminalFinalizationFailure : public ::Test::Base {
     public:
       TupleSetTerminalFinalizationFailure(void)
         : ::Test::Base(
             "Int::Extensional::TupleSet::Support::TerminalFailure") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int n = 371000;
         TupleSet ts(2);
         for (int i=0; i<n; i++)
           ts.add(IntArgs({i,n+i}));

         bool rejected = false;
         try {
           ts.finalize(EPK_DENSE);
         } catch (const Gecode::Int::OutOfLimits&) {
           rejected = true;
         }
         if (!rejected || !ts.failed() || ts.finalized())
           return false;

         try {
           (void) ts.representation();
           return false;
         } catch (const Gecode::Int::NotYetFinalized&) {
         }

         try {
           ts.finalize(EPK_SPARSE);
           return false;
         } catch (const Gecode::Int::AlreadyFinalized&) {
         }

         try {
           ts.add(IntArgs({0,0}));
           return false;
         } catch (const Gecode::Int::AlreadyFinalized&) {
         }

         class PostingSpace : public Space {
         public:
           IntVarArray x;
           PostingSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this,x,t);
           }
           PostingSpace(PostingSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new PostingSpace(*this);
           }
         };

         try {
           PostingSpace* s = new PostingSpace(ts);
           delete s;
           return false;
         } catch (const Gecode::Int::NotYetFinalized&) {
         }
         return true;
       }
     };

     /// DFA-derived tuple sets preserve the requested support representation
     class TupleSetDFARepresentation : public ::Test::Base {
     public:
       TupleSetDFARepresentation(void)
         : ::Test::Base(
             "Int::Extensional::TupleSet::Support::DFARepresentation") {}

       virtual bool run(void) {
         using namespace Gecode;

         DFA dfa((REG(0) + REG(1)) | (REG(2) + REG(3)));
         TupleSet sparse(2,dfa,EPK_SPARSE);
         if ((sparse.representation() != EPK_SPARSE) ||
             (sparse.tuples() != 2))
           return false;

         TupleSet compressed(2,dfa,EPK_DENSE_COMPRESSED);
         if ((compressed.representation() != EPK_DENSE_COMPRESSED) ||
             (compressed.tuples() != 2))
           return false;

         TupleSet dense(2,dfa);
         if ((dense.representation() != EPK_DENSE) || (dense.tuples() != 2))
           return false;

         TupleSet empty(3,dfa,EPK_SPARSE);
         return (empty.representation() == EPK_SPARSE) &&
           (empty.tuples() == 0);
       }
     };

     /// Disabled compact propagators preserve pending work when cloned
     class TupleSetDisabledClone : public ::Test::Base {
     public:
       TupleSetDisabledClone(void)
         : ::Test::Base(
             "Int::Extensional::TupleSet::Support::DisabledClone") {}

       virtual bool run(void) {
         using namespace Gecode;

         enum Mode {
           MODE_POSITIVE,
           MODE_NEGATIVE,
           MODE_REIFIED
         };

         class CloneSpace : public Space {
         public:
           IntVarArray x;
           BoolVar b;
           CloneSpace(const TupleSet& t, Mode mode)
             : x(*this,2,0,2), b(*this,0,1) {
             if (mode == MODE_REIFIED)
               extensional(*this,x,t,true,Reify(b,RM_EQV),IPL_DOM);
             else
               extensional(*this,x,t,mode == MODE_POSITIVE,IPL_DOM);
           }
           CloneSpace(CloneSpace& s)
             : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space* copy(void) {
             return new CloneSpace(*this);
           }
         };

         auto check = [](ExtensionalPropKind epk) {
           TupleSet ts(2);
           ts.add(IntArgs({0,0})).add(IntArgs({1,1})).add(IntArgs({2,2}));
           ts.finalize(epk);

           CloneSpace* source = new CloneSpace(ts,MODE_POSITIVE);
           if (source->status() == SS_FAILED) {
             delete source;
             return false;
           }
           PropagatorGroup::all.disable(*source);
           rel(*source,source->x[0],IRT_EQ,1);
           if ((source->status() == SS_FAILED) ||
               !source->x[0].assigned() || (source->x[0].val() != 1) ||
               (source->x[1].size() != 3)) {
             delete source;
             return false;
           }

           CloneSpace* clone = static_cast<CloneSpace*>(source->clone());
           PropagatorGroup::all.enable(*clone);
           const bool clone_ok = (clone->status() != SS_FAILED) &&
             clone->x[1].assigned() && (clone->x[1].val() == 1);
           delete clone;

           PropagatorGroup::all.enable(*source);
           const bool source_ok = (source->status() != SS_FAILED) &&
             source->x[1].assigned() && (source->x[1].val() == 1);
           delete source;
           return clone_ok && source_ok;
         };

         auto check_empty = [](ExtensionalPropKind epk, Mode mode) {
           TupleSet ts(2);
           ts.add(IntArgs({0,0})).add(IntArgs({1,1}));
           ts.finalize(epk);

           CloneSpace* source = new CloneSpace(ts,mode);
           if (source->status() == SS_FAILED) {
             delete source;
             return false;
           }
           PropagatorGroup::all.disable(*source);
           if (mode == MODE_NEGATIVE) {
             rel(*source,source->x[0],IRT_EQ,2);
           } else {
             rel(*source,source->x[0],IRT_EQ,0);
             rel(*source,source->x[1],IRT_EQ,1);
           }
           if (source->status() == SS_FAILED) {
             delete source;
             return false;
           }

           auto verify = [mode](CloneSpace& s) {
             const SpaceStatus status = s.status();
             if (mode == MODE_POSITIVE)
               return status == SS_FAILED;
             if (status == SS_FAILED)
               return false;
             if (mode == MODE_REIFIED)
               return s.b.assigned() && (s.b.val() == 0);
             return s.x[1].size() == 3;
           };

           CloneSpace* clone = static_cast<CloneSpace*>(source->clone());
           PropagatorGroup::all.enable(*clone);
           const bool clone_ok = verify(*clone);
           delete clone;

           PropagatorGroup::all.enable(*source);
           const bool source_ok = verify(*source);
           delete source;
           return clone_ok && source_ok;
         };

         for (ExtensionalPropKind epk :
                {EPK_DENSE,EPK_DENSE_COMPRESSED}) {
           if (!check(epk))
             return false;
           for (Mode mode : {MODE_POSITIVE,MODE_NEGATIVE,MODE_REIFIED})
             if (!check_empty(epk,mode))
               return false;
         }
         return true;
       }
     };

     /// AUTO finalization should work with default tuple-set posting overloads
     class TupleSetAutoDefaultDispatch : public ::Test::Base {
     public:
       TupleSetAutoDefaultDispatch(void)
         : ::Test::Base("Int::Extensional::TupleSet::Auto::DefaultDispatch") {}

       virtual bool run(void) {
         using namespace Gecode;

         TupleSet dense(2);
         dense.add(IntArgs({0,0})).add(IntArgs({1,1}));
         dense.finalize(EPK_AUTO);
         if (dense.representation() != EPK_DENSE) {
           std::cerr << "ERROR: Small AUTO table did not select dense"
                     << std::endl;
           return false;
         }

         class PositiveSpace : public Space {
         public:
           IntVarArray x;
           PositiveSpace(const TupleSet& t) : x(*this,2,0,1) {
             extensional(*this, x, t);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           PositiveSpace(PositiveSpace& s) : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new PositiveSpace(*this);
           }
         };

         PositiveSpace* dense_root = new PositiveSpace(dense);
         DFS<PositiveSpace> dense_engine(dense_root);
         delete dense_root;
         int dense_solutions = 0;
         while (PositiveSpace* sol = dense_engine.next()) {
           if (sol->x[0].val() != sol->x[1].val()) {
             delete sol;
             return false;
           }
           dense_solutions++;
           delete sol;
         }
         if (dense_solutions != 2)
           return false;

         const int n = 5000;
         TupleSet compressed(2);
         for (int i=0; i<n; i++)
           compressed.add(IntArgs({i, (i*7) % n}));
         compressed.finalize(EPK_AUTO);
         if (compressed.representation() != EPK_DENSE_COMPRESSED) {
           std::cerr << "ERROR: Large AUTO table did not select compressed"
                     << std::endl;
           return false;
         }

         class ReifiedSpace : public Space {
         public:
           IntVarArray x;
           BoolVar b;
           ReifiedSpace(const TupleSet& t, int n0)
             : x(*this,2,0,n0-1), b(*this,0,1) {
             extensional(*this, x, t, Reify(b,RM_EQV));
             rel(*this, b, IRT_EQ, 1);
             rel(*this, x[0], IRT_EQ, 3);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           ReifiedSpace(ReifiedSpace& s) : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space* copy(void) {
             return new ReifiedSpace(*this);
           }
         };

         ReifiedSpace* compressed_root = new ReifiedSpace(compressed,n);
         DFS<ReifiedSpace> compressed_engine(compressed_root);
         delete compressed_root;
         ReifiedSpace* compressed_sol = compressed_engine.next();
         if (compressed_sol == nullptr)
           return false;
         const bool ok =
           (compressed_sol->x[0].val() == 3) &&
           (compressed_sol->x[1].val() == ((3*7) % n));
         delete compressed_sol;
         ReifiedSpace* extra = compressed_engine.next();
         const bool no_extra = (extra == nullptr);
         delete extra;
         return ok && no_extra;
       }
     };

     /// Dense-compressed iterators should skip unsupported value gaps
     class DenseCompressedTupleSetWideGap : public ::Test::Base {
     public:
       DenseCompressedTupleSetWideGap(void)
         : ::Test::Base("Int::Extensional::TupleSet::DenseCompressed::WideGap") {}

       virtual bool run(void) {
         using namespace Gecode;

         const int gap = 1000000000;
         TupleSet ts(1);
         ts.add(IntArgs({0})).add(IntArgs({gap}));
         ts.finalize(EPK_DENSE_COMPRESSED);

         class NegativeSpace : public Space {
         public:
           IntVar x;
           NegativeSpace(const TupleSet& t, int gap0)
             : x(*this,0,gap0) {
             extensional(*this, IntVarArgs({x}), t, false, IPL_DOM);
           }
           NegativeSpace(NegativeSpace& s) : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new NegativeSpace(*this);
           }
         };

         NegativeSpace* ns = new NegativeSpace(ts,gap);
         if (ns->status() == SS_FAILED) {
           delete ns;
           return false;
         }
         if (ns->x.in(0) || ns->x.in(gap)) {
           delete ns;
           return false;
         }
         delete ns;

         class ReifiedSpace : public Space {
         public:
           IntVar x;
           BoolVar b;
           ReifiedSpace(const TupleSet& t, int gap0, bool force)
             : x(*this,0,gap0), b(*this,0,1) {
             extensional(*this, IntVarArgs({x}), t, true,
                         Reify(b,RM_EQV), IPL_DOM);
             if (force) {
               rel(*this, b, IRT_EQ, 1);
               rel(*this, x, IRT_EQ, 0);
             }
           }
           ReifiedSpace(ReifiedSpace& s) : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space* copy(void) {
             return new ReifiedSpace(*this);
           }
         };

         ReifiedSpace* rs = new ReifiedSpace(ts,gap,false);
         if (rs->status() == SS_FAILED) {
           delete rs;
           return false;
         }
         delete rs;

         ReifiedSpace* rfs = new ReifiedSpace(ts,gap,true);
         if (rfs->status() == SS_FAILED) {
           delete rfs;
           return false;
         }
         if (!rfs->x.assigned() || (rfs->x.val() != 0) ||
             !rfs->b.assigned() || (rfs->b.val() != 1)) {
           delete rfs;
           return false;
         }
         delete rfs;

         class PositiveDeltaSpace : public Space {
         public:
           IntVar x;
           PositiveDeltaSpace(const TupleSet& t, int gap0)
             : x(*this,0,gap0) {
             extensional(*this, IntVarArgs({x}), t, true, IPL_DOM);
             rel(*this, x, IRT_NQ, 0);
           }
           PositiveDeltaSpace(PositiveDeltaSpace& s) : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new PositiveDeltaSpace(*this);
           }
         };

         PositiveDeltaSpace* ps = new PositiveDeltaSpace(ts,gap);
         if (ps->status() == SS_FAILED) {
           delete ps;
           return false;
         }
         if (!ps->x.assigned() || (ps->x.val() != gap)) {
           delete ps;
           return false;
         }
         delete ps;

         return true;
       }
     };

     /// Sparse negative should fail if all combinations are forbidden
     class SparseTupleSetNegativeFail : public ::Test::Base {
     public:
       SparseTupleSetNegativeFail(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::NegativeFail") {}

       virtual bool run(void) {
         using namespace Gecode;
         class NegativeFailSpace : public Space {
         public:
           IntVarArray x;
           NegativeFailSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this, x, t, false, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           NegativeFailSpace(NegativeFailSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new NegativeFailSpace(*this);
           }
         };
         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({0,1}))
           .add(IntArgs({1,0})).add(IntArgs({1,1}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;
         NegativeFailSpace* root = new NegativeFailSpace(ts);
         DFS<NegativeFailSpace> e(root);
         delete root;
         NegativeFailSpace* sol = e.next();
         const bool ok = (sol == nullptr);
         delete sol;
         return ok;
       }
     };

     /// Sparse negative should prune values whose completions are all forbidden
     class SparseTupleSetNegativePrune : public ::Test::Base {
     public:
       SparseTupleSetNegativePrune(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::NegativePrune") {}

       virtual bool run(void) {
         using namespace Gecode;
         class NegativePruneSpace : public Space {
         public:
           IntVarArray x;
           NegativePruneSpace(const TupleSet& t)
             : x(*this,2,0,1) {
             extensional(*this, x, t, false, IPL_DOM);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           NegativePruneSpace(NegativePruneSpace& s)
             : Space(s) {
             x.update(*this,s.x);
           }
           virtual Space* copy(void) {
             return new NegativePruneSpace(*this);
           }
         };
         TupleSet ts(2);
         ts.add(IntArgs({0,0})).add(IntArgs({0,1}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;
         NegativePruneSpace* root = new NegativePruneSpace(ts);
         DFS<NegativePruneSpace> e(root);
         delete root;

         int n = 0;
         while (NegativePruneSpace* sol = e.next()) {
           if (sol->x[0].val() != 1) {
             delete sol;
             return false;
           }
           n++;
           delete sol;
         }
         return n == 2;
       }
     };

     /// Sparse reified posting should support all reify modes for positive/negative
     class SparseTupleSetReifiedModes : public ::Test::Base {
     public:
       SparseTupleSetReifiedModes(void)
         : ::Test::Base("Int::Extensional::TupleSet::Sparse::ReifiedModes") {}

       virtual bool run(void) {
         using namespace Gecode;
         class ReifModeSpace : public Space {
         public:
           IntVarArray x;
           BoolVar b;
           ReifModeSpace(const TupleSet& t, bool pos, ReifyMode rm, int bv)
             : x(*this,2,0,1), b(*this,0,1) {
             extensional(*this, x, t, pos, Reify(b,rm), IPL_DOM);
             rel(*this, b, IRT_EQ, bv);
             branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
           }
           ReifModeSpace(ReifModeSpace& s)
             : Space(s) {
             x.update(*this,s.x);
             b.update(*this,s.b);
           }
           virtual Space* copy(void) {
             return new ReifModeSpace(*this);
           }
         };

         TupleSet ts(2);
         ts.add(IntArgs({0,0}));
         ts.finalize(EPK_SPARSE);
         if (ts.representation() != EPK_SPARSE)
           return false;

         auto count = [&ts](bool pos, ReifyMode rm, int bv,
                            bool must_equal) {
           ReifModeSpace* root = new ReifModeSpace(ts,pos,rm,bv);
           DFS<ReifModeSpace> e(root);
           delete root;
           int n = 0;
           while (ReifModeSpace* sol = e.next()) {
             const bool eq = (sol->x[0].val() == 0) && (sol->x[1].val() == 0);
             if (must_equal != eq) {
               delete sol;
               return -1;
             }
             n++;
             delete sol;
           }
           return n;
         };

         if (count(true,RM_EQV,1,true) != 1)
           return false;
         if (count(true,RM_EQV,0,false) != 3)
           return false;
         if (count(true,RM_IMP,1,true) != 1)
           return false;
         if (count(true,RM_PMI,0,false) != 3)
           return false;
         if (count(false,RM_EQV,0,true) != 1)
           return false;
         if (count(false,RM_EQV,1,false) != 3)
           return false;

         return true;
       }
     };

     /// %Test with large tuple set
     class TupleSetLarge : public Test {
     protected:
       /// Whether the table is positive or negative
       bool pos;
       /// Support representation
       Gecode::ExtensionalPropKind epk;
       /// Tupleset used for testing
       mutable Gecode::TupleSet t;
     public:
       /// Create and register test
       TupleSetLarge(double prob, bool p, Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::" + extensional_kind_name(epk0) +
                "::" + str(p) + "::Large",
                5,1,5,true,Gecode::IPL_DOM),
           pos(p), epk(epk0), t(5) {
         using namespace Gecode;

         CpltAssignment ass(5, IntSet(1, 5));
         while (ass.has_more()) {
           if (_rand(100) <= prob*100) {
             IntArgs tuple(5);
             for (int i = 5; i--; ) tuple[i] = ass[i];
             t.add(tuple);
           }
           ass.next(_rand);
         }
         t.finalize(epk);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         using namespace Gecode;
         for (int i = 0; i < t.tuples(); ++i) {
           TupleSet::Tuple l = t[i];
           bool same = true;
           for (int j = 0; j < t.arity() && same; ++j)
             if (l[j] != x[j]) same = false;
           if (same)
             return pos;
         }
         return !pos;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x, t, pos, ipl);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         using namespace Gecode;
         extensional(home, x, t, pos, r, ipl);
       }
     };

     /// %Test with bool tuple set
     class TupleSetBool : public Test {
     protected:
       /// Whether the table is positive or negative
       bool pos;
       /// Support representation
       Gecode::ExtensionalPropKind epk;
       /// Tupleset used for testing
       mutable Gecode::TupleSet t;
     public:
       /// Create and register test
       TupleSetBool(double prob, bool p, Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::" + extensional_kind_name(epk0) +
                "::" + str(p) + "::Bool",
                5,0,1,true), pos(p), epk(epk0), t(5) {
         using namespace Gecode;

         CpltAssignment ass(5, IntSet(0, 1));
         while (ass.has_more()) {
           if (_rand(100) <= prob*100) {
             IntArgs tuple(5);
             for (int i = 5; i--; ) tuple[i] = ass[i];
             t.add(tuple);
           }
           ass.next(_rand);
         }
         t.finalize(epk);
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         using namespace Gecode;
         for (int i = 0; i < t.tuples(); ++i) {
           TupleSet::Tuple l = t[i];
           bool same = true;
           for (int j = 0; j < t.arity() && same; ++j)
             if (l[j] != x[j])
               same = false;
           if (same)
             return pos;
         }
         return !pos;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(x.size());
         for (int i = x.size(); i--; )
           y[i] = channel(home, x[i]);
         extensional(home, y, t, pos, ipl);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         using namespace Gecode;
         BoolVarArgs y(x.size());
         for (int i = x.size(); i--; )
           y[i] = channel(home, x[i]);
         extensional(home, y, t, pos, r, ipl);
       }
     };

     /// Help class to create and register tests with a fixed table size
     class TupleSetTestSize {
     public:
       /// Perform creation and registration
       TupleSetTestSize(int size, bool pos, Gecode::ExtensionalPropKind epk,
                        Gecode::Support::RandomGenerator& rand) {
         using namespace Gecode;
         /// Find the arity needed for creating sufficient number of tuples
         int arity = 2;
         int n_tuples = 5*5;
         while (n_tuples < size) {
           arity++;
           n_tuples*=5;
         }
         /// Build TupleSet
         TupleSet ts(arity);
         CpltAssignment ass(arity, IntSet(0, 4));
         for (int i = size; i--; ) {
           assert(ass.has_more());
           IntArgs tuple(arity);
           for (int j = arity; j--; ) tuple[j] = ass[j];
           ts.add(tuple);
           ass.next(rand);
         }
         ts.finalize(epk);
         assert(ts.tuples() == size);
         // Create and register test
         (void) new TupleSetTest(std::to_string(size),pos,IntSet(0,4),ts,
                                 size <= 128, epk);
       }
     };

     Gecode::TupleSet randomTupleSet(int n, int min, int max, double prob,
                                     Gecode::ExtensionalPropKind epk,
                                     Gecode::Support::RandomGenerator& rand) {
       using namespace Gecode;
       TupleSet t(n);
       CpltAssignment ass(n, IntSet(min, max));
       while (ass.has_more()) {
         if (rand(100) <= prob*100) {
           IntArgs tuple(n);
           for (int i = n; i--; ) tuple[i] = ass[i];
           t.add(tuple);
         }
         ass.next(rand);
       }
       t.finalize(epk);
       return t;
     }

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         // This code is executed on load, and thus a random number generator source from the supplied
         // seed is not available.
         // In order to get interesting data her, but still have deterministic and repeatable execution, a fixed seed
         // is used for a local random number generator.
         // TODO: Make this code later on test run, and use the supplied seed/random number generator.
         Gecode::Support::RandomGenerator rand(42);

         using namespace Gecode;
         for (ExtensionalPropKind epk :
                { EPK_DENSE, EPK_SPARSE, EPK_DENSE_COMPRESSED }) {
           for (bool pos : { false, true }) {
           {
             TupleSet ts(4);
             ts.add({2, 1, 2, 4}).add({2, 2, 1, 4})
               .add({4, 3, 4, 1}).add({1, 3, 2, 3})
               .add({3, 3, 3, 2}).add({5, 1, 4, 4})
               .add({2, 5, 1, 5}).add({4, 3, 5, 1})
               .add({1, 5, 2, 5}).add({5, 3, 3, 2})
               .finalize(epk);
             (void) new TupleSetTest("A",pos,IntSet(0,6),ts,true,epk);
           }
           {
             TupleSet ts(4);
             ts.finalize(epk);
             (void) new TupleSetTest("Empty",pos,IntSet(1,2),ts,true,epk);
           }
           {
             TupleSet ts(4);
             for (int n=1024*16; n--; )
               ts.add({1,2,3,4});
             ts.finalize(epk);
             (void) new TupleSetTest("Assigned",pos,IntSet(1,4),ts,true,epk);
           }
           {
             TupleSet ts(1);
             ts.add({1}).add({2}).add({3}).finalize(epk);
             (void) new TupleSetTest("Single",pos,IntSet(-4,4),ts,true,epk);
           }
           {
             int m = Gecode::Int::Limits::min;
             TupleSet ts(3);
             ts.add({m+0,m+1,m+2}).add({m+4,m+1,m+3})
               .add({m+2,m+3,m+0}).add({m+2,m+3,m+0})
               .add({m+1,m+2,m+5}).add({m+2,m+3,m+0})
               .add({m+3,m+6,m+5}).finalize(epk);
             (void) new TupleSetTest("Min",pos,IntSet(m,m+7),ts,true,epk);
           }
           {
             int M = Gecode::Int::Limits::max;
             TupleSet ts(3);
             ts.add({M-0,M-1,M-2}).add({M-4,M-1,M-3})
               .add({M-2,M-3,M-0}).add({M-2,M-3,M-0})
               .add({M-1,M-2,M-5}).add({M-2,M-3,M-0})
               .add({M-3,M-6,M-5}).finalize(epk);
             (void) new TupleSetTest("Max",pos,IntSet(M-7,M),ts,true,epk);
           }
           {
             int m = Gecode::Int::Limits::min;
             int M = Gecode::Int::Limits::max;
             TupleSet ts(3);
             ts.add({M-0,m+1,M-2}).add({m+4,M-1,M-3})
               .add({m+2,M-3,m+0}).add({M-2,M-3,M-0})
               .finalize(epk);
             (void) new TupleSetTest("MinMax",pos,
                                     IntSet(IntArgs({m,m+1,m+4,M-3,M-2,M})),
                                     ts,true,epk);
           }
           {
             TupleSet ts(7);
             const int triangle_tuples =
               (epk == EPK_DENSE) ? 10000 : 1000;
             for (int i = 0; i < triangle_tuples; i++) {
               IntArgs tuple(7);
               for (int j = 0; j < 7; j++) {
                 tuple[j] = rand(j+1);
               }
               ts.add(tuple);
             }
             ts.finalize(epk);
             (void) new RandomTupleSetTest("Triangle",pos,IntSet(0,6),ts,epk);
           }
           {
             for (int i = 0; i <= 64*6; i+=32)
               (void) new TupleSetTestSize(i, pos, epk, rand);
           }
           {
             const double prob_small =
               (epk == EPK_DENSE) ? 0.05 : 0.01;
             (void) new RandomTupleSetTest("Rand(10,-1,2)", pos,
                                           IntSet(-1,2),
                                           randomTupleSet(10, -1, 2, prob_small,
                                                          epk, rand),
                                           epk);
             if (epk == EPK_DENSE)
               (void) new RandomTupleSetTest("Rand(5,-10,10)", pos,
                                             IntSet(-10,10),
                                             randomTupleSet(5, -10, 10, 0.05,
                                                            epk, rand),
                                             epk);
           }
           {
             TupleSet t(5);
             CpltAssignment ass(4, IntSet(1, 4));
             while (ass.has_more()) {
               IntArgs tuple(5);
               tuple[4] = 1;
               for (int i = 4; i--; ) tuple[i] = ass[i];
               t.add(tuple);
               ass.next(rand);
             }
             t.add({2,2,4,3,4});
             t.finalize(epk);
             (void) new TupleSetTest("FewLast",pos,IntSet(1,4),t,false,epk);
           }
           {
             TupleSet t(4);
             CpltAssignment ass(4, IntSet(1, 6));
             while (ass.has_more()) {
               t.add({ass[0],0,ass[1],ass[2]});
               ass.next(rand);
             }
             t.add({2,-1,3,4});
             t.finalize(epk);
             (void) new TupleSetTest("FewMiddle",pos,IntSet(-1,6),t,false,epk);
           }
           if (epk == EPK_DENSE) {
             TupleSet t(10);
             CpltAssignment ass(9, IntSet(1, 4));
             while (ass.has_more()) {
               if (rand(100) <= 0.25*100) {
                 IntArgs tuple(10);
                 tuple[0] = 2;
                 for (int i = 9; i--; )
                   tuple[i+1] = ass[i];
                 t.add(tuple);
               }
               ass.next(rand);
             }
             t.add({1,1,1,1,1,1,1,1,1,1});
             t.add({1,2,3,4,4,2,1,2,3,3});
             t.finalize(epk);
             (void) new RandomTupleSetTest("FewHuge",pos,IntSet(1,4),t,epk);
           }
           (void) new TupleSetBase(pos,epk);
           (void) new TupleSetLarge(0.05,pos,epk);
           (void) new TupleSetBool(0.3,pos,epk);
           }
         }
       }
     };

     Create c;

     RegSimpleA ra;
     RegSimpleB rb;
     RegSimpleC rc;

     RegDistinct rd;

     RegRoland rr1(1);
     RegRoland rr2(2);
     RegRoland rr3(3);
     RegRoland rr4(4);

     RegSharedA rsa;
     RegSharedB rsb;
     RegSharedC rsc;
     RegSharedD rsd;

     RegEmptyDFA redfa;
     RegEmptyREG rereg;

     RegOpt ro0(CHAR_MAX-1);
     RegOpt ro1(CHAR_MAX);
     RegOpt ro2(static_cast<int>(UCHAR_MAX-1));
     RegOpt ro3(static_cast<int>(UCHAR_MAX));
     RegOpt ro4(SHRT_MAX-1);
     RegOpt ro5(SHRT_MAX);
     RegOpt ro6(static_cast<int>(USHRT_MAX-1));
     RegOpt ro7(static_cast<int>(USHRT_MAX));

     SparseTupleSetUnary sparse_tuple_set_unary;
     SparseTupleSetTernary sparse_tuple_set_ternary;
     SparseTupleSetHighArity sparse_tuple_set_high_arity;
     SparseTupleSetNullary sparse_tuple_set_nullary;
     SparseTupleSetIncrementalDelta sparse_tuple_set_incremental_delta;
     SparseTupleSetIncrementalAssign sparse_tuple_set_incremental_assign;
     SparseTupleSetIncrementalBool sparse_tuple_set_incremental_bool;
     SparseTupleSetDisabledFailure sparse_tuple_set_disabled_failure;
     SparseTupleSetWideDelta sparse_tuple_set_wide_delta;
     SparseTupleSetNegative sparse_tuple_set_negative;
     SparseTupleSetReified sparse_tuple_set_reified;
     TupleSetSingleRepresentation tuple_set_single_representation;
     TupleSetSupportOffsetBoundary tuple_set_support_offset_boundary;
     TupleSetTerminalFinalizationFailure tuple_set_terminal_failure;
     TupleSetDFARepresentation tuple_set_dfa_representation;
     TupleSetDisabledClone tuple_set_disabled_clone;
     TupleSetAutoDefaultDispatch tuple_set_auto_default_dispatch;
     DenseCompressedTupleSetWideGap dense_compressed_tuple_set_wide_gap;
     SparseTupleSetNegativeFail sparse_tuple_set_negative_fail;
     SparseTupleSetNegativePrune sparse_tuple_set_negative_prune;
     SparseTupleSetReifiedModes sparse_tuple_set_reified_modes;
     //@}

   }
}}


// STATISTICS: test-int
