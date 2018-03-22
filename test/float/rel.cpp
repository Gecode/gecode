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

   /// %Tests for relation constraints
   namespace Rel {

     /**
      * \defgroup TaskTestFloatRel Relation constraints
      * \ingroup TaskTestFloat
      */
     //@{
     /// %Test for simple relation involving float variables
     class FloatVarXY : public Test {
     protected:
       /// Float relation type to propagate
       Gecode::FloatRelType frt;
     public:
       /// Create and register test
       FloatVarXY(Gecode::FloatRelType frt0, int n, Gecode::FloatNum st)
         : Test("Rel::Float::Var::XY::"+str(frt0)+"::"+str(n),
                n+1,-3,3,st,CPLT_ASSIGNMENT,n==1),
           frt(frt0) {
         testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (x.size() == 2) {
           return cmp(x[0],frt,x[1]);
         } else {
           MaybeType r1 = cmp(x[0],frt,x[2]);
           MaybeType r2 = cmp(x[1],frt,x[2]);
           if ((r1 == MT_TRUE) && (r2 == MT_TRUE)) return MT_TRUE;
           else if ((r1 == MT_FALSE) || (r2 == MT_FALSE)) return MT_FALSE;
           else return MT_MAYBE;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         using namespace Gecode;
         if (x.size() == 2) {
           rel(home, x[0], frt, x[1]);
         } else {
           FloatVarArgs y(2);
           y[0]=x[0]; y[1]=x[1];
           rel(home, y, frt, x[2]);
         }
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 2);
         Gecode::rel(home, x[0], frt, x[1], r);
       }
     };

     /// %Test for simple relation involving shared float variables
     class FloatVarXX : public Test {
     protected:
       /// Float relation type to propagate
       Gecode::FloatRelType frt;
     public:
       /// Create and register test
       FloatVarXX(Gecode::FloatRelType frt0, Gecode::FloatNum st)
         : Test("Rel::Float::Var::XX::"+str(frt0),
                1,-3,3,st,CPLT_ASSIGNMENT,true),
           frt(frt0) {
         testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return cmp(x[0],frt,x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::rel(home, x[0], frt, x[0]);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         Gecode::rel(home, x[0], frt, x[0], r);
       }
     };

     /// %Test for simple relation involving float variable and float constant
     class FloatFloat : public Test {
     protected:
       /// Float relation type to propagate
       Gecode::FloatRelType frt;
       /// Float constant
       Gecode::FloatVal c;
     public:
       /// Create and register test
       FloatFloat(Gecode::FloatRelType frt0, int n, Gecode::FloatNum c0,
                  Gecode::FloatNum st)
         : Test("Rel::Float::Float::"+str(frt0)+"::"+str(n)+"::"+str(c0),
                n,-3,3,st,CPLT_ASSIGNMENT,n==1),
           frt(frt0), c(c0) {
         testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (x.size() == 1) {
           return cmp(x[0],frt,c);
         } else {
           return cmp(x[0],frt,c) & cmp(x[1],frt,c);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         using namespace Gecode;
         if (x.size() == 1)
           rel(home, x[0], frt, c);
         else
           rel(home, x, frt, c);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         assert(x.size() == 1);
         Gecode::rel(home, x[0], frt, c, r);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         Gecode::FloatNum step = 0.7;
         for (FloatRelTypes frts; frts(); ++frts) {
          (void) new FloatVarXY(frts.frt(),1,step);
          (void) new FloatVarXY(frts.frt(),2,step);
          (void) new FloatVarXX(frts.frt(),step);
           for (int c=-4; c<=4; c++) {
             (void) new FloatFloat(frts.frt(),1,c,step);
             (void) new FloatFloat(frts.frt(),2,c,step);
           }
         }
       }
     };

     Create c;
     //@}

   }

}}

// STATISTICS: test-float
