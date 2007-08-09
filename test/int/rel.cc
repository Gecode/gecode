/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"
#include "gecode/minimodel.hh"

static IntSet ds_22(-2,2);
static IntSet ds_01(0,1);

class RelEqBin : public IntTest {
private:
  IntConLevel icl;
public:
  RelEqBin(const char* t, IntConLevel icl0)
    : IntTest(t,2,ds_22,true), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_EQ, x[1], icl);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_EQ, x[1], b, icl);
  }
};
RelEqBin _releqbinbnd("Rel::EqBinBnd",ICL_BND);
RelEqBin _releqbindom("Rel::EqBinDom",ICL_DOM);

class RelEqBinInt : public IntTest {
private:
  IntConLevel icl;
public:
  RelEqBinInt(const char* t, IntConLevel icl0)
    : IntTest(t,1,ds_22,true), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]==0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_EQ, 0, icl);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_EQ, 0, b, icl);
  }
};
RelEqBinInt _releqbinbndint("Rel::EqBinBndInt",ICL_BND);
RelEqBinInt _releqbindomint("Rel::EqBinDomInt",ICL_DOM);


class RelEqNary : public IntTest {
private:
  IntConLevel icl;
public:
  RelEqNary(const char* t, IntConLevel icl0)
    : IntTest(t,4,ds_22,false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]==x[1]) && (x[1]==x[2]) && (x[2]==x[3]);
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x, IRT_EQ, icl);
  }
};
RelEqNary _releqnarybnd("Rel::EqNaryBnd", ICL_BND);
RelEqNary _releqnarydom("Rel::EqNaryDom", ICL_DOM);


class RelNq : public IntTest {
private:
  IntConLevel icl;
public:
  RelNq(const char* t, IntConLevel icl0)
    : IntTest(t,2,ds_22,true), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]!=x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_NQ, x[1], icl);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_NQ, x[1], b, icl);
  }
};
RelNq _relnqbnd("Rel::NqBnd",ICL_BND);
RelNq _relnqdom("Rel::NqDom",ICL_DOM);


class RelLq : public IntTest {
public:
  RelLq(void)
    : IntTest("Rel::Lq",2,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] <= x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_LQ, x[1]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_LQ, x[1], b);
  }
};
RelLq _rellq;

class RelLqInt : public IntTest {
public:
  RelLqInt(void)
    : IntTest("Rel::LqInt",1,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] <= 0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_LQ, 0);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_LQ, 0, b);
  }
};
RelLqInt _rellqint;


class RelLe : public IntTest {
public:
  RelLe(void)
    : IntTest("Rel::Le",2,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] < x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_LE, x[1]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_LE, x[1], b);
  }
};
RelLe _relle;

class RelLeInt : public IntTest {
public:
  RelLeInt(void)
    : IntTest("Rel::LeInt",1,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] < 0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_LE, 0);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_LE, 0, b);
  }
};
RelLeInt _relleint;

class RelGq : public IntTest {
public:
  RelGq(void)
    : IntTest("Rel::Gq",2,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] >= x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_GQ, x[1]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_GQ, x[1], b);
  }
};
RelGq _relgq;

class RelGqInt : public IntTest {
public:
  RelGqInt(void)
    : IntTest("Rel::GqInt",1,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] >= 0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_GQ, 0);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_GQ, 0, b);
  }
};
RelGqInt _relgqint;


class RelGr : public IntTest {
public:
  RelGr(void)
    : IntTest("Rel::Gr",2,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] > x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_GR, x[1]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_GR, x[1], b);
  }
};
RelGr _relgr;

class RelGrInt : public IntTest {
public:
  RelGrInt(void)
    : IntTest("Rel::GrInt",1,ds_22,true) {}
  virtual bool solution(const Assignment& x) const {
    return x[0] > 0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_GR, 0);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    rel(home, x[0], IRT_GR, 0, b);
  }
};
RelGrInt _relgrint;

class LexInt : public IntTest {
private:
  int  n;
  bool strict;
public:
  LexInt(const char* t, int m, bool _strict)
    : IntTest(t,m*2,ds_22), n(m), strict(_strict) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<n; i++)
      if (x[i]<x[n+i]) {
        return true;
      } else if (x[i]>x[n+i]) {
        return false;
      }
    return !strict;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs y(n); IntVarArgs z(n);
    for (int i=0; i<n; i++) {
      y[i]=x[i]; z[i]=x[n+i];
    }
    rel(home, y, strict ? IRT_LE : IRT_LQ, z);
  }
};
LexInt _lexlqi("Lex::Lq::Int",3,false);
LexInt _lexlei("Lex::Le::Int",3,true);

class LexBool : public IntTest {
private:
  int  n;
  bool strict;
public:
  LexBool(const char* t, int m, bool _strict)
    : IntTest(t,m*2,ds_01), n(m), strict(_strict) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<n; i++)
      if (x[i]<x[n+i]) {
        return true;
      } else if (x[i]>x[n+i]) {
        return false;
      }
    return !strict;
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs y(n); BoolVarArgs z(n);
    for (int i=0; i<n; i++) {
      y[i]=channel(home,x[i]);; z[i]=channel(home,x[n+i]);
    }
    rel(home, y, strict ? IRT_LE : IRT_LQ, z);
  }
};
LexBool _lexlq("Lex::Lq::Bool",3,false);
LexBool _lexle("Lex::Le::Bool",3,true);


class NaryNq : public IntTest {
private:
  int  n;
public:
  NaryNq(const char* t, int m)
    : IntTest(t,m*2,ds_22), n(m) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<n; i++)
      if (x[i] != x[n+i])
        return true;
    return false;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs y(n); IntVarArgs z(n);
    for (int i=0; i<n; i++) {
      y[i]=x[i]; z[i]=x[n+i];
    }
    rel(home, y, IRT_NQ, z);
  }
};
NaryNq _nnq("NaryNq",3);

// STATISTICS: test-int

