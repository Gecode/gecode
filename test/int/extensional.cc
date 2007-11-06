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

#include "gecode/minimodel.hh"

namespace Test { namespace Int {

   /// Tests for extensional (relation) constraints
   namespace Extensional {
   
     /**
      * \defgroup TaskTestIntExtensional Extensional (relation) constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// Test with simple regular expression
     class RegSimpleA : public Test {
     public:
       /// Create and register test
       RegSimpleA(void) : Test("Extensional::Reg::Simple::A",4,2,2) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     (REG(0) | REG(2)) +
                     (REG(-1) | REG(1)) +
                     (REG(7) | REG(0) | REG(1)) +
                     (REG(0) | REG(1)));
       }
     };
     
     /// Test with simple regular expression
     class RegSimpleB : public Test {
     public:
       /// Create and register test
       RegSimpleB(void) : Test("Extensional::Reg::Simple::B",4,2,2) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]<x[1]) && (x[1]<x[2]) && (x[2]<x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x,
                     (REG(-2) + REG(-1) + REG(0) + REG(1)) |
                     (REG(-2) + REG(-1) + REG(0) + REG(2)) |
                     (REG(-2) + REG(-1) + REG(1) + REG(2)) |
                     (REG(-2) + REG(0) + REG(1) + REG(2)) |
                     (REG(-1) + REG(0) + REG(1) + REG(2)));
         }
     };
     
     /// Test with regular expression for distinct constraint
     class RegDistinct : public Test {
     public:
       /// Create and register test
       RegDistinct(void) : Test("Extensional::Reg::Distinct",4,-1,4) {}
       /// Test whether \a x is solution
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
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
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
     
     /// Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedA : public Test {
     public:
       /// Create and register test
       RegSharedA(void) : Test("Extensional::Reg::Shared::A",4,2,2) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
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
   
     /// Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedB : public Test {
     public:
       /// Create and register test
       RegSharedB(void) : Test("Extensional::Reg::Shared::B",4,2,2) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (((x[0] == 0) || (x[0] == 2)) &&
                 ((x[1] == -1) || (x[1] == 1)) &&
                 ((x[2] == 0) || (x[2] == 1)) &&
                 ((x[3] == 0) || (x[3] == 1)));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
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
   
     /// Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedC : public Test {
     public:
       /// Create and register test
       RegSharedC(void) : Test("Extensional::Reg::Shared::C",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1]==1) && (x[2]==0) && (x[3]==1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolVarArgs y(8);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=channel(home,x[i]);
         unshare(home,y);
         extensional(home,y,
                     ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(2,2));
       }
     };
   
     /// Test with simple regular expression and shared variables (uses unsharing)
     class RegSharedD : public Test {
     public:
       /// Create and register test
       RegSharedD(void) : Test("Extensional::Reg::Shared::D",4,0,1) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[1]==1) && (x[2]==0) && (x[3]==1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         Gecode::BoolVarArgs y(12);
         for (int i=0; i<4; i++)
           y[i]=y[i+4]=y[i+8]=channel(home,x[i]);
         unshare(home, y);
         extensional(home, y,
                     ((REG(0) | REG(1)) + REG(1) + REG(0) + REG(1))(3,3));
       }
     };
   
     /// Test for empty DFA
     class RegEmptyDFA : public Test {
     public:
       /// Create and register test
       RegEmptyDFA(void) : Test("Extensional::Reg::Empty::DFA",1,0,0) {
         testsearch = false;
       }
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::DFA d;
         Gecode::extensional(home, x, d);
       }
     };
   
     /// Test for empty regular expression
     class RegEmptyREG : public Test {
     public:
       /// Create and register test
       RegEmptyREG(void) : Test("Extensional::Reg::Empty::REG",1,0,0) {
         testsearch = false;
       }
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::REG r;
         Gecode::extensional(home, x, r);
       }
     };
   
     /// Test with tuple set
     class TupleSetA : public Test {
     public:
       /// Create and register test
       TupleSetA(Gecode::PropKind pk)
         : Test("Extensional::TupleSet::A::"+str(pk),
                4,1,5,false,Gecode::ICL_DOM,pk) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0] == 1 && x[1] == 3 && x[2] == 2 && x[3] == 3) ||
                 (x[0] == 2 && x[1] == 1 && x[2] == 2 && x[3] == 4) ||
                 (x[0] == 2 && x[1] == 2 && x[2] == 1 && x[3] == 4) ||
                 (x[0] == 3 && x[1] == 3 && x[2] == 3 && x[3] == 2) ||
                 (x[0] == 4 && x[1] == 3 && x[2] == 4 && x[3] == 1)
                 );
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         TupleSet t;
         IntArgs t1(4,  2, 1, 2, 4);
         IntArgs t2(4,  2, 2, 1, 4);
         IntArgs t3(4,  4, 3, 4, 1);
         IntArgs t4(4,  1, 3, 2, 3);
         IntArgs t5(4,  3, 3, 3, 2);
         t.add(t1);
         t.add(t2);
         t.add(t3);
         t.add(t4);
         t.add(t5);
         t.finalize();

         extensional(home, x, t, ICL_DEF, pk);
       }
     };

     /// Test with tuple set
     class TupleSetB : public Test {
       mutable Gecode::TupleSet t;
     public:
       /// Create and register test
       TupleSetB(Gecode::PropKind pk)
         : Test("Extensional::TupleSet::B::"+str(pk),
                4,1,5,false,Gecode::ICL_DOM,pk) {
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
       }
       /// Test whether \a x is solution
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
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         extensional(home, x, t, ICL_DEF, pk);
       }
     };


     RegSimpleA ra;
     RegSimpleB rb;
   
     RegDistinct rd;
   
     RegSharedA rsa;
     RegSharedB rsb;
     RegSharedC rsc;
     RegSharedD rsd;

     RegEmptyDFA redfa;
     RegEmptyREG rereg;


     TupleSetA tsam(Gecode::PK_MEMORY);
     TupleSetA tsas(Gecode::PK_SPEED);

     TupleSetB tsbm(Gecode::PK_MEMORY);
     TupleSetB tsbs(Gecode::PK_SPEED);
     //@}

   }
}}


// STATISTICS: test-int

