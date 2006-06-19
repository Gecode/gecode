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
	    const int* c0, int n, IntRelType irt0, IntConLevel icl) 
    : IntTest(t,n,is,icl != ICL_DOM,icl), c(c0), irt(irt0) {}
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
    linear(home, ca, x, irt, 0, icl);
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

LinearInt _li_000_eq_b("Linear::Int::Bnd::Eq::000",d0,c00,1,IRT_EQ,ICL_DEF);
LinearInt _li_000_nq_b("Linear::Int::Bnd::Nq::000",d0,c00,1,IRT_NQ,ICL_DEF);
LinearInt _li_000_lq_b("Linear::Int::Bnd::Lq::000",d0,c00,1,IRT_LQ,ICL_DEF);
LinearInt _li_000_le_b("Linear::Int::Bnd::Le::000",d0,c00,1,IRT_LE,ICL_DEF);
LinearInt _li_000_gq_b("Linear::Int::Bnd::Gq::000",d0,c00,1,IRT_GQ,ICL_DEF);
LinearInt _li_000_gr_b("Linear::Int::Bnd::Gr::000",d0,c00,1,IRT_GR,ICL_DEF);

LinearInt _li_001_eq_b("Linear::Int::Bnd::Eq::001",d0,c0,1,IRT_EQ,ICL_DEF);
LinearInt _li_001_nq_b("Linear::Int::Bnd::Nq::001",d0,c0,1,IRT_NQ,ICL_DEF);
LinearInt _li_001_lq_b("Linear::Int::Bnd::Lq::001",d0,c0,1,IRT_LQ,ICL_DEF);
LinearInt _li_001_le_b("Linear::Int::Bnd::Le::001",d0,c0,1,IRT_LE,ICL_DEF);
LinearInt _li_001_gq_b("Linear::Int::Bnd::Gq::001",d0,c0,1,IRT_GQ,ICL_DEF);
LinearInt _li_001_gr_b("Linear::Int::Bnd::Gr::001",d0,c0,1,IRT_GR,ICL_DEF);

LinearInt _li_002_eq_b("Linear::Int::Bnd::Eq::002",d0,c0,2,IRT_EQ,ICL_DEF);
LinearInt _li_002_nq_b("Linear::Int::Bnd::Nq::002",d0,c0,2,IRT_NQ,ICL_DEF);
LinearInt _li_002_lq_b("Linear::Int::Bnd::Lq::002",d0,c0,2,IRT_LQ,ICL_DEF);
LinearInt _li_002_le_b("Linear::Int::Bnd::Le::002",d0,c0,2,IRT_LE,ICL_DEF);
LinearInt _li_002_gq_b("Linear::Int::Bnd::Gq::002",d0,c0,2,IRT_GQ,ICL_DEF);
LinearInt _li_002_gr_b("Linear::Int::Bnd::Gr::002",d0,c0,2,IRT_GR,ICL_DEF);

LinearInt _li_003_eq_b("Linear::Int::Bnd::Eq::003",d0,c0,3,IRT_EQ,ICL_DEF);
LinearInt _li_003_nq_b("Linear::Int::Bnd::Nq::003",d0,c0,3,IRT_NQ,ICL_DEF);
LinearInt _li_003_lq_b("Linear::Int::Bnd::Lq::003",d0,c0,3,IRT_LQ,ICL_DEF);
LinearInt _li_003_le_b("Linear::Int::Bnd::Le::003",d0,c0,3,IRT_LE,ICL_DEF);
LinearInt _li_003_gq_b("Linear::Int::Bnd::Gq::003",d0,c0,3,IRT_GQ,ICL_DEF);
LinearInt _li_003_gr_b("Linear::Int::Bnd::Gr::003",d0,c0,3,IRT_GR,ICL_DEF);

LinearInt _li_004_eq_b("Linear::Int::Bnd::Eq::004",d0,c0,4,IRT_EQ,ICL_DEF);
LinearInt _li_004_nq_b("Linear::Int::Bnd::Nq::004",d0,c0,4,IRT_NQ,ICL_DEF);
LinearInt _li_004_lq_b("Linear::Int::Bnd::Lq::004",d0,c0,4,IRT_LQ,ICL_DEF);
LinearInt _li_004_le_b("Linear::Int::Bnd::Le::004",d0,c0,4,IRT_LE,ICL_DEF);
LinearInt _li_004_gq_b("Linear::Int::Bnd::Gq::004",d0,c0,4,IRT_GQ,ICL_DEF);
LinearInt _li_004_gr_b("Linear::Int::Bnd::Gr::004",d0,c0,4,IRT_GR,ICL_DEF);


LinearInt _li_012_eq_b("Linear::Int::Bnd::Eq::012",d0,c1,2,IRT_EQ,ICL_DEF);
LinearInt _li_012_nq_b("Linear::Int::Bnd::Nq::012",d0,c1,2,IRT_NQ,ICL_DEF);
LinearInt _li_012_lq_b("Linear::Int::Bnd::Lq::012",d0,c1,2,IRT_LQ,ICL_DEF);
LinearInt _li_012_le_b("Linear::Int::Bnd::Le::012",d0,c1,2,IRT_LE,ICL_DEF);
LinearInt _li_012_gq_b("Linear::Int::Bnd::Gq::012",d0,c1,2,IRT_GQ,ICL_DEF);
LinearInt _li_012_gr_b("Linear::Int::Bnd::Gr::012",d0,c1,2,IRT_GR,ICL_DEF);

LinearInt _li_013_eq_b("Linear::Int::Bnd::Eq::013",d0,c1,3,IRT_EQ,ICL_DEF);
LinearInt _li_013_nq_b("Linear::Int::Bnd::Nq::013",d0,c1,3,IRT_NQ,ICL_DEF);
LinearInt _li_013_lq_b("Linear::Int::Bnd::Lq::013",d0,c1,3,IRT_LQ,ICL_DEF);
LinearInt _li_013_le_b("Linear::Int::Bnd::Le::013",d0,c1,3,IRT_LE,ICL_DEF);
LinearInt _li_013_gq_b("Linear::Int::Bnd::Gq::013",d0,c1,3,IRT_GQ,ICL_DEF);
LinearInt _li_013_gr_b("Linear::Int::Bnd::Gr::013",d0,c1,3,IRT_GR,ICL_DEF);

LinearInt _li_014_eq_b("Linear::Int::Bnd::Eq::014",d0,c1,4,IRT_EQ,ICL_DEF);
LinearInt _li_014_nq_b("Linear::Int::Bnd::Nq::014",d0,c1,4,IRT_NQ,ICL_DEF);
LinearInt _li_014_lq_b("Linear::Int::Bnd::Lq::014",d0,c1,4,IRT_LQ,ICL_DEF);
LinearInt _li_014_le_b("Linear::Int::Bnd::Le::014",d0,c1,4,IRT_LE,ICL_DEF);
LinearInt _li_014_gq_b("Linear::Int::Bnd::Gq::014",d0,c1,4,IRT_GQ,ICL_DEF);
LinearInt _li_014_gr_b("Linear::Int::Bnd::Gr::014",d0,c1,4,IRT_GR,ICL_DEF);

LinearInt _li_015_eq_b("Linear::Int::Bnd::Eq::015",d0,c1,5,IRT_EQ,ICL_DEF);
LinearInt _li_015_nq_b("Linear::Int::Bnd::Nq::015",d0,c1,5,IRT_NQ,ICL_DEF);
LinearInt _li_015_lq_b("Linear::Int::Bnd::Lq::015",d0,c1,5,IRT_LQ,ICL_DEF);
LinearInt _li_015_le_b("Linear::Int::Bnd::Le::015",d0,c1,5,IRT_LE,ICL_DEF);
LinearInt _li_015_gq_b("Linear::Int::Bnd::Gq::015",d0,c1,5,IRT_GQ,ICL_DEF);
LinearInt _li_015_gr_b("Linear::Int::Bnd::Gr::015",d0,c1,5,IRT_GR,ICL_DEF);


LinearInt _li_121_eq_b("Linear::Int::Bnd::Eq::121",d1,c2,1,IRT_EQ,ICL_DEF);
LinearInt _li_121_nq_b("Linear::Int::Bnd::Nq::121",d1,c2,1,IRT_NQ,ICL_DEF);
LinearInt _li_121_lq_b("Linear::Int::Bnd::Lq::121",d1,c2,1,IRT_LQ,ICL_DEF);
LinearInt _li_121_le_b("Linear::Int::Bnd::Le::121",d1,c2,1,IRT_LE,ICL_DEF);
LinearInt _li_121_gq_b("Linear::Int::Bnd::Gq::121",d1,c2,1,IRT_GQ,ICL_DEF);
LinearInt _li_121_gr_b("Linear::Int::Bnd::Gr::121",d1,c2,1,IRT_GR,ICL_DEF);

LinearInt _li_122_eq_b("Linear::Int::Bnd::Eq::122",d1,c2,2,IRT_EQ,ICL_DEF);
LinearInt _li_122_nq_b("Linear::Int::Bnd::Nq::122",d1,c2,2,IRT_NQ,ICL_DEF);
LinearInt _li_122_lq_b("Linear::Int::Bnd::Lq::122",d1,c2,2,IRT_LQ,ICL_DEF);
LinearInt _li_122_le_b("Linear::Int::Bnd::Le::122",d1,c2,2,IRT_LE,ICL_DEF);
LinearInt _li_122_gq_b("Linear::Int::Bnd::Gq::122",d1,c2,2,IRT_GQ,ICL_DEF);
LinearInt _li_122_gr_b("Linear::Int::Bnd::Gr::122",d1,c2,2,IRT_GR,ICL_DEF);

LinearInt _li_123_eq_b("Linear::Int::Bnd::Eq::123",d1,c2,3,IRT_EQ,ICL_DEF);
LinearInt _li_123_nq_b("Linear::Int::Bnd::Nq::123",d1,c2,3,IRT_NQ,ICL_DEF);
LinearInt _li_123_lq_b("Linear::Int::Bnd::Lq::123",d1,c2,3,IRT_LQ,ICL_DEF);
LinearInt _li_123_le_b("Linear::Int::Bnd::Le::123",d1,c2,3,IRT_LE,ICL_DEF);
LinearInt _li_123_gq_b("Linear::Int::Bnd::Gq::123",d1,c2,3,IRT_GQ,ICL_DEF);
LinearInt _li_123_gr_b("Linear::Int::Bnd::Gr::123",d1,c2,3,IRT_GR,ICL_DEF);

LinearInt _li_124_eq_b("Linear::Int::Bnd::Eq::124",d1,c2,4,IRT_EQ,ICL_DEF);
LinearInt _li_124_nq_b("Linear::Int::Bnd::Nq::124",d1,c2,4,IRT_NQ,ICL_DEF);
LinearInt _li_124_lq_b("Linear::Int::Bnd::Lq::124",d1,c2,4,IRT_LQ,ICL_DEF);
LinearInt _li_124_le_b("Linear::Int::Bnd::Le::124",d1,c2,4,IRT_LE,ICL_DEF);
LinearInt _li_124_gq_b("Linear::Int::Bnd::Gq::124",d1,c2,4,IRT_GQ,ICL_DEF);
LinearInt _li_124_gr_b("Linear::Int::Bnd::Gr::124",d1,c2,4,IRT_GR,ICL_DEF);

LinearInt _li_125_eq_b("Linear::Int::Bnd::Eq::125",d1,c2,5,IRT_EQ,ICL_DEF);
LinearInt _li_125_nq_b("Linear::Int::Bnd::Nq::125",d1,c2,5,IRT_NQ,ICL_DEF);
LinearInt _li_125_lq_b("Linear::Int::Bnd::Lq::125",d1,c2,5,IRT_LQ,ICL_DEF);
LinearInt _li_125_le_b("Linear::Int::Bnd::Le::125",d1,c2,5,IRT_LE,ICL_DEF);
LinearInt _li_125_gq_b("Linear::Int::Bnd::Gq::125",d1,c2,5,IRT_GQ,ICL_DEF);
LinearInt _li_125_gr_b("Linear::Int::Bnd::Gr::125",d1,c2,5,IRT_GR,ICL_DEF);



LinearInt _li_000_eq_d("Linear::Int::Dom::Eq::000",d0,c00,1,IRT_EQ,ICL_DOM);
LinearInt _li_001_eq_d("Linear::Int::Dom::Eq::001",d0,c0,1,IRT_EQ,ICL_DOM);
LinearInt _li_002_eq_d("Linear::Int::Dom::Eq::002",d0,c0,2,IRT_EQ,ICL_DOM);
LinearInt _li_003_eq_d("Linear::Int::Dom::Eq::003",d0,c0,3,IRT_EQ,ICL_DOM);
LinearInt _li_004_eq_d("Linear::Int::Dom::Eq::004",d0,c0,4,IRT_EQ,ICL_DOM);
LinearInt _li_012_eq_d("Linear::Int::Dom::Eq::012",d0,c1,2,IRT_EQ,ICL_DOM);
LinearInt _li_013_eq_d("Linear::Int::Dom::Eq::013",d0,c1,3,IRT_EQ,ICL_DOM);
LinearInt _li_014_eq_d("Linear::Int::Dom::Eq::014",d0,c1,4,IRT_EQ,ICL_DOM);
LinearInt _li_015_eq_d("Linear::Int::Dom::Eq::015",d0,c1,5,IRT_EQ,ICL_DOM);
LinearInt _li_121_eq_d("Linear::Int::Dom::Eq::121",d1,c2,1,IRT_EQ,ICL_DOM);
LinearInt _li_122_eq_d("Linear::Int::Dom::Eq::122",d1,c2,2,IRT_EQ,ICL_DOM);
LinearInt _li_123_eq_d("Linear::Int::Dom::Eq::123",d1,c2,3,IRT_EQ,ICL_DOM);
LinearInt _li_124_eq_d("Linear::Int::Dom::Eq::124",d1,c2,4,IRT_EQ,ICL_DOM);
LinearInt _li_125_eq_d("Linear::Int::Dom::Eq::125",d1,c2,5,IRT_EQ,ICL_DOM);


IntSet db(0,1);

class LinearBool : public IntTest {
private:
  const int rhs;
  const IntRelType irt;
public:
  LinearBool(const char* t, int n, int rhs0, IntRelType irt0) 
    : IntTest(t,n,db,false), rhs(rhs0), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    double m = 0;
    for (int i=0; i<x.size(); i++)
      m += x[i];
    return compare(m, irt, static_cast<double>(rhs));
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(x.size());
    for (int i=x.size(); i--; ) {
      BoolVar bx(x[i]);
      b[i] = bx;
    }
    linear(home, b, irt, rhs);
  }
};

LinearBool _lb_1_1_eq("Linear::Bool::Eq::1::1",1,1,IRT_EQ);
LinearBool _lb_1_1_nq("Linear::Bool::Nq::1::1",1,1,IRT_NQ);
LinearBool _lb_1_1_lq("Linear::Bool::Lq::1::1",1,1,IRT_LQ);
LinearBool _lb_1_1_le("Linear::Bool::Le::1::1",1,1,IRT_LE);
LinearBool _lb_1_1_gq("Linear::Bool::Gq::1::1",1,1,IRT_GQ);
LinearBool _lb_1_1_gr("Linear::Bool::Gr::1::1",1,1,IRT_GR);

LinearBool _lb_3_1_eq("Linear::Bool::Eq::3::1",3,1,IRT_EQ);
LinearBool _lb_3_1_nq("Linear::Bool::Nq::3::1",3,1,IRT_NQ);
LinearBool _lb_3_1_lq("Linear::Bool::Lq::3::1",3,1,IRT_LQ);
LinearBool _lb_3_1_le("Linear::Bool::Le::3::1",3,1,IRT_LE);
LinearBool _lb_3_1_gq("Linear::Bool::Gq::3::1",3,1,IRT_GQ);
LinearBool _lb_3_1_gr("Linear::Bool::Gr::3::1",3,1,IRT_GR);

LinearBool _lb_5_1_eq("Linear::Bool::Eq::5::1",5,1,IRT_EQ);
LinearBool _lb_5_1_nq("Linear::Bool::Nq::5::1",5,1,IRT_NQ);
LinearBool _lb_5_1_lq("Linear::Bool::Lq::5::1",5,1,IRT_LQ);
LinearBool _lb_5_1_le("Linear::Bool::Le::5::1",5,1,IRT_LE);
LinearBool _lb_5_1_gq("Linear::Bool::Gq::5::1",5,1,IRT_GQ);
LinearBool _lb_5_1_gr("Linear::Bool::Gr::5::1",5,1,IRT_GR);

LinearBool _lb_5_3_eq("Linear::Bool::Eq::5::3",5,3,IRT_EQ);
LinearBool _lb_5_3_nq("Linear::Bool::Nq::5::3",5,3,IRT_NQ);
LinearBool _lb_5_3_lq("Linear::Bool::Lq::5::3",5,3,IRT_LQ);
LinearBool _lb_5_3_le("Linear::Bool::Le::5::3",5,3,IRT_LE);
LinearBool _lb_5_3_gq("Linear::Bool::Gq::5::3",5,3,IRT_GQ);
LinearBool _lb_5_3_gr("Linear::Bool::Gr::5::3",5,3,IRT_GR);

LinearBool _lb_10_3_eq("Linear::Bool::Eq::10::3",10,3,IRT_EQ);
LinearBool _lb_10_3_nq("Linear::Bool::Nq::10::3",10,3,IRT_NQ);
LinearBool _lb_10_3_lq("Linear::Bool::Lq::10::3",10,3,IRT_LQ);
LinearBool _lb_10_3_le("Linear::Bool::Le::10::3",10,3,IRT_LE);
LinearBool _lb_10_3_gq("Linear::Bool::Gq::10::3",10,3,IRT_GQ);
LinearBool _lb_10_3_gr("Linear::Bool::Gr::10::3",10,3,IRT_GR);

LinearBool _lb_10_5_eq("Linear::Bool::Eq::10::5",10,5,IRT_EQ);
LinearBool _lb_10_5_nq("Linear::Bool::Nq::10::5",10,5,IRT_NQ);
LinearBool _lb_10_5_lq("Linear::Bool::Lq::10::5",10,5,IRT_LQ);
LinearBool _lb_10_5_le("Linear::Bool::Le::10::5",10,5,IRT_LE);
LinearBool _lb_10_5_gq("Linear::Bool::Gq::10::5",10,5,IRT_GQ);
LinearBool _lb_10_5_gr("Linear::Bool::Gr::10::5",10,5,IRT_GR);


// STATISTICS: test-int

