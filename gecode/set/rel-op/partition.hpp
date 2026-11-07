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
     * "Nary partition" propagator
     *
     */

    template <class View0, class View1>
    forceinline
    PartitionN<View0, View1>::PartitionN(Home home, ViewArray<View0>& x, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
    }

    template <class View0, class View1>
    forceinline
    PartitionN<View0, View1>::PartitionN(Home home, ViewArray<View0>& x, const IntSet& z, View1 y)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, x, y) {
        shared = Gecode::shared(x) || view_array_shared(x, y);
        IntSetRanges rz(z);
        union_of_dets.include_i(home, rz);
    }

    template <class View0, class View1>
    forceinline
    PartitionN<View0, View1>::PartitionN(Space& home, PartitionN& p)
        : MixNaryOnePropagator<View0, PC_SET_ANY, View1, PC_SET_ANY>(home, p), shared(p.shared) {
        union_of_dets.update(home, p.union_of_dets);
    }

    template <class View0, class View1>
    Actor*
    PartitionN<View0, View1>::copy(Space& home) {
        return new (home) PartitionN(home, *this);
    }

    template <class View0, class View1>
    ExecStatus
    PartitionN<View0, View1>::post(Home home, ViewArray<View0>& x, View1 y) {
        switch (x.size()) {
        case 0:
            GECODE_ME_CHECK(y.card_max(home, 0));
            return ES_OK;
        case 1:
            return Rel::Eq<View0, View1>::post(home, x[0], y);
        default:
            (void)new (home) PartitionN<View0, View1>(home, x, y);
            return ES_OK;
        }
    }

    template <class View0, class View1>
    ExecStatus
    PartitionN<View0, View1>::post(Home home, ViewArray<View0>& x, const IntSet& z, View1 y) {
        if (x.size() == 0) {
            // With no operands, y equals z
            IntSetRanges glb(z);
            GECODE_ME_CHECK(y.include_i(home, glb));
            IntSetRanges lub(z);
            GECODE_ME_CHECK(y.intersect_i(home, lub));
            return ES_OK;
        }
        (void)new (home) PartitionN<View0, View1>(home, x, z, y);
        return ES_OK;
    }

    template <class View0, class View1>
    PropCost
    PartitionN<View0, View1>::cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO, x.size() + 1);
    }

    template <class View0, class View1>
    ExecStatus
    PartitionN<View0, View1>::propagate(Space& home, const ModEventDelta& med) {

        ModEvent me0 = View0::me(med);
        ModEvent me1 = View1::me(med);
        bool ubevent = Rel::test_set_event_ub(me0, me1);
        bool lbevent = Rel::test_set_event_lb(me0, me1);
        bool anybevent = Rel::test_set_event_any_b(me0, me1);
        bool cardevent = Rel::test_set_event_card(me0, me1);

        bool modified = false;
        bool old_modified = false;

        do {
            old_modified = modified;
            modified = false;
            if (old_modified || anybevent)
                GECODE_ES_CHECK(partition_n_xi_ub(home, modified, x, y, union_of_dets));
            if (modified || old_modified || anybevent)
                GECODE_ES_CHECK(partition_n_xi_lb(home, modified, x, y, union_of_dets));
            if (modified || old_modified || ubevent)
                GECODE_ES_CHECK(partition_nyub(home, modified, x, y, union_of_dets));
            if (modified || old_modified || lbevent)
                GECODE_ES_CHECK(partition_nylb(home, modified, x, y, union_of_dets));
            if (modified || old_modified || ubevent)
                GECODE_ES_CHECK(union_n_xi_ub(home, modified, x, y, union_of_dets));
            if (modified || old_modified || cardevent)
                GECODE_ES_CHECK(partition_n_card(home, modified, x, y, union_of_dets));
        } while (modified);

        // removing assigned sets from x, accumulating the value:
        for (int i = 0; i < x.size(); i++) {
            // Do not reverse! Eats away the end of the array!
            while (i < x.size() && x[i].assigned()) {
                GlbRanges<View0> det(x[i]);
                union_of_dets.include_i(home, det);
                x.move_lst(i);
            }
        }
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
