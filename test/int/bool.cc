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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "test/int.hh"
#include "gecode/minimodel.hh"

static IntSet ds(0,1);

class BoolEq : public IntTest {
public:
  BoolEq(const char* t)
    : IntTest(t,2,ds) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), IRT_EQ, link(home,x[1]));
  }
};
BoolEq _booleq("Bool::Eq");

class BoolNot : public IntTest {
public:
  BoolNot(const char* t)
    : IntTest(t,2,ds) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]!=x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), IRT_NQ, link(home,x[1]));
  }
};
BoolNot _boolnot("Bool::Not");

class BoolAnd : public IntTest {
public:
  BoolAnd(const char* t)
    : IntTest(t,3,ds) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]&x[1])==x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), BOT_AND, link(home,x[1]), link(home,x[2]));
  }
};
BoolAnd _booland("Bool::And::Binary");

class BoolOr : public IntTest {
public:
  BoolOr(const char* t)
    : IntTest(t,3,ds) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0]|x[1])==x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), BOT_OR, link(home,x[1]), link(home,x[2]));
  }
};
BoolOr _boolor("Bool::Or::Binary");

class BoolImp : public IntTest {
public:
  BoolImp(const char* t)
    : IntTest(t,3,ds) {}
  virtual bool solution(const Assignment& x) const {
    return ((x[0] == 0 ? 1 : 0)|x[1])==x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), BOT_IMP, link(home,x[1]), link(home,x[2]));
  }
};
BoolImp _boolimp("Bool::Imp");

class BoolEqv : public IntTest {
public:
  BoolEqv(const char* t)
    : IntTest(t,3,ds) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0] == x[1])==x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), BOT_EQV, link(home,x[1]), link(home,x[2]));
  }
};
BoolEqv _booleqv("Bool::Eqv");

class BoolXor : public IntTest {
public:
  BoolXor(const char* t)
    : IntTest(t,3,ds) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0] != x[1])==x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    rel(home, link(home,x[0]), BOT_XOR, link(home,x[1]), link(home,x[2]));
  }
};
BoolXor _boolxor("Bool::Xor");


class BoolAndNary : public IntTest {
public:
  BoolAndNary(const char* t)
    : IntTest(t,14,ds) {}
  virtual bool solution(const Assignment& x) const {
    for (int i = x.size()-1; i--; )
      if (x[i] == 0)
        return x[x.size()-1] == 0;
    return x[x.size()-1] == 1;
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(2*(x.size()-1));
    for (int i=x.size()-1; i--; ) {
      BoolVar bx = link(home,x[i]);
      b[2*i+0] = bx; b[2*i+1] = bx;
    }
    bool_and(home, b, link(home,x[x.size()-1]));
  }
};
BoolAndNary _boolandnary("Bool::And::Nary");

class BoolOrNary : public IntTest {
public:
  BoolOrNary(const char* t)
    : IntTest(t,14,ds) {}
  virtual bool solution(const Assignment& x) const {
    for (int i = x.size()-1; i--; )
      if (x[i] == 1)
        return x[x.size()-1] == 1;
    return x[x.size()-1] == 0;
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(2*(x.size()-1));
    for (int i=x.size()-1; i--; ) {
      BoolVar bx = link(home,x[i]);
      b[2*i+0] = bx; b[2*i+1] = bx;
    }
    bool_or(home, b, link(home,x[x.size()-1]));
  }
};
BoolOrNary _boolornary("Bool::Or::Nary");


// STATISTICS: test-int

