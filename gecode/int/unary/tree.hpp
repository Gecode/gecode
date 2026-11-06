/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <algorithm>

namespace Gecode { namespace Int { namespace Unary {

    /*
     * Omega tree
     */

    forceinline void
    OmegaNode::init(const OmegaNode&, const OmegaNode&) {
        p = 0;
        ect = -Limits::infinity;
    }

    forceinline void
    OmegaNode::update(const OmegaNode& l, const OmegaNode& r) {
        p = l.p + r.p;
        ect = std::max(plus(l.ect, r.p), r.ect);
    }

    template <class TaskView>
    forceinline
    OmegaTree<TaskView>::OmegaTree(Region& r, const TaskViewArray<TaskView>& t)
        : TaskTree<TaskView, OmegaNode>(r, t) {
        for (int i = 0; i < tasks.size(); i++) {
            leaf(i).p = 0;
            leaf(i).ect = -Limits::infinity;
        }
        init();
    }

    template <class TaskView>
    forceinline void
    OmegaTree<TaskView>::insert(int i) {
        leaf(i).p = tasks[i].pmin();
        leaf(i).ect = tasks[i].est() + tasks[i].pmin();
        update(i);
    }

    template <class TaskView>
    forceinline void
    OmegaTree<TaskView>::remove(int i) {
        leaf(i).p = 0;
        leaf(i).ect = -Limits::infinity;
        update(i);
    }

    template <class TaskView>
    forceinline int
    OmegaTree<TaskView>::ect() const {
        return root().ect;
    }

    template <class TaskView>
    forceinline int
    OmegaTree<TaskView>::ect(int i) const {
        // Check whether task i is in?
        OmegaTree<TaskView>& o = const_cast<OmegaTree<TaskView>&>(*this);
        if (o.leaf(i).ect != -Limits::infinity) {
            o.remove(i);
            int ect = o.root().ect;
            o.insert(i);
            return ect;
        }
        return root().ect;
    }

    /*
     * Omega lambda tree
     */

    forceinline void
    OmegaLambdaNode::init(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
        OmegaNode::init(l, r);
        lp = p;
        lect = ect;
        res_ect = undef;
        res_lp = undef;
    }

    forceinline void
    OmegaLambdaNode::update(const OmegaLambdaNode& l, const OmegaLambdaNode& r) {
        OmegaNode::update(l, r);
        if (l.lp + r.p > l.p + r.lp) {
            res_lp = l.res_lp;
            lp = l.lp + r.p;
        } else {
            res_lp = r.res_lp;
            lp = l.p + r.lp;
        }
        if ((r.lect >= plus(l.ect, r.lp)) && (r.lect >= plus(l.lect, r.p))) {
            lect = r.lect;
            res_ect = r.res_ect;
        } else if (plus(l.ect, r.lp) >= plus(l.lect, r.p)) {
            assert(plus(l.ect, r.lp) > r.lect);
            lect = plus(l.ect, r.lp);
            res_ect = r.res_lp;
        } else {
            assert((plus(l.lect, r.p) > r.lect) && (plus(l.lect, r.p) > plus(l.ect, r.lp)));
            lect = plus(l.lect, r.p);
            res_ect = l.res_ect;
        }
    }

    template <class TaskView>
    forceinline
    OmegaLambdaTree<TaskView>::OmegaLambdaTree(Region& r, const TaskViewArray<TaskView>& t, bool inc)
        : TaskTree<TaskView, OmegaLambdaNode>(r, t) {
        if (inc) {
            // Enter all tasks into tree (omega = all tasks, lambda = empty)
            for (int i = 0; i < tasks.size(); i++) {
                leaf(i).p = leaf(i).lp = tasks[i].pmin();
                leaf(i).ect = leaf(i).lect = tasks[i].est() + tasks[i].pmin();
                leaf(i).res_ect = OmegaLambdaNode::undef;
                leaf(i).res_lp = OmegaLambdaNode::undef;
            }
            update();
        } else {
            // Enter no tasks into tree (omega = empty, lambda = empty)
            for (int i = 0; i < tasks.size(); i++) {
                leaf(i).p = leaf(i).lp = 0;
                leaf(i).ect = leaf(i).lect = -Limits::infinity;
                leaf(i).res_ect = OmegaLambdaNode::undef;
                leaf(i).res_lp = OmegaLambdaNode::undef;
            }
            init();
        }
    }

    template <class TaskView>
    forceinline void
    OmegaLambdaTree<TaskView>::shift(int i) {
        // That means that i is in omega
        assert(leaf(i).ect > -Limits::infinity);
        leaf(i).p = 0;
        leaf(i).ect = -Limits::infinity;
        leaf(i).res_ect = i;
        leaf(i).res_lp = i;
        update(i);
    }

    template <class TaskView>
    forceinline void
    OmegaLambdaTree<TaskView>::insert_omega(int i) {
        leaf(i).p = tasks[i].pmin();
        leaf(i).ect = tasks[i].est() + tasks[i].pmin();
        update(i);
    }

    template <class TaskView>
    forceinline void
    OmegaLambdaTree<TaskView>::insert_lambda(int i) {
        leaf(i).lp = tasks[i].pmin();
        leaf(i).lect = tasks[i].est() + tasks[i].pmin();
        leaf(i).res_ect = i;
        leaf(i).res_lp = i;
        update(i);
    }

    template <class TaskView>
    forceinline void
    OmegaLambdaTree<TaskView>::remove_lambda(int i) {
        leaf(i).lp = 0;
        leaf(i).lect = -Limits::infinity;
        leaf(i).res_ect = OmegaLambdaNode::undef;
        leaf(i).res_lp = OmegaLambdaNode::undef;
        update(i);
    }

    template <class TaskView>
    forceinline bool
    OmegaLambdaTree<TaskView>::lempty() const {
        return root().res_ect < 0;
    }

    template <class TaskView>
    forceinline int
    OmegaLambdaTree<TaskView>::responsible() const {
        return root().res_ect;
    }

    template <class TaskView>
    forceinline int
    OmegaLambdaTree<TaskView>::ect() const {
        return root().ect;
    }

    template <class TaskView>
    forceinline int
    OmegaLambdaTree<TaskView>::lect() const {
        return root().lect;
    }

}}} // namespace Gecode::Int::Unary

// STATISTICS: int-prop
