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
 * Temporary differential test: Float::Interval against the vendored Boost
 * interval that FloatVal uses today. It is removed together with the Boost
 * copy.
 *
 */

#include "test/float.hh"

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
    /// The interval type under test
    using Candidate = Gecode::Float::Interval;
    /// FloatVal's implementation type, which gives the expected results
    using Reference = gecode_boost::numeric::interval<
        FloatNum, gecode_boost::numeric::interval_lib::policies<
                      Gecode::Float::Rounding, gecode_boost::numeric::interval_lib::checking_strict<FloatNum>>>;

    namespace candidate = Gecode::Float;
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
        /// Bit patterns of the bounds, for Kind::BOUNDS
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

    /// Return the outcome for the bounds \a l and \a u
    Outcome
    bounds_outcome(FloatNum l, FloatNum u) {
        Outcome o;
        o.lower = std::bit_cast<std::uint64_t>(l);
        o.upper = std::bit_cast<std::uint64_t>(u);
        return o;
    }
    /// Return the outcome for interval \a x
    Outcome
    outcome_of(const Candidate& x) {
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

    /// Float::Interval gives the vendored Boost interval's results bit for bit
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

        /// Compare the default interval and the constants
        void
        constants(void) {
            operands = "";
            compare("default", [] { return Candidate(); }, [] { return Reference(); });
            compare(
                "pi_half", [] { return Candidate::pi_half(); },
                [] { return reference::interval_lib::pi_half<Reference>(); });
            compare("pi", [] { return Candidate::pi(); }, [] { return reference::interval_lib::pi<Reference>(); });
            compare(
                "pi_twice", [] { return Candidate::pi_twice(); },
                [] { return reference::interval_lib::pi_twice<Reference>(); });
        }
        /// Compare construction from \a n
        void
        from_number(FloatNum n) {
            operands = hex(n);
            compare("from_number", [&] { return Candidate(n); }, [&] { return Reference(n); });
            compare(
                "assign_number",
                [&] {
                    Candidate z(1.0, 2.0);
                    z = Candidate(n);
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
            compare("construct", [&] { return Candidate(l, u); }, [&] { return Reference(l, u); });
            compare(
                "assign",
                [&] {
                    Candidate z;
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
            const Candidate cx = candidate_interval(b);
            const Reference rx = reference_interval(b);
            compare("pos", [&] { return +cx; }, [&] { return +rx; });
            compare("neg", [&] { return -cx; }, [&] { return -rx; });
            compare("abs", [&] { return candidate::abs(cx); }, [&] { return reference::abs(rx); });
            compare("sqrt", [&] { return candidate::sqrt(cx); }, [&] { return reference::sqrt(rx); });
            compare("square", [&] { return candidate::square(cx); }, [&] { return reference::square(rx); });
            for (int n = -3; n <= 5; n++) {
                operands = hex(b) + " pow " + std::to_string(n);
                compare("pow", [&] { return candidate::pow(cx, n); }, [&] { return reference::pow(rx, n); });
            }
            for (int k = 1; k <= 5; k++) {
                operands = hex(b) + " root " + std::to_string(k);
                compare(
                    "nth_root", [&] { return candidate::nth_root(cx, k); }, [&] { return reference::nth_root(rx, k); });
            }
            operands = hex(b);
            compare("width", [&] { return candidate::width(cx); }, [&] { return reference::width(rx); });
            compare("singleton", [&] { return candidate::singleton(cx); }, [&] { return reference::singleton(rx); });
            compare("zero_in", [&] { return candidate::zero_in(cx); }, [&] { return reference::zero_in(rx); });
#ifdef GECODE_HAS_MPFR
            compare("exp", [&] { return candidate::exp(cx); }, [&] { return reference::exp(rx); });
            compare("log", [&] { return candidate::log(cx); }, [&] { return reference::log(rx); });
            compare("sin", [&] { return candidate::sin(cx); }, [&] { return reference::sin(rx); });
            compare("cos", [&] { return candidate::cos(cx); }, [&] { return reference::cos(rx); });
            compare("tan", [&] { return candidate::tan(cx); }, [&] { return reference::tan(rx); });
            compare("asin", [&] { return candidate::asin(cx); }, [&] { return reference::asin(rx); });
            compare("acos", [&] { return candidate::acos(cx); }, [&] { return reference::acos(rx); });
            compare("atan", [&] { return candidate::atan(cx); }, [&] { return reference::atan(rx); });
            compare("sinh", [&] { return candidate::sinh(cx); }, [&] { return reference::sinh(rx); });
            compare("cosh", [&] { return candidate::cosh(cx); }, [&] { return reference::cosh(rx); });
            compare("tanh", [&] { return candidate::tanh(cx); }, [&] { return reference::tanh(rx); });
            compare("asinh", [&] { return candidate::asinh(cx); }, [&] { return reference::asinh(rx); });
            compare("acosh", [&] { return candidate::acosh(cx); }, [&] { return reference::acosh(rx); });
            compare("atanh", [&] { return candidate::atanh(cx); }, [&] { return reference::atanh(rx); });
#endif
        }
        /// Compare the operations on the intervals \a a and \a b
        void
        binary(const Bounds& a, const Bounds& b) {
            operands = hex(a) + ' ' + hex(b);
            const Candidate cx = candidate_interval(a);
            const Candidate cy = candidate_interval(b);
            const Reference rx = reference_interval(a);
            const Reference ry = reference_interval(b);
            compare("add", [&] { return cx + cy; }, [&] { return rx + ry; });
            compare("sub", [&] { return cx - cy; }, [&] { return rx - ry; });
            compare("mul", [&] { return cx * cy; }, [&] { return rx * ry; });
            compare("div", [&] { return cx / cy; }, [&] { return rx / ry; });
            compare(
                "add_assign",
                [&] {
                    Candidate z = cx;
                    return z += cy;
                },
                [&] {
                    Reference z = rx;
                    return z += ry;
                });
            compare(
                "sub_assign",
                [&] {
                    Candidate z = cx;
                    return z -= cy;
                },
                [&] {
                    Reference z = rx;
                    return z -= ry;
                });
            compare(
                "mul_assign",
                [&] {
                    Candidate z = cx;
                    return z *= cy;
                },
                [&] {
                    Reference z = rx;
                    return z *= ry;
                });
            compare(
                "div_assign",
                [&] {
                    Candidate z = cx;
                    return z /= cy;
                },
                [&] {
                    Reference z = rx;
                    return z /= ry;
                });
            compare(
                "lt", [&] { return candidate::certainly_less(cx, cy); },
                [&] { return certainly([&] { return rx < ry; }); });
            compare(
                "le", [&] { return candidate::certainly_less_equal(cx, cy); },
                [&] { return certainly([&] { return rx <= ry; }); });
            compare(
                "gt", [&] { return candidate::certainly_greater(cx, cy); },
                [&] { return certainly([&] { return rx > ry; }); });
            compare(
                "ge", [&] { return candidate::certainly_greater_equal(cx, cy); },
                [&] { return certainly([&] { return rx >= ry; }); });
            compare(
                "eq", [&] { return candidate::equal(cx, cy); }, [&] { return certainly([&] { return rx == ry; }); });
            compare(
                "ne", [&] { return candidate::certainly_unequal(cx, cy); },
                [&] { return certainly([&] { return rx != ry; }); });
            compare("max", [&] { return candidate::max(cx, cy); }, [&] { return reference::max(rx, ry); });
            compare("min", [&] { return candidate::min(cx, cy); }, [&] { return reference::min(rx, ry); });
            compare("subset", [&] { return candidate::subset(cx, cy); }, [&] { return reference::subset(rx, ry); });
            compare(
                "proper_subset", [&] { return candidate::proper_subset(cx, cy); },
                [&] { return reference::proper_subset(rx, ry); });
            compare("overlap", [&] { return candidate::overlap(cx, cy); }, [&] { return reference::overlap(rx, ry); });
            compare("equal", [&] { return candidate::equal(cx, cy); }, [&] { return reference::equal(rx, ry); });
            compare(
                "intersect", [&] { return candidate::intersect(cx, cy); },
                [&] { return reference::intersect(rx, ry); });
            compare("hull", [&] { return candidate::hull(cx, cy); }, [&] { return reference::hull(rx, ry); });
            compare("fmod", [&] { return candidate::fmod(cx, cy); }, [&] { return reference::fmod(rx, ry); });
        }
        /// Compare the operations on the interval \a b and the number \a n
        void
        mixed(const Bounds& b, FloatNum n) {
            operands = hex(b) + ' ' + hex(n);
            const Candidate cx = candidate_interval(b);
            const Reference rx = reference_interval(b);
            compare("add", [&] { return cx + n; }, [&] { return rx + n; });
            compare("add_left", [&] { return n + cx; }, [&] { return n + rx; });
            compare("sub", [&] { return cx - n; }, [&] { return rx - n; });
            compare("sub_left", [&] { return n - cx; }, [&] { return n - rx; });
            compare("mul", [&] { return cx * n; }, [&] { return rx * n; });
            compare("mul_left", [&] { return n * cx; }, [&] { return n * rx; });
            compare("div", [&] { return cx / n; }, [&] { return rx / n; });
            compare("div_left", [&] { return n / cx; }, [&] { return n / rx; });
            compare(
                "add_assign",
                [&] {
                    Candidate z = cx;
                    return z += n;
                },
                [&] {
                    Reference z = rx;
                    return z += n;
                });
            compare(
                "sub_assign",
                [&] {
                    Candidate z = cx;
                    return z -= n;
                },
                [&] {
                    Reference z = rx;
                    return z -= n;
                });
            compare(
                "mul_assign",
                [&] {
                    Candidate z = cx;
                    return z *= n;
                },
                [&] {
                    Reference z = rx;
                    return z *= n;
                });
            compare(
                "div_assign",
                [&] {
                    Candidate z = cx;
                    return z /= n;
                },
                [&] {
                    Reference z = rx;
                    return z /= n;
                });
            compare(
                "lt", [&] { return candidate::certainly_less(cx, n); },
                [&] { return certainly([&] { return rx < n; }); });
            compare(
                "le", [&] { return candidate::certainly_less_equal(cx, n); },
                [&] { return certainly([&] { return rx <= n; }); });
            compare(
                "gt", [&] { return candidate::certainly_greater(cx, n); },
                [&] { return certainly([&] { return rx > n; }); });
            compare(
                "ge", [&] { return candidate::certainly_greater_equal(cx, n); },
                [&] { return certainly([&] { return rx >= n; }); });
            compare(
                "ne", [&] { return candidate::certainly_unequal(cx, n); },
                [&] { return certainly([&] { return rx != n; }); });
            compare("max", [&] { return candidate::max(cx, n); }, [&] { return reference::max(rx, n); });
            compare("max_left", [&] { return candidate::max(n, cx); }, [&] { return reference::max(n, rx); });
            compare("min", [&] { return candidate::min(cx, n); }, [&] { return reference::min(rx, n); });
            compare("min_left", [&] { return candidate::min(n, cx); }, [&] { return reference::min(n, rx); });
            compare("hull", [&] { return candidate::hull(cx, n); }, [&] { return reference::hull(rx, n); });
            compare("hull_left", [&] { return candidate::hull(n, cx); }, [&] { return reference::hull(n, rx); });
            compare("in", [&] { return candidate::in(n, cx); }, [&] { return reference::in(n, rx); });
            compare("fmod", [&] { return candidate::fmod(cx, n); }, [&] { return reference::fmod(rx, n); });
            compare("fmod_left", [&] { return candidate::fmod(n, cx); }, [&] { return reference::fmod(n, rx); });
        }

        /// Return the candidate interval with bounds \a b
        static Candidate
        candidate_interval(const Bounds& b) {
            return std::isnan(b.lower) ? Candidate(b.lower) : Candidate(b.lower, b.upper);
        }
        /// Return the reference interval with bounds \a b
        static Reference
        reference_interval(const Bounds& b) {
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
