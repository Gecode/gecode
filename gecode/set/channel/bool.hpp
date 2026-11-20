/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
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

#include <gecode/int.hh>

namespace Gecode { namespace Set { namespace Channel {

    template <class View>
    template <class A>
    forceinline
    ChannelBool<View>::IndexAdvisor::IndexAdvisor(Space& home, ChannelBool<View>& p, Council<A>& c, int index)
        : Advisor(home, p, c), idx(index) {
        if (idx == -1)
            p.y.subscribe(home, *this);
        else {
            p.x[idx].subscribe(home, *this);
        }
    }

    template <class View>
    forceinline
    ChannelBool<View>::IndexAdvisor::IndexAdvisor(Space& home, IndexAdvisor& a)
        : Advisor(home, a), idx(a.idx) {}

    template <class View>
    forceinline int
    ChannelBool<View>::IndexAdvisor::index(void) const {
        return idx;
    }

    template <class View>
    template <class A>
    forceinline void
    ChannelBool<View>::IndexAdvisor::dispose(Space& home, Council<A>& c) {
        ChannelBool<View>& p = static_cast<ChannelBool<View>&>(propagator());
        if (idx == -1)
            p.y.cancel(home, *this);
        else {
            p.x[idx].cancel(home, *this);
        }
        Advisor::dispose(home, c);
    }

    template <class View>
    forceinline
    ChannelBool<View>::ChannelBool(Home home, ViewArray<Gecode::Int::BoolView>& x0, View y0)
        : Super(home, x0, y0), co(home), running(false) {
        bool assigned = false;
        for (int i = x.size(); i--;) {
            if (x[i].zero()) {
                assigned = true;
                SetDelta d;
                zeros.include(home, i, i, d);
            } else if (x[i].one()) {
                assigned = true;
                SetDelta d;
                ones.include(home, i, i, d);
            } else {
                (void)new (home) IndexAdvisor(home, *this, co, i);
            }
        }
        if (assigned)
            Gecode::Int::BoolView::schedule(home, *this, Gecode::Int::ME_BOOL_VAL);
        View::schedule(home, *this, y.assigned() ? ME_SET_VAL : ME_SET_BB);
        if (y.assigned()) {
            if (y.glb_size() == static_cast<unsigned int>(y.glb_max() - y.glb_min() + 1)) {
                new (&delta) SetDelta(y.glb_min(), y.glb_max(), 2, 0);
            } else {
                new (&delta) SetDelta(2, 0, 2, 0);
            }
        }
        (void)new (home) IndexAdvisor(home, *this, co, -1);
    }

    template <class View>
    forceinline
    ChannelBool<View>::ChannelBool(Space& home, ChannelBool& p)
        : Super(home, p), running(false) {
        co.update(home, p.co);
    }

    template <class View>
    forceinline ExecStatus
    ChannelBool<View>::post(Home home, ViewArray<Gecode::Int::BoolView>& x, View y) {
        if (x.size() == 0) {
            GECODE_ME_CHECK(y.card_max(home, 0));
            return ES_OK;
        }
        GECODE_ME_CHECK(y.intersect(home, 0, x.size() - 1));
        (void)new (home) ChannelBool(home, x, y);
        return ES_OK;
    }

    template <class View>
    PropCost
    ChannelBool<View>::cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO, x.size() + 1);
    }

    template <class View>
    void
    ChannelBool<View>::reschedule(Space& home) {
        x.reschedule(home, *this, Gecode::Int::PC_BOOL_VAL);
        View::schedule(home, *this, y.assigned() ? ME_SET_VAL : ME_SET_BB);
    }

    template <class View>
    forceinline size_t
    ChannelBool<View>::dispose(Space& home) {
        co.dispose(home);
        (void)Super::dispose(home);
        return sizeof(*this);
    }

    template <class View>
    Actor*
    ChannelBool<View>::copy(Space& home) {
        return new (home) ChannelBool(home, *this);
    }

    template <class View>
    ExecStatus
    ChannelBool<View>::propagate(Space& home, const ModEventDelta&) {
        running = true;
        if (zeros.size() > 0) {
            BndSetRanges zi(zeros);
            GECODE_ME_CHECK(y.exclude_i(home, zi));
            zeros.init(home);
        }
        if (ones.size() > 0) {
            BndSetRanges oi(ones);
            GECODE_ME_CHECK(y.include_i(home, oi));
            ones.init(home);
        }
        running = false;

        if (delta.glb_min() != 1 || delta.glb_max() != 0) {
            if (!delta.glb_any()) {
                for (int i = delta.glb_min(); i <= delta.glb_max(); i++)
                    GECODE_ME_CHECK(x[i].one(home));
            } else {
                GlbRanges<View> glb(y);
                for (Iter::Ranges::ToValues<GlbRanges<View>> gv(glb); gv(); ++gv) {
                    GECODE_ME_CHECK(x[gv.val()].one(home));
                }
            }
        }
        if (delta.lub_min() != 1 || delta.lub_max() != 0) {
            if (!delta.lub_any()) {
                for (int i = delta.lub_min(); i <= delta.lub_max(); i++)
                    GECODE_ME_CHECK(x[i].zero(home));
            } else {
                int cur = 0;
                for (LubRanges<View> lub(y); lub(); ++lub) {
                    for (; cur < lub.min(); cur++) {
                        GECODE_ME_CHECK(x[cur].zero(home));
                    }
                    cur = lub.max() + 1;
                }
                for (; cur < x.size(); cur++) {
                    GECODE_ME_CHECK(x[cur].zero(home));
                }
            }
        }

        new (&delta) SetDelta();

        return y.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
    }

    template <class View>
    ExecStatus
    ChannelBool<View>::advise(Space& home, Advisor& _a, const Delta& _d) {
        IndexAdvisor& a = static_cast<IndexAdvisor&>(_a);
        const SetDelta& d = static_cast<const SetDelta&>(_d);

        ModEvent me = View::modification_event(d);
        int index = a.index();
        if ((running && index == -1 && me != ME_SET_VAL) || (index == -1 && me == ME_SET_CARD)) {
            return ES_OK;
        }

        if (index >= 0) {
            if (x[index].zero()) {
                SetDelta dummy;
                zeros.include(home, index, index, dummy);
            } else {
                assert(x[index].one());
                SetDelta dummy;
                ones.include(home, index, index, dummy);
            }
            return home.ES_NOFIX_DISPOSE(co, a);
        }

        if ((a.index() == -1) && Rel::test_set_event_lb(me)) {
            if (d.glb_any()) {
                new (&delta) SetDelta(2, 0, delta.lub_min(), delta.lub_max());
            } else {
                if (delta.glb_min() == 1 && delta.glb_max() == 0) {
                    new (&delta) SetDelta(d.glb_min(), d.glb_max(), delta.lub_min(), delta.lub_max());
                } else {
                    if (delta.glb_min() != 2 || delta.glb_max() != 0) {
                        if ((delta.glb_min() <= d.glb_min() && delta.glb_max() >= d.glb_min()) ||
                            (delta.glb_min() <= d.glb_max() && delta.glb_max() >= d.glb_max())) {
                            new (&delta)
                                SetDelta(std::min(delta.glb_min(), d.glb_min()), std::max(delta.glb_max(), d.glb_max()),
                                         delta.lub_min(), delta.lub_max());
                        } else {
                            new (&delta) SetDelta(2, 0, delta.lub_min(), delta.lub_max());
                        }
                    }
                }
            }
        }
        if ((a.index() == -1) && Rel::test_set_event_ub(me)) {
            if (d.lub_any()) {
                new (&delta) SetDelta(delta.glb_min(), delta.glb_max(), 2, 0);
            } else {
                if (delta.lub_min() == 1 && delta.lub_max() == 0) {
                    new (&delta) SetDelta(delta.glb_min(), delta.glb_max(), d.lub_min(), d.lub_max());
                } else {
                    if (delta.lub_min() != 2 || delta.lub_max() != 0) {
                        if ((delta.lub_min() <= d.lub_min() && delta.lub_max() >= d.lub_min()) ||
                            (delta.lub_min() <= d.lub_max() && delta.lub_max() >= d.lub_max())) {
                            new (&delta)
                                SetDelta(delta.lub_min(), delta.lub_max(), std::min(delta.lub_min(), d.lub_min()),
                                         std::max(delta.lub_max(), d.lub_max()));
                        } else {
                            new (&delta) SetDelta(delta.glb_min(), delta.glb_max(), 2, 0);
                        }
                    }
                }
            }
        }
        if (y.assigned())
            return home.ES_NOFIX_DISPOSE(co, a);
        else
            return ES_NOFIX;
    }

}}} // namespace Gecode::Set::Channel

// STATISTICS: set-prop
