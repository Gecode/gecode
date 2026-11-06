/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode::Set::RelOp {

    /*
     * "Ternary intersection" propagator
     *
     */

    template <class View0, class View1, class View2>
    ExecStatus
    Intersection<View0, View1, View2>::post(Home home, View0 x0, View1 x1, View2 x2) {
        (void)new (home) Intersection<View0, View1, View2>(home, x0, x1, x2);
        return ES_OK;
    }

    template <class View0, class View1, class View2>
    Actor*
    Intersection<View0, View1, View2>::copy(Space& home) {
        return new (home) Intersection(home, *this);
    }

    template <class View0, class View1, class View2>
    ExecStatus
    Intersection<View0, View1, View2>::propagate(Space& home, const ModEventDelta& med) {
        // This propagator implements the constraint
        // x2 = x0 \cap x1

        bool x0ass = x0.assigned();
        bool x1ass = x1.assigned();
        bool x2ass = x2.assigned();

        ModEvent me0 = View0::me(med);
        ModEvent me1 = View1::me(med);
        ModEvent me2 = View2::me(med);

        bool x0lbmod = false;
        bool x1lbmod = false;
        bool modified = false;

        do {

            modified = false;
            do {
                // 4) glb(x2) <= glb(x0) \cap glb(x1)
                {
                    GlbRanges<View0> x0lb(x0);
                    GlbRanges<View1> x1lb(x1);
                    Iter::Ranges::Inter<GlbRanges<View0>, GlbRanges<View1>> i2(x0lb, x1lb);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.include_i(home, i2));
                }

                if (modified || Rel::test_set_event_lb(me2)) {
                    modified = false;
                    // 5) x2 <= x0
                    GlbRanges<View2> x2lb1(x2);
                    GECODE_ME_CHECK_MODIFIED(modified, x0.include_i(home, x2lb1));
                    x0lbmod |= modified;

                    // 6) x2 <= x1
                    bool modified2 = false;
                    GlbRanges<View2> x2lb2(x2);
                    GECODE_ME_CHECK_MODIFIED(modified2, x1.include_i(home, x2lb2));
                    x1lbmod |= modified2;
                    modified |= modified2;
                }

            } while (modified);

            modified = false;
            do {
                bool modified_old = modified;
                modified = false;

                if (Rel::test_set_event_ub(me2) || Rel::test_set_event_lb(me0) || x0lbmod || modified_old) {
                    // 1) lub(x2) \ glb(x0) => lub(x1)
                    GlbRanges<View0> x0lb(x0);
                    LubRanges<View2> x2ub(x2);
                    Iter::Ranges::Diff<GlbRanges<View0>, LubRanges<View2>> diff(x0lb, x2ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x1.exclude_i(home, diff));
                }

                if (Rel::test_set_event_ub(me2) || Rel::test_set_event_lb(me1) || x1lbmod || modified_old) {
                    // 2) lub(x2) \ glb(x1) => lub(x0)
                    GlbRanges<View1> x1lb(x1);
                    LubRanges<View2> x2ub(x2);
                    Iter::Ranges::Diff<GlbRanges<View1>, LubRanges<View2>> diff(x1lb, x2ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x0.exclude_i(home, diff));
                }

                if (Rel::test_set_event_ub(me0, me1) || modified) {
                    //            modified = false;
                    // 3) lub(x0) \cap lub(x1) <= lub(x2)
                    LubRanges<View0> x0ub(x0);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Inter<LubRanges<View0>, LubRanges<View1>> i1(x0ub, x1ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.intersect_i(home, i1));
                }

            } while (modified);

            modified = false;
            {
                // cardinality
                ExecStatus ret = inter_card(home, modified, x0, x1, x2);
                GECODE_ES_CHECK(ret);
            }

            if (x2.card_min() == Set::Limits::card) {
                GECODE_ME_CHECK(x0.card_min(home, Set::Limits::card));
                GECODE_ME_CHECK(x1.card_min(home, Set::Limits::card));
                return home.ES_SUBSUMED(*this);
            }

            if (x0.card_min() == Set::Limits::card)
                GECODE_REWRITE(*this, (Rel::Eq<View1, View2>::post(home(*this), x1, x2)));
            if (x1.card_min() == Set::Limits::card)
                GECODE_REWRITE(*this, (Rel::Eq<View0, View2>::post(home(*this), x0, x2)));

        } while (modified);

        if (shared(x0, x1, x2)) {
            return (x0ass && x1ass && x2ass) ? home.ES_SUBSUMED(*this) : ES_NOFIX;
        }
        if (x0ass && x1ass && x2ass)
            return home.ES_SUBSUMED(*this);
        if (x0ass != x0.assigned() || x1ass != x1.assigned() || x2ass != x2.assigned()) {
            return ES_NOFIX;
        }
        return ES_FIX;
    }

    template <class View0, class View1, class View2>
    forceinline
    Intersection<View0, View1, View2>::Intersection(Home home, View0 y0, View1 y1, View2 y2)
        : MixTernaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY, View2, PC_SET_ANY>(home, y0, y1, y2) {}

    template <class View0, class View1, class View2>
    forceinline
    Intersection<View0, View1, View2>::Intersection(Space& home, Intersection<View0, View1, View2>& p)
        : MixTernaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY, View2, PC_SET_ANY>(home, p) {}

    /*
     * "Nary intersection" propagator
     *
     */

    template <class View0, class View1>
    forceinline
    IntersectionN<View0, View1>::IntersectionN(Home home, ViewArray<View0>& x, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y), int_of_dets(home) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
    }

    template <class View0, class View1>
    forceinline
    IntersectionN<View0, View1>::IntersectionN(Home home, ViewArray<View0>& x, const IntSet& z, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y), int_of_dets(home) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
        IntSetRanges rz(z);
        int_of_dets.intersect_i(home, rz);
    }

    template <class View0, class View1>
    forceinline
    IntersectionN<View0, View1>::IntersectionN(Space& home, IntersectionN& p)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, p), shared(p.shared), int_of_dets() {
        int_of_dets.update(home, p.int_of_dets);
    }

    template <class View0, class View1>
    ExecStatus
    IntersectionN<View0, View1>::post(Home home, ViewArray<View0>& x, View1 y) {
        switch (x.size()) {
        case 0:
            GECODE_ME_CHECK(y.card_min(home, Limits::card));
            return ES_OK;
        case 1:
            return Rel::Eq<View0, View1>::post(home, x[0], y);
        case 2:
            return Intersection<View0, View0, View1>::post(home, x[0], x[1], y);
        default:
            (void)new (home) IntersectionN<View0, View1>(home, x, y);
            return ES_OK;
        }
    }

    template <class View0, class View1>
    ExecStatus
    IntersectionN<View0, View1>::post(Home home, ViewArray<View0>& x, const IntSet& z, View1 y) {
        (void)new (home) IntersectionN<View0, View1>(home, x, z, y);
        return ES_OK;
    }

    template <class View0, class View1>
    Actor*
    IntersectionN<View0, View1>::copy(Space& home) {
        return new (home) IntersectionN<View0, View1>(home, *this);
    }

    template <class View0, class View1>
    PropCost
    IntersectionN<View0, View1>::cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::HI, x.size() + 1);
    }

    template <class View0, class View1>
    ExecStatus
    IntersectionN<View0, View1>::propagate(Space& home, const ModEventDelta&) {
        bool repeat = false;
        do {
            repeat = false;
            int xsize = x.size();
            if (xsize == 0)
                goto size0;
            for (int i = xsize; i--;) {
                GECODE_ME_CHECK(y.card_max(home, x[i].card_max()));
                GECODE_ME_CHECK(x[i].card_min(home, y.card_min()));
                if (x[i].card_max() == 0) {
                    GECODE_ME_CHECK(y.card_max(home, 0));
                    int_of_dets.dispose(home);
                    return home.ES_SUBSUMED(*this);
                }
            }
            {
                Region r;
                GlbRanges<View0>* x_l_bs = r.alloc<GlbRanges<View0>>(xsize);
                LubRanges<View0>* x_u_bs = r.alloc<LubRanges<View0>>(xsize);
                for (int i = xsize; i--;) {
                    GlbRanges<View0> lb(x[i]);
                    LubRanges<View0> ub(x[i]);
                    x_l_bs[i] = lb;
                    x_u_bs[i] = ub;
                }
                {
                    Iter::Ranges::NaryInter lbi(r, x_l_bs, xsize);
                    BndSetRanges dets(int_of_dets);
                    lbi &= dets;
                    GECODE_ME_CHECK(y.include_i(home, lbi));
                }
                {
                    Iter::Ranges::NaryInter ubi(r, x_u_bs, xsize);
                    BndSetRanges dets(int_of_dets);
                    ubi &= dets;
                    GECODE_ME_CHECK(y.intersect_i(home, ubi));
                }
            }

            for (int i = xsize; i--;) {
                GlbRanges<View1> ylb(y);
                GECODE_ME_CHECK(x[i].include_i(home, ylb));
            }

            // xi.exclude (Intersection(xj.lb) - y.ub)
            {
                Region r;
                GLBndSet* right_set = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));
                new (&right_set[xsize - 1]) GLBndSet(home);
                right_set[xsize - 1].update(home, int_of_dets);
                for (int i = xsize - 1; i--;) {
                    GlbRanges<View0> xilb(x[i + 1]);
                    BndSetRanges prev(right_set[i + 1]);
                    Iter::Ranges::Inter<GlbRanges<View0>, BndSetRanges> inter(xilb, prev);
                    new (&right_set[i]) GLBndSet(home);
                    right_set[i].include_i(home, inter);
                }

                LUBndSet left_acc(home);

                for (int i = 0; i < xsize; i++) {
                    BndSetRanges left(left_acc);
                    BndSetRanges right(right_set[i]);
                    Iter::Ranges::Inter<BndSetRanges, BndSetRanges> inter(left, right);
                    LubRanges<View1> yub(y);
                    Iter::Ranges::Diff<Iter::Ranges::Inter<BndSetRanges, BndSetRanges>, LubRanges<View1>> forbidden(
                        inter, yub);
                    GECODE_ME_CHECK(x[i].exclude_i(home, forbidden));
                    GlbRanges<View0> xlb(x[i]);
                    left_acc.intersect_i(home, xlb);
                }
                for (int i = xsize; i--;)
                    right_set[i].dispose(home);
                left_acc.dispose(home);
            }

            for (int i = 0; i < x.size(); i++) {
                // Do not reverse! Eats away the end of the array!
                while (i < x.size() && x[i].assigned()) {
                    GlbRanges<View0> det(x[i]);
                    if (int_of_dets.intersect_i(home, det)) {
                        repeat = true;
                    }
                    x.move_lst(i);
                    if (int_of_dets.empty()) {
                        GECODE_ME_CHECK(y.card_max(home, 0));
                        int_of_dets.dispose(home);
                        return home.ES_SUBSUMED(*this);
                    }
                }
            }
        size0:
            if (x.size() == 0) {
                BndSetRanges all1(int_of_dets);
                GECODE_ME_CHECK(y.intersect_i(home, all1));
                BndSetRanges all2(int_of_dets);
                GECODE_ME_CHECK(y.include_i(home, all2));
                int_of_dets.dispose(home);
                return home.ES_SUBSUMED(*this);
            }

        } while (repeat);

        return shared ? ES_NOFIX : ES_FIX;
    }

} // namespace Gecode::Set::RelOp

// STATISTICS: set-prop
