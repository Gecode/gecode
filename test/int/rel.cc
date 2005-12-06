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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/int.hh"

static IntSet ds_22(-2,2);

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
    eq(home, x, icl);
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

class Lex : public IntTest {
private:
  int  n;
  bool strict;
public:
  Lex(const char* t, int m, bool _strict) 
    : IntTest(t,m*2,ds_22,false,5), n(m), strict(_strict) {}
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
    lex(home, y, strict ? IRT_LE : IRT_LQ, z);
  }
};
Lex _lexlq("Lex::Lq",3,false);
Lex _lexle("Lex::Le",3,true);


// STATISTICS: test-int

