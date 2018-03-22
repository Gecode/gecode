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

#ifdef GECODE_HAS_MPFR

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
       virtual MaybeType solution(const Assignment& x) const {
         return eq(exp(x[0]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::exp(home, x[0], x[1]);
         else
           Gecode::rel(home, exp(x[0]) == x[1]);
       }
     };

     /// %Test for exponent constraint where solution is ensured
     class ExpXYSol : public Test {
     public:
       /// Create and register test
       ExpXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Transcendental::Exp::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(exp(x[0]), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = exp(x[0]);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
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
       virtual MaybeType solution(const Assignment& x) const {
         return eq(exp(x[0]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::exp(home, x[0], x[0]);
       }
     };

     /// %Test for logarithm constraint
     class LogXY : public Test {
     public:
       /// Create and register test
       LogXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Transcendental::Log::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (x[0].max() < 0.0)
           return MT_FALSE;
         return eq(log(x[0]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::log(home, x[0], x[1]);
         else
           Gecode::rel(home, log(x[0]) == x[1]);
       }
     };

     /// %Test for logarithm constraint where solution is ensured
     class LogXYSol : public Test {
     public:
       /// Create and register test
       LogXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Transcendental::Log::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (x[0].max() < 0.0)
           return MT_FALSE;
         return eq(log(x[0]), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if (x[0].max() < 0.0) return false;
         Gecode::FloatVal d = log(x[0]);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::log(home, x[0], x[1]);
       }
     };

     /// %Test for logarithm constraint with shared variables
     class LogXX : public Test {
     public:
       /// Create and register test
       LogXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Transcendental::Log::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (x[0].max() < 0.0)
           return MT_FALSE;
         return eq(log(x[0]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::log(home, x[0], x[0]);
       }
     };

     /// %Test for logarithm base n constraint
     class LogNXY : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       LogNXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
       : Test("Transcendental::Log::N::"+str(_base)+"::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((x[0].max() <= 0.0) || (base <= 0.0))
           return MT_FALSE;
         return eq(log(x[0]) / log(base), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::log(home, base, x[0], x[1]);
       }
     };

     /// %Test for logarithm base n constraint where solution is ensured
     class LogNXYSol : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       LogNXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
       : Test("Transcendental::Log::N::"+str(_base)+"::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((x[0].max() <= 0.0) || (base <= 0.0))
           return MT_FALSE;
         return eq(log(x[0]) / log(base), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if ((x[0].max() <= 0.0) || (base <= 0.0))
           return false;
         Gecode::FloatVal d = log(x[0])/log(base);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::log(home, base, x[0], x[1]);
       }
     };

     /// %Test for logarithm base n constraint with shared variables
     class LogNXX : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       LogNXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
         : Test("Transcendental::Log::N::"+str(_base)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((x[0].max() <= 0.0) || (base <= 0.0))
           return MT_FALSE;
         return eq(log(x[0]) / log(base), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::log(home, base, x[0], x[0]);
       }
     };

     /// %Test for pow exponent n constraint
     class PowXY : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       PowXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
       : Test("Transcendental::Pow::N::"+str(_base)+"::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (base <= 0.0)
           return MT_FALSE;
         return eq(exp(x[0] * log(base)), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::pow(home, base, x[0], x[1]);
       }
     };

     /// %Test for pow exponent n constraint where solution is ensured
     class PowXYSol : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       PowXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
       : Test("Transcendental::Pow::N::"+str(_base)+"::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if (base <= 0.0)
           return MT_FALSE;
         return eq(exp(x[0] * log(base)), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if (base <= 0.0) return false;
         Gecode::FloatVal d = exp(x[0]*log(base));
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::pow(home, base, x[0], x[1]);
       }
     };

     /// %Test for pow exponent n constraint with shared variables
     class PowXX : public Test {
       Gecode::FloatNum base;
     public:
       /// Create and register test
       PowXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum _base, Gecode::FloatNum st)
       : Test("Transcendental::Pow::N::"+str(_base)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false), base(_base) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((x[0].max() <= 0.0) || (base <= 0.0))
           return MT_FALSE;
         return eq(exp(x[0] * log(base)), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::pow(home, base, x[0], x[0]);
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

     ExpXYSol exp_xy_sol_a("A",a,step);
     ExpXYSol exp_xy_sol_b("B",b,step);
     ExpXYSol exp_xy_sol_c("C",c,step);

     ExpXX exp_xx_a("A",a,step);
     ExpXX exp_xx_b("B",b,step);
     ExpXX exp_xx_c("C",c,step);

     LogXY log_xy_a("A",a,step);
     LogXY log_xy_b("B",b,step);
     LogXY log_xy_c("C",c,step);

     LogXYSol log_xy_sol_a("A",a,step);
     LogXYSol log_xy_sol_b("B",b,step);
     LogXYSol log_xy_sol_c("C",c,step);

     LogXX log_xx_a("A",a,step);
     LogXX log_xx_b("B",b,step);
     LogXX log_xx_c("C",c,step);

     LogNXY logn_xy_a_1("A",a,-1.5,step);
     LogNXY logn_xy_b_1("B",b,-1.5,step);
     LogNXY logn_xy_c_1("C",c,-1.5,step);

     LogNXYSol logn_xy_sol_a_1("A",a,-1.5,step);
     LogNXYSol logn_xy_sol_b_1("B",b,-1.5,step);
     LogNXYSol logn_xy_sol_c_1("C",c,-1.5,step);

     LogNXX logn_xx_a_1("A",a,-1.5,step);
     LogNXX logn_xx_b_1("B",b,-1.5,step);
     LogNXX logn_xx_c_1("C",c,-1.5,step);

     LogNXY logn_xy_a_2("A",a,1.5,step);
     LogNXY logn_xy_b_2("B",b,1.5,step);
     LogNXY logn_xy_c_2("C",c,1.5,step);

     LogNXYSol logn_xy_sol_a_2("A",a,1.5,step);
     LogNXYSol logn_xy_sol_b_2("B",b,1.5,step);
     LogNXYSol logn_xy_sol_c_2("C",c,1.5,step);

     LogNXX logn_xx_a_2("A",a,1.5,step);
     LogNXX logn_xx_b_2("B",b,1.5,step);
     LogNXX logn_xx_c_2("C",c,1.5,step);

     LogNXY logn_xy_a_3("A",a,0,step);
     LogNXY logn_xy_b_3("B",b,0,step);
     LogNXY logn_xy_c_3("C",c,0,step);

     LogNXYSol logn_xy_sol_a_3("A",a,0,step);
     LogNXYSol logn_xy_sol_b_3("B",b,0,step);
     LogNXYSol logn_xy_sol_c_3("C",c,0,step);

     LogNXX logn_xx_a_3("A",a,0,step);
     LogNXX logn_xx_b_3("B",b,0,step);
     LogNXX logn_xx_c_3("C",c,0,step);

     PowXY pow_xy_a_1("A",a,-1.5,step);
     PowXY pow_xy_b_1("B",b,-1.5,step);
     PowXY pow_xy_c_1("C",c,-1.5,step);

     PowXYSol pow_xy_sol_a_1("A",a,-1.5,step);
     PowXYSol pow_xy_sol_b_1("B",b,-1.5,step);
     PowXYSol pow_xy_sol_c_1("C",c,-1.5,step);

     PowXX pow_xx_a_1("A",a,-1.5,step);
     PowXX pow_xx_b_1("B",b,-1.5,step);
     PowXX pow_xx_c_1("C",c,-1.5,step);

     PowXY pow_xy_a_2("A",a,1.5,step);
     PowXY pow_xy_b_2("B",b,1.5,step);
     PowXY pow_xy_c_2("C",c,1.5,step);

     PowXYSol pow_xy_sol_a_2("A",a,1.5,step);
     PowXYSol pow_xy_sol_b_2("B",b,1.5,step);
     PowXYSol pow_xy_sol_c_2("C",c,1.5,step);

     PowXX pow_xx_a_2("A",a,1.5,step);
     PowXX pow_xx_b_2("B",b,1.5,step);
     PowXX pow_xx_c_2("C",c,1.5,step);

     PowXY pow_xy_a_3("A",a,0,step);
     PowXY pow_xy_b_3("B",b,0,step);
     PowXY pow_xy_c_3("C",c,0,step);

     PowXYSol pow_xy_sol_a_3("A",a,0,step);
     PowXYSol pow_xy_sol_b_3("B",b,0,step);
     PowXYSol pow_xy_sol_c_3("C",c,0,step);

     PowXX pow_xx_a_3("A",a,0,step);
     PowXX pow_xx_b_3("B",b,0,step);
     PowXX pow_xx_c_3("C",c,0,step);

     //@}

   }
}}

#endif
// STATISTICS: test-float
