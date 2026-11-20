/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *     Vincent Barichard, 2012
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

#ifdef GECODE_HAS_FLOAT_VARS

#include <gecode/search.hh>

namespace Test::Assign {

    /// %Test assignment with distinct propagator
    class Float : public FloatTest {
    public:
        /// Create and register test
        Float(const std::string& s, const Gecode::FloatVal& d, int n) : FloatTest(s, n, d) {}
        /// Post propagators on variables \a x
        void
        post(Gecode::Space& home, Gecode::FloatVarArray& x) override {
            for (int i = 0; i < x.size() - 1; i++)
                Gecode::rel(home, x[i], Gecode::FRT_LQ, x[i + 1]);
        }
    };

    namespace {
        Gecode::FloatVal d_dense(-2, 2);
        Gecode::FloatVal d_large(-2, 10);

        Float d_3("Dense::3", d_dense, 3);
        Float d_5("Dense::5", d_dense, 5);
        Float l_2("Large::2", d_large, 2);
        Float l_3("Large::3", d_large, 3);
    } // namespace

    /// Merit that prefers the last of five variables
    static double
    float_merit_last(const Gecode::Space&, Gecode::FloatVar, int i) {
        return (i == 4) ? 1.0 : 0.0;
    }

    /// Space with five variables in [0,1] whose sum is at most one
    class FloatOrderSpace : public Gecode::Space {
    public:
        /// Variables to be assigned
        Gecode::FloatVarArray x;
        /// Initialize test space
        FloatOrderSpace() : x(*this, 5, 0.0, 1.0) {
            Gecode::linear(*this, x, Gecode::FRT_LQ, 1.0);
        }
        /// Constructor for cloning \a s
        FloatOrderSpace(FloatOrderSpace& s) : Gecode::Space(s) {
            x.update(*this, s.x);
        }
        /// Copy space during cloning
        Gecode::Space*
        copy() override {
            return new FloatOrderSpace(*this);
        }
    };

    /// %Test that assignment follows a single variable selection
    class FloatVarSelection : public Base {
    public:
        /// Create and register test
        FloatVarSelection() : Base("Float::Assign::VarSelection") {}
        /// Perform test
        bool
        run() override {
            using namespace Gecode;
            FloatOrderSpace* s = new FloatOrderSpace;
            assign(*s, s->x, FLOAT_VAR_MERIT_MAX(&float_merit_last), FLOAT_ASSIGN_MAX());
            DFS<FloatOrderSpace> e(s);
            delete s;
            FloatOrderSpace* solution = e.next();
            bool ok = solution != nullptr;
            for (int i = 0; ok && (i < solution->x.size()); i++)
                ok = (i == 4) ? (solution->x[i].min() > 0.5) : (solution->x[i].max() < 0.5);
            delete solution;
            return ok;
        }
    };

    namespace {
        FloatVarSelection var_selection;
    } // namespace

} // namespace Test::Assign

#endif

// STATISTICS: test-branch
