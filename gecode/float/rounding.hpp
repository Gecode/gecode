/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode { namespace Float {

    forceinline FloatNum
    RoundingBase::median(FloatNum l, FloatNum u) {
        const FloatNum inf = std::numeric_limits<FloatNum>::infinity();
        const FloatNum max = std::numeric_limits<FloatNum>::max();
        if (l == -inf) {
            if (u == -inf)
                return l;
            // Preserve the extended endpoint for semi-infinite intervals.
            return (u == inf) ? 0.0 : l;
        }
        if (u == inf)
            return u;

        const FloatNum h = max / 2.0;
        if ((u > h) || (l < -h))
            return std::ldexp(RoundingControl::median(l / 2.0, u / 2.0), 1);
        return RoundingControl::median(l, u);
    }

    forceinline FloatNum
    RoundingBase::barrier(FloatNum x) {
        // See the RoundingBase class comment for what this relies on.
#if (defined(__GNUC__) || defined(__clang__)) && defined(__aarch64__)
        __asm__ __volatile__("" : "+w"(x));
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
        __asm__ __volatile__("" : "+x"(x));
#else
        volatile FloatNum v = x;
        x = v;
#endif
        return x;
    }

#if defined(_MSC_VER) && (defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP > 0)))

    // RoundingControl is rounded_arith_std: each operation sets its direction.
    forceinline FloatNum
    RoundingBase::add_down(FloatNum x, FloatNum y) {
        RoundingControl::downward();
        return barrier(barrier(x) + barrier(y));
    }
    forceinline FloatNum
    RoundingBase::add_up(FloatNum x, FloatNum y) {
        RoundingControl::upward();
        return barrier(barrier(x) + barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sub_down(FloatNum x, FloatNum y) {
        RoundingControl::downward();
        return barrier(barrier(x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sub_up(FloatNum x, FloatNum y) {
        RoundingControl::upward();
        return barrier(barrier(x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::mul_down(FloatNum x, FloatNum y) {
        RoundingControl::downward();
        return barrier(barrier(x) * barrier(y));
    }
    forceinline FloatNum
    RoundingBase::mul_up(FloatNum x, FloatNum y) {
        RoundingControl::upward();
        return barrier(barrier(x) * barrier(y));
    }
    forceinline FloatNum
    RoundingBase::div_down(FloatNum x, FloatNum y) {
        RoundingControl::downward();
        return barrier(barrier(x) / barrier(y));
    }
    forceinline FloatNum
    RoundingBase::div_up(FloatNum x, FloatNum y) {
        RoundingControl::upward();
        return barrier(barrier(x) / barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sqrt_down(FloatNum x) {
        RoundingControl::downward();
        return barrier(std::sqrt(barrier(x)));
    }
    forceinline FloatNum
    RoundingBase::sqrt_up(FloatNum x) {
        RoundingControl::upward();
        return barrier(std::sqrt(barrier(x)));
    }

#else

    // RoundingControl is rounded_arith_opp: the mode stays upward.
    forceinline FloatNum
    RoundingBase::add_down(FloatNum x, FloatNum y) {
        return -barrier(barrier(-x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::add_up(FloatNum x, FloatNum y) {
        return barrier(barrier(x) + barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sub_down(FloatNum x, FloatNum y) {
        return -barrier(barrier(y) - barrier(x));
    }
    forceinline FloatNum
    RoundingBase::sub_up(FloatNum x, FloatNum y) {
        return barrier(barrier(x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::mul_down(FloatNum x, FloatNum y) {
        return -barrier(barrier(x) * barrier(-y));
    }
    forceinline FloatNum
    RoundingBase::mul_up(FloatNum x, FloatNum y) {
        return barrier(barrier(x) * barrier(y));
    }
    forceinline FloatNum
    RoundingBase::div_down(FloatNum x, FloatNum y) {
        return -barrier(barrier(x) / barrier(-y));
    }
    forceinline FloatNum
    RoundingBase::div_up(FloatNum x, FloatNum y) {
        return barrier(barrier(x) / barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sqrt_down(FloatNum x) {
        RoundingControl::downward();
        const FloatNum r = barrier(std::sqrt(barrier(x)));
        RoundingControl::upward();
        return r;
    }
    forceinline FloatNum
    RoundingBase::sqrt_up(FloatNum x) {
        return barrier(std::sqrt(barrier(x)));
    }

#endif

    forceinline FloatNum
    RoundingBase::int_down(FloatNum x) {
        return std::floor(x);
    }
    forceinline FloatNum
    RoundingBase::int_up(FloatNum x) {
        return std::ceil(x);
    }

}} // namespace Gecode::Float

// STATISTICS: float-var
