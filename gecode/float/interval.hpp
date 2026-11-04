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

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace Gecode { namespace Float {

    /**
     * \brief Closed interval of floating-point numbers with directed rounding
     *
     * An interval \f$[l,u]\f$ stands for every real number between its
     * bounds. Arithmetic rounds each lower bound toward \f$-\infty\f$ and each
     * upper bound toward \f$+\infty\f$, so a result contains the exact result
     * of the operation for all members of its operands.
     *
     * Contract:
     *  - An interval is never empty. Interval(l,u) and assign() throw
     *    std::runtime_error unless \f$l\leq u\f$, which also rejects NaN
     *    bounds. An operation whose exact result is empty throws the same
     *    exception: division by \f$[0,0]\f$ or by 0, pow() of \f$[0,0]\f$
     *    with an exponent of at most 0, sqrt() of an interval without a
     *    nonnegative member, log() of one without a positive member, an even
     *    nth_root() of a negative interval, a disjoint intersect(), and
     *    asin(), acos(), acosh(), and atanh() outside their domains.
     *  - Interval(v) and unchecked() check nothing. Results keep NaN bounds,
     *    for example from \f$0\cdot\infty\f$ or \f$\infty-\infty\f$.
     *  - Each operation that rounds opens one Rounding for its whole
     *    computation, so the caller's rounding mode is restored afterwards,
     *    also when the operation throws.
     *  - There are no comparison operators. FloatVal's relations hold only
     *    when they hold for all members of both operands, such as
     *    certainly_less(), while its equality compares bounds, as equal()
     *    does. Named functions keep the two meanings apart.
     *
     * \par Design decision
     * Gecode implements the interval arithmetic it needs itself, for double
     * bounds, with no external dependency. Directed rounding switches the
     * floating-point rounding mode through RoundingBase and Rounding. The
     * measured alternatives were a script-refreshed copy of Boost 1.92.0's
     * interval library, the 2012 copy of Boost 1.47.0 made reproducible, and
     * outward rounding with std::nextafter, either always widening or
     * widening only by the sign of an error term. The nextafter variants
     * loosen bounds, which was not accepted, and an installed Boost was ruled
     * out as a dependency. This implementation was chosen because it
     * reproduces the Boost copy's bounds, truth values, and search statistics
     * bit for bit on arm64 and x86-64. Without the copy, Gecode installs 95
     * fewer headers and no third-party code, exports the same symbols, and
     * runs as fast or faster. The interval type is trivially copyable.
     *
     * \par Acknowledgement
     * The operations reproduce the results of the Boost Interval Arithmetic
     * Library (Boost.Interval), which Gecode has used since 2012 and which
     * served as the reference during development.
     *
     * \ingroup TaskModelFloatVars
     */
    class Interval {
    private:
        /// Lower bound
        FloatNum lb = 0.0;
        /// Upper bound
        FloatNum ub = 0.0;

    public:
        /// \name Construction and access
        //@{
        /// Initialize as \f$[0,0]\f$
        Interval(void) = default;
        /// Initialize as \f$[v,v]\f$, accepting NaN
        explicit Interval(FloatNum v);
        /// Initialize as \f$[l,u]\f$, throwing unless \f$l\leq u\f$
        Interval(FloatNum l, FloatNum u);
        /// Return \f$[l,u]\f$ without checking the bounds
        static Interval unchecked(FloatNum l, FloatNum u);
        /// Set to \f$[l,u]\f$, or throw and stay unchanged unless \f$l\leq u\f$
        void assign(FloatNum l, FloatNum u);
        /// Return lower bound
        FloatNum lower(void) const;
        /// Return upper bound
        FloatNum upper(void) const;
        //@}

        /// \name Constants
        //@{
        /// Return enclosure of \f$\pi/2\f$
        static Interval pi_half(void);
        /// Return enclosure of \f$\pi\f$
        static Interval pi(void);
        /// Return enclosure of \f$2\pi\f$
        static Interval pi_twice(void);
        //@}

        /// \name Update operators
        //@{
        /// Add \a y
        Interval& operator+=(const Interval& y);
        /// Subtract \a y
        Interval& operator-=(const Interval& y);
        /// Multiply by \a y
        Interval& operator*=(const Interval& y);
        /// Divide by \a y
        Interval& operator/=(const Interval& y);
        /// Add \a n
        Interval& operator+=(FloatNum n);
        /// Subtract \a n
        Interval& operator-=(FloatNum n);
        /// Multiply by \a n
        Interval& operator*=(FloatNum n);
        /// Divide by \a n
        Interval& operator/=(FloatNum n);
        //@}
    };

    /// Implementation details of Interval
    namespace IntervalDetail {

        /// Throw the exception for an empty result
        [[noreturn]] inline void
        throw_empty(void) {
            throw std::runtime_error("Gecode::Float::Interval: empty interval");
        }

        /// Return \f$[-\infty,+\infty]\f$
        forceinline Interval
        whole(void) {
            const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
            return Interval::unchecked(-inf, inf);
        }

    } // namespace IntervalDetail

    /*
     * Construction and access
     *
     */

    forceinline
    Interval::Interval(FloatNum v)
        : lb(v), ub(v) {}

    forceinline
    Interval::Interval(FloatNum l, FloatNum u)
        : lb(l), ub(u) {
        if (!(l <= u))
            IntervalDetail::throw_empty();
    }

    forceinline Interval
    Interval::unchecked(FloatNum l, FloatNum u) {
        Interval x;
        x.lb = l;
        x.ub = u;
        return x;
    }

    forceinline void
    Interval::assign(FloatNum l, FloatNum u) {
        if (!(l <= u))
            IntervalDetail::throw_empty();
        lb = l;
        ub = u;
    }

    forceinline FloatNum
    Interval::lower(void) const {
        return lb;
    }
    forceinline FloatNum
    Interval::upper(void) const {
        return ub;
    }

    forceinline Interval
    Interval::pi_half(void) {
        return unchecked(pi_half_lower(), pi_half_upper());
    }
    forceinline Interval
    Interval::pi(void) {
        return unchecked(pi_lower(), pi_upper());
    }
    forceinline Interval
    Interval::pi_twice(void) {
        return unchecked(pi_twice_lower(), pi_twice_upper());
    }

    /*
     * Queries and set operations, which need no rounding
     *
     * Where two bounds tie, std::min and std::max return their left argument,
     * which decides the sign of a zero and which NaN is kept.
     *
     */

    /**
     * \brief Test whether \a x has a single member
     * \relates Interval
     */
    forceinline bool
    singleton(const Interval& x) {
        return x.lower() == x.upper();
    }
    /**
     * \brief Test whether \a n is a member of \a x
     * \relates Interval
     */
    forceinline bool
    in(FloatNum n, const Interval& x) {
        return (x.lower() <= n) && (n <= x.upper());
    }
    /**
     * \brief Test whether zero is a member of \a x
     *
     * Written as the absence of a sign, so that NaN bounds count as
     * containing zero.
     * \relates Interval
     */
    forceinline bool
    zero_in(const Interval& x) {
        return !((x.lower() > 0.0) || (x.upper() < 0.0));
    }
    /**
     * \brief Test whether \a x and \a y have the same bounds
     * \relates Interval
     */
    forceinline bool
    equal(const Interval& x, const Interval& y) {
        return (x.lower() == y.lower()) && (x.upper() == y.upper());
    }
    /**
     * \brief Test whether \a x is a subset of \a y
     * \relates Interval
     */
    forceinline bool
    subset(const Interval& x, const Interval& y) {
        return (y.lower() <= x.lower()) && (x.upper() <= y.upper());
    }
    /**
     * \brief Test whether \a x is a proper subset of \a y
     * \relates Interval
     */
    forceinline bool
    proper_subset(const Interval& x, const Interval& y) {
        return subset(x, y) && !equal(x, y);
    }
    /**
     * \brief Test whether \a x and \a y have a common member
     * \relates Interval
     */
    forceinline bool
    overlap(const Interval& x, const Interval& y) {
        return in(y.lower(), x) || in(x.lower(), y);
    }

    /**
     * \brief Return the intersection of \a x and \a y, throwing if it is empty
     * \relates Interval
     */
    forceinline Interval
    intersect(const Interval& x, const Interval& y) {
        const FloatNum l = std::max(x.lower(), y.lower());
        const FloatNum u = std::min(x.upper(), y.upper());
        if (!(l <= u))
            IntervalDetail::throw_empty();
        return Interval::unchecked(l, u);
    }
    /**
     * \brief Return the smallest interval containing \a x and \a y
     * \relates Interval
     */
    forceinline Interval
    hull(const Interval& x, const Interval& y) {
        return Interval::unchecked(std::min(x.lower(), y.lower()), std::max(x.upper(), y.upper()));
    }
    /**
     * \brief Return the smallest interval containing \a x and \a n
     * \relates Interval
     */
    forceinline Interval
    hull(const Interval& x, FloatNum n) {
        return Interval::unchecked(std::min(x.lower(), n), std::max(x.upper(), n));
    }
    /**
     * \brief Return the smallest interval containing \a n and \a x
     * \relates Interval
     */
    forceinline Interval
    hull(FloatNum n, const Interval& x) {
        return Interval::unchecked(std::min(n, x.lower()), std::max(n, x.upper()));
    }

    /**
     * \brief Return the bound-wise maximum of \a x and \a y
     * \relates Interval
     */
    forceinline Interval
    max(const Interval& x, const Interval& y) {
        return Interval::unchecked(std::max(x.lower(), y.lower()), std::max(x.upper(), y.upper()));
    }
    /**
     * \brief Return the bound-wise maximum of \a x and \a n
     * \relates Interval
     */
    forceinline Interval
    max(const Interval& x, FloatNum n) {
        return Interval::unchecked(std::max(x.lower(), n), std::max(x.upper(), n));
    }
    /**
     * \brief Return the bound-wise maximum of \a n and \a x
     * \relates Interval
     */
    forceinline Interval
    max(FloatNum n, const Interval& x) {
        return Interval::unchecked(std::max(n, x.lower()), std::max(n, x.upper()));
    }
    /**
     * \brief Return the bound-wise minimum of \a x and \a y
     * \relates Interval
     */
    forceinline Interval
    min(const Interval& x, const Interval& y) {
        return Interval::unchecked(std::min(x.lower(), y.lower()), std::min(x.upper(), y.upper()));
    }
    /**
     * \brief Return the bound-wise minimum of \a x and \a n
     * \relates Interval
     */
    forceinline Interval
    min(const Interval& x, FloatNum n) {
        return Interval::unchecked(std::min(x.lower(), n), std::min(x.upper(), n));
    }
    /**
     * \brief Return the bound-wise minimum of \a n and \a x
     * \relates Interval
     */
    forceinline Interval
    min(FloatNum n, const Interval& x) {
        return Interval::unchecked(std::min(n, x.lower()), std::min(n, x.upper()));
    }

    /**
     * \brief Return \a x
     * \relates Interval
     */
    forceinline Interval
    operator+(const Interval& x) {
        return x;
    }
    /**
     * \brief Return \f$\{-v \mid v\in x\}\f$, keeping the signs of zero bounds
     * \relates Interval
     */
    forceinline Interval
    operator-(const Interval& x) {
        return Interval::unchecked(-x.upper(), -x.lower());
    }
    /**
     * \brief Return \f$\{|v| \mid v\in x\}\f$
     * \relates Interval
     */
    inline Interval
    abs(const Interval& x) {
        if (!(x.lower() < 0.0))
            return x;
        if (!(x.upper() > 0.0))
            return -x;
        return Interval::unchecked(0.0, std::max(-x.lower(), x.upper()));
    }

    /*
     * Certain comparisons
     *
     * A relation between intervals holds certainly when it holds for every
     * member of the first and every member of the second interval. Each
     * relation compares only the bounds it needs, so a NaN bound makes a
     * relation false only when that bound takes part in it.
     *
     */

    /**
     * \brief Test whether every member of \a x is less than every member of \a y
     * \relates Interval
     */
    forceinline bool
    certainly_less(const Interval& x, const Interval& y) {
        return x.upper() < y.lower();
    }
    /**
     * \brief Test whether every member of \a x is less than \a n
     * \relates Interval
     */
    forceinline bool
    certainly_less(const Interval& x, FloatNum n) {
        return x.upper() < n;
    }
    /**
     * \brief Test whether every member of \a x is at most every member of \a y
     * \relates Interval
     */
    forceinline bool
    certainly_less_equal(const Interval& x, const Interval& y) {
        return x.upper() <= y.lower();
    }
    /**
     * \brief Test whether every member of \a x is at most \a n
     * \relates Interval
     */
    forceinline bool
    certainly_less_equal(const Interval& x, FloatNum n) {
        return x.upper() <= n;
    }
    /**
     * \brief Test whether every member of \a x is greater than every member of \a y
     * \relates Interval
     */
    forceinline bool
    certainly_greater(const Interval& x, const Interval& y) {
        return x.lower() > y.upper();
    }
    /**
     * \brief Test whether every member of \a x is greater than \a n
     * \relates Interval
     */
    forceinline bool
    certainly_greater(const Interval& x, FloatNum n) {
        return x.lower() > n;
    }
    /**
     * \brief Test whether every member of \a x is at least every member of \a y
     * \relates Interval
     */
    forceinline bool
    certainly_greater_equal(const Interval& x, const Interval& y) {
        return x.lower() >= y.upper();
    }
    /**
     * \brief Test whether every member of \a x is at least \a n
     * \relates Interval
     */
    forceinline bool
    certainly_greater_equal(const Interval& x, FloatNum n) {
        return x.lower() >= n;
    }
    /**
     * \brief Test whether no member of \a x equals a member of \a y
     * \relates Interval
     */
    forceinline bool
    certainly_unequal(const Interval& x, const Interval& y) {
        return certainly_less(x, y) || certainly_greater(x, y);
    }
    /**
     * \brief Test whether no member of \a x equals \a n
     * \relates Interval
     */
    forceinline bool
    certainly_unequal(const Interval& x, FloatNum n) {
        return certainly_less(x, n) || certainly_greater(x, n);
    }

    namespace IntervalDetail {

        /*
         * Rounded operations inside an open rounding scope
         *
         * The functions below take the scope that the calling operation
         * opened and open none themselves, so nested arithmetic runs in the
         * caller's scope. Each rounded step is a RoundingBase operation,
         * whose operand order decides the sign of a NaN result.
         *
         */

        /// Return \a x plus \a y
        forceinline Interval
        add(RoundingBase& r, const Interval& x, const Interval& y) {
            return Interval::unchecked(r.add_down(x.lower(), y.lower()), r.add_up(x.upper(), y.upper()));
        }
        /// Return \a n plus \a x, for either order of the operands
        forceinline Interval
        add(RoundingBase& r, FloatNum n, const Interval& x) {
            return Interval::unchecked(r.add_down(n, x.lower()), r.add_up(n, x.upper()));
        }
        /// Return \a x minus \a y
        forceinline Interval
        sub(RoundingBase& r, const Interval& x, const Interval& y) {
            return Interval::unchecked(r.sub_down(x.lower(), y.upper()), r.sub_up(x.upper(), y.lower()));
        }
        /// Return \a x minus \a n
        forceinline Interval
        sub(RoundingBase& r, const Interval& x, FloatNum n) {
            return Interval::unchecked(r.sub_down(x.lower(), n), r.sub_up(x.upper(), n));
        }
        /// Return \a n minus \a x
        forceinline Interval
        sub(RoundingBase& r, FloatNum n, const Interval& x) {
            return Interval::unchecked(r.sub_down(n, x.upper()), r.sub_up(n, x.lower()));
        }

        /// Signs of the members of an interval, for multiplication
        enum class Sign {
            NEGATIVE, ///< Members at most zero, some negative
            MIXED,    ///< Negative and positive members
            POSITIVE, ///< Members at least zero, some positive
            ZERO      ///< Only zero, or NaN bounds
        };
        /// Return the sign class of \a x
        forceinline Sign
        sign(const Interval& x) {
            if (x.lower() < 0.0)
                return (x.upper() > 0.0) ? Sign::MIXED : Sign::NEGATIVE;
            return (x.upper() > 0.0) ? Sign::POSITIVE : Sign::ZERO;
        }

        /**
         * \brief Return \a x times \a y
         *
         * Multiplication is monotone in each factor on each sign class, so
         * the bounds are products of endpoints chosen by the signs. Only when
         * both factors are mixed are two candidates compared. A zero factor
         * gives \f$[0,0]\f$, also against an infinite bound.
         */
        inline Interval
        mul(RoundingBase& r, const Interval& x, const Interval& y) {
            const FloatNum xl = x.lower();
            const FloatNum xu = x.upper();
            const FloatNum yl = y.lower();
            const FloatNum yu = y.upper();
            const Sign y_sign = sign(y);
            switch (sign(x)) {
            case Sign::ZERO:
                return Interval();
            case Sign::POSITIVE:
                switch (y_sign) {
                case Sign::POSITIVE:
                    return Interval::unchecked(r.mul_down(xl, yl), r.mul_up(xu, yu));
                case Sign::NEGATIVE:
                    return Interval::unchecked(r.mul_down(xu, yl), r.mul_up(xl, yu));
                case Sign::MIXED:
                    return Interval::unchecked(r.mul_down(xu, yl), r.mul_up(xu, yu));
                case Sign::ZERO:
                    return Interval();
                }
                break;
            case Sign::NEGATIVE:
                switch (y_sign) {
                case Sign::POSITIVE:
                    return Interval::unchecked(r.mul_down(xl, yu), r.mul_up(xu, yl));
                case Sign::NEGATIVE:
                    return Interval::unchecked(r.mul_down(xu, yu), r.mul_up(xl, yl));
                case Sign::MIXED:
                    return Interval::unchecked(r.mul_down(xl, yu), r.mul_up(xl, yl));
                case Sign::ZERO:
                    return Interval();
                }
                break;
            case Sign::MIXED:
                switch (y_sign) {
                case Sign::POSITIVE:
                    return Interval::unchecked(r.mul_down(xl, yu), r.mul_up(xu, yu));
                case Sign::NEGATIVE:
                    return Interval::unchecked(r.mul_down(xu, yl), r.mul_up(xl, yl));
                case Sign::MIXED:
                    return Interval::unchecked(std::min(r.mul_down(xl, yu), r.mul_down(xu, yl)),
                                               std::max(r.mul_up(xl, yl), r.mul_up(xu, yu)));
                case Sign::ZERO:
                    return Interval();
                }
                break;
            }
            GECODE_NEVER;
            return Interval();
        }
        /// Return \a n times \a x, for either order of the operands
        forceinline Interval
        mul(RoundingBase& r, FloatNum n, const Interval& x) {
            if (n < 0.0)
                return Interval::unchecked(r.mul_down(n, x.upper()), r.mul_up(n, x.lower()));
            if (n == 0.0)
                return Interval();
            return Interval::unchecked(r.mul_down(n, x.lower()), r.mul_up(n, x.upper()));
        }

        /// Where zero lies in a divisor
        enum class ZeroPosition {
            OUTSIDE,     ///< Not a member, so the divisor has one sign
            LOWER_BOUND, ///< Lower bound only, as in \f$[0,u]\f$
            UPPER_BOUND, ///< Upper bound only, as in \f$[l,0]\f$
            BOTH_BOUNDS, ///< The divisor is \f$[0,0]\f$
            INTERIOR     ///< Between the bounds, or NaN bounds
        };
        /// Return where zero lies in \a y
        forceinline ZeroPosition
        zero_position(const Interval& y) {
            if (!zero_in(y))
                return ZeroPosition::OUTSIDE;
            const bool lower_zero = y.lower() == 0.0;
            const bool upper_zero = y.upper() == 0.0;
            if (lower_zero && upper_zero)
                return ZeroPosition::BOTH_BOUNDS;
            if (lower_zero)
                return ZeroPosition::LOWER_BOUND;
            if (upper_zero)
                return ZeroPosition::UPPER_BOUND;
            return ZeroPosition::INTERIOR;
        }

        /**
         * \brief Return \a x divided by \a y, where \a y has one sign
         *
         * Division is monotone in each operand on each sign class, so the
         * bounds are quotients of endpoints chosen by the signs.
         */
        inline Interval
        div_sign_definite(RoundingBase& r, const Interval& x, const Interval& y) {
            const FloatNum xl = x.lower();
            const FloatNum xu = x.upper();
            const FloatNum yl = y.lower();
            const FloatNum yu = y.upper();
            const bool y_negative = yu < 0.0;
            if (xu < 0.0)
                return y_negative ? Interval::unchecked(r.div_down(xu, yl), r.div_up(xl, yu))
                                  : Interval::unchecked(r.div_down(xl, yl), r.div_up(xu, yu));
            if (xl < 0.0)
                return y_negative ? Interval::unchecked(r.div_down(xu, yu), r.div_up(xl, yu))
                                  : Interval::unchecked(r.div_down(xl, yl), r.div_up(xu, yl));
            return y_negative ? Interval::unchecked(r.div_down(xu, yu), r.div_up(xl, yl))
                              : Interval::unchecked(r.div_down(xl, yu), r.div_up(xu, yl));
        }
        /**
         * \brief Return \a x divided by \a y
         *
         * A divisor touching zero at one bound sends the quotient to infinity
         * on the side given by the dividend's sign, and a divisor with zero
         * inside gives the whole line. Over such divisors, a dividend of
         * \f$[0,0]\f$ stays itself, keeping the signs of its zeros.
         */
        inline Interval
        div(RoundingBase& r, const Interval& x, const Interval& y) {
            const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
            const FloatNum xl = x.lower();
            const FloatNum xu = x.upper();
            const bool x_zero = (xl == 0.0) && (xu == 0.0);
            switch (zero_position(y)) {
            case ZeroPosition::OUTSIDE:
                return div_sign_definite(r, x, y);
            case ZeroPosition::BOTH_BOUNDS:
                throw_empty();
            case ZeroPosition::LOWER_BOUND:
                if (x_zero)
                    return x;
                if (xu < 0.0)
                    return Interval::unchecked(-inf, r.div_up(xu, y.upper()));
                if (xl < 0.0)
                    return whole();
                return Interval::unchecked(r.div_down(xl, y.upper()), inf);
            case ZeroPosition::UPPER_BOUND:
                if (x_zero)
                    return x;
                if (xu < 0.0)
                    return Interval::unchecked(r.div_down(xu, y.lower()), inf);
                if (xl < 0.0)
                    return whole();
                return Interval::unchecked(-inf, r.div_up(xl, y.lower()));
            case ZeroPosition::INTERIOR:
                return x_zero ? x : whole();
            }
            GECODE_NEVER;
            return x;
        }
        /**
         * \brief Return \a n divided by \a y
         *
         * The cases follow the division of intervals, except that a zero
         * dividend over a divisor containing zero gives \f$[0,0]\f$.
         */
        inline Interval
        div(RoundingBase& r, FloatNum n, const Interval& y) {
            const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
            const FloatNum yl = y.lower();
            const FloatNum yu = y.upper();
            switch (zero_position(y)) {
            case ZeroPosition::OUTSIDE:
                return (n < 0.0) ? Interval::unchecked(r.div_down(n, yl), r.div_up(n, yu))
                                 : Interval::unchecked(r.div_down(n, yu), r.div_up(n, yl));
            case ZeroPosition::BOTH_BOUNDS:
                throw_empty();
            case ZeroPosition::LOWER_BOUND:
                if (n == 0.0)
                    return Interval();
                return (n < 0.0) ? Interval::unchecked(-inf, r.div_up(n, yu))
                                 : Interval::unchecked(r.div_down(n, yu), inf);
            case ZeroPosition::UPPER_BOUND:
                if (n == 0.0)
                    return Interval();
                return (n < 0.0) ? Interval::unchecked(r.div_down(n, yl), inf)
                                 : Interval::unchecked(-inf, r.div_up(n, yl));
            case ZeroPosition::INTERIOR:
                return (n == 0.0) ? Interval() : whole();
            }
            GECODE_NEVER;
            return whole();
        }
        /// Return \a x divided by \a n
        forceinline Interval
        div(RoundingBase& r, const Interval& x, FloatNum n) {
            if (n == 0.0)
                throw_empty();
            if (n < 0.0)
                return Interval::unchecked(r.div_down(x.upper(), n), r.div_up(x.lower(), n));
            return Interval::unchecked(r.div_down(x.lower(), n), r.div_up(x.upper(), n));
        }

        /**
         * \brief Return a lower bound of \f$a^n\f$ for \f$a\geq 0\f$ and \f$n>0\f$
         *
         * Binary powering from the least significant bit of \a n: \c b runs
         * through \f$a^{2^i}\f$ and \c p collects the factors whose bit is set.
         */
        inline FloatNum
        pow_down(RoundingBase& r, FloatNum a, unsigned int n) {
            FloatNum b = a;
            FloatNum p = (n % 2 != 0) ? a : 1.0;
            for (n /= 2; n > 0; n /= 2) {
                b = r.mul_down(b, b);
                if (n % 2 != 0)
                    p = r.mul_down(b, p);
            }
            return p;
        }
        /// Return an upper bound of \f$a^n\f$ for \f$a\geq 0\f$ and \f$n>0\f$ (see pow_down())
        inline FloatNum
        pow_up(RoundingBase& r, FloatNum a, unsigned int n) {
            FloatNum b = a;
            FloatNum p = (n % 2 != 0) ? a : 1.0;
            for (n /= 2; n > 0; n /= 2) {
                b = r.mul_up(b, b);
                if (n % 2 != 0)
                    p = r.mul_up(b, p);
            }
            return p;
        }
        /**
         * \brief Return \f$x^n\f$ for \f$n>0\f$
         *
         * On nonnegative intervals the power is monotone. A negative interval
         * is mirrored to its absolute values, and the result mirrored back for
         * odd \a n. An interval with both signs reaches its extremes at the
         * bounds, and its even powers start at zero.
         */
        inline Interval
        positive_pow(RoundingBase& r, const Interval& x, unsigned int n) {
            const FloatNum xl = x.lower();
            const FloatNum xu = x.upper();
            const bool odd = n % 2 != 0;
            if (xu < 0.0) {
                const Interval m = Interval::unchecked(pow_down(r, -xu, n), pow_up(r, -xl, n));
                return odd ? -m : m;
            }
            if (xl < 0.0)
                return odd ? Interval::unchecked(-pow_up(r, -xl, n), pow_up(r, xu, n))
                           : Interval::unchecked(0.0, pow_up(r, std::max(-xl, xu), n));
            return Interval::unchecked(pow_down(r, xl, n), pow_up(r, xu, n));
        }
        /**
         * \brief Return \f$x^n\f$
         *
         * A negative exponent divides 1 by the positive power. Its magnitude
         * is computed in unsigned arithmetic, so \a n may be \c INT_MIN.
         */
        inline Interval
        pow(RoundingBase& r, const Interval& x, int n) {
            if (n == 0) {
                if ((x.lower() == 0.0) && (x.upper() == 0.0))
                    throw_empty();
                return Interval(1.0);
            }
            if (n < 0)
                return div(r, 1.0, positive_pow(r, x, 0U - static_cast<unsigned int>(n)));
            return positive_pow(r, x, static_cast<unsigned int>(n));
        }

        /**
         * \brief Return an enclosure of \f$a^{1/k}\f$ for \f$a>1\f$ and \f$k>1\f$
         *
         * Interval Newton iteration for \f$f(t)=t^k-a\f$ on
         * \f$Y_0=[1,a]\f$: \f$Y_{i+1}=Y_i\cap(m-f(m)/f'(Y_i))\f$ with
         * \f$m\f$ the median of \f$Y_i\f$ and \f$f'(Y)=kY^{k-1}\f$, until
         * the enclosure stops shrinking. An empty intersection throws, which
         * ends the iteration for NaN.
         */
        inline Interval
        newton_root(RoundingBase& r, FloatNum a, int k) {
            Interval y = Interval::unchecked(1.0, a);
            while (true) {
                const FloatNum m = r.median(y.lower(), y.upper());
                const Interval f = sub(r, pow(r, Interval(m), k), a);
                const Interval derivative = mul(r, static_cast<FloatNum>(k), pow(r, y, k - 1));
                const Interval next = intersect(y, sub(r, m, div(r, f, derivative)));
                if (equal(y, next))
                    return y;
                y = next;
            }
        }
        /**
         * \brief Return a lower bound of \f$a^{1/k}\f$ for \f$a>0\f$ and \f$k>1\f$
         *
         * Below 1 the root is the reciprocal of the root of \f$1/a\f$.
         */
        inline FloatNum
        root_down(RoundingBase& r, FloatNum a, int k) {
            if (a > 1.0)
                return newton_root(r, a, k).lower();
            if (a == 1.0)
                return 1.0;
            return r.div_down(1.0, newton_root(r, r.div_up(1.0, a), k).upper());
        }
        /// Return an upper bound of \f$a^{1/k}\f$ for \f$a>0\f$ and \f$k>1\f$ (see root_down())
        inline FloatNum
        root_up(RoundingBase& r, FloatNum a, int k) {
            if (a > 1.0)
                return newton_root(r, a, k).upper();
            if (a == 1.0)
                return 1.0;
            return r.div_up(1.0, newton_root(r, r.div_down(1.0, a), k).lower());
        }

        /**
         * \brief Return an enclosure of \f$x-qy\f$ for some integer \f$q\f$
         *
         * \f$q\f$ is the rounded-down quotient of the lower bound of \a x and
         * the bound of \a y that makes it smallest for a positive \a y. The
         * result is \a x shifted by whole periods of \a y, not a tight
         * remainder.
         */
        forceinline Interval
        fmod(RoundingBase& r, const Interval& x, const Interval& y) {
            const FloatNum period = (x.lower() < 0.0) ? y.lower() : y.upper();
            const FloatNum q = r.int_down(r.div_down(x.lower(), period));
            return sub(r, x, mul(r, q, y));
        }
        /// Return an enclosure of \f$x-qn\f$ for some integer \f$q\f$ (see fmod())
        forceinline Interval
        fmod(RoundingBase& r, const Interval& x, FloatNum n) {
            const FloatNum q = r.int_down(r.div_down(x.lower(), n));
            return sub(r, x, mul(r, q, Interval(n)));
        }
        /// Return an enclosure of \f$n-qy\f$ for some integer \f$q\f$ (see fmod())
        forceinline Interval
        fmod(RoundingBase& r, FloatNum n, const Interval& y) {
            const FloatNum period = (n < 0.0) ? y.lower() : y.upper();
            const FloatNum q = r.int_down(r.div_down(n, period));
            return sub(r, n, mul(r, q, y));
        }

        /// Return an upper bound of the width of \a x
        forceinline FloatNum
        width(RoundingBase& r, const Interval& x) {
            return r.sub_up(x.upper(), x.lower());
        }

#ifdef GECODE_HAS_MPFR
        /**
         * \brief Return \f$\cos x\f$
         *
         * \a x is shifted into \f$[0,2\pi]\f$. Cosine falls on \f$[0,\pi]\f$
         * and rises on \f$[\pi,2\pi]\f$, so an interval in \f$[0,\pi]\f$ maps
         * to its reversed bounds, and one that passes \f$\pi\f$ reaches -1
         * and has its maximum at the bound nearer to \f$0\f$ or \f$2\pi\f$.
         * An interval starting after \f$\pi\f$ uses
         * \f$\cos x=-\cos(x-\pi)\f$.
         */
        inline Interval
        cos(RoundingBase& r, const Interval& x) {
            const Interval range = Interval::unchecked(-1.0, 1.0);
            const Interval t = fmod(r, x, Interval::pi_twice());
            if (width(r, t) >= pi_twice_lower())
                return range;
            if (t.lower() >= pi_upper())
                return -cos(r, sub(r, t, Interval::pi()));
            if (t.upper() <= pi_lower())
                return Interval::unchecked(r.cos_down(t.upper()), r.cos_up(t.lower()));
            if (t.upper() <= pi_twice_lower())
                return Interval::unchecked(-1.0,
                                           r.cos_up(std::min(r.sub_down(pi_twice_lower(), t.upper()), t.lower())));
            return range;
        }
#endif

    } // namespace IntervalDetail

    /*
     * Update operators
     *
     */

    forceinline Interval&
    Interval::operator+=(const Interval& y) {
        Rounding r;
        return *this = IntervalDetail::add(r, *this, y);
    }
    forceinline Interval&
    Interval::operator-=(const Interval& y) {
        Rounding r;
        return *this = IntervalDetail::sub(r, *this, y);
    }
    forceinline Interval&
    Interval::operator*=(const Interval& y) {
        Rounding r;
        return *this = IntervalDetail::mul(r, *this, y);
    }
    forceinline Interval&
    Interval::operator/=(const Interval& y) {
        Rounding r;
        return *this = IntervalDetail::div(r, *this, y);
    }
    forceinline Interval&
    Interval::operator+=(FloatNum n) {
        // The interval bound is the left operand, unlike in x + n.
        Rounding r;
        return *this = unchecked(r.add_down(lb, n), r.add_up(ub, n));
    }
    forceinline Interval&
    Interval::operator-=(FloatNum n) {
        Rounding r;
        return *this = IntervalDetail::sub(r, *this, n);
    }
    forceinline Interval&
    Interval::operator*=(FloatNum n) {
        Rounding r;
        return *this = IntervalDetail::mul(r, n, *this);
    }
    forceinline Interval&
    Interval::operator/=(FloatNum n) {
        Rounding r;
        return *this = IntervalDetail::div(r, *this, n);
    }

    /*
     * Arithmetic
     *
     */

    /**
     * \brief Return \a x plus \a y
     * \relates Interval
     */
    forceinline Interval
    operator+(const Interval& x, const Interval& y) {
        Rounding r;
        return IntervalDetail::add(r, x, y);
    }
    /**
     * \brief Return \a x plus \a n
     * \relates Interval
     */
    forceinline Interval
    operator+(const Interval& x, FloatNum n) {
        Rounding r;
        return IntervalDetail::add(r, n, x);
    }
    /**
     * \brief Return \a n plus \a x
     * \relates Interval
     */
    forceinline Interval
    operator+(FloatNum n, const Interval& x) {
        Rounding r;
        return IntervalDetail::add(r, n, x);
    }
    /**
     * \brief Return \a x minus \a y
     * \relates Interval
     */
    forceinline Interval
    operator-(const Interval& x, const Interval& y) {
        Rounding r;
        return IntervalDetail::sub(r, x, y);
    }
    /**
     * \brief Return \a x minus \a n
     * \relates Interval
     */
    forceinline Interval
    operator-(const Interval& x, FloatNum n) {
        Rounding r;
        return IntervalDetail::sub(r, x, n);
    }
    /**
     * \brief Return \a n minus \a x
     * \relates Interval
     */
    forceinline Interval
    operator-(FloatNum n, const Interval& x) {
        Rounding r;
        return IntervalDetail::sub(r, n, x);
    }
    /**
     * \brief Return \a x times \a y
     * \relates Interval
     */
    forceinline Interval
    operator*(const Interval& x, const Interval& y) {
        Rounding r;
        return IntervalDetail::mul(r, x, y);
    }
    /**
     * \brief Return \a x times \a n
     * \relates Interval
     */
    forceinline Interval
    operator*(const Interval& x, FloatNum n) {
        Rounding r;
        return IntervalDetail::mul(r, n, x);
    }
    /**
     * \brief Return \a n times \a x
     * \relates Interval
     */
    forceinline Interval
    operator*(FloatNum n, const Interval& x) {
        Rounding r;
        return IntervalDetail::mul(r, n, x);
    }
    /**
     * \brief Return \a x divided by \a y, throwing if \a y is \f$[0,0]\f$
     * \relates Interval
     */
    forceinline Interval
    operator/(const Interval& x, const Interval& y) {
        Rounding r;
        return IntervalDetail::div(r, x, y);
    }
    /**
     * \brief Return \a x divided by \a n, throwing if \a n is 0
     * \relates Interval
     */
    forceinline Interval
    operator/(const Interval& x, FloatNum n) {
        Rounding r;
        return IntervalDetail::div(r, x, n);
    }
    /**
     * \brief Return \a n divided by \a y, throwing if \a y is \f$[0,0]\f$
     * \relates Interval
     */
    forceinline Interval
    operator/(FloatNum n, const Interval& y) {
        Rounding r;
        return IntervalDetail::div(r, n, y);
    }

    /**
     * \brief Return an upper bound of the width of \a x
     * \relates Interval
     */
    forceinline FloatNum
    width(const Interval& x) {
        Rounding r;
        return IntervalDetail::width(r, x);
    }

    /**
     * \brief Return \f$\sqrt{x}\f$, throwing if \a x has no nonnegative member
     * \relates Interval
     */
    inline Interval
    sqrt(const Interval& x) {
        if (x.upper() < 0.0)
            IntervalDetail::throw_empty();
        Rounding r;
        const FloatNum l = (x.lower() > 0.0) ? r.sqrt_down(x.lower()) : 0.0;
        return Interval::unchecked(l, r.sqrt_up(x.upper()));
    }
    /**
     * \brief Return \f$\{v^2 \mid v\in x\}\f$
     * \relates Interval
     */
    inline Interval
    square(const Interval& x) {
        const FloatNum xl = x.lower();
        const FloatNum xu = x.upper();
        Rounding r;
        if (xu < 0.0)
            return Interval::unchecked(r.mul_down(xu, xu), r.mul_up(xl, xl));
        if (xl > 0.0)
            return Interval::unchecked(r.mul_down(xl, xl), r.mul_up(xu, xu));
        return Interval::unchecked(0.0, (-xl > xu) ? r.mul_up(xl, xl) : r.mul_up(xu, xu));
    }
    /**
     * \brief Return \f$x^n\f$
     *
     * Throws for \f$[0,0]\f$ with \f$n\leq 0\f$. Any other interval to the
     * power 0 is \f$[1,1]\f$.
     * \relates Interval
     */
    forceinline Interval
    pow(const Interval& x, int n) {
        Rounding r;
        return IntervalDetail::pow(r, x, n);
    }
    /**
     * \brief Return the real \a k-th roots of the members of \a x, for \a k > 0
     *
     * Throws for even \a k when \a x has no nonnegative member.
     * \relates Interval
     */
    inline Interval
    nth_root(const Interval& x, int k) {
        assert(k > 0);
        if (k == 1)
            return x;
        Rounding r;
        const FloatNum xl = x.lower();
        const FloatNum xu = x.upper();
        const bool odd = k % 2 != 0;
        if (!(xu > 0.0)) {
            if (xu == 0.0) {
                if (!odd || (xl == 0.0))
                    return Interval();
                return Interval::unchecked(-IntervalDetail::root_up(r, -xl, k), 0.0);
            }
            if (!odd)
                IntervalDetail::throw_empty();
            return Interval::unchecked(-IntervalDetail::root_up(r, -xl, k), -IntervalDetail::root_down(r, -xu, k));
        }
        const FloatNum u = IntervalDetail::root_up(r, xu, k);
        if (xl > 0.0)
            return Interval::unchecked(IntervalDetail::root_down(r, xl, k), u);
        if (odd && (xl != 0.0))
            return Interval::unchecked(-IntervalDetail::root_up(r, -xl, k), u);
        return Interval::unchecked(0.0, u);
    }

    /**
     * \brief Return \a x shifted by a whole multiple of \a y (see IntervalDetail::fmod())
     * \relates Interval
     */
    forceinline Interval
    fmod(const Interval& x, const Interval& y) {
        Rounding r;
        return IntervalDetail::fmod(r, x, y);
    }
    /**
     * \brief Return \a x shifted by a whole multiple of \a n (see IntervalDetail::fmod())
     * \relates Interval
     */
    forceinline Interval
    fmod(const Interval& x, FloatNum n) {
        Rounding r;
        return IntervalDetail::fmod(r, x, n);
    }
    /**
     * \brief Return \a n shifted by a whole multiple of \a y (see IntervalDetail::fmod())
     * \relates Interval
     */
    forceinline Interval
    fmod(FloatNum n, const Interval& y) {
        Rounding r;
        return IntervalDetail::fmod(r, n, y);
    }

#ifdef GECODE_HAS_MPFR

    /*
     * Transcendental functions
     *
     * Monotone functions map the bounds. The others clamp their bounds to the
     * function's range at the ends of its domain.
     *
     */

    /**
     * \brief Return \f$\exp x\f$
     * \relates Interval
     */
    forceinline Interval
    exp(const Interval& x) {
        Rounding r;
        return Interval::unchecked(r.exp_down(x.lower()), r.exp_up(x.upper()));
    }
    /**
     * \brief Return \f$\log x\f$, throwing if \a x has no positive member
     * \relates Interval
     */
    inline Interval
    log(const Interval& x) {
        if (!(x.upper() > 0.0))
            IntervalDetail::throw_empty();
        Rounding r;
        const FloatNum l = (x.lower() > 0.0) ? r.log_down(x.lower()) : -std::numeric_limits<FloatNum>::infinity();
        return Interval::unchecked(l, r.log_up(x.upper()));
    }

    /**
     * \brief Return \f$\cos x\f$
     * \relates Interval
     */
    forceinline Interval
    cos(const Interval& x) {
        Rounding r;
        return IntervalDetail::cos(r, x);
    }
    /**
     * \brief Return \f$\sin x\f$, computed as \f$\cos(x-\pi/2)\f$
     * \relates Interval
     */
    forceinline Interval
    sin(const Interval& x) {
        Rounding r;
        return IntervalDetail::cos(r, IntervalDetail::sub(r, x, Interval::pi_half()));
    }
    /**
     * \brief Return \f$\tan x\f$
     *
     * \a x is shifted into \f$[-\pi/2,\pi/2]\f$, where tangent rises. An
     * interval that reaches a pole gives the whole line.
     * \relates Interval
     */
    inline Interval
    tan(const Interval& x) {
        Rounding r;
        Interval t = IntervalDetail::fmod(r, x, Interval::pi());
        if (t.lower() >= pi_half_lower())
            t = IntervalDetail::sub(r, t, Interval::pi());
        if ((t.lower() <= -pi_half_lower()) || (t.upper() >= pi_half_lower()))
            return IntervalDetail::whole();
        return Interval::unchecked(r.tan_down(t.lower()), r.tan_up(t.upper()));
    }
    /**
     * \brief Return \f$\arcsin x\f$, throwing if \a x misses \f$[-1,1]\f$
     * \relates Interval
     */
    inline Interval
    asin(const Interval& x) {
        if ((x.upper() < -1.0) || (x.lower() > 1.0))
            IntervalDetail::throw_empty();
        Rounding r;
        const FloatNum l = (x.lower() <= -1.0) ? -pi_half_upper() : r.asin_down(x.lower());
        const FloatNum u = (x.upper() >= 1.0) ? pi_half_upper() : r.asin_up(x.upper());
        return Interval::unchecked(l, u);
    }
    /**
     * \brief Return \f$\arccos x\f$, throwing if \a x misses \f$[-1,1]\f$
     * \relates Interval
     */
    inline Interval
    acos(const Interval& x) {
        if ((x.upper() < -1.0) || (x.lower() > 1.0))
            IntervalDetail::throw_empty();
        Rounding r;
        const FloatNum l = (x.upper() >= 1.0) ? 0.0 : r.acos_down(x.upper());
        const FloatNum u = (x.lower() <= -1.0) ? pi_upper() : r.acos_up(x.lower());
        return Interval::unchecked(l, u);
    }
    /**
     * \brief Return \f$\arctan x\f$
     * \relates Interval
     */
    forceinline Interval
    atan(const Interval& x) {
        Rounding r;
        return Interval::unchecked(r.atan_down(x.lower()), r.atan_up(x.upper()));
    }

    /**
     * \brief Return \f$\sinh x\f$
     * \relates Interval
     */
    forceinline Interval
    sinh(const Interval& x) {
        Rounding r;
        return Interval::unchecked(r.sinh_down(x.lower()), r.sinh_up(x.upper()));
    }
    /**
     * \brief Return \f$\cosh x\f$, which falls below 0 and rises above
     * \relates Interval
     */
    inline Interval
    cosh(const Interval& x) {
        const FloatNum xl = x.lower();
        const FloatNum xu = x.upper();
        Rounding r;
        if (xu < 0.0)
            return Interval::unchecked(r.cosh_down(xu), r.cosh_up(xl));
        if (!(xl < 0.0))
            return Interval::unchecked(r.cosh_down(xl), r.cosh_up(xu));
        return Interval::unchecked(1.0, r.cosh_up((-xl > xu) ? xl : xu));
    }
    /**
     * \brief Return \f$\tanh x\f$
     * \relates Interval
     */
    forceinline Interval
    tanh(const Interval& x) {
        Rounding r;
        return Interval::unchecked(r.tanh_down(x.lower()), r.tanh_up(x.upper()));
    }
    /**
     * \brief Return \f$\operatorname{arsinh} x\f$
     * \relates Interval
     */
    forceinline Interval
    asinh(const Interval& x) {
        Rounding r;
        return Interval::unchecked(r.asinh_down(x.lower()), r.asinh_up(x.upper()));
    }
    /**
     * \brief Return \f$\operatorname{arcosh} x\f$, throwing if \a x misses \f$[1,+\infty]\f$
     * \relates Interval
     */
    inline Interval
    acosh(const Interval& x) {
        if (x.upper() < 1.0)
            IntervalDetail::throw_empty();
        Rounding r;
        const FloatNum l = (x.lower() <= 1.0) ? 0.0 : r.acosh_down(x.lower());
        return Interval::unchecked(l, r.acosh_up(x.upper()));
    }
    /**
     * \brief Return \f$\operatorname{artanh} x\f$, throwing if \a x misses \f$[-1,1]\f$
     * \relates Interval
     */
    inline Interval
    atanh(const Interval& x) {
        if ((x.upper() < -1.0) || (x.lower() > 1.0))
            IntervalDetail::throw_empty();
        const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
        Rounding r;
        const FloatNum l = (x.lower() <= -1.0) ? -inf : r.atanh_down(x.lower());
        const FloatNum u = (x.upper() >= 1.0) ? inf : r.atanh_up(x.upper());
        return Interval::unchecked(l, u);
    }

#endif

}} // namespace Gecode::Float

// STATISTICS: float-var
