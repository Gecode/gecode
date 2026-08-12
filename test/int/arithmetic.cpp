/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
     /// Compute the mathematical greatest common divisor for testing.
     int gcd_value(int a, int b) {
       a = (a < 0) ? -a : a;
       b = (b < 0) ? -b : b;
       while (b != 0) {
         int t = a % b;
         a = b; b = t;
       }
       return a;
     }

     /// %Test for the ternary greatest-common-divisor constraint
     class GcdXYZ : public Test {
     public:
       /// Create and register test
       GcdXYZ(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Gcd::XYZ::"+str(ipl)+"::"+s,3,d,true,ipl) {
         contest=CTL_NONE; testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return gcd_value(x[0],x[1]) == x[2];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::gcd(home,x[0],x[1],x[2],ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::gcd(home,x[0],x[1],x[2],r,ipl);
       }
     };

     /// %Test for gcd with identical operands
     class GcdXXY : public Test {
     public:
       /// Create and register test
       GcdXXY(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Gcd::XXY::"+str(ipl)+"::"+s,2,d,true,ipl) {
         contest=CTL_NONE; testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return gcd_value(x[0],x[0]) == x[1];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::gcd(home,x[0],x[0],x[1],ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::gcd(home,x[0],x[0],x[1],r,ipl);
       }
     };

     /// %Test for gcd with result aliased to the first operand
     class GcdXYX : public Test {
     public:
       /// Create and register test
       GcdXYX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Gcd::XYX::"+str(ipl)+"::"+s,2,d,true,ipl) {
         contest=CTL_NONE; testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return gcd_value(x[0],x[1]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::gcd(home,x[0],x[1],x[0],ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::gcd(home,x[0],x[1],x[0],r,ipl);
       }
     };

     /// %Test for gcd with all variables aliased
     class GcdXXX : public Test {
     public:
       /// Create and register test
       GcdXXX(const std::string& s, const Gecode::IntSet& d,
              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Gcd::XXX::"+str(ipl)+"::"+s,1,d,true,ipl) {
         contest=CTL_NONE; testfix=false;
       }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return gcd_value(x[0],x[0]) == x[0];
       }
       /// Post constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::gcd(home,x[0],x[0],x[0],ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::gcd(home,x[0],x[0],x[0],r,ipl);
       }
     };

     /// %Test for the reified divisibility constraint
     class DividesXY : public Test {
     public:
       /// Create and register test
       DividesXY(const std::string& s, const Gecode::IntSet& d,
                 Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Divides::XY::"+str(ipl)+"::"+s,
                2,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       /// %Test whether \a x is solution
       virtual bool solution(const Assignment& x) const {
         return (x[0] == 0) ? (x[1] == 0) : (x[1] % x[0] == 0);
       }
       /// The constraint is deliberately exposed only as reified
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVar b(home,1,1);
         Gecode::divides(home,x[0],x[1],Gecode::Reify(b),ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::divides(home,x[0],x[1],r,ipl);
       }
     };

     /// %Test divisibility with aliased operands
     class DividesXX : public Test {
     public:
       /// Create and register test
       DividesXX(const std::string& s, const Gecode::IntSet& d,
                 Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Divides::XX::"+str(ipl)+"::"+s,
                1,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       /// Every integer divides itself, including zero
       virtual bool solution(const Assignment&) const {
         return true;
       }
       /// The constraint is deliberately exposed only as reified
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::BoolVar b(home,1,1);
         Gecode::divides(home,x[0],x[0],Gecode::Reify(b),ipl);
       }
       /// Post reified constraint on \a x
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::divides(home,x[0],x[0],r,ipl);
       }
     };

     /// Evaluate an exact product for testing without overflowing.
     bool product_value(const Assignment& x, int n, int& product) {
       for (int i=0; i<n; i++)
         if (x[i] == 0) {
           product = 0;
           return true;
         }
       long long int p = 1;
       for (int i=0; i<n; i++) {
         if (Gecode::Int::Limits::overflow_mul
             (p,static_cast<long long int>(x[i])))
           return false;
         p *= static_cast<long long int>(x[i]);
       }
       if (!Gecode::Int::Limits::valid(p))
         return false;
       product = static_cast<int>(p);
       return true;
     }

     /// %Test for an ordinary and reified three-factor product
     class ProductXYZR : public Test {
     public:
       ProductXYZR(const std::string& s, const Gecode::IntSet& d,
                   Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Product::XYZR::"+str(ipl)+"::"+s,
                4,d,true,ipl) { contest = CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         int p;
         return product_value(x,3,p) && (p == x[3]);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::IntVarArgs f({x[0],x[1],x[2]});
         Gecode::product(home,f,x[3],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::IntVarArgs f({x[0],x[1],x[2]});
         Gecode::product(home,f,x[3],r,ipl);
       }
     };

     /// %Test for the empty product
     class ProductEmpty : public Test {
     public:
       ProductEmpty(const std::string& s, const Gecode::IntSet& d,
                    Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Product::Empty::"+str(ipl)+"::"+s,
                1,d,true,ipl) { contest = CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return x[0] == 1;
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product(home,Gecode::IntVarArgs(),x[0],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product(home,Gecode::IntVarArgs(),x[0],r,ipl);
       }
     };

     /// %Test for the singleton product
     class ProductSingleton : public Test {
     public:
       ProductSingleton(const std::string& s, const Gecode::IntSet& d,
                        Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Product::Singleton::"+str(ipl)+"::"+s,
                2,d,true,ipl) { contest = CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return x[0] == x[1];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product(home,Gecode::IntVarArgs({x[0]}),x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product(home,Gecode::IntVarArgs({x[0]}),x[1],r,ipl);
       }
     };

     /// %Test repeated factors and result aliasing
     class ProductXXYAlias : public Test {
     public:
       ProductXXYAlias(const std::string& s, const Gecode::IntSet& d,
                       Gecode::IntPropLevel ipl)
         : Test("Arithmetic::Product::XXYAlias::"+str(ipl)+"::"+s,
                2,d,true,ipl) { contest = CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         long long int p = static_cast<long long int>(x[0]) * x[0] * x[1];
         return (p >= Gecode::Int::Limits::min) &&
           (p <= Gecode::Int::Limits::max) && (p == x[1]);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product(home,Gecode::IntVarArgs({x[0],x[0],x[1]}),x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product(home,Gecode::IntVarArgs({x[0],x[0],x[1]}),x[1],r,ipl);
       }
     };

     /// %Test product bounds propagation beyond the support-enumeration cap
     class ProductBoundsLarge : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
     public:
       ProductBoundsLarge(void)
         : ::Test::Base("Int::Arithmetic::Product::BoundsLarge") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           IntVarArgs x(home,3,2,100);
           IntVar y(home,0,Gecode::Int::Limits::max);
           BoolVar b(home,1,1);
           product(home,x,y,Reify(b));
           if ((home.status() == SS_FAILED) ||
               (y.min() != 8) || (y.max() != 1000000))
             return false;
         }
         {
           TestSpace home;
           IntVarArgs x(home,3,10,100);
           IntVar y(home,1000,1000);
           product(home,x,y);
           if (home.status() == SS_FAILED)
             return false;
           for (int i=0; i<x.size(); i++)
             if (!x[i].assigned() || (x[i].val() != 10))
               return false;
         }
         return true;
       }
     };

     /// Test zero, unit, and aggregate-sign product simplifications
     class ProductSimplifySign : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
       class CloneSpace : public Gecode::Space {
       public:
         Gecode::IntVar x, q, y;
         CloneSpace(void)
           : x(*this,-2,2), q(*this,2,2), y(*this,-10,10) {
           Gecode::product(*this,Gecode::IntVarArgs({x,q}),y);
         }
         CloneSpace(CloneSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x); q.update(*this,s.q); y.update(*this,s.y);
         }
         virtual Gecode::Space* copy(void) { return new CloneSpace(*this); }
       };
     public:
       ProductSimplifySign(void)
         : ::Test::Base("Int::Arithmetic::Product::SimplifySign") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           IntVar z(home,0,0), x(home,-100,100), y(home,-100,100);
           product(home,IntVarArgs({x,z}),y);
           if ((home.status() == SS_FAILED) || !y.assigned() || (y.val()!=0))
             return false;
         }
         {
           TestSpace home;
           IntVar one(home,1,1), minus(home,-1,-1), x(home,2,5);
           IntVar y(home,-100,100);
           product(home,IntVarArgs({one,minus,minus,x}),y);
           if ((home.status() == SS_FAILED) || (y.min()!=2) || (y.max()!=5))
             return false;
         }
         {
           TestSpace home;
           IntVar minus(home,-1,-1), x(home,2,5), y(home,-100,100);
           product(home,IntVarArgs({minus,x}),y);
           if ((home.status() == SS_FAILED) || (y.min()!=-5) || (y.max()!=-2))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,0,5), q(home,1,3), y(home,2,10);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) || (x.min()!=1))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-2,2), q(home,1,3), y(home,0,0);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() || (x.val()!=0))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-5,0), q(home,2,4), y(home,-100,100);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) || (y.max()!=0))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-5,-2), q(home,-4,-2), y(home,-100,100);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) || (y.min()!=4))
             return false;
         }
         {
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar minus(home,-1,-1), x(home,hi,hi), y(home,-hi,hi);
           product(home,IntVarArgs({minus,x}),y);
           if ((home.status() == SS_FAILED) || !y.assigned() || (y.val()!=-hi))
             return false;
         }
         {
           CloneSpace home;
           if (home.status() == SS_FAILED)
             return false;
           CloneSpace* clone=static_cast<CloneSpace*>(home.clone());
           PropagatorGroup::all.disable(*clone);
           rel(home,home.x,IRT_NQ,0);
           rel(*clone,clone->x,IRT_NQ,0);
           (void) home.status();
           (void) clone->status();
           rel(home,home.y,IRT_GQ,2);
           rel(*clone,clone->y,IRT_GQ,2);
           PropagatorGroup::all.enable(*clone);
           const bool ok=(home.status()!=SS_FAILED) &&
             (clone->status()!=SS_FAILED) && (home.x.min()==1) &&
             (clone->x.min()==1) && (home.y.min()==2) &&
             (clone->y.min()==2);
           delete clone;
           if (!ok)
             return false;
         }
         return true;
       }
     };

     /// Test repeated powers and result-alias cancellation
     class ProductPowerAlias : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
     public:
       ProductPowerAlias(void)
         : ::Test::Base("Int::Arithmetic::Product::PowerAlias") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           IntVar x(home,-10,10), y(home,-10,100);
           product(home,IntVarArgs({x,x}),y);
           if ((home.status() == SS_FAILED) || (y.min() != 0))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-10,10), y(home,20,30);
           product(home,IntVarArgs({x,x}),y);
           if ((home.status() == SS_FAILED) ||
               (x.min() != -5) || (x.max() != 5))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-10,10), y(home,20,30);
           product(home,IntVarArgs({x,x,x}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() ||
               (x.val() != 3))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-10,10), y(home,-30,-20);
           product(home,IntVarArgs({x,x,x}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() ||
               (x.val() != -3))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-10,10), two(home,2,2), y(home,50,72);
           product(home,IntVarArgs({x,x,two}),y);
           if ((home.status() == SS_FAILED) ||
               (x.min() != -6) || (x.max() != 6))
             return false;
         }
         {
           // x*y=y has only the zero branch when x cannot be one.
           TestSpace home;
           IntVar x(home,2,4), y(home,-10,10);
           product(home,IntVarArgs({x,y}),y);
           if ((home.status() == SS_FAILED) || !y.assigned() ||
               (y.val() != 0))
             return false;
         }
         {
           // A nonzero result permits cancellation of one result occurrence.
           TestSpace home;
           IntVar x(home,0,2), y(home,2,10);
           product(home,IntVarArgs({x,y}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() ||
               (x.val() != 1))
             return false;
         }
         {
           // Cancelling one of two result occurrences leaves y*x=1.
           TestSpace home;
           IntVar x(home,-1,0), y(home,-2,-1);
           product(home,IntVarArgs({y,y,x}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() ||
               !y.assigned() || (x.val() != -1) || (y.val() != -1))
             return false;
         }
         {
           // Direct n-ary evaluation retains zero after an overflowing prefix.
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar a(home,hi,hi), z(home,0,0), y(home,0,0);
           product(home,IntVarArgs({a,a,z}),y);
           if (home.status() == SS_FAILED)
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-10,10), y(home,20,30);
           BoolVar b(home,1,1);
           product(home,IntVarArgs({x,x}),y,Reify(b,RM_EQV));
           if ((home.status() == SS_FAILED) ||
               (x.min() != -5) || (x.max() != 5))
             return false;
         }
         return true;
       }
     };

     /// Test zero-aware inverse bounds for every cofactor sign class
     class ProductInverseBounds : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
       static bool bounds(const Gecode::IntVar& x, int min, int max) {
         return (x.min() == min) && (x.max() == max);
       }
     public:
       ProductInverseBounds(void)
         : ::Test::Base("Int::Arithmetic::Product::InverseBounds") {}
       virtual bool run(void) {
         using namespace Gecode;
         struct Case { int qmin; int qmax; int xmin; int xmax; };
         const Case cases[] = {
           { 2, 4,   5, 12}, // Positive
           { 0, 4,   5, 24}, // Nonnegative
           {-4,-2, -12, -5}, // Negative
           {-4, 0, -24, -5}, // Nonpositive
           {-4, 3, -24, 24}  // Mixed across zero
         };
         for (unsigned int i=0; i<sizeof(cases)/sizeof(Case); i++) {
           TestSpace home;
           IntVar x(home,-100,100);
           IntVar q(home,cases[i].qmin,cases[i].qmax);
           IntVar y(home,20,24);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) ||
               !bounds(x,cases[i].xmin,cases[i].xmax))
             return false;
         }
         {
           // When both the cofactor and result can be zero, no inverse
           // pruning of the other factor is sound.
           TestSpace home;
           IntVar x(home,-100,100), q(home,-2,3), y(home,-10,10);
           product(home,IntVarArgs({x,q}),y);
           if ((home.status() == SS_FAILED) || !bounds(x,-100,100))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-100,100), zero(home,0,0), y(home,1,2);
           product(home,IntVarArgs({x,zero}),y);
           if (home.status() != SS_FAILED)
             return false;
         }
         {
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar x(home,-1,1), a(home,hi,hi), b(home,hi,hi), y(home,0,0);
           product(home,IntVarArgs({x,a,b}),y);
           if ((home.status() == SS_FAILED) || !x.assigned() ||
               (x.val() != 0))
             return false;
         }
         return true;
       }
     };

     /// Compute a canonical modular product for testing.
     int product_mod_value(const Assignment& x, int n, int m) {
       long long int p = 1 % m;
       for (int i=0; i<n; i++) {
         long long int q = static_cast<long long int>(x[i]) % m;
         if (q < 0)
           q += m;
         p = (p*q) % m;
       }
       return static_cast<int>(p);
     }

     /// %Test for an ordinary and reified three-factor modular product
     class ProductModXYZR : public Test {
     protected:
       int m;
     public:
       ProductModXYZR(const std::string& s, const Gecode::IntSet& d,
                      int m0, Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductMod::XYZR::"+str(ipl)+"::"+s,
                4,d,true,ipl), m(m0) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return product_mod_value(x,3,m) == x[3];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[1],x[2]}),
                             m,x[3],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[1],x[2]}),
                             m,x[3],r,ipl);
       }
     };

     /// %Test for the empty modular product, including modulus one
     class ProductModEmpty : public Test {
     protected:
       int m;
     public:
       ProductModEmpty(const std::string& s, const Gecode::IntSet& d,
                       int m0, Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductMod::Empty::"+str(ipl)+"::"+s,
                1,d,true,ipl), m(m0) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return x[0] == (1 % m);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs(),m,x[0],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs(),m,x[0],r,ipl);
       }
     };

     /// %Test for a singleton modular product
     class ProductModSingleton : public Test {
     protected:
       int m;
     public:
       ProductModSingleton(const std::string& s, const Gecode::IntSet& d,
                           int m0, Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductMod::Singleton::"+str(ipl)+"::"+s,
                2,d,true,ipl), m(m0) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         int r = x[0] % m;
         if (r < 0)
           r += m;
         return r == x[1];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),m,x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),m,x[1],r,ipl);
       }
     };

     /// %Test repeated factors with the result aliased to a factor
     class ProductModXXYAlias : public Test {
     protected:
       int m;
     public:
       ProductModXXYAlias(const std::string& s, const Gecode::IntSet& d,
                          int m0, Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductMod::XXYAlias::"+str(ipl)+"::"+s,
                2,d,true,ipl), m(m0) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         long long int a = x[0] % m;
         long long int b = x[1] % m;
         if (a < 0) a += m;
         if (b < 0) b += m;
         return (((a*a) % m)*b) % m == x[1];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[0],x[1]}),
                             m,x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[0],x[1]}),
                             m,x[1],r,ipl);
       }
     };

     /// Targeted algebraic and quotient-band propagation for fixed modulus
     class ProductModAlgebraic : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
     public:
       ProductModAlgebraic(void)
         : ::Test::Base("Int::Arithmetic::ProductMod::Algebraic") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           IntVarArgs x(home,3,-100,100);
           IntVar y(home,-10,10);
           product_mod(home,x,1,y);
           if ((home.status() == SS_FAILED) || !y.assigned() ||
               (y.val() != 0))
             return false;
         }
         {
           TestSpace home;
           IntVar z(home,14,14), x(home,-1000,1000), y(home,0,6);
           product_mod(home,IntVarArgs({z,x}),7,y);
           if ((home.status() == SS_FAILED) || !y.assigned() ||
               (y.val() != 0))
             return false;
         }
         {
           TestSpace home;
           IntVar one(home,1,1), x(home,20,30), y(home,0,99);
           product_mod(home,IntVarArgs({one,x}),100,y);
           if ((home.status() == SS_FAILED) ||
               (y.min() != 20) || (y.max() != 30))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,15,19), y(home,0,6);
           product_mod(home,IntVarArgs({x}),7,y);
           if ((home.status() == SS_FAILED) ||
               (y.min() != 1) || (y.max() != 5))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,-20,-16), y(home,0,6);
           product_mod(home,IntVarArgs({x}),7,y);
           if ((home.status() == SS_FAILED) ||
               (y.min() != 1) || (y.max() != 5))
             return false;
         }
         {
           TestSpace home;
           IntVar c(home,6,6), x(home,-100,100), y(home,9,9);
           product_mod(home,IntVarArgs({c,x}),15,y);
           if ((home.status() == SS_FAILED) ||
               (x.min() != -96) || (x.max() != 99))
             return false;
         }
         {
           TestSpace home;
           IntVar c(home,6,6), x(home,-100,100), y(home,8,8);
           product_mod(home,IntVarArgs({c,x}),15,y);
           if (home.status() != SS_FAILED)
             return false;
         }
         {
           // The former Cartesian cutoff is exceeded by several orders of
           // magnitude, but the zero algebra remains immediate.
           TestSpace home;
           IntVar zero(home,0,0);
           IntVarArgs x(home,4,-100,100);
           x << zero;
           IntVar y(home,0,96);
           product_mod(home,x,97,y);
           if ((home.status() == SS_FAILED) || !y.assigned() ||
               (y.val() != 0))
             return false;
         }
         {
           TestSpace home;
           IntVar z(home,0,0), x(home,-100,100), y(home,0,1);
           BoolVar b(home,0,1);
           product_mod(home,IntVarArgs({z,x}),7,y,Reify(b));
           rel(home,y,IRT_EQ,0);
           if ((home.status() == SS_FAILED) || !b.assigned() ||
               (b.val() != 1))
             return false;
         }
         return true;
       }
     };

     /// %Test for an ordinary two-factor product with a variable modulus
     class ProductModVarXYMR : public Test {
     public:
       ProductModVarXYMR(const std::string& s, const Gecode::IntSet& d,
                         Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::XYMR::"+str(ipl)+"::"+s,
                4,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return (x[2] > 0) &&
           (product_mod_value(x,2,x[2]) == x[3]);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[1]}),
                             x[2],x[3],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[1]}),
                             x[2],x[3],r,ipl);
       }
     };

     /// %Test for the empty product with a variable modulus
     class ProductModVarEmpty : public Test {
     public:
       ProductModVarEmpty(const std::string& s, const Gecode::IntSet& d,
                          Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::Empty::"+str(ipl)+"::"+s,
                2,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return (x[0] > 0) && (x[1] == (1 % x[0]));
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs(),x[0],x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs(),x[0],x[1],r,ipl);
       }
     };

     /// %Test for a singleton product with a variable modulus
     class ProductModVarSingleton : public Test {
     public:
       ProductModVarSingleton(const std::string& s, const Gecode::IntSet& d,
                              Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::Singleton::"+str(ipl)+"::"+s,
                3,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         if (x[1] <= 0)
           return false;
         int r = x[0] % x[1];
         if (r < 0)
           r += x[1];
         return r == x[2];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[2],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[2],r,ipl);
       }
     };

     /// %Test repeated factors with a variable modulus
     class ProductModVarRepeated : public Test {
     public:
       ProductModVarRepeated(const std::string& s, const Gecode::IntSet& d,
                             Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::Repeated::"+str(ipl)+"::"+s,
                3,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         if (x[1] <= 0)
           return false;
         long long int a = x[0] % x[1];
         if (a < 0) a += x[1];
         return (a*a) % x[1] == x[2];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[0]}),
                             x[1],x[2],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0],x[0]}),
                             x[1],x[2],r,ipl);
       }
     };

     /// %Test modulus/factor aliasing
     class ProductModVarModFactorAlias : public Test {
     public:
       ProductModVarModFactorAlias(const std::string& s,
                                   const Gecode::IntSet& d,
                                   Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::ModFactorAlias::"+
                str(ipl)+"::"+s,2,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         return (x[0] > 0) && (x[1] == 0);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[0],x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[0],x[1],r,ipl);
       }
     };

     /// %Test factor/result aliasing
     class ProductModVarFactorResultAlias : public Test {
     public:
       ProductModVarFactorResultAlias(const std::string& s,
                                      const Gecode::IntSet& d,
                                      Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::FactorResultAlias::"+
                str(ipl)+"::"+s,2,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment& x) const {
         if (x[1] <= 0)
           return false;
         int r = x[0] % x[1];
         if (r < 0) r += x[1];
         return r == x[0];
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[0],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[0],r,ipl);
       }
     };

     /// %Test modulus/result aliasing, which is necessarily inconsistent
     class ProductModVarModResultAlias : public Test {
     public:
       ProductModVarModResultAlias(const std::string& s,
                                   const Gecode::IntSet& d,
                                   Gecode::IntPropLevel ipl)
         : Test("Arithmetic::ProductModVar::ModResultAlias::"+
                str(ipl)+"::"+s,2,d,true,ipl) { contest=CTL_NONE; testfix=false; }
       virtual bool solution(const Assignment&) const { return false; }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[1],ipl);
       }
       virtual void post(Gecode::Space& home, Gecode::IntVarArray& x,
                         Gecode::Reify r) {
         Gecode::product_mod(home,Gecode::IntVarArgs({x[0]}),
                             x[1],x[1],r,ipl);
       }
     };

     /// Targeted bounds and arithmetic checks for a variable modulus
     class ProductModVarBounds : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
     public:
       ProductModVarBounds(void)
         : ::Test::Base("Int::Arithmetic::ProductModVar::Bounds") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           const int vm[5] = {-7,0,1,4,9};
           IntVar m(home,IntSet(vm,5));
           IntVar y(home,-5,12);
           product_mod(home,IntVarArgs(),m,y);
           if ((home.status() == SS_FAILED) || (m.min() != 1) ||
               (y.min() != 0) || (y.max() >= m.max()))
             return false;
         }
         {
           TestSpace home;
           IntVar x(home,Gecode::Int::Limits::max,Gecode::Int::Limits::max);
           IntVar m(home,Gecode::Int::Limits::max,Gecode::Int::Limits::max);
           IntVar y(home,0,0);
           product_mod(home,IntVarArgs({x}),m,y);
           if (home.status() == SS_FAILED)
             return false;
         }
         return true;
       }
     };

     /// Targeted algebraic propagation for a variable modulus
     class ProductModVarAlgebraic : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
       static bool domain(const Gecode::IntVar& x, const int* v, int n) {
         if (x.size() != static_cast<unsigned int>(n))
           return false;
         for (int i=0; i<n; i++)
           if (!x.in(v[i]))
             return false;
         return true;
       }
     public:
       ProductModVarAlgebraic(void)
         : ::Test::Base("Int::Arithmetic::ProductModVar::Algebraic") {}
       virtual bool run(void) {
         using namespace Gecode;
         {
           TestSpace home;
           IntVar m(home,2,Gecode::Int::Limits::max);
           IntVar z(home,-100,100);
           IntVar y(home,0,100);
           product_mod(home,IntVarArgs({z,m}),m,y);
           if ((home.status() == SS_FAILED) || !y.assigned() || (y.val()!=0))
             return false;
         }
         {
           TestSpace home;
           const int mv[5] = {-3,0,1,7,1000000};
           IntVar m(home,IntSet(mv,5));
           IntVar y(home,0,0);
           product_mod(home,IntVarArgs(),m,y);
           if ((home.status() == SS_FAILED) || !m.assigned() || (m.val()!=1))
             return false;
         }
         {
           TestSpace home;
           const int mv[4] = {1,2,17,1000000};
           const int keep[3] = {2,17,1000000};
           IntVar m(home,IntSet(mv,4));
           IntVar y(home,1,1);
           product_mod(home,IntVarArgs(),m,y);
           if ((home.status() == SS_FAILED) || !domain(m,keep,3))
             return false;
         }
         {
           TestSpace home;
           const int mv[10] = {5,7,8,9,14,16,28,55,56,1000000};
           IntVar a(home,6,6), c(home,10,10);
           IntVar m(home,IntSet(mv,10));
           IntVar y(home,4,4);
           product_mod(home,IntVarArgs({a,c}),m,y);
           if ((home.status() == SS_FAILED) || (m.min() != 7) ||
               (m.max() != 56))
             return false;
         }
         {
           TestSpace home;
           const int mv[7] = {4,5,6,7,10,11,60};
           IntVar a(home,6,6), c(home,10,10);
           IntVar m(home,IntSet(mv,7));
           IntVar y(home,0,0);
           product_mod(home,IntVarArgs({a,c}),m,y);
           if ((home.status() == SS_FAILED) || (m.min() != 4) ||
               (m.max() != 60))
             return false;
         }
         {
           TestSpace home;
           const int mv[3] = {13,101,1000000};
           IntVar y(home,12,12);
           IntVar m(home,IntSet(mv,3));
           product_mod(home,IntVarArgs({y}),m,y);
           if ((home.status() == SS_FAILED) || !domain(m,mv,3))
             return false;
         }
         {
           TestSpace home;
           const int mv[3] = {2,17,1000000};
           IntVar z(home,0,0), y(home,0,0);
           IntVar m(home,IntSet(mv,3));
           product_mod(home,IntVarArgs({z}),m,y);
           if ((home.status() == SS_FAILED) || !domain(m,mv,3))
             return false;
         }
         {
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar a(home,hi,hi), c(home,hi,hi), d(home,hi,hi);
           IntVar m(home,2,hi), y(home,0,0);
           product_mod(home,IntVarArgs({a,c,d}),m,y);
           if (home.status() == SS_FAILED)
             return false;
         }
         {
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar x(home,hi,hi), m(home,1,hi), y(home,0,0);
           product_mod(home,IntVarArgs({x}),m,y);
           if ((home.status() == SS_FAILED) || (m.min() != 1) ||
               (m.max() != hi))
             return false;
         }
         {
           TestSpace home;
           const int hi=Gecode::Int::Limits::max;
           IntVar x(home,hi,hi), m(home,2,hi), y(home,1,1);
           product_mod(home,IntVarArgs({x}),m,y);
           if ((home.status() == SS_FAILED) || (m.min() != 5) ||
               (m.max() != hi-1))
             return false;
         }
         return true;
       }
     };

     /// Verify that inactive implication modes leave all integer views alone
     class ProductModVarInactive : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
       static bool unchanged(const Gecode::IntVar& x, const int* v, int n) {
         if (x.size() != static_cast<unsigned int>(n))
           return false;
         for (int i=0; i<n; i++)
           if (!x.in(v[i]))
             return false;
         return true;
       }
     public:
       ProductModVarInactive(void)
         : ::Test::Base("Int::Arithmetic::ProductModVar::Inactive") {}
       virtual bool run(void) {
         using namespace Gecode;
         const int xv[5] = {-3,-1,0,2,5};
         const int mv[4] = {-2,0,1,4};
         const int yv[4] = {-4,0,3,7};
         for (int mode=0; mode<2; mode++) {
           TestSpace home;
           IntVar x(home,IntSet(xv,5));
           IntVar m(home,IntSet(mv,4));
           IntVar y(home,IntSet(yv,4));
           BoolVar b(home,mode,mode);
           Reify r(b,mode == 0 ? RM_IMP : RM_PMI);
           product_mod(home,IntVarArgs({x,x}),m,y,r);
           if ((home.status() == SS_FAILED) ||
               !unchanged(x,xv,5) || !unchanged(m,mv,4) ||
               !unchanged(y,yv,4))
             return false;
         }
         return true;
       }
     };

     /// Staged assigned leaves above the former Cartesian support cutoff
     class ArithmeticLargeLeaves : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
       static bool control(const Gecode::BoolVar& b, Gecode::ReifyMode rm,
                           bool truth) {
         if (truth && (rm != Gecode::RM_IMP))
           return b.assigned() && (b.val() == 1);
         if (!truth && (rm != Gecode::RM_PMI))
           return b.assigned() && (b.val() == 0);
         return true;
       }
     public:
       ArithmeticLargeLeaves(void)
         : ::Test::Base("Int::Arithmetic::LargeAssignedLeaves") {}
       virtual bool run(void) {
         using namespace Gecode;
         const ReifyMode rms[] = {RM_EQV,RM_IMP,RM_PMI};
         for (unsigned int r=0; r<3; r++)
           for (int truth=0; truth<=1; truth++) {
             {
               TestSpace home;
               IntVar a(home,1,1000), c(home,1,1000), g(home,0,1000);
               BoolVar b(home,0,1);
               gcd(home,a,c,g,Reify(b,rms[r]));
               rel(home,a,IRT_EQ,84); rel(home,c,IRT_EQ,30);
               rel(home,g,IRT_EQ,truth ? 6 : 7);
               if ((home.status() == SS_FAILED) || !control(b,rms[r],truth))
                 return false;
             }
             {
               TestSpace home;
               IntVar d(home,1,1000), n(home,1,1000); BoolVar b(home,0,1);
               divides(home,d,n,Reify(b,rms[r]));
               rel(home,d,IRT_EQ,truth ? 7 : 8); rel(home,n,IRT_EQ,84);
               if ((home.status() == SS_FAILED) || !control(b,rms[r],truth))
                 return false;
             }
             {
               TestSpace home;
               IntVarArgs x(home,3,1,1000); IntVar y(home,1,1000000000);
               BoolVar b(home,0,1); product(home,x,y,Reify(b,rms[r]));
               rel(home,x[0],IRT_EQ,2); rel(home,x[1],IRT_EQ,3);
               rel(home,x[2],IRT_EQ,5); rel(home,y,IRT_EQ,truth ? 30 : 31);
               if ((home.status() == SS_FAILED) || !control(b,rms[r],truth))
                 return false;
             }
             {
               TestSpace home;
               IntVarArgs x(home,3,1,1000); IntVar y(home,0,96);
               BoolVar b(home,0,1);
               product_mod(home,x,97,y,Reify(b,rms[r]));
               rel(home,x[0],IRT_EQ,2); rel(home,x[1],IRT_EQ,3);
               rel(home,x[2],IRT_EQ,5); rel(home,y,IRT_EQ,truth ? 30 : 31);
               if ((home.status() == SS_FAILED) || !control(b,rms[r],truth))
                 return false;
             }
             {
               TestSpace home;
               IntVarArgs x(home,3,1,1000); IntVar m(home,1,1000);
               IntVar y(home,0,999); BoolVar b(home,0,1);
               product_mod(home,x,m,y,Reify(b,rms[r]));
               rel(home,x[0],IRT_EQ,2); rel(home,x[1],IRT_EQ,3);
               rel(home,x[2],IRT_EQ,5); rel(home,m,IRT_EQ,97);
               rel(home,y,IRT_EQ,truth ? 30 : 31);
               if ((home.status() == SS_FAILED) || !control(b,rms[r],truth))
                 return false;
             }
           }
         return true;
       }
     };

     /// %Test that a nonpositive modular-product modulus is rejected
     class ProductModInvalidModulus : public ::Test::Base {
     protected:
       class TestSpace : public Gecode::Space {
       public:
         virtual Gecode::Space* copy(void) { return nullptr; }
       };
     public:
       ProductModInvalidModulus(void)
         : ::Test::Base("Int::Arithmetic::ProductMod::InvalidModulus") {}
       virtual bool run(void) {
         TestSpace home;
         Gecode::IntVar y(home,0,1);
         for (int m=0; m>=-1; m--) {
           try {
             Gecode::product_mod(home,Gecode::IntVarArgs(),m,y);
             return false;
           } catch (const Gecode::Int::OutOfLimits&) {
           }
         }
         return true;
       }
     };

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
         const int vg[7] = {-12,-6,-1,0,4,9,12};
         Gecode::IntSet g(vg,7);
         const int vp[5] = {
           Gecode::Int::Limits::min,-1,0,1,Gecode::Int::Limits::max
         };
         Gecode::IntSet p(vp,5);
         Gecode::IntSet q(-2,2);

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
           (void) new GcdXYZ("C",c,ipls.ipl());
           (void) new GcdXYZ("Sparse",g,ipls.ipl());
           (void) new GcdXXY("C",c,ipls.ipl());
           (void) new GcdXYX("C",c,ipls.ipl());
           (void) new GcdXXX("C",c,ipls.ipl());

           (void) new DividesXY("C",c,ipls.ipl());
           (void) new DividesXY("Sparse",g,ipls.ipl());
           (void) new DividesXX("C",c,ipls.ipl());

           (void) new ProductXYZR("C",q,ipls.ipl());
           (void) new ProductXYZR("Sparse",g,ipls.ipl());
           (void) new ProductXYZR("Limits",p,ipls.ipl());
           (void) new ProductEmpty("C",q,ipls.ipl());
           (void) new ProductSingleton("C",q,ipls.ipl());
           (void) new ProductXXYAlias("C",q,ipls.ipl());

           (void) new ProductModXYZR("C",q,5,ipls.ipl());
           (void) new ProductModXYZR("Sparse",g,7,ipls.ipl());
           (void) new ProductModEmpty("C",q,5,ipls.ipl());
           (void) new ProductModEmpty("ModulusOne",q,1,ipls.ipl());
           (void) new ProductModSingleton("C",g,5,ipls.ipl());
           (void) new ProductModXXYAlias("C",q,5,ipls.ipl());

           (void) new ProductModVarXYMR("C",q,ipls.ipl());
           (void) new ProductModVarXYMR("Sparse",g,ipls.ipl());
           (void) new ProductModVarEmpty("C",q,ipls.ipl());
           (void) new ProductModVarSingleton("C",q,ipls.ipl());
           (void) new ProductModVarRepeated("C",q,ipls.ipl());
           (void) new ProductModVarModFactorAlias("C",q,ipls.ipl());
           (void) new ProductModVarFactorResultAlias("C",q,ipls.ipl());
           (void) new ProductModVarModResultAlias("C",q,ipls.ipl());

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
         (void) new ProductModInvalidModulus;
         (void) new ProductModAlgebraic;
         (void) new ProductModVarBounds;
         (void) new ProductModVarAlgebraic;
         (void) new ProductModVarInactive;
         (void) new ArithmeticLargeLeaves;
         (void) new ProductBoundsLarge;
         (void) new ProductSimplifySign;
         (void) new ProductPowerAlias;
         (void) new ProductInverseBounds;
       }
     };

     Create c;
     //@}

   }
}}

// STATISTICS: test-int
