/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2007
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

     /// %Test with tuple set
     class TupleSetA : public Test {
     protected:
       /// Extensional propagation kind
       Gecode::ExtensionalPropKind epk;
     public:
       /// Create and register test
       TupleSetA(Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::A::"+str(epk0),
                4,1,5,false,Gecode::ICL_DOM), epk(epk0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0] == 1 && x[1] == 3 && x[2] == 2 && x[3] == 3) ||
                 (x[0] == 2 && x[1] == 1 && x[2] == 2 && x[3] == 4) ||
                 (x[0] == 2 && x[1] == 2 && x[2] == 1 && x[3] == 4) ||
                 (x[0] == 3 && x[1] == 3 && x[2] == 3 && x[3] == 2) ||
                 (x[0] == 4 && x[1] == 3 && x[2] == 4 && x[3] == 1)
                 );
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         TupleSet t;
         IntArgs t1(4,  2, 1, 2, 4);
         IntArgs t2(4,  2, 2, 1, 4);
         IntArgs t3(4,  4, 3, 4, 1);
         IntArgs t4(4,  1, 3, 2, 3);
         IntArgs t5(4,  3, 3, 3, 2);
         t.add(t1);
         t.add(t1);
         t.add(t2);
         t.add(t2);
         t.add(t3);
         t.add(t3);
         t.add(t4);
         t.add(t4);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.add(t5);
         t.finalize();

         extensional(home, x, t, epk, ICL_DEF);
       }
     };

     /// %Test with tuple set
     class TupleSetB : public Test {
       mutable Gecode::TupleSet t;
     protected:
       /// Extensional propagation kind
       Gecode::ExtensionalPropKind epk;
     public:
       /// Create and register test
       TupleSetB(Gecode::ExtensionalPropKind epk0)
         : Test("Extensional::TupleSet::B::"+str(epk0),
                4,1,5,false,Gecode::ICL_DOM), epk(epk0) {
         using namespace Gecode;
         IntArgs t1 (4,  2, 1, 2, 4);
         IntArgs t2 (4,  2, 2, 1, 4);
         IntArgs t3 (4,  4, 3, 4, 1);
         IntArgs t4 (4,  1, 3, 2, 3);
         IntArgs t5 (4,  3, 3, 3, 2);
         IntArgs t6 (4,  5, 1, 4, 4);
         IntArgs t7 (4,  2, 5, 1, 5);
         IntArgs t8 (4,  4, 3, 5, 1);
         IntArgs t9 (4,  1, 5, 2, 5);
         IntArgs t10(4,  5, 3, 3, 2);
         t.add(t1);
         t.add(t2);
         t.add(t3);
         t.add(t4);
         t.add(t5);
         t.add(t6);
         t.add(t7);
         t.add(t8);
         t.add(t9);
         t.add(t10);
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
           if (same) return true;
         }
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x, t, epk, ICL_DEF);
       }
     };



     /// %Test with bool tuple set
     class TupleSetBool : public Test {
       mutable Gecode::TupleSet t;
     protected:
       /// Extensional propagation kind
       Gecode::ExtensionalPropKind epk;
     public:
       /// Create and register test
       TupleSetBool(Gecode::ExtensionalPropKind epk0, double prob)
         : Test("Extensional::TupleSet::Bool::"+str(epk0),
                5,0,1,false,Gecode::ICL_DOM), epk(epk0) {
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
             if (l[j] != x[j]) same = false;
           if (same) return true;
         }
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(x.size());
         for (int i = x.size(); i--; ) y[i] = channel(home, x[i]);
         extensional(home, y, t, epk, ICL_DEF);
       }
     };


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

     TupleSetA tsam(Gecode::EPK_MEMORY);
     TupleSetA tsas(Gecode::EPK_SPEED);

     TupleSetB tsbm(Gecode::EPK_MEMORY);
     TupleSetB tsbs(Gecode::EPK_SPEED);

     TupleSetBool tsboolm(Gecode::EPK_MEMORY, 0.3);
     TupleSetBool tsbools(Gecode::EPK_SPEED, 0.3);
     //@}

   }
}}


// STATISTICS: test-int

