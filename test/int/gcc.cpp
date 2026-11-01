/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
 *     Christian Schulte, 2007
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

    /// %Tests for counting constraints (global cardinality)
    namespace GCC {

        /**
         * \defgroup TaskTestIntGCC Counting constraints (global cardinality)
         * \ingroup TaskTestInt
         */
        //@{
        /// %Test for integer cardinality with min and max for all variables
        class IntAllMinMax : public Test {
        public:
            /// Create and register test
            IntAllMinMax(Gecode::IntPropLevel ipl)
                : Test(TestTags::sweep(), "GCC::Int::All::MinMax::" + str(ipl), 4, -1, 3, false, ipl) {}
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                int n[5];
                for (int i = 5; i--;)
                    n[i] = 0;
                for (int i = x.size(); i--;)
                    n[x[i] + 1]++;
                if (n[2] > 0)
                    return false;
                for (int i = 5; i--;)
                    if (n[i] > 2)
                        return false;
                return true;
            }
            /// Post constraint on \a x
            void
            post(Gecode::Space& home, Gecode::IntVarArray& x) override {
                using namespace Gecode;
                IntArgs values(5);
                IntSet fixed(0, 2);
                IntSetArgs cards(5);
                for (int i = 0; i < 5; i++) {
                    values[i] = i - 1;
                    cards[i] = fixed;
                }
                cards[2] = IntSet(0, 0);
                count(home, x, cards, values, ipl);
            }
        };

        /// %Test for integer cardinality with min and max for all variables
        class IntAllMinMaxDef : public Test {
        public:
            /// Create and register test
            IntAllMinMaxDef(Gecode::IntPropLevel ipl)
                : Test(TestTags::sweep(), "GCC::Int::All::MinMaxDef::" + str(ipl), 4, 0, 3, false, ipl) {}
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                int n[4];
                for (int i = 4; i--;)
                    n[i] = 0;
                for (int i = x.size(); i--;)
                    n[x[i]]++;
                if (n[2] > 0)
                    return false;
                for (int i = 4; i--;)
                    if (n[i] > 2)
                        return false;
                return true;
            }
            /// Post constraint on \a x
            void
            post(Gecode::Space& home, Gecode::IntVarArray& x) override {
                using namespace Gecode;
                IntSet fixed(0, 2);
                IntSetArgs cards(4);
                for (int i = 0; i < 4; i++) {
                    cards[i] = fixed;
                }
                cards[2] = IntSet(0, 0);
                count(home, x, cards, ipl);
            }
        };

        /// %Test for integer cardinality with max cardinality for all variables
        class IntAllMax : public Test {
        public:
            /// Create and register test
            IntAllMax(Gecode::IntPropLevel ipl)
                : Test(ipl == Gecode::IPL_BND ? TestTags::standard() : TestTags::sweep(),
                       "GCC::Int::All::Max::" + str(ipl), 4, 1, 2, false, ipl) {}
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                int n[2];
                for (int i = 2; i--;)
                    n[i] = 0;
                for (int i = x.size(); i--;)
                    n[x[i] - 1]++;
                if (n[0] != 2 || n[1] != 2)
                    return false;
                return true;
            }
            /// Post constraint on \a x
            void
            post(Gecode::Space& home, Gecode::IntVarArray& x) override {
                Gecode::IntArgs values({1, 2});
                Gecode::count(home, x, Gecode::IntSet(2, 2), values, ipl);
            }
        };

        /// %Test for integer cardinality for some variables
        template <bool hole>
        class IntSome : public Test {
        public:
            /// Create and register test
            IntSome(Gecode::IntPropLevel ipl)
                : Test(TestTags::sweep(), std::string("GCC::Int::Some::") + (hole ? "::Hole" : "::Full") + str(ipl), 4,
                       1, 4, false, ipl) {}
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                int n[4];
                for (int i = 4; i--;)
                    n[i] = 0;
                for (int i = x.size(); i--;)
                    n[x[i] - 1]++;
                if ((n[0] < 2) || (n[1] < 2) || (n[2] > 0) || (n[3] > 0))
                    return false;
                return true;
            }
            /// Post constraint on \a x
            void
            post(Gecode::Space& home, Gecode::IntVarArray& x) override {
                using namespace Gecode;
                IntArgs values({1, 2});
                Gecode::IntSet fixed;
                if (!hole) {
                    fixed = IntSet(0, 2);
                } else {
                    int ish[] = {0, 2};
                    fixed = IntSet(ish, 2);
                }
                Gecode::IntSetArgs cards(2);
                cards[0] = fixed;
                cards[1] = fixed;
                count(home, x, cards, values, ipl);
            }
        };

        /// %Test for variable cardinality for all cardinality values
        class VarAll : public Test {
        protected:
            /// Number of non-cardinality variables
            static const int n = 4;

        public:
            /// Create and register test
            VarAll(Gecode::IntPropLevel ipl)
                : Test(TestTags::sweep(), "GCC::Var::All::" + str(ipl), 7, 0, 2, false, ipl) {}
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                // Number of cardinality variables
                int m = x.size() - n;
                for (int i = 0; i < n; i++)
                    if ((x[i] < 0) || (x[i] > 2))
                        return false;
                int* card = new int[m];
                for (int i = 0; i < m; i++) {
                    card[i] = 0;
                    if ((x[n + i] < 0) || (x[n + i] > 2)) {
                        delete[] card;
                        return false;
                    }
                }
                for (int i = 0; i < n; i++)
                    card[x[i]]++;
                for (int i = 0; i < m; i++)
                    if (card[i] != x[n + i]) {
                        delete[] card;
                        return false;
                    }
                delete[] card;
                return true;
            }
            /// Post constraint on \a xy
            void
            post(Gecode::Space& home, Gecode::IntVarArray& xy) override {
                using namespace Gecode;
                // Number of cardinality variables
                int m = xy.size() - n;

                IntVarArgs x(n), y(m);
                for (int i = 0; i < n; i++)
                    x[i] = xy[i];
                for (int i = 0; i < m; i++)
                    y[i] = xy[n + i];
                count(home, x, y, ipl);
            }
        };

        /// %Test for variable cardinality for some cardinality values
        class VarSome : public Test {
        protected:
            /// Number of non-cardinality variables
            int n;
            /// Arity beyond which to use randomized tests
            static const int random_arity = 7;

        public:
            /// Create and register test
            VarSome(std::string s, int n0, int min, int max, Gecode::IntPropLevel ipl)
                : Test(TestTags::sweep(), "GCC::Var::Some::" + s + "::" + str(ipl), n0 + (max - min) + 1, min, max,
                       false, ipl),
                  n(n0) {
                consistency_test_level = CTL_NONE;
                if (arity > random_arity)
                    test_search = false;
            }
            /// %Test whether \a x is solution
            bool
            solution(const Assignment& x) const override {
                // Number of cardinality variables
                int m = x.size() - n;
                int* card = new int[m];
                for (int i = 0; i < m; i++) {
                    card[i] = 0;
                    if ((x[n + i] < 0) || (x[n + i] > n)) {
                        delete[] card;
                        return false;
                    }
                }
                for (int i = 0; i < n; i++)
                    card[x[i] - dom.min()]++;
                for (int i = 0; i < m; i++)
                    if (card[i] != x[n + i]) {
                        delete[] card;
                        return false;
                    }
                delete[] card;
                return true;
            }
            /// Create and register initial assignment
            Assignment*
            assignment(void) const override {
                if (arity > random_arity)
                    return new RandomAssignment(arity, dom, 4000, _rand);
                else
                    return new CpltAssignment(arity, dom);
            }
            /// Post constraint on \a xy
            void
            post(Gecode::Space& home, Gecode::IntVarArray& xy) override {
                using namespace Gecode;
                // Number of cardinality variables
                int m = xy.size() - n;
                IntVarArgs x(n), y(m);
                for (int i = 0; i < n; i++)
                    x[i] = xy[i];
                for (int i = 0; i < m; i++)
                    y[i] = xy[n + i];
                IntArgs values(m);
                for (int i = m; i--;)
                    values[i] = i + dom.min();
                count(home, x, y, values, ipl);
            }
        };

        /// %Test every count overload with no variables or no values
        class Empty : public Base {
        private:
            /// Space without variables
            class Fixture : public Gecode::Space {
            public:
                Fixture(void) = default;
                Fixture(Fixture& fixture) : Gecode::Space(fixture) {}
                Gecode::Space*
                copy(void) override {
                    return new Fixture(*this);
                }
            };

        public:
            Empty(void) : Base("Int::GCC::Empty") {}
            bool
            run(void) override {
                using namespace Gecode;
                IntVarArgs x;
                IntVarArgs c;
                IntSetArgs cs;
                IntArgs v;
                Fixture var, var_v, var_val, var_bnd, var_dom, set, set_v, set_c;
                count(var, x, c);
                count(var_v, x, c, v);
                count(var_val, x, c, v, IPL_VAL);
                count(var_bnd, x, c, v, IPL_BND);
                count(var_dom, x, c, v, IPL_DOM);
                count(set, x, cs);
                count(set_v, x, cs, v);
                count(set_c, x, IntSet(0, 1), v);
                bool empty_solved = (var.status() == SS_SOLVED) && (var_v.status() == SS_SOLVED) &&
                                    (var_val.status() == SS_SOLVED) && (var_bnd.status() == SS_SOLVED) &&
                                    (var_dom.status() == SS_SOLVED) && (set.status() == SS_SOLVED) &&
                                    (set_v.status() == SS_SOLVED) && (set_c.status() == SS_SOLVED);

                // Without variables, every value occurs zero times
                Fixture card, card_zero, card_one, card_hole;
                IntVarArgs y(card, 2, 0, 2);
                count(card, x, y, IntArgs({1, 1}));
                count(card_zero, x, IntSetArgs({IntSet({0, 2})}), IntArgs({1}));
                count(card_one, x, IntSetArgs({IntSet(1, 2)}), IntArgs({1}));
                count(card_hole, x, IntSet({1, 3}), IntArgs({1}));
                bool card_counted = (card.status() == SS_SOLVED) && y[0].assigned() && (y[0].val() == 0) &&
                                    y[1].assigned() && (y[1].val() == 0) && (card_zero.status() == SS_SOLVED) &&
                                    (card_one.status() == SS_FAILED) && (card_hole.status() == SS_FAILED);

                // Without values, no variable can take a value
                Fixture no_var, no_var_v, no_set, no_set_v, no_set_c;
                count(no_var, IntVarArgs(no_var, 1, 0, 2), c);
                count(no_var_v, IntVarArgs(no_var_v, 1, 0, 2), c, v);
                count(no_set, IntVarArgs(no_set, 1, 0, 2), cs);
                count(no_set_v, IntVarArgs(no_set_v, 1, 0, 2), cs, v);
                count(no_set_c, IntVarArgs(no_set_c, 1, 0, 2), IntSet(0, 1), v);
                bool no_value_failed = (no_var.status() == SS_FAILED) && (no_var_v.status() == SS_FAILED) &&
                                       (no_set.status() == SS_FAILED) && (no_set_v.status() == SS_FAILED) &&
                                       (no_set_c.status() == SS_FAILED);
                return empty_solved && card_counted && no_value_failed;
            }
        };

        Empty empty;

        /// Help class to create and register tests
        class Create {
        public:
            /// Perform creation and registration
            Create(void) {
                for (IntPropLevels ipls; ipls(); ++ipls) {
                    (void)new IntAllMinMax(ipls.ipl());
                    (void)new IntAllMinMaxDef(ipls.ipl());
                    (void)new IntAllMax(ipls.ipl());
                    (void)new IntSome<false>(ipls.ipl());
                    (void)new IntSome<true>(ipls.ipl());
                    (void)new VarAll(ipls.ipl());
                    (void)new VarSome("Small", 2, -1, 3, ipls.ipl());
                    (void)new VarSome("Large", 3, -1, 4, ipls.ipl());
                }
            }
        };

        Create c;
        //@}

    } // namespace GCC
}} // namespace Test::Int

// STATISTICS: test-int
