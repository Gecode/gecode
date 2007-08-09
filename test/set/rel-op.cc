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

static IntSet ds_33(-2,2);
static IntSet ds_22(-1,2);

class RelUnionEq : public SetTest {
public:
  RelUnionEq(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> u(xr0,xr1);
    return Iter::Ranges::equal(xr2, u);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_EQ, x[2]);
  }
};
RelUnionEq _relunioneq("RelOp::UnionEq");

class RelUnionEqS1 : public SetTest {
public:
  RelUnionEqS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::equal(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_EQ, x[1]);
  }
};
RelUnionEqS1 _relunioneqs1("RelOp::Sharing::UnionEqS1");

class RelUnionEqS2 : public SetTest {
public:
  RelUnionEqS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr1, xr0);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_EQ, x[0]);
  }
};
RelUnionEqS2 _relunioneqs2("RelOp::Sharing::UnionEqS2");

class RelUnionEqS3 : public SetTest {
public:
  RelUnionEqS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_EQ, x[0]);
  }
};
RelUnionEqS3 _relunioneqs3("RelOp::Sharing::UnionEqS3");

class RelDUnionEq : public SetTest {
public:
  RelDUnionEq(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

    CountableSetRanges xr02(x.lub, x[0]);
    CountableSetRanges xr12(x.lub, x[1]);

    return
      Iter::Ranges::equal(xr2, u) &&
      Iter::Ranges::disjoint(xr02, xr12);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_DUNION, x[1], SRT_EQ, x[2]);
  }
};
RelDUnionEq _reldunioneq("RelOp::DUnionEq");

class RelDUnionEqS1 : public SetTest {
public:
  RelDUnionEqS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return !xr0() && !xr1();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_DUNION, x[0], SRT_EQ, x[1]);
  }
};
RelDUnionEqS1 _reldunioneqs1("RelOp::Sharing::DUnionEqS1");

class RelDUnionEqS2 : public SetTest {
public:
  RelDUnionEqS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    return !xr0();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_DUNION, x[1], SRT_EQ, x[1]);
  }
};
RelDUnionEqS2 _reldunioneqs2("RelOp::Sharing::DUnionEqS2");

class RelDUnionEqS3 : public SetTest {
public:
  RelDUnionEqS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    return !xr0();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_DUNION, x[0], SRT_EQ, x[0]);
  }
};
RelDUnionEqS3 _reldunioneqs3("RelOp::Sharing::DUnionEqS3");

class RelInterEq : public SetTest {
public:
  RelInterEq(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

    return Iter::Ranges::equal(xr2, u);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_EQ, x[2]);
  }
};
RelInterEq _relintereq("RelOp::InterEq");

class RelInterEqS1 : public SetTest {
public:
  RelInterEqS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_EQ, x[0]);
  }
};
RelInterEqS1 _relintereqs1("RelOp::Sharing::InterEqS1");

class RelInterEqS2 : public SetTest {
public:
  RelInterEqS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::equal(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_EQ, x[1]);
  }
};
RelInterEqS2 _relintereqs2("RelOp::Sharing::InterEqS2");

class RelInterEqS3 : public SetTest {
public:
  RelInterEqS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_EQ, x[0]);
  }
};
RelInterEqS3 _relintereqs3("RelOp::Sharing::InterEqS3");

class RelInterSub : public SetTest {
public:
  RelInterSub(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Inter<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

    return Iter::Ranges::subset(u, xr2);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_SUB, x[2]);
  }
};
RelInterSub _relintersub("RelOp::InterSub");

class RelInterSubS1 : public SetTest {
public:
  RelInterSubS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[1], SRT_SUB, x[1]);
  }
};
RelInterSubS1 _relintersub1("RelOp::Sharing::InterSub1");

class RelInterSubS2 : public SetTest {
public:
  RelInterSubS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr0, xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_SUB, x[1]);
  }
};
RelInterSubS2 _relintersub2("RelOp::Sharing::InterSub2");

class RelInterSubS3 : public SetTest {
public:
  RelInterSubS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_INTER, x[0], SRT_SUB, x[0]);
  }
};
RelInterSubS3 _relintersub3("RelOp::Sharing::InterSub3");

class RelMinusEq : public SetTest {
public:
  RelMinusEq(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

    return Iter::Ranges::equal(xr2, u);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_MINUS, x[1], SRT_EQ, x[2]);
  }
};
RelMinusEq _relminuseq("RelOp::MinusEq");

class RelMinusEqS1 : public SetTest {
public:
  RelMinusEqS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr12(x.lub, x[1]);
    Iter::Ranges::Diff<CountableSetRanges, CountableSetRanges> u(xr0,xr1);

    return Iter::Ranges::equal(xr12, u);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_MINUS, x[1], SRT_EQ, x[1]);
  }
};
RelMinusEqS1 _relminuseqs1("RelOp::Sharing::MinusEqS1");

class RelMinusEqS2 : public SetTest {
public:
  RelMinusEqS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr1(x.lub, x[1]);
    return !xr1();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_MINUS, x[0], SRT_EQ, x[1]);
  }
};
RelMinusEqS2 _relminuseqs2("RelOp::Sharing::MinusEqS2");

class RelMinusEqS3 : public SetTest {
public:
  RelMinusEqS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    return !xr0();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_MINUS, x[0], SRT_EQ, x[0]);
  }
};
RelMinusEqS3 _relminuseqs3("RelOp::Sharing::MinusEqS3");

class RelUnionSup : public SetTest {
public:
  RelUnionSup(const char* t)
    : SetTest(t,3,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    Iter::Ranges::Union<CountableSetRanges, CountableSetRanges> u(xr0,xr1);
    return Iter::Ranges::subset(xr2, u);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_SUP, x[2]);
  }
};
RelUnionSup _relunionsup("RelOp::UnionSup");

class RelUnionSupS1 : public SetTest {
public:
  RelUnionSupS1(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[1], SRT_SUP, x[1]);
  }
};
RelUnionSupS1 _relunionsups1("RelOp::Sharing::UnionSupS1");

class RelUnionSupS2 : public SetTest {
public:
  RelUnionSupS2(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr1, xr0);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_SUP, x[1]);
  }
};
RelUnionSupS2 _relunionsups2("RelOp::Sharing::UnionSupS2");

class RelUnionSupS3 : public SetTest {
public:
  RelUnionSupS3(const char* t)
    : SetTest(t,1,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    return true;
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    Gecode::rel(home, x[0], SOT_UNION, x[0], SRT_SUP, x[0]);
  }
};
RelUnionSupS3 _relunionsups3("RelOp::Sharing::UnionSupS3");

class RelDUnionN : public SetTest {
public:
  RelDUnionN(const char* t)
    : SetTest(t,4,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    {
      CountableSetRanges x0r(x.lub, x[0]);
      CountableSetRanges x1r(x.lub, x[1]);
      if (!Iter::Ranges::disjoint(x0r,x1r))
        return false;
    }
    {
      CountableSetRanges x0r(x.lub, x[0]);
      CountableSetRanges x2r(x.lub, x[2]);
      if (!Iter::Ranges::disjoint(x0r,x2r))
        return false;
    }
    {
      CountableSetRanges x1r(x.lub, x[1]);
      CountableSetRanges x2r(x.lub, x[2]);
      if (!Iter::Ranges::disjoint(x1r,x2r))
        return false;
    }

    GECODE_AUTOARRAY(CountableSetRanges, isrs, 3);
    isrs[0].init(x.lub, x[0]);
    isrs[1].init(x.lub, x[1]);
    isrs[2].init(x.lub, x[2]);
    Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, 3);
    CountableSetRanges x3r(x.lub, x[3]);
    return Iter::Ranges::equal(u, x3r);

  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::rel(home, SOT_DUNION, xs, x[x.size()-1]);
  }
};
RelDUnionN _reldunionn("RelOp::DUnionN");

class RelDUnionNS1 : public SetTest {
public:
  RelDUnionNS1(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr2(x.lub, x[2]);
    return !xr0() && Iter::Ranges::equal(xr1,xr2);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[0];
    xs[2]=x[1];
    Gecode::rel(home, SOT_DUNION, xs, x[2]);
  }
};
RelDUnionNS1 _reldunionns1("RelOp::Sharing::DUnionNS1");

class RelDUnionNS2 : public SetTest {
public:
  RelDUnionNS2(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return !xr0() && !xr1();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[2];
    Gecode::rel(home, SOT_DUNION, xs, x[2]);
  }
};
RelDUnionNS2 _reldunionns2("RelOp::Sharing::DUnionNS2");

class RelDUnionNS3 : public SetTest {
public:
  RelDUnionNS3(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return !xr0() && !xr1();
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[0];
    Gecode::rel(home, SOT_DUNION, xs, x[0]);
  }
};
RelDUnionNS3 _reldunionns3("RelOp::Sharing::DUnionNS3");

class RelUnionN : public SetTest {
public:
  RelUnionN(const char* t)
    : SetTest(t,4,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    GECODE_AUTOARRAY(CountableSetRanges, isrs, 3);
    isrs[0].init(x.lub, x[0]);
    isrs[1].init(x.lub, x[1]);
    isrs[2].init(x.lub, x[2]);
    Iter::Ranges::NaryUnion<CountableSetRanges> u(isrs, 3);
    CountableSetRanges x3r(x.lub, x[3]);
    return Iter::Ranges::equal(u, x3r);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::rel(home, SOT_UNION, xs, x[x.size()-1]);
  }
};
RelUnionN _relunionn("RelOp::UnionN");

class RelUnionNS1 : public SetTest {
public:
  RelUnionNS1(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    Iter::Ranges::Union<CountableSetRanges,CountableSetRanges> u(xr0,xr1);
    CountableSetRanges xr2(x.lub, x[2]);
    return Iter::Ranges::equal(u,xr2);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[0];
    xs[2]=x[1];
    Gecode::rel(home, SOT_UNION, xs, x[2]);
  }
};
RelUnionNS1 _relunionns1("RelOp::Sharing::UnionNS1");

class RelUnionNS2 : public SetTest {
public:
  RelUnionNS2(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr21(x.lub, x[2]);
    CountableSetRanges xr22(x.lub, x[2]);
    return Iter::Ranges::subset(xr0,xr21) && Iter::Ranges::subset(xr1,xr22);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[2];
    Gecode::rel(home, SOT_UNION, xs, x[2]);
  }
};
RelUnionNS2 _relunionns2("RelOp::Sharing::UnionNS2");

class RelUnionNS3 : public SetTest {
public:
  RelUnionNS3(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr1,xr0);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[0];
    Gecode::rel(home, SOT_UNION, xs, x[0]);
  }
};
RelUnionNS3 _relunionns3("RelOp::Sharing::UnionNS3");

class RelInterN : public SetTest {
public:
  RelInterN(const char* t)
    : SetTest(t,4,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    GECODE_AUTOARRAY(CountableSetRanges, isrs, 3);
    isrs[0].init(x.lub, x[0]);
    isrs[1].init(x.lub, x[1]);
    isrs[2].init(x.lub, x[2]);
    Iter::Ranges::NaryInter<CountableSetRanges> u(isrs, 3);
    CountableSetRanges x3r(x.lub, x[3]);
    return Iter::Ranges::equal(u, x3r);

  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(x.size()-1);
    for (int i=x.size()-1; i--;)
      xs[i]=x[i];
    Gecode::rel(home, SOT_INTER, xs, x[x.size()-1]);
  }
};
RelInterN _relintern("RelOp::InterN");

class RelInterNS1 : public SetTest {
public:
  RelInterNS1(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    Iter::Ranges::Inter<CountableSetRanges,CountableSetRanges> u(xr0,xr1);
    CountableSetRanges xr2(x.lub, x[2]);
    return Iter::Ranges::equal(u,xr2);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[0];
    xs[2]=x[1];
    Gecode::rel(home, SOT_INTER, xs, x[2]);
  }
};
RelInterNS1 _relinterns1("RelOp::Sharing::InterNS1");

class RelInterNS2 : public SetTest {
public:
  RelInterNS2(const char* t)
    : SetTest(t,3,ds_22,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    CountableSetRanges xr21(x.lub, x[2]);
    CountableSetRanges xr22(x.lub, x[2]);
    return Iter::Ranges::subset(xr21,xr0) && Iter::Ranges::subset(xr22,xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[2];
    Gecode::rel(home, SOT_INTER, xs, x[2]);
  }
};
RelInterNS2 _relinterns2("RelOp::Sharing::InterNS2");

class RelInterNS3 : public SetTest {
public:
  RelInterNS3(const char* t)
    : SetTest(t,2,ds_33,false) {}
  virtual bool solution(const SetAssignment& x) const {
    CountableSetRanges xr0(x.lub, x[0]);
    CountableSetRanges xr1(x.lub, x[1]);
    return Iter::Ranges::subset(xr0,xr1);
  }
  virtual void post(Space* home, SetVarArray& x, IntVarArray&) {
    SetVarArgs xs(3);
    xs[0]=x[0];
    xs[1]=x[1];
    xs[2]=x[0];
    Gecode::rel(home, SOT_INTER, xs, x[0]);
  }
};
RelInterNS3 _relinterns3("RelOp::Sharing::InterNS3");

// STATISTICS: test-set
