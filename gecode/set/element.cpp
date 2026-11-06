/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include <gecode/set/element.hh>

namespace Gecode {

    namespace Int {
        template <>
        /// VarArg type for constant %Set views
        class ViewToVarArg<Gecode::Set::ConstSetView> {
        public:
            using argtype = IntSetArgs;
        };
    } // namespace Int

    using namespace Gecode::Set;

    void
    element(Home home, SetOpType op, const SetVarArgs& x, SetVar y, SetVar z, const IntSet& universe) {
        GECODE_POST;

        switch (op) {
        case SOT_DISJOINT_UNION: {
            Set::Element::ElementDisjoint<SetView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL((Element::ElementDisjoint<SetView, SetView>::post(home, iv, y)));
        }
            // fall through
        case SOT_UNION: {
            Set::Element::ElementUnion<SetView, SetView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL((Element::ElementUnion<SetView, SetView, SetView>::post(home, iv, y, z)));
        } break;
        case SOT_INTERSECTION: {
            Set::Element::ElementIntersection<SetView, SetView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL((Element::ElementIntersection<SetView, SetView, SetView>::post(home, iv, y, z, universe)));
        } break;
        case SOT_MINUS:
            throw IllegalOperation("Set::element");
            break;
        default:
            throw UnknownOperation("Set::element");
        }
    }

    void
    element(Home home, SetOpType op, const IntVarArgs& x, SetVar y, SetVar z, const IntSet& universe) {
        GECODE_POST;

        switch (op) {
        case SOT_DISJOINT_UNION: {
            Set::Element::ElementDisjoint<SingletonView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL((Element::ElementDisjoint<SingletonView, SetView>::post(home, iv, y)));
        }
            // fall through
        case SOT_UNION: {
            Set::Element::ElementUnion<SingletonView, SetView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL((Element::ElementUnion<SingletonView, SetView, SetView>::post(home, iv, y, z)));
        } break;
        case SOT_INTERSECTION: {
            Set::Element::ElementIntersection<SingletonView, SetView, SetView>::IdxViewArray iv(home, x);
            GECODE_ES_FAIL(
                (Element::ElementIntersection<SingletonView, SetView, SetView>::post(home, iv, y, z, universe)));
        } break;
        case SOT_MINUS:
            throw IllegalOperation("Set::element");
            break;
        default:
            throw UnknownOperation("Set::element");
        }
    }

    void
    element(Home home, SetOpType op, const IntSetArgs& x, SetVar y, SetVar z, const IntSet& universe) {
        const int n = x.size();
        GECODE_POST;

        switch (op) {
        case SOT_DISJOINT_UNION: {
            Set::Element::ElementDisjoint<ConstSetView, SetView>::IdxViewArray iv(home, n);
            for (int i = n; i--;) {
                iv[i].idx = i;
                iv[i].view = ConstSetView(home, x[i]);
            }
            GECODE_ES_FAIL((Element::ElementDisjoint<ConstSetView, SetView>::post(home, iv, y)));
        }
            // fall through
        case SOT_UNION: {
            GECODE_ES_FAIL((Element::ElementUnionConst<SetView, SetView>::post(home, z, x, y)));
        } break;
        case SOT_INTERSECTION: {
            Set::Element::ElementIntersection<ConstSetView, SetView, SetView>::IdxViewArray iv(home, n);
            for (int i = n; i--;) {
                iv[i].idx = i;
                iv[i].view = ConstSetView(home, x[i]);
            }
            GECODE_ES_FAIL(
                (Element::ElementIntersection<ConstSetView, SetView, SetView>::post(home, iv, y, z, universe)));
        } break;
        case SOT_MINUS:
            throw IllegalOperation("Set::element");
            break;
        default:
            throw UnknownOperation("Set::element");
        }
    }

    void
    element(Home home, SetOpType op, const IntArgs& x, SetVar y, SetVar z, const IntSet& universe) {
        const int n = x.size();
        IntSetArgs xs(n);
        for (int i = n; i--;)
            xs[i] = IntSet(x[i], x[i]);
        element(home, op, xs, y, z, universe);
    }

    void
    element(Home home, const SetVarArgs& x, IntVar y, SetVar z) {
        if (x.size() == 0)
            throw Set::TooFewArguments("Set::element");
        GECODE_POST;
        Set::Element::ElementUnion<SetView, SingletonView, SetView>::IdxViewArray iv(home, x);
        SetView zv(z);

        Int::IntView yv(y);
        SingletonView single(yv);
        GECODE_ES_FAIL((Element::ElementUnion<SetView, SingletonView, SetView>::post(home, iv, single, zv)));
    }

    void
    element(Home home, const IntSetArgs& x, IntVar y, SetVar z) {
        if (x.size() == 0)
            throw Set::TooFewArguments("Set::element");
        for (int i = x.size(); i--;)
            Set::Limits::check(x[i], "Set::element");
        GECODE_POST;
        SetView zv(z);

        Int::IntView yv(y);
        SingletonView single(yv);
        GECODE_ES_FAIL((Element::ElementUnionConst<SetView, SingletonView>::post(home, z, x, single)));
    }

    namespace {
        /// Check that \a n elements form a non-empty matrix of width \a w and height \a h
        void
        check_matrix(int n, int w, int h) {
            if (n == 0)
                throw Set::TooFewArguments("Set::element");
            if ((w < 1) || (h < 1) || (static_cast<long long int>(w) * h != n))
                throw Set::ArgumentSizeMismatch("Set::element");
        }

        IntVar
        pair(Home home, IntVar x, int w, IntVar y, int h) {
            IntVar xy(home, 0, w * h - 1);
            if (Int::Element::Pair::post(home, x, y, xy, w, h) != ES_OK)
                home.fail();
            return xy;
        }
    } // namespace

    void
    element(Home home, const IntSetArgs& a, IntVar x, int w, IntVar y, int h, SetVar z) {
        check_matrix(a.size(), w, h);
        GECODE_POST;
        element(home, a, pair(home, x, w, y, h), z);
    }

    void
    element(Home home, const SetVarArgs& a, IntVar x, int w, IntVar y, int h, SetVar z) {
        check_matrix(a.size(), w, h);
        GECODE_POST;
        element(home, a, pair(home, x, w, y, h), z);
    }

} // namespace Gecode

// STATISTICS: set-post
