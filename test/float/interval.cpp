/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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

#include <cfenv>
#include <cmath>
#include <ios>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace Test { namespace Float {

    /// %Tests for the interval type behind float values
    namespace IntervalCore {

        using Gecode::FloatNum;
        using Gecode::Float::Interval;

        static_assert(std::is_trivially_copyable_v<Interval>);

        /**
         * \defgroup TaskTestFloatInterval Interval arithmetic
         * \ingroup TaskTestFloat
         */
        //@{

        /// Caller modes under which the interval operations are tested
        const int caller_modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};

        /// Test whether \a x has the bounds \a l and \a u, and log a mismatch
        bool
        has_bounds(const char* name, const Interval& x, FloatNum l, FloatNum u) {
            if ((x.lower() == l) && (x.upper() == u))
                return true;
            olog << name << ": [" << std::hexfloat << x.lower() << ", " << x.upper() << "], expected [" << l << ", "
                 << u << ']' << std::defaultfloat << '\n';
            return false;
        }

        /// Arithmetic rounds each bound outward and keeps the caller's mode
        class OutwardRounding : public Base {
        private:
            /// Test whether \a x has the bounds \a l and \a u in caller mode \a mode
            static bool
            check(int mode, const char* name, const Interval& x, FloatNum l, FloatNum u) {
                bool result = has_bounds(name, x, l, u);
                if (std::fegetround() != mode) {
                    olog << name << " changed caller mode " << mode << '\n';
                    result = false;
                }
                return result;
            }

        public:
            OutwardRounding() : Base("Float::Interval::OutwardRounding", TestTags::check()) {}
            bool
            run() override {
                const int previous = std::fegetround();
                bool result = true;
                // Each expected bound is a floating-point neighbour of the
                // exact result, computed with rational arithmetic.
                for (int mode : caller_modes) {
                    if (std::fesetround(mode) != 0)
                        continue;
                    result &= check(mode, "[0.1,0.1]+[0.2,0.2]", Interval(0.1) + Interval(0.2), 0x1.3333333333333p-2,
                                    0x1.3333333333334p-2);
                    result &=
                        check(mode, "0.1-[0.7,0.7]", 0.1 - Interval(0.7), -0x1.3333333333333p-1, -0x1.3333333333332p-1);
                    result &= check(mode, "[-0.1,0.3]*[0.3,0.7]", Interval(-0.1, 0.3) * Interval(0.3, 0.7),
                                    -0x1.1eb851eb851ecp-4, 0x1.ae147ae147ae1p-3);
                    result &= check(mode, "[1,1]/[3,3]", Interval(1.0) / Interval(3.0), 0x1.5555555555555p-2,
                                    0x1.5555555555556p-2);
                    result &= check(mode, "[-1,2]/[3,7]", Interval(-1.0, 2.0) / Interval(3.0, 7.0),
                                    -0x1.5555555555556p-2, 0x1.5555555555556p-1);
                    result &=
                        check(mode, "sqrt [2,2]", sqrt(Interval(2.0)), 0x1.6a09e667f3bccp+0, 0x1.6a09e667f3bcdp+0);
                    result &=
                        check(mode, "sqrt [0.1,0.1]", sqrt(Interval(0.1)), 0x1.43d136248490fp-2, 0x1.43d1362484910p-2);
                }
                if (previous != -1)
                    std::fesetround(previous);
                return result;
            }
        } outward_rounding;

        /// Singletons stay exact, and empty intervals are never created
        class SingletonAndEmpty : public Base {
        private:
            /// Test whether \a operation throws std::runtime_error, and log otherwise
            template <class Operation>
            static bool
            throws_empty(const char* name, Operation operation) {
                try {
                    (void)operation();
                } catch (const std::runtime_error&) {
                    return true;
                }
                olog << name << " did not throw\n";
                return false;
            }

        public:
            SingletonAndEmpty() : Base("Float::Interval::SingletonAndEmpty", TestTags::standard()) {}
            bool
            run() override {
                const FloatNum nan = std::numeric_limits<FloatNum>::quiet_NaN();
                bool result = true;

                const Interval zero;
                result &=
                    has_bounds("default", zero, 0.0, 0.0) && !std::signbit(zero.lower()) && !std::signbit(zero.upper());
                result &= has_bounds("[0.1,0.1]", Interval(0.1), 0.1, 0.1);
                result &= singleton(Interval(0.1)) && (width(Interval(0.1)) == 0.0);
                result &= has_bounds("[2,2]*[3,3]", Interval(2.0) * Interval(3.0), 6.0, 6.0);
                result &= has_bounds("sqrt [4,4]", sqrt(Interval(4.0)), 2.0, 2.0);
                const int min_exponent = std::numeric_limits<int>::min();
                result &= has_bounds("pow([1,1],INT_MIN)", pow(Interval(1.0), min_exponent), 1.0, 1.0);
                result &= has_bounds("pow([-1,-1],INT_MIN)", pow(Interval(-1.0), min_exponent), 1.0, 1.0);
                const Interval not_a_number(nan);
                result &= std::isnan(not_a_number.lower()) && std::isnan(not_a_number.upper());

                result &= throws_empty("Interval(2,1)", [] { return Interval(2.0, 1.0); });
                result &= throws_empty("Interval(NaN,1)", [&] { return Interval(nan, 1.0); });
                result &= throws_empty("Interval(1,NaN)", [&] { return Interval(1.0, nan); });
                Interval assigned(1.0, 2.0);
                result &= throws_empty("assign(2,1)", [&] {
                    assigned.assign(2.0, 1.0);
                    return assigned;
                });
                result &= has_bounds("after assign(2,1)", assigned, 1.0, 2.0);

                const Interval x(1.0, 2.0);
                result &= throws_empty("[1,2]/[0,0]", [&] { return x / Interval(); });
                result &= throws_empty("[1,2]/0", [&] { return x / 0.0; });
                result &= throws_empty("sqrt [-2,-1]", [] { return sqrt(Interval(-2.0, -1.0)); });
                result &= throws_empty("pow([0,0],0)", [] { return pow(Interval(), 0); });
                result &= throws_empty("pow([0,0],-1)", [] { return pow(Interval(), -1); });
                result &= throws_empty("nth_root([-2,-1],2)", [] { return nth_root(Interval(-2.0, -1.0), 2); });
                result &= throws_empty("intersect([1,2],[3,4])", [&] { return intersect(x, Interval(3.0, 4.0)); });
#ifdef GECODE_HAS_MPFR
                result &= throws_empty("log [-1,0]", [] { return log(Interval(-1.0, 0.0)); });
                result &= throws_empty("asin [2,3]", [] { return asin(Interval(2.0, 3.0)); });
                result &= throws_empty("acosh [0,0.5]", [] { return acosh(Interval(0.0, 0.5)); });
#endif
                return result;
            }
        } singleton_and_empty;

        /// Comparisons hold only when they hold for all members
        class Comparison : public Base {
        private:
            /// Truth values of the relations between two intervals
            struct IntervalRelations {
                bool less;
                bool less_equal;
                bool greater;
                bool greater_equal;
                bool equal;
                bool unequal;
                bool operator==(const IntervalRelations&) const = default;
            };
            /// Truth values of the relations between an interval and a number
            struct NumberRelations {
                bool less;
                bool less_equal;
                bool greater;
                bool greater_equal;
                bool unequal;
                bool operator==(const NumberRelations&) const = default;
            };

            /// Test the relations between \a x and \a y against \a expected
            static bool
            check(const Interval& x, const Interval& y, const IntervalRelations& expected) {
                const IntervalRelations actual = {certainly_less(x, y),
                                                  certainly_less_equal(x, y),
                                                  certainly_greater(x, y),
                                                  certainly_greater_equal(x, y),
                                                  equal(x, y),
                                                  certainly_unequal(x, y)};
                if (actual == expected)
                    return true;
                olog << "Relations of [" << x.lower() << ',' << x.upper() << "] and [" << y.lower() << ',' << y.upper()
                     << "] differ\n";
                return false;
            }
            /// Test the relations between \a x and \a n against \a expected
            static bool
            check(const Interval& x, FloatNum n, const NumberRelations& expected) {
                const NumberRelations actual = {certainly_less(x, n), certainly_less_equal(x, n),
                                                certainly_greater(x, n), certainly_greater_equal(x, n),
                                                certainly_unequal(x, n)};
                if (actual == expected)
                    return true;
                olog << "Relations of [" << x.lower() << ',' << x.upper() << "] and " << n << " differ\n";
                return false;
            }

        public:
            Comparison() : Base("Float::Interval::Comparison", TestTags::standard()) {}
            bool
            run() override {
                const bool t = true;
                const bool f = false;
                const FloatNum one_up = std::nextafter(1.0, 2.0);
                const Interval x(1.0, 2.0);
                const Interval one(1.0);
                const Interval tight(1.0, one_up);
                bool result = true;
                // Columns: < <= > >= == !=
                result &= check(x, Interval(3.0, 4.0), {t, t, f, f, f, t});
                result &= check(x, Interval(2.0, 3.0), {f, t, f, f, f, f});
                result &= check(x, Interval(1.5, 3.0), {f, f, f, f, f, f});
                result &= check(x, Interval(1.0, 2.0), {f, f, f, f, t, f});
                result &= check(x, Interval(1.2, 1.8), {f, f, f, f, f, f});
                result &= check(x, Interval(0.0, 1.0), {f, f, f, t, f, f});
                result &= check(x, Interval(-1.0, 0.0), {f, f, t, t, f, t});
                result &= check(one, one, {f, t, f, t, t, f});
                result &= check(one, Interval(2.0), {t, t, f, f, f, t});
                result &= check(one, x, {f, t, f, f, f, f});
                result &= check(tight, tight, {f, f, f, f, t, f});
                // Columns: < <= > >= !=
                result &= check(x, 0.5, {f, f, t, t, t});
                result &= check(x, 1.0, {f, f, f, t, f});
                result &= check(x, 1.5, {f, f, f, f, f});
                result &= check(x, 2.0, {f, t, f, f, f});
                result &= check(x, 3.0, {t, t, f, f, t});
                result &= check(one, 1.0, {f, t, f, t, f});
                result &= check(tight, 1.0, {f, f, f, t, f});
                result &= check(tight, one_up, {f, t, f, f, f});
                return result;
            }
        } comparison;

        //@}

    } // namespace IntervalCore

}} // namespace Test::Float

// STATISTICS: test-float
