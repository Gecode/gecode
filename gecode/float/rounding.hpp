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

namespace Gecode::Float {

    /*
     * Rounding mode control
     *
     */

#ifdef GECODE_FLOAT_MODE_CONTROL87

    /**
     * \brief Control word settings that RoundingBase writes with `_control87`
     *
     * Gecode writes the exception masks and the rounding control, and on x86
     * also the precision and infinity control, which x64 does not support.
     * Besides the rounding direction, it masks all exceptions and, on x86,
     * selects 64-bit precision and affine infinity.
     */
    namespace Control87 {
#ifdef _M_IX86
        /// Fields that Gecode writes
        constexpr unsigned int fields = _MCW_EM | _MCW_RC | _MCW_PC | _MCW_IC;
        /// Settings of those fields besides the rounding direction
        constexpr unsigned int base_settings = _MCW_EM | _PC_64 | _IC_AFFINE;
#else
        /// Fields that Gecode writes
        constexpr unsigned int fields = _MCW_EM | _MCW_RC;
        /// Settings of those fields besides the rounding direction
        constexpr unsigned int base_settings = _MCW_EM;
#endif
    } // namespace Control87

    forceinline RoundingBase::Mode
    RoundingBase::current_mode() {
        return _control87(0, 0);
    }
    forceinline void
    RoundingBase::restore_mode(Mode m) {
        _control87(m, Control87::fields);
    }
    forceinline void
    RoundingBase::upward() {
        _control87(Control87::base_settings | _RC_UP, Control87::fields);
    }
    forceinline void
    RoundingBase::downward() {
        _control87(Control87::base_settings | _RC_DOWN, Control87::fields);
    }
    forceinline void
    RoundingBase::to_nearest() {
        _control87(Control87::base_settings | _RC_NEAR, Control87::fields);
    }

#else

    forceinline RoundingBase::Mode
    RoundingBase::current_mode() {
        return std::fegetround();
    }
    forceinline void
    RoundingBase::restore_mode(Mode m) {
        std::fesetround(m);
    }
    forceinline void
    RoundingBase::upward() {
        std::fesetround(FE_UPWARD);
    }
    forceinline void
    RoundingBase::downward() {
        std::fesetround(FE_DOWNWARD);
    }
    forceinline void
    RoundingBase::to_nearest() {
        std::fesetround(FE_TONEAREST);
    }

#endif

    forceinline void
    RoundingBase::init() {
#ifndef GECODE_FLOAT_DIRECTED_PER_OPERATION
        upward();
#endif
    }

    forceinline
    Rounding::Rounding()
        : saved(current_mode()) {
        init();
    }
    forceinline Rounding::~Rounding() {
        restore_mode(saved);
    }

    forceinline FloatNum
    RoundingBase::nearest_median(FloatNum x, FloatNum y) {
        to_nearest();
        const FloatNum m = barrier((barrier(x) + barrier(y)) / 2.0);
#ifndef GECODE_FLOAT_DIRECTED_PER_OPERATION
        upward();
#endif
        return m;
    }

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
            return std::ldexp(nearest_median(l / 2.0, u / 2.0), 1);
        return nearest_median(l, u);
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

#ifdef GECODE_FLOAT_DIRECTED_PER_OPERATION

    // Each operation sets its direction.
    forceinline FloatNum
    RoundingBase::add_down(FloatNum x, FloatNum y) {
        downward();
        return barrier(barrier(x) + barrier(y));
    }
    forceinline FloatNum
    RoundingBase::add_up(FloatNum x, FloatNum y) {
        upward();
        return barrier(barrier(x) + barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sub_down(FloatNum x, FloatNum y) {
        downward();
        return barrier(barrier(x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sub_up(FloatNum x, FloatNum y) {
        upward();
        return barrier(barrier(x) - barrier(y));
    }
    forceinline FloatNum
    RoundingBase::mul_down(FloatNum x, FloatNum y) {
        downward();
        return barrier(barrier(x) * barrier(y));
    }
    forceinline FloatNum
    RoundingBase::mul_up(FloatNum x, FloatNum y) {
        upward();
        return barrier(barrier(x) * barrier(y));
    }
    forceinline FloatNum
    RoundingBase::div_down(FloatNum x, FloatNum y) {
        downward();
        return barrier(barrier(x) / barrier(y));
    }
    forceinline FloatNum
    RoundingBase::div_up(FloatNum x, FloatNum y) {
        upward();
        return barrier(barrier(x) / barrier(y));
    }
    forceinline FloatNum
    RoundingBase::sqrt_down(FloatNum x) {
        downward();
        return barrier(std::sqrt(barrier(x)));
    }
    forceinline FloatNum
    RoundingBase::sqrt_up(FloatNum x) {
        upward();
        return barrier(std::sqrt(barrier(x)));
    }

#else

    // The mode stays upward.
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
        downward();
        const FloatNum r = barrier(std::sqrt(barrier(x)));
        upward();
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

} // namespace Gecode::Float

// STATISTICS: float-var
