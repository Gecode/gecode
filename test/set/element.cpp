/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2005
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

#include <gecode/minimodel.hh>

#include "test/set.hh"

using namespace Gecode;

namespace Test { namespace Set {

    /// %Tests for set element constraints
    namespace Element {

        /**
         * \defgroup TaskTestSetElement Element constraints
         * \ingroup TaskTestSet
         */
        //@{

        static IntSet ds_12(-1, 2);
        static IntSet ds_13(-1, 3);

        /// %Test for %ElementUnion constraint
        class ElementUnion : public SetTest {
        public:
            /// Create and register test
            ElementUnion(const char* t) : SetTest(TestTags::sweep(), t, 5, ds_12, false) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                int selected = 0;
                for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++) {
                }
                CountableSetValues x4v(x.lub, x[4]);
                if (selected == 0)
                    return !x4v();
                CountableSetRanges* sel = new CountableSetRanges[selected];
                CountableSetValues selector(x.lub, x[3]);
                for (int i = selected; i--; ++selector) {
                    if (selector.val() >= 3 || selector.val() < 0) {
                        delete[] sel;
                        return false;
                    }
                    sel[i].init(x.lub, x[selector.val()]);
                }

                bool ret;
                {
                    Region r;
                    Iter::Ranges::NaryUnion u(r, sel, selected);

                    CountableSetRanges z(x.lub, x[4]);
                    ret = Iter::Ranges::equal(u, z);
                }
                delete[] sel;
                return ret;
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                SetVarArgs xs(x.size() - 2);
                for (int i = x.size() - 2; i--;)
                    xs[i] = x[i];
                Gecode::element(home, SOT_UNION, xs, x[x.size() - 2], x[x.size() - 1]);
            }
        };
        ElementUnion _elementunion("Element::Union");

        /// %Test for %ElementUnionConst constraint
        class ElementUnionConst : public SetTest {
        private:
            const IntSet i0;
            const IntSet i1;
            const IntSet i2;

        public:
            /// Create and register test
            ElementUnionConst(const char* t)
                : SetTest(TestTags::sweep(), t, 2, ds_13, false), i0(-3, -3), i1(-1, 1), i2(0, 2) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                int selected = 0;
                for (CountableSetValues sel2(x.lub, x[0]); sel2(); ++sel2, selected++) {
                }
                CountableSetValues x4v(x.lub, x[1]);
                if (selected == 0)
                    return !x4v();
                IntSet iss[] = {i0, i1, i2};
                IntSetRanges* sel = new IntSetRanges[selected];
                CountableSetValues selector(x.lub, x[0]);
                for (int i = selected; i--; ++selector) {
                    if (selector.val() >= 3 || selector.val() < 0) {
                        delete[] sel;
                        return false;
                    }
                    sel[i].init(iss[selector.val()]);
                }

                bool ret;
                {
                    Region r;
                    Iter::Ranges::NaryUnion u(r, sel, selected);

                    CountableSetRanges z(x.lub, x[1]);
                    ret = Iter::Ranges::equal(u, z);
                }
                delete[] sel;
                return ret;
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                IntSetArgs xs(3);
                xs[0] = i0;
                xs[1] = i1;
                xs[2] = i2;
                Gecode::element(home, SOT_UNION, xs, x[0], x[1]);
            }
        };
        ElementUnionConst _elementunionconst("Element::UnionConst");

        /// %Test for %ElementInter constraint
        class ElementInter : public SetTest {
        public:
            /// Create and register test
            ElementInter(const char* t) : SetTest(TestTags::sweep(), t, 5, ds_12, false) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                int selected = 0;
                for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++) {
                }
                CountableSetRanges x4r(x.lub, x[4]);
                if (selected == 0)
                    return Iter::Ranges::size(x4r) == Gecode::Set::Limits::card;
                CountableSetRanges* sel = new CountableSetRanges[selected];
                CountableSetValues selector(x.lub, x[3]);
                for (int i = selected; i--; ++selector) {
                    if (selector.val() >= 3 || selector.val() < 0) {
                        delete[] sel;
                        return false;
                    }
                    sel[i].init(x.lub, x[selector.val()]);
                }
                bool ret;
                {
                    Region r;
                    Iter::Ranges::NaryInter u(r, sel, selected);

                    CountableSetRanges z(x.lub, x[4]);
                    ret = Iter::Ranges::equal(u, z);
                }
                delete[] sel;
                return ret;
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                SetVarArgs xs(x.size() - 2);
                for (int i = x.size() - 2; i--;)
                    xs[i] = x[i];
                Gecode::element(home, SOT_INTERSECTION, xs, x[x.size() - 2], x[x.size() - 1]);
            }
        };
        ElementInter _elementinter("Element::Inter");

        /// %Test for %ElementInter constraint
        class ElementInterIn : public SetTest {
        public:
            /// Create and register test
            ElementInterIn(const char* t) : SetTest(TestTags::sweep(), t, 5, ds_12, false) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                int selected = 0;
                for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++) {
                }
                CountableSetRanges x4r(x.lub, x[4]);
                if (selected == 0)
                    return Iter::Ranges::size(x4r) == 4;
                CountableSetRanges* sel = new CountableSetRanges[selected];
                CountableSetValues selector(x.lub, x[3]);
                for (int i = selected; i--; ++selector) {
                    if (selector.val() >= 3 || selector.val() < 0) {
                        delete[] sel;
                        return false;
                    }
                    sel[i].init(x.lub, x[selector.val()]);
                }
                bool ret;
                {
                    Region r;
                    Iter::Ranges::NaryInter u(r, sel, selected);

                    CountableSetRanges z(x.lub, x[4]);
                    ret = Iter::Ranges::equal(u, z);
                }
                delete[] sel;
                return ret;
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                SetVarArgs xs(x.size() - 2);
                for (int i = x.size() - 2; i--;)
                    xs[i] = x[i];
                Gecode::element(home, SOT_INTERSECTION, xs, x[x.size() - 2], x[x.size() - 1], ds_12);
            }
        };
        ElementInterIn _elementinterin("Element::InterIn");

        /// %Test for %ElementDisjoint constraint
        class ElementDisjoint : public SetTest {
        public:
            /// Create and register test
            ElementDisjoint(const char* t) : SetTest(TestTags::standard(), t, 5, ds_12, false) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                int selected = 0;
                for (CountableSetValues sel2(x.lub, x[3]); sel2(); ++sel2, selected++) {
                    if (sel2.val() < 0)
                        return false;
                }
                CountableSetValues x4v(x.lub, x[4]);
                if (selected == 0)
                    return !x4v();
                CountableSetRanges* sel = new CountableSetRanges[selected];
                CountableSetValues selector(x.lub, x[3]);
                unsigned int cardsum = 0;
                for (int i = selected; i--; ++selector) {
                    if (selector.val() >= 3 || selector.val() < 0) {
                        delete[] sel;
                        return false;
                    }
                    sel[i].init(x.lub, x[selector.val()]);
                    CountableSetRanges xicard(x.lub, x[selector.val()]);
                    cardsum += Iter::Ranges::size(xicard);
                }

                bool ret;
                {
                    Region r;
                    Iter::Ranges::NaryUnion u(r, sel, selected);
                    ret = Iter::Ranges::size(u) == cardsum;
                    u.reset();
                    CountableSetRanges z(x.lub, x[4]);
                    ret &= Iter::Ranges::equal(u, z);
                }
                delete[] sel;
                return ret;
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                SetVarArgs xs(x.size() - 2);
                for (int i = x.size() - 2; i--;)
                    xs[i] = x[i];
                Gecode::element(home, SOT_DISJOINT_UNION, xs, x[x.size() - 2], x[x.size() - 1]);
            }
        };
        ElementDisjoint _elementdisjoint("Element::Disjoint");

        /// %Test for %ElementSet constraint
        class ElementSet : public SetTest {
        public:
            /// Create and register test
            ElementSet(const char* t) : SetTest(TestTags::sweep(), t, 4, ds_12, false, true) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                if (x.int_value() < 0 || x.int_value() > 2)
                    return false;
                CountableSetRanges z(x.lub, x[3]);
                CountableSetRanges y(x.lub, x[x.int_value()]);
                return Iter::Ranges::equal(y, z);
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray& y) override {
                SetVarArgs xs(x.size() - 1);
                for (int i = x.size() - 1; i--;)
                    xs[i] = x[i];
                Gecode::element(home, xs, y[0], x[x.size() - 1]);
            }
        };
        ElementSet _elementset("Element::Set");

        /// %Test for %ElementSetConst constraint
        class ElementSetConst : public SetTest {
        private:
            const IntSet i0;
            const IntSet i1;
            const IntSet i2;

        public:
            /// Create and register test
            ElementSetConst(const char* t)
                : SetTest(TestTags::sweep(), t, 1, ds_13, false, true), i0(-3, -3), i1(-1, 1), i2(0, 2) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                if (x.int_value() < 0 || x.int_value() > 2)
                    return false;
                CountableSetRanges xr(x.lub, x[0]);
                IntSet iss[] = {i0, i1, i2};
                IntSetRanges isr(iss[x.int_value()]);
                return Iter::Ranges::equal(xr, isr);
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray& y) override {
                IntSetArgs xs(3);
                xs[0] = i0;
                xs[1] = i1;
                xs[2] = i2;
                Gecode::element(home, xs, y[0], x[0]);
            }
        };
        ElementSetConst _elementsetconst("Element::SetConst");

        /// Universe for element with intersection over no sets
        enum class Universe {
            Given,   ///< Pass a universe that fits the result
            Default, ///< Use the default universe, which does not fit the result
        };

        /**
         * \brief %Test element over an empty array of type \a Args
         *
         * No index can be selected. An empty union is empty and an empty
         * intersection is the universe.
         */
        template <class Args>
        class ElementEmpty : public SetTest {
        protected:
            /// Operation to combine the selected sets
            SetOpType op;
            /// Universe for intersection
            Universe universe;
            /// Universe passed for Universe::Given
            IntSet given;

        public:
            /// Create and register test
            ElementEmpty(const std::string& args, SetOpType op0, Universe universe0 = Universe::Given)
                : SetTest("Element::Empty::" + args + "::" + str(op0) +
                              (universe0 == Universe::Default ? "::Default" : ""),
                          2, ds_12, false),
                  op(op0), universe(universe0), given(0, 1) {}
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                CountableSetRanges y(x.lub, x[0]);
                if (y())
                    return false;
                CountableSetRanges z(x.lub, x[1]);
                if (op != SOT_INTERSECTION)
                    return !z();
                if (universe == Universe::Default)
                    return false;
                IntSetRanges u(given);
                return Iter::Ranges::equal(z, u);
            }
            /// Post constraint on \a x
            void
            post(Space& home, SetVarArray& x, IntVarArray&) override {
                Args xs;
                if ((op == SOT_INTERSECTION) && (universe == Universe::Given))
                    Gecode::element(home, op, xs, x[0], x[1], given);
                else
                    Gecode::element(home, op, xs, x[0], x[1]);
            }
        };

        ElementEmpty<IntSetArgs> _element_empty_const_union("Const", SOT_UNION);
        ElementEmpty<IntSetArgs> _element_empty_const_dunion("Const", SOT_DISJOINT_UNION);
        ElementEmpty<IntSetArgs> _element_empty_const_inter("Const", SOT_INTERSECTION);
        ElementEmpty<IntSetArgs> _element_empty_const_inter_default("Const", SOT_INTERSECTION, Universe::Default);
        ElementEmpty<IntArgs> _element_empty_int_union("Int", SOT_UNION);
        ElementEmpty<IntArgs> _element_empty_int_dunion("Int", SOT_DISJOINT_UNION);
        ElementEmpty<IntArgs> _element_empty_int_inter("Int", SOT_INTERSECTION);
        ElementEmpty<IntArgs> _element_empty_int_inter_default("Int", SOT_INTERSECTION, Universe::Default);
        ElementEmpty<SetVarArgs> _element_empty_set_var_union("SetVar", SOT_UNION);
        ElementEmpty<SetVarArgs> _element_empty_set_var_dunion("SetVar", SOT_DISJOINT_UNION);
        ElementEmpty<SetVarArgs> _element_empty_set_var_inter("SetVar", SOT_INTERSECTION);
        ElementEmpty<SetVarArgs> _element_empty_set_var_inter_default("SetVar", SOT_INTERSECTION, Universe::Default);
        ElementEmpty<IntVarArgs> _element_empty_int_var_union("IntVar", SOT_UNION);
        ElementEmpty<IntVarArgs> _element_empty_int_var_dunion("IntVar", SOT_DISJOINT_UNION);
        ElementEmpty<IntVarArgs> _element_empty_int_var_inter("IntVar", SOT_INTERSECTION);
        ElementEmpty<IntVarArgs> _element_empty_int_var_inter_default("IntVar", SOT_INTERSECTION, Universe::Default);

        /// Space with the variables for posting element with an integer index
        class IndexFixture : public Gecode::Space {
        public:
            /// Index or column
            IntVar x;
            /// Row
            IntVar y;
            /// Result set
            SetVar z;
            /// Create fixture
            IndexFixture() : x(*this, 0, 1), y(*this, 0, 1), z(*this, IntSet::empty, IntSet(0, 3)) {}
            /// Constructor for cloning \a fixture
            IndexFixture(IndexFixture& fixture) : Gecode::Space(fixture) {
                x.update(*this, fixture.x);
                y.update(*this, fixture.y);
                z.update(*this, fixture.z);
            }
            /// Copy during cloning
            Gecode::Space*
            copy() override {
                return new IndexFixture(*this);
            }
        };

        /// Test whether posting a matrix of \a n sets throws \a E for both array types
        template <class E>
        bool
        is_matrix_rejected(int n, int w, int h) {
            IndexFixture constant, variable;
            bool constant_rejected = false;
            bool variable_rejected = false;
            try {
                element(constant, IntSetArgs(n), constant.x, w, constant.y, h, constant.z);
            } catch (const E&) {
                constant_rejected = true;
            }
            try {
                element(variable, SetVarArgs(variable, n, IntSet::empty, 0, 3), variable.x, w, variable.y, h,
                        variable.z);
            } catch (const E&) {
                variable_rejected = true;
            }
            return constant_rejected && variable_rejected;
        }

        /// Test whether posting over no sets throws Set::TooFewArguments for array type \a Args
        template <class Args>
        bool
        is_array_rejected() {
            IndexFixture home;
            try {
                element(home, Args(), home.x, home.z);
            } catch (const Gecode::Set::TooFewArguments&) {
                return true;
            }
            return false;
        }

        /// %Test that element with an integer index rejects invalid arrays and matrices
        class ElementIndexInvalid : public Base {
        public:
            /// Create and register test
            ElementIndexInvalid() : Base("Set::Element::Index::Invalid") {}
            /// Perform test
            bool
            run() override {
                using Gecode::Set::ArgumentSizeMismatch;
                using Gecode::Set::TooFewArguments;
                return is_array_rejected<IntSetArgs>() && is_array_rejected<SetVarArgs>() &&
                       is_matrix_rejected<TooFewArguments>(0, 0, 0) && is_matrix_rejected<TooFewArguments>(0, 1, 1) &&
                       is_matrix_rejected<ArgumentSizeMismatch>(3, 0, 5) &&
                       is_matrix_rejected<ArgumentSizeMismatch>(1, -1, -1) &&
                       is_matrix_rejected<ArgumentSizeMismatch>(6, -2, -3) &&
                       // The product is 2 * 2^32 + 1
                       is_matrix_rejected<ArgumentSizeMismatch>(1, 9, 954437177);
            }
        };
        ElementIndexInvalid _element_index_invalid;

        /// %Test for matrix element with integer set array and set variable
        class MatrixIntSet : public SetTest {
        protected:
            /// Array for test matrix
            Gecode::IntSetArgs tm;

        public:
            /// Create and register test
            MatrixIntSet() : SetTest(TestTags::sweep(), "Element::Matrix::IntSet", 1, IntSet(0, 3), false, 2), tm(4) {
                tm[0] = IntSet(0, 0);
                tm[1] = IntSet(1, 1);
                tm[2] = IntSet(2, 2);
                tm[3] = IntSet(3, 3);
            }
            /// %Test whether \a x is solution
            bool
            solution(const SetAssignment& x) const override {
                // Get integer assignment
                const Int::Assignment& y = x.ints();
                // x-coordinate: y[0], y-coordinate: y[1], result: x[0]
                using namespace Gecode;
                if ((y[0] > 1) || (y[1] > 1))
                    return false;
                Matrix<IntSetArgs> m(tm, 2, 2);
                IntSetRanges a(m(y[0], y[1]));
                CountableSetRanges b(x.lub, x[0]);
                return Iter::Ranges::equal(a, b);
            }
            /// Post constraint on \a x and \a y
            void
            post(Gecode::Space& home, Gecode::SetVarArray& x, Gecode::IntVarArray& y) override {
                // x-coordinate: x[0], y-coordinate: x[1], result: x[2]
                using namespace Gecode;
                Matrix<IntSetArgs> m(tm, 2, 2);
                element(home, m, y[0], y[1], x[0]);
            }
        };

        MatrixIntSet _emis;

        //@}

    } // namespace Element
}} // namespace Test::Set

// STATISTICS: test-set
