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
#ifdef GECODE_HAS_MPFR

#include <cmath>
#include <algorithm>

namespace Test { namespace Float {

   /// %Tests for trigonometric constraints
   namespace Trigonometric {
     
     /// %Test for sinus constraint
     class SinXY : public Test {
     public:
       /// Create and register test
       SinXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Trigonometric::Sin::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == sin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sin(home, x[0], x[1]);
       }
     };

     /// %Test for sinus constraint where solution is ensured
     class SinXY_Sol : public Test {
     public:
       /// Create and register test
       SinXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Sin::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == sin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = sin(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sin(home, x[0], x[1]);
       }
     };
     
     /// %Test for sinus constraint with shared variables
     class SinXX : public Test {
     public:
       /// Create and register test
       SinXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Trigonometric::Sin::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == sin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sin(home, x[0], x[0]);
       }
     };

     /// %Test for cosinus constraint
     class CosXY : public Test {
     public:
       /// Create and register test
       CosXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Cos::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == cos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::cos(home, x[0], x[1]);
       }
     };
     
     /// %Test for cosinus constraint where solution is ensured
     class CosXY_Sol : public Test {
     public:
       /// Create and register test
       CosXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Cos::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == cos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = cos(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::cos(home, x[0], x[1]);
       }
     };
     
     /// %Test for cosinus constraint with shared variables
     class CosXX : public Test {
     public:
       /// Create and register test
       CosXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Cos::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == cos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::cos(home, x[0], x[0]);
       }
     };
     
     /// %Test for tangent constraint
     class TanXY : public Test {
     public:
       /// Create and register test
       TanXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Tan::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == tan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::tan(home, x[0], x[1]);
       }
     };
     
     /// %Test for tangent constraint where solution is ensured
     class TanXY_Sol : public Test {
     public:
       /// Create and register test
       TanXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Tan::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == tan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = tan(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::tan(home, x[0], x[1]);
       }
     };
     
     /// %Test for tangent constraint with shared variables
     class TanXX : public Test {
     public:
       /// Create and register test
       TanXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::Tan::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == tan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::tan(home, x[0], x[0]);
       }
     };
     
     /// %Test for asinus constraint
     class ASinXY : public Test {
     public:
       /// Create and register test
       ASinXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ASin::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == asin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::asin(home, x[0], x[1]);
       }
     };
     
     /// %Test for asinus constraint where solution is ensured
     class ASinXY_Sol : public Test {
     public:
       /// Create and register test
       ASinXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ASin::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == asin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return false;
         Gecode::FloatVal d = asin(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::asin(home, x[0], x[1]);
       }
     };
     
     /// %Test for asinus constraint with shared variables
     class ASinXX : public Test {
     public:
       /// Create and register test
       ASinXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ASin::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == asin(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::asin(home, x[0], x[0]);
       }
     };

     /// %Test for acosinus constraint
     class ACosXY : public Test {
     public:
       /// Create and register test
       ACosXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ACos::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == acos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::acos(home, x[0], x[1]);
       }
     };
     
     /// %Test for acosinus constraint where solution is ensured
     class ACosXY_Sol : public Test {
     public:
       /// Create and register test
       ACosXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ACos::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == acos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return false;
         Gecode::FloatVal d = acos(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::acos(home, x[0], x[1]);
       }
     };
     
     /// %Test for acosinus constraint with shared variables
     class ACosXX : public Test {
     public:
       /// Create and register test
       ACosXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ACos::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         if ((x[0].min() > 1.0) || (x[0].max() < -1.0)) return NO_SOLUTION;
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == acos(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::acos(home, x[0], x[0]);
       }
     };
     
     /// %Test for atangent constraint
     class ATanXY : public Test {
     public:
       /// Create and register test
       ATanXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ATan::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == atan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::atan(home, x[0], x[1]);
       }
     };
     
     /// %Test for atangent constraint where solution is ensured
     class ATanXY_Sol : public Test {
     public:
       /// Create and register test
       ATanXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ATan::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == atan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = atan(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::atan(home, x[0], x[1]);
       }
     };
     
     /// %Test for atangent constraint with shared variables
     class ATanXX : public Test {
     public:
       /// Create and register test
       ATanXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Trigonometric::ATan::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == atan(d0))?SOLUTION:NO_SOLUTION;
         } catch (Gecode::Float::ComparisonError&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::atan(home, x[0], x[0]);
       }
     };

     const Gecode::FloatNum step = 0.15;
     const Gecode::FloatNum step2 = 2*step;
     Gecode::FloatVal a(-8,5);
     Gecode::FloatVal b(9,12);
     Gecode::FloatVal c(-8,8);
     
     SinXY sin_xy_a("A",a,step);
     SinXY sin_xy_b("B",b,step);
     SinXY sin_xy_c("C",c,step);

     SinXY_Sol sin_xy_sol_a("A",a,step);
     SinXY_Sol sin_xy_sol_b("B",b,step);
     SinXY_Sol sin_xy_sol_c("C",c,step);
     
     SinXX sin_xx_a("A",a,step);
     SinXX sin_xx_b("B",b,step);
     SinXX sin_xx_c("C",c,step);

     CosXY cos_xy_a("A",a,step);
     CosXY cos_xy_b("B",b,step);
     CosXY cos_xy_c("C",c,step);
     
     CosXY_Sol cos_xy_sol_a("A",a,step);
     CosXY_Sol cos_xy_sol_b("B",b,step);
     CosXY_Sol cos_xy_sol_c("C",c,step);
     
     CosXX cos_xx_a("A",a,step);
     CosXX cos_xx_b("B",b,step);
     CosXX cos_xx_c("C",c,step);
     
     TanXY tan_xy_a("A",a,step);
     TanXY tan_xy_b("B",b,step);
     TanXY tan_xy_c("C",c,step);
     
     TanXY_Sol tan_xy_sol_a("A",a,step);
     TanXY_Sol tan_xy_sol_b("B",b,step);
     TanXY_Sol tan_xy_sol_c("C",c,step);
     
     TanXX tan_xx_a("A",a,step);
     TanXX tan_xx_b("B",b,step);
     TanXX tan_xx_c("C",c,step);
     
     ASinXY asin_xy_a("A",a,step);
     ASinXY asin_xy_b("B",b,step);
     ASinXY asin_xy_c("C",c,step);
     
     ASinXY_Sol asin_xy_sol_a("A",a,step);
     ASinXY_Sol asin_xy_sol_b("B",b,step);
     ASinXY_Sol asin_xy_sol_c("C",c,step);
     
     ASinXX asin_xx_a("A",a,step);
     ASinXX asin_xx_b("B",b,step);
     ASinXX asin_xx_c("C",c,step);
     
     ACosXY acos_xy_a("A",a,step);
     ACosXY acos_xy_b("B",b,step);
     ACosXY acos_xy_c("C",c,step);
     
     ACosXY_Sol acos_xy_sol_a("A",a,step);
     ACosXY_Sol acos_xy_sol_b("B",b,step);
     ACosXY_Sol acos_xy_sol_c("C",c,step);
     
     ACosXX acos_xx_a("A",a,step);
     ACosXX acos_xx_b("B",b,step);
     ACosXX acos_xx_c("C",c,step);
     
     ATanXY atan_xy_a("A",a,step);
     ATanXY atan_xy_b("B",b,step);
     ATanXY atan_xy_c("C",c,step);
     
     ATanXY_Sol atan_xy_sol_a("A",a,step);
     ATanXY_Sol atan_xy_sol_b("B",b,step);
     ATanXY_Sol atan_xy_sol_c("C",c,step);
     
     ATanXX atan_xx_a("A",a,step);
     ATanXX atan_xx_b("B",b,step);
     ATanXX atan_xx_c("C",c,step);
     
     //@}

   }
}}

#endif
// STATISTICS: test-float
