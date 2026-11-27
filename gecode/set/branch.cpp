/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <gecode/set/branch.hh>

namespace Gecode {

    void
    branch(Home home, const SetVarArgs& x, SetVarBranch vars, const SetValBranch& vals, SetBranchFilter bf,
           SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.expand(home, x);
        ViewArray<SetView> xv(home, x);
        ViewSel<SetView>* vs[1] = {Branch::view_selection(home, vars)};
        post_view_value_brancher<SetView, 1, int, 2>(home, xv, vs, Branch::value_selection_commit(home, vals),
                                                     std::move(bf), std::move(vvp));
    }

    void
    branch(Home home, const SetVarArgs& x, TieBreak<SetVarBranch> vars, const SetValBranch& vals, SetBranchFilter bf,
           SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.a.expand(home, x);
        if ((vars.a.select() == SetVarBranch::SEL_NONE) || (vars.a.select() == SetVarBranch::SEL_RND))
            vars.b = SET_VAR_NONE();
        vars.b.expand(home, x);
        if ((vars.b.select() == SetVarBranch::SEL_NONE) || (vars.b.select() == SetVarBranch::SEL_RND))
            vars.c = SET_VAR_NONE();
        vars.c.expand(home, x);
        if ((vars.c.select() == SetVarBranch::SEL_NONE) || (vars.c.select() == SetVarBranch::SEL_RND))
            vars.d = SET_VAR_NONE();
        vars.d.expand(home, x);
        if (vars.b.select() == SetVarBranch::SEL_NONE) {
            branch(home, x, std::move(vars.a), vals, std::move(bf), std::move(vvp));
        } else {
            ViewArray<SetView> xv(home, x);
            ValSelCommitBase<SetView, int>* vsc = Branch::value_selection_commit(home, vals);
            if (vars.c.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[2] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b)};
                post_view_value_brancher<SetView, 2, int, 2>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            } else if (vars.d.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[3] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c)};
                post_view_value_brancher<SetView, 3, int, 2>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            } else {
                ViewSel<SetView>* vs[4] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c), Branch::view_selection(home, vars.d)};
                post_view_value_brancher<SetView, 4, int, 2>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            }
        }
    }

    void
    branch(Home home, SetVar x, const SetValBranch& vals, SetVarValPrint vvp) {
        SetVarArgs xv(1);
        xv[0] = x;
        branch(home, xv, SET_VAR_NONE(), vals, nullptr, std::move(vvp));
    }

    void
    assign(Home home, const SetVarArgs& x, SetVarBranch vars, const SetAssign& vals, SetBranchFilter bf,
           SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.expand(home, x);
        ViewArray<SetView> xv(home, x);
        ViewSel<SetView>* vs[1] = {Branch::view_selection(home, vars)};
        post_view_value_brancher<SetView, 1, int, 1>(home, xv, vs, Branch::value_selection_commit(home, vals),
                                                     std::move(bf), std::move(vvp));
    }

    void
    assign(Home home, const SetVarArgs& x, TieBreak<SetVarBranch> vars, const SetAssign& vals, SetBranchFilter bf,
           SetVarValPrint vvp) {
        using namespace Set;
        if (home.failed())
            return;
        vars.a.expand(home, x);
        if ((vars.a.select() == SetVarBranch::SEL_NONE) || (vars.a.select() == SetVarBranch::SEL_RND))
            vars.b = SET_VAR_NONE();
        vars.b.expand(home, x);
        if ((vars.b.select() == SetVarBranch::SEL_NONE) || (vars.b.select() == SetVarBranch::SEL_RND))
            vars.c = SET_VAR_NONE();
        vars.c.expand(home, x);
        if ((vars.c.select() == SetVarBranch::SEL_NONE) || (vars.c.select() == SetVarBranch::SEL_RND))
            vars.d = SET_VAR_NONE();
        vars.d.expand(home, x);
        if (vars.b.select() == SetVarBranch::SEL_NONE) {
            assign(home, x, std::move(vars.a), vals, std::move(bf), std::move(vvp));
        } else {
            ViewArray<SetView> xv(home, x);
            ValSelCommitBase<SetView, int>* vsc = Branch::value_selection_commit(home, vals);
            if (vars.c.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[2] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b)};
                post_view_value_brancher<SetView, 2, int, 1>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            } else if (vars.d.select() == SetVarBranch::SEL_NONE) {
                ViewSel<SetView>* vs[3] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c)};
                post_view_value_brancher<SetView, 3, int, 1>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            } else {
                ViewSel<SetView>* vs[4] = {Branch::view_selection(home, vars.a), Branch::view_selection(home, vars.b),
                                           Branch::view_selection(home, vars.c), Branch::view_selection(home, vars.d)};
                post_view_value_brancher<SetView, 4, int, 1>(home, xv, vs, vsc, std::move(bf), std::move(vvp));
            }
        }
    }

    void
    assign(Home home, SetVar x, const SetAssign& vars, SetVarValPrint vvp) {
        SetVarArgs xv(1);
        xv[0] = x;
        assign(home, xv, SET_VAR_NONE(), vars, nullptr, std::move(vvp));
    }

} // namespace Gecode

// STATISTICS: set-post
