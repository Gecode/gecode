/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004,2006,2007
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Set { namespace Element {

    template <class SView, class RView>
    forceinline
    ElementUnionConst<SView, RView>::ElementUnionConst(Home home, SView y0, const IntSetArgs& iv0, RView y1)
        : Propagator(home), x0(y0), n_iv(iv0.size()), x1(y1) {
        home.notice(*this, AP_DISPOSE);
        x0.subscribe(home, *this, PC_SET_ANY);
        x1.subscribe(home, *this, PC_SET_ANY);
        iv = static_cast<Space&>(home).alloc<IntSet>(n_iv);
        for (unsigned int i = iv0.size(); i--;)
            iv[i] = iv0[i];
    }

    template <class SView, class RView>
    forceinline
    ElementUnionConst<SView, RView>::ElementUnionConst(Space& home, ElementUnionConst<SView, RView>& p)
        : Propagator(home, p), n_iv(p.n_iv) {
        x0.update(home, p.x0);
        x1.update(home, p.x1);
        iv = home.alloc<IntSet>(n_iv);
        for (unsigned int i = n_iv; i--;)
            iv[i] = p.iv[i];
    }

    template <class SView, class RView>
    PropCost
    ElementUnionConst<SView, RView>::cost(const Space&, const ModEventDelta&) const {
        return PropCost::linear(PropCost::HI, n_iv + 2);
    }

    template <class SView, class RView>
    void
    ElementUnionConst<SView, RView>::reschedule(Space& home) {
        x0.reschedule(home, *this, PC_SET_ANY);
        x1.reschedule(home, *this, PC_SET_ANY);
    }

    template <class SView, class RView>
    forceinline size_t
    ElementUnionConst<SView, RView>::dispose(Space& home) {
        home.ignore(*this, AP_DISPOSE);
        if (!home.failed()) {
            x0.cancel(home, *this, PC_SET_ANY);
            x1.cancel(home, *this, PC_SET_ANY);
        }
        for (unsigned int i = n_iv; i--;)
            iv[i].~IntSet();
        (void)Propagator::dispose(home);
        return sizeof(*this);
    }

    template <class SView, class RView>
    ExecStatus
    ElementUnionConst<SView, RView>::post(Home home, SView x0, const IntSetArgs& xs, RView x1) {
        int n = xs.size();

        // s2 \subseteq {1,...,n}
        Iter::Ranges::Singleton s(0, n - 1);
        GECODE_ME_CHECK(x1.intersect_i(home, s));
        if (n == 0) {
            // Nothing can be selected, and the empty union is empty
            GECODE_ME_CHECK(x0.card_max(home, 0));
            return ES_OK;
        }
        (void)new (home) ElementUnionConst<SView, RView>(home, x0, xs, x1);
        return ES_OK;
    }

    template <class SView, class RView>
    Actor*
    ElementUnionConst<SView, RView>::copy(Space& home) {
        return new (home) ElementUnionConst<SView, RView>(home, *this);
    }

    template <class SView, class RView>
    ExecStatus
    ElementUnionConst<SView, RView>::propagate(Space& home, const ModEventDelta&) {
        Region r;

        bool* still_selected = r.alloc<bool>(n_iv);

        bool loop_var;
        do {
            loop_var = false;
            for (int i = n_iv; i--;)
                still_selected[i] = false;

            // Cache the upper bound iterator, as we have to
            // modify the upper bound while iterating
            LubRanges<RView> x1ub(x1);
            Iter::Ranges::Cache x1ubc(r, x1ub);
            Iter::Ranges::ToValues<Iter::Ranges::Cache> vx1ub(x1ubc);

            GlbRanges<RView> x1lb(x1);
            Iter::Ranges::Cache x1lbc(r, x1lb);
            Iter::Ranges::ToValues<Iter::Ranges::Cache> vx1(x1lbc);

            // In the first iteration, compute in before[i] the union
            // of all the upper bounds of the x_i. At the same time,
            // exclude inconsistent x_i from x1.

            GLBndSet sofar_before(home);
            LUBndSet selected_inter(home, IntSet(Limits::min, Limits::max));
            GLBndSet* before = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * n_iv));

            unsigned int max_card = 0;
            unsigned int min_card = Limits::card;

            while (vx1ub()) {
                int i = vx1ub.val();

                IntSetRanges cand_card_r(iv[i]);
                unsigned int candidate_card = Iter::Ranges::size(cand_card_r);

                IntSetRanges candlb(iv[i]);
                LubRanges<SView> x0ub(x0);
                Iter::Ranges::Diff<IntSetRanges, LubRanges<SView>> diff(candlb, x0ub);

                bool select_single_inconsistent = false;
                if (x1.card_max() <= 1) {
                    GlbRanges<SView> x0lb(x0);
                    IntSetRanges candub(iv[i]);
                    Iter::Ranges::Diff<GlbRanges<SView>, IntSetRanges> diff2(x0lb, candub);
                    select_single_inconsistent = diff2() || candidate_card < x0.card_min();
                }

                // exclude inconsistent x_i
                // an x_i is inconsistent if
                //  * at most one x_i can be selected and there are
                //    elements in x_0 that can't be in x_i
                //    (selectSingleInconsistent)
                //  * its min cardinality is greater than maxCard of x0
                //  * inter is not empty (there are elements in x_i
                //    that can't be in x_0)
                if (select_single_inconsistent || candidate_card > x0.card_max() || diff()) {
                    ModEvent me = (x1.exclude(home, i));
                    loop_var |= me_modified(me);
                    GECODE_ME_CHECK(me);
                } else {
                    still_selected[i] = true;
                    // if x_i is consistent, check whether we know
                    // that its index is in x1
                    if (vx1() && vx1.val() == i) {
                        // x0 >= candidate, candidate <= x0
                        // GlbRanges<SView> candlb(candidate);
                        IntSetRanges candlb(iv[i]);
                        ModEvent me = x0.include_i(home, candlb);
                        loop_var |= me_modified(me);
                        GECODE_ME_CHECK(me);
                        ++vx1;
                    }
                    new (&before[i]) GLBndSet(home);
                    before[i].update(home, sofar_before);
                    IntSetRanges cub(iv[i]);
                    sofar_before.include_i(home, cub);
                    IntSetRanges clb(iv[i]);
                    selected_inter.intersect_i(home, clb);
                    max_card = std::max(max_card, candidate_card);
                    min_card = std::min(min_card, candidate_card);
                }

                ++vx1ub;
            }

            if (x1.card_max() == 0) {
                // Selector is empty, hence the result must be empty
                {
                    GECODE_ME_CHECK(x0.card_max(home, 0));
                }
                for (int i = n_iv; i--;)
                    if (still_selected[i])
                        before[i].dispose(home);
                selected_inter.dispose(home);
                sofar_before.dispose(home);
                return home.ES_SUBSUMED(*this);
            }

            if (x1.card_min() > 0) {
                // Selector is not empty, hence the intersection of the
                // possibly selected lower bounds is contained in x0
                BndSetRanges si(selected_inter);
                ModEvent me = x0.include_i(home, si);
                loop_var |= me_modified(me);
                GECODE_ME_CHECK(me);
                me = x0.card_min(home, min_card);
                loop_var |= me_modified(me);
                GECODE_ME_CHECK(me);
            }
            selected_inter.dispose(home);

            if (x1.card_max() <= 1) {
                ModEvent me = x0.card_max(home, max_card);
                loop_var |= me_modified(me);
                GECODE_ME_CHECK(me);
            }

            {
                // x0 <= sofarBefore
                BndSetRanges sf_b(sofar_before);
                ModEvent me = x0.intersect_i(home, sf_b);
                loop_var |= me_modified(me);
                GECODE_ME_CHECK(me);
            }

            sofar_before.dispose(home);

            GLBndSet sofar_after(home);

            // In the second iteration, this time backwards, compute
            // sofarAfter as the union of all lub(x_j) with j>i
            for (int i = n_iv; i--;) {
                if (!still_selected[i])
                    continue;
                BndSetRanges b(before[i]);
                BndSetRanges s(sofar_after);
                GlbRanges<SView> x0lb(x0);
                Iter::Ranges::Union<BndSetRanges, BndSetRanges> inter(b, s);
                Iter::Ranges::Diff<GlbRanges<SView>, Iter::Ranges::Union<BndSetRanges, BndSetRanges>> diff(x0lb, inter);
                if (diff()) {
                    ModEvent me = (x1.include(home, i));
                    loop_var |= me_modified(me);
                    GECODE_ME_CHECK(me);

                    // candidate != extra
                    IntSetRanges ivi(iv[i]);
                    if (!Iter::Ranges::subset(diff, ivi))
                        GECODE_ME_CHECK(ME_SET_FAILED);
                }

                IntSetRanges iviub(iv[i]);
                sofar_after.include_i(home, iviub);
                before[i].dispose(home);
            }
            sofar_after.dispose(home);

        } while (loop_var);

        if (x1.assigned()) {
            assert(x0.assigned());
            return home.ES_SUBSUMED(*this);
        }

        return ES_FIX;
    }

}}} // namespace Gecode::Set::Element

// STATISTICS: set-prop
