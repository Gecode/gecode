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

#include <cmath>
#include <algorithm>

static const int s1r[7] = {
  Limits::Int::int_min, Limits::Int::int_min+1, 
  -1,0,1,
  Limits::Int::int_max-1, Limits::Int::int_max
}; 
static const int s2r[9] = {
  static_cast<int>(-sqrt(static_cast<double>(-Limits::Int::int_min))),
  -4,-2,-1,0,1,2,4,
  static_cast<int>(sqrt(static_cast<double>(Limits::Int::int_max)))
}; 
static IntSet s1(s1r,7);
static IntSet s2(s2r,9);
static IntSet s3(-8,8);
static IntSet s4(-3,3);

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
    mult(home, x[0], x[1], x[2]);
  }
};
static Mult _multmax("Arithmetic::Mult::A",s1);
static Mult _multmed("Arithmetic::Mult::B",s2);
static Mult _multmin("Arithmetic::Mult::C",s3);

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
    mult(home, x[0], x[0], x[1]);
  }
};
static Square _squaremax("Arithmetic::Square::A",s1);
static Square _squaremed("Arithmetic::Square::B",s2);
static Square _squaremin("Arithmetic::Square::C",s3);

class AbsBnd : public IntTest {
public:
  AbsBnd(const char* t, const IntSet& is) 
    : IntTest(t,2,is) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return (d0<0 ? -d0 : d0) == d1;
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("post abs(x0, x1)",
	     "\tabs(this, x[0], x[1], ICL_BND);");
    abs(home, x[0], x[1], ICL_BND);
  }
};
static AbsBnd _absbndmax("Arithmetic::AbsBnd::A",s1);
static AbsBnd _absbndmed("Arithmetic::AbsBnd::B",s2);
static AbsBnd _absbndmin("Arithmetic::AbsBnd::C",s3);

class AbsDom : public IntTest {
public:
  AbsDom(const char* t, const IntSet& is) 
    : IntTest(t,2,is,false,ICL_DOM) {}
  virtual bool solution(const Assignment& x) const {
    double d0 = static_cast<double>(x[0]);
    double d1 = static_cast<double>(x[1]);
    return (d0<0 ? -d0 : d0) == d1;
  }
  virtual void post(Space* home, IntVarArray& x) {
    Log::log("post abs(x0, x1)",
	     "\tabs(this, x[0], x[1], ICL_DOM);");
    abs(home, x[0], x[1], ICL_DOM);
  }
};
static AbsDom _absdommax("Arithmetic::AbsDom::A",s1);
static AbsDom _absdommed("Arithmetic::AbsDom::B",s2);
static AbsDom _absdommin("Arithmetic::AbsDom::C",s3);

class Min : public IntTest {
public:
  Min(const char* t, const IntSet& is) 
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(x[0],x[1]) == x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    min(home, x[0], x[1], x[2]);
  }
};
static Min _minmax("Arithmetic::Min::Bin::A",s1);
static Min _minmed("Arithmetic::Min::Bin::B",s2);
static Min _minmin("Arithmetic::Min::Bin::C",s3);

class Max : public IntTest {
public:
  Max(const char* t, const IntSet& is) 
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(x[0],x[1]) == x[2];
  }
  virtual void post(Space* home, IntVarArray& x) {
    max(home, x[0], x[1], x[2]);
  }
};
static Max _maxmax("Arithmetic::Max::Bin::A",s1);
static Max _maxmed("Arithmetic::Max::Bin::B",s2);
static Max _maxmin("Arithmetic::Max::Bin::C",s3);

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
    min(home, m, x[3]);
  }
};
static MinNary _minnary("Arithmetic::Min::Nary",s4);

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
    max(home, m, x[3]);
  }
};
static MaxNary _maxnary("Arithmetic::Max::Nary",s4);

// STATISTICS: test-int
