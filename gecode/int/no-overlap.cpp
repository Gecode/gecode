/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

#include <gecode/int/no-overlap.hh>

namespace Gecode {

    namespace Int { namespace NoOverlap {

        bool
        optional(const BoolVarArgs& m) {
            for (int i = 0; i < m.size(); i++)
                if (m[i].none())
                    return true;
            return false;
        }

    }} // namespace Int::NoOverlap

    void
    no_overlap(Home home, const IntVarArgs& x, const IntArgs& w, const IntVarArgs& y, const IntArgs& h, IntPropLevel) {
        using namespace Int;
        using namespace NoOverlap;
        const int n_boxes = x.size();
        if ((n_boxes != w.size()) || (n_boxes != y.size()) || (n_boxes != h.size()))
            throw ArgumentSizeMismatch("Int::nooverlap");
        for (int i = 0; i < n_boxes; i++) {
            Limits::nonnegative(w[i], "Int::nooverlap");
            Limits::nonnegative(h[i], "Int::nooverlap");
            Limits::check(static_cast<long long int>(x[i].max()) + w[i], "Int::nooverlap");
            Limits::check(static_cast<long long int>(y[i].max()) + h[i], "Int::nooverlap");
        }
        GECODE_POST;
        if (n_boxes == 0)
            return;

        ManBox<FixDim, 2>* b = static_cast<Space&>(home).alloc<ManBox<FixDim, 2>>(n_boxes);
        for (int i = 0; i < n_boxes; i++) {
            b[i][0] = FixDim(x[i], w[i]);
            b[i][1] = FixDim(y[i], h[i]);
        }

        GECODE_ES_FAIL((NoOverlap::ManProp<ManBox<FixDim, 2>>::post(home, b, n_boxes)));
    }

    void
    no_overlap(Home home, const IntVarArgs& x, const IntArgs& w, const IntVarArgs& y, const IntArgs& h,
               const BoolVarArgs& m, IntPropLevel) {
        using namespace Int;
        using namespace NoOverlap;
        const int n_boxes = x.size();
        if ((n_boxes != w.size()) || (n_boxes != y.size()) || (n_boxes != h.size()) || (n_boxes != m.size()))
            throw ArgumentSizeMismatch("Int::nooverlap");
        for (int i = 0; i < n_boxes; i++) {
            Limits::nonnegative(w[i], "Int::nooverlap");
            Limits::nonnegative(h[i], "Int::nooverlap");
            Limits::check(static_cast<long long int>(x[i].max()) + w[i], "Int::nooverlap");
            Limits::check(static_cast<long long int>(y[i].max()) + h[i], "Int::nooverlap");
        }
        GECODE_POST;
        if (n_boxes == 0)
            return;

        if (optional(m)) {
            OptBox<FixDim, 2>* b = static_cast<Space&>(home).alloc<OptBox<FixDim, 2>>(n_boxes);
            for (int i = 0; i < n_boxes; i++) {
                b[i][0] = FixDim(x[i], w[i]);
                b[i][1] = FixDim(y[i], h[i]);
                b[i].optional(m[i]);
            }
            GECODE_ES_FAIL((NoOverlap::OptProp<OptBox<FixDim, 2>>::post(home, b, n_boxes)));
        } else {
            ManBox<FixDim, 2>* b = static_cast<Space&>(home).alloc<ManBox<FixDim, 2>>(n_boxes);
            int n = 0;
            for (int i = 0; i < n_boxes; i++)
                if (m[i].one()) {
                    b[n][0] = FixDim(x[i], w[i]);
                    b[n][1] = FixDim(y[i], h[i]);
                    n++;
                }
            GECODE_ES_FAIL((NoOverlap::ManProp<ManBox<FixDim, 2>>::post(home, b, n)));
        }
    }

    void
    no_overlap(Home home, const IntVarArgs& x0, const IntVarArgs& w, const IntVarArgs& x1, const IntVarArgs& y0,
               const IntVarArgs& h, const IntVarArgs& y1, IntPropLevel) {
        using namespace Int;
        using namespace NoOverlap;
        const int n_boxes = x0.size();
        if ((n_boxes != w.size()) || (n_boxes != x1.size()) || (n_boxes != y0.size()) || (n_boxes != h.size()) ||
            (n_boxes != y1.size()))
            throw ArgumentSizeMismatch("Int::nooverlap");
        GECODE_POST;
        if (n_boxes == 0)
            return;

        for (int i = 0; i < n_boxes; i++) {
            GECODE_ME_FAIL(IntView(w[i]).gq(home, 0));
            GECODE_ME_FAIL(IntView(h[i]).gq(home, 0));
        }

        if (w.assigned() && h.assigned()) {
            IntArgs wc(n_boxes), hc(n_boxes);
            for (int i = 0; i < n_boxes; i++) {
                wc[i] = w[i].val();
                hc[i] = h[i].val();
            }
            no_overlap(home, x0, wc, y0, hc);
        } else {
            ManBox<FlexDim, 2>* b = static_cast<Space&>(home).alloc<ManBox<FlexDim, 2>>(n_boxes);
            for (int i = 0; i < n_boxes; i++) {
                b[i][0] = FlexDim(x0[i], w[i], x1[i]);
                b[i][1] = FlexDim(y0[i], h[i], y1[i]);
            }
            GECODE_ES_FAIL((NoOverlap::ManProp<ManBox<FlexDim, 2>>::post(home, b, n_boxes)));
        }
    }

    void
    no_overlap(Home home, const IntVarArgs& x0, const IntVarArgs& w, const IntVarArgs& x1, const IntVarArgs& y0,
               const IntVarArgs& h, const IntVarArgs& y1, const BoolVarArgs& m, IntPropLevel) {
        using namespace Int;
        using namespace NoOverlap;
        const int n_boxes = x0.size();
        if ((n_boxes != w.size()) || (n_boxes != x1.size()) || (n_boxes != y0.size()) || (n_boxes != h.size()) ||
            (n_boxes != y1.size()) || (n_boxes != m.size()))
            throw ArgumentSizeMismatch("Int::nooverlap");
        GECODE_POST;
        if (n_boxes == 0)
            return;

        for (int i = 0; i < n_boxes; i++) {
            GECODE_ME_FAIL(IntView(w[i]).gq(home, 0));
            GECODE_ME_FAIL(IntView(h[i]).gq(home, 0));
        }

        if (w.assigned() && h.assigned()) {
            IntArgs wc(n_boxes), hc(n_boxes);
            for (int i = 0; i < n_boxes; i++) {
                wc[i] = w[i].val();
                hc[i] = h[i].val();
            }
            no_overlap(home, x0, wc, y0, hc, m);
        } else if (optional(m)) {
            OptBox<FlexDim, 2>* b = static_cast<Space&>(home).alloc<OptBox<FlexDim, 2>>(n_boxes);
            for (int i = 0; i < n_boxes; i++) {
                b[i][0] = FlexDim(x0[i], w[i], x1[i]);
                b[i][1] = FlexDim(y0[i], h[i], y1[i]);
                b[i].optional(m[i]);
            }
            GECODE_ES_FAIL((NoOverlap::OptProp<OptBox<FlexDim, 2>>::post(home, b, n_boxes)));
        } else {
            ManBox<FlexDim, 2>* b = static_cast<Space&>(home).alloc<ManBox<FlexDim, 2>>(n_boxes);
            int n = 0;
            for (int i = 0; i < n_boxes; i++)
                if (m[i].one()) {
                    b[n][0] = FlexDim(x0[i], w[i], x1[i]);
                    b[n][1] = FlexDim(y0[i], h[i], y1[i]);
                    n++;
                }
            GECODE_ES_FAIL((NoOverlap::ManProp<ManBox<FlexDim, 2>>::post(home, b, n)));
        }
    }

} // namespace Gecode

// STATISTICS: int-post
