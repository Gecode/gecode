/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

   /// %Tests for unsharing variables in arrays
   namespace Unshare {

     /**
      * \defgroup TaskTestIntUnshare Unsharing variables in arrays
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for unsharing integer variables
     class Int : public Test {
     public:
       /// Create and register test
       Int(Gecode::IntPropLevel ipl)
         : Test("Unshare::Int::"+str(ipl),9,-1,1,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0] == x[3]) &&
                 (x[1] == x[4]) && (x[1] == x[6]) &&
                 (x[2] == x[5]) && (x[2] == x[7]) && (x[2] == x[8]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         IntVarArgs y(6);
         y[0]=x[0]; y[1]=y[3]=x[1]; y[2]=y[4]=y[5]=x[2];
         unshare(home, y, ipl);
         for (int i=0; i<6; i++)
           rel(home, y[i], IRT_EQ, x[3+i], IPL_DOM);
       }
     };

     /// %Test for unsharing Boolean variables
     class Bool : public Test {
     public:
       /// Create and register test
       Bool(void)
         : Test("Unshare::Bool",9,0,1,false) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return ((x[0] == x[3]) &&
                 (x[1] == x[4]) && (x[1] == x[6]) &&
                 (x[2] == x[5]) && (x[2] == x[7]) && (x[2] == x[8]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         BoolVarArgs y(6);
         y[0]=channel(home,x[0]);
         y[1]=y[3]=channel(home,x[1]);
         y[2]=y[4]=y[5]=channel(home,x[2]);
         unshare(home, y);
         for (int i=0; i<6; i++)
           rel(home, y[i], IRT_EQ, channel(home,x[3+i]));
       }
     };

     /// %Test for unsharing in failed spaces
     class Failed : public Test {
     public:
       /// Create and register test
       Failed(void)
         : Test("Unshare::Failed",1,-1,1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         (void) x;
         return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         home.fail();
         IntVarArgs y(2);
         y[0]=x[0]; y[1]=x[0];
         unshare(home, y);
         REG r(1);
         extensional(home, y, r);
       }
     };

     Int i_bnd(Gecode::IPL_BND);
     Int i_dom(Gecode::IPL_DOM);

     Bool b;

     Failed f;
     //@}

   }
}}

// STATISTICS: test-int
