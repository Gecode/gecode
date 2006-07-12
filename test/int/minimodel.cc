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
#include "test/log.hh"
#include "gecode/minimodel.hh"

static IntSet is(-2,2);

class MmLinEqA : public IntTest {
public:
  MmLinEqA(void)
    : IntTest("MiniModel::Lin::Eq::A",2,is,true) {}
  virtual bool solution(const Assignment& x) const {
    return (2*x[0]+1) == (x[1]-1);
  }
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, 2*x[0]+1 == x[1]-1);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, 2*x[0]+1-x[1] == -1);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, -(1-(1-x[1])) == x[0]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, 2*(1-x[1]) == x[0]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, 1>x[0]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, 1<x[0]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
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
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::post(home, -1==x[0]);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    Gecode::post(home, tt(eqv(-1==x[0],b)));
  }
};
MmLinEqG _mmlineqg;

class MmLinExprA : public IntTest {
public:
  MmLinExprA(void)
    : IntTest("MiniModel::Lin::Expr::A",4,is,false) {}
  virtual bool solution(const Assignment& x) const {
    return 2*x[0]+3*x[1]-x[2] == x[3];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVar y = Gecode::post(home, 2*x[0] + 3*x[1] - x[2]);
    Gecode::eq(home,y,x[3]);
  }
};
MmLinExprA _mmlinexpra;

static IntSet b(0,1);

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
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(4);
    for (int i=x.size(); i--; ) {
      Gecode::dom(home, x[i], 0, 1);
      BoolVar bx(x[i]); b[i]=bx;
    }
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
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(4);
    for (int i=x.size(); i--; ) {
      Gecode::dom(home, x[i], 0, 1);
      BoolVar bx(x[i]); b[i]=bx;
    }
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
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(4);
    for (int i=x.size(); i--; ) {
      Gecode::dom(home, x[i], 0, 1);
      BoolVar bx(x[i]); b[i]=bx;
    }
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
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(4);
    for (int i=x.size(); i--; ) {
      Gecode::dom(home, x[i], 0, 1);
      BoolVar bx(x[i]); b[i]=bx;
    }
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
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(4);
    for (int i=x.size(); i--; ) {
      Gecode::dom(home, x[i], 0, 1);
      BoolVar bx(x[i]); b[i]=bx;
    }
    Gecode::post(home, ff(b[0] && b[1] && b[2] && b[3]));
  }
};
MmBoolE _mmboole;


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
  IntSet s1(s1r,7);
  IntSet s2(s2r,9);
  IntSet s3(-8,8);
  IntSet s4(-3,3);
}

class Mult : public IntTest {
public:
  Mult(const char* t, const IntSet& is) 
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    double d2 = static_cast<double>(x[2]);
    return d0*d1 == d2;
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("x[0]*x[1]==x[2]",
	     "\tmult(this, x[0], x[1], x[2]);");
    IntVar y = mult(home, x[0], x[1]);
    eq(home, y, x[2], ICL_DOM);
  }
};
namespace {
  Mult _mmmultmax("MiniModel::Mult::A",s1);
  Mult _mmmultmed("MiniModel::Mult::B",s2);
  Mult _mmmultmin("MiniModel::Mult::C",s3);
}

class Square : public IntTest {
public:
  Square(const char* t, const IntSet& is) 
    : IntTest(t,2,is) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return d0*d0 == d1;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVar y = sqr(home, x[0]);
    eq(home, y, x[1], ICL_DOM);
  }
};
namespace {
  Square _mmsquaremax("MiniModel::Square::A",s1);
  Square _mmsquaremed("MiniModel::Square::B",s2);
  Square _mmsquaremin("MiniModel::Square::C",s3);
}

class Abs : public IntTest {
public:
  Abs(const char* t, IntConLevel icl, const IntSet& is) 
    : IntTest(t,2,is,false,icl) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return (d0<0 ? -d0 : d0) == d1;
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("post abs(x0, x1)",
	     "\tabs(this, x[0], x[1], icl);");
    IntVar y = abs(home, x[0], icl);
    eq(home, y, x[1], ICL_DOM);
  }
};
namespace {
  Abs _mmabsbndmax("MiniModel::Abs::Bnd::A",ICL_BND,s1);
  Abs _mmabsbndmed("MiniModel::Abs::Bnd::B",ICL_BND,s2);
  Abs _mmabsbndmin("MiniModel::Abs::Bnd::C",ICL_BND,s3);
  Abs _mmabsdommax("MiniModel::Abs::Dom::A",ICL_DOM,s1);
  Abs _mmabsdommed("MiniModel::Abs::Dom::B",ICL_DOM,s2);
  Abs _mmabsdommin("MiniModel::Abs::Dom::C",ICL_DOM,s3);
}

class Min : public IntTest {
public:
  Min(const char* t, const IntSet& is) 
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(x[0],x[1]) == x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVar y = min(home, x[0], x[1]);
    eq(home, y, x[2], ICL_DOM);
  }
};
namespace {
  Min _mmminmax("MiniModel::Min::Bin::A",s1);
  Min _mmminmed("MiniModel::Min::Bin::B",s2);
  Min _mmminmin("MiniModel::Min::Bin::C",s3);
}

class Max : public IntTest {
public:
  Max(const char* t, const IntSet& is) 
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(x[0],x[1]) == x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVar y = max(home, x[0], x[1]);
    eq(home, y, x[2], ICL_DOM);
  }
};
namespace {
  Max _mmmaxmax("MiniModel::Max::Bin::A",s1);
  Max _mmmaxmed("MiniModel::Max::Bin::B",s2);
  Max _mmmaxmin("MiniModel::Max::Bin::C",s3);
}

class MinNary : public IntTest {
public:
  MinNary(const char* t, const IntSet& is) 
    : IntTest(t,4,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(std::min(x[0],x[1]),
		    x[2]) == x[3];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("min(home, x[0:2], x[3])",
	     "\tIntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];\n"
	     "\tmin(this, m, x[3]);");
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    IntVar y = min(home, m);
    eq(home, y, x[3], ICL_DOM);
  }
};
namespace {
  MinNary _mmminnary("MiniModel::Min::Nary",s4);
}

class MaxNary : public IntTest {
public:
  MaxNary(const char* t, const IntSet& is) 
    : IntTest(t,4,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(std::max(x[0],x[1]),
		    x[2]) == x[3];
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("max(home, x[0:2], x[3])",
	     "\tIntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];\n"
	     "\tmax(this, m, x[3]);");
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    IntVar y = max(home, m);
    eq(home, y, x[3], ICL_DOM);
  }
};
namespace {
  MaxNary _mmmaxnary("MiniModel::Max::Nary",s4);
}



// STATISTICS: test-int

