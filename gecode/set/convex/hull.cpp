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

#include <gecode/set/convex.hh>

namespace Gecode { namespace Set { namespace Convex {

    Actor*
    ConvexHull::copy(Space& home) {
        return new (home) ConvexHull(home, *this);
    }

    ExecStatus
    ConvexHull::propagate(Space& home, const ModEventDelta&) {
        // x1 is the convex hull of x0

        GECODE_ME_CHECK(x1.card_min(home, x0.card_min()));
        GECODE_ME_CHECK(x0.card_max(home, x1.card_max()));

        do {

            // intersect x1 with (x0.lub_min(),x0.lub_max())
            // This empties x1 if x0.ub is empty. twice.
            GECODE_ME_CHECK(x1.exclude(home, Limits::min, x0.lub_min() - 1));
            GECODE_ME_CHECK(x1.exclude(home, x0.lub_max() + 1, Limits::max));

            int min_element = std::min(x1.glb_min(), x0.glb_min());
            int max_element = std::max(x1.glb_max(), x0.glb_max());

            if (min_element < max_element) {
                GECODE_ME_CHECK(x1.include(home, min_element, max_element));
            }

            unsigned int card_min = x1.card_min();

            Region r;
            LubRanges<SetView> ub_range_it(x1);
            Iter::Ranges::Cache ub_range_it_c(r, ub_range_it);
            for (; ub_range_it_c(); ++ub_range_it_c) {
                if (ub_range_it_c.width() < card_min ||
                    ub_range_it_c.min() > min_element // No need to test for empty lb.
                    || ub_range_it_c.max() < max_element) {
                    GECODE_ME_CHECK(x1.exclude(home, ub_range_it_c.min(), ub_range_it_c.max()));
                }
            }

            LubRanges<SetView> ub_range_it2(x1);
            GECODE_ME_CHECK(x0.intersect_i(home, ub_range_it2));

            if (x1.lub_min() != BndSet::MIN_OF_EMPTY) {
                if (x1.lub_min() == x1.glb_min()) {
                    GECODE_ME_CHECK(x0.include(home, x1.lub_min()));
                }
                if (x1.lub_max() == x1.glb_max()) {
                    GECODE_ME_CHECK(x0.include(home, x1.lub_max()));
                }
            }
        } while (x0.assigned() && !x1.assigned());

        // If x0 is assigned, x1 should be too.
        assert(x1.assigned() || !x0.assigned());

        if (x1.assigned()) {
            return home.ES_SUBSUMED(*this);
        }

        return ES_NOFIX;
    }

}}} // namespace Gecode::Set::Convex

// STATISTICS: set-prop
