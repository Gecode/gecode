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
     * "Ternary union" propagator
     *
     */

    template <class View0, class View1, class View2>
    forceinline
    Union<View0, View1, View2>::Union(Home home, View0 y0, View1 y1, View2 y2)
        : MixTernaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY, View2, PC_SET_ANY>(home, y0, y1, y2) {}

    template <class View0, class View1, class View2>
    forceinline
    Union<View0, View1, View2>::Union(Space& home, Union<View0, View1, View2>& p)
        : MixTernaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY, View2, PC_SET_ANY>(home, p) {}

    template <class View0, class View1, class View2>
    ExecStatus
    Union<View0, View1, View2>::post(Home home, View0 x0, View1 x1, View2 x2) {
        (void)new (home) Union<View0, View1, View2>(home, x0, x1, x2);
        return ES_OK;
    }

    template <class View0, class View1, class View2>
    Actor*
    Union<View0, View1, View2>::copy(Space& home) {
        return new (home) Union(home, *this);
    }

    template <class View0, class View1, class View2>
    ExecStatus
    Union<View0, View1, View2>::propagate(Space& home, const ModEventDelta& med) {
        // This propagator implements the constraint
        // x2 = x0 u x1

        bool x0ass = x0.assigned();
        bool x1ass = x1.assigned();
        bool x2ass = x2.assigned();

        ModEvent me0 = View0::me(med);
        ModEvent me1 = View1::me(med);
        ModEvent me2 = View2::me(med);

        bool x0ubmod = false;
        bool x1ubmod = false;
        bool modified = false;

        do {

            modified = false;
            do {
                // 4) lub(x2) <= lub(x0) u lub(x1)
                {
                    LubRanges<View0> x0ub(x0);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Union<LubRanges<View0>, LubRanges<View1>> u2(x0ub, x1ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.intersect_i(home, u2));
                }

                if (modified || Rel::test_set_event_ub(me2)) {
                    modified = false;
                    // 5) x0 <= x2
                    LubRanges<View2> x2ub1(x2);
                    GECODE_ME_CHECK_MODIFIED(modified, x0.intersect_i(home, x2ub1));
                    x0ubmod |= modified;

                    // 6) x1 <= x2
                    bool modified2 = false;
                    LubRanges<View2> x2ub2(x2);
                    GECODE_ME_CHECK_MODIFIED(modified2, x1.intersect_i(home, x2ub2));
                    x1ubmod |= modified2;
                    modified |= modified2;
                }

            } while (modified);

            modified = false;
            do {
                bool modified_old = modified;
                modified = false;

                if (Rel::test_set_event_lb(me2) || Rel::test_set_event_ub(me0) || x0ubmod || modified_old) {
                    // 1) glb(x0) \ lub(x1) <= glb(x2)
                    GlbRanges<View2> x2lb(x2);
                    LubRanges<View0> x0ub(x0);
                    Iter::Ranges::Diff<GlbRanges<View2>, LubRanges<View0>> diff(x2lb, x0ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x1.include_i(home, diff));
                }

                if (Rel::test_set_event_lb(me2) || Rel::test_set_event_ub(me1) || x1ubmod || modified_old) {
                    // 2) glb(x0) \ lub(x2) <= glb(x1)
                    GlbRanges<View2> x2lb(x2);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Diff<GlbRanges<View2>, LubRanges<View1>> diff(x2lb, x1ub);
                    GECODE_ME_CHECK_MODIFIED(modified, x0.include_i(home, diff));
                }

                if (Rel::test_set_event_lb(me0, me1) || modified) {
                    //            modified = false;
                    // 3) glb(x1) u glb(x2) <= glb(x0)
                    GlbRanges<View0> x0lb(x0);
                    GlbRanges<View1> x1lb(x1);
                    Iter::Ranges::Union<GlbRanges<View0>, GlbRanges<View1>> u1(x0lb, x1lb);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.include_i(home, u1));
                }

            } while (modified);

            modified = false;
            {
                // cardinality
                ExecStatus ret = union_card(home, modified, x0, x1, x2);
                GECODE_ES_CHECK(ret);
            }

            if (x2.card_max() == 0) {
                GECODE_ME_CHECK(x0.card_max(home, 0));
                GECODE_ME_CHECK(x1.card_max(home, 0));
                return home.ES_SUBSUMED(*this);
            }

            if (x0.card_max() == 0)
                GECODE_REWRITE(*this, (Rel::Eq<View1, View2>::post(home(*this), x1, x2)));
            if (x1.card_max() == 0)
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

    /*
     * "Nary union" propagator
     *
     */

    template <class View0, class View1>
    forceinline
    UnionN<View0, View1>::UnionN(Home home, ViewArray<View0>& x, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
    }

    template <class View0, class View1>
    forceinline
    UnionN<View0, View1>::UnionN(Home home, ViewArray<View0>& x, const IntSet& z, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
        IntSetRanges rz(z);
        union_of_dets.include_i(home, rz);
    }

    template <class View0, class View1>
    forceinline
    UnionN<View0, View1>::UnionN(Space& home, UnionN& p)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, p), shared(p.shared) {
        union_of_dets.update(home, p.union_of_dets);
    }

    template <class View0, class View1>
    Actor*
    UnionN<View0, View1>::copy(Space& home) {
        return new (home) UnionN<View0, View1>(home, *this);
    }

    template <class View0, class View1>
    ExecStatus
    UnionN<View0, View1>::post(Home home, ViewArray<View0>& x, View1 y) {
        switch (x.size()) {
        case 0:
            GECODE_ME_CHECK(y.card_max(home, 0));
            return ES_OK;
        case 1:
            return Rel::Eq<View0, View1>::post(home, x[0], y);
        case 2:
            return Union<View0, View0, View1>::post(home, x[0], x[1], y);
        default:
            (void)new (home) UnionN<View0, View1>(home, x, y);
            return ES_OK;
        }
    }

    template <class View0, class View1>
    ExecStatus
    UnionN<View0, View1>::post(Home home, ViewArray<View0>& x, const IntSet& z, View1 y) {
        if (x.size() == 0) {
            // With no operands, y equals z
            IntSetRanges glb(z);
            GECODE_ME_CHECK(y.include_i(home, glb));
            IntSetRanges lub(z);
            GECODE_ME_CHECK(y.intersect_i(home, lub));
            return ES_OK;
        }
        (void)new (home) UnionN<View0, View1>(home, x, z, y);
        return ES_OK;
    }

    template <class View0, class View1>
    PropCost
    UnionN<View0, View1>::cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO, x.size() + 1);
    }

    template <class View0, class View1>
    ExecStatus
    UnionN<View0, View1>::propagate(Space& home, const ModEventDelta& med) {
        ModEvent me0 = View0::me(med);
        ModEvent me1 = View1::me(med);
        bool ubevent = Rel::test_set_event_ub(me0, me1);
        bool lbevent = Rel::test_set_event_lb(me0, me1);
        bool anybevent = Rel::test_set_event_any_b(me0, me1);
        bool cardevent = Rel::test_set_event_card(me0, me1);

        bool modified = false;
        bool old_modified = false;

        do {
            do {
                old_modified = modified;
                modified = false;
                if (modified || old_modified || ubevent)
                    GECODE_ES_CHECK(union_n_xi_ub(home, modified, x, y, union_of_dets));
                if (modified || old_modified || ubevent)
                    GECODE_ES_CHECK(partition_nyub(home, modified, x, y, union_of_dets));
                if (modified || old_modified || anybevent)
                    GECODE_ES_CHECK(partition_n_xi_lb(home, modified, x, y, union_of_dets));
                if (modified || old_modified || lbevent)
                    GECODE_ES_CHECK(partition_nylb(home, modified, x, y, union_of_dets));
                if (modified || old_modified || cardevent || ubevent) {
                    GECODE_ES_CHECK(union_n_card(home, modified, x, y, union_of_dets));
                }
            } while (modified);

            for (int i = 0; i < x.size(); i++) {
                // Do not reverse! Eats away the end of the array!
                while (i < x.size() && x[i].assigned()) {
                    GlbRanges<View0> det(x[i]);
                    union_of_dets.include_i(home, det);
                    x.move_lst(i);
                    modified = true;
                }
            }

        } while (modified);
        // When we run out of variables, make a final check and dissolve:
        if (x.size() == 0) {
            BndSetRanges all1(union_of_dets);
            GECODE_ME_CHECK(y.intersect_i(home, all1));
            BndSetRanges all2(union_of_dets);
            GECODE_ME_CHECK(y.include_i(home, all2));
            union_of_dets.dispose(home);
            return home.ES_SUBSUMED(*this);
        }

        return shared ? ES_NOFIX : ES_FIX;
    }

} // namespace Gecode::Set::RelOp

// STATISTICS: set-prop
