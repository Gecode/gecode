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

#include "test/set.hh"
#include "test/int.hh"
#include <gecode/minimodel.hh>
#include <utility>

using namespace Gecode;

/// %Tests for combined int/set constraints
namespace Test::Set::Int {

    /**
     * \defgroup TaskTestSetInt Combined integer/set constraints
     * \ingroup TaskTestSet
     */
    //@{

    static const int d1r[4][2] = {{-4, -3}, {-1, -1}, {1, 1}, {3, 5}};
    static IntSet d1(d1r, 4);

    static IntSet d2(-1, 3);
    static IntSet d3(0, 3);

    static IntSet d4(0, 4);

    static IntSet ds_33(-3, 3);

    /// %Test for cardinality constraint
    class Card : public SetTest {
    public:
        /// Create and register test
        Card(const char* t) : SetTest(t, 1, ds_33, true, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            unsigned int s = 0;
            for (CountableSetRanges xr(x.lub, x[0]); xr(); ++xr)
                s += xr.width();
            if (x.int_value() < 0)
                return false;
            return std::cmp_equal(s, x.int_value());
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::cardinality(home, x[0], y[0]);
        }
        /// Post reified constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            Gecode::cardinality(home, x[0], y[0], r);
        }
    };
    Card _card("Int::Card");

    /// %Test for minimal element constraint
    class Min : public SetTest {
    public:
        /// Create and register test
        Min(const char* t) : SetTest(t, 1, ds_33, true, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges xr0(x.lub, x[0]);
            return xr0() && xr0.min() == x.int_value();
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::min(home, x[0], y[0]);
        }
        /// Post reified constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            Gecode::min(home, x[0], y[0], r);
        }
    };
    Min _min("Int::Min");

    /// %Test for negated minimal element constraint
    class NotMin : public SetTest {
    public:
        /// Create and register test
        NotMin(const char* t) : SetTest(t, 1, ds_33, false, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges xr0(x.lub, x[0]);
            return !(xr0() && xr0.min() == x.int_value());
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::not_min(home, x[0], y[0]);
        }
    };
    NotMin _notmin("Int::NotMin");

    /// %Test for maximal element constraint
    class Max : public SetTest {
    public:
        /// Create and register test
        Max(const char* t) : SetTest(t, 1, ds_33, true, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges xr0(x.lub, x[0]);
            IntSet x0(xr0);
            return x0.ranges() > 0 && x0.max() == x.int_value();
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::max(home, x[0], y[0]);
        }
        /// Post reified constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            Gecode::max(home, x[0], y[0], r);
        }
    };
    Max _max("Int::Max");

    /// %Test for negated maximal element constraint
    class NotMax : public SetTest {
    public:
        /// Create and register test
        NotMax(const char* t) : SetTest(t, 1, ds_33, false, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges xr0(x.lub, x[0]);
            IntSet x0(xr0);
            return !(x0.ranges() > 0 && x0.max() == x.int_value());
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::not_max(home, x[0], y[0]);
        }
    };
    NotMax _notmax("Int::NotMax");

    /// %Test for element constraint
    class Elem : public SetTest {
    public:
        /// Create and register test
        Elem(const char* t) : SetTest(t, 1, ds_33, true, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            for (CountableSetValues xr(x.lub, x[0]); xr(); ++xr)
                if (xr.val() == x.int_value())
                    return true;
            return false;
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::rel(home, x[0], SRT_SUP, y[0]);
        }
        /// Post reified constraint on \a x for \a b
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            Gecode::rel(home, x[0], SRT_SUP, y[0], r);
        }
    };
    Elem _elem("Int::Elem");

    /// %Test for negated element constraint
    class NoElem : public SetTest {
    public:
        /// Create and register test
        NoElem(const char* t) : SetTest(t, 1, ds_33, false, 1) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            for (CountableSetValues xr(x.lub, x[0]); xr(); ++xr)
                if (xr.val() == x.int_value())
                    return false;
            return true;
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::rel(home, x[0], SRT_DISJOINT, y[0]);
        }
    };
    NoElem _noelem("Int::NoElem");

    /// %Test for relation constraint
    class Rel : public SetTest {
    private:
        /// The set relation type
        Gecode::SetRelType srt;
        /// Whether relation is swapped
        bool swapped;

    public:
        /// Create and register test
        Rel(Gecode::SetRelType srt0, bool s)
            : SetTest("Int::Rel::" + str(srt0) + (s ? "::s" : ""), 1, ds_33, true, 1), srt(srt0), swapped(s) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges xr(x.lub, x[0]);
            IntSet is(x.int_value(), x.int_value());
            IntSetRanges dr(is);
            Gecode::SetRelType rsrt = srt;
            if (swapped) {
                switch (srt) {
                case SRT_SUB:
                    rsrt = SRT_SUP;
                    break;
                case SRT_SUP:
                    rsrt = SRT_SUB;
                    break;
                default:
                    break;
                }
            }
            switch (rsrt) {
            case SRT_EQ:
                return Iter::Ranges::equal(xr, dr);
            case SRT_NQ:
                return !Iter::Ranges::equal(xr, dr);
            case SRT_SUB:
                return Iter::Ranges::subset(xr, dr);
            case SRT_SUP:
                return Iter::Ranges::subset(dr, xr);
            case SRT_DISJOINT: {
                Gecode::Iter::Ranges::Inter<CountableSetRanges, IntSetRanges> inter(xr, dr);
                return !inter();
            }
            case SRT_COMPLEMENT: {
                Gecode::Set::RangesCompl<IntSetRanges> drc(dr);
                return Iter::Ranges::equal(xr, drc);
            }
            default:
                GECODE_NEVER;
            }
            return false;
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            if (!swapped)
                Gecode::rel(home, x[0], srt, y[0]);
            else
                Gecode::rel(home, y[0], srt, x[0]);
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            if (!swapped)
                Gecode::rel(home, x[0], srt, y[0], r);
            else
                Gecode::rel(home, y[0], srt, x[0], r);
        }
    };
    Rel _rel_eq(Gecode::SRT_EQ, false);
    Rel _rel_nq(Gecode::SRT_NQ, false);
    Rel _rel_sub(Gecode::SRT_SUB, false);
    Rel _rel_sup(Gecode::SRT_SUP, false);
    Rel _rel_disj(Gecode::SRT_DISJOINT, false);
    Rel _rel_cmpl(Gecode::SRT_COMPLEMENT, false);
    Rel _rel_eqs(Gecode::SRT_EQ, true);
    Rel _rel_nqs(Gecode::SRT_NQ, true);
    Rel _rel_subs(Gecode::SRT_SUB, true);
    Rel _rel_sups(Gecode::SRT_SUP, true);
    Rel _rel_disjs(Gecode::SRT_DISJOINT, true);
    Rel _rel_cmpls(Gecode::SRT_COMPLEMENT, true);

    /// %Test for integer relation constraint
    class IntRel : public SetTest {
    private:
        /// The integer relation type
        Gecode::IntRelType irt;
        /// Whether the arguments are swapped
        bool swapped;

    public:
        /// Create and register test
        IntRel(Gecode::IntRelType irt0, bool s)
            : SetTest("Int::IntRel::" + Test::Int::Test::str(irt0) + (s ? "::s" : ""), 1, ds_33, true, 1), irt(irt0),
              swapped(s) {
            test_subsumption = false;
        }
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetValues xr(x.lub, x[0]);
            if (!xr())
                return false;
            for (; xr(); ++xr)
                switch (irt) {
                case Gecode::IRT_EQ:
                    if (xr.val() != x.int_value())
                        return false;
                    break;
                case Gecode::IRT_NQ:
                    if (xr.val() == x.int_value())
                        return false;
                    break;
                case Gecode::IRT_GR:
                    if (!swapped && xr.val() <= x.int_value())
                        return false;
                    if (swapped && xr.val() >= x.int_value())
                        return false;
                    break;
                case Gecode::IRT_GQ:
                    if (!swapped && xr.val() < x.int_value())
                        return false;
                    if (swapped && xr.val() > x.int_value())
                        return false;
                    break;
                case Gecode::IRT_LT:
                    if (!swapped && xr.val() >= x.int_value())
                        return false;
                    if (swapped && xr.val() <= x.int_value())
                        return false;
                    break;
                case Gecode::IRT_LQ:
                    if (!swapped && xr.val() > x.int_value())
                        return false;
                    if (swapped && xr.val() < x.int_value())
                        return false;
                    break;
                default:
                    GECODE_NEVER;
                    return false;
                }
            return true;
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            if (!swapped)
                Gecode::rel(home, x[0], irt, y[0]);
            else
                Gecode::rel(home, y[0], irt, x[0]);
        }
        /// Post reified constraint on \a x for \a r
        void
        post(Space& home, SetVarArray& x, IntVarArray& y, Reify r) override {
            assert((x.size() == 1) && (y.size() == 1));
            if ((r.mode() != Gecode::RM_EQV) || (_rand(2) != 0)) {
                if (!swapped)
                    Gecode::rel(home, x[0], irt, y[0], r);
                else
                    Gecode::rel(home, y[0], irt, x[0], r);
            } else if (swapped) {
                switch (irt) {
                case Gecode::IRT_EQ:
                    Gecode::rel(home, (y[0] == x[0]) == r.var());
                    break;
                case Gecode::IRT_NQ:
                    Gecode::rel(home, (y[0] != x[0]) == r.var());
                    break;
                case Gecode::IRT_LT:
                    Gecode::rel(home, (y[0] < x[0]) == r.var());
                    break;
                case Gecode::IRT_LQ:
                    Gecode::rel(home, (y[0] <= x[0]) == r.var());
                    break;
                case Gecode::IRT_GR:
                    Gecode::rel(home, (y[0] > x[0]) == r.var());
                    break;
                case Gecode::IRT_GQ:
                    Gecode::rel(home, (y[0] >= x[0]) == r.var());
                    break;
                default:
                    GECODE_NEVER;
                }
            } else {
                switch (irt) {
                case Gecode::IRT_EQ:
                    Gecode::rel(home, (x[0] == y[0]) == r.var());
                    break;
                case Gecode::IRT_NQ:
                    Gecode::rel(home, (x[0] != y[0]) == r.var());
                    break;
                case Gecode::IRT_LT:
                    Gecode::rel(home, (x[0] < y[0]) == r.var());
                    break;
                case Gecode::IRT_LQ:
                    Gecode::rel(home, (x[0] <= y[0]) == r.var());
                    break;
                case Gecode::IRT_GR:
                    Gecode::rel(home, (x[0] > y[0]) == r.var());
                    break;
                case Gecode::IRT_GQ:
                    Gecode::rel(home, (x[0] >= y[0]) == r.var());
                    break;
                default:
                    GECODE_NEVER;
                }
            }
        }
    };
    IntRel _intrel_eq(Gecode::IRT_EQ, false);
    IntRel _intrel_nq(Gecode::IRT_NQ, false);
    IntRel _intrel_gr(Gecode::IRT_GR, false);
    IntRel _intrel_gq(Gecode::IRT_GQ, false);
    IntRel _intrel_lt(Gecode::IRT_LT, false);
    IntRel _intrel_lq(Gecode::IRT_LQ, false);
    IntRel _intrel_eqs(Gecode::IRT_EQ, true);
    IntRel _intrel_nqs(Gecode::IRT_NQ, true);
    IntRel _intrel_grs(Gecode::IRT_GR, true);
    IntRel _intrel_gqs(Gecode::IRT_GQ, true);
    IntRel _intrel_lts(Gecode::IRT_LT, true);
    IntRel _intrel_lqs(Gecode::IRT_LQ, true);

    template <class I>
    int
    weight_i(const IntArgs& elements, const IntArgs& weights, I& iter) {
        int sum = 0;
        int i = 0;
        for (Iter::Ranges::ToValues<I> v(iter); v(); ++v) {
            // Skip all elements below the current
            while (elements[i] < v.val())
                i++;
            assert(elements[i] == v.val());
            sum += weights[i];
        }
        return sum;
    }

    /// %Test for set weight constraint
    class Weights : public SetTest {
    public:
        IntArgs elements;
        IntArgs weights;
        int min_weight;
        int max_weight;
        /// Create and register test
        Weights(const char* t, IntArgs& el, IntArgs& w, int min = -10000, int max = 10000)
            : SetTest(t, 1, ds_33, false, 1), elements(el), weights(w), min_weight(min), max_weight(max) {}
        /// %Test whether \a x is solution
        bool
        solution(const SetAssignment& x) const override {
            CountableSetRanges x0(x.lub, x[0]);
            return x.int_value() == weight_i(elements, weights, x0) && x.int_value() >= min_weight &&
                   x.int_value() <= max_weight;
        }
        /// Post constraint on \a x
        void
        post(Space& home, SetVarArray& x, IntVarArray& y) override {
            Gecode::rel(home, min_weight <= y[0]);
            Gecode::rel(home, max_weight >= y[0]);
            Gecode::weights(home, elements, weights, x[0], y[0]);
        }
    };

    const int el1v[] = {-3, -2, -1, 0, 1, 2, 3};
    IntArgs el1(7, el1v);
    const int w1v[] = {1, -2, 1, 1, 1, 6, 1};
    IntArgs w1(7, w1v);
    Weights _weights1("Int::Weights::1", el1, w1);

    const int w2v[] = {-1, -1, -1, 10, -1, -1, -1};
    IntArgs w2(7, w2v);
    Weights _weights2("Int::Weights::2", el1, w2);
    Weights _weights3("Int::Weights::3", el1, w2, 3);

    const int w4v[] = {1, 1, 0, 0, 0, 0, 0};
    IntArgs w4(7, w4v);
    Weights _weights4("Int::Weights::4", el1, w4);

    /// Space with a set and a weight variable for tests without elements
    class WeightsFixture : public Gecode::Space {
    public:
        /// Weighted set
        SetVar x;
        /// Weight of \a x
        IntVar y;
        /// Create fixture with greatest lower bound \a glb for \a x and minimal weight \a min
        WeightsFixture(const IntSet& glb, int min) : x(*this, glb, ds_33), y(*this, min, 2) {}
        /// Constructor for cloning \a fixture
        WeightsFixture(WeightsFixture& fixture) : Gecode::Space(fixture) {
            x.update(*this, fixture.x);
            y.update(*this, fixture.y);
        }
        /// Copy during cloning
        Gecode::Space*
        copy() override {
            return new WeightsFixture(*this);
        }
    };

    /// %Test weights without elements: the set is empty and weighs 0
    class WeightsEmpty : public Base {
    public:
        /// Create and register test
        WeightsEmpty() : Base("Set::Int::Weights::Empty") {}
        /// Perform test
        bool
        run() override {
            IntArgs none;
            WeightsFixture empty(IntSet::empty, -2);
            weights(empty, none, none, empty.x, empty.y);
            bool solved = (empty.status() == SS_SOLVED) && (empty.x.lub_size() == 0) && empty.y.assigned() &&
                          (empty.y.val() == 0);

            WeightsFixture nonempty(IntSet(0, 0), -2), heavy(IntSet::empty, 1);
            weights(nonempty, none, none, nonempty.x, nonempty.y);
            weights(heavy, none, none, heavy.x, heavy.y);
            bool failed = (nonempty.status() == SS_FAILED) && (heavy.status() == SS_FAILED);

            WeightsFixture mismatch(IntSet::empty, -2);
            try {
                weights(mismatch, none, IntArgs({1}), mismatch.x, mismatch.y);
                return false;
            } catch (Gecode::Set::ArgumentSizeMismatch&) {
                return solved && failed;
            }
        }
    };
    WeightsEmpty _weights_empty;

} // namespace Test::Set::Int
// STATISTICS: test-set
