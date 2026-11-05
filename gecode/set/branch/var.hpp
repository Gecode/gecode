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
    SetVarBranch::SetVarBranch()
        : s(SEL_NONE) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, BranchTbl t)
        : VarBranch<SetVar>(std::move(t)), s(s0) {}

    forceinline
    SetVarBranch::SetVarBranch(const Rnd& r)
        : VarBranch<SetVar>(r), s(SEL_RND) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, double d, BranchTbl t)
        : VarBranch<SetVar>(d, std::move(t)), s(s0) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, const SetAFC& a, BranchTbl t)
        : VarBranch<SetVar>(a, std::move(t)), s(s0) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, const SetAction& a, BranchTbl t)
        : VarBranch<SetVar>(a, std::move(t)), s(s0) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, const SetCHB& c, BranchTbl t)
        : VarBranch<SetVar>(c, std::move(t)), s(s0) {}

    forceinline
    SetVarBranch::SetVarBranch(Select s0, SetBranchMerit mf, BranchTbl t)
        : VarBranch<SetVar>(std::move(mf), std::move(t)), s(s0) {}

    forceinline SetVarBranch::Select
    SetVarBranch::select() const {
        return s;
    }

    forceinline void
    SetVarBranch::expand(Home home, const SetVarArgs& x) {
        switch (select()) {
        case SEL_AFC_MIN:
        case SEL_AFC_MAX:
        case SEL_AFC_SIZE_MIN:
        case SEL_AFC_SIZE_MAX:
            if (!_afc)
                _afc = SetAFC(home, x, decay());
            break;
        case SEL_ACTION_MIN:
        case SEL_ACTION_MAX:
        case SEL_ACTION_SIZE_MIN:
        case SEL_ACTION_SIZE_MAX:
            if (!_act)
                _act = SetAction(home, x, decay());
            break;
        case SEL_CHB_MIN:
        case SEL_CHB_MAX:
        case SEL_CHB_SIZE_MIN:
        case SEL_CHB_SIZE_MAX:
            if (!_chb)
                _chb = SetCHB(home, x);
            break;
        default:;
        }
    }

    inline SetVarBranch
    SET_VAR_NONE() {
        return SetVarBranch(SetVarBranch::SEL_NONE, nullptr);
    }

    inline SetVarBranch
    SET_VAR_RND(const Rnd& r) {
        return SetVarBranch(r);
    }

    inline SetVarBranch
    SET_VAR_MERIT_MIN(SetBranchMerit bm, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MERIT_MIN, std::move(bm), std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_MERIT_MAX(SetBranchMerit bm, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MERIT_MAX, std::move(bm), std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_DEGREE_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_DEGREE_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_DEGREE_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_DEGREE_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_MIN(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_MIN, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_MIN(const SetAFC& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_MIN, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_MAX(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_MAX, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_MAX(const SetAFC& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_MAX, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_MIN(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_MIN, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_MIN(const SetAction& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_MIN, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_MAX(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_MAX, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_MAX(const SetAction& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_MAX, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_MIN(const SetCHB& c, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_MIN, c, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_MAX(const SetCHB& c, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_MAX, c, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_MIN_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MIN_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_MIN_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MIN_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_MAX_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MAX_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_MAX_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_MAX_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_SIZE_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_SIZE_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_SIZE_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_SIZE_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_DEGREE_SIZE_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_DEGREE_SIZE_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_DEGREE_SIZE_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_DEGREE_SIZE_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_SIZE_MIN(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MIN, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_SIZE_MIN(const SetAFC& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MIN, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_SIZE_MAX(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MAX, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_AFC_SIZE_MAX(const SetAFC& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_AFC_SIZE_MAX, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_SIZE_MIN(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MIN, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_SIZE_MIN(const SetAction& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MIN, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_SIZE_MAX(double d, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MAX, d, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_ACTION_SIZE_MAX(const SetAction& a, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_ACTION_SIZE_MAX, a, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_SIZE_MIN(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MIN, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_SIZE_MIN(const SetCHB& c, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MIN, c, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_SIZE_MAX(BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MAX, std::move(tbl));
    }

    inline SetVarBranch
    SET_VAR_CHB_SIZE_MAX(const SetCHB& c, BranchTbl tbl) {
        return SetVarBranch(SetVarBranch::SEL_CHB_SIZE_MAX, c, std::move(tbl));
    }

} // namespace Gecode

// STATISTICS: set-branch
