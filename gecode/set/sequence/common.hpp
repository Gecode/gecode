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

namespace Gecode::Set::Sequence {

    inline ExecStatus
    propagate_seq(Space& home, bool& modified, bool& assigned, ViewArray<SetView>& x) {
        int last_elem = x.size() - 1;
        int cur_max = BndSet::MAX_OF_EMPTY;
        int cur_min = BndSet::MIN_OF_EMPTY;

        Region r;
        Support::DynamicArray<int, Region> ub(r);

        for (int i = 0; i < last_elem; i++) {
            if (x[i].glb_size() > 0)
                cur_max = std::max(cur_max, x[i].glb_max());
            if (x[i].card_min() > 0)
                cur_max = std::max(cur_max, x[i].lub_min_n(x[i].card_min() - 1));
            if (cur_max >= Limits::min)
                GECODE_SET_ME_CHECK_VAL_B(modified, x[i + 1].exclude(home, Limits::min, cur_max), assigned);

            if (x[last_elem - i].lub_size() > 0) {
                cur_min = std::min(cur_min, x[last_elem - i].glb_min());
                if (x[last_elem - i].card_min() > 0) {
                    // Compute n-th largest element in x[lastElem-i].lub
                    // for n = x[lastElem-i].card_min()-1
                    int max_n = BndSet::MAX_OF_EMPTY;
                    int j = 0;
                    for (LubRanges<SetView> ubr(x[last_elem - i]); ubr(); ++ubr, ++j) {
                        ub[2 * j] = ubr.min();
                        ub[2 * j + 1] = ubr.max();
                    }
                    unsigned int xcm = x[last_elem - i].card_min() - 1;
                    while (j--) {
                        unsigned int width = static_cast<unsigned int>(ub[2 * j + 1] - ub[2 * j] + 1);
                        if (width > xcm) {
                            max_n = static_cast<int>(ub[2 * j + 1] - xcm);
                            break;
                        }
                        xcm -= width;
                    }
                    cur_min = std::min(cur_min, max_n);
                }
            }
            if (Limits::max >= cur_min)
                GECODE_SET_ME_CHECK_VAL_B(modified, x[last_elem - i - 1].exclude(home, cur_min, Limits::max), assigned);
        }
        return ES_NOFIX;
    }

} // namespace Gecode::Set::Sequence

// STATISTICS: set-prop
