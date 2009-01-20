/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2005
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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
#include <gecode/set/projectors.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

namespace Test { namespace Set {

  /// Tests for projection constraints
  namespace Projection {

    /**
      * \defgroup TaskTestSetProj Projection constraints
      * \ingroup TaskTestSet
      */
    //@{

    static IntSet ds_22(-2,2);
    static IntSet ds_33(-3,3);

    /// Test for binary disequality constraint
    class RelBinNEq : public SetTest {
    public:
      /// Create and register test
      RelBinNEq(const char* t)
        : SetTest(t,2,ds_33) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return !Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p(0, Gecode::SetExpr(1), Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(0), Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], ps, true);
      }
    };
    RelBinNEq _relneq("Proj::Rel::BinNEq");

    /// Test for binary equality constraint
    class RelBinEq : public SetTest {
    public:
      /// Create and register test
      RelBinEq(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::equal(xr0, xr1);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::SetExpr xe(0);
        Gecode::SetExpr ye(1);
        Gecode::Projector p(0, ye, ye);
        Gecode::Projector q(1, xe, xe);
        Gecode::ProjectorSet ps(2);
        ps.add(p);
        ps.add(q);
        Gecode::projector(home, x[0], x[1], ps);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,
                        BoolVar b) {
        Gecode::Projector p(0, Gecode::SetExpr(1), Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(0), Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], b, ps);
      }
    };
    RelBinEq _releq("Proj::Rel::BinEq");

    /// Test for binary subset constraint
    class RelBinSub : public SetTest {
    public:
      /// Create and register test
      RelBinSub(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::subset(xr0, xr1);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p(0, Gecode::SetExpr(), Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(0), -Gecode::SetExpr());
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], ps);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,
                        BoolVar b) {
        Gecode::Projector p(0, Gecode::SetExpr(), Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(0), -Gecode::SetExpr());
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], b, ps);
      }
    };
    RelBinSub _relsub("Proj::Rel::BinSub");

    /// Test for binary disjointness constraint
    class RelBinDisj : public SetTest {
    public:
      /// Create and register test
      RelBinDisj(const char* t)
        : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        return Iter::Ranges::disjoint(xr0, xr1);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p(0, Gecode::SetExpr(), -Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(), -Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], ps);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,
                        BoolVar b) {
        Gecode::Projector p(0, Gecode::SetExpr(), -Gecode::SetExpr(1));
        Gecode::Projector q(1, Gecode::SetExpr(), -Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], b, ps);
      }
    };
    RelBinDisj _reldisj("Proj::Rel::BinDisj");

    /// Test for binary complement constraint
    class RelBinCompl : public SetTest {
    public:
      /// Create and register test
      RelBinCompl(const char* t)
      : SetTest(t,2,ds_33,true) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        Gecode::Set::RangesCompl<CountableSetRanges> xr1c(xr1);
        return Iter::Ranges::equal(xr0, xr1c);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p(0, -Gecode::SetExpr(1), -Gecode::SetExpr(1));
        Gecode::Projector q(1, -Gecode::SetExpr(0), -Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], ps);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,
                        BoolVar b) {
        Gecode::Projector p(0, -Gecode::SetExpr(1), -Gecode::SetExpr(1));
        Gecode::Projector q(1, -Gecode::SetExpr(0), -Gecode::SetExpr(0));
        Gecode::ProjectorSet ps(2);
        ps.add(p); ps.add(q);
        Gecode::projector(home, x[0], x[1], b, ps);
      }
    };
    RelBinCompl _relcompl("Proj::Rel::BinCompl");

    /// Test for ternary union constraint
    class RelUnionEq : public SetTest {
    public:
      /// Create and register test
      RelUnionEq(const char* t)
        : SetTest(t,3,ds_22,true) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr0,xr1);
        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p0(0,
                             Gecode::SetExpr(2) - Gecode::SetExpr(1),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p1(1,
                             Gecode::SetExpr(2) - Gecode::SetExpr(0),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p2(2,
                             Gecode::SetExpr(0) || Gecode::SetExpr(1),
                             Gecode::SetExpr(0) || Gecode::SetExpr(1)
                             );
        Gecode::ProjectorSet ps(3);
        ps.add(p0); ps.add(p1); ps.add(p2);
        Gecode::projector(home, x[0], x[1], x[2], ps);

        IntVar x0c(home, 0, 10);
        IntVar x1c(home, 0, 10);
        IntVar x2c(home, 0, 10);
        Gecode::cardinality(home, x[0], x0c);
        Gecode::cardinality(home, x[1], x1c);
        Gecode::cardinality(home, x[2], x2c);

        Gecode::Projector p3(2,
                             Gecode::SetExpr(0) && Gecode::SetExpr(1),
                             Gecode::SetExpr(0) && Gecode::SetExpr(1)
                             );
        IntVar x0ix1c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x0ix1c, p3);
        /// Post constraint on \a x
        Gecode::post(home, x2c == x0c + x1c - x0ix1c);

        Gecode::Projector p4(2,
                             Gecode::SetExpr(0) - Gecode::SetExpr(1),
                             Gecode::SetExpr(0) - Gecode::SetExpr(1)
                             );
        IntVar x0mx1c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x0mx1c, p4);
        /// Post constraint on \a x
        Gecode::post(home, x1c == x2c - x0mx1c);

        Gecode::Projector p5(2,
                             Gecode::SetExpr(1) - Gecode::SetExpr(0),
                             Gecode::SetExpr(1) - Gecode::SetExpr(0)
                             );
        IntVar x1mx0c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x1mx0c, p5);
        /// Post constraint on \a x
        Gecode::post(home, x0c == x2c - x1mx0c);
      }
      /// Post reified constraint on \a x for \a b
      virtual void post(Space& home, SetVarArray& x, IntVarArray&,
                        BoolVar b) {
        Gecode::Projector p0(0,
                             Gecode::SetExpr(2) - Gecode::SetExpr(1),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p1(1,
                             Gecode::SetExpr(2) - Gecode::SetExpr(0),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p2(2,
                             Gecode::SetExpr(0) || Gecode::SetExpr(1),
                             Gecode::SetExpr(0) || Gecode::SetExpr(1)
                             );
        Gecode::ProjectorSet ps(3);
        ps.add(p0); ps.add(p1); ps.add(p2);
        Gecode::projector(home, x[0], x[1], x[2], b, ps);
      }
    };
    RelUnionEq _relunioneq("Proj::RelOp::UnionEq");

    /// Test for ternary union constraint, posted using a formula
    class RelUnionEqFormula : public SetTest {
    public:
      /// Create and register test
      RelUnionEqFormula(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr0,xr1);
        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Formula f = (Formula(0) | Formula(1)) == Formula(2);
        Gecode::ProjectorSet ps = f.projectors();
        Gecode::projector(home, x[0], x[1], x[2], ps);
      }
    };
    RelUnionEqFormula _relunioneqfor("Proj::Formula::RelOp::UnionEq");

    /// Test for ternary intersection constraint with cardinality propagation
    class RelInterEqCard : public SetTest {
    public:
      /// Create and register test
      RelInterEqCard(const char* t)
        : SetTest(t,3,ds_22,false) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges>
          u(xr0,xr1);
        return Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p0(0,
                             Gecode::SetExpr(2),
                             Gecode::SetExpr(2) || -Gecode::SetExpr(1)
                             );
        Gecode::Projector p1(1,
                             Gecode::SetExpr(2),
                             Gecode::SetExpr(2) || -Gecode::SetExpr(0)
                             );
        Gecode::Projector p2(2,
                             Gecode::SetExpr(0) && Gecode::SetExpr(1),
                             Gecode::SetExpr(0) && Gecode::SetExpr(1)
                             );
        Gecode::ProjectorSet ps(3);
        ps.add(p0); ps.add(p1); ps.add(p2);
        Gecode::projector(home, x[0], x[1], x[2], ps);

        IntVar x0c(home, 0, 10);
        IntVar x1c(home, 0, 10);
        IntVar x2c(home, 0, 10);
        Gecode::cardinality(home, x[0], x0c);
        Gecode::cardinality(home, x[1], x1c);
        Gecode::cardinality(home, x[2], x2c);

        Gecode::Projector p3(2,
                             Gecode::SetExpr(0) || Gecode::SetExpr(1),
                             Gecode::SetExpr(0) || Gecode::SetExpr(1)
                             );
        IntVar x0ux1c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x0ux1c, p3);
        /// Post constraint on \a x
        Gecode::post(home, x2c == x0c + x1c - x0ux1c);

        Gecode::Projector p4(2,
                             Gecode::SetExpr(0) - Gecode::SetExpr(1),
                             Gecode::SetExpr(0) - Gecode::SetExpr(1)
                             );
        IntVar x0mx1c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x0mx1c, p4);
        /// Post constraint on \a x
        Gecode::post(home, x2c == x0c - x0mx1c);

        Gecode::Projector p5(2,
                             Gecode::SetExpr(1) - Gecode::SetExpr(0),
                             Gecode::SetExpr(1) - Gecode::SetExpr(0)
                             );
        IntVar x1mx0c(home, 0, 20);
        Gecode::projector(home, x[0], x[1], x[2], x1mx0c, p5);
        /// Post constraint on \a x
        Gecode::post(home, x2c == x1c - x1mx0c);

      }
    };
    RelInterEqCard _relintereqcard("Proj::RelOp::InterEqCard");

    /// Test for negated ternary union constraint
    class NegRelUnionEq : public SetTest {
    public:
      /// Create and register test
      NegRelUnionEq(const char* t)
        : SetTest(t,3,ds_22) {}
      /// Test whether \a x is solution
      virtual bool solution(const SetAssignment& x) const {
        CountableSetRanges xr0(x.lub, x[0]);
        CountableSetRanges xr1(x.lub, x[1]);
        CountableSetRanges xr2(x.lub, x[2]);
        Iter::Ranges::Union<CountableSetRanges, CountableSetRanges>
          u(xr0,xr1);
        return !Iter::Ranges::equal(xr2, u);
      }
      /// Post constraint on \a x
      virtual void post(Space& home, SetVarArray& x, IntVarArray&) {
        Gecode::Projector p0(0,
                             Gecode::SetExpr(2) - Gecode::SetExpr(1),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p1(1,
                             Gecode::SetExpr(2) - Gecode::SetExpr(0),
                             Gecode::SetExpr(2)
                             );
        Gecode::Projector p2(2,
                             Gecode::SetExpr(0) || Gecode::SetExpr(1),
                             Gecode::SetExpr(0) || Gecode::SetExpr(1)
                             );
        Gecode::ProjectorSet ps(3);
        ps.add(p0); ps.add(p1); ps.add(p2);
        Gecode::projector(home, x[0], x[1], x[2], ps, true);
      }
    };

    NegRelUnionEq _negrelunioneq("Proj::RelOp::NegUnionEq");

}}}

// STATISTICS: test-set
