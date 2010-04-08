/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
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

   /// %Tests for element constraints
   namespace Element {

     /**
      * \defgroup TaskTestIntElement Element constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for element with integer array and integer variables
     class IntIntVar : public Test {
     protected:
       /// Array of integers
       Gecode::IntArgs c;
     public:
       /// Create and register test
       IntIntVar(const std::string& s, const Gecode::IntArgs& c0,
                 int min, int max)
         : Test("Element::Int::Int::Var::"+s,2,min,max), 
           c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<c.size()) && c[x[0]]==x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::element(home, c, x[0], x[1]);
       }
     };

     /// %Test for element with integer array and integer variables
     class IntIntInt : public Test {
     protected:
       /// Array of integers
       Gecode::IntArgs c;
       /// Integer result
       int r;
     public:
       /// Create and register test
       IntIntInt(const std::string& s, const Gecode::IntArgs& c0, int r0)
         : Test("Element::Int::Int::Int::"+s+"::"+str(r0),1,-4,8),
           c(c0), r(r0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<c.size()) && c[x[0]]==r;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::element(home, c, x[0], r);
       }
     };

     /// %Test for element with integer array and single shared integer variable
     class IntIntShared : public Test {
     protected:
       /// Array of integers
       Gecode::IntArgs c;
     public:
       /// Create and register test
       IntIntShared(const std::string& s, const Gecode::IntArgs& c0,
                    int minDomain=-4)
         : Test("Element::Int::Int::Shared::"+s,1,minDomain,8), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<c.size()) && c[x[0]]==x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::element(home, c, x[0], x[0]);
       }
     };

     /// %Test for element with integer array and integer and Boolean variable
     class IntBoolVar : public Test {
     protected:
       /// Array of integers
       Gecode::IntArgs c;
     public:
       /// Create and register test
       IntBoolVar(const std::string& s, const Gecode::IntArgs& c0)
         : Test("Element::Int::Bool::Var::"+s,2,-4,8), c(c0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<c.size()) && c[x[0]]==x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::element(home, c, x[0], Gecode::channel(home,x[1]));
       }
     };

     /// %Test for element with integer array and integer and Boolean variable
     class IntBoolInt : public Test {
     protected:
       /// Array of integers
       Gecode::IntArgs c;
       /// Integer result
       int r;
     public:
       /// Create and register test
       IntBoolInt(const std::string& s, const Gecode::IntArgs& c0, int r0)
         : Test("Element::Int::Bool::Int::"+s+"::"+str(r0),1,-4,8),
           c(c0), r(r0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<c.size()) && c[x[0]]==r;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::element(home, c, x[0], r);
       }
     };

     /// %Test for element with variable array and integer variables
     class VarIntVar : public Test {
     public:
       /// Create and register test
       VarIntVar(Gecode::IntConLevel icl)
         : Test("Element::Var::Int::Var::"+str(icl),6,-1,3,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<x.size()-2) && x[2+x[0]]==x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs c(x.size()-2);
         for (int i=0; i<x.size()-2; i++)
           c[i]=x[2+i];
         Gecode::element(home, c, x[0], x[1], icl);
       }
     };

     /// %Test for element with variable array and integer variables
     class VarIntInt : public Test {
     protected:
       /// Integer result
       int r;
     public:
       /// Create and register test
       VarIntInt(Gecode::IntConLevel icl, int r0)
         : Test("Element::Var::Int::Int::"+str(icl)+"::"+str(r0),
                5,-1,3,false,icl), r(r0) {
         contest = CTL_NONE;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<x.size()-1) && x[1+x[0]]==r;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs c(x.size()-1);
         for (int i=0; i<x.size()-1; i++)
           c[i]=x[1+i];
         Gecode::element(home, c, x[0], r, icl);
       }
     };

     /// %Test for element with variable array and shared integer variable
     class VarIntShared : public Test {
     public:
       /// Create and register test
       VarIntShared(Gecode::IntConLevel icl)
         : Test("Element::Var::Int::Shared::"+str(icl),5,-1,3,false,icl) {
         contest = CTL_NONE;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0]>= 0) && (x[0]<x.size()-1) && x[1+x[0]]==x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs c(x.size()-1);
         for (int i=0; i<x.size()-1; i++)
           c[i]=x[1+i];
         Gecode::element(home, c, x[0], x[0], icl);
       }
     };

     /// %Test for element with Boolean variable array and integer variable
     class VarBoolVar : public Test {
     public:
       /// Create and register test
       VarBoolVar(void) : Test("Element::Var::Bool::Var",6,-1,3,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size()-2; i++)
           if ((x[2+i] < 0) || (x[2+i]>1))
             return false;
         return ((x[0]>= 0) && (x[0]<x.size()-2) && x[2+x[0]]==x[1]
                 && (x[1]>=0) && (x[1]<=1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs c(x.size()-2);
         for (int i=0; i<x.size()-2; i++)
           c[i]=channel(home,x[2+i]);
         element(home, c, x[0], channel(home,x[1]));
       }
     };

     /// %Test for element with Boolean variable array and integer variable
     class VarBoolInt : public Test {
     protected:
       /// Integer result
       int r;
     public:
       /// Create and register test
       VarBoolInt(int r0)
         : Test("Element::Var::Bool::Int::"+str(r0),5,-1,3,false), r(r0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size()-1; i++)
           if ((x[1+i] < 0) || (x[1+i]>1))
             return false;
         return ((x[0]>= 0) && (x[0]<x.size()-1) && x[1+x[0]]==r);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs c(x.size()-1);
         for (int i=0; i<x.size()-1; i++)
           c[i]=channel(home,x[1+i]);
         element(home, c, x[0], r);
       }
     };


     /// %Test for matrix element with integer array and integer variable
     class MatrixIntIntVarXY : public Test {
     protected:
       /// Array for test matrix
       Gecode::IntArgs tm;
     public:
       /// Create and register test
       MatrixIntIntVarXY(void)
         : Test("Element::Matrix::Int::IntVar::XY",3,0,5,false), 
           tm(6, 0,1,2,3,4,5) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         if ((x[0] > 2) || (x[1] > 1))
           return false;
         Matrix<IntArgs> m(tm,3,2);
         return m(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         Matrix<IntArgs> m(tm,3,2);
         element(home, m, x[0], x[1], x[2]);
       }
     };

     /// %Test for matrix element with integer array and integer variable
     class MatrixIntIntVarXX : public Test {
     protected:
       /// Array for test matrix
       Gecode::IntArgs tm;
     public:
       /// Create and register test
       MatrixIntIntVarXX(void)
         : Test("Element::Matrix::Int::IntVar::XX",2,0,3,false), 
           tm(4, 0,1,2,3) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         using namespace Gecode;
         if (x[0] > 1)
           return false;
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         using namespace Gecode;
         Matrix<IntArgs> m(tm,2,2);
         element(home, m, x[0], x[0], x[1]);
       }
     };

     /// %Test for matrix element with integer array and Boolean variable
     class MatrixIntBoolVarXY : public Test {
     protected:
       /// Array for test matrix
       Gecode::IntArgs tm;
     public:
       /// Create and register test
       MatrixIntBoolVarXY(void)
         : Test("Element::Matrix::Int::BoolVar::XY",3,0,3,false), 
           tm(4, 0,1,1,0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         if ((x[0] > 1) || (x[1] > 1))
           return false;
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         Matrix<IntArgs> m(tm,2,2);
         element(home, m, x[0], x[1], channel(home,x[2]));
       }
     };

     /// %Test for matrix element with integer array and Boolean variable
     class MatrixIntBoolVarXX : public Test {
     protected:
       /// Array for test matrix
       Gecode::IntArgs tm;
     public:
       /// Create and register test
       MatrixIntBoolVarXX(void)
         : Test("Element::Matrix::Int::BoolVar::XX",2,0,3,false), 
           tm(4, 0,1,1,0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         using namespace Gecode;
         if (x[0] > 1)
           return false;
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         using namespace Gecode;
         Matrix<IntArgs> m(tm,2,2);
         element(home, m, x[0], x[0], channel(home,x[1]));
       }
     };

     /// %Test for matrix element with integer variable array and integer variable
     class MatrixIntVarIntVarXY : public Test {
     public:
       /// Create and register test
       MatrixIntVarIntVarXY(void)
         : Test("Element::Matrix::IntVar::IntVar::XY",3+4,0,3,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         // remaining: matrix
         using namespace Gecode;
         if ((x[0] > 1) || (x[1] > 1))
           return false;
         IntArgs tm(4);
         tm[0]=x[3]; tm[1]=x[4]; tm[2]=x[5]; tm[3]=x[6];
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         IntVarArgs tm(4);
         tm[0]=x[3]; tm[1]=x[4]; tm[2]=x[5]; tm[3]=x[6];
         Matrix<IntVarArgs> m(tm,2,2);
         element(home, m, x[0], x[1], x[2]);
       }
     };

     /// %Test for matrix element with integer variable array and integer variable
     class MatrixIntVarIntVarXX : public Test {
     public:
       /// Create and register test
       MatrixIntVarIntVarXX(void)
         : Test("Element::Matrix::IntVar::IntVar::XX",2+4,0,3,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         // remaining: matrix
         using namespace Gecode;
         if (x[0] > 1)
           return false;
         IntArgs tm(4);
         tm[0]=x[2]; tm[1]=x[3]; tm[2]=x[4]; tm[3]=x[5];
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[1]
         using namespace Gecode;
         IntVarArgs tm(4);
         tm[0]=x[2]; tm[1]=x[3]; tm[2]=x[4]; tm[3]=x[5];
         Matrix<IntVarArgs> m(tm,2,2);
         element(home, m, x[0], x[0], x[1]);
       }
     };

     /// %Test for matrix element with Boolean variable array and Boolean variable
     class MatrixBoolVarBoolVarXY : public Test {
     public:
       /// Create and register test
       MatrixBoolVarBoolVarXY(void)
         : Test("Element::Matrix::BoolVar::BoolVar::XY",3+4,0,1,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         // remaining: matrix
         using namespace Gecode;
         IntArgs tm(4);
         tm[0]=x[3]; tm[1]=x[4]; tm[2]=x[5]; tm[3]=x[6];
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
         using namespace Gecode;
         BoolVarArgs tm(4);
         tm[0]=channel(home,x[3]); tm[1]=channel(home,x[4]); 
         tm[2]=channel(home,x[5]); tm[3]=channel(home,x[6]);
         Matrix<BoolVarArgs> m(tm,2,2);
         element(home, m, x[0], x[1], channel(home,x[2]));
       }
     };

     /// %Test for matrix element with Boolean variable array and Boolean variable
     class MatrixBoolVarBoolVarXX : public Test {
     public:
       /// Create and register test
       MatrixBoolVarBoolVarXX(void)
         : Test("Element::Matrix::BoolVar::BoolVar::XX",2+4,0,1,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         // x-coordinate: x[0], y-coordinate: x[0], result: x[1]
         // remaining: matrix
         using namespace Gecode;
         IntArgs tm(4);
         tm[0]=x[2]; tm[1]=x[3]; tm[2]=x[4]; tm[3]=x[5];
         Matrix<IntArgs> m(tm,2,2);
         return m(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         // x-coordinate: x[0], y-coordinate: x[1], result: x[1]
         using namespace Gecode;
         BoolVarArgs tm(4);
         tm[0]=channel(home,x[2]); tm[1]=channel(home,x[3]); 
         tm[2]=channel(home,x[4]); tm[3]=channel(home,x[5]);
         Matrix<BoolVarArgs> m(tm,2,2);
         element(home, m, x[0], x[0], channel(home,x[1]));
       }
     };




     /// Help class to create and register tests
     class Create {
     public:
       /// %Test size-dependent optimizations
       void optimized(int idx, int val) {
         Gecode::IntArgs c(idx);
         for (int i=0; i<idx; i++)
           c[i]=std::max(val-i,0);
         (void) new IntIntVar(Test::str(idx)+"::"+Test::str(val)+"::val",c,
                              val-8,val-1);
         if (idx != val)
           (void) new IntIntVar(Test::str(idx)+"::"+Test::str(val)+"::idx",c,
                                idx-8,idx-1);
       }
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         IntArgs ic1(5, -1,1,-3,3,-4);
         IntArgs ic2(8, -1,1,-1,1,-1,1,0,0);
         IntArgs ic3(1, -1);
         IntArgs ic4(7, 0,-1,2,-2,4,-3,6);
         IntArgs ic5(6, 0,0,1,2,3,4);

         IntArgs bc1(5, 0,1,1,0,1);
         IntArgs bc2(8, 1,1,0,1,0,1,0,0);
         IntArgs bc3(1, 1);

         (void) new IntIntVar("A",ic1,-8,8);
         (void) new IntIntVar("B",ic2,-8,8);
         (void) new IntIntVar("C",ic3,-8,8);
         (void) new IntIntVar("D",ic4,-8,8);

         // Test optimizations
         {
           int ov[] = {
             SCHAR_MAX-1,SCHAR_MAX,
             SHRT_MAX-1,SHRT_MAX,
             0
           };
           for (int i=0; ov[i] != 0; i++)
             for (int j=0; ov[j] != 0; j++)
               optimized(ov[i],ov[j]);
         }

         for (int i=-4; i<=4; i++) {
           (void) new IntIntInt("A",ic1,i);
           (void) new IntIntInt("B",ic2,i);
           (void) new IntIntInt("C",ic3,i);
           (void) new IntIntInt("D",ic4,i);
         }

         (void) new IntIntShared("A",ic1);
         (void) new IntIntShared("B",ic2);
         (void) new IntIntShared("C",ic3);
         (void) new IntIntShared("D",ic4);
         (void) new IntIntShared("E",ic5,1);

         (void) new IntBoolVar("A",bc1);
         (void) new IntBoolVar("B",bc2);
         (void) new IntBoolVar("C",bc3);

         for (int i=0; i<=1; i++) {
           (void) new IntBoolInt("A",bc1,i);
           (void) new IntBoolInt("B",bc2,i);
           (void) new IntBoolInt("C",bc3,i);
         }

         (void) new VarIntVar(ICL_BND);
         (void) new VarIntVar(ICL_DOM);

         for (int i=-4; i<=4; i++) {
           (void) new VarIntInt(ICL_BND,i);
           (void) new VarIntInt(ICL_DOM,i);
         }

         (void) new VarIntShared(ICL_BND);
         (void) new VarIntShared(ICL_DOM);

         (void) new VarBoolVar();
         (void) new VarBoolInt(0);
         (void) new VarBoolInt(1);

         // Matrix tests
         (void) new MatrixIntIntVarXY();
         (void) new MatrixIntIntVarXX();
         (void) new MatrixIntBoolVarXY();
         (void) new MatrixIntBoolVarXX();

         (void) new MatrixIntVarIntVarXY();
         (void) new MatrixIntVarIntVarXX();
         (void) new MatrixBoolVarBoolVarXY();
         (void) new MatrixBoolVarBoolVarXX();
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
