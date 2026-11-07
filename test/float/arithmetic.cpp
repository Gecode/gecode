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
#include <cstddef>
#include <ios>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

/// %Tests for arithmetic constraints
namespace Test::Float::Arithmetic {

    /**
     * \defgroup TaskTestFloatArithmetic Arithmetic constraints
     * \ingroup TaskTestFloat
     */
    //@{
    // A copied scope would restore the caller's mode a second time.
    static_assert(!std::is_copy_constructible_v<Gecode::Float::Rounding> &&
                  !std::is_copy_assignable_v<Gecode::Float::Rounding>);

    /// Rounding scopes and interval operations restore the enclosing mode
    class RoundingScope : public Base {
    private:
        /// Test interval operations that open their own rounding scope
        static bool
        interval_operations() {
            using Gecode::Float::Interval;
            const int mode = std::fegetround();
            // fmod runs its nested arithmetic in its own scope.
            const Interval remainder = Gecode::Float::fmod(Interval(5.0, 5.0), Interval(3.0, 3.0));
            bool result = (remainder.lower() == 2.0) && (remainder.upper() == 2.0) && (std::fegetround() == mode);
#ifdef GECODE_HAS_MPFR
            // Nested operations must retain Gecode's MPFR operations.
            const Interval sine = Gecode::Float::sin(Interval(0.5));
            result &= (sine.lower() <= 0.479425538604203) && (sine.upper() >= 0.479425538604203) &&
                      (std::fegetround() == mode);
#endif
            return result;
        }

    public:
        RoundingScope() : Base("Float::Arithmetic::RoundingScope", TestTags::check()) {}
        bool
        run() override {
            const int previous = std::fegetround();
            const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
            bool result = true;
            for (int mode : modes) {
                if (std::fesetround(mode) != 0)
                    continue;
                {
                    Gecode::Float::Rounding outer;
                    const volatile Gecode::FloatNum one = 1.0;
                    const volatile Gecode::FloatNum half_ulp = std::ldexp(1.0, -53);
                    const Gecode::FloatNum lower = outer.add_down(one, half_ulp);
                    const Gecode::FloatNum upper = outer.add_up(one, half_ulp);
                    result &= (lower == 1.0) && (upper == std::nextafter(1.0, 2.0));
                    const int enclosing = std::fegetround();
                    {
                        Gecode::Float::Rounding inner;
                        result &= inner.median(1.0, 3.0) == 2.0;
                    }
                    result &= std::fegetround() == enclosing;
                    result &= interval_operations();
                    {
                        Gecode::Float::Rounding nested;
                        result &= interval_operations();
                    }
                    result &= std::fegetround() == enclosing;
                }
                if (std::fegetround() != mode) {
                    olog << "Rounding scope changed caller mode " << mode << '\n';
                    result = false;
                }
            }
            if (previous != -1)
                std::fesetround(previous);
            return result;
        }
    } rounding_scope;

    /// Directed rounding gives exact bounds for literal and runtime operands
    class DirectedRounding : public Base {
    private:
        /// Operation computed with directed rounding
        enum class Op { ADD, SUB, MUL, DIV, SQRT, INTEGER };
        /// Operation and operands (\a y is unused by unary operations)
        struct Case {
            Op op;
            Gecode::FloatNum x;
            Gecode::FloatNum y;
        };
        /// Lower and upper bound
        struct Bounds {
            Gecode::FloatNum down;
            Gecode::FloatNum up;
        };
        /// Round-to-nearest result and the sign-carrying rounding error
        struct Nearest {
            Gecode::FloatNum value;
            Gecode::FloatNum error;
        };
        /// Bounds observed through both interfaces in one caller mode
        struct Observed {
            Bounds rounding;
            Bounds float_val;
            bool mode_kept;
        };
        /// Cases whose two directed bounds differ
        static constexpr Case cases[] = {
            {Op::ADD, 0.1, 0.2},  {Op::ADD, -0.1, -0.2}, {Op::SUB, 0.1, 0.7},     {Op::SUB, -0.1, 0.2},
            {Op::MUL, 0.1, 0.3},  {Op::MUL, -0.1, 0.3},  {Op::DIV, 1.0, 3.0},     {Op::DIV, -1.0, 3.0},
            {Op::SQRT, 2.0, 0.0}, {Op::SQRT, 0.1, 0.0},  {Op::INTEGER, 2.5, 0.0}, {Op::INTEGER, -2.5, 0.0},
        };

        /// Return \a a + \a b and its exact error (TwoSum)
        static Nearest
        two_sum(Gecode::FloatNum a, Gecode::FloatNum b) {
            const Gecode::FloatNum s = a + b;
            const Gecode::FloatNum b_virtual = s - a;
            return {s, (a - (s - b_virtual)) + (b - b_virtual)};
        }
        /// Return the round-to-nearest result of \a c and its error
        static Nearest
        nearest(const Case& c) {
            switch (c.op) {
            case Op::ADD:
                return two_sum(c.x, c.y);
            case Op::SUB:
                return two_sum(c.x, -c.y);
            case Op::MUL: {
                const Gecode::FloatNum p = c.x * c.y;
                return {p, std::fma(c.x, c.y, -p)};
            }
            case Op::DIV: {
                const Gecode::FloatNum q = c.x / c.y;
                return {q, std::fma(-q, c.y, c.x) * (c.y < 0.0 ? -1.0 : 1.0)};
            }
            case Op::SQRT: {
                const Gecode::FloatNum s = std::sqrt(c.x);
                return {s, std::fma(-s, s, c.x)};
            }
            case Op::INTEGER:
                break;
            }
            return {c.x, 0.0};
        }
        /// Return the exact directed bounds of \a c (in round-to-nearest mode)
        static Bounds
        expected(const Case& c) {
            if (c.op == Op::INTEGER)
                return {std::floor(c.x), std::ceil(c.x)};
            const Gecode::FloatNum inf = std::numeric_limits<Gecode::FloatNum>::infinity();
            const Nearest n = nearest(c);
            return {n.error < 0.0 ? std::nextafter(n.value, -inf) : n.value,
                    n.error > 0.0 ? std::nextafter(n.value, inf) : n.value};
        }

        /// Return the bounds computed in one Float::Rounding scope
        template <Op op>
        forceinline static Bounds
        rounding_bounds(Gecode::FloatNum x, Gecode::FloatNum y) {
            Gecode::Float::Rounding r;
            if constexpr (op == Op::ADD)
                return {r.add_down(x, y), r.add_up(x, y)};
            else if constexpr (op == Op::SUB)
                return {r.sub_down(x, y), r.sub_up(x, y)};
            else if constexpr (op == Op::MUL)
                return {r.mul_down(x, y), r.mul_up(x, y)};
            else if constexpr (op == Op::DIV)
                return {r.div_down(x, y), r.div_up(x, y)};
            else if constexpr (op == Op::SQRT)
                return {r.sqrt_down(x), r.sqrt_up(x)};
            else
                return {r.int_down(x), r.int_up(x)};
        }
        /// Return the bounds of the FloatVal result
        template <Op op>
        forceinline static Bounds
        float_val_bounds(Gecode::FloatNum x, Gecode::FloatNum y) {
            Gecode::FloatVal v;
            if constexpr (op == Op::ADD)
                v = Gecode::FloatVal(x) + Gecode::FloatVal(y);
            else if constexpr (op == Op::SUB)
                v = Gecode::FloatVal(x) - Gecode::FloatVal(y);
            else if constexpr (op == Op::MUL)
                v = Gecode::FloatVal(x) * Gecode::FloatVal(y);
            else if constexpr (op == Op::DIV)
                v = Gecode::FloatVal(x) / Gecode::FloatVal(y);
            else
                v = sqrt(Gecode::FloatVal(x));
            return {v.min(), v.max()};
        }
        /// Observe both interfaces and whether they keep the caller's mode
        template <Op op>
        forceinline static Observed
        observe(Gecode::FloatNum x, Gecode::FloatNum y) {
            const int mode = std::fegetround();
            Observed o;
            o.rounding = rounding_bounds<op>(x, y);
            o.mode_kept = std::fegetround() == mode;
            // FloatVal has no integer rounding operation.
            if constexpr (op == Op::INTEGER)
                o.float_val = o.rounding;
            else
                o.float_val = float_val_bounds<op>(x, y);
            o.mode_kept &= std::fegetround() == mode;
            return o;
        }
        /// Observe case \a i with operands the compiler can fold
        template <std::size_t i>
        [[gnu::noinline]] static Observed
        observe_literal() {
            // Anything else in this function can stop Clang from folding.
            constexpr Case c = cases[i];
            return observe<c.op>(c.x, c.y);
        }
        /// Observe case \a i with runtime operands
        template <std::size_t i>
        [[gnu::noinline]] static Observed
        observe_volatile() {
            const volatile Gecode::FloatNum x = cases[i].x;
            const volatile Gecode::FloatNum y = cases[i].y;
            return observe<cases[i].op>(x, y);
        }

        /// Log \a b in hexadecimal floating-point notation
        static void
        log_bounds(const char* name, const Bounds& b) {
            olog << ' ' << name << " [" << std::hexfloat << b.down << ", " << b.up << std::defaultfloat << ']';
        }
        /// Test whether \a o matches \a e, and log a mismatch
        static bool
        matches(int i, const char* kind, int mode, const Bounds& e, const Observed& o) {
            const bool rounding_exact = (o.rounding.down == e.down) && (o.rounding.up == e.up);
            const bool float_val_exact = (o.float_val.down == e.down) && (o.float_val.up == e.up);
            if (o.mode_kept && rounding_exact && float_val_exact)
                return true;
            olog << "Case " << i << ' ' << kind << " in mode " << mode << ':';
            log_bounds("Rounding", o.rounding);
            log_bounds("FloatVal", o.float_val);
            log_bounds("expected", e);
            olog << (o.mode_kept ? "" : " mode changed") << '\n';
            return false;
        }
        /// Test case \a i under caller mode \a mode
        template <std::size_t i>
        static bool
        check_case(int mode) {
            std::fesetround(FE_TONEAREST);
            const Bounds e = expected(cases[i]);
            std::fesetround(mode);
            bool result = e.down < e.up;
            result &= matches(i, "literal", mode, e, observe_literal<i>());
            result &= matches(i, "volatile", mode, e, observe_volatile<i>());
            return result;
        }
        /// Test all cases under caller mode \a mode
        template <std::size_t... i>
        static bool
        check_cases(int mode, std::index_sequence<i...>) {
            return (check_case<i>(mode) & ...);
        }

    public:
        DirectedRounding() : Base("Float::Arithmetic::DirectedRounding", TestTags::check()) {}
        bool
        run() override {
            const int previous = std::fegetround();
            const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
            bool result = true;
            for (int mode : modes) {
                if (std::fesetround(mode) != 0)
                    continue;
                result &= check_cases(mode, std::make_index_sequence<std::size(cases)>());
            }
            if (previous != -1)
                std::fesetround(previous);
            return result;
        }
    } directed_rounding;

    /// Median queries preserve the caller's floating-point rounding mode
    class MedianRoundingMode : public Base {
    public:
        MedianRoundingMode() : Base("Float::Arithmetic::MedianRoundingMode", TestTags::check()) {}
        bool
        run() override {
            const int previous = std::fegetround();
            const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
            const Gecode::FloatNum limit = std::numeric_limits<Gecode::FloatNum>::max();
#if !defined(__FINITE_MATH_ONLY__) || !__FINITE_MATH_ONLY__
            const Gecode::FloatNum infinity = std::numeric_limits<Gecode::FloatNum>::infinity();
#endif
            const Gecode::FloatNum cases[][3] = {{1.0, 3.0, 2.0},           {-3.0, -1.0, -2.0},
                                                 {-limit, limit, 0.0},      {limit, limit, limit},
                                                 {-limit, -limit, -limit},
            // Finite-math builds do not support infinite inputs.
#if !defined(__FINITE_MATH_ONLY__) || !__FINITE_MATH_ONLY__
                                                 {-infinity, infinity, 0.0}
#endif
            };
            bool result = true;
            for (int mode : modes)
                for (const auto& c : cases) {
                    const Gecode::FloatVal value(c[0], c[1]);
                    if (std::fesetround(mode) != 0)
                        continue;
                    volatile Gecode::FloatNum direct;
                    {
                        Gecode::Float::Rounding r;
                        direct = r.median(c[0], c[1]);
                    }
                    if ((std::fegetround() != mode) || (direct != c[2])) {
                        olog << "Direct median changed its result or rounding mode " << mode << '\n';
                        result = false;
                    }
                    const volatile Gecode::FloatNum median = value.med();
                    if ((std::fegetround() != mode) || (median != c[2])) {
                        olog << "Median query changed its result or rounding mode " << mode << '\n';
                        result = false;
                    }
                }
            if (previous != -1)
                std::fesetround(previous);
            return result;
        }
    } median_rounding_mode;

    /// Regression test for outward positive even-root bounds
    class PositiveNRootBounds : public Base {
    protected:
        /// Space exercising the Pow and NthRoot propagators
        class RootSpace : public Gecode::Space {
        public:
            /// Variables used by the propagator
            Gecode::FloatVar x0, x1;
            /// Post an extreme Pow or NthRoot instance
            RootSpace(Gecode::FloatNum hi, Gecode::FloatNum witness, int n, bool use_pow)
                : x0(*this, 0.0, use_pow ? witness : hi), x1(*this, use_pow ? 0.0 : witness, use_pow ? hi : witness) {
                if (use_pow) {
                    Gecode::pow(*this, x0, n, x1);
                    Gecode::rel(*this, x0, Gecode::FRT_EQ, witness);
                } else {
                    Gecode::nroot(*this, x0, n, x1);
                }
            }
            /// Clone constructor
            RootSpace(RootSpace& s) : Gecode::Space(s) {
                x0.update(*this, s.x0);
                x1.update(*this, s.x1);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
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
                : source(*this, 0.0, hi), root(*this, 0.0, Gecode::Float::Limits::max) {
                Gecode::nroot(*this, source, n, root);
            }
            /// Clone constructor
            NRootBoundSpace(NRootBoundSpace& s) : Gecode::Space(s) {
                source.update(*this, s.source);
                root.update(*this, s.root);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
                return new NRootBoundSpace(*this);
            }
        };

        /// Check that a known feasible boundary value is not removed
        bool
        check(Gecode::FloatNum hi, Gecode::FloatNum witness, int n, int mode, bool use_pow) const {
            if (std::fesetround(mode) != 0)
                return true;
            RootSpace s(hi, witness, n, use_pow);
            if (s.status() != Gecode::SS_FAILED)
                return true;
            olog << "Root bound removed a solution for " << (use_pow ? "pow" : "nroot") << ", n=" << n
                 << ", mode=" << mode << '\n';
            return false;
        }
        /// Check that propagation returns a genuinely outward upper bound
        bool
        check_bound(Gecode::FloatNum hi, Gecode::FloatNum rejected, int n, int mode) const {
            if (std::fesetround(mode) != 0)
                return true;
            NRootBoundSpace s(hi, n);
            if ((s.status() != Gecode::SS_FAILED) && (s.root.max() > rejected))
                return true;
            olog << "NRoot upper bound was not outward for n=" << n << ", mode=" << mode << '\n';
            return false;
        }

    public:
        /// Create and register test
        PositiveNRootBounds() : Base("Float::Arithmetic::PositiveNRootBounds", TestTags::check()) {}
        /// Run test under every supported IEEE-754 rounding mode
        bool
        run() override {
            const int old_mode = std::fegetround();
            const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
            struct Case {
                Gecode::FloatNum hi;
                Gecode::FloatNum witness;
                int n;
            } cases[] = {{std::ldexp(static_cast<Gecode::FloatNum>(0x156e1fc2f8f359ULL), -1049),
                          std::ldexp(static_cast<Gecode::FloatNum>(0x14484bfeebc28cULL), -152), 10},
                         {std::numeric_limits<Gecode::FloatNum>::denorm_min(),
                          std::ldexp(static_cast<Gecode::FloatNum>(0x18406003b2ae42ULL), -160), 10},
                         {std::numeric_limits<Gecode::FloatNum>::max(),
                          std::ldexp(static_cast<Gecode::FloatNum>(0x1965fea53d6e0fULL), 118), 6}};
            bool result = true;
            const Gecode::FloatNum rounding_hi = std::ldexp(static_cast<Gecode::FloatNum>(0x1751def03d6f38ULL), 458);
            const Gecode::FloatNum rejected_upper = std::ldexp(static_cast<Gecode::FloatNum>(0x1350f4c43079ceULL), 203);
            for (int mode : modes) {
                for (auto& c : cases) {
                    result = check(c.hi, c.witness, c.n, mode, false) && result;
                    result = check(c.hi, c.witness, c.n, mode, true) && result;
                }
                result = check_bound(rounding_hi, rejected_upper, 2, mode) && result;
            }
            if (old_mode != -1)
                std::fesetround(old_mode);
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
            ZeroSpace(int n, bool direct) : x(*this, direct ? 0.0 : -1.0, direct ? 0.0 : 1.0), y(*this, -1.0, 2.0) {
                Gecode::pow(*this, x, n, y);
            }
            /// Clone constructor
            ZeroSpace(ZeroSpace& s) : Gecode::Space(s) {
                x.update(*this, s.x);
                y.update(*this, s.y);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
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
                : x(*this, positive ? 0.0 : -std::numeric_limits<Gecode::FloatNum>::denorm_min(),
                    positive ? std::numeric_limits<Gecode::FloatNum>::denorm_min() : -0.0),
                  y(*this, -1.0, 2.0) {
                Gecode::pow(*this, x, 0, y);
            }
            /// Clone constructor
            AdjacentZeroSpace(AdjacentZeroSpace& s) : Gecode::Space(s) {
                x.update(*this, s.x);
                y.update(*this, s.y);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
                return new AdjacentZeroSpace(*this);
            }
        };

        /// Space reproducing a Pow inverse-to-forward fixpoint
        class FixpointSpace : public Gecode::Space {
        public:
            /// Base and result variables
            Gecode::FloatVar x, y;
            /// Post the fixpoint regression instance
            FixpointSpace()
                : x(*this, -std::ldexp(static_cast<Gecode::FloatNum>(0x10000000000006ULL), -53),
                    -std::ldexp(static_cast<Gecode::FloatNum>(0x10000000000004ULL), -53)),
                  y(*this, std::ldexp(static_cast<Gecode::FloatNum>(0x1000000000000aULL), -54),
                    std::ldexp(static_cast<Gecode::FloatNum>(0x1000000000000cULL), -54)) {
                Gecode::pow(*this, x, 2, y);
            }
            /// Clone constructor
            FixpointSpace(FixpointSpace& s) : Gecode::Space(s) {
                x.update(*this, s.x);
                y.update(*this, s.y);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
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
                : x(*this, -1.0, 1.0), y(*this, direct ? 0.0 : -1.0, direct ? 0.0 : 1.0) {
                Gecode::pow(*this, x, n, y);
            }
            /// Clone constructor
            ZeroResultSpace(ZeroResultSpace& s) : Gecode::Space(s) {
                x.update(*this, s.x);
                y.update(*this, s.y);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
                return new ZeroResultSpace(*this);
            }
        };

        /// Test one exponent with direct and delayed zero assignment
        bool
        check_zero(int n) const {
            ZeroSpace direct(n, true);
            ZeroSpace delayed(n, false);
            if (delayed.status() == Gecode::SS_FAILED) {
                olog << "Pow wide base failed for n=" << n << '\n';
                return false;
            }
            ZeroSpace* clone = static_cast<ZeroSpace*>(delayed.clone());
            Gecode::rel(*clone, clone->x, Gecode::FRT_EQ, 0.0);
            if (n == 0) {
                const bool zero_rejected =
                    (direct.status() == Gecode::SS_FAILED) && (clone->status() == Gecode::SS_FAILED);
                delete clone;
                ZeroSpace* non_zero = static_cast<ZeroSpace*>(delayed.clone());
                Gecode::rel(*non_zero, non_zero->x, Gecode::FRT_EQ, 0.5);
                const bool non_zero_accepted = (non_zero->status() != Gecode::SS_FAILED) &&
                                               (non_zero->y.min() == 1.0) && (non_zero->y.max() == 1.0);
                delete non_zero;
                if (!zero_rejected || !non_zero_accepted)
                    olog << "Pow exponent-zero semantics failed" << '\n';
                return zero_rejected && non_zero_accepted;
            }
            const bool result = (direct.status() != Gecode::SS_FAILED) && (direct.y.min() == 0.0) &&
                                (direct.y.max() == 0.0) && (clone->status() != Gecode::SS_FAILED) &&
                                (clone->y.min() == 0.0) && (clone->y.max() == 0.0);
            delete clone;
            if (!result)
                olog << "Pow delayed zero failed for n=" << n << '\n';
            return result;
        }
        /// Test that a zero result forces a zero base
        bool
        check_zero_result(int n, bool direct) const {
            ZeroResultSpace initial(n, direct);
            if (initial.status() == Gecode::SS_FAILED) {
                olog << "Pow zero result failed for n=" << n << '\n';
                return false;
            }
            ZeroResultSpace* clone = static_cast<ZeroResultSpace*>(initial.clone());
            if (!direct)
                Gecode::rel(*clone, clone->y, Gecode::FRT_EQ, 0.0);
            const bool contracted =
                (clone->status() != Gecode::SS_FAILED) && (clone->x.min() == 0.0) && (clone->x.max() == 0.0);
            Gecode::rel(*clone, clone->x, Gecode::FRT_EQ, 1.0);
            const bool inconsistent_rejected = clone->status() == Gecode::SS_FAILED;
            delete clone;
            if (!contracted || !inconsistent_rejected)
                olog << "Pow zero result did not force zero base for n=" << n << (direct ? " (direct)" : " (delayed)")
                     << '\n';
            return contracted && inconsistent_rejected;
        }
        /// Test exponent zero on an adjacent interval containing zero
        bool
        check_adjacent_zero(bool positive) const {
            AdjacentZeroSpace initial(positive);
            // Adjacent intervals are assigned and cannot be narrowed further.
            // Keep the unresolved nonzero constraint through propagation and cloning.
            const bool actor_retained = (initial.status() != Gecode::SS_FAILED) && (initial.y.min() == 1.0) &&
                                        (initial.y.max() == 1.0) && (Gecode::PropagatorGroup::all.size(initial) > 0);

            AdjacentZeroSpace* clone = static_cast<AdjacentZeroSpace*>(initial.clone());
            const bool clone_retained = (clone->status() != Gecode::SS_FAILED) && (clone->y.min() == 1.0) &&
                                        (clone->y.max() == 1.0) && (Gecode::PropagatorGroup::all.size(*clone) > 0);
            delete clone;

            if (!actor_retained || !clone_retained)
                olog << "Pow adjacent exponent-zero semantics failed" << (positive ? " (positive)" : " (negative)")
                     << '\n';
            return actor_retained && clone_retained;
        }

    public:
        /// Create and register test
        PowConsistency() : Base("Float::Arithmetic::PowConsistency", TestTags::check()) {}
        /// Run zero and fixpoint regressions
        bool
        run() override {
            bool result = true;
            for (int n = 0; n <= 3; n++)
                result = check_zero(n) && result;
            for (int n = 1; n <= 3; n++) {
                result = check_zero_result(n, true) && result;
                result = check_zero_result(n, false) && result;
            }
            result = check_adjacent_zero(true) && result;
            result = check_adjacent_zero(false) && result;

            FixpointSpace once;
            if (once.status() == Gecode::SS_FAILED) {
                olog << "Pow fixpoint instance failed" << '\n';
                return false;
            }
            FixpointSpace* twice = static_cast<FixpointSpace*>(once.clone());
            Gecode::pow(*twice, twice->x, 2, twice->y);
            const bool fixpoint = (twice->status() != Gecode::SS_FAILED) && (once.x.min() == twice->x.min()) &&
                                  (once.x.max() == twice->x.max()) && (once.y.min() == twice->y.min()) &&
                                  (once.y.max() == twice->y.max());
            delete twice;
            if (!fixpoint)
                olog << "Pow posting did not reach a fixpoint" << '\n';
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
            MultSpace(Gecode::FloatNum xl, Gecode::FloatNum xu, Gecode::FloatNum yl, Gecode::FloatNum yu,
                      Gecode::FloatNum zl, Gecode::FloatNum zu)
                : x(*this, xl, xu), y(*this, yl, yu), z(*this, zl, zu) {
                Gecode::mult(*this, x, y, z);
            }
            /// Clone constructor
            MultSpace(MultSpace& s) : Gecode::Space(s) {
                x.update(*this, s.x);
                y.update(*this, s.y);
                z.update(*this, s.z);
            }
            /// Copy space during cloning
            Gecode::Space*
            copy() override {
                return new MultSpace(*this);
            }
        };

        /// Check one endpoint-zero sign and magnitude contraction
        bool
        check(Gecode::FloatNum xl, Gecode::FloatNum xu, Gecode::FloatNum yl, Gecode::FloatNum yu, Gecode::FloatNum zl,
              Gecode::FloatNum zu, Gecode::FloatNum expected_min, Gecode::FloatNum expected_max) const {
            MultSpace s(xl, xu, yl, yu, zl, zu);
            if ((s.status() == Gecode::SS_FAILED) || (s.x.min() < expected_min) || (s.x.max() > expected_max)) {
                olog << "Multiplication endpoint-zero contraction failed: x=" << s.x << ", y=" << s.y << ", z=" << s.z
                     << '\n';
                return false;
            }
            MultSpace* clone = static_cast<MultSpace*>(s.clone());
            Gecode::mult(*clone, clone->x, clone->y, clone->z);
            const bool fixpoint = (clone->status() != Gecode::SS_FAILED) && (clone->x.min() == s.x.min()) &&
                                  (clone->x.max() == s.x.max()) && (clone->y.min() == s.y.min()) &&
                                  (clone->y.max() == s.y.max()) && (clone->z.min() == s.z.min()) &&
                                  (clone->z.max() == s.z.max());
            delete clone;
            if (!fixpoint)
                olog << "Multiplication endpoint-zero case was not at a fixpoint" << '\n';
            return fixpoint;
        }
        /// Exercise supported points across endpoint-zero sign combinations
        bool
        fuzz() const {
            unsigned int state = 0x6d756c74U;
            for (unsigned int i = 0; i < 5000; i++) {
                state = state * 1664525U + 1013904223U;
                const Gecode::FloatNum am = 0.125 + static_cast<Gecode::FloatNum>((state >> 8) % 8000) / 512.0;
                state = state * 1664525U + 1013904223U;
                const Gecode::FloatNum bm = 0.125 + static_cast<Gecode::FloatNum>((state >> 8) % 8000) / 512.0;
                const Gecode::FloatNum a = (i & 1U) ? -am : am;
                const Gecode::FloatNum b = (i & 2U) ? -bm : bm;
                const Gecode::FloatNum p = a * b;
                const Gecode::FloatNum slack = 1.0 + am / 4.0;
                const Gecode::FloatNum xl = a - slack;
                const Gecode::FloatNum xu = a + slack;
                const Gecode::FloatNum yl = (b < 0.0) ? -(bm + 1.0) : 0.0;
                const Gecode::FloatNum yu = (b < 0.0) ? -0.0 : bm + 1.0;
                MultSpace s(xl, xu, yl, yu, p, p);
                if ((s.status() == Gecode::SS_FAILED) || (s.x.min() > a) || (s.x.max() < a) || (s.y.min() > b) ||
                    (s.y.max() < b) || (s.z.min() > p) || (s.z.max() < p)) {
                    olog << "Multiplication removed supported fuzz point " << i << ": (" << a << ", " << b << ", " << p
                         << ")" << '\n';
                    return false;
                }
            }
            return true;
        }

    public:
        /// Create and register test
        MultZeroEndpoint() : Base("Float::Arithmetic::MultZeroEndpoint", TestTags::check()) {}
        /// Run sign, symmetry, signed-zero, and zero-product cases
        bool
        run() override {
            bool result = true;
            result = check(-1.0, 1.0, 0.0, 1.0, 0.5, 1.0, 0.5, 1.0) && result;
            result = check(-1.0, 1.0, -1.0, -0.0, 0.5, 1.0, -1.0, -0.5) && result;
            result = check(-1.0, 1.0, 0.0, 1.0, -1.0, -0.5, -1.0, -0.5) && result;
            result = check(-1.0, 1.0, -1.0, -0.0, -1.0, -0.5, 0.5, 1.0) && result;

            // Swapping the factors must recover the same contraction.
            MultSpace swapped(0.0, 1.0, -1.0, 1.0, 0.5, 1.0);
            result = (swapped.status() != Gecode::SS_FAILED) && (swapped.y.min() >= 0.5) && result;

            // A zero product gives no sign information about the other factor.
            MultSpace zero_product(-1.0, 1.0, 0.0, 1.0, 0.0, 0.0);
            result = (zero_product.status() != Gecode::SS_FAILED) && (zero_product.x.min() == -1.0) &&
                     (zero_product.x.max() == 1.0) && result;
            MultSpace zero_factor(0.0, 0.0, -1.0, 1.0, 0.0, 0.0);
            result = (zero_factor.status() != Gecode::SS_FAILED) && (zero_factor.y.min() == -1.0) &&
                     (zero_factor.y.max() == 1.0) && result;
            if (!result)
                olog << "Multiplication zero or swapped regression failed" << '\n';
            return fuzz() && result;
        }
    } mult_zero_endpoint;

    /// %Test for multiplication constraint
    class MultXYZ : public Test {
    public:
        /// Create and register test
        MultXYZ(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XYZ::" + s, 3, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[1], x[2]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
        MultXYZSol(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Mult::XYZ::Sol::" + s, 3, d, st, EXTEND_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[1], x[2]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = x[0] * x[1];
            if (Gecode::Float::subset(d, dom)) {
                x.set(2, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[1], x[2]);
        }
    };

    /// %Test for multiplication constraint with shared variables
    class MultXXY : public Test {
    public:
        /// Create and register test
        MultXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XXY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[0], x[1]);
        }
    };

    /// %Test for multiplication constraint with shared variables when solution is ensured
    class MultXXYSol : public Test {
    public:
        /// Create and register test
        MultXXYSol(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XXY::Sol::" + s, 2, d, st, EXTEND_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[1]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = x[0] * x[0];
            if (Gecode::Float::subset(d, dom)) {
                x.set(1, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[0], x[1]);
        }
    };

    /// %Test for multiplication constraint with shared variables
    class MultXYX : public Test {
    public:
        /// Create and register test
        MultXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XYX::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[1], x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[1], x[0]);
        }
    };

    /// %Test for multiplication constraint with shared variables
    class MultXYY : public Test {
    public:
        /// Create and register test
        MultXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XYY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[1], x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[1], x[1]);
        }
    };

    /// %Test for multiplication constraint with shared variables
    class MultXXX : public Test {
    public:
        /// Create and register test
        MultXXX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Mult::XXX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::mult(home, x[0], x[0], x[0]);
        }
    };

    /// %Test for division constraint
    class Div : public Test {
    public:
        /// Create and register test
        Div(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Div::" + s, 3, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] / x[1], x[2]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
            : Test(TestTags::sweep(), "Arithmetic::Div::Sol::" + s, 3, d, st, EXTEND_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] / x[1], x[2]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = x[0] / x[1];
            if (Gecode::Float::subset(d, dom)) {
                x.set(2, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::div(home, x[0], x[1], x[2]);
        }
    };

    /// %Test for squaring constraint
    class SqrXY : public Test {
    public:
        /// Create and register test
        SqrXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Sqr::XY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
            : Test(TestTags::sweep(), "Arithmetic::Sqr::XY::Sol::" + s, 2, d, st, EXTEND_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[1]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = sqr(x[0]);
            if (Gecode::Float::subset(d, dom)) {
                x.set(1, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::sqr(home, x[0], x[1]);
        }
    };

    /// %Test for squaring constraint with shared variables
    class SqrXX : public Test {
    public:
        /// Create and register test
        SqrXX(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Sqr::XX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(x[0] * x[0], x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::sqr(home, x[0], x[0]);
        }
    };

    /// %Test for square root constraint
    class SqrtXY : public Test {
    public:
        /// Create and register test
        SqrtXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Sqrt::XY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
            case MT_FALSE:
                return MT_FALSE;
            case MT_MAYBE:
                return MT_MAYBE;
            default:
                return eq(sqrt(x[0]), x[1]);
            }
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
            : Test(TestTags::sweep(), "Arithmetic::Sqrt::XY::Sol::" + s, 2, d, st, EXTEND_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
            case MT_FALSE:
                return MT_FALSE;
            case MT_MAYBE:
                return MT_MAYBE;
            default:
                return eq(sqrt(x[0]), x[1]);
            }
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = sqrt(abs(x[0]));
            if (Gecode::Float::subset(d, dom)) {
                x.set(1, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::sqrt(home, x[0], x[1]);
        }
    };

    /// %Test for square root constraint with shared variables
    class SqrtXX : public Test {
    public:
        /// Create and register test
        SqrtXX(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Sqrt::XX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            switch (cmp(x[0], Gecode::FRT_GQ, 0.0)) {
            case MT_FALSE:
                return MT_FALSE;
            case MT_MAYBE:
                return MT_MAYBE;
            default:
                return eq(sqrt(x[0]), x[0]);
            }
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::sqrt(home, x[0], x[0]);
        }
    };

    /// %Test for pow  constraint
    class PowXY : public Test {
        unsigned int n;

    public:
        /// Create and register test
        PowXY(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Pow::N::" + str(_n) + "::XY::" + s, 2, d, st, COMPLETE_ASSIGNMENT,
                   false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(pow(x[0], n), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            if (flip())
                Gecode::pow(home, x[0], n, x[1]);
            else
                Gecode::rel(home, pow(x[0], n) == x[1]);
        }
    };

    /// %Test for pow  constraint where solution is ensured
    class PowXYSol : public Test {
        unsigned int n;

    public:
        /// Create and register test
        PowXYSol(TestTags tags, const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Pow::N::" + str(_n) + "::XY::Sol::" + s, 2, d, st, EXTEND_ASSIGNMENT, false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(pow(x[0], n), x[1]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            Gecode::FloatVal d = pow(x[0], n);
            if (Gecode::Float::subset(d, dom)) {
                x.set(1, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::pow(home, x[0], n, x[1]);
        }
    };

    /// %Test for pow  constraint with shared variables
    class PowXX : public Test {
        unsigned int n;

    public:
        /// Create and register test
        PowXX(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Pow::N::" + str(_n) + "::XX::" + s, 1, d, st, COMPLETE_ASSIGNMENT,
                   false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(pow(x[0], n), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::pow(home, x[0], n, x[0]);
        }
    };

    /// %Test for nroot  constraint
    class NRootXY : public Test {
        unsigned int n;

    public:
        /// Create and register test
        NRootXY(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::NRoot::N::" + str(_n) + "::XY::" + s, 2, d, st, COMPLETE_ASSIGNMENT,
                   false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            if ((n == 0) || (x[0].min() < 0.0))
                return MT_FALSE;
            return eq(nroot(x[0], n), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            if (flip())
                Gecode::nroot(home, x[0], n, x[1]);
            else
                Gecode::rel(home, nroot(x[0], n) == x[1]);
        }
    };

    /// %Test for nroot  constraint where solution is ensured
    class NRootXYSol : public Test {
        unsigned int n;

    public:
        /// Create and register test
        NRootXYSol(TestTags tags, const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::NRoot::N::" + str(_n) + "::XY::Sol::" + s, 2, d, st, EXTEND_ASSIGNMENT, false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            if ((n == 0) || (x[0].min() < 0.0))
                return MT_FALSE;
            return eq(nroot(x[0], n), x[1]);
        }
        /// Extend assignment \a x
        bool
        extend_assignment(Assignment& x) const override {
            if ((n == 0) || (x[0].min() < 0))
                return false;
            Gecode::FloatVal d = nroot(x[0], n);
            if (Gecode::Float::subset(d, dom)) {
                x.set(1, d);
                return true;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::nroot(home, x[0], n, x[1]);
        }
    };

    /// %Test for nroot  constraint with shared variables
    class NRootXX : public Test {
        unsigned int n;

    public:
        /// Create and register test
        NRootXX(const std::string& s, const Gecode::FloatVal& d, unsigned int _n, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::NRoot::N::" + str(_n) + "::XX::" + s, 1, d, st, COMPLETE_ASSIGNMENT,
                   false),
              n(_n) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            if ((n == 0) || (x[0].min() < 0))
                return MT_FALSE;
            return eq(nroot(x[0], n), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::nroot(home, x[0], n, x[0]);
        }
    };

    /// %Test for absolute value constraint
    class AbsXY : public Test {
    public:
        /// Create and register test
        AbsXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Abs::XY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(abs(x[0]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
        AbsXX(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Abs::XX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(abs(x[0]), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::abs(home, x[0], x[0]);
        }
    };

    /// %Test for binary minimum constraint
    class MinXYZ : public Test {
    public:
        /// Create and register test
        MinXYZ(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Min::Bin::XYZ::" + s, 3, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(x[0], x[1]), x[2]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            if (flip())
                Gecode::min(home, x[0], x[1], x[2]);
            else
                Gecode::rel(home, min(x[0], x[1]) == x[2]);
        }
    };

    /// %Test for binary minimum constraint with shared variables
    class MinXXY : public Test {
    public:
        /// Create and register test
        MinXXY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Min::Bin::XXY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(x[0], x[0]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::min(home, x[0], x[0], x[1]);
        }
    };

    /// %Test for binary minimum constraint with shared variables
    class MinXYX : public Test {
    public:
        /// Create and register test
        MinXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Min::Bin::XYX::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(x[0], x[1]), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::min(home, x[0], x[1], x[0]);
        }
    };

    /// %Test for binary minimum constraint with shared variables
    class MinXYY : public Test {
    public:
        /// Create and register test
        MinXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Min::Bin::XYY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(x[0], x[1]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::min(home, x[0], x[1], x[1]);
        }
    };

    /// %Test for binary minimum constraint with shared variables
    class MinXXX : public Test {
    public:
        /// Create and register test
        MinXXX(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Min::Bin::XXX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(x[0], x[0]), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::min(home, x[0], x[0], x[0]);
        }
    };

    /// %Test for binary maximum constraint
    class MaxXYZ : public Test {
    public:
        /// Create and register test
        MaxXYZ(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Max::Bin::XYZ::" + s, 3, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(x[0], x[1]), x[2]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
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
            : Test(TestTags::sweep(), "Arithmetic::Max::Bin::XXY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(x[0], x[0]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::max(home, x[0], x[0], x[1]);
        }
    };

    /// %Test for binary maximum constraint with shared variables
    class MaxXYX : public Test {
    public:
        /// Create and register test
        MaxXYX(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Max::Bin::XYX::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(x[0], x[1]), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::max(home, x[0], x[1], x[0]);
        }
    };

    /// %Test for binary maximum constraint with shared variables
    class MaxXYY : public Test {
    public:
        /// Create and register test
        MaxXYY(const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(TestTags::sweep(), "Arithmetic::Max::Bin::XYY::" + s, 2, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(x[0], x[1]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::max(home, x[0], x[1], x[1]);
        }
    };

    /// %Test for binary maximum constraint with shared variables
    class MaxXXX : public Test {
    public:
        /// Create and register test
        MaxXXX(TestTags tags, const std::string& s, const Gecode::FloatVal& d, Gecode::FloatNum st)
            : Test(tags, "Arithmetic::Max::Bin::XXX::" + s, 1, d, st, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(x[0], x[0]), x[0]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::max(home, x[0], x[0], x[0]);
        }
    };

    /// %Test for n-ary minimmum constraint
    class MinNary : public Test {
    public:
        /// Create and register test
        MinNary() : Test(TestTags::sweep(), "Arithmetic::Min::Nary", 4, -4, 4, 0.5, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(min(x[0], x[1]), x[2]), x[3]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::FloatVarArgs m(3);
            m[0] = x[0];
            m[1] = x[1];
            m[2] = x[2];
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
        MinNaryShared()
            : Test(TestTags::sweep(), "Arithmetic::Min::Nary::Shared", 3, -4, 4, 0.5, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(min(min(x[0], x[1]), x[2]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::FloatVarArgs m(3);
            m[0] = x[0];
            m[1] = x[1];
            m[2] = x[2];
            Gecode::min(home, m, x[1]);
        }
    };

    /// %Test for n-ary maximum constraint
    class MaxNary : public Test {
    public:
        /// Create and register test
        MaxNary() : Test(TestTags::sweep(), "Arithmetic::Max::Nary", 4, -4, 4, 0.5, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(max(x[0], x[1]), x[2]), x[3]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::FloatVarArgs m(3);
            m[0] = x[0];
            m[1] = x[1];
            m[2] = x[2];
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
        MaxNaryShared()
            : Test(TestTags::sweep(), "Arithmetic::Max::Nary::Shared", 3, -4, 4, 0.5, COMPLETE_ASSIGNMENT, false) {}
        /// %Test whether \a x is solution
        MaybeType
        solution(const Assignment& x) const override {
            return eq(max(max(x[0], x[1]), x[2]), x[1]);
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            Gecode::FloatVarArgs m(3);
            m[0] = x[0];
            m[1] = x[1];
            m[2] = x[2];
            Gecode::max(home, m, x[1]);
        }
    };

    const Gecode::FloatNum step = 0.15;
    Gecode::FloatVal a(-8, 5);
    Gecode::FloatVal b(9, 12);
    Gecode::FloatVal c(-8, 8);

    MultXXY mult_xxy_a("A", a, step);
    MultXXY mult_xxy_b("B", b, step);
    MultXXY mult_xxy_c("C", c, step);

    MultXXYSol mult_xxy_sol_a("A", a, step);
    MultXXYSol mult_xxy_sol_b("B", b, step);
    MultXXYSol mult_xxy_sol_c("C", c, step);

    MultXYX mult_xyx_a("A", a, step);
    MultXYX mult_xyx_b("B", b, step);
    MultXYX mult_xyx_c("C", c, step);

    MultXYY mult_xyy_a("A", a, step);
    MultXYY mult_xyy_b("B", b, step);
    MultXYY mult_xyy_c("C", c, step);

    MultXXX mult_xxx_a("A", a, step);
    MultXXX mult_xxx_b("B", b, step);
    MultXXX mult_xxx_c("C", c, step);

    MultXYZ mult_xyz_a("A", a, step);
    MultXYZ mult_xyz_b("B", b, step);
    MultXYZ mult_xyz_c("C", c, step);

    MultXYZSol mult_xyz_sol_a(TestTags::sweep(), "A", a, step);
    MultXYZSol mult_xyz_sol_b(TestTags::sweep(), "B", b, step);
    MultXYZSol mult_xyz_sol_c(TestTags::check(), "C", c, step);

    Div div_a(TestTags::standard(), "A", a, step);
    Div div_b(TestTags::sweep(), "B", b, step);
    Div div_c(TestTags::sweep(), "C", c, step);

    DivSol div_sol_a("A", a, step);
    DivSol div_sol_b("B", b, step);
    DivSol div_sol_c("C", c, step);

    SqrXY sqr_xy_a("A", a, step);
    SqrXY sqr_xy_b("B", b, step);
    SqrXY sqr_xy_c("C", c, step);

    SqrXYSol sqr_xy_sol_a("A", a, step);
    SqrXYSol sqr_xy_sol_b("B", b, step);
    SqrXYSol sqr_xy_sol_c("C", c, step);

    SqrXX sqr_xx_a(TestTags::standard(), "A", a, step);
    SqrXX sqr_xx_b(TestTags::sweep(), "B", b, step);
    SqrXX sqr_xx_c(TestTags::sweep(), "C", c, step);

    SqrtXY sqrt_xy_a("A", a, step);
    SqrtXY sqrt_xy_b("B", b, step);
    SqrtXY sqrt_xy_c("C", c, step);

    SqrtXYSol sqrt_xy_sol_a("A", a, step);
    SqrtXYSol sqrt_xy_sol_b("B", b, step);
    SqrtXYSol sqrt_xy_sol_c("C", c, step);

    SqrtXX sqrt_xx_a(TestTags::standard(), "A", a, step);
    SqrtXX sqrt_xx_b(TestTags::sweep(), "B", b, step);
    SqrtXX sqrt_xx_c(TestTags::sweep(), "C", c, step);

    PowXY pow_xy_a_1("A", a, 2, step);
    PowXY pow_xy_b_1("B", b, 2, step);
    PowXY pow_xy_c_1("C", c, 2, step);

    PowXYSol pow_xy_sol_a_1(TestTags::sweep(), "A", a, 2, step);
    PowXYSol pow_xy_sol_b_1(TestTags::sweep(), "B", b, 2, step);
    PowXYSol pow_xy_sol_c_1(TestTags::check(), "C", c, 2, step);

    PowXX pow_xx_a_1("A", a, 2, step);
    PowXX pow_xx_b_1("B", b, 2, step);
    PowXX pow_xx_c_1("C", c, 2, step);

    PowXY pow_xy_a_2("A", a, 3, step);
    PowXY pow_xy_b_2("B", b, 3, step);
    PowXY pow_xy_c_2("C", c, 3, step);

    PowXYSol pow_xy_sol_a_2(TestTags::sweep(), "A", a, 3, step);
    PowXYSol pow_xy_sol_b_2(TestTags::sweep(), "B", b, 3, step);
    PowXYSol pow_xy_sol_c_2(TestTags::sweep(), "C", c, 3, step);

    PowXX pow_xx_a_2("A", a, 3, step);
    PowXX pow_xx_b_2("B", b, 3, step);
    PowXX pow_xx_c_2("C", c, 3, step);

    PowXY pow_xy_a_3("A", a, 0, step);
    PowXY pow_xy_b_3("B", b, 0, step);
    PowXY pow_xy_c_3("C", c, 0, step);

    PowXYSol pow_xy_sol_a_3(TestTags::sweep(), "A", a, 0, step);
    PowXYSol pow_xy_sol_b_3(TestTags::sweep(), "B", b, 0, step);
    PowXYSol pow_xy_sol_c_3(TestTags::sweep(), "C", c, 0, step);

    PowXX pow_xx_a_3("A", a, 0, step);
    PowXX pow_xx_b_3("B", b, 0, step);
    PowXX pow_xx_c_3("C", c, 0, step);

    NRootXY nroot_xy_a_1("A", a, 2, step);
    NRootXY nroot_xy_b_1("B", b, 2, step);
    NRootXY nroot_xy_c_1("C", c, 2, step);

    NRootXYSol nroot_xy_sol_a_1(TestTags::sweep(), "A", a, 2, step);
    NRootXYSol nroot_xy_sol_b_1(TestTags::sweep(), "B", b, 2, step);
    NRootXYSol nroot_xy_sol_c_1(TestTags::check(), "C", c, 2, step);

    NRootXX nroot_xx_a_1("A", a, 2, step);
    NRootXX nroot_xx_b_1("B", b, 2, step);
    NRootXX nroot_xx_c_1("C", c, 2, step);

    NRootXY nroot_xy_a_2("A", a, 3, step);
    NRootXY nroot_xy_b_2("B", b, 3, step);
    NRootXY nroot_xy_c_2("C", c, 3, step);

    NRootXYSol nroot_xy_sol_a_2(TestTags::sweep(), "A", a, 3, step);
    NRootXYSol nroot_xy_sol_b_2(TestTags::sweep(), "B", b, 3, step);
    NRootXYSol nroot_xy_sol_c_2(TestTags::sweep(), "C", c, 3, step);

    NRootXX nroot_xx_a_2("A", a, 3, step);
    NRootXX nroot_xx_b_2("B", b, 3, step);
    NRootXX nroot_xx_c_2("C", c, 3, step);

    NRootXY nroot_xy_a_3("A", a, 0, step);
    NRootXY nroot_xy_b_3("B", b, 0, step);
    NRootXY nroot_xy_c_3("C", c, 0, step);

    NRootXYSol nroot_xy_sol_a_3(TestTags::sweep(), "A", a, 0, step);
    NRootXYSol nroot_xy_sol_b_3(TestTags::sweep(), "B", b, 0, step);
    NRootXYSol nroot_xy_sol_c_3(TestTags::sweep(), "C", c, 0, step);

    NRootXX nroot_xx_a_3("A", a, 0, step);
    NRootXX nroot_xx_b_3("B", b, 0, step);
    NRootXX nroot_xx_c_3("C", c, 0, step);

    AbsXY abs_xy_a("A", a, step);
    AbsXY abs_xy_b("B", b, step);
    AbsXY abs_xy_c("C", c, step);

    AbsXX abs_xx_a(TestTags::standard(), "A", a, step);
    AbsXX abs_xx_b(TestTags::sweep(), "B", b, step);
    AbsXX abs_xx_c(TestTags::sweep(), "C", c, step);

    MinXYZ min_xyz_normal(TestTags::standard(), "Normal", Gecode::FloatVal(-1, 1), 1.0);

    MinXYZ min_xyz_a(TestTags::sweep(), "A", a, step);
    MinXYZ min_xyz_b(TestTags::sweep(), "B", b, step);
    MinXYZ min_xyz_c(TestTags::sweep(), "C", c, step);

    MinXXY min_xxy_a("A", a, step);
    MinXXY min_xxy_b("B", b, step);
    MinXXY min_xxy_c("C", c, step);

    MinXYX min_xyx_a("A", a, step);
    MinXYX min_xyx_b("B", b, step);
    MinXYX min_xyx_c("C", c, step);

    MinXYY min_xyy_a("A", a, step);
    MinXYY min_xyy_b("B", b, step);
    MinXYY min_xyy_c("C", c, step);

    MinXXX min_xxx_a(TestTags::standard(), "A", a, step);
    MinXXX min_xxx_b(TestTags::sweep(), "B", b, step);
    MinXXX min_xxx_c(TestTags::sweep(), "C", c, step);

    MaxXYZ max_xyz_normal(TestTags::standard(), "Normal", Gecode::FloatVal(-1, 1), 1.0);

    MaxXYZ max_xyz_a(TestTags::sweep(), "A", a, step);
    MaxXYZ max_xyz_b(TestTags::sweep(), "B", b, step);
    MaxXYZ max_xyz_c(TestTags::sweep(), "C", c, step);

    MaxXXY max_xxy_a("A", a, step);
    MaxXXY max_xxy_b("B", b, step);
    MaxXXY max_xxy_c("C", c, step);

    MaxXYX max_xyx_a("A", a, step);
    MaxXYX max_xyx_b("B", b, step);
    MaxXYX max_xyx_c("C", c, step);

    MaxXYY max_xyy_a("A", a, step);
    MaxXYY max_xyy_b("B", b, step);
    MaxXYY max_xyy_c("C", c, step);

    MaxXXX max_xxx_a(TestTags::standard(), "A", a, step);
    MaxXXX max_xxx_b(TestTags::sweep(), "B", b, step);
    MaxXXX max_xxx_c(TestTags::sweep(), "C", c, step);

    MinNary min_nary;
    MinNaryShared min_s_nary;
    MaxNary max_nary;
    MaxNaryShared max_s_nary;

    /// Space without variables for minimum and maximum over no variables
    class EmptyFixture : public Gecode::Space {
    public:
        /// Create fixture
        EmptyFixture() = default;
        /// Constructor for cloning \a fixture
        // NOLINTNEXTLINE(modernize-use-equals-default)
        EmptyFixture(EmptyFixture& fixture) : Gecode::Space(fixture) {}
        /// Copy during cloning
        Gecode::Space*
        copy() override {
            return new EmptyFixture(*this);
        }
    };

    /// %Test that minimum and maximum expressions over no variables throw at posting
    class MinMaxEmpty : public Base {
    public:
        /// Create and register test
        MinMaxEmpty() : Base("Float::Arithmetic::MinMax::Empty") {}
        /// Test whether posting \a e throws TooFewArguments
        static bool
        throws_too_few(const Gecode::LinFloatExpr& e) {
            EmptyFixture home;
            try {
                (void)Gecode::expr(home, e);
            } catch (Gecode::Float::TooFewArguments&) {
                return true;
            }
            return false;
        }
        /// Perform test
        bool
        run() override {
            Gecode::FloatVarArgs x;
            return throws_too_few(Gecode::min(x)) && throws_too_few(Gecode::max(x));
        }
    };
    MinMaxEmpty min_max_empty;
    //@}

} // namespace Test::Float::Arithmetic
// STATISTICS: test-float
