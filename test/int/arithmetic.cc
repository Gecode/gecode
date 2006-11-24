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
#include "test/log.hh"

#include <cmath>
#include <algorithm>

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
    mult(home, x[0], x[1], x[2]);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "x[0]*x[1] == x[2]" << std::endl;
    c << "mult(this, x[0], x[1], x[2]);" << std::endl;
  }
};
namespace {
  Mult _multmax("Arithmetic::Mult::A",s1);
  Mult _multmed("Arithmetic::Mult::B",s2);
  Mult _multmin("Arithmetic::Mult::C",s3);
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
    mult(home, x[0], x[0], x[1]);
  }
};
namespace {
  Square _squaremax("Arithmetic::Square::A",s1);
  Square _squaremed("Arithmetic::Square::B",s2);
  Square _squaremin("Arithmetic::Square::C",s3);
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
    abs(home, x[0], x[1], icl);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "abs(x[0]) == x[1]" << std::endl;
    c << "abs(this, x[0], x[1], icl);" << std::endl;
  }
};
namespace {
  Abs _absbndmax("Arithmetic::Abs::Bnd::A",ICL_BND,s1);
  Abs _absbndmed("Arithmetic::Abs::Bnd::B",ICL_BND,s2);
  Abs _absbndmin("Arithmetic::Abs::Bnd::C",ICL_BND,s3);
  Abs _absdommax("Arithmetic::Abs::Dom::A",ICL_DOM,s1);
  Abs _absdommed("Arithmetic::Abs::Dom::B",ICL_DOM,s2);
  Abs _absdommin("Arithmetic::Abs::Dom::C",ICL_DOM,s3);
}

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
namespace {
  Min _minmax("Arithmetic::Min::Bin::A",s1);
  Min _minmed("Arithmetic::Min::Bin::B",s2);
  Min _minmin("Arithmetic::Min::Bin::C",s3);
}

class MinShared : public IntTest {
public:
  MinShared(const char* t, const IntSet& is)
    : IntTest(t,2,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(x[0],x[1]) == x[0];
  }
  virtual void post(Space* home, IntVarArray& x) {
    min(home, x[0], x[1], x[0]);
  }
};
namespace {
  MinShared _minsmax("Arithmetic::Min::Bin::Shared::A",s1);
  MinShared _minsmed("Arithmetic::Min::Bin::Shared::B",s2);
  MinShared _minsmin("Arithmetic::Min::Bin::Shared::C",s3);
}

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
namespace {
  Max _maxmax("Arithmetic::Max::Bin::A",s1);
  Max _maxmed("Arithmetic::Max::Bin::B",s2);
  Max _maxmin("Arithmetic::Max::Bin::C",s3);
}

class MaxShared : public IntTest {
public:
  MaxShared(const char* t, const IntSet& is)
    : IntTest(t,2,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(x[0],x[1]) == x[0];
  }
  virtual void post(Space* home, IntVarArray& x) {
    max(home, x[0], x[1], x[0]);
  }
};
namespace {
  MaxShared _maxsmax("Arithmetic::Max::Bin::Shared::A",s1);
  MaxShared _maxsmed("Arithmetic::Max::Bin::Shared::B",s2);
  MaxShared _maxsmin("Arithmetic::Max::Bin::Shared::C",s3);
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
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    min(home, m, x[3]);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "min(x[0], x[1], x[2]) == x[3]" << std::endl;
    c << "IntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];" << std::endl;
    c << "\tmin(this, m, x[3]);" << std::endl;
  }
};
namespace {
  MinNary _minnary("Arithmetic::Min::Nary",s4);
}

class MinNaryShared : public IntTest {
public:
  MinNaryShared(const char* t, const IntSet& is)
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::min(std::min(x[0],x[1]),
                    x[2]) == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    min(home, m, x[1]);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "min(x[0], x[1], x[2]) == x[1]" << std::endl;
    c << "IntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];" << std::endl;
    c << "\tmin(this, m, x[1]);" << std::endl;
  }
};
namespace {
  MinNaryShared _minsnary("Arithmetic::Min::Nary::Shared",s4);
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
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    max(home, m, x[3]);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "max(x[0], x[1], x[2]) == x[3]" << std::endl;
    c << "IntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];" << std::endl;
    c << "\tmax(this, m, x[3]);" << std::endl;
  }
};
namespace {
  MaxNary _maxnary("Arithmetic::Max::Nary",s4);
}

class MaxNaryShared : public IntTest {
public:
  MaxNaryShared(const char* t, const IntSet& is)
    : IntTest(t,3,is) {}
  virtual bool solution(const Assignment& x) const {
    return std::max(std::max(x[0],x[1]),
                    x[2]) == x[1];
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntVarArgs m(3);
    m[0]=x[0]; m[1]=x[1]; m[2]=x[2];
    max(home, m, x[1]);
  }
  virtual void description(std::ostream& h, std::ostream& c) {
    h << "max(x[0], x[1], x[2]) == x[1]" << std::endl;
    c << "IntVarArgs m(3); m[0]=x[0]; m[1]=x[1]; m[2]=x[2];" << std::endl;
    c << "\tmax(this, m, x[1]);" << std::endl;
  }
};
namespace {
  MaxNaryShared _maxsnary("Arithmetic::Max::Nary::Shared",s4);
}

// STATISTICS: test-int
