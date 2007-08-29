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
#include "test/log.hh"

using namespace Gecode;

static Gecode::IntSet ds_22(-2,2);

static const int ints[4] = {1,0,3,2};

class CountIntInt : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntInt(const char* t, IntRelType irt0)
    : IntTest(t,4,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == 0)
        m++;
    return cmp(m,irt,2);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    count(home, x, 0, irt, 2);
  }
};

CountIntInt _ceqii("Count::Eq::IntInt",Gecode::IRT_EQ);
CountIntInt _cnqii("Count::Nq::IntInt",Gecode::IRT_NQ);
CountIntInt _clqii("Count::Lq::IntInt",Gecode::IRT_LQ);
CountIntInt _cleii("Count::Le::IntInt",Gecode::IRT_LE);
CountIntInt _cgrii("Count::Gr::IntInt",Gecode::IRT_GR);
CountIntInt _cgqii("Count::Gq::IntInt",Gecode::IRT_GQ);

class CountIntsInt : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntsInt(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == ints[i])
        m++;
    return cmp(m,irt,2);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(4); IntArgs a(4);
    for (int i=0; i<4; i++) {
      y[i]=x[i]; a[i]=ints[i];
    }
    count(home, y, a, irt, 2);
  }
};

CountIntsInt _ceqiis("Count::Eq::IntsInt",Gecode::IRT_EQ);
CountIntsInt _cnqiis("Count::Nq::IntsInt",Gecode::IRT_NQ);
CountIntsInt _clqiis("Count::Lq::IntsInt",Gecode::IRT_LQ);
CountIntsInt _cleiis("Count::Le::IntsInt",Gecode::IRT_LE);
CountIntsInt _cgriis("Count::Gr::IntsInt",Gecode::IRT_GR);
CountIntsInt _cgqiis("Count::Gq::IntsInt",Gecode::IRT_GQ);


class CountIntIntDup : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntIntDup(const char* t, IntRelType irt0)
    : IntTest(t,4,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == 0)
        m += 2;
    return cmp(m,irt,4);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(8);
    for (int i=0; i<4; i++) {
      y[i]=x[i]; y[4+i]=x[i];
    }
    count(home, y, 0, irt, 4);
  }
};

CountIntIntDup _ceqiid("Count::Eq::IntInt::Dup",Gecode::IRT_EQ);
CountIntIntDup _cnqiid("Count::Nq::IntInt::Dup",Gecode::IRT_NQ);
CountIntIntDup _clqiid("Count::Lq::IntInt::Dup",Gecode::IRT_LQ);
CountIntIntDup _cleiid("Count::Le::IntInt::Dup",Gecode::IRT_LE);
CountIntIntDup _cgriid("Count::Gr::IntInt::Dup",Gecode::IRT_GR);
CountIntIntDup _cgqiid("Count::Gq::IntInt::Dup",Gecode::IRT_GQ);

class CountIntVar : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntVar(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == 0)
        m++;
    return cmp(m,irt,x[4]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(4);
    for (int i=0; i<4; i++)
      y[i]=x[i];
    count(home, y, 0, irt, x[4]);
  }
};

CountIntVar _ceqiv("Count::Eq::IntVar",Gecode::IRT_EQ);
CountIntVar _cnqiv("Count::Nq::IntVar",Gecode::IRT_NQ);
CountIntVar _clqiv("Count::Lq::IntVar",Gecode::IRT_LQ);
CountIntVar _cleiv("Count::Le::IntVar",Gecode::IRT_LE);
CountIntVar _cgriv("Count::Gr::IntVar",Gecode::IRT_GR);
CountIntVar _cgqiv("Count::Gq::IntVar",Gecode::IRT_GQ);


class CountIntsVar : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntsVar(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == ints[i])
        m++;
    return cmp(m,irt,x[4]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(4); IntArgs a(4);
    for (int i=0; i<4; i++) {
      y[i]=x[i]; a[i]=ints[i];
    }
    count(home, y, a, irt, x[4]);
  }
};

CountIntsVar _ceqisv("Count::Eq::IntsVar",Gecode::IRT_EQ);
CountIntsVar _cnqisv("Count::Nq::IntsVar",Gecode::IRT_NQ);
CountIntsVar _clqisv("Count::Lq::IntsVar",Gecode::IRT_LQ);
CountIntsVar _cleisv("Count::Le::IntsVar",Gecode::IRT_LE);
CountIntsVar _cgrisv("Count::Gr::IntsVar",Gecode::IRT_GR);
CountIntsVar _cgqisv("Count::Gq::IntsVar",Gecode::IRT_GQ);


class CountIntVarShared : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountIntVarShared(const char* t, IntRelType irt0)
    : IntTest(t,4,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == 0)
        m++;
    return cmp(m,irt,x[2]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    count(home, x, 0, irt, x[2]);
  }
};

CountIntVarShared _ceqivs("Count::Eq::IntVarShared",Gecode::IRT_EQ);
CountIntVarShared _cnqivs("Count::Nq::IntVarShared",Gecode::IRT_NQ);
CountIntVarShared _clqivs("Count::Lq::IntVarShared",Gecode::IRT_LQ);
CountIntVarShared _cleivs("Count::Le::IntVarShared",Gecode::IRT_LE);
CountIntVarShared _cgrivs("Count::Gr::IntVarShared",Gecode::IRT_GR);
CountIntVarShared _cgqivs("Count::Gq::IntVarShared",Gecode::IRT_GQ);

class CountVarVar : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountVarVar(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<3; i++)
      if (x[i] == x[3])
        m++;
    return cmp(m,irt,x[4]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(3);
    for (int i=0; i<3; i++)
      y[i]=x[i];
    count(home, y, x[3], irt, x[4]);
  }
};

CountVarVar _ceqvv("Count::Eq::VarVar",Gecode::IRT_EQ);
CountVarVar _cnqvv("Count::Nq::VarVar",Gecode::IRT_NQ);
CountVarVar _clqvv("Count::Lq::VarVar",Gecode::IRT_LQ);
CountVarVar _clevv("Count::Le::VarVar",Gecode::IRT_LE);
CountVarVar _cgrvv("Count::Gr::VarVar",Gecode::IRT_GR);
CountVarVar _cgqvv("Count::Gq::VarVar",Gecode::IRT_GQ);

class CountVarVarSharedA : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountVarVarSharedA(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == x[1])
        m++;
    return cmp(m,irt,x[4]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(4);
    for (int i=0; i<4; i++)
      y[i]=x[i];
    count(home, y, x[1], irt, x[4]);
  }
};

CountVarVarSharedA _ceqvvsa("Count::Eq::VarVarShared::A",Gecode::IRT_EQ);
CountVarVarSharedA _cnqvvsa("Count::Nq::VarVarShared::A",Gecode::IRT_NQ);
CountVarVarSharedA _clqvvsa("Count::Lq::VarVarShared::A",Gecode::IRT_LQ);
CountVarVarSharedA _clevvsa("Count::Le::VarVarShared::A",Gecode::IRT_LE);
CountVarVarSharedA _cgrvvsa("Count::Gr::VarVarShared::A",Gecode::IRT_GR);
CountVarVarSharedA _cgqvvsa("Count::Gq::VarVarShared::A",Gecode::IRT_GQ);

class CountVarVarSharedB : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountVarVarSharedB(const char* t, IntRelType irt0)
    : IntTest(t,5,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == x[4])
        m++;
    return cmp(m,irt,x[3]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs y(4);
    for (int i=0; i<4; i++)
      y[i]=x[i];
    count(home, y, x[4], irt, x[3]);
  }
};

CountVarVarSharedB _ceqvvsb("Count::Eq::VarVarShared::B",Gecode::IRT_EQ);
CountVarVarSharedB _cnqvvsb("Count::Nq::VarVarShared::B",Gecode::IRT_NQ);
CountVarVarSharedB _clqvvsb("Count::Lq::VarVarShared::B",Gecode::IRT_LQ);
CountVarVarSharedB _clevvsb("Count::Le::VarVarShared::B",Gecode::IRT_LE);
CountVarVarSharedB _cgrvvsb("Count::Gr::VarVarShared::B",Gecode::IRT_GR);
CountVarVarSharedB _cgqvvsb("Count::Gq::VarVarShared::B",Gecode::IRT_GQ);

class CountVarVarSharedC : public IntTest {
private:
  IntRelType irt;
public:
  /// Create and register test
  CountVarVarSharedC(const char* t, IntRelType irt0)
    : IntTest(t,4,ds_22), irt(irt0) {}
  /// Test whether \a x is solution
  virtual bool solution(const Assignment& x) const {
    int m = 0;
    for (int i=0; i<4; i++)
      if (x[i] == x[1])
        m++;
    return cmp(m,irt,x[3]);
  }
  /// Post constraint on \a x
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    count(home, x, x[1], irt, x[3]);
  }
};

CountVarVarSharedC _ceqvvsc("Count::Eq::VarVarShared::C",Gecode::IRT_EQ);
CountVarVarSharedC _cnqvvsc("Count::Nq::VarVarShared::C",Gecode::IRT_NQ);
CountVarVarSharedC _clqvvsc("Count::Lq::VarVarShared::C",Gecode::IRT_LQ);
CountVarVarSharedC _clevvsc("Count::Le::VarVarShared::C",Gecode::IRT_LE);
CountVarVarSharedC _cgrvvsc("Count::Gr::VarVarShared::C",Gecode::IRT_GR);
CountVarVarSharedC _cgqvvsc("Count::Gq::VarVarShared::C",Gecode::IRT_GQ);


// STATISTICS: test-int

