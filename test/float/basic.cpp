/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/float.hh"

#include <cmath>

/// %Tests for basic setup
namespace Test::Float::Basic {

    /**
     * \defgroup TaskTestFloatBasic Basic setup
     * \ingroup TaskTestFloat
     */
    //@{

    /// %Test whether testing infrastructure for float variables works
    class Basic : public Test {
    public:
        /// Initialize test
        Basic(Gecode::FloatNum n, Gecode::FloatNum s) : Test("Basic", 3, -n, n, s, COMPLETE_ASSIGNMENT, false) {}
        /// Initialize test
        Basic(const Gecode::FloatVal& v, Gecode::FloatNum s) : Test("Basic", 3, v, s, COMPLETE_ASSIGNMENT, false) {}
        /// Check whether \a x is a solution
        MaybeType
        solution(const Assignment&) const override {
            return MT_TRUE;
        }
        /// Post constraint on \a x
        void
        post(Gecode::Space&, Gecode::FloatVarArray&) override {}
        /// Post reified constraint on \a x for \a r
        void
        post(Gecode::Space&, Gecode::FloatVarArray&, Gecode::Reify) override {
            // Reification for no constraint is not interesting
            GECODE_NEVER;
        }
    };

    namespace {
        Basic b1(3, 1.5);
        Basic b2(Gecode::FloatVal(-2, 10), 1.5);
    } // namespace

    /// Test interval medians at the numerical limits and special values
    class Median : public Base {
    private:
        /// Test median inclusion and splitting of non-tight intervals
        static bool
        valid(const Gecode::FloatVal& x) {
            const Gecode::FloatNum m = x.med();
            return (m >= x.min()) && (m <= x.max()) && (x.tight() || ((m > x.min()) && (m < x.max())));
        }

    public:
        /// Create and register test
        Median() : Base("Float::Basic::Median") {}
        /// Perform test
        bool
        run() override {
            using namespace Gecode;
            const FloatNum m = Gecode::Float::Limits::max;
            const FloatNum i = std::numeric_limits<FloatNum>::infinity();
            const FloatNum d = std::numeric_limits<FloatNum>::denorm_min();
            const FloatVal negative(-m, -m / 2.0);
            const FloatVal positive(m / 2.0, m);
            const FloatVal mixed(-m, m);
            // The subtraction-based formula rounds this median one ULP upward.
            const FloatVal closest(3.07187504409887e-53, 7.805365587017815e-45);
            return valid(negative) && valid(positive) && valid(mixed) && (closest.med() == 3.902682808868283e-45) &&
                   (FloatVal(-i, -1.0).med() == -i) && (FloatVal(1.0, i).med() == i) &&
                   (FloatVal(-i, i).med() == 0.0) && (FloatVal(-i, -i).med() == -i) && (FloatVal(i, i).med() == i) &&
                   !std::signbit(FloatVal(-0.0, 0.0).med()) && (FloatVal(0.0, 2.0 * d).med() == d) &&
                   (FloatVal(-2.0 * d, 0.0).med() == -d);
        }
    };

    namespace {
        Median median;
    }

    /// Test comparing float views
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
        ViewCompare() : Base("Float::Basic::View::Compare") {}
        /// Perform test
        bool
        run() override {
            using namespace Gecode;
            using namespace Gecode::Float;
            Fixture home;
            FloatView x(FloatVar(home, 0.0, 1.0));
            FloatView y(FloatVar(home, 0.0, 1.0));
            MinusView mx(x);
            return compares_equal(mx, MinusView(x)) && compares_unequal(mx, MinusView(y)) &&
                   compares_equal(OffsetView(x, 1.0), OffsetView(x, 1.0)) &&
                   compares_unequal(OffsetView(x, 1.0), OffsetView(x, 2.0)) &&
                   compares_unequal(OffsetView(x, 1.0), OffsetView(y, 1.0)) &&
                   compares_equal(ScaleView(2.0, x), ScaleView(2.0, x)) &&
                   compares_unequal(ScaleView(2.0, x), ScaleView(3.0, x)) &&
                   compares_unequal(ScaleView(2.0, x), ScaleView(2.0, y)) && compares_unequal(x, mx);
        }
    };

    namespace {
        ViewCompare view_compare;
    }
    //@}

} // namespace Test::Float::Basic
// STATISTICS: test-float
