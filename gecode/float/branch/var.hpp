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

#include <utility>

namespace Gecode {

    forceinline
    FloatVarBranch::FloatVarBranch(void)
        : s(SEL_NONE) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, BranchTbl t)
        : VarBranch<FloatVar>(std::move(t)), s(s0) {}

    forceinline
    FloatVarBranch::FloatVarBranch(const Rnd& r)
        : VarBranch<FloatVar>(r), s(SEL_RND) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, double d, BranchTbl t)
        : VarBranch<FloatVar>(d, std::move(t)), s(s0) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, const FloatAFC& a, BranchTbl t)
        : VarBranch<FloatVar>(a, std::move(t)), s(s0) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, const FloatAction& a, BranchTbl t)
        : VarBranch<FloatVar>(a, std::move(t)), s(s0) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, const FloatCHB& c, BranchTbl t)
        : VarBranch<FloatVar>(c, std::move(t)), s(s0) {}

    forceinline
    FloatVarBranch::FloatVarBranch(Select s0, FloatBranchMerit mf, BranchTbl t)
        : VarBranch<FloatVar>(std::move(mf), std::move(t)), s(s0) {}

    forceinline FloatVarBranch::Select
    FloatVarBranch::select(void) const {
        return s;
    }

    forceinline void
    FloatVarBranch::expand(Home home, const FloatVarArgs& x) {
        switch (select()) {
        case SEL_AFC_MIN:
        case SEL_AFC_MAX:
        case SEL_AFC_SIZE_MIN:
        case SEL_AFC_SIZE_MAX:
            if (!_afc)
                _afc = FloatAFC(home, x, decay());
            break;
        case SEL_ACTION_MIN:
        case SEL_ACTION_MAX:
        case SEL_ACTION_SIZE_MIN:
        case SEL_ACTION_SIZE_MAX:
            if (!_act)
                _act = FloatAction(home, x, decay());
            break;
        case SEL_CHB_MIN:
        case SEL_CHB_MAX:
        case SEL_CHB_SIZE_MIN:
        case SEL_CHB_SIZE_MAX:
            if (!_chb)
                _chb = FloatCHB(home, x);
            break;
        default:;
        }
    }

    inline FloatVarBranch
    FLOAT_VAR_NONE(void) {
        return FloatVarBranch(FloatVarBranch::SEL_NONE, nullptr);
    }

    inline FloatVarBranch
    FLOAT_VAR_MERIT_MIN(FloatBranchMerit bm, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MERIT_MIN, std::move(bm), std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_MERIT_MAX(FloatBranchMerit bm, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MERIT_MAX, std::move(bm), std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_RND(const Rnd& r) {
        return FloatVarBranch(r);
    }

    inline FloatVarBranch
    FLOAT_VAR_DEGREE_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_DEGREE_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_DEGREE_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_DEGREE_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_MIN(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_MIN, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_MIN(const FloatAFC& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_MIN, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_MAX(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_MAX, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_MAX(const FloatAFC& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_MAX, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_MIN(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_MIN, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_MIN(const FloatAction& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_MIN, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_MAX(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_MAX, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_MAX(const FloatAction& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_MAX, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_MIN(const FloatCHB& c, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_MIN, c, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_MAX(const FloatCHB& c, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_MAX, c, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_MIN_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MIN_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_MIN_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MIN_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_MAX_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MAX_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_MAX_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_MAX_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_SIZE_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_SIZE_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_SIZE_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_SIZE_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_DEGREE_SIZE_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_DEGREE_SIZE_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_DEGREE_SIZE_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_DEGREE_SIZE_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_SIZE_MIN(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_SIZE_MIN, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_SIZE_MIN(const FloatAFC& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_SIZE_MIN, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_SIZE_MAX(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_SIZE_MAX, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_AFC_SIZE_MAX(const FloatAFC& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_AFC_SIZE_MAX, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_SIZE_MIN(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_SIZE_MIN, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_SIZE_MIN(const FloatAction& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_SIZE_MIN, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_SIZE_MAX(double d, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_SIZE_MAX, d, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_ACTION_SIZE_MAX(const FloatAction& a, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_ACTION_SIZE_MAX, a, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_SIZE_MIN(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_SIZE_MIN, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_SIZE_MIN(const FloatCHB& c, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_SIZE_MIN, c, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_SIZE_MAX(BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_SIZE_MAX, std::move(tbl));
    }

    inline FloatVarBranch
    FLOAT_VAR_CHB_SIZE_MAX(const FloatCHB& c, BranchTbl tbl) {
        return FloatVarBranch(FloatVarBranch::SEL_CHB_SIZE_MAX, c, std::move(tbl));
    }

} // namespace Gecode

// STATISTICS: float-branch
