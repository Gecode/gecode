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

/*
 * Temporary differential test: FloatVal, and Float::Interval where FloatVal
 * does not forward to it, against the vendored Boost interval that FloatVal
 * used before. It is removed together with the Boost copy.
 *
 */

#include "test/float.hh"

#include <gecode/third-party/boost/numeric/interval.hpp>

#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <exception>
#include <ios>
#include <limits>
#include <sstream>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace Test { namespace Float { namespace {

    using Gecode::FloatNum;
    using Gecode::FloatVal;
    using Gecode::Float::Interval;

    /// Boost's rounding interface over Gecode's rounding operations and mode control
    class ReferenceRounding : public Gecode::Float::RoundingBase {
    public:
        /// Rounding mode, as Boost's save_state stores it
        typedef Mode rounding_mode;
        /// Store the current rounding mode in \a m
        static void
        get_rounding_mode(rounding_mode& m) {
            m = current_mode();
        }
        /// Restore rounding mode \a m
        static void
        set_rounding_mode(rounding_mode m) {
            restore_mode(m);
        }
        using RoundingBase::init;
    };

    /// FloatVal's former implementation type, which gives the expected results
    using Reference =
        gecode_boost::numeric::interval<FloatNum,
                                        gecode_boost::numeric::interval_lib::policies<
                                            gecode_boost::numeric::interval_lib::save_state<ReferenceRounding>,
                                            gecode_boost::numeric::interval_lib::checking_strict<FloatNum>>>;

    namespace reference = gecode_boost::numeric;

    /// Bounds of an operand interval
    struct Bounds {
        FloatNum lower;
        FloatNum upper;
    };

    /// Return the operand numbers: infinities, extreme normals,
    /// denormals, signed zeros, inexact decimals, a 1-ulp neighbour, and NaN
    std::vector<FloatNum>
    grid_numbers(void) {
        const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
        const FloatNum max_normal = std::numeric_limits<FloatNum>::max();
        const FloatNum min_normal = std::numeric_limits<FloatNum>::min();
        const FloatNum denorm = std::numeric_limits<FloatNum>::denorm_min();
        const FloatNum nan = std::numeric_limits<FloatNum>::quiet_NaN();
        return {-inf,   -max_normal, -1e5,    -3.0, -1.0,
                -0.1,   -min_normal, -denorm, -0.0, 0.0,
                denorm, min_normal,  0.1,     1.0,  std::nextafter(1.0, 2.0),
                2.0,    3.0,         7.0,     1e5,  max_normal,
                inf,    nan};
    }

    /// Return the operand intervals: every non-NaN singleton, ranges
    /// straddling or touching zero, infinite and extreme bounds, tight
    /// intervals, \f$[0,-0]\f$, and the NaN singleton
    std::vector<Bounds>
    grid_intervals(void) {
        const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
        const FloatNum max_normal = std::numeric_limits<FloatNum>::max();
        const FloatNum min_normal = std::numeric_limits<FloatNum>::min();
        const FloatNum denorm = std::numeric_limits<FloatNum>::denorm_min();
        const FloatNum nan = std::numeric_limits<FloatNum>::quiet_NaN();
        const FloatNum one_up = std::nextafter(1.0, 2.0);
        std::vector<Bounds> v;
        for (FloatNum n : grid_numbers())
            if (!std::isnan(n))
                v.push_back({n, n});
        const std::vector<Bounds> ranges = {{-inf, inf},
                                            {-inf, -1.0},
                                            {-inf, 0.0},
                                            {-inf, 0.1},
                                            {0.0, inf},
                                            {1.0, inf},
                                            {-max_normal, max_normal},
                                            {-1.0, 1.0},
                                            {-0.0, 0.0},
                                            {0.0, -0.0},
                                            {-1.0, 0.0},
                                            {0.0, 1.0},
                                            {-0.1, 0.1},
                                            {0.1, 0.3},
                                            {1.0, one_up},
                                            {1.0, 2.0},
                                            {2.0, 3.0},
                                            {-3.0, -2.0},
                                            {-3.0, 7.0},
                                            {-0.1, -0.0},
                                            {0.0, 0.1},
                                            {denorm, min_normal},
                                            {-denorm, denorm},
                                            {1e5, max_normal},
                                            {-max_normal, -1e5},
                                            {0.1, 7.0},
                                            {3.0, 1e5},
                                            {-1.0, -0.0},
                                            {0.5, 1.0},
                                            {-7.0, -0.1}};
        v.insert(v.end(), ranges.begin(), ranges.end());
        v.push_back({nan, nan});
        return v;
    }

    /// What an operation produced
    enum class Kind { BOUNDS, TRUTH, THROW };

    /// Observed result of one operation in one caller mode
    struct Outcome {
        Kind kind = Kind::BOUNDS;
        /// Bit patterns of the bounds, as bound_bits() gives them, for Kind::BOUNDS
        std::uint64_t lower = 0;
        std::uint64_t upper = 0;
        /// Truth value, for Kind::TRUTH
        bool truth = false;
        /// Dynamic type of the exception, for Kind::THROW
        std::type_index exception = typeid(void);
        /// Whether the caller's rounding mode was unchanged afterwards
        bool mode_kept = true;
        bool operator==(const Outcome&) const = default;
    };

    /**
     * \brief Return the bit pattern of bound \a b, without the sign of a NaN
     *
     * IEEE 754 leaves the sign of a NaN result unspecified. On x86-64 it
     * depends on the operand order that the compiler picks, so GCC and Clang
     * builds of the same code differ in it. All other bits of a NaN, its
     * payload and its quiet bit, are compared.
     */
    std::uint64_t
    bound_bits(FloatNum b) {
        return std::bit_cast<std::uint64_t>(std::isnan(b) ? std::fabs(b) : b);
    }
    /// Return the outcome for the bounds \a l and \a u
    Outcome
    bounds_outcome(FloatNum l, FloatNum u) {
        Outcome o;
        o.lower = bound_bits(l);
        o.upper = bound_bits(u);
        return o;
    }
    /// Return the outcome for value \a x
    Outcome
    outcome_of(const FloatVal& x) {
        return bounds_outcome(x.min(), x.max());
    }
    /// Return the outcome for interval \a x
    Outcome
    outcome_of(const Interval& x) {
        return bounds_outcome(x.lower(), x.upper());
    }
    /// Return the outcome for interval \a x
    Outcome
    outcome_of(const Reference& x) {
        return bounds_outcome(x.lower(), x.upper());
    }
    /// Return the outcome for number \a n, as the bounds \f$[n,n]\f$
    Outcome
    outcome_of(FloatNum n) {
        return bounds_outcome(n, n);
    }
    /// Return the outcome for truth value \a b
    Outcome
    outcome_of(bool b) {
        Outcome o;
        o.kind = Kind::TRUTH;
        o.truth = b;
        return o;
    }

    /// Return the outcome of \a operation in caller mode \a mode, and reset the mode
    template <class Operation>
    Outcome
    observe(int mode, Operation operation) {
        std::fesetround(mode);
        Outcome o;
        try {
            o = outcome_of(operation());
        } catch (const std::exception& e) {
            o.kind = Kind::THROW;
            o.exception = typeid(e);
        } catch (...) {
            o.kind = Kind::THROW;
        }
        o.mode_kept = std::fegetround() == mode;
        std::fesetround(FE_TONEAREST);
        return o;
    }

    /// Return the result of the Boost comparison \a comparison, with an uncertain result as false (as FloatVal does)
    template <class Comparison>
    bool
    certainly(Comparison comparison) {
        try {
            return comparison();
        } catch (const gecode_boost::numeric::interval_lib::comparison_error&) {
            return false;
        }
    }

    /// Return \a n in hexadecimal floating-point notation
    std::string
    hex(FloatNum n) {
        std::ostringstream s;
        s << std::hexfloat << n;
        return s.str();
    }
    /// Return \a b in hexadecimal floating-point notation
    std::string
    hex(const Bounds& b) {
        return '[' + hex(b.lower) + ',' + hex(b.upper) + ']';
    }
    /// Return a readable form of \a o
    std::string
    text(const Outcome& o) {
        std::string s;
        switch (o.kind) {
        case Kind::BOUNDS:
            s = '[' + hex(std::bit_cast<FloatNum>(o.lower)) + ',' + hex(std::bit_cast<FloatNum>(o.upper)) + ']';
            break;
        case Kind::TRUTH:
            s = o.truth ? "true" : "false";
            break;
        case Kind::THROW:
            s = std::string("throw ") + o.exception.name();
            break;
        }
        return s + (o.mode_kept ? "" : " (mode changed)");
    }

    /// FloatVal and Float::Interval give the vendored Boost interval's results bit for bit
    class MatchesBoost : public Base {
    private:
        /// Most mismatches to log
        static constexpr int max_logged = 20;
        /// Caller modes that the platform supports
        std::vector<int> modes;
        /// Operands of the operations being compared, for the log
        std::string operands;
        /// Number of mismatches
        int mismatches = 0;

        /// Compare \a candidate_op with \a reference_op in every caller mode
        template <class CandidateOp, class ReferenceOp>
        void
        compare(const char* op, CandidateOp candidate_op, ReferenceOp reference_op) {
            for (int mode : modes) {
                const Outcome c = observe(mode, candidate_op);
                const Outcome r = observe(mode, reference_op);
                if ((c == r) && c.mode_kept)
                    continue;
                if (mismatches < max_logged)
                    olog << "Mode " << mode << ' ' << op << ' ' << operands << ": " << text(c) << ", Boost " << text(r)
                         << '\n';
                mismatches++;
            }
        }

        /// Compare the default value and the constants
        void
        constants(void) {
            operands = "";
            compare("default", [] { return FloatVal(); }, [] { return Reference(); });
            compare(
                "pi_half", [] { return FloatVal::pi_half(); },
                [] { return reference::interval_lib::pi_half<Reference>(); });
            compare("pi", [] { return FloatVal::pi(); }, [] { return reference::interval_lib::pi<Reference>(); });
            compare(
                "pi_twice", [] { return FloatVal::pi_twice(); },
                [] { return reference::interval_lib::pi_twice<Reference>(); });
        }
        /// Compare construction from \a n
        void
        from_number(FloatNum n) {
            operands = hex(n);
            compare("from_number", [&] { return FloatVal(n); }, [&] { return Reference(n); });
            compare(
                "assign_number",
                [&] {
                    FloatVal z(1.0, 2.0);
                    z = n;
                    return z;
                },
                [&] {
                    Reference z(1.0, 2.0);
                    z = n;
                    return z;
                });
        }
        /// Compare construction from \a l and \a u
        void
        from_numbers(FloatNum l, FloatNum u) {
            operands = hex(l) + ' ' + hex(u);
            compare("construct", [&] { return FloatVal(l, u); }, [&] { return Reference(l, u); });
            compare(
                "assign",
                [&] {
                    FloatVal z;
                    z.assign(l, u);
                    return z;
                },
                [&] {
                    Reference z;
                    z.assign(l, u);
                    return z;
                });
        }
        /// Compare the operations on one interval \a b
        void
        unary(const Bounds& b) {
            operands = hex(b);
            const FloatVal x = value_of(b);
            const Reference rx = reference_of(b);
            compare("pos", [&] { return +x; }, [&] { return +rx; });
            // FloatVal's negation makes zero bounds positive and was never Boost's.
            const Interval ix = interval_of(b);
            compare("neg", [&] { return -ix; }, [&] { return -rx; });
            compare("abs", [&] { return Gecode::abs(x); }, [&] { return reference::abs(rx); });
            compare("sqrt", [&] { return Gecode::sqrt(x); }, [&] { return reference::sqrt(rx); });
            compare("sqr", [&] { return Gecode::sqr(x); }, [&] { return reference::square(rx); });
            for (int n = -3; n <= 5; n++) {
                operands = hex(b) + " pow " + std::to_string(n);
                compare("pow", [&] { return Gecode::pow(x, n); }, [&] { return reference::pow(rx, n); });
            }
            for (int k = 1; k <= 5; k++) {
                operands = hex(b) + " root " + std::to_string(k);
                compare("nroot", [&] { return Gecode::nroot(x, k); }, [&] { return reference::nth_root(rx, k); });
            }
            operands = hex(b);
            compare("size", [&] { return x.size(); }, [&] { return reference::width(rx); });
            compare("singleton", [&] { return x.singleton(); }, [&] { return reference::singleton(rx); });
            compare("zero_in", [&] { return x.zero_in(); }, [&] { return reference::zero_in(rx); });
#ifdef GECODE_HAS_MPFR
            compare("exp", [&] { return Gecode::exp(x); }, [&] { return reference::exp(rx); });
            compare("log", [&] { return Gecode::log(x); }, [&] { return reference::log(rx); });
            compare("sin", [&] { return Gecode::sin(x); }, [&] { return reference::sin(rx); });
            compare("cos", [&] { return Gecode::cos(x); }, [&] { return reference::cos(rx); });
            compare("tan", [&] { return Gecode::tan(x); }, [&] { return reference::tan(rx); });
            compare("asin", [&] { return Gecode::asin(x); }, [&] { return reference::asin(rx); });
            compare("acos", [&] { return Gecode::acos(x); }, [&] { return reference::acos(rx); });
            compare("atan", [&] { return Gecode::atan(x); }, [&] { return reference::atan(rx); });
            compare("sinh", [&] { return Gecode::sinh(x); }, [&] { return reference::sinh(rx); });
            compare("cosh", [&] { return Gecode::cosh(x); }, [&] { return reference::cosh(rx); });
            compare("tanh", [&] { return Gecode::tanh(x); }, [&] { return reference::tanh(rx); });
            compare("asinh", [&] { return Gecode::asinh(x); }, [&] { return reference::asinh(rx); });
            compare("acosh", [&] { return Gecode::acosh(x); }, [&] { return reference::acosh(rx); });
            compare("atanh", [&] { return Gecode::atanh(x); }, [&] { return reference::atanh(rx); });
#endif
        }
        /// Compare the operations on the intervals \a a and \a b
        void
        binary(const Bounds& a, const Bounds& b) {
            operands = hex(a) + ' ' + hex(b);
            const FloatVal x = value_of(a);
            const FloatVal y = value_of(b);
            const Reference rx = reference_of(a);
            const Reference ry = reference_of(b);
            compare("add", [&] { return x + y; }, [&] { return rx + ry; });
            compare("sub", [&] { return x - y; }, [&] { return rx - ry; });
            compare("mul", [&] { return x * y; }, [&] { return rx * ry; });
            compare("div", [&] { return x / y; }, [&] { return rx / ry; });
            compare(
                "add_assign",
                [&] {
                    FloatVal z = x;
                    return z += y;
                },
                [&] {
                    Reference z = rx;
                    return z += ry;
                });
            compare(
                "sub_assign",
                [&] {
                    FloatVal z = x;
                    return z -= y;
                },
                [&] {
                    Reference z = rx;
                    return z -= ry;
                });
            compare(
                "mul_assign",
                [&] {
                    FloatVal z = x;
                    return z *= y;
                },
                [&] {
                    Reference z = rx;
                    return z *= ry;
                });
            compare(
                "div_assign",
                [&] {
                    FloatVal z = x;
                    return z /= y;
                },
                [&] {
                    Reference z = rx;
                    return z /= ry;
                });
            compare("lt", [&] { return x < y; }, [&] { return certainly([&] { return rx < ry; }); });
            compare("le", [&] { return x <= y; }, [&] { return certainly([&] { return rx <= ry; }); });
            compare("gt", [&] { return x > y; }, [&] { return certainly([&] { return rx > ry; }); });
            compare("ge", [&] { return x >= y; }, [&] { return certainly([&] { return rx >= ry; }); });
            compare("eq", [&] { return x == y; }, [&] { return certainly([&] { return rx == ry; }); });
            compare("ne", [&] { return x != y; }, [&] { return certainly([&] { return rx != ry; }); });
            compare("equal", [&] { return x == y; }, [&] { return reference::equal(rx, ry); });
            compare("max", [&] { return Gecode::max(x, y); }, [&] { return reference::max(rx, ry); });
            compare("min", [&] { return Gecode::min(x, y); }, [&] { return reference::min(rx, ry); });
            compare("subset", [&] { return Gecode::Float::subset(x, y); }, [&] { return reference::subset(rx, ry); });
            compare(
                "proper_subset", [&] { return Gecode::Float::proper_subset(x, y); },
                [&] { return reference::proper_subset(rx, ry); });
            compare(
                "overlap", [&] { return Gecode::Float::overlap(x, y); }, [&] { return reference::overlap(rx, ry); });
            compare(
                "intersect", [&] { return Gecode::Float::intersect(x, y); },
                [&] { return reference::intersect(rx, ry); });
            compare("hull", [&] { return Gecode::Float::hull(x, y); }, [&] { return reference::hull(rx, ry); });
#ifdef GECODE_HAS_MPFR
            compare("fmod", [&] { return Gecode::fmod(x, y); }, [&] { return reference::fmod(rx, ry); });
#else
            // FloatVal has fmod only with MPFR.
            const Interval ix = interval_of(a);
            const Interval iy = interval_of(b);
            compare("fmod", [&] { return Gecode::Float::fmod(ix, iy); }, [&] { return reference::fmod(rx, ry); });
#endif
        }
        /// Compare the operations on the interval \a b and the number \a n
        void
        mixed(const Bounds& b, FloatNum n) {
            operands = hex(b) + ' ' + hex(n);
            const FloatVal x = value_of(b);
            const Reference rx = reference_of(b);
            compare("add", [&] { return x + n; }, [&] { return rx + n; });
            compare("add_left", [&] { return n + x; }, [&] { return n + rx; });
            compare("sub", [&] { return x - n; }, [&] { return rx - n; });
            compare("sub_left", [&] { return n - x; }, [&] { return n - rx; });
            compare("mul", [&] { return x * n; }, [&] { return rx * n; });
            compare("mul_left", [&] { return n * x; }, [&] { return n * rx; });
            compare("div", [&] { return x / n; }, [&] { return rx / n; });
            compare("div_left", [&] { return n / x; }, [&] { return n / rx; });
            compare(
                "add_assign",
                [&] {
                    FloatVal z = x;
                    return z += n;
                },
                [&] {
                    Reference z = rx;
                    return z += n;
                });
            compare(
                "sub_assign",
                [&] {
                    FloatVal z = x;
                    return z -= n;
                },
                [&] {
                    Reference z = rx;
                    return z -= n;
                });
            compare(
                "mul_assign",
                [&] {
                    FloatVal z = x;
                    return z *= n;
                },
                [&] {
                    Reference z = rx;
                    return z *= n;
                });
            compare(
                "div_assign",
                [&] {
                    FloatVal z = x;
                    return z /= n;
                },
                [&] {
                    Reference z = rx;
                    return z /= n;
                });
            compare("lt", [&] { return x < n; }, [&] { return certainly([&] { return rx < n; }); });
            compare("le", [&] { return x <= n; }, [&] { return certainly([&] { return rx <= n; }); });
            compare("gt", [&] { return x > n; }, [&] { return certainly([&] { return rx > n; }); });
            compare("ge", [&] { return x >= n; }, [&] { return certainly([&] { return rx >= n; }); });
            compare("ne", [&] { return x != n; }, [&] { return certainly([&] { return rx != n; }); });
            // Boost has no comparisons with the number on the left; FloatVal mirrors them.
            compare("lt_left", [&] { return n < x; }, [&] { return certainly([&] { return rx > n; }); });
            compare("le_left", [&] { return n <= x; }, [&] { return certainly([&] { return rx >= n; }); });
            compare("gt_left", [&] { return n > x; }, [&] { return certainly([&] { return rx < n; }); });
            compare("ge_left", [&] { return n >= x; }, [&] { return certainly([&] { return rx <= n; }); });
            compare("ne_left", [&] { return n != x; }, [&] { return certainly([&] { return rx != n; }); });
            compare("max", [&] { return Gecode::max(x, n); }, [&] { return reference::max(rx, n); });
            compare("max_left", [&] { return Gecode::max(n, x); }, [&] { return reference::max(n, rx); });
            compare("min", [&] { return Gecode::min(x, n); }, [&] { return reference::min(rx, n); });
            compare("min_left", [&] { return Gecode::min(n, x); }, [&] { return reference::min(n, rx); });
            compare("hull", [&] { return Gecode::Float::hull(x, n); }, [&] { return reference::hull(rx, n); });
            compare("hull_left", [&] { return Gecode::Float::hull(n, x); }, [&] { return reference::hull(n, rx); });
            compare("in", [&] { return x.in(n); }, [&] { return reference::in(n, rx); });
#ifdef GECODE_HAS_MPFR
            compare("fmod", [&] { return Gecode::fmod(x, n); }, [&] { return reference::fmod(rx, n); });
            compare("fmod_left", [&] { return Gecode::fmod(n, x); }, [&] { return reference::fmod(n, rx); });
#else
            // FloatVal has fmod only with MPFR.
            const Interval ix = interval_of(b);
            compare("fmod", [&] { return Gecode::Float::fmod(ix, n); }, [&] { return reference::fmod(rx, n); });
            compare("fmod_left", [&] { return Gecode::Float::fmod(n, ix); }, [&] { return reference::fmod(n, rx); });
#endif
        }

        /// Return the value with bounds \a b
        static FloatVal
        value_of(const Bounds& b) {
            return std::isnan(b.lower) ? FloatVal(b.lower) : FloatVal(b.lower, b.upper);
        }
        /// Return the interval with bounds \a b
        static Interval
        interval_of(const Bounds& b) {
            return std::isnan(b.lower) ? Interval(b.lower) : Interval(b.lower, b.upper);
        }
        /// Return the reference interval with bounds \a b
        static Reference
        reference_of(const Bounds& b) {
            return std::isnan(b.lower) ? Reference(b.lower) : Reference(b.lower, b.upper);
        }

    public:
        MatchesBoost(void) : Base("Float::Interval::MatchesBoost", TestTags::standard()) {}
        bool
        run(void) override {
            const int previous = std::fegetround();
            modes.clear();
            for (int mode : {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO})
                if (std::fesetround(mode) == 0)
                    modes.push_back(mode);
            std::fesetround(FE_TONEAREST);
            mismatches = 0;

            const std::vector<FloatNum> numbers = grid_numbers();
            const std::vector<Bounds> intervals = grid_intervals();
            constants();
            for (FloatNum n : numbers)
                from_number(n);
            for (FloatNum l : numbers)
                for (FloatNum u : numbers)
                    from_numbers(l, u);
            for (const Bounds& b : intervals)
                unary(b);
            for (const Bounds& a : intervals)
                for (const Bounds& b : intervals)
                    binary(a, b);
            for (const Bounds& b : intervals)
                for (FloatNum n : numbers)
                    mixed(b, n);

            if (previous != -1)
                std::fesetround(previous);
            if (mismatches > 0)
                olog << mismatches << " mismatches\n";
            return mismatches == 0;
        }
    } matches_boost;

}}} // namespace Test::Float

// STATISTICS: test-float
