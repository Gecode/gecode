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

namespace Gecode { namespace Set { namespace Rel {

    template <class View0, class View1>
    forceinline
    Eq<View0, View1>::Eq(Home home, View0 x, View1 y)
        : MixBinaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y) {}

    template <class View0, class View1>
    forceinline
    Eq<View0, View1>::Eq(Space& home, Eq& p)
        : MixBinaryPropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, p) {}

    template <class View0, class View1>
    ExecStatus
    Eq<View0, View1>::post(Home home, View0 x, View1 y) {
        if (!same(x, y))
            (void)new (home) Eq(home, x, y);
        return ES_OK;
    }

    template <class View0, class View1>
    Actor*
    Eq<View0, View1>::copy(Space& home) {
        return new (home) Eq(home, *this);
    }

    template <class View0, class View1>
    ExecStatus
    Eq<View0, View1>::propagate(Space& home, const ModEventDelta& med) {

        ModEvent me0 = View0::me(med);
        ModEvent me1 = View1::me(med);

        Region r;

        if (test_set_event_lb(me0, me1)) {
            GlbRanges<View0> x0lb(x0);
            GlbRanges<View1> x1lb(x1);
            Iter::Ranges::Union<GlbRanges<View0>, GlbRanges<View1>> lbu(x0lb, x1lb);
            Iter::Ranges::Cache lbuc(r, lbu);
            GECODE_ME_CHECK(x0.include_i(home, lbuc));
            lbuc.reset();
            GECODE_ME_CHECK(x1.include_i(home, lbuc));
        }

        if (test_set_event_ub(me0, me1)) {
            LubRanges<View0> x0ub(x0);
            LubRanges<View1> x1ub(x1);
            Iter::Ranges::Inter<LubRanges<View0>, LubRanges<View1>> ubi(x0ub, x1ub);
            Iter::Ranges::Cache ubic(r, ubi);
            GECODE_ME_CHECK(x0.intersect_i(home, ubic));
            ubic.reset();
            GECODE_ME_CHECK(x1.intersect_i(home, ubic));
        }

        if (test_set_event_card(me0, me1)) {
            unsigned int max = std::min(x0.card_max(), x1.card_max());
            unsigned int min = std::max(x0.card_min(), x1.card_min());
            GECODE_ME_CHECK(x0.card_max(home, max));
            GECODE_ME_CHECK(x1.card_max(home, max));
            GECODE_ME_CHECK(x0.card_min(home, min));
            GECODE_ME_CHECK(x1.card_min(home, min));
        }

        if (x0.assigned()) {
            assert(x1.assigned());
            return home.ES_SUBSUMED(*this);
        }
        return shared(x0, x1) ? ES_NOFIX : ES_FIX;
    }

}}} // namespace Gecode::Set::Rel

// STATISTICS: set-prop
