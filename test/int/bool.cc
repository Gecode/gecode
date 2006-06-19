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

static IntSet ds(0,1);

class BoolEq : public IntTest {
public:
  BoolEq(const char* t) 
    : IntTest(t,2,ds) {}
  virtual bool solution(const Assignment& x) const {
    return x[0]==x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]);
    bool_eq(home, b0, b1);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]);
    bool_not(home, b0, b1);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]); BoolVar b2(x[2]);
    bool_and(home, b0, b1, b2);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]); BoolVar b2(x[2]);
    bool_or(home, b0, b1, b2);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]); BoolVar b2(x[2]);
    bool_imp(home, b0, b1, b2);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]); BoolVar b2(x[2]);
    bool_eqv(home, b0, b1, b2);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVar b0(x[0]); BoolVar b1(x[1]); BoolVar b2(x[2]);
    bool_xor(home, b0, b1, b2);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVarArgs b(2*(x.size()-1));
    for (int i=x.size()-1; i--; ) {
      BoolVar bx(x[i]); 
      b[2*i+0] = bx; b[2*i+1] = bx;
    }
    BoolVar bx(x[x.size()-1]);
    bool_and(home, b, bx);
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
    for (int i=x.size(); i--; )
      Gecode::dom(home, x[i], 0, 1);
    BoolVarArgs b(2*(x.size()-1));
    for (int i=x.size()-1; i--; ) {
      BoolVar bx(x[i]); 
      b[2*i+0] = bx; b[2*i+1] = bx;
    }
    BoolVar bx(x[x.size()-1]);
    bool_or(home, b, bx);
  }
};
BoolOrNary _boolornary("Bool::Or::Nary");


// STATISTICS: test-int

