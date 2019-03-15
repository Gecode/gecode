/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2019
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

#include <gecode/minimodel.hh>
#include "test/int.hh"

namespace Test { namespace Int {

   /// %Tests for order constraint
   namespace Order {

     /**
      * \defgroup TaskTestIntOrder Order constraint
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test order for integer variables
     class Int : public Test {
     protected:
       /// Processing times
       int p0, p1;
     public:
       /// Create and register test
       Int(int _p0, int _p1)
         : Test("Order::"+str(_p0)+"::"+str(_p1),3,0,6),
           p0(_p0), p1(_p1) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int s0 = x[0], s1 = x[1], b = x[2];
         if (b > 1)
           return false;
         return ((s0+p0<=s1) && (b == 0)) || ((s1+p1<=s0) && (b == 1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         order(home, x[0], p0, x[1], p1, channel(home, x[2]));
       }
     };
     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         for (int i=1; i<=4; i++)
           for (int j=1; j<=4; j++)
             (void) new Int(i,j);
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int

