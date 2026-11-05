/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

namespace Gecode { namespace FlatZinc {

    forceinline
    IntBoolVarBranch::IntBoolVarBranch(Select s0, double d)
        : VarBranch<IntVar>(d, nullptr), s(s0) {}

    forceinline
    IntBoolVarBranch::IntBoolVarBranch(Select s0, const IntAFC& i, const BoolAFC& b)
        : s(s0), int_afc_data(i), bool_afc_data(b) {}

    forceinline
    IntBoolVarBranch::IntBoolVarBranch(Select s0, const IntAction& i, const BoolAction& b)
        : s(s0), int_action_data(i), bool_action_data(b) {}

    forceinline
    IntBoolVarBranch::IntBoolVarBranch(Select s0, const IntCHB& i, const BoolCHB& b)
        : s(s0), int_chb_data(i), bool_chb_data(b) {}

    forceinline IntBoolVarBranch::Select
    IntBoolVarBranch::select() const {
        return s;
    }

    forceinline IntAFC
    IntBoolVarBranch::int_afc() const {
        return int_afc_data;
    }
    forceinline BoolAFC
    IntBoolVarBranch::bool_afc() const {
        return bool_afc_data;
    }

    forceinline IntAction
    IntBoolVarBranch::int_action() const {
        return int_action_data;
    }
    forceinline BoolAction
    IntBoolVarBranch::bool_action() const {
        return bool_action_data;
    }

    forceinline IntCHB
    IntBoolVarBranch::int_chb() const {
        return int_chb_data;
    }
    forceinline BoolCHB
    IntBoolVarBranch::bool_chb() const {
        return bool_chb_data;
    }
    forceinline void
    IntBoolVarBranch::expand(Home home, const IntVarArgs& x, const BoolVarArgs& y) {
        switch (select()) {
        case IntBoolVarBranch::SEL_AFC_MAX:
        case IntBoolVarBranch::SEL_AFC_SIZE_MAX:
            if (!int_afc_data)
                int_afc_data = IntAFC(home, x, decay());
            if (!bool_afc_data)
                bool_afc_data = BoolAFC(home, y, decay());
            break;
        case IntBoolVarBranch::SEL_ACTION_MAX:
        case IntBoolVarBranch::SEL_ACTION_SIZE_MAX:
            if (!int_action_data)
                int_action_data = IntAction(home, x, decay());
            if (!bool_action_data)
                bool_action_data = BoolAction(home, y, decay());
            break;
        case IntBoolVarBranch::SEL_CHB_MAX:
        case IntBoolVarBranch::SEL_CHB_SIZE_MAX:
            if (!int_chb_data)
                int_chb_data = IntCHB(home, x);
            if (!bool_chb_data)
                bool_chb_data = BoolCHB(home, y);
            break;
        default:;
        }
    }

    inline IntBoolVarBranch
    int_bool_var_afc_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_afc_max(const IntAFC& ia, const BoolAFC& ba) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_MAX, ia, ba);
    }
    inline IntBoolVarBranch
    int_bool_var_action_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_action_max(const IntAction& ia, const BoolAction& ba) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_MAX, ia, ba);
    }
    inline IntBoolVarBranch
    int_bool_var_chb_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_chb_max(const IntCHB& ic, const BoolCHB& bc) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_MAX, ic, bc);
    }
    inline IntBoolVarBranch
    int_bool_var_afc_size_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_SIZE_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_afc_size_max(const IntAFC& ia, const BoolAFC& ba) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_AFC_SIZE_MAX, ia, ba);
    }
    inline IntBoolVarBranch
    int_bool_var_action_size_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_SIZE_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_action_size_max(const IntAction& ia, const BoolAction& ba) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_ACTION_SIZE_MAX, ia, ba);
    }
    inline IntBoolVarBranch
    int_bool_var_chb_size_max(double d) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_SIZE_MAX, d);
    }
    inline IntBoolVarBranch
    int_bool_var_chb_size_max(const IntCHB& ic, const BoolCHB& bc) {
        return IntBoolVarBranch(IntBoolVarBranch::SEL_CHB_SIZE_MAX, ic, bc);
    }

    forceinline
    MeritMaxAFC::MeritMaxAFC(Space&, const IntBoolVarBranch& ibvb)
        : int_afc_data(ibvb.int_afc()), bool_afc_data(ibvb.bool_afc()) {}
    forceinline
    MeritMaxAFC::MeritMaxAFC(Space&, MeritMaxAFC& m)
        : int_afc_data(m.int_afc_data), bool_afc_data(m.bool_afc_data) {}
    forceinline double
    MeritMaxAFC::operator()(Int::IntView x, int) const {
        return x.afc();
    }
    forceinline double
    MeritMaxAFC::operator()(Int::BoolView x, int) const {
        return x.afc();
    }
    forceinline void
    MeritMaxAFC::dispose() {
        int_afc_data.~IntAFC();
        bool_afc_data.~BoolAFC();
    }

    forceinline
    MeritMaxAFCSize::MeritMaxAFCSize(Space&, const IntBoolVarBranch& ibvb)
        : int_afc_data(ibvb.int_afc()), bool_afc_data(ibvb.bool_afc()) {}
    forceinline
    MeritMaxAFCSize::MeritMaxAFCSize(Space&, MeritMaxAFCSize& m)
        : int_afc_data(m.int_afc_data), bool_afc_data(m.bool_afc_data) {}
    forceinline double
    MeritMaxAFCSize::operator()(Int::IntView x, int) const {
        return x.afc() / x.size();
    }
    forceinline double
    MeritMaxAFCSize::operator()(Int::BoolView x, int) const {
        return x.afc() / 2.0;
    }
    forceinline void
    MeritMaxAFCSize::dispose() {
        int_afc_data.~IntAFC();
        bool_afc_data.~BoolAFC();
    }

    forceinline
    MeritMaxAction::MeritMaxAction(Space&, const IntBoolVarBranch& ibvb)
        : int_action_data(ibvb.int_action()), bool_action_data(ibvb.bool_action()) {}
    forceinline
    MeritMaxAction::MeritMaxAction(Space&, MeritMaxAction& m)
        : int_action_data(m.int_action_data), bool_action_data(m.bool_action_data) {}
    forceinline double
    MeritMaxAction::operator()(Int::IntView, int i) const {
        return int_action_data[i];
    }
    forceinline double
    MeritMaxAction::operator()(Int::BoolView, int i) const {
        return bool_action_data[i];
    }
    forceinline void
    MeritMaxAction::dispose() {
        int_action_data.~IntAction();
        bool_action_data.~BoolAction();
    }

    forceinline
    MeritMaxActionSize::MeritMaxActionSize(Space&, const IntBoolVarBranch& ibvb)
        : int_action_data(ibvb.int_action()), bool_action_data(ibvb.bool_action()) {}
    forceinline
    MeritMaxActionSize::MeritMaxActionSize(Space&, MeritMaxActionSize& m)
        : int_action_data(m.int_action_data), bool_action_data(m.bool_action_data) {}
    forceinline double
    MeritMaxActionSize::operator()(Int::IntView x, int i) const {
        return int_action_data[i] / x.size();
    }
    forceinline double
    MeritMaxActionSize::operator()(Int::BoolView, int i) const {
        return bool_action_data[i] / 2.0;
    }
    forceinline void
    MeritMaxActionSize::dispose() {
        int_action_data.~IntAction();
        bool_action_data.~BoolAction();
    }

    forceinline
    MeritMaxCHB::MeritMaxCHB(Space&, const IntBoolVarBranch& ibvb)
        : int_chb_data(ibvb.int_chb()), bool_chb_data(ibvb.bool_chb()) {}
    forceinline
    MeritMaxCHB::MeritMaxCHB(Space&, MeritMaxCHB& m)
        : int_chb_data(m.int_chb_data), bool_chb_data(m.bool_chb_data) {}
    forceinline double
    MeritMaxCHB::operator()(Int::IntView, int i) const {
        return int_chb_data[i];
    }
    forceinline double
    MeritMaxCHB::operator()(Int::BoolView, int i) const {
        return bool_chb_data[i];
    }
    forceinline void
    MeritMaxCHB::dispose() {
        int_chb_data.~IntCHB();
        bool_chb_data.~BoolCHB();
    }

    forceinline
    MeritMaxCHBSize::MeritMaxCHBSize(Space&, const IntBoolVarBranch& ibvb)
        : int_chb_data(ibvb.int_chb()), bool_chb_data(ibvb.bool_chb()) {}
    forceinline
    MeritMaxCHBSize::MeritMaxCHBSize(Space&, MeritMaxCHBSize& m)
        : int_chb_data(m.int_chb_data), bool_chb_data(m.bool_chb_data) {}
    forceinline double
    MeritMaxCHBSize::operator()(Int::IntView x, int i) const {
        return int_chb_data[i] / x.size();
    }
    forceinline double
    MeritMaxCHBSize::operator()(Int::BoolView, int i) const {
        return bool_chb_data[i] / 2.0;
    }
    forceinline void
    MeritMaxCHBSize::dispose() {
        int_chb_data.~IntCHB();
        bool_chb_data.~BoolCHB();
    }

    forceinline
    PosIntChoice::PosIntChoice(const Brancher& b, unsigned int a, int p, int n)
        : Choice(b, a), _pos(p), _val(n) {}
    forceinline int
    PosIntChoice::pos() const {
        return _pos;
    }
    forceinline int
    PosIntChoice::val() const {
        return _val;
    }

    forceinline
    IntBoolBrancherBase::IntBoolBrancherBase(Home home, ViewArray<Int::IntView> x0, ViewArray<Int::BoolView> y0,
                                             ValSelCommitBase<Int::IntView, int>* xvsc0,
                                             ValSelCommitBase<Int::BoolView, int>* yvsc0)
        : Brancher(home), x(x0), y(y0), start(0), xvsc(xvsc0), yvsc(yvsc0) {
        home.notice(*this, AP_DISPOSE, true);
    }

    forceinline
    IntBoolBrancherBase::IntBoolBrancherBase(Space& home, IntBoolBrancherBase& b)
        : Brancher(home, b), start(b.start), xvsc(b.xvsc->copy(home)), yvsc(b.yvsc->copy(home)) {
        x.update(home, b.x);
        y.update(home, b.y);
    }

    forceinline size_t
    IntBoolBrancherBase::dispose(Space& home) {
        home.ignore(*this, AP_DISPOSE, true);
        xvsc->dispose(home);
        yvsc->dispose(home);
        (void)Brancher::dispose(home);
        return sizeof(IntBoolBrancherBase);
    }

    template <class Merit>
    forceinline
    IntBoolBrancher<Merit>::IntBoolBrancher(Home home, ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y, Merit& m,
                                            ValSelCommitBase<Int::IntView, int>* xvsc,
                                            ValSelCommitBase<Int::BoolView, int>* yvsc)
        : IntBoolBrancherBase(home, x, y, xvsc, yvsc), merit(m) {}

    template <class Merit>
    forceinline void
    IntBoolBrancher<Merit>::post(Home home, ViewArray<Int::IntView> x, ViewArray<Int::BoolView> y, Merit& m,
                                 ValSelCommitBase<Int::IntView, int>* xvsc,
                                 ValSelCommitBase<Int::BoolView, int>* yvsc) {
        (void)new (home) IntBoolBrancher<Merit>(home, x, y, m, xvsc, yvsc);
    }

    template <class Merit>
    forceinline
    IntBoolBrancher<Merit>::IntBoolBrancher(Space& home, IntBoolBrancher& b)
        : IntBoolBrancherBase(home, b), merit(home, b.merit) {}

    template <class Merit>
    Actor*
    IntBoolBrancher<Merit>::copy(Space& home) {
        return new (home) IntBoolBrancher<Merit>(home, *this);
    }

    template <class Merit>
    const Choice*
    IntBoolBrancher<Merit>::choice(Space& home) {
        int p = start;
        double m;
        if (p < x.size()) {
            assert(!x[p].assigned());
            m = merit(x[p], p);
            for (int i = p + 1; i < x.size(); i++)
                if (!x[i].assigned()) {
                    double mi = merit(x[i], i);
                    if (mi > m) {
                        m = mi;
                        p = i;
                    }
                }
            for (int i = 0; i < y.size(); i++)
                if (!y[i].assigned()) {
                    double mi = merit(y[i], i);
                    if (mi > m) {
                        m = mi;
                        p = i + x.size();
                    }
                }
        } else {
            assert(!y[p - x.size()].assigned());
            m = merit(y[p - x.size()], p - x.size());
            for (int i = p - x.size() + 1; i < y.size(); i++)
                if (!y[i].assigned()) {
                    double mi = merit(y[i], i);
                    if (mi > m) {
                        m = mi;
                        p = i + x.size();
                    }
                }
        }
        int v;
        if (p < x.size()) {
            v = xvsc->val(home, x[p], p);
        } else {
            v = yvsc->val(home, y[p - x.size()], p - x.size());
        }
        return new PosIntChoice(*this, 2, p, v);
    }

    template <class Merit>
    size_t
    IntBoolBrancher<Merit>::dispose(Space& home) {
        merit.dispose();
        (void)IntBoolBrancherBase::dispose(home);
        return sizeof(IntBoolBrancher<Merit>);
    }

    forceinline BoolValBranch
    i2b(const IntValBranch& ivb) {
        switch (ivb.select()) {
        case IntValBranch::SEL_MIN:
        case IntValBranch::SEL_MED:
        case IntValBranch::SEL_SPLIT_MIN:
        case IntValBranch::SEL_RANGE_MIN:
            return BOOL_VAL_MIN();
        case IntValBranch::SEL_MAX:
        case IntValBranch::SEL_SPLIT_MAX:
        case IntValBranch::SEL_RANGE_MAX:
            return BOOL_VAL_MAX();
        case IntValBranch::SEL_RND:
            return BOOL_VAL_RND(ivb.rnd());
        case IntValBranch::SEL_VAL_COMMIT:
        default:
            GECODE_NEVER;
        }
        GECODE_NEVER;
        return BOOL_VAL_MIN();
    }

}} // namespace Gecode::FlatZinc

// STATISTICS: flatzinc-branch
