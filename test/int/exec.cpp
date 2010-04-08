/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Test { namespace Int {

   /// %Tests for synchronized execution
   namespace Exec {

     /**
      * \defgroup TaskTestIntExec Synchronized execution
      * \ingroup TaskTestInt
      */
     //@{
     /// Simple test for wait (integer variables)
     class IntWait : public Test {
     public:
       /// Create and register test
       IntWait(int n) 
         : Test("Wait::Int::"+str(n),n,0,n,false) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         for (int i=0; i<x.size(); i++)
           for (int j=i+1; j<x.size(); j++)
             if (x[i] == x[j])
               return false;
         return true;
       }
       /// Post wait on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         if (x.size() > 1)
           Gecode::wait(home, x, &c);
         else
           Gecode::wait(home, x[0], &c);
       }
       /// Continuation to be executed
       static void c(Gecode::Space& _home) {
         TestSpace& home = static_cast<TestSpace&>(_home);
         for (int i=0; i<home.x.size(); i++)
           for (int j=i+1; j<home.x.size(); j++)
             if (home.x[i].val() == home.x[j].val())
               home.fail();
       }
     };

     /// Simple test for wait (Boolean variables)
     class BoolWait : public Test {
     public:
       /// Create and register test
       BoolWait(int n) 
         : Test("Wait::Bool::"+str(n),n,0,1,false) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int t=0;
         for (int i=0; i<x.size(); i++)
           t += x[i];
         return t==2;
       }
       /// Post wait on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVarArgs b(x.size());
         for (int i=b.size(); i--; )
           b[i]=Gecode::channel(home,x[i]);
         if (b.size() > 1)
           Gecode::wait(home, b, &c);
         else
           Gecode::wait(home, b[0], &c);
       }
       /// Continuation to be executed
       static void c(Gecode::Space& _home) {
         TestSpace& home = static_cast<TestSpace&>(_home);
         int t=0;
         for (int i=0; i<home.x.size(); i++)
           t += home.x[i].val();
         if (t!=2)
           home.fail();
       }
     };

     /// Simple test for when
     class When : public Test {
     public:
       /// Create and register test
       When(void) : Test("When",1,0,1,false) {}
       /// Check whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return x[0]==0;
       }
       /// Post when on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::when(home, Gecode::channel(home, x[0]), &t, &e);
       }
       /// Then-function to be executed
       static void t(Gecode::Space& home) {
         home.fail();
       }
       /// Else-function to be executed
       static void e(Gecode::Space& home) {
         (void) home;
       }
     };

     IntWait iw1(1), iw2(2), iw3(3), iw4(4);
     BoolWait bw1(1), bw2(2), bw3(3), bw4(4);

     When when;
     //@}

   }

}}

// STATISTICS: test-int
