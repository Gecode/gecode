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

#include <cmath>
#include <algorithm>

namespace Test { namespace Float {

   /// %Tests for arithmetic constraints
   namespace Arithmetic {

     /**
      * \defgroup TaskTestFloatArithmetic Arithmetic constraints
      * \ingroup TaskTestFloat
      */
     //@{
     /// %Test for multiplication constraint
     class MultXYZ : public Test {
     public:
       /// Create and register test
       MultXYZ(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XYZ::"+s,3,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[1], x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::mult(home, x[0], x[1], x[2]);
         else
           Gecode::rel(home, x[0] * x[1] == x[2]);
       }
     };

     /// %Test for multiplication constraint when solution is ensured
     class MultXYZSol : public Test {
     public:
       /// Create and register test
       MultXYZSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XYZ::Sol::"+s,3,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[1], x[2]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]*x[1];
         if (Gecode::Float::subset(d, dom)) {
           x.set(2, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXY : public Test {
     public:
       /// Create and register test
       MultXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XXY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[1]);
       }
     };

     /// %Test for multiplication constraint with shared variables when solution is ensured
     class MultXXYSol : public Test {
     public:
       /// Create and register test
       MultXXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Mult::XXY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]*x[0];
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[1]);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYX : public Test {
     public:
       /// Create and register test
       MultXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XYX::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[1], x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[0]);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYY : public Test {
     public:
       /// Create and register test
       MultXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XYY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[1], x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[1]);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXX : public Test {
     public:
       /// Create and register test
       MultXXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Mult::XXX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[0]);
       }
     };

     /// %Test for division constraint
     class Div : public Test {
     public:
       /// Create and register test
       Div(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Div::"+s,3,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] / x[1], x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::div(home, x[0], x[1], x[2]);
         else
           Gecode::rel(home, x[0] / x[1] == x[2]);
       }
     };

     /// %Test for division constraint when solution is ensured
     class DivSol : public Test {
     public:
       /// Create and register test
       DivSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Div::Sol::"+s,3,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] / x[1], x[2]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]/x[1];
         if (Gecode::Float::subset(d, dom)) {
           x.set(2, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::div(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for squaring constraint
     class SqrXY : public Test {
     public:
       /// Create and register test
       SqrXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Sqr::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::sqr(home, x[0], x[1]);
         else
           Gecode::rel(home, sqr(x[0]) == x[1]);
       }
     };

     /// %Test for squaring constraint where solution is ensured
     class SqrXYSol : public Test {
     public:
       /// Create and register test
       SqrXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Sqr::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = sqr(x[0]);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqr(home, x[0], x[1]);
       }
     };

     /// %Test for squaring constraint with shared variables
     class SqrXX : public Test {
     public:
       /// Create and register test
       SqrXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Sqr::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(x[0] * x[0], x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqr(home, x[0], x[0]);
       }
     };

     /// %Test for square root constraint
     class SqrtXY : public Test {
     public:
       /// Create and register test
       SqrtXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Sqrt::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
         case MT_FALSE: return MT_FALSE;
         case MT_MAYBE: return MT_MAYBE;
         default:
           return eq(sqrt(x[0]), x[1]);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::sqrt(home, x[0], x[1]);
         else
           Gecode::rel(home, sqrt(x[0]) == x[1]);
       }
     };

     /// %Test for square root constraint where solution is ensured
     class SqrtXYSol : public Test {
     public:
       /// Create and register test
       SqrtXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Sqrt::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
         case MT_FALSE: return MT_FALSE;
         case MT_MAYBE: return MT_MAYBE;
         default:
           return eq(sqrt(x[0]), x[1]);
         }
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = sqrt(abs(x[0]));
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqrt(home, x[0], x[1]);
       }
     };

     /// %Test for square root constraint with shared variables
     class SqrtXX : public Test {
     public:
       /// Create and register test
       SqrtXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Sqrt::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
         case MT_FALSE: return MT_FALSE;
         case MT_MAYBE: return MT_MAYBE;
         default:
           return eq(sqrt(x[0]), x[0]);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqrt(home, x[0], x[0]);
       }
     };

     /// %Test for pow  constraint
     class PowXY : public Test {
       unsigned int n;
     public:
       /// Create and register test
       PowXY(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::Pow::N::"+str(_n)+"::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false), n(_n) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(pow(x[0],n), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::pow(home, x[0], n, x[1]);
         else
           Gecode::rel(home, pow(x[0],n) == x[1]);
       }
     };

     /// %Test for pow  constraint where solution is ensured
     class PowXYSol : public Test {
       unsigned int n;
     public:
       /// Create and register test
       PowXYSol(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::Pow::N::"+str(_n)+"::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false), n(_n) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(pow(x[0],n), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = pow(x[0],n);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::pow(home, x[0], n, x[1]);
       }
     };

     /// %Test for pow  constraint with shared variables
     class PowXX : public Test {
       unsigned int n;
     public:
       /// Create and register test
       PowXX(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::Pow::N::"+str(_n)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(pow(x[0],n), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::pow(home, x[0], n, x[0]);
       }
     };

     /// %Test for nroot  constraint
     class NRootXY : public Test {
       unsigned int n;
     public:
       /// Create and register test
       NRootXY(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::NRoot::N::"+str(_n)+"::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false), n(_n) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((n == 0) || (x[0].max() < 0.0))
           return MT_FALSE;
         return eq(nroot(x[0],n), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::nroot(home, x[0], n, x[1]);
         else
           Gecode::rel(home, nroot(x[0],n) == x[1]);
       }
     };

     /// %Test for nroot  constraint where solution is ensured
     class NRootXYSol : public Test {
       unsigned int n;
     public:
       /// Create and register test
       NRootXYSol(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::NRoot::N::"+str(_n)+"::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false), n(_n) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((n == 0) || (x[0].max() < 0.0))
           return MT_FALSE;
         return eq(nroot(x[0],n), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if ((n == 0) || (x[0].max() < 0))
           return false;
         Gecode::FloatVal d = nroot(x[0],n);
         if (Gecode::Float::subset(d, dom)) {
           x.set(1, d);
           return true;
         } else {
           return false;
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::nroot(home, x[0], n, x[1]);
       }
     };

     /// %Test for nroot  constraint with shared variables
     class NRootXX : public Test {
       unsigned int n;
     public:
       /// Create and register test
       NRootXX(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
       : Test("Arithmetic::NRoot::N::"+str(_n)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((n == 0) || (x[0].max() < 0))
           return MT_FALSE;
         return eq(nroot(x[0],n), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::nroot(home, x[0], n, x[0]);
       }
     };

     /// %Test for absolute value constraint
     class AbsXY : public Test {
     public:
       /// Create and register test
       AbsXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Abs::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(abs(x[0]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::abs(home, x[0], x[1]);
         else
           Gecode::rel(home, abs(x[0]) == x[1]);
       }
     };

     /// %Test for absolute value constraint with shared variables
     class AbsXX : public Test {
     public:
       /// Create and register test
       AbsXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Abs::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(abs(x[0]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::abs(home, x[0], x[0]);
       }
     };

     /// %Test for binary minimum constraint
     class MinXYZ : public Test {
     public:
       /// Create and register test
       MinXYZ(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Min::Bin::XYZ::"+s,3,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(x[0],x[1]), x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::min(home, x[0], x[1], x[2]);
         else
           Gecode::rel(home, min(x[0],x[1]) == x[2]);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXY : public Test {
     public:
       /// Create and register test
       MinXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Min::Bin::XXY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(x[0],x[0]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::min(home, x[0], x[0], x[1]);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYX : public Test {
     public:
       /// Create and register test
       MinXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Min::Bin::XYX::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(x[0],x[1]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::min(home, x[0], x[1], x[0]);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYY : public Test {
     public:
       /// Create and register test
       MinXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Min::Bin::XYY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(x[0],x[1]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::min(home, x[0], x[1], x[1]);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXX : public Test {
     public:
       /// Create and register test
       MinXXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Min::Bin::XXX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(x[0],x[0]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::min(home, x[0], x[0], x[0]);
       }
     };

     /// %Test for binary maximum constraint
     class MaxXYZ : public Test {
     public:
       /// Create and register test
       MaxXYZ(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Max::Bin::XYZ::"+s,3,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(x[0],x[1]), x[2]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         if (flip())
           Gecode::max(home, x[0], x[1], x[2]);
         else
           Gecode::rel(home, max(x[0], x[1]) == x[2]);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXY : public Test {
     public:
       /// Create and register test
       MaxXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Max::Bin::XXY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(x[0],x[0]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::max(home, x[0], x[0], x[1]);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYX : public Test {
     public:
       /// Create and register test
       MaxXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Max::Bin::XYX::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(x[0],x[1]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::max(home, x[0], x[1], x[0]);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYY : public Test {
     public:
       /// Create and register test
       MaxXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Max::Bin::XYY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(x[0],x[1]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::max(home, x[0], x[1], x[1]);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXX : public Test {
     public:
       /// Create and register test
       MaxXXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Max::Bin::XXX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(x[0],x[0]), x[0]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::max(home, x[0], x[0], x[0]);
       }
     };

     /// %Test for n-ary minimmum constraint
     class MinNary : public Test {
     public:
       /// Create and register test
       MinNary(void)
         : Test("Arithmetic::Min::Nary",4,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(min(x[0],x[1]),x[2]), x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         if (flip())
           Gecode::min(home, m, x[3]);
         else
           Gecode::rel(home, min(m) == x[3]);
       }
     };

     /// %Test for n-ary minimmum constraint with shared variables
     class MinNaryShared : public Test {
     public:
       /// Create and register test
       MinNaryShared(void)
          : Test("Arithmetic::Min::Nary::Shared",3,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(min(min(x[0],x[1]),x[2]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[1]);
       }
     };

     /// %Test for n-ary maximum constraint
     class MaxNary : public Test {
     public:
       /// Create and register test
       MaxNary(void)
          : Test("Arithmetic::Max::Nary",4,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(max(x[0],x[1]),x[2]), x[3]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         if (flip())
           Gecode::max(home, m, x[3]);
         else
           Gecode::rel(home, max(m) == x[3]);
       }
     };

     /// %Test for n-ary maximum constraint with shared variables
     class MaxNaryShared : public Test {
     public:
       /// Create and register test
       MaxNaryShared(void)
          : Test("Arithmetic::Max::Nary::Shared",3,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         return eq(max(max(x[0],x[1]),x[2]), x[1]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[1]);
       }
     };

     const Gecode::FloatNum step = 0.15;
     Gecode::FloatVal a(-8,5);
     Gecode::FloatVal b(9,12);
     Gecode::FloatVal c(-8,8);

     MultXXY mult_xxy_a("A",a,step);
     MultXXY mult_xxy_b("B",b,step);
     MultXXY mult_xxy_c("C",c,step);

     MultXXYSol mult_xxy_sol_a("A",a,step);
     MultXXYSol mult_xxy_sol_b("B",b,step);
     MultXXYSol mult_xxy_sol_c("C",c,step);

     MultXYX mult_xyx_a("A",a,step);
     MultXYX mult_xyx_b("B",b,step);
     MultXYX mult_xyx_c("C",c,step);

     MultXYY mult_xyy_a("A",a,step);
     MultXYY mult_xyy_b("B",b,step);
     MultXYY mult_xyy_c("C",c,step);

     MultXXX mult_xxx_a("A",a,step);
     MultXXX mult_xxx_b("B",b,step);
     MultXXX mult_xxx_c("C",c,step);

     MultXYZ mult_xyz_a("A",a,step);
     MultXYZ mult_xyz_b("B",b,step);
     MultXYZ mult_xyz_c("C",c,step);

     MultXYZSol mult_xyz_sol_a("A",a,step);
     MultXYZSol mult_xyz_sol_b("B",b,step);
     MultXYZSol mult_xyz_sol_c("C",c,step);

     Div div_a("A",a,step);
     Div div_b("B",b,step);
     Div div_c("C",c,step);

     DivSol div_sol_a("A",a,step);
     DivSol div_sol_b("B",b,step);
     DivSol div_sol_c("C",c,step);

     SqrXY sqr_xy_a("A",a,step);
     SqrXY sqr_xy_b("B",b,step);
     SqrXY sqr_xy_c("C",c,step);

     SqrXYSol sqr_xy_sol_a("A",a,step);
     SqrXYSol sqr_xy_sol_b("B",b,step);
     SqrXYSol sqr_xy_sol_c("C",c,step);

     SqrXX sqr_xx_a("A",a,step);
     SqrXX sqr_xx_b("B",b,step);
     SqrXX sqr_xx_c("C",c,step);

     SqrtXY sqrt_xy_a("A",a,step);
     SqrtXY sqrt_xy_b("B",b,step);
     SqrtXY sqrt_xy_c("C",c,step);

     SqrtXYSol sqrt_xy_sol_a("A",a,step);
     SqrtXYSol sqrt_xy_sol_b("B",b,step);
     SqrtXYSol sqrt_xy_sol_c("C",c,step);

     SqrtXX sqrt_xx_a("A",a,step);
     SqrtXX sqrt_xx_b("B",b,step);
     SqrtXX sqrt_xx_c("C",c,step);

     PowXY pow_xy_a_1("A",a,2,step);
     PowXY pow_xy_b_1("B",b,2,step);
     PowXY pow_xy_c_1("C",c,2,step);

     PowXYSol pow_xy_sol_a_1("A",a,2,step);
     PowXYSol pow_xy_sol_b_1("B",b,2,step);
     PowXYSol pow_xy_sol_c_1("C",c,2,step);

     PowXX pow_xx_a_1("A",a,2,step);
     PowXX pow_xx_b_1("B",b,2,step);
     PowXX pow_xx_c_1("C",c,2,step);

     PowXY pow_xy_a_2("A",a,3,step);
     PowXY pow_xy_b_2("B",b,3,step);
     PowXY pow_xy_c_2("C",c,3,step);

     PowXYSol pow_xy_sol_a_2("A",a,3,step);
     PowXYSol pow_xy_sol_b_2("B",b,3,step);
     PowXYSol pow_xy_sol_c_2("C",c,3,step);

     PowXX pow_xx_a_2("A",a,3,step);
     PowXX pow_xx_b_2("B",b,3,step);
     PowXX pow_xx_c_2("C",c,3,step);

     PowXY pow_xy_a_3("A",a,0,step);
     PowXY pow_xy_b_3("B",b,0,step);
     PowXY pow_xy_c_3("C",c,0,step);

     PowXYSol pow_xy_sol_a_3("A",a,0,step);
     PowXYSol pow_xy_sol_b_3("B",b,0,step);
     PowXYSol pow_xy_sol_c_3("C",c,0,step);

     PowXX pow_xx_a_3("A",a,0,step);
     PowXX pow_xx_b_3("B",b,0,step);
     PowXX pow_xx_c_3("C",c,0,step);

     NRootXY nroot_xy_a_1("A",a,2,step);
     NRootXY nroot_xy_b_1("B",b,2,step);
     NRootXY nroot_xy_c_1("C",c,2,step);

     NRootXYSol nroot_xy_sol_a_1("A",a,2,step);
     NRootXYSol nroot_xy_sol_b_1("B",b,2,step);
     NRootXYSol nroot_xy_sol_c_1("C",c,2,step);

     NRootXX nroot_xx_a_1("A",a,2,step);
     NRootXX nroot_xx_b_1("B",b,2,step);
     NRootXX nroot_xx_c_1("C",c,2,step);

     NRootXY nroot_xy_a_2("A",a,3,step);
     NRootXY nroot_xy_b_2("B",b,3,step);
     NRootXY nroot_xy_c_2("C",c,3,step);

     NRootXYSol nroot_xy_sol_a_2("A",a,3,step);
     NRootXYSol nroot_xy_sol_b_2("B",b,3,step);
     NRootXYSol nroot_xy_sol_c_2("C",c,3,step);

     NRootXX nroot_xx_a_2("A",a,3,step);
     NRootXX nroot_xx_b_2("B",b,3,step);
     NRootXX nroot_xx_c_2("C",c,3,step);

     NRootXY nroot_xy_a_3("A",a,0,step);
     NRootXY nroot_xy_b_3("B",b,0,step);
     NRootXY nroot_xy_c_3("C",c,0,step);

     NRootXYSol nroot_xy_sol_a_3("A",a,0,step);
     NRootXYSol nroot_xy_sol_b_3("B",b,0,step);
     NRootXYSol nroot_xy_sol_c_3("C",c,0,step);

     NRootXX nroot_xx_a_3("A",a,0,step);
     NRootXX nroot_xx_b_3("B",b,0,step);
     NRootXX nroot_xx_c_3("C",c,0,step);

     AbsXY abs_xy_a("A",a,step);
     AbsXY abs_xy_b("B",b,step);
     AbsXY abs_xy_c("C",c,step);

     AbsXX abs_xx_a("A",a,step);
     AbsXX abs_xx_b("B",b,step);
     AbsXX abs_xx_c("C",c,step);

     MinXYZ min_xyz_a("A",a,step);
     MinXYZ min_xyz_b("B",b,step);
     MinXYZ min_xyz_c("C",c,step);

     MinXXY min_xxy_a("A",a,step);
     MinXXY min_xxy_b("B",b,step);
     MinXXY min_xxy_c("C",c,step);

     MinXYX min_xyx_a("A",a,step);
     MinXYX min_xyx_b("B",b,step);
     MinXYX min_xyx_c("C",c,step);

     MinXYY min_xyy_a("A",a,step);
     MinXYY min_xyy_b("B",b,step);
     MinXYY min_xyy_c("C",c,step);

     MinXXX min_xxx_a("A",a,step);
     MinXXX min_xxx_b("B",b,step);
     MinXXX min_xxx_c("C",c,step);

     MaxXYZ max_xyz_a("A",a,step);
     MaxXYZ max_xyz_b("B",b,step);
     MaxXYZ max_xyz_c("C",c,step);

     MaxXXY max_xxy_a("A",a,step);
     MaxXXY max_xxy_b("B",b,step);
     MaxXXY max_xxy_c("C",c,step);

     MaxXYX max_xyx_a("A",a,step);
     MaxXYX max_xyx_b("B",b,step);
     MaxXYX max_xyx_c("C",c,step);

     MaxXYY max_xyy_a("A",a,step);
     MaxXYY max_xyy_b("B",b,step);
     MaxXYY max_xyy_c("C",c,step);

     MaxXXX max_xxx_a("A",a,step);
     MaxXXX max_xxx_b("B",b,step);
     MaxXXX max_xxx_c("C",c,step);

     MinNary       min_nary;
     MinNaryShared min_s_nary;
     MaxNary       max_nary;
     MaxNaryShared max_s_nary;
     //@}

   }
}}

// STATISTICS: test-float
