/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

   /// Tests for minimal modelling constraints
   namespace MiniModel {
   
     /**
      * \defgroup TaskTestIntMiniModelLin Minimal modelling constraints (linear constraints)
      * \ingroup TaskTestInt
      */
     //@{
     /// Test for equality
     class LinEqA : public Test {
     public:
       /// Create and register test
       LinEqA(void) : Test("MiniModel::Lin::Eq::A",2,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (2*x[0]+1) == (x[1]-1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, 2*x[0]+1 == x[1]-1);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(2*x[0]+1 == x[1]-1,b)));
       }
     };
   
     /// Test for linear equality
     class LinEqB : public Test {
     public:
       /// Create and register test
       LinEqB(void) : Test("MiniModel::Lin::Eq::B",2,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (2*x[0]+1) == (x[1]-1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, 2*x[0]+1-x[1] == -1);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(2*x[0]+1-x[1] == -1,b)));
       }
     };
   
     /// Test for linear equality
     class LinEqC : public Test {
     public:
       /// Create and register test
       LinEqC(void) : Test("MiniModel::Lin::Eq::C",2,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return -(1-(1-x[1])) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, -(1-(1-x[1])) == x[0]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(-(1-(1-x[1])) == x[0],b)));
       }
     };
   
     /// Test for linear equality
     class LinEqD : public Test {
     public:
       /// Create and register test
       LinEqD(void) : Test("MiniModel::Lin::Eq::D",2,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return 2*(1-x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, 2*(1-x[1]) == x[0]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(2*(1-x[1]) == x[0],b)));
       }
     };
   
     /// Test for linear equality
     class LinEqE : public Test {
     public:
       /// Create and register test
       LinEqE(void) : Test("MiniModel::Lin::Eq::E",1,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return -1==x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, -1==x[0]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(-1==x[0],b)));
       }
     };
   
     /// Test for linear equality
     class LinEqF : public Test {
     public:
       /// Create and register test
       LinEqF(void) : Test("MiniModel::Lin::Eq::F",2,-2,2,false) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (2*x[0]+2) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, Gecode::post(home, 2*x[0]+2) == x[1]);
       }
     };
   
     /// Test for linear inequality
     class LinGr : public Test {
     public:
       /// Create and register test
       LinGr(void) : Test("MiniModel::Lin::Gr",1,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return 1>x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, 1>x[0]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(1>x[0],b)));
       }
     };
   
     /// Test for linear inequality
     class LinLe : public Test {
     public:
       /// Create and register test
       LinLe(void) : Test("MiniModel::Lin::Le",1,-2,2,true) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return 1<x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         Gecode::post(home, 1<x[0]);
       }
       /// Post reified constraint on \a x for \a b
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, 
                         Gecode::BoolVar b) {
         Gecode::post(home, Gecode::tt(eqv(1<x[0],b)));
       }
     };
   
     /// Test for linear equality over Boolean variables
     class LinEqBool : public Test {
     public:
       /// Create and register test
       LinEqBool(void) : Test("MiniModel::Lin::Eq::Bool",3,-2,2,false) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if ((x[i] < 0) || (x[i] > 1))
             return false;
         return (2*x[0]+1) == (x[1]+x[2]-1);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(x.size());
         for (int i=x.size(); i--; )
           y[i] = channel(home,x[i]);
         Gecode::post(home, 2*y[0]+1 == y[1]+y[2]-1);
       }
     };
     
     /// Test for posting linear expressions
     class LinExprInt : public Test {
     public:
       /// Create and register test
       LinExprInt(void) : Test("MiniModel::Lin::Expr::Int",4,-2,2,false) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return 2*x[0]+3*x[1]-x[2] == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVar y = Gecode::post(home, 2*x[0] + 3*x[1] - x[2]);
         rel(home,y,IRT_EQ,x[3],ICL_DOM);
       }
     };
   
     /// Test for posting linear expressions over Boolean variables
     class LinExprBool : public Test {
     public:
       /// Create and register test
       LinExprBool(void) : Test("MiniModel::Lin::Expr::Bool",4,-2,2,false) {}
       /// Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=x.size()-1; i--; )
           if ((x[i] < 0) || (x[i] > 1))
             return false;
         return -2*x[0]+3*x[1]-x[2] == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(x.size()-1);
         for (int i=x.size()-1; i--; )
           y[i] = channel(home,x[i]);
         IntVar z = Gecode::post(home, -2*y[0] + 3*y[1] - y[2]);
         rel(home,z,Gecode::IRT_EQ,x[3]);
       }
     };

     LinEqA lineqa;
     LinEqB lineqb;
     LinEqC lineqc;
     LinEqD lineqd;
     LinEqE lineqe;
     LinEqF lineqf;
   
     LinGr lingr;
     LinLe linle;
   
     LinEqBool lineqbool;
     LinExprInt linexprint;
     LinExprBool linexprbool;
     //@}
   
   }
}}

// STATISTICS: test-int
