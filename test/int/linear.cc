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

static inline bool
compare(double x, IntRelType r, double y) {
  switch (r) {
  case IRT_EQ: return x == y;
  case IRT_NQ: return x != y;
  case IRT_LQ: return x <= y;
  case IRT_LE: return x < y;
  case IRT_GR: return x > y;
  case IRT_GQ: return x >= y;
  default: ;
  }
  return false;
}

class LinearInt : public IntTest {
private:
  const int* c;
  const IntRelType irt;
public:
  LinearInt(const char* t, 
	    const IntSet& is,
	    const int* c0, int n, IntRelType irt0, int cost = 1) 
    : IntTest(t,n,is,true, cost), c(c0), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    double e = 0.0;
    for (int i=0; i<x.size(); i++)
      e += c[i]*x[i];
    return compare(e, irt, static_cast<double>(0));
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ca(x.size());
    for (int i=0; i<x.size(); i++)
      ca[i]=c[i];
    linear(home, ca, x, irt, 0);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    IntArgs ca(x.size());
    for (int i=0; i<x.size(); i++)
      ca[i]=c[i];
    linear(home, ca, x, irt, 0, b);
  }
};

static IntSet d0(-2,2);
const int d1r[5] = {-4,-1,0,1,4};
static IntSet d1(d1r,5);

const int c00[1] = {0};
const int c0[5] = {1,1,1,1,1};
const int c1[5] = {1,-1,-1,1,-1};
const int c2[5] = {-2,3,-5,7,-11};

LinearInt _li_000_eq("Linear::Int::Eq::000",d0,c00,1,IRT_EQ);
LinearInt _li_000_nq("Linear::Int::Nq::000",d0,c00,1,IRT_NQ);
LinearInt _li_000_lq("Linear::Int::Lq::000",d0,c00,1,IRT_LQ);
LinearInt _li_000_le("Linear::Int::Le::000",d0,c00,1,IRT_LE);
LinearInt _li_000_gq("Linear::Int::Gq::000",d0,c00,1,IRT_GQ);
LinearInt _li_000_gr("Linear::Int::Gr::000",d0,c00,1,IRT_GR);

LinearInt _li_001_eq("Linear::Int::Eq::001",d0,c0,1,IRT_EQ);
LinearInt _li_001_nq("Linear::Int::Nq::001",d0,c0,1,IRT_NQ);
LinearInt _li_001_lq("Linear::Int::Lq::001",d0,c0,1,IRT_LQ);
LinearInt _li_001_le("Linear::Int::Le::001",d0,c0,1,IRT_LE);
LinearInt _li_001_gq("Linear::Int::Gq::001",d0,c0,1,IRT_GQ);
LinearInt _li_001_gr("Linear::Int::Gr::001",d0,c0,1,IRT_GR);

LinearInt _li_002_eq("Linear::Int::Eq::002",d0,c0,2,IRT_EQ);
LinearInt _li_002_nq("Linear::Int::Nq::002",d0,c0,2,IRT_NQ);
LinearInt _li_002_lq("Linear::Int::Lq::002",d0,c0,2,IRT_LQ);
LinearInt _li_002_le("Linear::Int::Le::002",d0,c0,2,IRT_LE);
LinearInt _li_002_gq("Linear::Int::Gq::002",d0,c0,2,IRT_GQ);
LinearInt _li_002_gr("Linear::Int::Gr::002",d0,c0,2,IRT_GR);

LinearInt _li_003_eq("Linear::Int::Eq::003",d0,c0,3,IRT_EQ);
LinearInt _li_003_nq("Linear::Int::Nq::003",d0,c0,3,IRT_NQ);
LinearInt _li_003_lq("Linear::Int::Lq::003",d0,c0,3,IRT_LQ);
LinearInt _li_003_le("Linear::Int::Le::003",d0,c0,3,IRT_LE);
LinearInt _li_003_gq("Linear::Int::Gq::003",d0,c0,3,IRT_GQ);
LinearInt _li_003_gr("Linear::Int::Gr::003",d0,c0,3,IRT_GR);

LinearInt _li_004_eq("Linear::Int::Eq::004",d0,c0,4,IRT_EQ, 2);
LinearInt _li_004_nq("Linear::Int::Nq::004",d0,c0,4,IRT_NQ, 2);
LinearInt _li_004_lq("Linear::Int::Lq::004",d0,c0,4,IRT_LQ, 2);
LinearInt _li_004_le("Linear::Int::Le::004",d0,c0,4,IRT_LE, 2);
LinearInt _li_004_gq("Linear::Int::Gq::004",d0,c0,4,IRT_GQ, 2);
LinearInt _li_004_gr("Linear::Int::Gr::004",d0,c0,4,IRT_GR, 2);


LinearInt _li_012_eq("Linear::Int::Eq::012",d0,c1,2,IRT_EQ);
LinearInt _li_012_nq("Linear::Int::Nq::012",d0,c1,2,IRT_NQ);
LinearInt _li_012_lq("Linear::Int::Lq::012",d0,c1,2,IRT_LQ);
LinearInt _li_012_le("Linear::Int::Le::012",d0,c1,2,IRT_LE);
LinearInt _li_012_gq("Linear::Int::Gq::012",d0,c1,2,IRT_GQ);
LinearInt _li_012_gr("Linear::Int::Gr::012",d0,c1,2,IRT_GR);

LinearInt _li_013_eq("Linear::Int::Eq::013",d0,c1,3,IRT_EQ);
LinearInt _li_013_nq("Linear::Int::Nq::013",d0,c1,3,IRT_NQ);
LinearInt _li_013_lq("Linear::Int::Lq::013",d0,c1,3,IRT_LQ);
LinearInt _li_013_le("Linear::Int::Le::013",d0,c1,3,IRT_LE);
LinearInt _li_013_gq("Linear::Int::Gq::013",d0,c1,3,IRT_GQ);
LinearInt _li_013_gr("Linear::Int::Gr::013",d0,c1,3,IRT_GR);

LinearInt _li_014_eq("Linear::Int::Eq::014",d0,c1,4,IRT_EQ, 2);
LinearInt _li_014_nq("Linear::Int::Nq::014",d0,c1,4,IRT_NQ, 2);
LinearInt _li_014_lq("Linear::Int::Lq::014",d0,c1,4,IRT_LQ, 2);
LinearInt _li_014_le("Linear::Int::Le::014",d0,c1,4,IRT_LE, 2);
LinearInt _li_014_gq("Linear::Int::Gq::014",d0,c1,4,IRT_GQ, 2);
LinearInt _li_014_gr("Linear::Int::Gr::014",d0,c1,4,IRT_GR, 2);

LinearInt _li_015_eq("Linear::Int::Eq::015",d0,c1,5,IRT_EQ, 6);
LinearInt _li_015_nq("Linear::Int::Nq::015",d0,c1,5,IRT_NQ, 6);
LinearInt _li_015_lq("Linear::Int::Lq::015",d0,c1,5,IRT_LQ, 6);
LinearInt _li_015_le("Linear::Int::Le::015",d0,c1,5,IRT_LE, 6);
LinearInt _li_015_gq("Linear::Int::Gq::015",d0,c1,5,IRT_GQ, 6);
LinearInt _li_015_gr("Linear::Int::Gr::015",d0,c1,5,IRT_GR, 6);


LinearInt _li_121_eq("Linear::Int::Eq::121",d1,c2,1,IRT_EQ);
LinearInt _li_121_nq("Linear::Int::Nq::121",d1,c2,1,IRT_NQ);
LinearInt _li_121_lq("Linear::Int::Lq::121",d1,c2,1,IRT_LQ);
LinearInt _li_121_le("Linear::Int::Le::121",d1,c2,1,IRT_LE);
LinearInt _li_121_gq("Linear::Int::Gq::121",d1,c2,1,IRT_GQ);
LinearInt _li_121_gr("Linear::Int::Gr::121",d1,c2,1,IRT_GR);

LinearInt _li_122_eq("Linear::Int::Eq::122",d1,c2,2,IRT_EQ);
LinearInt _li_122_nq("Linear::Int::Nq::122",d1,c2,2,IRT_NQ);
LinearInt _li_122_lq("Linear::Int::Lq::122",d1,c2,2,IRT_LQ);
LinearInt _li_122_le("Linear::Int::Le::122",d1,c2,2,IRT_LE);
LinearInt _li_122_gq("Linear::Int::Gq::122",d1,c2,2,IRT_GQ);
LinearInt _li_122_gr("Linear::Int::Gr::122",d1,c2,2,IRT_GR);

LinearInt _li_123_eq("Linear::Int::Eq::123",d1,c2,3,IRT_EQ);
LinearInt _li_123_nq("Linear::Int::Nq::123",d1,c2,3,IRT_NQ);
LinearInt _li_123_lq("Linear::Int::Lq::123",d1,c2,3,IRT_LQ);
LinearInt _li_123_le("Linear::Int::Le::123",d1,c2,3,IRT_LE);
LinearInt _li_123_gq("Linear::Int::Gq::123",d1,c2,3,IRT_GQ);
LinearInt _li_123_gr("Linear::Int::Gr::123",d1,c2,3,IRT_GR);

LinearInt _li_124_eq("Linear::Int::Eq::124",d1,c2,4,IRT_EQ, 3);
LinearInt _li_124_nq("Linear::Int::Nq::124",d1,c2,4,IRT_NQ, 3);
LinearInt _li_124_lq("Linear::Int::Lq::124",d1,c2,4,IRT_LQ, 3);
LinearInt _li_124_le("Linear::Int::Le::124",d1,c2,4,IRT_LE, 3);
LinearInt _li_124_gq("Linear::Int::Gq::124",d1,c2,4,IRT_GQ, 3);
LinearInt _li_124_gr("Linear::Int::Gr::124",d1,c2,4,IRT_GR, 3);

LinearInt _li_125_eq("Linear::Int::Eq::125",d1,c2,5,IRT_EQ, 6);
LinearInt _li_125_nq("Linear::Int::Nq::125",d1,c2,5,IRT_NQ, 6);
LinearInt _li_125_lq("Linear::Int::Lq::125",d1,c2,5,IRT_LQ, 6);
LinearInt _li_125_le("Linear::Int::Le::125",d1,c2,5,IRT_LE, 6);
LinearInt _li_125_gq("Linear::Int::Gq::125",d1,c2,5,IRT_GQ, 6);
LinearInt _li_125_gr("Linear::Int::Gr::125",d1,c2,5,IRT_GR, 6);


IntSet db(0,1);

class LinearBool : public IntTest {
private:
  const IntRelType irt;
public:
  LinearBool(const char* t, int n, IntRelType irt0) 
    : IntTest(t,n,db,false), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    double m = 0;
    for (int i=0; i<x.size(); i++)
      m += x[i];
    return compare(m, irt, static_cast<double>(1));
  }
  virtual void post(Space* home, IntVarArray& x) {
    linear(home, x, irt, 1);
  }
};

LinearBool _lb_1_eq("Linear::Bool::Eq::1",1,IRT_EQ);
LinearBool _lb_1_nq("Linear::Bool::Nq::1",1,IRT_NQ);
LinearBool _lb_1_lq("Linear::Bool::Lq::1",1,IRT_LQ);
LinearBool _lb_1_le("Linear::Bool::Le::1",1,IRT_LE);
LinearBool _lb_1_gq("Linear::Bool::Gq::1",1,IRT_GQ);
LinearBool _lb_1_gr("Linear::Bool::Gr::1",1,IRT_GR);

LinearBool _lb_3_eq("Linear::Bool::Eq::3",3,IRT_EQ);
LinearBool _lb_3_nq("Linear::Bool::Nq::3",3,IRT_NQ);
LinearBool _lb_3_lq("Linear::Bool::Lq::3",3,IRT_LQ);
LinearBool _lb_3_le("Linear::Bool::Le::3",3,IRT_LE);
LinearBool _lb_3_gq("Linear::Bool::Gq::3",3,IRT_GQ);
LinearBool _lb_3_gr("Linear::Bool::Gr::3",3,IRT_GR);

// STATISTICS: test-int

