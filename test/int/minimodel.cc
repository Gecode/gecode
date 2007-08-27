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

using namespace Gecode;

static Gecode::IntSet is(-2,2);

class MmLinEqA : public IntTest {
public:
  MmLinEqA(void)
    : IntTest("MiniModel::Lin::Eq::A",2,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return (2*x[0]+1) == (x[1]-1);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, 2*x[0]+1 == x[1]-1);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(2*x[0]+1 == x[1]-1,b)));
  }
};
MmLinEqA _mmlineqa;

class MmLinEqB : public IntTest {
public:
  MmLinEqB(void)
    : IntTest("MiniModel::Lin::Eq::B",2,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return (2*x[0]+1) == (x[1]-1);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, 2*x[0]+1-x[1] == -1);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(2*x[0]+1-x[1] == -1,b)));
  }
};
MmLinEqB _mmlineqb;

class MmLinEqC : public IntTest {
public:
  MmLinEqC(void)
    : IntTest("MiniModel::Lin::Eq::C",2,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return -(1-(1-x[1])) == x[0];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, -(1-(1-x[1])) == x[0]);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(-(1-(1-x[1])) == x[0],b)));
  }
};
MmLinEqC _mmlineqc;

class MmLinEqD : public IntTest {
public:
  MmLinEqD(void)
    : IntTest("MiniModel::Lin::Eq::D",2,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return 2*(1-x[1]) == x[0];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, 2*(1-x[1]) == x[0]);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(2*(1-x[1]) == x[0],b)));
  }
};
MmLinEqD _mmlineqd;

class MmLinEqE : public IntTest {
public:
  MmLinEqE(void)
    : IntTest("MiniModel::Lin::Eq::E",1,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return 1>x[0];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, 1>x[0]);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(1>x[0],b)));
  }
};
MmLinEqE _mmlineqe;

class MmLinEqF : public IntTest {
public:
  MmLinEqF(void)
    : IntTest("MiniModel::Lin::Eq::F",1,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return 1<x[0];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, 1<x[0]);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(1<x[0],b)));
  }
};
MmLinEqF _mmlineqf;

class MmLinEqG : public IntTest {
public:
  MmLinEqG(void)
    : IntTest("MiniModel::Lin::Eq::G",1,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return -1==x[0];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::post(home, -1==x[0]);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x, Gecode::BoolVar b) {
    Gecode::post(home, tt(eqv(-1==x[0],b)));
  }
};
MmLinEqG _mmlineqg;

class MmLinEqBool : public IntTest {
public:
  MmLinEqBool(void)
    : IntTest("MiniModel::Lin::Eq::Bool",3,is,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=x.size(); i--; )
      if ((x[i] < 0) || (x[i] > 1))
        return false;
    return (2*x[0]+1) == (x[1]+x[2]-1);
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArray y(home,x.size());
    for (int i=x.size(); i--; )
      y[i] = channel(home,x[i]);
    Gecode::post(home, 2*y[0]+1 == y[1]+y[2]-1);
  }
};
MmLinEqBool _mmlineqbool;

class MmLinExprInt : public IntTest {
public:
  MmLinExprInt(void)
    : IntTest("MiniModel::Lin::Expr::Int",4,is,false) {}
  virtual bool solution(const Assignment& x) const {
    return 2*x[0]+3*x[1]-x[2] == x[3];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = Gecode::post(home, 2*x[0] + 3*x[1] - x[2]);
    Gecode::rel(home,y,Gecode::IRT_EQ,x[3]);
  }
};
MmLinExprInt _mmlinexprint;

class MmLinExprBool : public IntTest {
public:
  MmLinExprBool(void)
    : IntTest("MiniModel::Lin::Expr::Bool",4,is,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=x.size()-1; i--; )
      if ((x[i] < 0) || (x[i] > 1))
        return false;
    return -2*x[0]+3*x[1]-x[2] == x[3];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArray y(home,x.size()-1);
    for (int i=x.size()-1; i--; )
      y[i] = channel(home,x[i]);
    IntVar z = Gecode::post(home, -2*y[0] + 3*y[1] - y[2]);
    Gecode::rel(home,z,Gecode::IRT_EQ,x[3]);
  }
};
MmLinExprBool _mmlinexprbool;

static Gecode::IntSet b(0,1);

class MmBoolA : public IntTest {
public:
  MmBoolA(void)
    : IntTest("MiniModel::Bool::A",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return ((x[0]==1) && (x[1]==1)) || ((x[2]==1)!=(x[3]==1));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, tt((b[0] && b[1]) || !eqv(b[2],b[3])));
  }
};
MmBoolA _mmboola;

class MmBoolB : public IntTest {
public:
  MmBoolB(void)
    : IntTest("MiniModel::Bool::B",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return ((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, tt((b[0] && b[1]) && (b[2] && b[3])));
  }
};
MmBoolB _mmboolb;

class MmBoolC : public IntTest {
public:
  MmBoolC(void)
    : IntTest("MiniModel::Bool::C",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return ((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, tt(!!(b[0] && b[1]) && !(!b[2] || !b[3])));
  }
};
MmBoolC _mmboolc;

class MmBoolD : public IntTest {
public:
  MmBoolD(void)
    : IntTest("MiniModel::Bool::D",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return (((x[0]&x[1])==x[2]) && (x[0]==x[3]));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, tt(eqv(b[0] && b[1], b[2]) && eqv(b[0],b[3])));
  }
};
MmBoolD _mmboold;

class MmBoolE : public IntTest {
public:
  MmBoolE(void)
    : IntTest("MiniModel::Bool::E",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return !(((x[0]==1) && (x[1]==1)) && ((x[2]==1) && (x[3]==1)));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, ff(b[0] && b[1] && b[2] && b[3]));
  }
};
MmBoolE _mmboole;

class MmBoolF : public IntTest {
public:
  MmBoolF(void)
    : IntTest("MiniModel::Bool::F",4,b,false) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      if ((x[i] <0) || (x[i]>1))
        return false;
    return ((x[0]==1) || (x[1]==1)) || ((x[2]==1) || (x[3]==1));
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    Gecode::BoolVarArgs b(4);
    for (int i=x.size(); i--; )
      b[i]=channel(home,x[i]);
    Gecode::post(home, tt(b[0] || b[1] || b[2] || b[3]));
  }
};
MmBoolF _mmboolf;


namespace {
  const int s1r[7] = {
    Limits::Int::int_min, Limits::Int::int_min+1,
    -1,0,1,
    Limits::Int::int_max-1, Limits::Int::int_max
  };
  const int s2r[9] = {
    static_cast<int>(-sqrt(static_cast<double>(-Limits::Int::int_min))),
    -4,-2,-1,0,1,2,4,
    static_cast<int>(sqrt(static_cast<double>(Limits::Int::int_max)))
  };
  Gecode::IntSet s1(s1r,7);
  Gecode::IntSet s2(s2r,9);
  Gecode::IntSet s3(-8,8);
  Gecode::IntSet s4(-3,3);
}

class Mult : public IntTest {
public:
  Mult(const char* t, const Gecode::IntSet& is)
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    double d2 = static_cast<double>(x[2]);
    return d0*d1 == d2;
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = mult(home, x[0], x[1]);
    rel(home, y, Gecode::IRT_EQ, x[2], Gecode::ICL_DOM);
  }
};
namespace {
  Mult _mmmultmax("MiniModel::Mult::A",s1);
  Mult _mmmultmed("MiniModel::Mult::B",s2);
  Mult _mmmultmin("MiniModel::Mult::C",s3);
}

class Square : public IntTest {
public:
  Square(const char* t, const Gecode::IntSet& is)
    : IntTest(t,2,is) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return d0*d0 == d1;
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = sqr(home, x[0]);
    rel(home, y, Gecode::IRT_EQ, x[1], Gecode::ICL_DOM);
  }
};
namespace {
  Square _mmsquaremax("MiniModel::Square::A",s1);
  Square _mmsquaremed("MiniModel::Square::B",s2);
  Square _mmsquaremin("MiniModel::Square::C",s3);
}

class Abs : public IntTest {
public:
  Abs(const char* t, Gecode::IntConLevel icl, const Gecode::IntSet& is)
    : IntTest(t,2,is,false,icl) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return (d0<0 ? -d0 : d0) == d1;
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = abs(home, x[0], icl);
    rel(home, y, Gecode::IRT_EQ, x[1], Gecode::ICL_DOM);
  }
};
namespace {
  Abs _mmabsbndmax("MiniModel::Abs::Bnd::A",Gecode::ICL_BND,s1);
  Abs _mmabsbndmed("MiniModel::Abs::Bnd::B",Gecode::ICL_BND,s2);
  Abs _mmabsbndmin("MiniModel::Abs::Bnd::C",Gecode::ICL_BND,s3);
  Abs _mmabsdommax("MiniModel::Abs::Dom::A",Gecode::ICL_DOM,s1);
  Abs _mmabsdommed("MiniModel::Abs::Dom::B",Gecode::ICL_DOM,s2);
  Abs _mmabsdommin("MiniModel::Abs::Dom::C",Gecode::ICL_DOM,s3);
}

class Min : public IntTest {
public:
  Min(const char* t, const Gecode::IntSet& is)
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(x[0],x[1]) == x[2];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = min(home, x[0], x[1]);
    rel(home, y, Gecode::IRT_EQ, x[2], Gecode::ICL_DOM);
  }
};
namespace {
  Min _mmminmax("MiniModel::Min::Bin::A",s1);
  Min _mmminmed("MiniModel::Min::Bin::B",s2);
  Min _mmminmin("MiniModel::Min::Bin::C",s3);
}

class Max : public IntTest {
public:
  Max(const char* t, const Gecode::IntSet& is)
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(x[0],x[1]) == x[2];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVar y = max(home, x[0], x[1]);
    rel(home, y, Gecode::IRT_EQ, x[2], Gecode::ICL_DOM);
  }
};
namespace {
  Max _mmmaxmax("MiniModel::Max::Bin::A",s1);
  Max _mmmaxmed("MiniModel::Max::Bin::B",s2);
  Max _mmmaxmin("MiniModel::Max::Bin::C",s3);
}

class MinNary : public IntTest {
public:
  MinNary(const char* t, const Gecode::IntSet& is)
    : IntTest(t,4,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(std::min(x[0],x[1]),
                    x[2]) == x[3];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    IntVar y = min(home, m);
    rel(home, y, Gecode::IRT_EQ, x[3], Gecode::ICL_DOM);
  }
};
namespace {
  MinNary _mmminnary("MiniModel::Min::Nary",s4);
}

class MaxNary : public IntTest {
public:
  MaxNary(const char* t, const Gecode::IntSet& is)
    : IntTest(t,4,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(std::max(x[0],x[1]),
                    x[2]) == x[3];
  }
  virtual void post(Gecode::Space* home, Gecode::IntVarArray& x) {
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    IntVar y = max(home, m);
    rel(home, y, Gecode::IRT_EQ, x[3], Gecode::ICL_DOM);
  }
};
namespace {
  MaxNary _mmmaxnary("MiniModel::Max::Nary",s4);
}



// STATISTICS: test-int

