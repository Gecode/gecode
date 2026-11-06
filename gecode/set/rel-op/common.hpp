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

#ifndef GECODE_SET_RELOP_COMM_ICC
#define GECODE_SET_RELOP_COMM_ICC

namespace Gecode {

    template <class View0, class View1>
    forceinline bool
    view_array_shared(const ViewArray<View0>& va, const View1& y) {
        return shared(va, y);
    }

    template <>
    forceinline bool
    view_array_shared<Set::SingletonView, Set::SetView>(const ViewArray<Set::SingletonView>&, const Set::SetView&) {
        return false;
    }

    /// \cond INTERNAL
    template <>
    forceinline bool
    view_array_shared<Set::ComplementView<Set::SingletonView>, Set::SetView>(
        const ViewArray<Set::ComplementView<Set::SingletonView>>&, const Set::SetView&) {
        return false;
    }

    template <>
    forceinline bool
    view_array_shared<Set::ComplementView<Set::SingletonView>, Set::ComplementView<Set::SetView>>(
        const ViewArray<Set::ComplementView<Set::SingletonView>>&, const Set::ComplementView<Set::SetView>&) {
        return false;
    }
    /// \endcond

    namespace Set::RelOp {

        /*
         * Detect sharing between 3 variables
         *
         */
        template <class View0, class View1, class View2>
        forceinline bool
        shared(View0 v0, View1 v1, View2 v2) {
            return shared(v0, v1) || shared(v0, v2) || shared(v1, v2);
        }

        template <class View0, class View1, class View2>
        ExecStatus
        inter_card(Space& home, bool& retmodified, View0& x0, View1& x1, View2& x2) {
            bool modified = false;
            do {
                retmodified |= modified;
                modified = false;

                {
                    LubRanges<View0> x0ub(x0);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Union<LubRanges<View0>, LubRanges<View1>> u1(x0ub, x1ub);
                    unsigned int s1 = Iter::Ranges::size(u1);

                    if (x0.card_min() + x1.card_min() > s1) {
                        GECODE_ME_CHECK_MODIFIED(modified, x2.card_min(home, x0.card_min() + x1.card_min() - s1));
                    }

                    // unsigned int res = std::max(x0.card_min()+
                    //                             (x1.card_min()<s1 ?
                    //                              0 : x1.card_min()-s1),
                    //                             std::max(x0.card_min(),
                    //                                      x1.card_min()));
                    // GECODE_ME_CHECK_MODIFIED(modified, x2.card_min(home,res));
                }

                {
                    GlbRanges<View0> x0lb(x0);
                    GlbRanges<View1> x1lb(x1);
                    Iter::Ranges::Union<GlbRanges<View0>, GlbRanges<View1>> u1(x0lb, x1lb);
                    unsigned int s1 = Iter::Ranges::size(u1);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.card_max(home, x0.card_max() + x1.card_max() - s1));
                }

                if (x2.card_max() < x1.card_min())
                    GECODE_ME_CHECK_MODIFIED(modified,
                                             x0.card_max(home, Set::Limits::card + x2.card_max() - x1.card_min()));

                if (x2.card_max() < x0.card_min())
                    GECODE_ME_CHECK_MODIFIED(modified,
                                             x1.card_max(home, Set::Limits::card + x2.card_max() - x0.card_min()));

                GECODE_ME_CHECK_MODIFIED(modified, x0.card_min(home, x2.card_min()));
                GECODE_ME_CHECK_MODIFIED(modified, x1.card_min(home, x2.card_min()));
            } while (modified);
            return ES_FIX;
        }
        template <class View0, class View1, class View2>
        ExecStatus
        union_card(Space& home, bool& retmodified, View0& x0, View1& x1, View2& x2) {
            bool modified = false;
            do {
                retmodified |= modified;
                modified = false;

                {
                    LubRanges<View0> x0ub(x0);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Inter<LubRanges<View0>, LubRanges<View1>> i1(x0ub, x1ub);
                    unsigned int s1 = Iter::Ranges::size(i1);
                    unsigned int res = std::max(x0.card_min() + (x1.card_min() < s1 ? 0 : x1.card_min() - s1),
                                                std::max(x0.card_min(), x1.card_min()));
                    GECODE_ME_CHECK_MODIFIED(modified, x2.card_min(home, res));
                }

                {
                    LubRanges<View0> x0ub(x0);
                    LubRanges<View1> x1ub(x1);
                    Iter::Ranges::Union<LubRanges<View0>, LubRanges<View1>> u1(x0ub, x1ub);
                    unsigned int s1 = Iter::Ranges::size(u1);
                    GECODE_ME_CHECK_MODIFIED(modified, x2.card_max(home, std::min(x0.card_max() + x1.card_max(), s1)));
                }

                if (x2.card_min() > x1.card_max())
                    GECODE_ME_CHECK_MODIFIED(modified, x0.card_min(home, x2.card_min() - x1.card_max()));

                if (x2.card_min() > x0.card_max())
                    GECODE_ME_CHECK_MODIFIED(modified, x1.card_min(home, x2.card_min() - x0.card_max()));

                GECODE_ME_CHECK_MODIFIED(modified, x0.card_max(home, x2.card_max()));
                GECODE_ME_CHECK_MODIFIED(modified, x1.card_max(home, x2.card_max()));
            } while (modified);
            return ES_FIX;
        }

        template <class View0, class View1>
        ExecStatus
        union_n_card(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            int xsize = x.size();
            // Max(Xi.card_min) <= y.card <= Sum(Xi.card_max)
            // Xi.card <=y.card_max
            unsigned int card_max_sum = union_of_dets.size();
            bool max_valid = true;
            for (int i = xsize; i--;) {
                card_max_sum += x[i].card_max();
                if (card_max_sum < x[i].card_max()) {
                    max_valid = false;
                } // overflow
                GECODE_ME_CHECK_MODIFIED(modified, y.card_min(home, x[i].card_min()));
                GECODE_ME_CHECK_MODIFIED(modified, x[i].card_max(home, y.card_max()));
            }
            if (max_valid) {
                GECODE_ME_CHECK_MODIFIED(modified, y.card_max(home, card_max_sum));
            }
            // y.card_min - Sum(Xj.card_max) <= Xi.card

            if (x.size() == 0)
                return ES_NOFIX;

            Region r;
            // TODO: overflow management is a waste now.
            {
                unsigned int* right_sum = r.alloc<unsigned int>(xsize);
                right_sum[xsize - 1] = 0;

                for (int i = x.size() - 1; i--;) {
                    right_sum[i] = right_sum[i + 1] + x[i + 1].card_max();
                    if (right_sum[i] < right_sum[i + 1]) {
                        // overflow, fill the rest of the array.
                        for (int j = i; j > 0; j--) {
                            right_sum[j] = Limits::card;
                        }
                        break;
                    }
                }

                // Size of union of determied vars missing from x sneaked in here:
                unsigned int left_acc = union_of_dets.size();

                for (int i = 0; i < xsize; i++) {
                    unsigned int jsum = left_acc + right_sum[i];
                    // If jsum did not overflow and is less than y.card_min:
                    if (jsum >= left_acc && jsum < y.card_min()) {
                        GECODE_ME_CHECK_MODIFIED(modified, x[i].card_min(home, y.card_min() - jsum));
                    }
                    left_acc += x[i].card_max();
                    if (left_acc < x[i].card_max()) {
                        left_acc = Limits::card;
                    }
                }
            }

            // y.card_min - |U(Xj.ub)| <= Xi.card

            {
                GLBndSet* right_union = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));
                new (&right_union[xsize - 1]) GLBndSet(home);
                for (int i = xsize - 1; i--;) {
                    BndSetRanges prev(right_union[i + 1]);
                    LubRanges<View0> prev_x(x[i + 1]);
                    Iter::Ranges::Union<BndSetRanges, LubRanges<View0>> iter(prev, prev_x);
                    new (&right_union[i]) GLBndSet(home);
                    right_union[i].include_i(home, iter);
                }

                // union of determied vars missing from x sneaked in here:
                GLBndSet left_acc;
                left_acc.update(home, union_of_dets);
                for (int i = 0; i < xsize; i++) {
                    BndSetRanges left(left_acc);
                    BndSetRanges right(right_union[i]);
                    Iter::Ranges::Union<BndSetRanges, BndSetRanges> iter(left, right);
                    unsigned int union_size = Iter::Ranges::size(iter);
                    if (y.card_min() > union_size) {
                        GECODE_ME_CHECK_MODIFIED(modified, x[i].card_min(home, y.card_min() - union_size));
                    }
                    LubRanges<View0> xiub(x[i]);
                    left_acc.include_i(home, xiub);
                }

                for (int i = xsize; i--;)
                    right_union[i].dispose(home);
                left_acc.dispose(home);
            }

            // no need for this: |y.lb - U(Xj.card_max)| <= S.card

            return ES_NOFIX;
        }

        /*
         * Xi UB is subset of YUB
         * Subscribes to Y UB
         */
        template <class View0, class View1>
        ExecStatus
        union_n_xi_ub(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet&) {
            int xsize = x.size();
            for (int i = xsize; i--;) {
                LubRanges<View1> yub(y);
                GECODE_ME_CHECK_MODIFIED(modified, x[i].intersect_i(home, yub));
            }
            return ES_FIX;
        }

        // cardinality rules for PartitionN constraint
        template <class View0, class View1>
        ExecStatus
        partition_n_card(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            unsigned int card_min_sum = union_of_dets.size();
            unsigned int card_max_sum = union_of_dets.size();
            int xsize = x.size();
            for (int i = xsize; i--;) {
                card_min_sum += x[i].card_min();
                if (card_min_sum < x[i].card_min()) {
                    // sum of mins overflows: fail the space.
                    GECODE_ME_CHECK(ME_SET_FAILED);
                }
            }
            GECODE_ME_CHECK_MODIFIED(modified, y.card_min(home, card_min_sum));
            for (int i = xsize; i--;) {
                card_max_sum += x[i].card_max();
                if (card_max_sum < x[i].card_max()) {
                    // sum of maxes overflows: no useful information to tell.
                    goto overflow;
                }
            }
            GECODE_ME_CHECK_MODIFIED(modified, y.card_max(home, card_max_sum));

            if (x.size() == 0)
                return ES_NOFIX;

        overflow:

            // Cardinality of each x[i] limited by cardinality of y minus all x[j]s:

            {
                Region r;
                unsigned int* right_min_sum = r.alloc<unsigned int>(xsize);
                unsigned int* right_max_sum = r.alloc<unsigned int>(xsize);
                right_min_sum[xsize - 1] = 0;
                right_max_sum[xsize - 1] = 0;

                for (int i = x.size() - 1; i--;) {
                    right_max_sum[i] = right_max_sum[i + 1] + x[i + 1].card_max();
                    if (right_max_sum[i] < right_max_sum[i + 1]) {
                        // overflow, fill the rest of the array.
                        for (int j = i; j > 0; j--) {
                            right_max_sum[j] = Limits::card;
                        }
                        break;
                    }
                }
                for (int i = x.size() - 1; i--;) {
                    right_min_sum[i] = right_min_sum[i + 1] + x[i + 1].card_min();
                    if (right_min_sum[i] < right_min_sum[i + 1]) {
                        // overflow, fail the space
                        GECODE_ME_CHECK(ME_SET_FAILED);
                    }
                }
                unsigned int left_min_acc = union_of_dets.size();
                unsigned int left_max_acc = union_of_dets.size();

                for (int i = 0; i < xsize; i++) {
                    unsigned int max_sum = left_max_acc + right_max_sum[i];
                    unsigned int min_sum = left_min_acc + right_min_sum[i];
                    // If maxSum did not overflow and is less than y.card_min:
                    if (max_sum >= left_max_acc && max_sum < y.card_min()) {
                        GECODE_ME_CHECK_MODIFIED(modified, x[i].card_min(home, y.card_min() - max_sum));
                    }

                    // Overflow, fail.
                    if (min_sum < left_min_acc || y.card_max() < min_sum) {
                        GECODE_ME_CHECK(ME_SET_FAILED);
                    } else {
                        GECODE_ME_CHECK_MODIFIED(modified, x[i].card_max(home, y.card_max() - min_sum));
                    }

                    left_max_acc += x[i].card_max();
                    if (left_max_acc < x[i].card_max())
                        left_max_acc = Limits::card;
                    left_min_acc += x[i].card_min();
                    if (left_min_acc < x[i].card_min())
                        GECODE_ME_CHECK(ME_SET_FAILED);
                }
            }

            return ES_NOFIX;
        }

        // Xi LB includes YLB minus union Xj UB
        // Xi UB is subset of YUB minus union of Xj LBs
        template <class View0, class View1>
        ExecStatus
        partition_n_xi(Space& home, bool& modified, ViewArray<View0>& x, View1& y) {
            int xsize = x.size();
            Region r;
            GLBndSet* after_ub = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));
            GLBndSet* after_lb = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));

            {
                GLBndSet sofar_after_ub;
                GLBndSet sofar_after_lb;
                for (int i = xsize; i--;) {
                    new (&after_ub[i]) GLBndSet(home);
                    new (&after_lb[i]) GLBndSet(home);
                    after_ub[i].update(home, sofar_after_ub);
                    after_lb[i].update(home, sofar_after_lb);
                    LubRanges<View0> xiub(x[i]);
                    GlbRanges<View0> xilb(x[i]);
                    sofar_after_ub.include_i(home, xiub);
                    sofar_after_lb.include_i(home, xilb);
                }
                sofar_after_ub.dispose(home);
                sofar_after_lb.dispose(home);
            }

            {
                GLBndSet sofar_before_ub;
                GLBndSet sofar_before_lb;
                for (int i = 0; i < xsize; i++) {
                    LubRanges<View1> yub(y);
                    BndSetRanges slb(sofar_before_lb);
                    BndSetRanges after_lower_bound_ranges(after_lb[i]);
                    Iter::Ranges::Union<BndSetRanges, BndSetRanges> xjlb(slb, after_lower_bound_ranges);
                    Iter::Ranges::Diff<LubRanges<View1>, Iter::Ranges::Union<BndSetRanges, BndSetRanges>> diff1(yub,
                                                                                                                xjlb);
                    GECODE_ME_CHECK_MODIFIED(modified, x[i].intersect_i(home, diff1));

                    GlbRanges<View1> ylb(y);
                    BndSetRanges sub(sofar_before_ub);
                    BndSetRanges after_upper_bound_ranges(after_ub[i]);
                    Iter::Ranges::Union<BndSetRanges, BndSetRanges> xjub(sub, after_upper_bound_ranges);
                    Iter::Ranges::Diff<GlbRanges<View1>, Iter::Ranges::Union<BndSetRanges, BndSetRanges>> diff2(ylb,
                                                                                                                xjub);
                    GECODE_ME_CHECK_MODIFIED(modified, x[i].include_i(home, diff2));

                    LubRanges<View0> xiub(x[i]);
                    GlbRanges<View0> xilb(x[i]);
                    sofar_before_ub.include_i(home, xiub);
                    sofar_before_lb.include_i(home, xilb);
                }
                sofar_before_lb.dispose(home);
                sofar_before_ub.dispose(home);
            }

            for (int i = xsize; i--;) {
                after_ub[i].dispose(home);
                after_lb[i].dispose(home);
            }

            return ES_NOFIX;
        }

        // Xi UB is subset of YUB minus union of Xj LBs
        template <class View0, class View1>
        ExecStatus
        partition_n_xi_ub(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            int xsize = x.size();
            Region r;
            GLBndSet* after_lb = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));

            {
                GLBndSet sofar_after_lb;
                for (int i = xsize; i--;) {
                    new (&after_lb[i]) GLBndSet(home);
                    after_lb[i].update(home, sofar_after_lb);
                    GlbRanges<View0> xilb(x[i]);
                    sofar_after_lb.include_i(home, xilb);
                }
                sofar_after_lb.dispose(home);
            }

            {
                GLBndSet sofar_before_lb;
                sofar_before_lb.update(home, union_of_dets);
                for (int i = 0; i < xsize; i++) {
                    LubRanges<View1> yub(y);
                    BndSetRanges slb(sofar_before_lb);
                    BndSetRanges after_lower_bound_ranges(after_lb[i]);
                    Iter::Ranges::Union<BndSetRanges, BndSetRanges> xjlb(slb, after_lower_bound_ranges);
                    Iter::Ranges::Diff<LubRanges<View1>, Iter::Ranges::Union<BndSetRanges, BndSetRanges>> diff1(yub,
                                                                                                                xjlb);
                    GECODE_ME_CHECK_MODIFIED(modified, x[i].intersect_i(home, diff1));

                    GlbRanges<View0> xilb(x[i]);
                    sofar_before_lb.include_i(home, xilb);
                }
                sofar_before_lb.dispose(home);
            }
            for (int i = xsize; i--;)
                after_lb[i].dispose(home);
            return ES_NOFIX;
        }

        // Xi LB includes YLB minus union Xj UB
        template <class View0, class View1>
        ExecStatus
        partition_n_xi_lb(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            int xsize = x.size();
            Region r;
            GLBndSet* after_ub = static_cast<GLBndSet*>(r.ralloc(sizeof(GLBndSet) * xsize));

            {
                GLBndSet sofar_after_ub;
                for (int i = xsize; i--;) {
                    new (&after_ub[i]) GLBndSet(home);
                    after_ub[i].update(home, sofar_after_ub);
                    LubRanges<View0> xiub(x[i]);
                    sofar_after_ub.include_i(home, xiub);
                }
                sofar_after_ub.dispose(home);
            }

            {
                // The union of previously determined x[j]-s is added to the mix here:
                GLBndSet sofar_before_ub;
                sofar_before_ub.update(home, union_of_dets);
                for (int i = 0; i < xsize; i++) {
                    GlbRanges<View1> ylb(y);
                    BndSetRanges sub(sofar_before_ub);
                    BndSetRanges after_upper_bound_ranges(after_ub[i]);
                    Iter::Ranges::Union<BndSetRanges, BndSetRanges> xjub(sub, after_upper_bound_ranges);
                    Iter::Ranges::Diff<GlbRanges<View1>, Iter::Ranges::Union<BndSetRanges, BndSetRanges>> diff2(ylb,
                                                                                                                xjub);
                    GECODE_ME_CHECK_MODIFIED(modified, x[i].include_i(home, diff2));

                    LubRanges<View0> xiub(x[i]);
                    sofar_before_ub.include_i(home, xiub);
                }
                sofar_before_ub.dispose(home);
            }
            for (int i = xsize; i--;)
                after_ub[i].dispose(home);
            return ES_NOFIX;
        }

        // Y LB contains union of X LBs
        template <class View0, class View1>
        ExecStatus
        partition_nylb(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            assert(union_of_dets.is_consistent());
            int xsize = x.size();
            Region reg;
            GlbRanges<View0>* x_l_bs = reg.alloc<GlbRanges<View0>>(xsize);
            int non_empty_counter = 0;
            for (int i = xsize; i--;) {
                GlbRanges<View0> r(x[i]);
                if (r()) {
                    x_l_bs[non_empty_counter] = r;
                    non_empty_counter++;
                }
            }
            if (non_empty_counter != 0) {
                Iter::Ranges::NaryUnion x_lb_union(reg, x_l_bs, non_empty_counter);
                BndSetRanges dets(union_of_dets);
                x_lb_union |= dets;
                GECODE_ME_CHECK_MODIFIED(modified, y.include_i(home, x_lb_union));
            }
            return ES_FIX;
        }

        // Y UB is subset of union of X UBs
        template <class View0, class View1>
        ExecStatus
        partition_nyub(Space& home, bool& modified, ViewArray<View0>& x, View1& y, GLBndSet& union_of_dets) {
            int xsize = x.size();
            Region reg;
            LubRanges<View0>* x_u_bs = reg.alloc<LubRanges<View0>>(xsize);
            int non_empty_counter = 0;
            for (int i = xsize; i--;) {
                LubRanges<View0> r(x[i]);
                if (r()) {
                    x_u_bs[non_empty_counter] = r;
                    non_empty_counter++;
                }
            }
            if (non_empty_counter != 0) {
                Iter::Ranges::NaryUnion x_ub_union(reg, x_u_bs, non_empty_counter);
                BndSetRanges dets(union_of_dets);
                x_ub_union |= dets;
                GECODE_ME_CHECK_MODIFIED(modified, y.intersect_i(home, x_ub_union));
            }
            return ES_FIX;
        }

    } // namespace Set::RelOp
} // namespace Gecode

#endif

// STATISTICS: set-prop
