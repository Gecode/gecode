/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include <gecode/minimodel.hh>

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
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Mult::XYZ::"+str(ipl)+"::"+s,3,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[2]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[2], ipl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXY : public Test {
     public:
       /// Create and register test
       MultXXY(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Mult::XXY::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         double d2 = static_cast<double>(x[1]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[1], ipl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYX : public Test {
     public:
       /// Create and register test
       MultXYX(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Mult::XYX::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[0]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[0], ipl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXYY : public Test {
     public:
       /// Create and register test
       MultXYY(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Mult::XYY::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         double d2 = static_cast<double>(x[1]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[1], x[1], ipl);
       }
     };

     /// %Test for multiplication constraint with shared variables
     class MultXXX : public Test {
     public:
       /// Create and register test
       MultXXX(const std::string& s, const Gecode::IntSet& d,
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Mult::XXX::"+str(ipl)+"::"+s,1,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         double d2 = static_cast<double>(x[0]);
         return d0*d1 == d2;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::mult(home, x[0], x[0], x[0], ipl);
       }
     };

     /// %Test for squaring constraint
     class SqrXY : public Test {
     public:
       /// Create and register test
       SqrXY(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Sqr::XY::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return d0*d0 == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqr(home, x[0], x[1], ipl);
       }
     };

     /// %Test for squaring constraint with shared variables
     class SqrXX : public Test {
     public:
       /// Create and register test
       SqrXX(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Sqr::XX::"+str(ipl)+"::"+s,1,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         return d0*d0 == d0;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqr(home, x[0], x[0], ipl);
       }
     };

     /// %Test for square root constraint
     class SqrtXY : public Test {
     public:
       /// Create and register test
       SqrtXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Sqrt::XY::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0 >= 0) && (d0 >= d1*d1) && (d0 < (d1+1)*(d1+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqrt(home, x[0], x[1], ipl);
       }
     };

     /// %Test for square root constraint with shared variables
     class SqrtXX : public Test {
     public:
       /// Create and register test
       SqrtXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Sqrt::XX::"+str(ipl)+"::"+s,1,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         return (d0 >= 0) && (d0 >= d0*d0) && (d0 < (d0+1)*(d0+1));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::sqrt(home, x[0], x[0], ipl);
       }
     };

     /// %Test for power constraint
     class PowXY : public Test {
     protected:
       /// The exponent
       int n;
     public:
       /// Create and register test
       PowXY(const std::string& s, int n0, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Pow::XY::"+str(n0)+"::"+str(ipl)+"::"+s,
                2,d,false,ipl), n(n0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         long long int p = 1;
         for (int i=0; i<n; i++) {
           p *= x[0];
           if ((p < Gecode::Int::Limits::min) ||
               (p > Gecode::Int::Limits::max))
             return false;
         }
         return p == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (n > 4)
           pow(home, x[0], n, x[1], ipl);
         else
           rel(home, expr(home, pow(x[0],n), ipl), IRT_EQ, x[1], ipl);
       }
     };

     /// %Test for power constraint with shared variables
     class PowXX : public Test {
     protected:
       /// The exponent
       int n;
     public:
       /// Create and register test
       PowXX(const std::string& s, int n0, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Pow::XX::"+str(n0)+"::"+str(ipl)+"::"+s,
                1,d,false,ipl), n(n0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         long long int p = 1;
         for (int i=0; i<n; i++) {
           p *= x[0];
           if ((p < Gecode::Int::Limits::min) ||
               (p > Gecode::Int::Limits::max))
             return false;
         }
         return p == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::pow(home, x[0], n, x[0], ipl);
       }
     };

     bool powgr(int n, long long int r, int x) {
       assert(r >= 0);
       long long int y = r;
       long long int p = 1;
       do {
         p *= y; n--;
         if (p > x)
           return true;
       } while (n > 0);
       return false;
     }

     int fnroot(int n, int x) {
       if (x < 2)
         return x;
       /*
        * We look for l such that: l^n <= x < (l+1)^n
        */
       long long int l = 1;
       long long int u = x;
       do {
         long long int m = (l + u) >> 1;
         if (powgr(n,m,x)) u=m; else l=m;
       } while (l+1 < u);
       return static_cast<int>(l);
     }

     bool powle(int n, long long int r, int x) {
       assert(r >= 0);
       long long int y = r;
       long long int p = 1;
       do {
         p *= y; n--;
         if (p >= x)
           return false;
       } while (n > 0);
       assert(y < x);
       return true;
     }

     int cnroot(int n, int x) {
       if (x < 2)
         return x;
       /*
        * We look for u such that: (u-1)^n < x <= u^n
        */
       long long int l = 1;
       long long int u = x;
       do {
         long long int m = (l + u) >> 1;
         if (powle(n,m,x)) l=m; else u=m;
       } while (l+1 < u);
       return static_cast<int>(u);
     }

     /// %Test for nroot constraint
     class NrootXY : public Test {
     protected:
       /// The root index
       int n;
       /// Floor
     public:
       /// Create and register test
       NrootXY(const std::string& s, int n0, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Nroot::XY::"+str(n0)+"::"+str(ipl)+"::"+s,
                2,d,false,ipl), n(n0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (n == 1)
           return x[0] == x[1];
         if ((n % 2 == 0) && ((x[0] < 0) || (x[1] < 0)))
           return false;
         int r = (x[0] < 0) ? -cnroot(n,-x[0]) : fnroot(n,x[0]);
         return r == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         using namespace Gecode;
         if (n > 4)
           nroot(home, x[0], n, x[1], ipl);
         else
           rel(home, expr(home, nroot(x[0],n), ipl), IRT_EQ, x[1], ipl);
       }
     };

     /// %Test for nroot constraint with shared variables
     class NrootXX : public Test {
     protected:
       /// The root index
       int n;
     public:
       /// Create and register test
       NrootXX(const std::string& s, int n0, const Gecode::IntSet& d,
               Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Nroot::XX::"+str(n0)+"::"+str(ipl)+"::"+s,
                1,d,false,ipl), n(n0) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         if (n == 1)
           return true;
         if (n % 2 == 0) {
           return (x[0] >= 0) && (x[0] <= 1);
         } else {
           return (x[0] >= -2) && (x[0] <= 1);
         }
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::nroot(home, x[0], n, x[0], ipl);
       }
     };

     /// %Test for division/modulo constraint
     class DivMod : public Test {
     private:
       /// Return the absolute value of \a a
       static int abs(int a) { return a<0 ? -a:a; }
       /// Return the sign of \a a
       static int sgn(int a) { return a<0 ? -1:1; }
     public:
       /// Create and register test
       DivMod(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::DivMod::"+s,4,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return x[0] == x[1]*x[2]+x[3] &&
                abs(x[3]) < abs(x[1]) &&
                (x[3] == 0 || sgn(x[3]) == sgn(x[0]));
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::divmod(home, x[0], x[1], x[2], x[3]);
       }
     };

     /// %Test for division constraint
     class Div : public Test {
     public:
       /// Create and register test
       Div(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Div::"+s,3,d) {}
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
         Gecode::div(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for modulo constraint
     class Mod : public Test {
     public:
       /// Create and register test
       Mod(const std::string& s, const Gecode::IntSet& d)
         : Test("Arithmetic::Mod::"+s,3,d) {}
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
         Gecode::mod(home, x[0], x[1], x[2]);
       }
     };

     /// %Test for absolute value constraint
     class AbsXY : public Test {
     public:
       /// Create and register test
       AbsXY(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Abs::XY::"+str(ipl)+"::"+s,2,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[1]);
         return (d0<0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::abs(home, x[0], x[1], ipl);
       }
     };

     /// %Test for absolute value constraint with shared variables
     class AbsXX : public Test {
     public:
       /// Create and register test
       AbsXX(const std::string& s, const Gecode::IntSet& d,
             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Abs::XX::"+str(ipl)+"::"+s,1,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         double d0 = static_cast<double>(x[0]);
         double d1 = static_cast<double>(x[0]);
         return (d0<0 ? -d0 : d0) == d1;
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::abs(home, x[0], x[0], ipl);
       }
     };

     /// %Test for binary minimum constraint
     class MinXYZ : public Test {
     public:
       /// Create and register test
       MinXYZ(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Bin::XYZ::"+str(ipl)+"::"+s,3,d,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[2], ipl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXY : public Test {
     public:
       /// Create and register test
       MinXXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Bin::XYX::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[0], x[1], ipl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYX : public Test {
     public:
       /// Create and register test
       MinXYX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Bin::XYX::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[0], ipl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXYY : public Test {
     public:
       /// Create and register test
       MinXYY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Bin::XYY::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[1]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[1], x[1], ipl);
       }
     };

     /// %Test for binary minimum constraint with shared variables
     class MinXXX : public Test {
     public:
       /// Create and register test
       MinXXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Bin::XXX::"+str(ipl)+"::"+s,1,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(x[0],x[0]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::min(home, x[0], x[0], x[0], ipl);
       }
     };

     /// %Test for binary maximum constraint
     class MaxXYZ : public Test {
     public:
       /// Create and register test
       MaxXYZ(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Bin::XYZ::"+str(ipl)+"::"+s,3,d) {
         contest = CTL_BOUNDS_Z;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[2], ipl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXY : public Test {
     public:
       /// Create and register test
       MaxXXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Bin::XXY::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[0], x[1], ipl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYX : public Test {
     public:
       /// Create and register test
       MaxXYX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Bin::XYX::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[0], ipl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXYY : public Test {
     public:
       /// Create and register test
       MaxXYY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Bin::XYY::"+str(ipl)+"::"+s,2,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[1]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[1], x[1], ipl);
       }
     };

     /// %Test for binary maximum constraint with shared variables
     class MaxXXX : public Test {
     public:
       /// Create and register test
       MaxXXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Bin::XXX::"+str(ipl)+"::"+s,1,d) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(x[0],x[0]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::max(home, x[0], x[0], x[0], ipl);
       }
     };

     /// %Test for n-ary minimmum constraint
     class MinNary : public Test {
     public:
       /// Create and register test
       MinNary(Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Nary::"+str(ipl),4,-4,4,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[3], ipl);
       }
     };

     /// %Test for n-ary minimmum constraint with shared variables
     class MinNaryShared : public Test {
     public:
       /// Create and register test
       MinNaryShared(Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Min::Nary::Shared::"+str(ipl),3,-4,4,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::min(std::min(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::min(home, m, x[1], ipl);
       }
     };

     /// %Test for n-ary maximum constraint
     class MaxNary : public Test {
     public:
       /// Create and register test
       MaxNary(Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Nary::"+str(ipl),4,-4,4,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[3];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[3], ipl);
       }
     };

     /// %Test for n-ary maximum constraint with shared variables
     class MaxNaryShared : public Test {
     public:
       /// Create and register test
       MaxNaryShared(Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Max::Nary::Shared::"+str(ipl),3,-4,4,false,ipl) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return std::max(std::max(x[0],x[1]), x[2]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs m(3);
         m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
         Gecode::max(home, m, x[1], ipl);
       }
     };

     /// %Test for argument maximum constraint
     class ArgMax : public Test {
     protected:
       /// Offset to be used
       int offset;
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMax(int n, int o, bool tb)
         : Test("Arithmetic::ArgMax::"+str(o)+"::"+str(tb)+"::"+str(n),
                n+1,0,n+1,
                false,tb ? Gecode::IPL_DEF : Gecode::IPL_DOM),
           offset(o), tiebreak(tb) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < offset) || (x[n] >= n + offset))
           return false;
         int m=x[0]; int p=0;
         for (int i=1; i<n; i++)
           if (x[i] > m) {
             p=i; m=x[i];
           }
         return tiebreak ? (p + offset == x[n]) : (m == x[x[n]-offset]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::IntVarArgs m(n);
         for (int i=0; i<n; i++)
           m[i]=x[i];
         Gecode::argmax(home, m, offset, x[n], tiebreak);
       }
     };

     /// %Test for argument maximum constraint with shared variables
     class ArgMaxShared : public Test {
     protected:
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMaxShared(int n, bool tb)
         : Test("Arithmetic::ArgMax::Shared::"+str(tb)+"::"+str(n),n+1,0,n+1,
                false),
           tiebreak(tb)  {
         testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < 0) || (x[n] >= 2*n))
           return false;
         Gecode::IntArgs y(2*n);
         for (int i=0; i<n; i++)
           y[2*i+0]=y[2*i+1]=x[i];
         int m=y[0]; int p=0;
         for (int i=1; i<2*n; i++)
           if (y[i] > m) {
             p=i; m=y[i];
           }
         return tiebreak ? (p == x[n]) : (m == y[x[n]]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::IntVarArgs m(2*n);
         for (int i=0; i<n; i++)
           m[2*i+0]=m[2*i+1]=x[i];
         Gecode::argmax(home, m, x[n], tiebreak);
       }
     };

     /// %Test for argument minimum constraint
     class ArgMin : public Test {
     protected:
       /// Which offset to use
       int offset;
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMin(int n, int o, bool tb)
         : Test("Arithmetic::ArgMin::"+str(o)+"::"+str(tb)+"::"+str(n),
                n+1,0,n+1,
                false,tb ? Gecode::IPL_DEF : Gecode::IPL_DOM),
           offset(o), tiebreak(tb)  {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < offset) || (x[n] >= n + offset))
           return false;
         int m=x[0]; int p=0;
         for (int i=1; i<n; i++)
           if (x[i] < m) {
             p=i; m=x[i];
           }
         return tiebreak ? (p+offset == x[n]) : (m == x[x[n]-offset]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::IntVarArgs m(n);
         for (int i=0; i<n; i++)
           m[i]=x[i];
         Gecode::argmin(home, m, offset, x[n], tiebreak);
       }
     };

     /// %Test for argument minimum constraint with shared variables
     class ArgMinShared : public Test {
     protected:
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMinShared(int n, bool tb)
         : Test("Arithmetic::ArgMin::Shared::"+str(tb)+"::"+str(n),n+1,0,n+1,
                false),
           tiebreak(tb) {
         testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < 0) || (x[n] >= 2*n))
           return false;
         Gecode::IntArgs y(2*n);
         for (int i=0; i<n; i++)
           y[2*i+0]=y[2*i+1]=x[i];
         int m=y[0]; int p=0;
         for (int i=1; i<2*n; i++)
           if (y[i] < m) {
             p=i; m=y[i];
           }
         return tiebreak ? (p == x[n]) : (m == y[x[n]]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::IntVarArgs m(2*n);
         for (int i=0; i<n; i++)
           m[2*i+0]=m[2*i+1]=x[i];
         Gecode::argmin(home, m, x[n], tiebreak);
       }
     };

     /// %Test for Boolean argument maximum constraint
     class ArgMaxBool : public Test {
     protected:
       /// Offset to be used
       int offset;
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMaxBool(int n, int o, bool tb)
         : Test("Arithmetic::ArgMaxBool::"+str(o)+"::"+str(tb)+"::"+str(n),
                n+1,0,n+1,
                false,tb ? Gecode::IPL_DEF : Gecode::IPL_DOM),
           offset(o), tiebreak(tb) {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < offset) || (x[n] >= n + offset))
           return false;
         int m=x[0]; int p=0;
         if (x[0] > 1)
           return false;
         for (int i=1; i<n; i++) {
           if (x[i] > 1)
             return false;
           if (x[i] > m) {
             p=i; m=x[i];
           }
         }
         return tiebreak ? (p + offset == x[n]) : (m == x[x[n]-offset]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::BoolVarArgs m(n);
         for (int i=0; i<n; i++)
           m[i]=channel(home,x[i]);
         Gecode::argmax(home, m, offset, x[n], tiebreak);
       }
     };

     /// %Test for argument maximum constraint with shared variables
     class ArgMaxBoolShared : public Test {
     protected:
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMaxBoolShared(int n, bool tb)
         : Test("Arithmetic::ArgMaxBool::Shared::"+str(tb)+"::"+str(n),n+1,0,n+1,
                false),
           tiebreak(tb)  {
         testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < 0) || (x[n] >= 2*n))
           return false;
         Gecode::IntArgs y(2*n);
         for (int i=0; i<n; i++)
           y[2*i+0]=y[2*i+1]=x[i];
         int m=y[0]; int p=0;
         if (y[0] > 1)
           return false;
         for (int i=1; i<2*n; i++) {
           if (y[i] > 1)
             return false;
           if (y[i] > m) {
             p=i; m=y[i];
           }
         }
         return tiebreak ? (p == x[n]) : (m == y[x[n]]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::BoolVarArgs m(2*n);
         for (int i=0; i<n; i++)
           m[2*i+0]=m[2*i+1]=channel(home,x[i]);
         Gecode::argmax(home, m, x[n], tiebreak);
       }
     };

     /// %Test for argument minimum constraint
     class ArgMinBool : public Test {
     protected:
       /// Which offset to use
       int offset;
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMinBool(int n, int o, bool tb)
         : Test("Arithmetic::ArgMinBool::"+str(o)+"::"+str(tb)+"::"+str(n),
                n+1,0,n+1,
                false,tb ? Gecode::IPL_DEF : Gecode::IPL_DOM),
           offset(o), tiebreak(tb)  {}
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < offset) || (x[n] >= n + offset))
           return false;
         int m=x[0]; int p=0;
         if (x[0] < 0 || x[0] > 1)
           return false;
         for (int i=1; i<n; i++) {
           if (x[i] < 0 || x[i] > 1)
             return false;
           if (x[i] < m) {
             p=i; m=x[i];
           }
         }
         return tiebreak ? (p+offset == x[n]) : (m == x[x[n]-offset]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::BoolVarArgs m(n);
         for (int i=0; i<n; i++)
           m[i]=channel(home,x[i]);
         Gecode::argmin(home, m, offset, x[n], tiebreak);
       }
     };

     /// %Test for argument minimum constraint with shared variables
     class ArgMinBoolShared : public Test {
     protected:
       /// Whether to use tie-breaking
       bool tiebreak;
     public:
       /// Create and register test
       ArgMinBoolShared(int n, bool tb)
         : Test("Arithmetic::ArgMinBool::Shared::"+str(tb)+"::"+str(n),n+1,0,n+1,
                false),
           tiebreak(tb) {
         testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         int n=x.size()-1;
         if ((x[n] < 0) || (x[n] >= 2*n))
           return false;
         Gecode::IntArgs y(2*n);
         for (int i=0; i<n; i++)
           y[2*i+0]=y[2*i+1]=x[i];
         int m=y[0]; int p=0;
         if (y[0] > 1)
           return false;
         for (int i=1; i<2*n; i++) {
           if (y[i] > 1)
             return false;
           if (y[i] < m) {
             p=i; m=y[i];
           }
         }
         return tiebreak ? (p == x[n]) : (m == y[x[n]]);
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         int n=x.size()-1;
         Gecode::BoolVarArgs m(2*n);
         for (int i=0; i<n; i++)
           m[2*i+0]=m[2*i+1]=channel(home,x[i]);
         Gecode::argmin(home, m, x[n], tiebreak);
       }
     };

     /// Help class to create and register tests
     class Create {
     public:
       /// Perform creation and registration
       Create(void) {

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
         Gecode::IntSet d(-70,70);

         (void) new DivMod("A",a);
         (void) new DivMod("B",b);
         (void) new DivMod("C",c);

         (void) new Div("A",a);
         (void) new Div("B",b);
         (void) new Div("C",c);

         (void) new Mod("A",a);
         (void) new Mod("B",b);
         (void) new Mod("C",c);


         for (IntPropLevels ipls; ipls(); ++ipls) {
           (void) new AbsXY("A",a,ipls.ipl());
           (void) new AbsXY("B",b,ipls.ipl());
           (void) new AbsXY("C",c,ipls.ipl());

           (void) new AbsXX("A",a,ipls.ipl());
           (void) new AbsXX("B",b,ipls.ipl());
           (void) new AbsXX("C",c,ipls.ipl());
           if (ipls.ipl() != Gecode::IPL_VAL) {
             (void) new MultXYZ("A",a,ipls.ipl());
             (void) new MultXYZ("B",b,ipls.ipl());
             (void) new MultXYZ("C",c,ipls.ipl());

             (void) new MultXXY("A",a,ipls.ipl());
             (void) new MultXXY("B",b,ipls.ipl());
             (void) new MultXXY("C",c,ipls.ipl());

             (void) new MultXYX("A",a,ipls.ipl());
             (void) new MultXYX("B",b,ipls.ipl());
             (void) new MultXYX("C",c,ipls.ipl());

             (void) new MultXYY("A",a,ipls.ipl());
             (void) new MultXYY("B",b,ipls.ipl());
             (void) new MultXYY("C",c,ipls.ipl());

             (void) new MultXXX("A",a,ipls.ipl());
             (void) new MultXXX("B",b,ipls.ipl());
             (void) new MultXXX("C",c,ipls.ipl());

             (void) new SqrXY("A",a,ipls.ipl());
             (void) new SqrXY("B",b,ipls.ipl());
             (void) new SqrXY("C",c,ipls.ipl());

             (void) new SqrXX("A",a,ipls.ipl());
             (void) new SqrXX("B",b,ipls.ipl());
             (void) new SqrXX("C",c,ipls.ipl());

             for (int n=0; n<=6; n++) {
               (void) new PowXY("A",n,a,ipls.ipl());
               (void) new PowXY("B",n,b,ipls.ipl());
               (void) new PowXY("C",n,c,ipls.ipl());
               (void) new PowXY("D",n,d,ipls.ipl());

               (void) new PowXX("A",n,a,ipls.ipl());
               (void) new PowXX("B",n,b,ipls.ipl());
               (void) new PowXX("C",n,c,ipls.ipl());
               (void) new PowXX("D",n,d,ipls.ipl());
             }

             for (int n=1; n<=6; n++) {
               (void) new NrootXY("A",n,a,ipls.ipl());
               (void) new NrootXY("B",n,b,ipls.ipl());
               (void) new NrootXY("C",n,c,ipls.ipl());
               (void) new NrootXY("D",n,d,ipls.ipl());

               (void) new NrootXX("A",n,a,ipls.ipl());
               (void) new NrootXX("B",n,b,ipls.ipl());
               (void) new NrootXX("C",n,c,ipls.ipl());
               (void) new NrootXX("D",n,d,ipls.ipl());
             }

             for (int n=30; n<=34; n++) {
               (void) new PowXY("C",n,c,ipls.ipl());
               (void) new PowXX("C",n,c,ipls.ipl());
               (void) new NrootXY("C",n,c,ipls.ipl());
               (void) new NrootXX("C",n,c,ipls.ipl());
             }

             (void) new SqrtXY("A",a,ipls.ipl());
             (void) new SqrtXY("B",b,ipls.ipl());
             (void) new SqrtXY("C",c,ipls.ipl());

             (void) new SqrtXX("A",a,ipls.ipl());
             (void) new SqrtXX("B",b,ipls.ipl());
             (void) new SqrtXX("C",c,ipls.ipl());

             (void) new MinXYZ("A",a,ipls.ipl());
             (void) new MinXYZ("B",b,ipls.ipl());
             (void) new MinXYZ("C",c,ipls.ipl());

             (void) new MinXXY("A",a,ipls.ipl());
             (void) new MinXXY("B",b,ipls.ipl());
             (void) new MinXXY("C",c,ipls.ipl());

             (void) new MinXYX("A",a,ipls.ipl());
             (void) new MinXYX("B",b,ipls.ipl());
             (void) new MinXYX("C",c,ipls.ipl());

             (void) new MinXYY("A",a,ipls.ipl());
             (void) new MinXYY("B",b,ipls.ipl());
             (void) new MinXYY("C",c,ipls.ipl());

             (void) new MinXXX("A",a,ipls.ipl());
             (void) new MinXXX("B",b,ipls.ipl());
             (void) new MinXXX("C",c,ipls.ipl());

             (void) new MaxXYZ("A",a,ipls.ipl());
             (void) new MaxXYZ("B",b,ipls.ipl());
             (void) new MaxXYZ("C",c,ipls.ipl());

             (void) new MaxXXY("A",a,ipls.ipl());
             (void) new MaxXXY("B",b,ipls.ipl());
             (void) new MaxXXY("C",c,ipls.ipl());

             (void) new MaxXYX("A",a,ipls.ipl());
             (void) new MaxXYX("B",b,ipls.ipl());
             (void) new MaxXYX("C",c,ipls.ipl());

             (void) new MaxXYY("A",a,ipls.ipl());
             (void) new MaxXYY("B",b,ipls.ipl());
             (void) new MaxXYY("C",c,ipls.ipl());

             (void) new MaxXXX("A",a,ipls.ipl());
             (void) new MaxXXX("B",b,ipls.ipl());
             (void) new MaxXXX("C",c,ipls.ipl());

             (void) new MinNary(ipls.ipl());
             (void) new MinNaryShared(ipls.ipl());
             (void) new MaxNary(ipls.ipl());
             (void) new MaxNaryShared(ipls.ipl());
           }
         }

         for (int i=1; i<5; i++) {
           (void) new ArgMax(i,0,true);
           (void) new ArgMax(i,1,true);
           (void) new ArgMaxShared(i,true);
           (void) new ArgMin(i,0,true);
           (void) new ArgMin(i,1,true);
           (void) new ArgMinShared(i,true);
           (void) new ArgMax(i,0,false);
           (void) new ArgMax(i,1,false);
           (void) new ArgMaxShared(i,false);
           (void) new ArgMin(i,0,false);
           (void) new ArgMin(i,1,false);
           (void) new ArgMinShared(i,false);

           (void) new ArgMaxBool(i,0,true);
           (void) new ArgMaxBool(i,1,true);
           (void) new ArgMaxBoolShared(i,true);
           (void) new ArgMinBool(i,0,true);
           (void) new ArgMinBool(i,1,true);
           (void) new ArgMinBoolShared(i,true);
           (void) new ArgMaxBool(i,0,false);
           (void) new ArgMaxBool(i,1,false);
           (void) new ArgMaxBoolShared(i,false);
           (void) new ArgMinBool(i,0,false);
           (void) new ArgMinBool(i,1,false);
           (void) new ArgMinBoolShared(i,false);
         }
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
