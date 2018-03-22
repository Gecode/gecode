/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Vincent Barichard, 2012
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

#include "test/float.hh"

#include <gecode/minimodel.hh>

namespace Test { namespace Float {

   /// %Tests for domain constraints
   namespace Dom {

     /**
      * \defgroup TaskTestFloatDom Domain constraints
      * \ingroup TaskTestFloat
      */
     //@{
     /// %Test for domain constraints with a float value
     class Val : public Test {
     protected:
       /// Float constant
       Gecode::FloatVal c;
     public:
       /// Create and register test
       Val(int n, Gecode::FloatVal c0, Gecode::FloatNum st)
         : Test("Dom::Val::"+str(n)+"::"+str(c0),
                n,-3,3,st,CPLT_ASSIGNMENT,n==1), c(c0) {
         testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         for (int i=x.size(); i--; )
           if ((x[i].max() > c.max()) || (x[i].min() < c.min()))
             return MT_FALSE;
         return MT_TRUE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (x.size() == 1)
           Gecode::dom(home, x[0], c);
         else
           Gecode::dom(home, x, c);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 1);
         Gecode::dom(home, x[0], c, r);
       }
     };

     /// %Test for domain constraints with float numbers
     class Num : public Test {
     protected:
       /// Float number
       Gecode::FloatNum min;
       /// Float number
       Gecode::FloatNum max;
     public:
       /// Create and register test
       Num(int n, Gecode::FloatNum min0, Gecode::FloatNum max0,
           Gecode::FloatNum st)
         : Test("Dom::Num::"+str(n)+"::"+str(min0)+"::"+str(max0),
                n,-3,3,st,CPLT_ASSIGNMENT,n==1), min(min0), max(max0) {
         testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (max < min)
           return MT_FALSE;
         for (int i=x.size(); i--; )
           if ((x[i].max() > max) || (x[i].min() < min))
             return MT_FALSE;
         return MT_TRUE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (x.size() == 1)
           Gecode::dom(home, x[0], min, max);
         else
           Gecode::dom(home, x, min, max);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 1);
         Gecode::dom(home, x[0], min, max, r);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         FloatNum step = 0.7;
         for (int c=-4; c<=4; c++) {
           (void) new Val(1,c,step);
           (void) new Val(2,c,step);
           for (int d=-3; d<=3; d++) {
             (void) new Num(1,c,d,step);
             (void) new Num(2,c,d,step);
           }
         }
       }
     };

     Create c;
     //@}

   }

}}

// STATISTICS: test-float
