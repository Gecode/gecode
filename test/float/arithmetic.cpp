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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for multiplication constraint when solution is ensured
     class MultXYZ_Sol : public Test {
     public:
       /// Create and register test
       MultXYZ_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Mult::XYZ::Sol::"+s,3,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]*x[1];
         if (Gecode::Float::subset(d, dom))
         {
           x.set(2, d);
           return true;
         } else
           return false;
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[1];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[1]);
       }
     };

     /// %Test for multiplication constraint with shared variables when solution is ensured
     class MultXXY_Sol : public Test {
     public:
       /// Create and register test
       MultXXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Mult::XXY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[1];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]*x[0];
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[0];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[1];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[0];
         try {
           return (d2 == d0*d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (d2 == d0/d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::div(home, x[0], x[1], x[2]);
       }
     };
     
     /// %Test for division constraint when solution is ensured
     class Div_Sol : public Test {
     public:
       /// Create and register test
       Div_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Div::Sol::"+s,3,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (d2 == d0/d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = x[0]/x[1];
         if (Gecode::Float::subset(d, dom))
         {
           x.set(2, d);
           return true;
         } else
           return false;
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == d0*d0)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqr(home, x[0], x[1]);
       }
     };

     /// %Test for squaring constraint where solution is ensured
     class SqrXY_Sol : public Test {
     public:
       /// Create and register test
       SqrXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Sqr::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (d1 == d0*d0)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = square(x[0]);
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return (d0 == d0*d0)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return ((d0 >= 0) && (sqrt(d0) == d1))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqrt(home, x[0], x[1]);
       }
     };

     /// %Test for square root constraint where solution is ensured
     class SqrtXY_Sol : public Test {
     public:
       /// Create and register test
       SqrtXY_Sol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Sqrt::XY::Sol::"+s,2,d,st,EXTEND_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return ((d0 >= 0) && (sqrt(d0) == d1))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Extend assignment \a x
       virtual bool extendAssignement(Assignment& x) const {
         Gecode::FloatVal d = sqrt(abs(x[0]));
         if (Gecode::Float::subset(d, dom))
         {
           x.set(1, d);
           return true;
         } else
           return false;
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         try {
           return ((d0 >= 0) && (sqrt(d0) == d0))?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::sqrt(home, x[0], x[0]);
       }
     };

     /// %Test for absolute value constraint
     class AbsXY : public Test {
     public:
       /// Create and register test
       AbsXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Abs::XY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         try {
           return (abs(d0) == d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::abs(home, x[0], x[1]);
       }
     };

     /// %Test for absolute value constraint with shared variables
     class AbsXX : public Test {
     public:
       /// Create and register test
       AbsXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
         : Test("Arithmetic::Abs::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         try {
           return (abs(d0) == d1)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (min(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::min(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXY : public Test {
     public:
       /// Create and register test
       MinXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Min::Bin::XXY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[1];
         try {
           return (min(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[0];
         try {
           return (min(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[1];
         try {
           return (min(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[0];
         try {
           return (min(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[2];
         try {
           return (max(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::max(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXY : public Test {
     public:
       /// Create and register test
       MaxXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
       : Test("Arithmetic::Max::Bin::XXY::"+s,2,d,st,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[1];
         try {
           return (max(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[0];
         try {
           return (max(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[1];
         Gecode::FloatVal d2 = x[1];
         try {
           return (max(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         Gecode::FloatVal d0 = x[0];
         Gecode::FloatVal d1 = x[0];
         Gecode::FloatVal d2 = x[0];
         try {
           return (max(d0,d1) == d2)?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         try {
           return (min(min(x[0],x[1]), x[2]) == x[3])?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[3]);
       }
     };

     /// %Test for n-ary minimmum constraint with shared variables
     class MinNaryShared : public Test {
     public:
       /// Create and register test
       MinNaryShared(void)
          : Test("Arithmetic::Min::Nary::Shared",3,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         try {
           return (min(min(x[0],x[1]), x[2]) == x[1])?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
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
       virtual SolutionTestType solution(const Assignment& x) const {
         try {
           return (max(max(x[0],x[1]), x[2]) == x[3])?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[3]);
       }
     };

     /// %Test for n-ary maximum constraint with shared variables
     class MaxNaryShared : public Test {
     public:
       /// Create and register test
       MaxNaryShared(void)
          : Test("Arithmetic::Max::Nary::Shared",3,-4,4,0.5,CPLT_ASSIGNMENT,false) {}
       /// %Test whether \a x is solution
       virtual SolutionTestType solution(const Assignment& x) const {
         try {
           return (max(max(x[0],x[1]), x[2]) == x[1])?SOLUTION:NO_SOLUTION;
         } catch (boost::numeric::interval_lib::comparison_error&) {
           return UNCERTAIN;
         }         
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
         Gecode::FloatVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[1]);
       }
     };

     const Gecode::FloatNum step = 0.15;
     const Gecode::FloatNum step2 = 2*step;
     Gecode::FloatVal a(-8,5);
     Gecode::FloatVal b(9,12);
     Gecode::FloatVal c(-8,8);

     MultXXY mult_xxy_a("A",a,step);
     MultXXY mult_xxy_b("B",b,step);
     MultXXY mult_xxy_c("C",c,step);

     MultXXY_Sol mult_xxy_sol_a("A",a,step);
     MultXXY_Sol mult_xxy_sol_b("B",b,step);
     MultXXY_Sol mult_xxy_sol_c("C",c,step);
     
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

     MultXYZ_Sol mult_xyz_sol_a("A",a,step);
     MultXYZ_Sol mult_xyz_sol_b("B",b,step);
     MultXYZ_Sol mult_xyz_sol_c("C",c,step);
     
     Div div_a("A",a,step);
     Div div_b("B",b,step);
     Div div_c("C",c,step);
     
     Div_Sol div_sol_a("A",a,step);
     Div_Sol div_sol_b("B",b,step);
     Div_Sol div_sol_c("C",c,step);
     
     SqrXY sqr_xy_a("A",a,step);
     SqrXY sqr_xy_b("B",b,step);
     SqrXY sqr_xy_c("C",c,step);

     SqrXY_Sol sqr_xy_sol_a("A",a,step);
     SqrXY_Sol sqr_xy_sol_b("B",b,step);
     SqrXY_Sol sqr_xy_sol_c("C",c,step);
     
     SqrXX sqr_xx_a("A",a,step);
     SqrXX sqr_xx_b("B",b,step);
     SqrXX sqr_xx_c("C",c,step);

     SqrtXY sqrt_xy_a("A",a,step);
     SqrtXY sqrt_xy_b("B",b,step);
     SqrtXY sqrt_xy_c("C",c,step);

     SqrtXY_Sol sqrt_xy_sol_a("A",a,step);
     SqrtXY_Sol sqrt_xy_sol_b("B",b,step);
     SqrtXY_Sol sqrt_xy_sol_c("C",c,step);
     
     SqrtXX sqrt_xx_a("A",a,step);
     SqrtXX sqrt_xx_b("B",b,step);
     SqrtXX sqrt_xx_c("C",c,step);

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
