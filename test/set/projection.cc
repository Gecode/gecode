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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/set.hh"
#include "gecode/set/projectors.hh"
#include "gecode/minimodel.hh"

static IntSet ds_22(-2,2);
static IntSet ds_33(-3,3);

namespace Projection {

  class RelBinNEq : public SetTest {
  public:
    RelBinNEq(const char* t)
      : SetTest(t,2,ds_33) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      return !Iter::Ranges::equal(xr0, xr1);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
      Gecode::Projector p(0, Gecode::SetExpr(1), Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(0), Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], ps, true);
    }
  };
  RelBinNEq _relneq("Proj::Rel::BinNEq");

  class RelBinEq : public SetTest {
  public:
    RelBinEq(const char* t)
      : SetTest(t,2,ds_33,true) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      return Iter::Ranges::equal(xr0, xr1);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
      Gecode::SetExpr xe(0);
      Gecode::SetExpr ye(1);
      Gecode::Projector p(0, ye, ye);
      Gecode::Projector q(1, xe, xe);
      Gecode::ProjectorSet ps;
      ps.add(p);
      ps.add(q);
      Gecode::projector(home, x[0], x[1], ps);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
      Gecode::Projector p(0, Gecode::SetExpr(1), Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(0), Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], b, ps);
    }
  };
  RelBinEq _releq("Proj::Rel::BinEq");

  class RelBinSub : public SetTest {
  public:
    RelBinSub(const char* t)
      : SetTest(t,2,ds_33,true) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      return Iter::Ranges::subset(xr0, xr1);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
      Gecode::Projector p(0, Gecode::SetExpr(), Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(0), -Gecode::SetExpr());
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], ps);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
      Gecode::Projector p(0, Gecode::SetExpr(), Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(0), -Gecode::SetExpr());
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], b, ps);
    }
  };
  RelBinSub _relsub("Proj::Rel::BinSub");

  class RelBinDisj : public SetTest {
  public:
    RelBinDisj(const char* t)
      : SetTest(t,2,ds_33,true) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      return Iter::Ranges::disjoint(xr0, xr1);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
      Gecode::Projector p(0, Gecode::SetExpr(), -Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(), -Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], ps);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
      Gecode::Projector p(0, Gecode::SetExpr(), -Gecode::SetExpr(1));
      Gecode::Projector q(1, Gecode::SetExpr(), -Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], b, ps);
    }
  };
  RelBinDisj _reldisj("Proj::Rel::BinDisj");

  class RelBinCompl : public SetTest {
  public:
    RelBinCompl(const char* t)
    : SetTest(t,2,ds_33,true) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      Set::RangesCompl<CountableSetRanges> xr1c(xr1);
      return Iter::Ranges::equal(xr0, xr1c);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
      Gecode::Projector p(0, -Gecode::SetExpr(1), -Gecode::SetExpr(1));
      Gecode::Projector q(1, -Gecode::SetExpr(0), -Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], ps);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
      Gecode::Projector p(0, -Gecode::SetExpr(1), -Gecode::SetExpr(1));
      Gecode::Projector q(1, -Gecode::SetExpr(0), -Gecode::SetExpr(0));
      Gecode::ProjectorSet ps;
      ps.add(p); ps.add(q);
      Gecode::projector(home, x[0], x[1], b, ps);
    }
  };
  RelBinCompl _relcompl("Proj::Rel::BinCompl");

  class RelUnionEq : public SetTest {
  public:
    RelUnionEq(const char* t)
      : SetTest(t,3,ds_22,true) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> u(xr0,xr1);
      return Iter::Ranges::equal(xr2, u);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
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
      Gecode::ProjectorSet ps;
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
      Gecode::post(home, x2c == x0c + x1c - x0ix1c);

      Gecode::Projector p4(2,
                           Gecode::SetExpr(0) - Gecode::SetExpr(1),
                           Gecode::SetExpr(0) - Gecode::SetExpr(1)
                           );
      IntVar x0mx1c(home, 0, 20);
      Gecode::projector(home, x[0], x[1], x[2], x0mx1c, p4);
      Gecode::post(home, x1c == x2c - x0mx1c);

      Gecode::Projector p5(2,
                           Gecode::SetExpr(1) - Gecode::SetExpr(0),
                           Gecode::SetExpr(1) - Gecode::SetExpr(0)
                           );
      IntVar x1mx0c(home, 0, 20);
      Gecode::projector(home, x[0], x[1], x[2], x1mx0c, p5);
      Gecode::post(home, x0c == x2c - x1mx0c);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&, BoolVar b) {
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
      Gecode::ProjectorSet ps;
      ps.add(p0); ps.add(p1); ps.add(p2);
      Gecode::projector(home, x[0], x[1], x[2], b, ps);
    }
  };
  RelUnionEq _relunioneq("Proj::RelOp::UnionEq");

  class RelInterEqCard : public SetTest {
  public:
    RelInterEqCard(const char* t)
      : SetTest(t,3,ds_22,false) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> u(xr0,xr1);
      return Iter::Ranges::equal(xr2, u);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
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
      Gecode::ProjectorSet ps;
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
      Gecode::post(home, x2c == x0c + x1c - x0ux1c);

      Gecode::Projector p4(2,
                           Gecode::SetExpr(0) - Gecode::SetExpr(1),
                           Gecode::SetExpr(0) - Gecode::SetExpr(1)
                           );
      IntVar x0mx1c(home, 0, 20);
      Gecode::projector(home, x[0], x[1], x[2], x0mx1c, p4);
      Gecode::post(home, x2c == x0c - x0mx1c);

      Gecode::Projector p5(2,
                           Gecode::SetExpr(1) - Gecode::SetExpr(0),
                           Gecode::SetExpr(1) - Gecode::SetExpr(0)
                           );
      IntVar x1mx0c(home, 0, 20);
      Gecode::projector(home, x[0], x[1], x[2], x1mx0c, p5);
      Gecode::post(home, x2c == x1c - x1mx0c);

    }
  };
  RelInterEqCard _relintereqcard("Proj::RelOp::InterEqCard");

  class NegRelUnionEq : public SetTest {
  public:
    NegRelUnionEq(const char* t)
      : SetTest(t,3,ds_22) {}
    virtual bool solution(const SetAssignment& x) const {
      CountableSetRanges xr0(x.lub, x[0]);
      CountableSetRanges xr1(x.lub, x[1]);
      CountableSetRanges xr2(x.lub, x[2]);
      Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> u(xr0,xr1);
      return !Iter::Ranges::equal(xr2, u);
    }
    virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
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
      Gecode::ProjectorSet ps;
      ps.add(p0); ps.add(p1); ps.add(p2);
      Gecode::projector(home, x[0], x[1], x[2], ps, true);
    }
  };
  NegRelUnionEq _negrelunioneq("Proj::RelOp::NegUnionEq");

}

// STATISTICS: test-set
