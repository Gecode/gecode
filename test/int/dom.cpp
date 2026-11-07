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

#include <gecode/minimodel.hh>

#include "test/int.hh"

/// %Tests for domain constraints
namespace Test::Int::Dom {

    /**
     * \defgroup TaskTestIntDom Domain constraints
     * \ingroup TaskTestInt
     */
    //@{
    /// %Test for domain constraint (integer)
    class DomInt : public Test {
    public:
        /// Create and register test
        DomInt(int n) : Test("Dom::Int::" + str(n), n, -4, 4, n == 1, Gecode::IPL_DOM) {}
        /// %Test whether \a x is solution
        bool
        solution(const Assignment& x) const override {
            for (int i = x.size(); i--;)
                if (x[i] != -2)
                    return false;
            return true;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x) override {
            if (x.size() == 1)
                Gecode::dom(home, x[0], -2);
            else
                Gecode::dom(home, x, -2);
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x, Gecode::Reify r) override {
            assert(x.size() == 1);
            if (_rand(2) != 0) {
                Gecode::dom(home, x[0], -2, r);
            } else {
                switch (r.mode()) {
                case Gecode::RM_EQV:
                    Gecode::rel(home, Gecode::dom(x[0], -2) == r.var());
                    break;
                case Gecode::RM_IMP:
                    Gecode::rel(home, Gecode::dom(x[0], -2) << r.var());
                    break;
                case Gecode::RM_RIMP:
                    Gecode::rel(home, Gecode::dom(x[0], -2) >> r.var());
                    break;
                default:
                    GECODE_NEVER;
                }
            }
        }
    };

    /// %Test for domain constraint (range)
    class DomRange : public Test {
    public:
        /// Create and register test
        DomRange(int n) : Test("Dom::Range::" + str(n), n, -4, 4, n == 1, Gecode::IPL_DOM) {}
        /// %Test whether \a x is solution
        bool
        solution(const Assignment& x) const override {
            for (int i = x.size(); i--;)
                if ((x[i] < -2) || (x[i] > 2))
                    return false;
            return true;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x) override {
            if (x.size() == 1)
                Gecode::dom(home, x[0], -2, 2);
            else
                Gecode::dom(home, x, -2, 2);
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x, Gecode::Reify r) override {
            assert(x.size() == 1);
            if (_rand(2) != 0) {
                Gecode::dom(home, x[0], -2, 2, r);
            } else {
                switch (r.mode()) {
                case Gecode::RM_EQV:
                    Gecode::rel(home, Gecode::dom(x[0], -2, 2) == r.var());
                    break;
                case Gecode::RM_IMP:
                    Gecode::rel(home, Gecode::dom(x[0], -2, 2) << r.var());
                    break;
                case Gecode::RM_RIMP:
                    Gecode::rel(home, Gecode::dom(x[0], -2, 2) >> r.var());
                    break;
                default:
                    GECODE_NEVER;
                }
            }
        }
    };

    /// %Test for domain constraint (empty range)
    class DomRangeEmpty : public Test {
    public:
        /// Create and register test
        DomRangeEmpty() : Test("Dom::Range::Empty", 1, -4, 4, true) {}
        /// %Test whether \a x is solution
        bool
        solution(const Assignment&) const override {
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray&) override {
            home.fail();
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x, Gecode::Reify r) override {
            Gecode::dom(home, x[0], 3, 2, r);
        }
    };

    /// %Test for domain constraint (set from an empty initializer list)
    class DomDomEmpty : public Test {
        /// Whether the list holds ranges rather than values
        bool ranges;
        /// Return the empty set built from an empty list
        Gecode::IntSet
        empty_set() const {
            if (ranges)
                return Gecode::IntSet(std::initializer_list<std::pair<int, int>>{});
            return Gecode::IntSet(std::initializer_list<int>{});
        }

    public:
        /// Create and register test
        DomDomEmpty(bool r)
            : Test(std::string("Dom::Dom::Empty::") + (r ? "Ranges" : "Values"), 1, -4, 4, true), ranges(r) {}
        /// %Test whether \a x is solution
        bool
        solution(const Assignment&) const override {
            return false;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x) override {
            Gecode::dom(home, x[0], empty_set());
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x, Gecode::Reify r) override {
            Gecode::dom(home, x[0], empty_set(), r);
        }
    };

    const int r[4][2] = {{-4, -3}, {-1, -1}, {1, 1}, {3, 5}};
    Gecode::IntSet d(r, 4);

    /// %Test for domain constraint (full integer set)
    class DomDom : public Test {
    public:
        /// Create and register test
        DomDom(int n) : Test("Dom::Dom::" + str(n), n, -6, 6, n == 1, Gecode::IPL_DOM) {}
        /// %Test whether \a x is solution
        bool
        solution(const Assignment& x) const override {
            for (int i = x.size(); i--;)
                if (!(((x[i] >= -4) && (x[i] <= -3)) || ((x[i] >= -1) && (x[i] <= -1)) ||
                      ((x[i] >= 1) && (x[i] <= 1)) || ((x[i] >= 3) && (x[i] <= 5))))
                    return false;
            return true;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x) override {
            if (x.size() == 1)
                Gecode::dom(home, x[0], d);
            else
                Gecode::dom(home, x, d);
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space& home, Gecode::IntVarArray& x, Gecode::Reify r) override {
            assert(x.size() == 1);
            if (_rand(2) != 0) {
                Gecode::dom(home, x[0], d, r);
            } else {
                switch (r.mode()) {
                case Gecode::RM_EQV:
                    Gecode::rel(home, Gecode::dom(x[0], d) == r.var());
                    break;
                case Gecode::RM_IMP:
                    Gecode::rel(home, Gecode::dom(x[0], d) << r.var());
                    break;
                case Gecode::RM_RIMP:
                    Gecode::rel(home, Gecode::dom(x[0], d) >> r.var());
                    break;
                default:
                    GECODE_NEVER;
                }
            }
        }
    };

    DomInt di1(1);
    DomInt di3(3);
    DomRange dr1(1);
    DomRange dr3(3);
    DomDom dd1(1);
    DomDom dd3(3);
    DomRangeEmpty dre;
    DomDomEmpty dde_values(false);
    DomDomEmpty dde_ranges(true);
    //@}

} // namespace Test::Int::Dom
// STATISTICS: test-int
