/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *     Guido Tack, 2010
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

#include <gecode/int/unary.hh>
#include <gecode/int/distinct.hh>

#include <algorithm>

namespace Gecode {

    void
    unary(Home home, const IntVarArgs& s, const IntArgs& p, IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = s.size();
        if (same(s))
            throw Int::ArgumentSame("Int::unary");
        if (n_tasks != p.size())
            throw Int::ArgumentSizeMismatch("Int::unary");
        for (int i = 0; i < p.size(); i++) {
            Int::Limits::nonnegative(p[i], "Int::unary");
            Int::Limits::check(static_cast<long long int>(s[i].max()) + p[i], "Int::unary");
        }
        GECODE_POST;
        bool all_one = true;
        for (int i = 0; i < p.size(); i++) {
            if (p[i] != 1) {
                all_one = false;
                break;
            }
        }
        if (all_one) {
            ViewArray<IntView> xv(home, s);
            switch (vbd(ipl)) {
            case IPL_BND:
                GECODE_ES_FAIL(Distinct::Bnd<IntView>::post(home, xv));
                break;
            case IPL_DOM:
                GECODE_ES_FAIL(Distinct::Dom<IntView>::post(home, xv));
                break;
            default:
                GECODE_ES_FAIL(Distinct::Val<IntView>::post(home, xv));
            }
        } else {
            TaskArray<ManFixPTask> t(home, n_tasks);
            for (int i = 0; i < n_tasks; i++)
                t[i].init(s[i], p[i]);
            GECODE_ES_FAIL(post_mandatory(home, t, ipl));
        }
    }

    void
    unary(Home home, const TaskTypeArgs& t, const IntVarArgs& flex, const IntArgs& fix, IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = flex.size();
        if ((n_tasks != fix.size()) || (n_tasks != t.size()))
            throw Int::ArgumentSizeMismatch("Int::unary");
        for (int i = 0; i < fix.size(); i++) {
            if (t[i] == TT_FIXP)
                Int::Limits::nonnegative(fix[i], "Int::unary");
            else
                Int::Limits::check(fix[i], "Int::unary");
            Int::Limits::check(static_cast<long long int>(flex[i].max()) + fix[i], "Int::unary");
        }
        GECODE_POST;
        bool fixp = true;
        for (int i = 0; i < t.size(); i++)
            if (t[i] != TT_FIXP) {
                fixp = false;
                break;
            }
        if (fixp) {
            unary(home, flex, fix, ipl);
        } else {
            TaskArray<ManFixPSETask> tasks(home, n_tasks);
            for (int i = 0; i < n_tasks; i++)
                tasks[i].init(t[i], flex[i], fix[i]);
            GECODE_ES_FAIL(post_mandatory(home, tasks, ipl));
        }
    }

    void
    unary(Home home, const IntVarArgs& s, const IntArgs& p, const BoolVarArgs& m, IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = s.size();
        if (same(s))
            throw Int::ArgumentSame("Int::unary");
        if ((n_tasks != p.size()) || (n_tasks != m.size()))
            throw Int::ArgumentSizeMismatch("Int::unary");
        for (int i = 0; i < p.size(); i++) {
            Int::Limits::nonnegative(p[i], "Int::unary");
            Int::Limits::check(static_cast<long long int>(s[i].max()) + p[i], "Int::unary");
        }
        bool all_mandatory = true;
        for (int i = 0; i < m.size(); i++) {
            if (!m[i].one()) {
                all_mandatory = false;
                break;
            }
        }
        if (all_mandatory) {
            unary(home, s, p, ipl);
        } else {
            GECODE_POST;
            TaskArray<OptFixPTask> t(home, n_tasks);
            for (int i = 0; i < n_tasks; i++)
                t[i].init(s[i], p[i], m[i]);
            GECODE_ES_FAIL(post_optional(home, t, ipl));
        }
    }

    void
    unary(Home home, const TaskTypeArgs& t, const IntVarArgs& flex, const IntArgs& fix, const BoolVarArgs& m,
          IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = flex.size();
        if ((n_tasks != fix.size()) || (n_tasks != t.size()) || (n_tasks != m.size()))
            throw Int::ArgumentSizeMismatch("Int::unary");
        bool fixp = true;
        for (int i = 0; i < fix.size(); i++) {
            if (t[i] == TT_FIXP) {
                Int::Limits::nonnegative(fix[i], "Int::unary");
            } else {
                fixp = false;
                Int::Limits::check(fix[i], "Int::unary");
            }
            Int::Limits::check(static_cast<long long int>(flex[i].max()) + fix[i], "Int::unary");
        }
        GECODE_POST;
        bool all_mandatory = true;
        for (int i = 0; i < m.size(); i++) {
            if (!m[i].one()) {
                all_mandatory = false;
                break;
            }
        }
        if (all_mandatory) {
            unary(home, t, flex, fix, ipl);
        } else {
            if (fixp) {
                TaskArray<OptFixPTask> tasks(home, n_tasks);
                for (int i = 0; i < n_tasks; i++)
                    tasks[i].init(flex[i], fix[i], m[i]);
                GECODE_ES_FAIL(post_optional(home, tasks, ipl));
            } else {
                TaskArray<OptFixPSETask> tasks(home, n_tasks);
                for (int i = 0; i < n_tasks; i++)
                    tasks[i].init(t[i], flex[i], fix[i], m[i]);
                GECODE_ES_FAIL(post_optional(home, tasks, ipl));
            }
        }
    }

    void
    unary(Home home, const IntVarArgs& s, const IntVarArgs& p, const IntVarArgs& e, IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = s.size();
        if ((n_tasks != p.size()) || (n_tasks != e.size()))
            throw Int::ArgumentSizeMismatch("Int::unary");
        GECODE_POST;
        for (int i = 0; i < p.size(); i++) {
            IntView pi(p[i]);
            GECODE_ME_FAIL(pi.gq(home, 0));
        }
        bool fix_p = true;
        for (int i = 0; i < p.size(); i++) {
            if (!p[i].assigned()) {
                fix_p = false;
                break;
            }
        }
        if (fix_p) {
            IntArgs pp(p.size());
            for (int i = 0; i < p.size(); i++)
                pp[i] = p[i].val();
            unary(home, s, pp, ipl);
        } else {
            TaskArray<ManFlexTask> t(home, n_tasks);
            for (int i = 0; i < n_tasks; i++)
                t[i].init(s[i], p[i], e[i]);
            GECODE_ES_FAIL(post_mandatory(home, t, ipl));
        }
    }

    void
    unary(Home home, const IntVarArgs& s, const IntVarArgs& p, const IntVarArgs& e, const BoolVarArgs& m,
          IntPropLevel ipl) {
        using namespace Gecode::Int;
        using namespace Gecode::Int::Unary;
        const int n_tasks = s.size();
        if ((n_tasks != p.size()) || (n_tasks != m.size()) || (n_tasks != e.size()))
            throw Int::ArgumentSizeMismatch("Int::unary");
        GECODE_POST;
        for (int i = 0; i < p.size(); i++) {
            IntView pi(p[i]);
            GECODE_ME_FAIL(pi.gq(home, 0));
        }
        bool all_mandatory = true;
        for (int i = 0; i < m.size(); i++) {
            if (!m[i].one()) {
                all_mandatory = false;
                break;
            }
        }
        if (all_mandatory) {
            unary(home, s, p, e, ipl);
        } else {
            TaskArray<OptFlexTask> t(home, n_tasks);
            for (int i = 0; i < n_tasks; i++)
                t[i].init(s[i], p[i], e[i], m[i]);
            GECODE_ES_FAIL(post_optional(home, t, ipl));
        }
    }

} // namespace Gecode

// STATISTICS: int-post
