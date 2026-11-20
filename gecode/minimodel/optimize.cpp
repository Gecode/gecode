/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <gecode/minimodel.hh>

#include <cmath>

namespace Gecode {

    SpaceComparison
    compare_lex(const IntVarArgs& x, const IntVarArgs& y, Sense s) {
        if (x.size() != y.size())
            throw SpaceIncompatibleObjectives("compare_lex");
        for (int i = 0; i < x.size(); i++)
            if (x[i].val() != y[i].val())
                return compare_int(x[i].val(), y[i].val(), s);
        return SpaceComparison::Equivalent;
    }

    void
    IntMinimizeSpace::constrain(const Space& _best) {
        const IntMinimizeSpace* best = dynamic_cast<const IntMinimizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("IntMinimizeSpace::constrain");
        rel(*this, cost(), IRT_LT, best->cost().val());
    }

    SpaceComparison
    IntMinimizeSpace::compare(const Space& other) const {
        const IntMinimizeSpace* s = dynamic_cast<const IntMinimizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("IntMinimizeSpace::compare");
        return compare_int(cost().val(), s->cost().val(), Sense::Minimize);
    }

    void
    IntMaximizeSpace::constrain(const Space& _best) {
        const IntMaximizeSpace* best = dynamic_cast<const IntMaximizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("IntMaximizeSpace::constrain");
        rel(*this, cost(), IRT_GR, best->cost().val());
    }

    SpaceComparison
    IntMaximizeSpace::compare(const Space& other) const {
        const IntMaximizeSpace* s = dynamic_cast<const IntMaximizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("IntMaximizeSpace::compare");
        return compare_int(cost().val(), s->cost().val(), Sense::Maximize);
    }

    void
    IntLexMinimizeSpace::constrain(const Space& _best) {
        const IntLexMinimizeSpace* best = dynamic_cast<const IntLexMinimizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("IntLexMinimizeSpace::constrain");
        IntVarArgs cx(cost()), bx(best->cost());
        IntArgs bn(bx.size());
        for (int i = bn.size(); i--;)
            bn[i] = bx[i].val();
        rel(*this, cx, IRT_LT, bn);
    }

    SpaceComparison
    IntLexMinimizeSpace::compare(const Space& other) const {
        const IntLexMinimizeSpace* s = dynamic_cast<const IntLexMinimizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("IntLexMinimizeSpace::compare");
        return compare_lex(cost(), s->cost(), Sense::Minimize);
    }

    void
    IntLexMaximizeSpace::constrain(const Space& _best) {
        const IntLexMaximizeSpace* best = dynamic_cast<const IntLexMaximizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("IntLexMaximizeSpace::constrain");
        IntVarArgs cx(cost()), bx(best->cost());
        IntArgs bn(bx.size());
        for (int i = bn.size(); i--;)
            bn[i] = bx[i].val();
        rel(*this, cx, IRT_GR, bn);
    }

    SpaceComparison
    IntLexMaximizeSpace::compare(const Space& other) const {
        const IntLexMaximizeSpace* s = dynamic_cast<const IntLexMaximizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("IntLexMaximizeSpace::compare");
        return compare_lex(cost(), s->cost(), Sense::Maximize);
    }

#ifdef GECODE_HAS_FLOAT_VARS

    void
    FloatMinimizeSpace::constrain(const Space& _best) {
        const FloatMinimizeSpace* best = dynamic_cast<const FloatMinimizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("FloatMinimizeSpace::constrain");
        // An assigned cost is below the bound of the best solution minus the
        // step exactly when its upper bound is below the lower bound of that
        // difference. Posting this strict bound prunes the incumbent value
        // before the cost is assigned.
        FloatNum n = (FloatVal(best->cost().max()) - step).min();
        if (n <= Float::Limits::min)
            fail();
        else
            rel(*this, cost(), FRT_LQ, std::nextafter(n, Float::Limits::min));
    }

    SpaceComparison
    FloatMinimizeSpace::compare(const Space& other) const {
        const FloatMinimizeSpace* s = dynamic_cast<const FloatMinimizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("FloatMinimizeSpace::compare");
        if (step != s->step)
            throw SpaceIncompatibleObjectives("FloatMinimizeSpace::compare");
        return compare_float(cost().val().max(), s->cost().val().max(), Sense::Minimize);
    }

    void
    FloatMaximizeSpace::constrain(const Space& _best) {
        const FloatMaximizeSpace* best = dynamic_cast<const FloatMaximizeSpace*>(&_best);
        if (best == nullptr)
            throw DynamicCastFailed("FloatMaximizeSpace::constrain");
        FloatNum n = (FloatVal(best->cost().min()) + step).max();
        if (n >= Float::Limits::max)
            fail();
        else
            rel(*this, cost(), FRT_GQ, std::nextafter(n, Float::Limits::max));
    }

    SpaceComparison
    FloatMaximizeSpace::compare(const Space& other) const {
        const FloatMaximizeSpace* s = dynamic_cast<const FloatMaximizeSpace*>(&other);
        if (s == nullptr)
            throw DynamicCastFailed("FloatMaximizeSpace::compare");
        if (step != s->step)
            throw SpaceIncompatibleObjectives("FloatMaximizeSpace::compare");
        return compare_float(cost().val().min(), s->cost().val().min(), Sense::Maximize);
    }

#endif

} // namespace Gecode

// STATISTICS: minimodel-search
