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

    /// %Test assignment with distinct propagator
    class Int : public IntTest {
    public:
        /// Create and register test
        Int(const std::string& s, const Gecode::IntSet& d, int n) : IntTest(s, n, d) {}
        /// Post propagators on variables \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x) override {
            Gecode::distinct(home, x);
        }
    };

    Gecode::IntSet d_dense(-2, 2);
    const int v_sparse[5] = {-100, -10, 0, 10, 100};
    Gecode::IntSet d_sparse(v_sparse, 5);

    Gecode::IntSet d_large(-2, 10);

    Int d_3("Dense::3", d_dense, 3);
    Int d_5("Dense::5", d_dense, 5);
    Int s_3("Sparse::3", d_sparse, 3);
    Int s_5("Sparse::5", d_sparse, 5);
    Int l_2("Large::2", d_large, 2);
    Int l_3("Large::3", d_large, 3);

    /// Merit that prefers the variable at position \a p
    template <int p>
    double
    int_merit_at(const Gecode::Space&, Gecode::IntVar, int i) {
        return (i == p) ? 1.0 : 0.0;
    }

    /// Space with five 0/1 variables of which exactly \a n are one
    class IntOrderSpace : public Gecode::Space {
    public:
        /// Variables to be assigned
        Gecode::IntVarArray x;
        /// Initialize test space
        IntOrderSpace(int n) : x(*this, 5, 0, 1) {
            Gecode::linear(*this, x, Gecode::IRT_EQ, n);
        }
        /// Constructor for cloning \a s
        IntOrderSpace(IntOrderSpace& s) : Gecode::Space(s) {
            x.update(*this, s.x);
        }
        /// Copy space during cloning
        Gecode::Space*
        copy(void) override {
            return new IntOrderSpace(*this);
        }
    };

    /// Whether assigning one first in \a s sets exactly the last \a n variables
    bool
    assigned_last_first(IntOrderSpace* s, int n) {
        Gecode::DFS<IntOrderSpace> e(s);
        delete s;
        IntOrderSpace* solution = e.next();
        bool ok = solution != nullptr;
        for (int i = 0; ok && (i < solution->x.size()); i++)
            ok = solution->x[i].val() == ((i >= solution->x.size() - n) ? 1 : 0);
        delete solution;
        return ok;
    }

    /// %Test that assignment follows each of \a n tie-breaking selections
    class IntTieBreak : public Base {
    protected:
        /// Number of variable selections
        int n;

    public:
        /// Create and register test
        IntTieBreak(int n0) : Base("Int::Assign::TieBreak::" + std::to_string(n0)), n(n0) {}
        /// Perform test
        bool
        run(void) override {
            using namespace Gecode;
            TieBreak<IntVarBranch> vars(INT_VAR_MERIT_MAX(&int_merit_at<4>));
            if (n > 1)
                vars.b = INT_VAR_MERIT_MAX(&int_merit_at<3>);
            if (n > 2)
                vars.c = INT_VAR_MERIT_MAX(&int_merit_at<2>);
            if (n > 3)
                vars.d = INT_VAR_MERIT_MAX(&int_merit_at<1>);
            IntOrderSpace* s = new IntOrderSpace(n);
            assign(*s, s->x, vars, INT_ASSIGN_MAX());
            return assigned_last_first(s, n);
        }
    };

    namespace {
        IntTieBreak tb_1(1);
        IntTieBreak tb_2(2);
        IntTieBreak tb_3(3);
        IntTieBreak tb_4(4);
    } // namespace

    /// %Test that assignment follows a single variable selection
    class IntVarSelection : public Base {
    public:
        /// Create and register test
        IntVarSelection(void) : Base("Int::Assign::VarSelection") {}
        /// Perform test
        bool
        run(void) override {
            using namespace Gecode;
            IntOrderSpace* s = new IntOrderSpace(1);
            assign(*s, s->x, INT_VAR_MERIT_MAX(&int_merit_at<4>), INT_ASSIGN_MAX());
            return assigned_last_first(s, 1);
        }
    };

    namespace {
        IntVarSelection var_selection;
    } // namespace

}} // namespace Test::Assign

// STATISTICS: test-branch
