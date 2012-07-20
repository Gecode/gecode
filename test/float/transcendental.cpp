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

#include "test/float.hh"

#include <cmath>
#include <algorithm>

namespace Test { namespace Float {

   /// %Tests for transcendental constraints
   namespace Transcendental {
     
     /// %Test for exponent constraint
     class ExpXY : public Test {
     public:
       /// Create and register test
       ExpXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Transcendental::Exp::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == exp(d0))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::exp(home, x[0], x[1]);
       }
     };

     /// %Test for exponent constraint where solution is ensured
     class ExpXY_Sol : public Test {
     public:
       /// Create and register test
       ExpXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Transcendental::Exp::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == exp(d0))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = exp(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::exp(home, x[0], x[1]);
       }
     };
     
     /// %Test for exponent constraint with shared variables
     class ExpXX : public Test {
     public:
       /// Create and register test
       ExpXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Transcendental::Exp::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == exp(d0))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::exp(home, x[0], x[0]);
       }
     };

     const Gecode::FloatNum step = 0.15;
     const Gecode::FloatNum step2 = 2*step;
     Gecode::FloatVal a(-8,5);
     Gecode::FloatVal b(9,12);
     Gecode::FloatVal c(-8,8);
     
     ExpXY exp_xy_a("A",a,step);
     ExpXY exp_xy_b("B",b,step);
     ExpXY exp_xy_c("C",c,step);

     ExpXY_Sol exp_xy_sol_a("A",a,step);
     ExpXY_Sol exp_xy_sol_b("B",b,step);
     ExpXY_Sol exp_xy_sol_c("C",c,step);
     
     ExpXX exp_xx_a("A",a,step);
     ExpXX exp_xx_b("B",b,step);
     ExpXX exp_xx_c("C",c,step);

     //@}

   }
}}

// STATISTICS: test-float
