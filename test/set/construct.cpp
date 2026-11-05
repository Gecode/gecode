/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2008
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

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {

    /// %Tests for set variable conors
    namespace Var {

        /// %Test for cardinality constraint
        class Construct : public SetTest {
        public:
            /// Create and register test
            Construct(void) : SetTest("Var::Construct", 1, IntSet(0, 0), false, 0) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment&) const override {
                return true;
            }
            void
            con(Space& home, int glb_min, int glb_max, int lub_min, int lub_max, unsigned int min_card = 0,
                unsigned int max_card = Gecode::Set::Limits::card, bool fail = true) {
                bool ok = false;
                try {
                    SetVar x(home, glb_min, glb_max, lub_min, lub_max, min_card, max_card);
                    ok = true;
                } catch (Gecode::Set::VariableEmptyDomain&) {
                    if (!fail) {
                        home.fail();
                        return;
                    }
                    ok = true;
                }

                if (ok) {
                    try {
                        SetVarArray xs(home, 1, glb_min, glb_max, lub_min, lub_max, min_card, max_card);
                    } catch (Gecode::Set::VariableEmptyDomain&) {
                        if (!fail) {
                            home.fail();
                            return;
                        } else {
                            return;
                        }
                    }
                    if (fail) {
                        home.fail();
                    }
                }
            }
            void
            con(Space& home, const IntSet& glb, int lub_min, int lub_max, unsigned int min_card = 0,
                unsigned int max_card = Gecode::Set::Limits::card, bool fail = true) {
                bool ok = false;
                try {
                    SetVar x(home, glb, lub_min, lub_max, min_card, max_card);
                    ok = true;
                } catch (Gecode::Set::VariableEmptyDomain&) {
                    if (!fail) {
                        home.fail();
                        return;
                    }
                    ok = true;
                }
                if (ok) {
                    try {
                        SetVarArray xs(home, 1, glb, lub_min, lub_max, min_card, max_card);
                    } catch (Gecode::Set::VariableEmptyDomain&) {
                        if (!fail) {
                            home.fail();
                            return;
                        } else {
                            return;
                        }
                    }
                    if (fail) {
                        home.fail();
                    }
                }
            }
            void
            con(Space& home, int glb_min, int glb_max, const IntSet& lub, unsigned int min_card = 0,
                unsigned int max_card = Gecode::Set::Limits::card, bool fail = true) {
                bool ok = false;
                try {
                    SetVar x(home, glb_min, glb_max, lub, min_card, max_card);
                    ok = true;
                } catch (Gecode::Set::VariableEmptyDomain&) {
                    if (!fail) {
                        home.fail();
                        return;
                    }
                    ok = true;
                }
                if (ok) {
                    try {
                        SetVarArray xs(home, 1, glb_min, glb_max, lub, min_card, max_card);
                    } catch (Gecode::Set::VariableEmptyDomain&) {
                        if (!fail) {
                            home.fail();
                            return;
                        } else {
                            return;
                        }
                    }
                    if (fail) {
                        home.fail();
                    }
                }
            }
            void
            con(Space& home, const IntSet& glb, const IntSet& lub, unsigned int min_card = 0,
                unsigned int max_card = Gecode::Set::Limits::card, bool fail = true) {
                bool ok = false;
                try {
                    SetVar x(home, glb, lub, min_card, max_card);
                    ok = true;
                } catch (Gecode::Set::VariableEmptyDomain&) {
                    if (!fail) {
                        home.fail();
                        return;
                    }
                    ok = true;
                }
                if (ok) {
                    try {
                        SetVarArray xs(home, 1, glb, lub, min_card, max_card);
                    } catch (Gecode::Set::VariableEmptyDomain&) {
                        if (!fail) {
                            home.fail();
                            return;
                        } else {
                            return;
                        }
                    }
                    if (fail) {
                        home.fail();
                    }
                }
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray&, IntVarArray&) override {
                // Check non-empty lower, empty upper bound
                con(home, 1, 1, 1, 0);
                con(home, IntSet(1, 1), 1, 0);
                con(home, 1, 1, IntSet(1, 0));
                con(home, IntSet(1, 1), IntSet(1, 0));

                // Check min cardinality
                con(home, 1, 0, 1, 1, 2, 3);
                con(home, IntSet(1, 0), 1, 1, 2U, 3U);
                con(home, 1, 0, IntSet(1, 1), 2U, 3U);
                con(home, IntSet(1, 0), IntSet(1, 1), 2U, 3U);

                // Check max cardinality
                con(home, 1, 3, 1, 4, 0, 2);
                con(home, IntSet(1, 3), 1, 4, 0U, 2U);
                con(home, 1, 3, IntSet(1, 4), 0U, 2U);
                con(home, IntSet(1, 3), IntSet(1, 4), 0U, 2U);

                // Check non-subset bounds
                con(home, 1, 3, 2, 4);
                con(home, IntSet(1, 3), 2, 4);
                con(home, 1, 3, IntSet(2, 4));
                con(home, IntSet(1, 3), IntSet(2, 4));
                con(home, 1, 4, 1, 3);
                con(home, IntSet(1, 4), 1, 3);
                con(home, 1, 4, IntSet(1, 3));
                con(home, IntSet(1, 4), IntSet(1, 3));

                con(home, IntSet::empty, 2, 4, 0U, 10U, false);
            }
        };
        Construct _con;

        /// %Test for comparing set views
        class ViewCompare : public ::Test::Base {
        private:
            /// Space for the variables behind the views
            class Fixture : public Gecode::Space {
            public:
                Fixture(void) = default;
                // NOLINTNEXTLINE(modernize-use-equals-default)
                Fixture(Fixture& fixture) : Gecode::Space(fixture) {}
                Gecode::Space*
                copy(void) override {
                    return new Fixture(*this);
                }
            };

        public:
            /// Create and register test
            ViewCompare(void) : ::Test::Base("Set::Var::View::Compare") {}
            /// Perform test
            bool
            run(void) override {
                using namespace Gecode::Set;
                Fixture home;
                SetView x(SetVar(home, IntSet::empty, 0, 5));
                SetView y(SetVar(home, IntSet::empty, 0, 5));
                IntVar i(home, 0, 5);
                IntVar j(home, 0, 5);
                ConstSetView c(home, IntSet(1, 3));
                ComplementView<SetView> cx(x);
                ComplementView<SetView> cy(y);
                return compares_equal(c, ConstSetView(home, IntSet(1, 3))) &&
                       compares_unequal(c, ConstSetView(home, IntSet(1, 4))) &&
                       compares_equal(EmptyView(), EmptyView()) && compares_equal(UniverseView(), UniverseView()) &&
                       compares_equal(SingletonView(i), SingletonView(i)) &&
                       compares_unequal(SingletonView(i), SingletonView(j)) &&
                       compares_equal(cx, ComplementView<SetView>(x)) && compares_unequal(cx, cy) &&
                       compares_equal(CachedView<SetView>(x), CachedView<SetView>(x)) &&
                       compares_unequal(CachedView<SetView>(x), CachedView<SetView>(y));
            }
        };

        ViewCompare view_compare;

        /// %Test default construction of value iterators
        class DefaultValues : public ::Test::Base {
        private:
            /// Space for the variable behind the iterators
            class Fixture : public Gecode::Space {
            public:
                Fixture(void) = default;
                // NOLINTNEXTLINE(modernize-use-equals-default)
                Fixture(Fixture& fixture) : Gecode::Space(fixture) {}
                Gecode::Space*
                copy(void) override {
                    return new Fixture(*this);
                }
            };

        public:
            /// Create and register test
            DefaultValues(void) : ::Test::Base("Set::Var::DefaultValues") {}
            /// Perform test
            bool
            run(void) override {
                using namespace Gecode;
                Fixture home;
                SetVar x(home, IntSet(1, 1), IntSet(1, 2));
                SetVarGlbValues glb;
                SetVarLubValues lub;
                SetVarUnknownValues unknown;
                glb = SetVarGlbValues(x);
                lub = SetVarLubValues(x);
                unknown = SetVarUnknownValues(x);
                return glb() && (glb.val() == 1) && lub() && (lub.val() == 1) && unknown() && (unknown.val() == 2);
            }
        };

        DefaultValues default_values;

    } // namespace Var
}} // namespace Test::Set

// STATISTICS: test-set
