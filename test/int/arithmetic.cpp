/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"

#include <cmath>
#include <algorithm>

namespace Test { namespace Int {

   /// %Tests for arithmetic constraints
   namespace Arithmetic {

     /**
      * \defgroup TaskTestIntArithmetic Arithmetic constraints
      * \ingroup TaskTestInt
      */
     //@{
     /// %Test for multiplication constraint
     class MultXYZ : public Test {
     public:
       /// Create and register test
       MultXYZ(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntConLevel icl)
         : Test("Arithmetic::Mult::XYZ::"+str(icl)+"::"+s,3,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[2], icl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXY : public Test {
     public:
       /// Create and register test
       MultXXY(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntConLevel icl)
         : Test("Arithmetic::Mult::XXY::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         double d2 = static_cast<double>(x[1]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[1], icl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYX : public Test {
     public:
       /// Create and register test
       MultXYX(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntConLevel icl)
         : Test("Arithmetic::Mult::XYX::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[0]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[0], icl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYY : public Test {
     public:
       /// Create and register test
       MultXYY(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntConLevel icl)
         : Test("Arithmetic::Mult::XYY::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[1]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[1], icl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXX : public Test {
     public:
       /// Create and register test
       MultXXX(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntConLevel icl)
         : Test("Arithmetic::Mult::XXX::"+str(icl)+"::"+s,1,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         double d2 = static_cast<double>(x[0]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[0], icl);
       }
     };

     /// %Test for squaring constraint
     class SqrXY : public Test {
     public:
       /// Create and register test
       SqrXY(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntConLevel icl)
         : Test("Arithmetic::Sqr::XY::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return d0*d0 == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqr(home, x[0], x[1], icl);
       }
     };

     /// %Test for squaring constraint with shared variables
     class SqrXX : public Test {
     public:
       /// Create and register test
       SqrXX(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntConLevel icl)
         : Test("Arithmetic::Sqr::XX::"+str(icl)+"::"+s,1,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         return d0*d0 == d0;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqr(home, x[0], x[0], icl);
       }
     };

     /// %Test for square root constraint
     class SqrtXY : public Test {
     public:
       /// Create and register test
       SqrtXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Sqrt::XY::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0 >= 0) && (d0 >= d1*d1) && (d0 < (d1+1)*(d1+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqrt(home, x[0], x[1], icl);
       }
     };

     /// %Test for square root constraint with shared variables
     class SqrtXX : public Test {
     public:
       /// Create and register test
       SqrtXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Sqrt::XX::"+str(icl)+"::"+s,1,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         return (d0 >= 0) && (d0 >= d0*d0) && (d0 < (d0+1)*(d0+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqrt(home, x[0], x[0], icl);
       }
     };

     /// %Test for division/modulo constraint
     class DivMod : public Test {
     private:
       static int abs(int a) { return a<0 ? -a:a; }
       static int sgn(int a) { return a<0 ? -1:1; }
     public:
       /// Create and register test
       DivMod(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::DivMod::"+s,4,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return x[0] == x[1]*x[2]+x[3] &&
                abs(x[3]) < abs(x[1]) &&
                (x[3] == 0 || sgn(x[3]) == sgn(x[0]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::divmod(home, x[0], x[1], x[2], x[3], icl);
       }
     };

     /// %Test for division constraint
     class Div : public Test {
     public:
       /// Create and register test
       Div(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Div::"+s,3,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x[1] == 0)
           return false;
         int divsign = (x[0] / x[1] < 0) ? -1 : 1;
         int divresult =
           divsign *
           static_cast<int>(floor(static_cast<double>(std::abs(x[0]))/
                                  static_cast<double>(std::abs(x[1]))));
         return x[2] == divresult;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::div(home, x[0], x[1], x[2], icl);
       }
     };

     /// %Test for modulo constraint
     class Mod : public Test {
     public:
       /// Create and register test
       Mod(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Mod::"+s,3,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (x[1] == 0)
           return false;
         int divsign = (x[0] / x[1] < 0) ? -1 : 1;
         int divresult =
           divsign *
           static_cast<int>(floor(static_cast<double>(std::abs(x[0]))/
                                  static_cast<double>(std::abs(x[1]))));
         return x[0] == x[1]*divresult+x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mod(home, x[0], x[1], x[2], icl);
       }
     };

     /// %Test for absolute value constraint
     class AbsXY : public Test {
     public:
       /// Create and register test
       AbsXY(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntConLevel icl)
         : Test("Arithmetic::Abs::XY::"+str(icl)+"::"+s,2,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0<0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::abs(home, x[0], x[1], icl);
       }
     };

     /// %Test for absolute value constraint with shared variables
     class AbsXX : public Test {
     public:
       /// Create and register test
       AbsXX(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntConLevel icl)
         : Test("Arithmetic::Abs::XX::"+str(icl)+"::"+s,1,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         return (d0<0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::abs(home, x[0], x[0], icl);
       }
     };

     /// %Test for binary minimum constraint
     class MinXYZ : public Test {
     public:
       /// Create and register test
       MinXYZ(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Bin::XYZ::"+str(icl)+"::"+s,3,d,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[2], icl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXY : public Test {
     public:
       /// Create and register test
       MinXXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Bin::XYX::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[0], x[1], icl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYX : public Test {
     public:
       /// Create and register test
       MinXYX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Bin::XYX::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[0], icl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYY : public Test {
     public:
       /// Create and register test
       MinXYY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Bin::XYY::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[1], icl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXX : public Test {
     public:
       /// Create and register test
       MinXXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Bin::XXX::"+str(icl)+"::"+s,1,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[0]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[0], x[0], icl);
       }
     };

     /// %Test for binary maximum constraint
     class MaxXYZ : public Test {
     public:
       /// Create and register test
       MaxXYZ(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Bin::XYZ::"+str(icl)+"::"+s,3,d) {
         contest = CTL_BOUNDS_Z;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[2], icl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXY : public Test {
     public:
       /// Create and register test
       MaxXXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Bin::XXY::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[0], x[1], icl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYX : public Test {
     public:
       /// Create and register test
       MaxXYX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Bin::XYX::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[0], icl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYY : public Test {
     public:
       /// Create and register test
       MaxXYY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Bin::XYY::"+str(icl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[1], icl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXX : public Test {
     public:
       /// Create and register test
       MaxXXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Bin::XXX::"+str(icl)+"::"+s,1,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[0]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[0], x[0], icl);
       }
     };

     /// %Test for n-ary minimmum constraint
     class MinNary : public Test {
     public:
       /// Create and register test
       MinNary(Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Nary::"+str(icl),4,-4,4,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[3], icl);
       }
     };

     /// %Test for n-ary minimmum constraint with shared variables
     class MinNaryShared : public Test {
     public:
       /// Create and register test
       MinNaryShared(Gecode::IntConLevel icl)
         : Test("Arithmetic::Min::Nary::Shared::"+str(icl),3,-4,4,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[1], icl);
       }
     };

     /// %Test for n-ary maximum constraint
     class MaxNary : public Test {
     public:
       /// Create and register test
       MaxNary(Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Nary::"+str(icl),4,-4,4,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[3], icl);
       }
     };

     /// %Test for n-ary maximum constraint with shared variables
     class MaxNaryShared : public Test {
     public:
       /// Create and register test
       MaxNaryShared(Gecode::IntConLevel icl)
         : Test("Arithmetic::Max::Nary::Shared::"+str(icl),3,-4,4,false,icl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[1], icl);
       }
     };



     const int va[7] = {
       Gecode::Int::Limits::min, Gecode::Int::Limits::min+1,
       -1,0,1,
       Gecode::Int::Limits::max-1, Gecode::Int::Limits::max
     };
     const int vb[9] = {
       static_cast<int>(-sqrt(static_cast<double>
                              (-Gecode::Int::Limits::min))),
       -4,-2,-1,0,1,2,4,
       static_cast<int>(sqrt(static_cast<double>
                             (Gecode::Int::Limits::max)))
     };

     Gecode::IntSet a(va,7);
     Gecode::IntSet b(vb,9);
     Gecode::IntSet c(-8,8);

     MultXYZ mult_xyz_b_a("A",a,Gecode::ICL_BND);
     MultXYZ mult_xyz_b_b("B",b,Gecode::ICL_BND);
     MultXYZ mult_xyz_b_c("C",c,Gecode::ICL_BND);

     MultXXY mult_xxy_b_a("A",a,Gecode::ICL_BND);
     MultXXY mult_xxy_b_b("B",b,Gecode::ICL_BND);
     MultXXY mult_xxy_b_c("C",c,Gecode::ICL_BND);

     MultXYX mult_xyx_b_a("A",a,Gecode::ICL_BND);
     MultXYX mult_xyx_b_b("B",b,Gecode::ICL_BND);
     MultXYX mult_xyx_b_c("C",c,Gecode::ICL_BND);

     MultXYY mult_xyy_b_a("A",a,Gecode::ICL_BND);
     MultXYY mult_xyy_b_b("B",b,Gecode::ICL_BND);
     MultXYY mult_xyy_b_c("C",c,Gecode::ICL_BND);

     MultXXX mult_xxx_b_a("A",a,Gecode::ICL_BND);
     MultXXX mult_xxx_b_b("B",b,Gecode::ICL_BND);
     MultXXX mult_xxx_b_c("C",c,Gecode::ICL_BND);

     MultXYZ mult_xyz_d_a("A",a,Gecode::ICL_DOM);
     MultXYZ mult_xyz_d_b("B",b,Gecode::ICL_DOM);
     MultXYZ mult_xyz_d_c("C",c,Gecode::ICL_DOM);

     MultXXY mult_xxy_d_a("A",a,Gecode::ICL_DOM);
     MultXXY mult_xxy_d_b("B",b,Gecode::ICL_DOM);
     MultXXY mult_xxy_d_c("C",c,Gecode::ICL_DOM);

     MultXYX mult_xyx_d_a("A",a,Gecode::ICL_DOM);
     MultXYX mult_xyx_d_b("B",b,Gecode::ICL_DOM);
     MultXYX mult_xyx_d_c("C",c,Gecode::ICL_DOM);

     MultXYY mult_xyy_d_a("A",a,Gecode::ICL_DOM);
     MultXYY mult_xyy_d_b("B",b,Gecode::ICL_DOM);
     MultXYY mult_xyy_d_c("C",c,Gecode::ICL_DOM);

     MultXXX mult_xxx_d_a("A",a,Gecode::ICL_DOM);
     MultXXX mult_xxx_d_b("B",b,Gecode::ICL_DOM);
     MultXXX mult_xxx_d_c("C",c,Gecode::ICL_DOM);

     SqrXY sqr_xy_b_a("A",a,Gecode::ICL_BND);
     SqrXY sqr_xy_b_b("B",b,Gecode::ICL_BND);
     SqrXY sqr_xy_b_c("C",c,Gecode::ICL_BND);
     SqrXY sqr_xy_d_a("A",a,Gecode::ICL_DOM);
     SqrXY sqr_xy_d_b("B",b,Gecode::ICL_DOM);
     SqrXY sqr_xy_d_c("C",c,Gecode::ICL_DOM);

     SqrXX sqr_xx_b_a("A",a,Gecode::ICL_BND);
     SqrXX sqr_xx_b_b("B",b,Gecode::ICL_BND);
     SqrXX sqr_xx_b_c("C",c,Gecode::ICL_BND);
     SqrXX sqr_xx_d_a("A",a,Gecode::ICL_DOM);
     SqrXX sqr_xx_d_b("B",b,Gecode::ICL_DOM);
     SqrXX sqr_xx_d_c("C",c,Gecode::ICL_DOM);

     SqrtXY sqrt_xy_b_a("A",a,Gecode::ICL_BND);
     SqrtXY sqrt_xy_b_b("B",b,Gecode::ICL_BND);
     SqrtXY sqrt_xy_b_c("C",c,Gecode::ICL_BND);
     SqrtXY sqrt_xy_d_a("A",a,Gecode::ICL_DOM);
     SqrtXY sqrt_xy_d_b("B",b,Gecode::ICL_DOM);
     SqrtXY sqrt_xy_d_c("C",c,Gecode::ICL_DOM);

     SqrtXX sqrt_xx_b_a("A",a,Gecode::ICL_BND);
     SqrtXX sqrt_xx_b_b("B",b,Gecode::ICL_BND);
     SqrtXX sqrt_xx_b_c("C",c,Gecode::ICL_BND);
     SqrtXX sqrt_xx_d_a("A",a,Gecode::ICL_DOM);
     SqrtXX sqrt_xx_d_b("B",b,Gecode::ICL_DOM);
     SqrtXX sqrt_xx_d_c("C",c,Gecode::ICL_DOM);

     DivMod divmod_a_bnd("A",a);
     DivMod divmod_b_bnd("B",b);
     DivMod divmod_c_bnd("C",c);

     Div div_a_bnd("A",a);
     Div div_b_bnd("B",b);
     Div div_c_bnd("C",c);

     Mod mod_a_bnd("A",a);
     Mod mod_b_bnd("B",b);
     Mod mod_c_bnd("C",c);

     AbsXY abs_xy_b_a("A",a,Gecode::ICL_BND);
     AbsXY abs_xy_b_b("B",b,Gecode::ICL_BND);
     AbsXY abs_xy_b_c("C",c,Gecode::ICL_BND);
     AbsXY abs_xy_d_a("A",a,Gecode::ICL_DOM);
     AbsXY abs_xy_d_b("B",b,Gecode::ICL_DOM);
     AbsXY abs_xy_d_c("C",c,Gecode::ICL_DOM);

     AbsXX abs_xx_b_a("A",a,Gecode::ICL_BND);
     AbsXX abs_xx_b_b("B",b,Gecode::ICL_BND);
     AbsXX abs_xx_b_c("C",c,Gecode::ICL_BND);
     AbsXX abs_xx_d_a("A",a,Gecode::ICL_DOM);
     AbsXX abs_xx_d_b("B",b,Gecode::ICL_DOM);
     AbsXX abs_xx_d_c("C",c,Gecode::ICL_DOM);

     MinXYZ min_xyz_b_a("A",a,Gecode::ICL_BND);
     MinXYZ min_xyz_b_b("B",b,Gecode::ICL_BND);
     MinXYZ min_xyz_b_c("C",c,Gecode::ICL_BND);
     MinXYZ min_xyz_d_a("A",a,Gecode::ICL_DOM);
     MinXYZ min_xyz_d_b("B",b,Gecode::ICL_DOM);
     MinXYZ min_xyz_d_c("C",c,Gecode::ICL_DOM);

     MinXXY min_xxy_b_a("A",a,Gecode::ICL_BND);
     MinXXY min_xxy_b_b("B",b,Gecode::ICL_BND);
     MinXXY min_xxy_b_c("C",c,Gecode::ICL_BND);
     MinXXY min_xxy_d_a("A",a,Gecode::ICL_DOM);
     MinXXY min_xxy_d_b("B",b,Gecode::ICL_DOM);
     MinXXY min_xxy_d_c("C",c,Gecode::ICL_DOM);

     MinXYX min_xyx_b_a("A",a,Gecode::ICL_BND);
     MinXYX min_xyx_b_b("B",b,Gecode::ICL_BND);
     MinXYX min_xyx_b_c("C",c,Gecode::ICL_BND);
     MinXYX min_xyx_d_a("A",a,Gecode::ICL_DOM);
     MinXYX min_xyx_d_b("B",b,Gecode::ICL_DOM);
     MinXYX min_xyx_d_c("C",c,Gecode::ICL_DOM);

     MinXYY min_xyy_b_a("A",a,Gecode::ICL_BND);
     MinXYY min_xyy_b_b("B",b,Gecode::ICL_BND);
     MinXYY min_xyy_b_c("C",c,Gecode::ICL_BND);
     MinXYY min_xyy_d_a("A",a,Gecode::ICL_DOM);
     MinXYY min_xyy_d_b("B",b,Gecode::ICL_DOM);
     MinXYY min_xyy_d_c("C",c,Gecode::ICL_DOM);

     MinXXX min_xxx_b_a("A",a,Gecode::ICL_BND);
     MinXXX min_xxx_b_b("B",b,Gecode::ICL_BND);
     MinXXX min_xxx_b_c("C",c,Gecode::ICL_BND);
     MinXXX min_xxx_d_a("A",a,Gecode::ICL_DOM);
     MinXXX min_xxx_d_b("B",b,Gecode::ICL_DOM);
     MinXXX min_xxx_d_c("C",c,Gecode::ICL_DOM);

     MaxXYZ max_xyz_b_a("A",a,Gecode::ICL_BND);
     MaxXYZ max_xyz_b_b("B",b,Gecode::ICL_BND);
     MaxXYZ max_xyz_b_c("C",c,Gecode::ICL_BND);
     MaxXYZ max_xyz_d_a("A",a,Gecode::ICL_DOM);
     MaxXYZ max_xyz_d_b("B",b,Gecode::ICL_DOM);
     MaxXYZ max_xyz_d_c("C",c,Gecode::ICL_DOM);

     MaxXXY max_xxy_b_a("A",a,Gecode::ICL_BND);
     MaxXXY max_xxy_b_b("B",b,Gecode::ICL_BND);
     MaxXXY max_xxy_b_c("C",c,Gecode::ICL_BND);
     MaxXXY max_xxy_d_a("A",a,Gecode::ICL_DOM);
     MaxXXY max_xxy_d_b("B",b,Gecode::ICL_DOM);
     MaxXXY max_xxy_d_c("C",c,Gecode::ICL_DOM);

     MaxXYX max_xyx_b_a("A",a,Gecode::ICL_BND);
     MaxXYX max_xyx_b_b("B",b,Gecode::ICL_BND);
     MaxXYX max_xyx_b_c("C",c,Gecode::ICL_BND);
     MaxXYX max_xyx_d_a("A",a,Gecode::ICL_DOM);
     MaxXYX max_xyx_d_b("B",b,Gecode::ICL_DOM);
     MaxXYX max_xyx_d_c("C",c,Gecode::ICL_DOM);

     MaxXYY max_xyy_b_a("A",a,Gecode::ICL_BND);
     MaxXYY max_xyy_b_b("B",b,Gecode::ICL_BND);
     MaxXYY max_xyy_b_c("C",c,Gecode::ICL_BND);
     MaxXYY max_xyy_d_a("A",a,Gecode::ICL_DOM);
     MaxXYY max_xyy_d_b("B",b,Gecode::ICL_DOM);
     MaxXYY max_xyy_d_c("C",c,Gecode::ICL_DOM);

     MaxXXX max_xxx_b_a("A",a,Gecode::ICL_BND);
     MaxXXX max_xxx_b_b("B",b,Gecode::ICL_BND);
     MaxXXX max_xxx_b_c("C",c,Gecode::ICL_BND);
     MaxXXX max_xxx_d_a("A",a,Gecode::ICL_DOM);
     MaxXXX max_xxx_d_b("B",b,Gecode::ICL_DOM);
     MaxXXX max_xxx_d_c("C",c,Gecode::ICL_DOM);

     MinNary       min_nary_b(Gecode::ICL_BND);
     MinNary       min_nary_d(Gecode::ICL_DOM);
     MinNaryShared min_s_nary_b(Gecode::ICL_BND);
     MinNaryShared min_s_nary_d(Gecode::ICL_DOM);
     MaxNary       max_nary_b(Gecode::ICL_BND);
     MaxNary       max_nary_d(Gecode::ICL_DOM);
     MaxNaryShared max_s_nary_b(Gecode::ICL_BND);
     MaxNaryShared max_s_nary_d(Gecode::ICL_DOM);
     //@}

   }
}}

// STATISTICS: test-int
