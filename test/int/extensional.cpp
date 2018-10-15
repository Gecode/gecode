/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Linnea Ingmar <linnea.ingmar@hotmail.com>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Linnea Ingmar, 2017
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

namespace Test { namespace Int {

   /// %Tests for extensional (relation) constraints
   namespace Extensional {

     /**
      * \defgroup TaskTestIntExtensional Extensional (relation) constraints
      * \ingroup TaskTestInt
      */
     //@{
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
     public:
       /// Create and register test
       TupleSetBase(bool p)
         : Test("Extensional::TupleSet::" + str(p) + "::Base",
                4,1,5,true,Gecode::IPL_DOM), t(4), pos(p) {
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
          .finalize();
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
         TupleSet ts = TupleSet(t.arity(),tupleset2dfa(t));
         assert(t == ts);
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
       /// The tuple set to use
       Gecode::TupleSet ts;
       /// Whether to validate dfa2tupleset
       bool toDFA;
     public:
       /// Create and register test
       TupleSetTest(const std::string& s, bool p,
                    Gecode::IntSet d0, Gecode::TupleSet ts0, bool td)
         : Test("Extensional::TupleSet::" + str(p) + "::" + s,
                ts0.arity(),d0,true,Gecode::IPL_DOM),
           pos(p), ts(ts0), toDFA(td) {
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
           TupleSet t = TupleSet(ts.arity(),tupleset2dfa(ts));
           assert(ts == t);
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
                          Gecode::IntSet d0, Gecode::TupleSet ts0)
         : TupleSetTest(s,p,d0,ts0,false) {
         testsearch = false;
       }
       /// Create and register initial assignment
       virtual Assignment* assignment(void) const {
         using namespace Gecode;
         return new RandomAssignment(arity,dom,1000);
       }
     };

     /// %Test with large tuple set
     class TupleSetLarge : public Test {
     protected:
       /// Whether the table is positive or negative
       bool pos;
       /// Tupleset used for testing
       mutable Gecode::TupleSet t;
     public:
       /// Create and register test
       TupleSetLarge(double prob, bool p)
         : Test("Extensional::TupleSet::" + str(p) + "::Large",
                5,1,5,true,Gecode::IPL_DOM), pos(p), t(5) {
         using namespace Gecode;

         CpltAssignment ass(5, IntSet(1, 5));
         while (ass()) {
           if (Base::rand(100) <= prob*100) {
             IntArgs tuple(5);
             for (int i = 5; i--; ) tuple[i] = ass[i];
             t.add(tuple);
           }
           ++ass;
         }
         t.finalize();
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
       /// Tupleset used for testing
       mutable Gecode::TupleSet t;
     public:
       /// Create and register test
       TupleSetBool(double prob, bool p)
         : Test("Extensional::TupleSet::" + str(p) + "::Bool",
                5,0,1,true), pos(p), t(5) {
         using namespace Gecode;

         CpltAssignment ass(5, IntSet(0, 1));
         while (ass()) {
           if (Base::rand(100) <= prob*100) {
             IntArgs tuple(5);
             for (int i = 5; i--; ) tuple[i] = ass[i];
             t.add(tuple);
           }
           ++ass;
         }
         t.finalize();
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
       TupleSetTestSize(int size, bool pos) {
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
           assert(ass());
           IntArgs tuple(arity);
           for (int j = arity; j--; ) tuple[j] = ass[j];
           ts.add(tuple);
           ++ass;
         }
         ts.finalize();
         assert(ts.tuples() == size);
         // Create and register test
         (void) new TupleSetTest(std::to_string(size),pos,IntSet(0,4),ts,
                                 size <= 128);
       }
     };

     Gecode::TupleSet randomTupleSet(int n, int min, int max, double prob) {
       using namespace Gecode;
       TupleSet t(n);
       CpltAssignment ass(n, IntSet(min, max));
       while (ass()) {
         if (Base::rand(100) <= prob*100) {
           IntArgs tuple(n);
           for (int i = n; i--; ) tuple[i] = ass[i];
           t.add(tuple);
         }
         ++ass;
       }
       t.finalize();
       return t;
     }
     
     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         for (bool pos : { false, true }) {
           {
             TupleSet ts(4);
             ts.add({2, 1, 2, 4}).add({2, 2, 1, 4})
               .add({4, 3, 4, 1}).add({1, 3, 2, 3})
               .add({3, 3, 3, 2}).add({5, 1, 4, 4})
               .add({2, 5, 1, 5}).add({4, 3, 5, 1})
               .add({1, 5, 2, 5}).add({5, 3, 3, 2})
               .finalize();
             (void) new TupleSetTest("A",pos,IntSet(0,6),ts,true);
           }
           {
             TupleSet ts(4);
             ts.finalize();
             (void) new TupleSetTest("Empty",pos,IntSet(1,2),ts,true);
           }
           {
             TupleSet ts(4);
             for (int n=1024*16; n--; )
               ts.add({1,2,3,4});
             ts.finalize();
             (void) new TupleSetTest("Assigned",pos,IntSet(1,4),ts,true);
           }
           {
             TupleSet ts(1);
             ts.add({1}).add({2}).add({3}).finalize();
             (void) new TupleSetTest("Single",pos,IntSet(-4,4),ts,true);
           }
           {
             int m = Gecode::Int::Limits::min;
             TupleSet ts(3);
             ts.add({m+0,m+1,m+2}).add({m+4,m+1,m+3})
               .add({m+2,m+3,m+0}).add({m+2,m+3,m+0})
               .add({m+1,m+2,m+5}).add({m+2,m+3,m+0})
               .add({m+3,m+6,m+5}).finalize();
             (void) new TupleSetTest("Min",pos,IntSet(m,m+7),ts,true);
           }
           {
             int M = Gecode::Int::Limits::max;
             TupleSet ts(3);
             ts.add({M-0,M-1,M-2}).add({M-4,M-1,M-3})
               .add({M-2,M-3,M-0}).add({M-2,M-3,M-0})
               .add({M-1,M-2,M-5}).add({M-2,M-3,M-0})
               .add({M-3,M-6,M-5}).finalize();
             (void) new TupleSetTest("Max",pos,IntSet(M-7,M),ts,true);
           }
           {
             int m = Gecode::Int::Limits::min;
             int M = Gecode::Int::Limits::max;
             TupleSet ts(3);
             ts.add({M-0,m+1,M-2}).add({m+4,M-1,M-3})
               .add({m+2,M-3,m+0}).add({M-2,M-3,M-0})
               .finalize();
             (void) new TupleSetTest("MinMax",pos,
                                     IntSet(IntArgs({m,m+1,m+4,M-3,M-2,M})),
                                     ts,true);
           }
           {
             TupleSet ts(7);
             for (int i = 0; i < 10000; i++) {
               IntArgs tuple(7);
               for (int j = 0; j < 7; j++) {
                 tuple[j] = Base::rand(j+1);
               }
               ts.add(tuple);
             }
             ts.finalize();
             (void) new RandomTupleSetTest("Triangle",pos,IntSet(0,6),ts);
           }
           {
             for (int i = 0; i <= 64*6; i+=32)
               (void) new TupleSetTestSize(i,pos);
           }
           {
             (void) new RandomTupleSetTest("Rand(10,-1,2)", pos,
                                           IntSet(-1,2),
                                           randomTupleSet(10,-1,2,0.05));
             (void) new RandomTupleSetTest("Rand(5,-10,10)", pos,
                                           IntSet(-10,10),
                                           randomTupleSet(5,-10,10,0.05));
           }
           {
             TupleSet t(5);
             CpltAssignment ass(4, IntSet(1, 4));
             while (ass()) {
               IntArgs tuple(5);
               tuple[4] = 1;
               for (int i = 4; i--; ) tuple[i] = ass[i];
               t.add(tuple);
               ++ass;
             }
             t.add({2,2,4,3,4});
             t.finalize();
             (void) new TupleSetTest("FewLast",pos,IntSet(1,4),t,false);
           }
           {
             TupleSet t(4);
             CpltAssignment ass(4, IntSet(1, 6));
             while (ass()) {
               t.add({ass[0],0,ass[1],ass[2]});
               ++ass;
             }
             t.add({2,-1,3,4});
             t.finalize();
             (void) new TupleSetTest("FewMiddle",pos,IntSet(-1,6),t,false);
           }
           {
             TupleSet t(10);
             CpltAssignment ass(9, IntSet(1, 4));
             while (ass()) {
               if (Base::rand(100) <= 0.25*100) {
                 IntArgs tuple(10);
                 tuple[0] = 2;
                 for (int i = 9; i--; ) tuple[i+1] = ass[i];
                 t.add(tuple);
               }
               ++ass;
             }
             t.add({1,1,1,1,1,1,1,1,1,1});
             t.add({1,2,3,4,4,2,1,2,3,3});
             t.finalize();
             (void) new RandomTupleSetTest("FewHuge",pos,IntSet(1,4),t);
           }
           (void) new TupleSetBase(pos);
           (void) new TupleSetLarge(0.05,pos);
           (void) new TupleSetBool(0.3,pos);
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
     //@}

   }
}}


// STATISTICS: test-int
