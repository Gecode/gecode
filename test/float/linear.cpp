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

   /// %Tests for linear constraints
   namespace Linear {

     /// Check whether \a has only one coefficients
     bool one(const Gecode::FloatValArgs& a) {
      for (int i=a.size(); i--; )
        if (a[i] != 1)
          return false;
      return true;
    }

     /**
      * \defgroup TaskTestFloatLinear Linear constraints
      * \ingroup TaskTestFloat
      */
     //@{
     /// %Test linear relation over float variables
     class FloatFloat : public Test {
     protected:
       /// Coefficients
       Gecode::FloatValArgs a;
       /// Float relation type to propagate
       Gecode::FloatRelType frt;
       /// Result
       Gecode::FloatNum c;
     public:
       /// Create and register test
       FloatFloat(const std::string& s, const Gecode::FloatVal& d,
                  const Gecode::FloatValArgs& a0, Gecode::FloatRelType frt0,
                  Gecode::FloatNum c0, Gecode::FloatNum st)
         : Test("Linear::Float::"+
                str(frt0)+"::"+s+"::"+str(c0)+"::"
                +str(a0.size()),
                a0.size(),d,st,CPLT_ASSIGNMENT,true),
           a(a0), frt(frt0), c(c0) {
         using namespace Gecode;
         testfix = false;
         if ((frt == FRT_NQ) || (frt == FRT_LE) || (frt == FRT_GR) || reified)
           testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         Gecode::FloatVal e = 0.0;
         for (int i=x.size(); i--; )
           e += a[i]*x[i];
         switch (cmp(e, frt, Gecode::FloatVal(c))) {
         case MT_FALSE: {
           Gecode::FloatVal eError = e;
           for (int i=x.size(); i--; )
             eError -= a[i]*x[i];
           if (cmp(e+eError, frt, Gecode::FloatVal(c)) == MT_FALSE)
             return MT_FALSE;
           else
             return MT_MAYBE;
         }
         case MT_TRUE:
           return MT_TRUE;
         case MT_MAYBE:
           return MT_MAYBE;
         }
         GECODE_NEVER;
         return MT_FALSE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (one(a))
           Gecode::linear(home, x, frt, c);
         else
           Gecode::linear(home, a, x, frt, c);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         if (one(a))
           Gecode::linear(home, x, frt, c, r);
         else
           Gecode::linear(home, a, x, frt, c, r);
       }
     };

     /// %Test linear relation over float variables
     class FloatVar : public Test {
     protected:
       /// Coefficients
       Gecode::FloatValArgs a;
       /// Float relation type to propagate
       Gecode::FloatRelType frt;
     public:
       /// Create and register test
       FloatVar(const std::string& s, const Gecode::FloatVal& d,
                const Gecode::FloatValArgs& a0, Gecode::FloatRelType frt0, Gecode::FloatNum st)
         : Test("Linear::Var::"+
                str(frt0)+"::"+s+"::"+str(a0.size()),
                a0.size()+1,d,st,CPLT_ASSIGNMENT,true),
           a(a0), frt(frt0) {
         using namespace Gecode;
         testfix = false;
         if ((frt == FRT_NQ) || (frt == FRT_LE) || (frt == FRT_GR) || reified)
           testsubsumed = false;
       }
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         Gecode::FloatVal e = 0.0;
         for (int i=a.size(); i--; )
           e += a[i]*x[i];
         switch (cmp(e, frt, x[a.size()])) {
         case MT_FALSE: {
           Gecode::FloatVal eError = e;
           for (int i=a.size(); i--; )
             eError -= a[i]*x[i];
           if (cmp(e+eError, frt, x[a.size()]) == MT_FALSE)
             return MT_FALSE;
           else
             return MT_MAYBE;
         }
         case MT_TRUE:
           return MT_TRUE;
         case MT_MAYBE:
           return MT_MAYBE;
         }
         GECODE_NEVER;
         return MT_FALSE;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         int n = a.size();
         Gecode::FloatVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         if (one(a))
           Gecode::linear(home, y, frt, x[n]);
         else
           Gecode::linear(home, a, y, frt, x[n]);
       }
       /// Post reified constraint on \a x for \a r
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x,
                         Gecode::Reify r) {
         int n = a.size();
         Gecode::FloatVarArgs y(n);
         for (int i=n; i--; )
           y[i] = x[i];
         if (one(a))
           Gecode::linear(home, y, frt, x[n], r);
         else
           Gecode::linear(home, a, y, frt, x[n], r);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {
         using namespace Gecode;
         {
           FloatNum step = 0.7;
           FloatVal f1(-2,2);
           FloatVal f2(-3,-1);
           FloatVal f3(3,8);

           FloatValArgs a1(1);
           a1[0] = 0.0;

           for (FloatRelTypes frts; frts(); ++frts) {
             (void) new FloatFloat("11",f1,a1,frts.frt(),0.0,step);
             (void) new FloatVar("11",f1,a1,frts.frt(),step);
             (void) new FloatFloat("21",f2,a1,frts.frt(),0.0,step);
             (void) new FloatVar("21",f2,a1,frts.frt(),step);
             (void) new FloatFloat("31",f3,a1,frts.frt(),1.0,step);
           }

           const FloatVal av2[4] = {1.0,1.0,1.0,1.0};
           const FloatVal av3[4] = {1.0,-1.0,-1.0,1.0};
           const FloatVal av4[4] = {2.0,3.0,5.0,7.0};
           const FloatVal av5[4] = {-2.0,3.0,-5.0,7.0};

           for (int i=1; i<=4; i++) {
             FloatValArgs a2(i, av2);
             FloatValArgs a3(i, av3);
             FloatValArgs a4(i, av4);
             FloatValArgs a5(i, av5);
             for (FloatRelTypes frts; frts(); ++frts) {
               (void) new FloatFloat("12",f1,a2,frts.frt(),0.0,step);
               (void) new FloatFloat("13",f1,a3,frts.frt(),0.0,step);
               (void) new FloatFloat("14",f1,a4,frts.frt(),0.0,step);
               (void) new FloatFloat("15",f1,a5,frts.frt(),0.0,step);
               (void) new FloatFloat("22",f2,a2,frts.frt(),0.0,step);
               (void) new FloatFloat("23",f2,a3,frts.frt(),0.0,step);
               (void) new FloatFloat("24",f2,a4,frts.frt(),0.0,step);
               (void) new FloatFloat("25",f2,a5,frts.frt(),0.0,step);
               (void) new FloatFloat("32",f3,a2,frts.frt(),1.0,step);
               if (i < 4) {
                 (void) new FloatVar("12",f1,a2,frts.frt(),step);
                 (void) new FloatVar("13",f1,a3,frts.frt(),step);
                 (void) new FloatVar("14",f1,a4,frts.frt(),step);
                 (void) new FloatVar("15",f1,a5,frts.frt(),step);
                 (void) new FloatVar("22",f2,a2,frts.frt(),step);
                 (void) new FloatVar("23",f2,a3,frts.frt(),step);
                 (void) new FloatVar("24",f2,a4,frts.frt(),step);
                 (void) new FloatVar("25",f2,a5,frts.frt(),step);
               }
             }
           }
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-float
