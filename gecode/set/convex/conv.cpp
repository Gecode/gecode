/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
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
    Convex::copy(Space& home) {
        return new (home) Convex(home, *this);
    }

    ExecStatus
    Convex::propagate(Space& home, const ModEventDelta&) {
        // I, drop ranges from UB that are shorter than card_min
        // II, add range LB.smallest to LB.largest to LB
        // III, Drop ranges from UB that do not contain all elements of LB
        // that is: range.min()>LB.smallest or range.max()<LB.largest
        // This leaves only one range.
        // II
        if (x0.glb_size() > 0) {
            GECODE_ME_CHECK(x0.include(home, x0.glb_min(), x0.glb_max()));
        } else {
            // If lower bound is empty, we can still constrain the cardinality
            // maximum with the width of the longest range in UB.
            // No need to do this if there is anything in LB because UB
            // becomes a single range then.
            LubRanges<SetView> ub_range_it(x0);
            unsigned int max_width = 0;
            for (; ub_range_it(); ++ub_range_it) {
                assert(ub_range_it());
                max_width = std::max(max_width, ub_range_it.width());
            }
            GECODE_ME_CHECK(x0.card_max(home, max_width));
        }

        // I + III

        Region r;
        LubRanges<SetView> ub_range_it(x0);
        Iter::Ranges::Cache ub_range_it_c(r, ub_range_it);

        for (; ub_range_it_c(); ++ub_range_it_c) {
            if (ub_range_it_c.width() < x0.card_min() ||
                ub_range_it_c.min() > x0.glb_min() // No need to test for empty lb.
                || ub_range_it_c.max() < x0.glb_max()) {
                GECODE_ME_CHECK(x0.exclude(home, ub_range_it_c.min(), ub_range_it_c.max()));
            }
        }
        if (x0.assigned())
            return home.ES_SUBSUMED(*this);
        return ES_FIX;
    }

}}} // namespace Gecode::Set::Convex

// STATISTICS: set-prop
