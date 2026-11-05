/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"

namespace Test { namespace Int {

    /// %Tests for basic setup
    namespace Basic {

        /**
         * \defgroup TaskTestIntBasic Basic setup
         * \ingroup TaskTestInt
         */
        //@{

        /// %Test whether testing infrastructure for integer variables works
        class Basic : public Test {
        public:
            /// Initialize test
            Basic(int n) : Test("Basic::A", 3, -n, n, true) {}
            /// Initialize test
            Basic(Gecode::IntArgs& i) : Test("Basic::B", 3, Gecode::IntSet(i), true) {}
            /// Check whether \a x is a solution
            bool
            solution(const Assignment&) const override {
                return true;
            }
            /// Post constraint on \a x
            void
            post(Gecode::Space&, Gecode::IntVarArray&) override {}
            /// Post reified constraint on \a x for \a r
            void
            post(Gecode::Space& home, Gecode::IntVarArray&, Gecode::Reify r) override {
                if (r.mode() != Gecode::RM_IMP)
                    Gecode::rel(home, r.var(), Gecode::IRT_EQ, 1);
            }
        };

        Gecode::IntArgs i({1, 2, 3, 4});
        Basic b1(3);
        Basic b2(i);

        /// %Test for comparing integer sets
        class IntSetCompare : public Base {
        public:
            /// Create and register test
            IntSetCompare() : Base("Int::Basic::IntSet::Compare") {}
            /// Perform test
            bool
            run() override {
                using namespace Gecode;
                IntSet s({1, 3, 5});
                IntSet same({1, 3, 5});
                IntSet other(1, 5);
                IntSet s_copy(s);
                return compares_equal(s, same) && compares_equal(s, s_copy) && compares_unequal(s, other) &&
                       compares_equal(IntSet::empty, IntSet(1, 0)) && compares_unequal(s, IntSet::empty);
            }
        };

        IntSetCompare int_set_compare;

        /// %Test for comparing integer and Boolean views
        class ViewCompare : public Base {
        private:
            /// Space for the variables behind the views
            class Fixture : public Gecode::Space {
            public:
                Fixture() = default;
                // NOLINTNEXTLINE(modernize-use-equals-default)
                Fixture(Fixture& fixture) : Gecode::Space(fixture) {}
                Gecode::Space*
                copy() override {
                    return new Fixture(*this);
                }
            };

        public:
            /// Create and register test
            ViewCompare() : Base("Int::Basic::View::Compare") {}
            /// Perform test
            bool
            run() override {
                using namespace Gecode;
                using namespace Gecode::Int;
                Fixture home;
                IntView x(IntVar(home, 0, 5));
                IntView y(IntVar(home, 0, 5));
                BoolView b(BoolVar(home, 0, 1));
                BoolView c(BoolVar(home, 0, 1));
                MinusView mx(x);
                return compares_equal(mx, MinusView(x)) && compares_unequal(mx, MinusView(y)) &&
                       compares_equal(OffsetView(x, 2), OffsetView(x, 2)) &&
                       compares_unequal(OffsetView(x, 2), OffsetView(x, 3)) &&
                       compares_unequal(OffsetView(x, 2), OffsetView(y, 2)) &&
                       compares_equal(IntScaleView(2, x), IntScaleView(2, x)) &&
                       compares_unequal(IntScaleView(2, x), IntScaleView(3, x)) &&
                       compares_unequal(IntScaleView(2, x), IntScaleView(2, y)) &&
                       compares_equal(ConstIntView(4), ConstIntView(4)) &&
                       compares_unequal(ConstIntView(4), ConstIntView(5)) &&
                       compares_equal(ZeroIntView(), ZeroIntView()) &&
                       compares_equal(CachedView<IntView>(x), CachedView<IntView>(x)) &&
                       compares_unequal(CachedView<IntView>(x), CachedView<IntView>(y)) &&
                       compares_equal(NegBoolView(b), NegBoolView(b)) &&
                       compares_unequal(NegBoolView(b), NegBoolView(c)) && compares_unequal(x, mx);
            }
        };

        ViewCompare view_compare;
        //@}

    } // namespace Basic
}} // namespace Test::Int

// STATISTICS: test-int
