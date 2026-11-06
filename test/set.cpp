/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
 *     Mikael Zayenz Lagerkvist, 2005
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

#include <algorithm>

namespace Test::Set {

    CountableSet::CountableSet(const Gecode::IntSet& d0) : d(d0), cur(0) {
        Gecode::IntSetRanges isr(d);
        lubmax = static_cast<unsigned int>(pow(2.0, static_cast<int>(Gecode::Iter::Ranges::size(isr))));
    }

    void
    CountableSet::operator++() {
        cur++;
    }

    void
    CountableSet::init(const Gecode::IntSet& d0) {
        d = d0;
        cur = 0;
        Gecode::IntSetRanges isr(d);
        lubmax = static_cast<unsigned int>(pow(2.0, static_cast<int>(Gecode::Iter::Ranges::size(isr))));
    }

    int
    CountableSet::val() const {
        return cur;
    }

    SetAssignment::SetAssignment(int n0, const Gecode::IntSet& d0, int _with_int)
        : n(n0), dsv(new CountableSet[n]), ir(_with_int, d0), done(false), lub(d0), with_int(_with_int) {
        for (int i = n; i--;)
            dsv[i].init(lub);
    }

    void
    SetAssignment::next(Gecode::Support::RandomGenerator& rand) {
        int i = n - 1;
        while (true) {
            ++dsv[i];
            if (dsv[i]())
                return;
            dsv[i].init(lub);
            --i;
            if (i < 0) {
                if (with_int == 0) {
                    done = true;
                    return;
                }
                ir.next(rand);
                if (ir.has_more()) {
                    i = n - 1;
                    for (int j = n; j--;)
                        dsv[j].init(lub);
                } else {
                    done = true;
                    return;
                }
            }
        }
    }

} // namespace Test::Set

std::ostream&
operator<<(std::ostream& os, const Test::Set::SetAssignment& a) {
    int n = a.size();
    os << "{";
    for (int i = 0; i < n; i++) {
        Test::Set::CountableSetRanges csv(a.lub, a[i]);
        Gecode::IntSet icsv(csv);
        os << icsv << ((i != n - 1) ? "," : "}");
    }
    if (a.with_int > 0)
        os << a.ints();
    return os;
}

namespace Test::Set {

    SetTestSpace::SetTestSpace(int n, Gecode::IntSet& d0, int i, SetTest* t, bool log)
        : d(d0), y(*this, i, d), with_int(i), r(Gecode::BoolVar(*this, 0, 1), Gecode::RM_EQV), reified(false), test(t) {
        using namespace Gecode;
        IntSet u(Gecode::Set::Limits::min, Gecode::Set::Limits::max);
        x = SetVarArray(*this, n, Gecode::IntSet::empty, u);
        SetVarArgs _x(*this, n, Gecode::IntSet::empty, d);
        if (x.size() == 1)
            dom(*this, x[0], _x[0]);
        else
            dom(*this, x, _x);
        if (opt.log && log) {
            olog << ind(2) << "Initial: x[]=" << x;
            olog << " y[]=" << y;
            olog << '\n';
        }
    }

    SetTestSpace::SetTestSpace(int n, Gecode::IntSet& d0, int i, SetTest* t, Gecode::ReifyMode rm, bool log)
        : d(d0), x(*this, n, Gecode::IntSet::empty, d), y(*this, i, d), with_int(i),
          r(Gecode::BoolVar(*this, 0, 1), rm), reified(true), test(t) {
        if (opt.log && log) {
            olog << ind(2) << "Initial: x[]=" << x;
            olog << " y[]=" << y;
            olog << " b=" << r.var();
            olog << '\n';
        }
    }

    SetTestSpace::SetTestSpace(SetTestSpace& s)
        : Gecode::Space(s), d(s.d), with_int(s.with_int), reified(s.reified), test(s.test) {
        x.update(*this, s.x);
        y.update(*this, s.y);
        Gecode::BoolVar b;
        Gecode::BoolVar sr(s.r.var());
        b.update(*this, sr);
        r.var(b);
        r.mode(s.r.mode());
    }

    Gecode::Space*
    SetTestSpace::copy() {
        return new SetTestSpace(*this);
    }

    void
    SetTestSpace::post() {
        if (reified) {
            test->post(*this, x, y, r);
            if (opt.log)
                olog << ind(3) << "Posting reified propagator" << '\n';
        } else {
            test->post(*this, x, y);
            if (opt.log)
                olog << ind(3) << "Posting propagator" << '\n';
        }
    }

    bool
    SetTestSpace::failed() {
        if (opt.log) {
            olog << ind(3) << "Fixpoint: x[]=" << x << " y[]=" << y << '\n';
            bool f = (status() == Gecode::SS_FAILED);
            olog << ind(3) << "     -->  x[]=" << x << " y[]=" << y << '\n';
            return f;
        }
        return status() == Gecode::SS_FAILED;
    }

    bool
    SetTestSpace::subsumed(bool b) {
        return b ? (propagators() == 0) : true;
    }

    void
    SetTestSpace::rel(int i, Gecode::SetRelType srt, const Gecode::IntSet& is) {
        if (opt.log) {
            olog << ind(4) << "x[" << i << "] ";
            switch (srt) {
            case Gecode::SRT_EQ:
                olog << "=";
                break;
            case Gecode::SRT_LQ:
                olog << "<=";
                break;
            case Gecode::SRT_LT:
                olog << "<";
                break;
            case Gecode::SRT_GQ:
                olog << ">=";
                break;
            case Gecode::SRT_GR:
                olog << ">";
                break;
            case Gecode::SRT_NQ:
                olog << "!=";
                break;
            case Gecode::SRT_SUB:
                olog << "sub";
                break;
            case Gecode::SRT_SUP:
                olog << "sup";
                break;
            case Gecode::SRT_DISJOINT:
                olog << "||";
                break;
            case Gecode::SRT_COMPLEMENT:
                olog << "^-1 = ";
                break;
            }
            olog << is << '\n';
        }
        Gecode::dom(*this, x[i], srt, is);
    }

    void
    SetTestSpace::cardinality(int i, int min_cardinality, int max_cardinality) {
        if (opt.log) {
            olog << ind(4) << min_cardinality << " <= #(x[" << i << "]) <= " << max_cardinality << '\n';
        }
        Gecode::cardinality(*this, x[i], min_cardinality, max_cardinality);
    }

    void
    SetTestSpace::rel(int i, Gecode::IntRelType irt, int n) {
        if (opt.log) {
            olog << ind(4) << "y[" << i << "] ";
            switch (irt) {
            case Gecode::IRT_EQ:
                olog << "=";
                break;
            case Gecode::IRT_NQ:
                olog << "!=";
                break;
            case Gecode::IRT_LQ:
                olog << "<=";
                break;
            case Gecode::IRT_LT:
                olog << "<";
                break;
            case Gecode::IRT_GQ:
                olog << ">=";
                break;
            case Gecode::IRT_GR:
                olog << ">";
                break;
            }
            olog << " " << n << '\n';
        }
        Gecode::rel(*this, y[i], irt, n);
    }

    void
    SetTestSpace::rel(bool sol) {
        int n = sol ? 1 : 0;
        assert(reified);
        if (opt.log)
            olog << ind(4) << "b = " << n << '\n';
        Gecode::rel(*this, r.var(), Gecode::IRT_EQ, n);
    }

    void
    SetTestSpace::assign(const SetAssignment& a, Gecode::Support::RandomGenerator& rand) {
        for (int i = a.size(); i--;) {
            CountableSetRanges csv(a.lub, a[i]);
            Gecode::IntSet ai(csv);
            rel(i, Gecode::SRT_EQ, ai);
            if (Base::fixpoint(rand) && failed())
                return;
        }
        for (int i = with_int; i--;) {
            rel(i, Gecode::IRT_EQ, a.ints()[i]);
            if (Base::fixpoint(rand) && failed())
                return;
        }
    }

    bool
    SetTestSpace::assigned() const {
        for (int i = x.size(); i--;)
            if (!x[i].assigned())
                return false;
        for (int i = y.size(); i--;)
            if (!y[i].assigned())
                return false;
        return true;
    }

    void
    SetTestSpace::remove_from_lub(int v, int i, const SetAssignment& a) {
        using namespace Gecode;
        SetVarUnknownRanges ur(x[i]);
        CountableSetRanges air(a.lub, a[i]);
        Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges> diff(ur, air);
        Gecode::Iter::Ranges::ToValues<Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges>>
            diff_v(diff);
        for (int j = 0; j < v; j++, ++diff_v) {
        }
        rel(i, Gecode::SRT_DISJOINT, Gecode::IntSet(diff_v.val(), diff_v.val()));
    }

    void
    SetTestSpace::remove_from_lub(int v, int i, const SetAssignment& a, SetTestSpace& c) {
        using namespace Gecode;
        SetVarUnknownRanges ur(x[i]);
        CountableSetRanges air(a.lub, a[i]);
        Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges> diff(ur, air);
        Gecode::Iter::Ranges::ToValues<Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges>>
            diff_v(diff);
        for (int j = 0; j < v; j++, ++diff_v) {
        }
        rel(i, Gecode::SRT_DISJOINT, Gecode::IntSet(diff_v.val(), diff_v.val()));
        c.rel(i, Gecode::SRT_DISJOINT, Gecode::IntSet(diff_v.val(), diff_v.val()));
    }

    void
    SetTestSpace::add_to_glb(int v, int i, const SetAssignment& a) {
        using namespace Gecode;
        SetVarUnknownRanges ur(x[i]);
        CountableSetRanges air(a.lub, a[i]);
        Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges> inter(ur, air);
        Gecode::Iter::Ranges::ToValues<Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges>>
            inter_v(inter);
        for (int j = 0; j < v; j++, ++inter_v) {
        }
        rel(i, Gecode::SRT_SUP, Gecode::IntSet(inter_v.val(), inter_v.val()));
    }

    void
    SetTestSpace::add_to_glb(int v, int i, const SetAssignment& a, SetTestSpace& c) {
        using namespace Gecode;
        SetVarUnknownRanges ur(x[i]);
        CountableSetRanges air(a.lub, a[i]);
        Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges> inter(ur, air);
        Gecode::Iter::Ranges::ToValues<Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges>>
            inter_v(inter);
        for (int j = 0; j < v; j++, ++inter_v) {
        }
        rel(i, Gecode::SRT_SUP, Gecode::IntSet(inter_v.val(), inter_v.val()));
        c.rel(i, Gecode::SRT_SUP, Gecode::IntSet(inter_v.val(), inter_v.val()));
    }

    bool
    SetTestSpace::check_fixpoint() {
        if (failed())
            return true;
        SetTestSpace* c = static_cast<SetTestSpace*>(clone());
        if (opt.log)
            olog << ind(3) << "Testing fixpoint on copy" << '\n';
        c->post();
        if (c->failed()) {
            delete c;
            return false;
        }

        for (int i = x.size(); i--;)
            if (x[i].glb_size() != c->x[i].glb_size() || x[i].lub_size() != c->x[i].lub_size() ||
                x[i].card_min() != c->x[i].card_min() || x[i].card_max() != c->x[i].card_max()) {
                delete c;
                return false;
            }
        for (int i = y.size(); i--;)
            if (y[i].size() != c->y[i].size()) {
                delete c;
                return false;
            }
        if (reified && (r.var().size() != c->r.var().size())) {
            delete c;
            return false;
        }
        if (opt.log)
            olog << ind(3) << "Finished testing fixpoint on copy" << '\n';
        delete c;
        return true;
    }

    bool
    SetTestSpace::same(SetTestSpace& c) {
        if (opt.log)
            olog << ind(3) << "Testing whether enabled space is the same" << '\n';
        bool f = failed();
        bool cf = c.failed();
        if (f != cf)
            return false;
        if (f)
            return true;

        for (int i = x.size(); i--;)
            if (x[i].glb_size() != c.x[i].glb_size() || x[i].lub_size() != c.x[i].lub_size() ||
                x[i].card_min() != c.x[i].card_min() || x[i].card_max() != c.x[i].card_max())
                return false;

        for (int i = y.size(); i--;)
            if (y[i].size() != c.y[i].size())
                return false;

        if (reified && (r.var().size() != c.r.var().size()))
            return false;
        if (opt.log)
            olog << ind(3) << "Finished testing whether enabled space is the same" << '\n';
        return true;
    }

    bool
    SetTestSpace::prune(const SetAssignment& a, Gecode::Support::RandomGenerator& rand) {
        using namespace Gecode;
        bool sets_assigned = true;
        for (int j = x.size(); j--;)
            if (!x[j].assigned()) {
                sets_assigned = false;
                break;
            }
        bool ints_assigned = true;
        for (int j = y.size(); j--;)
            if (!y[j].assigned()) {
                ints_assigned = false;
                break;
            }

        // Select variable to be pruned
        int i;
        if (ints_assigned) {
            i = rand(x.size());
        } else if (sets_assigned) {
            i = rand(y.size());
        } else {
            i = rand(x.size() + y.size());
        }

        if (sets_assigned || i >= x.size()) {
            if (i >= x.size())
                i = i - x.size();
            while (y[i].assigned()) {
                i = (i + 1) % y.size();
            }
            // Prune int var

            // Select mode for pruning
            switch (rand(3)) {
            case 0:
                if (a.ints()[i] < y[i].max()) {
                    int v = a.ints()[i] + 1 + rand(static_cast<unsigned int>(y[i].max() - a.ints()[i]));
                    assert((v > a.ints()[i]) && (v <= y[i].max()));
                    rel(i, Gecode::IRT_LT, v);
                }
                break;
            case 1:
                if (a.ints()[i] > y[i].min()) {
                    int v = y[i].min() + rand(static_cast<unsigned int>(a.ints()[i] - y[i].min()));
                    assert((v < a.ints()[i]) && (v >= y[i].min()));
                    rel(i, Gecode::IRT_GR, v);
                }
                break;
            default:
                int v;
                Gecode::Int::ViewRanges<Gecode::Int::IntView> it(y[i]);
                unsigned int skip = rand(y[i].size() - 1);
                while (true) {
                    if (it.width() > skip) {
                        v = it.min() + skip;
                        if (v == a.ints()[i]) {
                            if (it.width() == 1) {
                                ++it;
                                v = it.min();
                            } else if (v < it.max()) {
                                ++v;
                            } else {
                                --v;
                            }
                        }
                        break;
                    }
                    skip -= it.width();
                    ++it;
                }
                rel(i, Gecode::IRT_NQ, v);
            }
            return (!Base::fixpoint(rand) || check_fixpoint());
        }
        while (x[i].assigned()) {
            i = (i + 1) % x.size();
        }
        Gecode::SetVarUnknownRanges ur1(x[i]);
        CountableSetRanges air1(a.lub, a[i]);
        Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges> diff(ur1, air1);
        Gecode::SetVarUnknownRanges ur2(x[i]);
        CountableSetRanges air2(a.lub, a[i]);
        Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges> inter(ur2, air2);

        CountableSetRanges assignment_ranges(a.lub, a[i]);
        unsigned int assignment_size = Gecode::Iter::Ranges::size(assignment_ranges);

        // Select mode for pruning
        switch (rand(5)) {
        case 0:
            if (inter()) {
                int v = rand(Gecode::Iter::Ranges::size(inter));
                add_to_glb(v, i, a);
            }
            break;
        case 1:
            if (diff()) {
                int v = rand(Gecode::Iter::Ranges::size(diff));
                remove_from_lub(v, i, a);
            }
            break;
        case 2:
            if (x[i].card_min() < assignment_size) {
                unsigned int new_cardinality = x[i].card_min() + 1 + rand(assignment_size - x[i].card_min());
                assert(new_cardinality > x[i].card_min());
                assert(new_cardinality <= assignment_size);
                cardinality(i, new_cardinality, Gecode::Set::Limits::card);
            }
            break;
        case 3:
            if (x[i].card_max() > assignment_size) {
                unsigned int new_cardinality = x[i].card_max() - 1 - rand(x[i].card_max() - assignment_size);
                assert(new_cardinality < x[i].card_max());
                assert(new_cardinality >= assignment_size);
                cardinality(i, 0, new_cardinality);
            }
            break;
        default:
            if (inter()) {
                int v = rand(Gecode::Iter::Ranges::size(inter));
                add_to_glb(v, i, a);
            } else {
                int v = rand(Gecode::Iter::Ranges::size(diff));
                remove_from_lub(v, i, a);
            }
        }
        return (!Base::fixpoint(rand) || check_fixpoint());
    }

    bool
    SetTestSpace::disabled(const SetAssignment& a, SetTestSpace& c, Gecode::Support::RandomGenerator& rand) {
        c.disable();
        using namespace Gecode;
        bool sets_assigned = true;
        for (int j = x.size(); j--;)
            if (!x[j].assigned()) {
                sets_assigned = false;
                break;
            }
        bool ints_assigned = true;
        for (int j = y.size(); j--;)
            if (!y[j].assigned()) {
                ints_assigned = false;
                break;
            }

        // Select variable to be pruned
        int i;
        if (ints_assigned) {
            i = rand(x.size());
        } else if (sets_assigned) {
            i = rand(y.size());
        } else {
            i = rand(x.size() + y.size());
        }

        if (sets_assigned || i >= x.size()) {
            if (i >= x.size())
                i = i - x.size();
            while (y[i].assigned()) {
                i = (i + 1) % y.size();
            }
            // Prune int var

            // Select mode for pruning
            switch (rand(3)) {
            case 0:
                if (a.ints()[i] < y[i].max()) {
                    int v = a.ints()[i] + 1 + rand(static_cast<unsigned int>(y[i].max() - a.ints()[i]));
                    assert((v > a.ints()[i]) && (v <= y[i].max()));
                    rel(i, Gecode::IRT_LT, v);
                    c.rel(i, Gecode::IRT_LT, v);
                }
                break;
            case 1:
                if (a.ints()[i] > y[i].min()) {
                    int v = y[i].min() + rand(static_cast<unsigned int>(a.ints()[i] - y[i].min()));
                    assert((v < a.ints()[i]) && (v >= y[i].min()));
                    rel(i, Gecode::IRT_GR, v);
                    c.rel(i, Gecode::IRT_GR, v);
                }
                break;
            default:
                int v;
                Gecode::Int::ViewRanges<Gecode::Int::IntView> it(y[i]);
                unsigned int skip = rand(y[i].size() - 1);
                while (true) {
                    if (it.width() > skip) {
                        v = it.min() + skip;
                        if (v == a.ints()[i]) {
                            if (it.width() == 1) {
                                ++it;
                                v = it.min();
                            } else if (v < it.max()) {
                                ++v;
                            } else {
                                --v;
                            }
                        }
                        break;
                    }
                    skip -= it.width();
                    ++it;
                }
                rel(i, Gecode::IRT_NQ, v);
                c.rel(i, Gecode::IRT_NQ, v);
            }
            c.enable();
            return same(c);
        }
        while (x[i].assigned()) {
            i = (i + 1) % x.size();
        }
        Gecode::SetVarUnknownRanges ur1(x[i]);
        CountableSetRanges air1(a.lub, a[i]);
        Gecode::Iter::Ranges::Diff<Gecode::SetVarUnknownRanges, CountableSetRanges> diff(ur1, air1);
        Gecode::SetVarUnknownRanges ur2(x[i]);
        CountableSetRanges air2(a.lub, a[i]);
        Gecode::Iter::Ranges::Inter<Gecode::SetVarUnknownRanges, CountableSetRanges> inter(ur2, air2);

        CountableSetRanges assignment_ranges(a.lub, a[i]);
        unsigned int assignment_size = Gecode::Iter::Ranges::size(assignment_ranges);

        // Select mode for pruning
        switch (rand(5)) {
        case 0:
            if (inter()) {
                int v = rand(Gecode::Iter::Ranges::size(inter));
                add_to_glb(v, i, a, c);
            }
            break;
        case 1:
            if (diff()) {
                int v = rand(Gecode::Iter::Ranges::size(diff));
                remove_from_lub(v, i, a, c);
            }
            break;
        case 2:
            if (x[i].card_min() < assignment_size) {
                unsigned int new_cardinality = x[i].card_min() + 1 + rand(assignment_size - x[i].card_min());
                assert(new_cardinality > x[i].card_min());
                assert(new_cardinality <= assignment_size);
                cardinality(i, new_cardinality, Gecode::Set::Limits::card);
                c.cardinality(i, new_cardinality, Gecode::Set::Limits::card);
            }
            break;
        case 3:
            if (x[i].card_max() > assignment_size) {
                unsigned int new_cardinality = x[i].card_max() - 1 - rand(x[i].card_max() - assignment_size);
                assert(new_cardinality < x[i].card_max());
                assert(new_cardinality >= assignment_size);
                cardinality(i, 0, new_cardinality);
                c.cardinality(i, 0, new_cardinality);
            }
            break;
        default:
            if (inter()) {
                int v = rand(Gecode::Iter::Ranges::size(inter));
                add_to_glb(v, i, a, c);
            } else {
                int v = rand(Gecode::Iter::Ranges::size(diff));
                remove_from_lub(v, i, a, c);
            }
        }
        c.enable();
        return same(c);
    }

    unsigned int
    SetTestSpace::propagators() {
        return Gecode::PropagatorGroup::all.size(*this);
    }

    void
    SetTestSpace::enable() {
        Gecode::PropagatorGroup::all.enable(*this);
    }

    void
    SetTestSpace::disable() {
        Gecode::PropagatorGroup::all.disable(*this);
        (void)status();
    }

    /// Check the test result and handle failed test
#define CHECK_TEST(T, M)                                                                                               \
    do {                                                                                                               \
        if (opt.log)                                                                                                   \
            olog << ind(3) << "Check: " << (M) << '\n';                                                                \
        if (!(T)) {                                                                                                    \
            problem = (M);                                                                                             \
            delete s;                                                                                                  \
            goto failed;                                                                                               \
        }                                                                                                              \
    } while (false)

    /// Start new test
#define START_TEST(T)                                                                                                  \
    do {                                                                                                               \
        if (opt.log) {                                                                                                 \
            olog.str("");                                                                                              \
            olog << ind(2) << "Testing: " << (T) << '\n';                                                              \
        }                                                                                                              \
        test = (T);                                                                                                    \
    } while (false)

    bool
    SetTest::run() {
        using namespace Gecode;
        const char* test = "NONE";
        const char* problem = "NONE";

        SetAssignment* ap = new SetAssignment(arity, lub, with_int);
        SetAssignment& a = *ap;
        while (a()) {
            bool is_sol = solution(a);
            if (opt.log)
                olog << ind(1) << "Assignment: " << a << (is_sol ? " (solution)" : " (no solution)") << '\n';
            START_TEST("Assignment (after posting)");
            {
                SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this);
                SetTestSpace* sc = nullptr;
                s->post();
                switch (_rand(2)) {
                case 0:
                    if (opt.log)
                        olog << ind(3) << "No copy" << '\n';
                    sc = s;
                    s = nullptr;
                    break;
                case 1:
                    if (opt.log)
                        olog << ind(3) << "Copy" << '\n';
                    if (s->status() != Gecode::SS_FAILED) {
                        sc = static_cast<SetTestSpace*>(s->clone());
                    } else {
                        sc = s;
                        s = nullptr;
                    }
                    break;
                default:
                    assert(false);
                }
                sc->assign(a, _rand);
                if (is_sol) {
                    CHECK_TEST(!sc->failed(), "Failed on solution");
                    CHECK_TEST(sc->subsumed(test_subsumption), "No subsumption");
                } else {
                    CHECK_TEST(sc->failed(), "Solved on non-solution");
                }
                delete s;
                delete sc;
            }
            START_TEST("Assignment (after posting, disable)");
            {
                SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this);
                s->post();
                s->disable();
                s->assign(a, _rand);
                s->enable();
                if (is_sol) {
                    CHECK_TEST(!s->failed(), "Failed on solution");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                } else {
                    CHECK_TEST(s->failed(), "Solved on non-solution");
                }
                delete s;
            }
            START_TEST("Assignment (before posting)");
            {
                SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this);
                s->assign(a, _rand);
                s->post();
                if (is_sol) {
                    CHECK_TEST(!s->failed(), "Failed on solution");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                } else {
                    CHECK_TEST(s->failed(), "Solved on non-solution");
                }
                delete s;
            }
            START_TEST("Prune");
            {
                SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this);
                s->post();
                while (!s->failed() && !s->assigned())
                    if (!s->prune(a, _rand)) {
                        problem = "No fixpoint";
                        delete s;
                        goto failed;
                    }
                s->assign(a, _rand);
                if (is_sol) {
                    CHECK_TEST(!s->failed(), "Failed on solution");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                } else {
                    CHECK_TEST(s->failed(), "Solved on non-solution");
                }
                delete s;
            }
            if (disabled) {
                START_TEST("Prune (disable)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this);
                    SetTestSpace* c = new SetTestSpace(arity, lub, with_int, this);
                    s->post();
                    c->post();
                    while (!s->failed() && !s->assigned())
                        if (!s->disabled(a, *c, _rand)) {
                            problem = "Different result after re-enable";
                            delete s;
                            delete c;
                            goto failed;
                        }
                    s->assign(a, _rand);
                    c->assign(a, _rand);
                    if (s->failed() != c->failed()) {
                        problem = "Different failure after re-enable";
                        delete s;
                        delete c;
                        goto failed;
                    }
                    delete s;
                    delete c;
                }
            }
            if (reified) {
                START_TEST("Assignment reified (rewrite after post, <=>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->post();
                    s->rel(is_sol);
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                START_TEST("Assignment reified (rewrite after post, =>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->post();
                    s->rel(is_sol);
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                START_TEST("Assignment reified (rewrite after post, <=)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->post();
                    s->rel(is_sol);
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                {
                    START_TEST("Assignment reified (rewrite failure, <=>)");
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->post();
                    s->rel(!is_sol);
                    s->assign(a, _rand);
                    CHECK_TEST(s->failed(), "Not failed");
                    delete s;
                }
                {
                    START_TEST("Assignment reified (rewrite failure, =>)");
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->post();
                    s->rel(!is_sol);
                    s->assign(a, _rand);
                    if (is_sol) {
                        CHECK_TEST(!s->failed(), "Failed");
                        CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    } else {
                        CHECK_TEST(s->failed(), "Not failed");
                    }
                    delete s;
                }
                {
                    START_TEST("Assignment reified (rewrite failure, <=)");
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->post();
                    s->rel(!is_sol);
                    s->assign(a, _rand);
                    if (is_sol) {
                        CHECK_TEST(s->failed(), "Not failed");
                    } else {
                        CHECK_TEST(!s->failed(), "Failed");
                        CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (immediate rewrite, <=>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->rel(is_sol);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                START_TEST("Assignment reified (immediate rewrite, =>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->rel(is_sol);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                START_TEST("Assignment reified (immediate rewrite, <=)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->rel(is_sol);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    delete s;
                }
                START_TEST("Assignment reified (immediate failure, <=>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->rel(!is_sol);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(s->failed(), "Not failed");
                    delete s;
                }
                START_TEST("Assignment reified (immediate failure, =>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->rel(!is_sol);
                    s->post();
                    s->assign(a, _rand);
                    if (is_sol) {
                        CHECK_TEST(!s->failed(), "Failed");
                        CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    } else {
                        CHECK_TEST(s->failed(), "Not failed");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (immediate failure, <=)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->rel(!is_sol);
                    s->post();
                    s->assign(a, _rand);
                    if (is_sol) {
                        CHECK_TEST(s->failed(), "Not failed");
                    } else {
                        CHECK_TEST(!s->failed(), "Failed");
                        CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (before posting, <=>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->assign(a, _rand);
                    s->post();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (before posting, =>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->assign(a, _rand);
                    s->post();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    } else {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (before posting, <=)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->assign(a, _rand);
                    s->post();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, <=>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, =>)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    } else {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, <=)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->post();
                    s->assign(a, _rand);
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, <=>, disable)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->post();
                    s->disable();
                    s->assign(a, _rand);
                    s->enable();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, =>, disable)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->post();
                    s->disable();
                    s->assign(a, _rand);
                    s->enable();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    } else {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Assignment reified (after posting, <=, disable)");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->post();
                    s->disable();
                    s->assign(a, _rand);
                    s->enable();
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    }
                    delete s;
                }
                START_TEST("Prune reified, <=>");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_EQV);
                    s->post();
                    while (!s->failed() && (!s->assigned() || !s->r.var().assigned()))
                        if (!s->prune(a, _rand)) {
                            problem = "No fixpoint";
                            delete s;
                            goto failed;
                        }
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Prune reified, =>");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_IMP);
                    s->post();
                    while (!s->failed() && (!s->assigned() || (!is_sol && !s->r.var().assigned()))) {
                        if (!s->prune(a, _rand)) {
                            problem = "No fixpoint";
                            delete s;
                            goto failed;
                        }
                    }
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    } else {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 0, "One on non-solution");
                    }
                    delete s;
                }
                START_TEST("Prune reified, <=");
                {
                    SetTestSpace* s = new SetTestSpace(arity, lub, with_int, this, RM_RIMP);
                    s->post();
                    while (!s->failed() && (!s->assigned() || (is_sol && !s->r.var().assigned())))
                        if (!s->prune(a, _rand)) {
                            problem = "No fixpoint";
                            delete s;
                            goto failed;
                        }
                    CHECK_TEST(!s->failed(), "Failed");
                    CHECK_TEST(s->subsumed(test_subsumption), "No subsumption");
                    if (is_sol) {
                        CHECK_TEST(s->r.var().assigned(), "Control variable unassigned");
                        CHECK_TEST(s->r.var().val() == 1, "Zero on solution");
                    } else {
                        CHECK_TEST(!s->r.var().assigned(), "Control variable assigned");
                    }
                    delete s;
                }
            }
            a.next(_rand);
        }
        delete ap;
        return true;
    failed:
        if (opt.log)
            olog << "FAILURE" << '\n'
                 << ind(1) << "Test:       " << test << '\n'
                 << ind(1) << "Problem:    " << problem << '\n';
        if (a() && opt.log)
            olog << ind(1) << "Assignment: " << a << '\n';
        delete ap;

        return false;
    }

    const Gecode::SetRelType SetRelTypes::srts[] = {Gecode::SRT_EQ,  Gecode::SRT_NQ,       Gecode::SRT_SUB,
                                                    Gecode::SRT_SUP, Gecode::SRT_DISJOINT, Gecode::SRT_COMPLEMENT};

    const Gecode::SetOpType SetOpTypes::sots[] = {Gecode::SOT_UNION, Gecode::SOT_DISJOINT_UNION,
                                                  Gecode::SOT_INTERSECTION, Gecode::SOT_MINUS};

} // namespace Test::Set

#undef START_TEST
#undef CHECK_TEST

// STATISTICS: test-set
