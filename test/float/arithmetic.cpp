/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Vincent Barichard, 2012
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

#include "test/float.hh"

#include <gecode/minimodel.hh>

#include <cmath>
#include <algorithm>
#include <cfenv>
#include <limits>

namespace Test { namespace Float {

   /// %Tests for arithmetic constraints
   namespace Arithmetic {

     /**
      * \defgroup TaskTestFloatArithmetic Arithmetic constraints
      * \ingroup TaskTestFloat
      */
     //@{
     /// Regression test for outward positive even-root bounds
     class PositiveNRootBounds : public Base {
     protected:
       /// Space exercising the Pow and NthRoot propagators
       class RootSpace : public Gecode::Space {
       public:
         /// Variables used by the propagator
         Gecode::FloatVar x0, x1;
         /// Post an extreme Pow or NthRoot instance
         RootSpace(Gecode::FloatNum hi, Gecode::FloatNum witness, int n,
                   bool usePow)
           : x0(*this, 0.0, usePow ? witness : hi),
             x1(*this, usePow ? 0.0 : witness, usePow ? hi : witness) {
           if (usePow) {
             Gecode::pow(*this,x0,n,x1);
             Gecode::rel(*this,x0,Gecode::FRT_EQ,witness);
           } else {
             Gecode::nroot(*this,x0,n,x1);
           }
         }
         /// Clone constructor
         RootSpace(RootSpace& s) : Gecode::Space(s) {
           x0.update(*this,s.x0);
           x1.update(*this,s.x1);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new RootSpace(*this);
         }
       };

       /// Space exposing the propagated NthRoot upper bound
       class NRootBoundSpace : public Gecode::Space {
       public:
         /// Source and root variables
         Gecode::FloatVar source, root;
         /// Post an NthRoot constraint with an unconstrained root
         NRootBoundSpace(Gecode::FloatNum hi, int n)
           : source(*this,0.0,hi),
             root(*this,0.0,Gecode::Float::Limits::max) {
           Gecode::nroot(*this,source,n,root);
         }
         /// Clone constructor
         NRootBoundSpace(NRootBoundSpace& s) : Gecode::Space(s) {
           source.update(*this,s.source);
           root.update(*this,s.root);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new NRootBoundSpace(*this);
         }
       };

       /// Check that a known feasible boundary value is not removed
       bool check(Gecode::FloatNum hi, Gecode::FloatNum witness, int n,
                  int mode, bool usePow) const {
         if (std::fesetround(mode) != 0)
           return true;
         RootSpace s(hi,witness,n,usePow);
         if (s.status() != Gecode::SS_FAILED)
           return true;
         olog << "Root bound removed a solution for "
              << (usePow ? "pow" : "nroot") << ", n=" << n
              << ", mode=" << mode << std::endl;
         return false;
       }
       /// Check that propagation returns a genuinely outward upper bound
       bool checkBound(Gecode::FloatNum hi, Gecode::FloatNum rejected,
                       int n, int mode) const {
         if (std::fesetround(mode) != 0)
           return true;
         NRootBoundSpace s(hi,n);
         if ((s.status() != Gecode::SS_FAILED) &&
             (s.root.max() > rejected))
           return true;
         olog << "NRoot upper bound was not outward for n=" << n
              << ", mode=" << mode << std::endl;
         return false;
       }
     public:
       /// Create and register test
       PositiveNRootBounds(void)
         : Base("Float::Arithmetic::PositiveNRootBounds") {}
       /// Run test under every supported IEEE-754 rounding mode
       virtual bool run(void) {
         const int oldMode = std::fegetround();
         const int modes[] = {
           FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO
         };
         struct Case {
           Gecode::FloatNum hi;
           Gecode::FloatNum witness;
           int n;
         } cases[] = {
           {std::ldexp(static_cast<Gecode::FloatNum>(0x156e1fc2f8f359ULL),
                       -1049),
            std::ldexp(static_cast<Gecode::FloatNum>(0x14484bfeebc28cULL),
                       -152), 10},
           {std::numeric_limits<Gecode::FloatNum>::denorm_min(),
            std::ldexp(static_cast<Gecode::FloatNum>(0x18406003b2ae42ULL),
                       -160), 10},
           {std::numeric_limits<Gecode::FloatNum>::max(),
            std::ldexp(static_cast<Gecode::FloatNum>(0x1965fea53d6e0fULL),
                       118), 6}
         };
         bool result = true;
         const Gecode::FloatNum roundingHi =
           std::ldexp(static_cast<Gecode::FloatNum>(0x1751def03d6f38ULL),
                      458);
         const Gecode::FloatNum rejectedUpper =
           std::ldexp(static_cast<Gecode::FloatNum>(0x1350f4c43079ceULL),
                      203);
         for (unsigned int m=0; m<sizeof(modes)/sizeof(modes[0]); m++) {
           for (unsigned int c=0; c<sizeof(cases)/sizeof(cases[0]); c++) {
             result = check(cases[c].hi,cases[c].witness,cases[c].n,
                            modes[m],false) && result;
             result = check(cases[c].hi,cases[c].witness,cases[c].n,
                            modes[m],true) && result;
           }
           result = checkBound(roundingHi,rejectedUpper,2,modes[m]) && result;
         }
         if (oldMode != -1)
           std::fesetround(oldMode);
         return result;
       }
     } positive_nroot_bounds;

     /// Regression tests for Pow zero semantics and propagation fixpoints
     class PowConsistency : public Base {
     protected:
       /// Space for direct and delayed assignment of the base to zero
       class ZeroSpace : public Gecode::Space {
       public:
         /// Base and result variables
         Gecode::FloatVar x, y;
         /// Post a power constraint with either a zero or a wide base
         ZeroSpace(int n, bool direct)
           : x(*this,direct ? 0.0 : -1.0,direct ? 0.0 : 1.0),
             y(*this,-1.0,2.0) {
           Gecode::pow(*this,x,n,y);
         }
         /// Clone constructor
         ZeroSpace(ZeroSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x);
           y.update(*this,s.y);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new ZeroSpace(*this);
         }
       };

       /// Space for an adjacent interval containing zero at one endpoint
       class AdjacentZeroSpace : public Gecode::Space {
       public:
         /// Base and result variables
         Gecode::FloatVar x, y;
         /// Post exponent zero on a positive or negative adjacent interval
         AdjacentZeroSpace(bool positive)
           : x(*this,
               positive ? 0.0
                        : -std::numeric_limits<Gecode::FloatNum>::denorm_min(),
               positive ? std::numeric_limits<Gecode::FloatNum>::denorm_min()
                        : -0.0),
             y(*this,-1.0,2.0) {
           Gecode::pow(*this,x,0,y);
         }
         /// Clone constructor
         AdjacentZeroSpace(AdjacentZeroSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x);
           y.update(*this,s.y);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new AdjacentZeroSpace(*this);
         }
       };

       /// Space reproducing a Pow inverse-to-forward fixpoint
       class FixpointSpace : public Gecode::Space {
       public:
         /// Base and result variables
         Gecode::FloatVar x, y;
         /// Post the fixpoint regression instance
         FixpointSpace(void)
           : x(*this,
               -std::ldexp(static_cast<Gecode::FloatNum>
                           (0x10000000000006ULL),-53),
               -std::ldexp(static_cast<Gecode::FloatNum>
                           (0x10000000000004ULL),-53)),
             y(*this,
               std::ldexp(static_cast<Gecode::FloatNum>
                          (0x1000000000000aULL),-54),
               std::ldexp(static_cast<Gecode::FloatNum>
                          (0x1000000000000cULL),-54)) {
           Gecode::pow(*this,x,2,y);
         }
         /// Clone constructor
         FixpointSpace(FixpointSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x);
           y.update(*this,s.y);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new FixpointSpace(*this);
         }
       };

       /// Space for direct and delayed assignment of the result to zero
       class ZeroResultSpace : public Gecode::Space {
       public:
         /// Base and result variables
         Gecode::FloatVar x, y;
         /// Post a power constraint with either a zero or a wide result
         ZeroResultSpace(int n, bool direct)
           : x(*this,-1.0,1.0),
             y(*this,direct ? 0.0 : -1.0,direct ? 0.0 : 1.0) {
           Gecode::pow(*this,x,n,y);
         }
         /// Clone constructor
         ZeroResultSpace(ZeroResultSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x);
           y.update(*this,s.y);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new ZeroResultSpace(*this);
         }
       };

       /// Test one exponent with direct and delayed zero assignment
       bool checkZero(int n) const {
         ZeroSpace direct(n,true);
         ZeroSpace delayed(n,false);
         if (delayed.status() == Gecode::SS_FAILED) {
           olog << "Pow wide base failed for n=" << n << std::endl;
           return false;
         }
         ZeroSpace* clone = static_cast<ZeroSpace*>(delayed.clone());
         Gecode::rel(*clone,clone->x,Gecode::FRT_EQ,0.0);
         if (n == 0) {
           const bool zeroRejected =
             (direct.status() == Gecode::SS_FAILED) &&
             (clone->status() == Gecode::SS_FAILED);
           delete clone;
           ZeroSpace* nonZero = static_cast<ZeroSpace*>(delayed.clone());
           Gecode::rel(*nonZero,nonZero->x,Gecode::FRT_EQ,0.5);
           const bool nonZeroAccepted =
             (nonZero->status() != Gecode::SS_FAILED) &&
             (nonZero->y.min() == 1.0) && (nonZero->y.max() == 1.0) &&
             (Gecode::PropagatorGroup::all.size(*nonZero) == 0);
           delete nonZero;
           if (!zeroRejected || !nonZeroAccepted)
             olog << "Pow exponent-zero semantics failed" << std::endl;
           return zeroRejected && nonZeroAccepted;
         }
         const bool result =
           (direct.status() != Gecode::SS_FAILED) &&
           (direct.y.min() == 0.0) && (direct.y.max() == 0.0) &&
           (clone->status() != Gecode::SS_FAILED) &&
           (clone->y.min() == 0.0) && (clone->y.max() == 0.0);
         delete clone;
         if (!result)
           olog << "Pow delayed zero failed for n=" << n << std::endl;
         return result;
       }
       /// Test that a zero result forces a zero base
       bool checkZeroResult(int n, bool direct) const {
         ZeroResultSpace initial(n,direct);
         if (initial.status() == Gecode::SS_FAILED) {
           olog << "Pow zero result failed for n=" << n << std::endl;
           return false;
         }
         ZeroResultSpace* clone =
           static_cast<ZeroResultSpace*>(initial.clone());
         if (!direct)
           Gecode::rel(*clone,clone->y,Gecode::FRT_EQ,0.0);
         const bool contracted =
           (clone->status() != Gecode::SS_FAILED) &&
           (clone->x.min() == 0.0) && (clone->x.max() == 0.0);
         Gecode::rel(*clone,clone->x,Gecode::FRT_EQ,1.0);
         const bool inconsistentRejected =
           clone->status() == Gecode::SS_FAILED;
         delete clone;
         if (!contracted || !inconsistentRejected)
           olog << "Pow zero result did not force zero base for n=" << n
                << (direct ? " (direct)" : " (delayed)") << std::endl;
         return contracted && inconsistentRejected;
       }
       /// Test exponent zero on an adjacent interval containing zero
       bool checkAdjacentZero(bool positive) const {
         AdjacentZeroSpace initial(positive);
         const bool actorRetained =
           (initial.status() != Gecode::SS_FAILED) &&
           (initial.y.min() == 1.0) && (initial.y.max() == 1.0) &&
           (Gecode::PropagatorGroup::all.size(initial) == 1);

         AdjacentZeroSpace* clone =
           static_cast<AdjacentZeroSpace*>(initial.clone());
         const bool cloneRetained =
           (clone->status() != Gecode::SS_FAILED) &&
           (clone->y.min() == 1.0) && (clone->y.max() == 1.0) &&
           (Gecode::PropagatorGroup::all.size(*clone) == 1);
         delete clone;

         if (!actorRetained || !cloneRetained)
           olog << "Pow adjacent exponent-zero semantics failed"
                << (positive ? " (positive)" : " (negative)") << std::endl;
         return actorRetained && cloneRetained;
       }
     public:
       /// Create and register test
       PowConsistency(void) : Base("Float::Arithmetic::PowConsistency") {}
       /// Run zero and fixpoint regressions
       virtual bool run(void) {
         bool result = true;
         for (int n=0; n<=3; n++)
           result = checkZero(n) && result;
         for (int n=1; n<=3; n++) {
           result = checkZeroResult(n,true) && result;
           result = checkZeroResult(n,false) && result;
         }
         result = checkAdjacentZero(true) && result;
         result = checkAdjacentZero(false) && result;

         FixpointSpace once;
         if (once.status() == Gecode::SS_FAILED) {
           olog << "Pow fixpoint instance failed" << std::endl;
           return false;
         }
         FixpointSpace* twice = static_cast<FixpointSpace*>(once.clone());
         Gecode::pow(*twice,twice->x,2,twice->y);
         const bool fixpoint =
           (twice->status() != Gecode::SS_FAILED) &&
           (once.x.min() == twice->x.min()) &&
           (once.x.max() == twice->x.max()) &&
           (once.y.min() == twice->y.min()) &&
           (once.y.max() == twice->y.max());
         delete twice;
         if (!fixpoint)
           olog << "Pow posting did not reach a fixpoint" << std::endl;
         return fixpoint && result;
       }
     } pow_consistency;

     /// Regression tests for multiplication through a zero endpoint
     class MultZeroEndpoint : public Base {
     protected:
       /// Space exposing multiplication propagation at the public API
       class MultSpace : public Gecode::Space {
       public:
         /// Factors and product
         Gecode::FloatVar x, y, z;
         /// Post x * y = z for the supplied intervals
         MultSpace(Gecode::FloatNum xl, Gecode::FloatNum xu,
                   Gecode::FloatNum yl, Gecode::FloatNum yu,
                   Gecode::FloatNum zl, Gecode::FloatNum zu)
           : x(*this,xl,xu), y(*this,yl,yu), z(*this,zl,zu) {
           Gecode::mult(*this,x,y,z);
         }
         /// Clone constructor
         MultSpace(MultSpace& s) : Gecode::Space(s) {
           x.update(*this,s.x);
           y.update(*this,s.y);
           z.update(*this,s.z);
         }
         /// Copy space during cloning
         virtual Gecode::Space* copy(void) {
           return new MultSpace(*this);
         }
       };

       /// Check one endpoint-zero sign and magnitude contraction
       bool check(Gecode::FloatNum xl, Gecode::FloatNum xu,
                  Gecode::FloatNum yl, Gecode::FloatNum yu,
                  Gecode::FloatNum zl, Gecode::FloatNum zu,
                  Gecode::FloatNum expectedMin,
                  Gecode::FloatNum expectedMax) const {
         MultSpace s(xl,xu,yl,yu,zl,zu);
         if ((s.status() == Gecode::SS_FAILED) ||
             (s.x.min() < expectedMin) || (s.x.max() > expectedMax)) {
           olog << "Multiplication endpoint-zero contraction failed: x="
                << s.x << ", y=" << s.y << ", z=" << s.z << std::endl;
           return false;
         }
         MultSpace* clone = static_cast<MultSpace*>(s.clone());
         Gecode::mult(*clone,clone->x,clone->y,clone->z);
         const bool fixpoint =
           (clone->status() != Gecode::SS_FAILED) &&
           (clone->x.min() == s.x.min()) && (clone->x.max() == s.x.max()) &&
           (clone->y.min() == s.y.min()) && (clone->y.max() == s.y.max()) &&
           (clone->z.min() == s.z.min()) && (clone->z.max() == s.z.max());
         delete clone;
         if (!fixpoint)
           olog << "Multiplication endpoint-zero case was not at a fixpoint"
                << std::endl;
         return fixpoint;
       }
       /// Exercise supported points across endpoint-zero sign combinations
       bool fuzz(void) const {
         unsigned int state = 0x6d756c74U;
         for (unsigned int i=0; i<5000; i++) {
           state = state * 1664525U + 1013904223U;
           const Gecode::FloatNum am =
             0.125 + static_cast<Gecode::FloatNum>((state >> 8) % 8000) /
                     512.0;
           state = state * 1664525U + 1013904223U;
           const Gecode::FloatNum bm =
             0.125 + static_cast<Gecode::FloatNum>((state >> 8) % 8000) /
                     512.0;
           const Gecode::FloatNum a = (i & 1U) ? -am : am;
           const Gecode::FloatNum b = (i & 2U) ? -bm : bm;
           const Gecode::FloatNum p = a * b;
           const Gecode::FloatNum slack = 1.0 + am / 4.0;
           const Gecode::FloatNum xl = a - slack;
           const Gecode::FloatNum xu = a + slack;
           const Gecode::FloatNum yl = (b < 0.0) ? -(bm + 1.0) : 0.0;
           const Gecode::FloatNum yu = (b < 0.0) ? -0.0 : bm + 1.0;
           MultSpace s(xl,xu,yl,yu,p,p);
           if ((s.status() == Gecode::SS_FAILED) ||
               (s.x.min() > a) || (s.x.max() < a) ||
               (s.y.min() > b) || (s.y.max() < b) ||
               (s.z.min() > p) || (s.z.max() < p)) {
             olog << "Multiplication removed supported fuzz point " << i
                  << ": (" << a << ", " << b << ", " << p << ")"
                  << std::endl;
             return false;
           }
         }
         return true;
       }
     public:
       /// Create and register test
       MultZeroEndpoint(void)
         : Base("Float::Arithmetic::MultZeroEndpoint") {}
       /// Run sign, symmetry, signed-zero, and zero-product cases
       virtual bool run(void) {
         bool result = true;
         result = check(-1.0,1.0, 0.0,1.0, 0.5,1.0,
                        0.5,1.0) && result;
         result = check(-1.0,1.0,-1.0,-0.0, 0.5,1.0,
                       -1.0,-0.5) && result;
         result = check(-1.0,1.0, 0.0,1.0,-1.0,-0.5,
                       -1.0,-0.5) && result;
         result = check(-1.0,1.0,-1.0,-0.0,-1.0,-0.5,
                        0.5,1.0) && result;

         // Swapping the factors must recover the same contraction.
         MultSpace swapped(0.0,1.0,-1.0,1.0,0.5,1.0);
         result = (swapped.status() != Gecode::SS_FAILED) &&
                  (swapped.y.min() >= 0.5) && result;

         // A zero product gives no sign information about the other factor.
         MultSpace zeroProduct(-1.0,1.0,0.0,1.0,0.0,0.0);
         result = (zeroProduct.status() != Gecode::SS_FAILED) &&
                  (zeroProduct.x.min() == -1.0) &&
                  (zeroProduct.x.max() == 1.0) && result;
         MultSpace zeroFactor(0.0,0.0,-1.0,1.0,0.0,0.0);
         result = (zeroFactor.status() != Gecode::SS_FAILED) &&
                  (zeroFactor.y.min() == -1.0) &&
                  (zeroFactor.y.max() == 1.0) && result;
         if (!result)
           olog << "Multiplication zero or swapped regression failed"
                << std::endl;
         return fuzz() && result;
       }
     } mult_zero_endpoint;

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
       virtual bool extendAssignment(Assignment& x) const {
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
       virtual bool extendAssignment(Assignment& x) const {
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
       virtual bool extendAssignment(Assignment& x) const {
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
       virtual bool extendAssignment(Assignment& x) const {
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
       virtual bool extendAssignment(Assignment& x) const {
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
       virtual bool extendAssignment(Assignment& x) const {
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
       : Test("Arithmetic::Pow::N::"+str(_n)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false), n(_n) {}
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
         if ((n == 0) || (x[0].min() < 0.0))
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
         if ((n == 0) || (x[0].min() < 0.0))
           return MT_FALSE;
         return eq(nroot(x[0],n), x[1]);
       }
       /// Extend assignment \a x
       virtual bool extendAssignment(Assignment& x) const {
         if ((n == 0) || (x[0].min() < 0))
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
       : Test("Arithmetic::NRoot::N::"+str(_n)+"::XX::"+s,1,d,st,CPLT_ASSIGNMENT,false), n(_n) {}
       /// %Test whether \a x is solution
       virtual MaybeType solution(const Assignment& x) const {
         if ((n == 0) || (x[0].min() < 0))
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
