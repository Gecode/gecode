/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include "test/assign.hh"

#include <gecode/search.hh>

#include <string>

namespace Test { namespace Assign {

    /// %Test branching with distinct propagator
    class Bool : public BoolTest {
    public:
        /// Create and register test
        Bool(const std::string& s, int n) : BoolTest(s, n) {}
        /// Post propagators on variables \a x
        void
        post(Gecode::Space& home, Gecode::BoolVarArray& x) override {
            Gecode::rel(home, Gecode::BOT_OR, x, 1);
        }
    };

    Bool b_3("3", 3);
    Bool b_5("5", 5);
    Bool b_7("7", 7);

    /// Merit that prefers the variable at position \a p
    template <int p>
    double
    bool_merit_at(const Gecode::Space&, Gecode::BoolVar, int i) {
        return (i == p) ? 1.0 : 0.0;
    }

    /// Filter that accepts every variable
    bool
    bool_accept(const Gecode::Space&, Gecode::BoolVar, int) {
        return true;
    }

    /// Print function that prints nothing
    void
    bool_print_nothing(const Gecode::Space&, const Gecode::Brancher&, unsigned int, Gecode::BoolVar, int, const int&,
                       std::ostream&) {}

    /// Space with five Boolean variables of which exactly \a n are one
    class BoolOrderSpace : public Gecode::Space {
    public:
        /// Variables to be assigned
        Gecode::BoolVarArray x;
        /// Initialize test space
        BoolOrderSpace(int n) : x(*this, 5, 0, 1) {
            Gecode::linear(*this, x, Gecode::IRT_EQ, n);
        }
        /// Constructor for cloning \a s
        BoolOrderSpace(BoolOrderSpace& s) : Gecode::Space(s) {
            x.update(*this, s.x);
        }
        /// Copy space during cloning
        Gecode::Space*
        copy() override {
            return new BoolOrderSpace(*this);
        }
    };

    /// Whether assigning one first in \a s sets exactly the last \a n variables
    bool
    assigned_last_first(BoolOrderSpace* s, int n) {
        Gecode::DFS<BoolOrderSpace> e(s);
        delete s;
        BoolOrderSpace* solution = e.next();
        bool ok = solution != nullptr;
        for (int i = 0; ok && (i < solution->x.size()); i++)
            ok = solution->x[i].val() == ((i >= solution->x.size() - n) ? 1 : 0);
        delete solution;
        return ok;
    }

    /// %Test that assignment follows each of \a n tie-breaking selections
    class BoolTieBreak : public Base {
    protected:
        /// Number of variable selections
        int n;

    public:
        /// Create and register test
        BoolTieBreak(int n0) : Base("Bool::Assign::TieBreak::" + std::to_string(n0)), n(n0) {}
        /// Perform test
        bool
        run() override {
            using namespace Gecode;
            TieBreak<BoolVarBranch> vars(BOOL_VAR_MERIT_MAX(&bool_merit_at<4>));
            if (n > 1)
                vars.b = BOOL_VAR_MERIT_MAX(&bool_merit_at<3>);
            if (n > 2)
                vars.c = BOOL_VAR_MERIT_MAX(&bool_merit_at<2>);
            if (n > 3)
                vars.d = BOOL_VAR_MERIT_MAX(&bool_merit_at<1>);
            BoolOrderSpace* s = new BoolOrderSpace(n);
            assign(*s, s->x, vars, BOOL_ASSIGN_MAX(), &bool_accept, &bool_print_nothing);
            return assigned_last_first(s, n);
        }
    };

    namespace {
        BoolTieBreak tb_1(1);
        BoolTieBreak tb_2(2);
        BoolTieBreak tb_3(3);
        BoolTieBreak tb_4(4);
    } // namespace

    /// %Test that assignment follows a single variable selection
    class BoolVarSelection : public Base {
    public:
        /// Create and register test
        BoolVarSelection() : Base("Bool::Assign::VarSelection") {}
        /// Perform test
        bool
        run() override {
            using namespace Gecode;
            BoolOrderSpace* s = new BoolOrderSpace(1);
            assign(*s, s->x, BOOL_VAR_MERIT_MAX(&bool_merit_at<4>), BOOL_ASSIGN_MAX());
            return assigned_last_first(s, 1);
        }
    };

    namespace {
        BoolVarSelection var_selection;
    } // namespace

}} // namespace Test::Assign

// STATISTICS: test-branch
