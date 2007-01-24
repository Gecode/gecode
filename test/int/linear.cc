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

class LinearBoolIntUnitPos : public IntTest {
private:
  const int rhs;
  const IntRelType irt;
public:
  LinearBoolIntUnitPos(const char* t, int n, int rhs0, IntRelType irt0)
    : IntTest(t,n,db,false), rhs(rhs0), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    double m = 0;
    for (int i=0; i<x.size(); i++)
      m += x[i];
    return compare(m, irt, static_cast<double>(rhs));
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(x.size());
    for (int i=x.size(); i--; )
      b[i]=link(home,x[i]);
    linear(home, b, irt, rhs);
  }
};

LinearBoolIntUnitPos _lb_i_up_1_1_eq("Linear::Bool::Int::Unit::Pos::Eq::1::1",1,1,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_1_1_nq("Linear::Bool::Int::Unit::Pos::Nq::1::1",1,1,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_1_1_lq("Linear::Bool::Int::Unit::Pos::Lq::1::1",1,1,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_1_1_le("Linear::Bool::Int::Unit::Pos::Le::1::1",1,1,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_1_1_gq("Linear::Bool::Int::Unit::Pos::Gq::1::1",1,1,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_1_1_gr("Linear::Bool::Int::Unit::Pos::Gr::1::1",1,1,IRT_GR);

LinearBoolIntUnitPos _lb_i_up_3_1_eq("Linear::Bool::Int::Unit::Pos::Eq::3::1",3,1,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_3_1_nq("Linear::Bool::Int::Unit::Pos::Nq::3::1",3,1,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_3_1_lq("Linear::Bool::Int::Unit::Pos::Lq::3::1",3,1,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_3_1_le("Linear::Bool::Int::Unit::Pos::Le::3::1",3,1,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_3_1_gq("Linear::Bool::Int::Unit::Pos::Gq::3::1",3,1,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_3_1_gr("Linear::Bool::Int::Unit::Pos::Gr::3::1",3,1,IRT_GR);

LinearBoolIntUnitPos _lb_i_up_5_1_eq("Linear::Bool::Int::Unit::Pos::Eq::5::1",5,1,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_5_1_nq("Linear::Bool::Int::Unit::Pos::Nq::5::1",5,1,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_5_1_lq("Linear::Bool::Int::Unit::Pos::Lq::5::1",5,1,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_5_1_le("Linear::Bool::Int::Unit::Pos::Le::5::1",5,1,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_5_1_gq("Linear::Bool::Int::Unit::Pos::Gq::5::1",5,1,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_5_1_gr("Linear::Bool::Int::Unit::Pos::Gr::5::1",5,1,IRT_GR);

LinearBoolIntUnitPos _lb_i_up_5_3_eq("Linear::Bool::Int::Unit::Pos::Eq::5::3",5,3,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_5_3_nq("Linear::Bool::Int::Unit::Pos::Nq::5::3",5,3,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_5_3_lq("Linear::Bool::Int::Unit::Pos::Lq::5::3",5,3,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_5_3_le("Linear::Bool::Int::Unit::Pos::Le::5::3",5,3,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_5_3_gq("Linear::Bool::Int::Unit::Pos::Gq::5::3",5,3,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_5_3_gr("Linear::Bool::Int::Unit::Pos::Gr::5::3",5,3,IRT_GR);

LinearBoolIntUnitPos _lb_i_up_10_3_eq("Linear::Bool::Int::Unit::Pos::Eq::10::3",10,3,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_10_3_nq("Linear::Bool::Int::Unit::Pos::Nq::10::3",10,3,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_10_3_lq("Linear::Bool::Int::Unit::Pos::Lq::10::3",10,3,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_10_3_le("Linear::Bool::Int::Unit::Pos::Le::10::3",10,3,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_10_3_gq("Linear::Bool::Int::Unit::Pos::Gq::10::3",10,3,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_10_3_gr("Linear::Bool::Int::Unit::Pos::Gr::10::3",10,3,IRT_GR);

LinearBoolIntUnitPos _lb_i_up_10_5_eq("Linear::Bool::Int::Unit::Pos::Eq::10::5",10,5,IRT_EQ);
LinearBoolIntUnitPos _lb_i_up_10_5_nq("Linear::Bool::Int::Unit::Pos::Nq::10::5",10,5,IRT_NQ);
LinearBoolIntUnitPos _lb_i_up_10_5_lq("Linear::Bool::Int::Unit::Pos::Lq::10::5",10,5,IRT_LQ);
LinearBoolIntUnitPos _lb_i_up_10_5_le("Linear::Bool::Int::Unit::Pos::Le::10::5",10,5,IRT_LE);
LinearBoolIntUnitPos _lb_i_up_10_5_gq("Linear::Bool::Int::Unit::Pos::Gq::10::5",10,5,IRT_GQ);
LinearBoolIntUnitPos _lb_i_up_10_5_gr("Linear::Bool::Int::Unit::Pos::Gr::10::5",10,5,IRT_GR);


class LinearBoolIntUnitNeg : public IntTest {
private:
  const int rhs;
  const IntRelType irt;
public:
  LinearBoolIntUnitNeg(const char* t, int n, int rhs0, IntRelType irt0)
    : IntTest(t,n,db,false), rhs(rhs0), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    double m = 0;
    for (int i=0; i<x.size(); i++)
      m -= x[i];
    return compare(m, irt, -static_cast<double>(rhs));
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(x.size());
    for (int i=x.size(); i--; )
      b[i]=link(home,x[i]);
    IntArgs c(x.size());
    for (int i=x.size(); i--; )
      c[i]=-1;
    linear(home, c, b, irt, -rhs);
  }
};

LinearBoolIntUnitNeg _lb_i_un_1_1_eq("Linear::Bool::Int::Unit::Neg::Eq::1::1",1,1,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_1_1_nq("Linear::Bool::Int::Unit::Neg::Nq::1::1",1,1,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_1_1_lq("Linear::Bool::Int::Unit::Neg::Lq::1::1",1,1,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_1_1_le("Linear::Bool::Int::Unit::Neg::Le::1::1",1,1,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_1_1_gq("Linear::Bool::Int::Unit::Neg::Gq::1::1",1,1,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_1_1_gr("Linear::Bool::Int::Unit::Neg::Gr::1::1",1,1,IRT_GR);

LinearBoolIntUnitNeg _lb_i_un_3_1_eq("Linear::Bool::Int::Unit::Neg::Eq::3::1",3,1,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_3_1_nq("Linear::Bool::Int::Unit::Neg::Nq::3::1",3,1,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_3_1_lq("Linear::Bool::Int::Unit::Neg::Lq::3::1",3,1,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_3_1_le("Linear::Bool::Int::Unit::Neg::Le::3::1",3,1,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_3_1_gq("Linear::Bool::Int::Unit::Neg::Gq::3::1",3,1,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_3_1_gr("Linear::Bool::Int::Unit::Neg::Gr::3::1",3,1,IRT_GR);

LinearBoolIntUnitNeg _lb_i_un_5_1_eq("Linear::Bool::Int::Unit::Neg::Eq::5::1",5,1,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_5_1_nq("Linear::Bool::Int::Unit::Neg::Nq::5::1",5,1,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_5_1_lq("Linear::Bool::Int::Unit::Neg::Lq::5::1",5,1,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_5_1_le("Linear::Bool::Int::Unit::Neg::Le::5::1",5,1,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_5_1_gq("Linear::Bool::Int::Unit::Neg::Gq::5::1",5,1,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_5_1_gr("Linear::Bool::Int::Unit::Neg::Gr::5::1",5,1,IRT_GR);

LinearBoolIntUnitNeg _lb_i_un_5_3_eq("Linear::Bool::Int::Unit::Neg::Eq::5::3",5,3,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_5_3_nq("Linear::Bool::Int::Unit::Neg::Nq::5::3",5,3,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_5_3_lq("Linear::Bool::Int::Unit::Neg::Lq::5::3",5,3,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_5_3_le("Linear::Bool::Int::Unit::Neg::Le::5::3",5,3,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_5_3_gq("Linear::Bool::Int::Unit::Neg::Gq::5::3",5,3,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_5_3_gr("Linear::Bool::Int::Unit::Neg::Gr::5::3",5,3,IRT_GR);

LinearBoolIntUnitNeg _lb_i_un_10_3_eq("Linear::Bool::Int::Unit::Neg::Eq::10::3",10,3,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_10_3_nq("Linear::Bool::Int::Unit::Neg::Nq::10::3",10,3,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_10_3_lq("Linear::Bool::Int::Unit::Neg::Lq::10::3",10,3,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_10_3_le("Linear::Bool::Int::Unit::Neg::Le::10::3",10,3,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_10_3_gq("Linear::Bool::Int::Unit::Neg::Gq::10::3",10,3,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_10_3_gr("Linear::Bool::Int::Unit::Neg::Gr::10::3",10,3,IRT_GR);

LinearBoolIntUnitNeg _lb_i_un_10_5_eq("Linear::Bool::Int::Unit::Neg::Eq::10::5",10,5,IRT_EQ);
LinearBoolIntUnitNeg _lb_i_un_10_5_nq("Linear::Bool::Int::Unit::Neg::Nq::10::5",10,5,IRT_NQ);
LinearBoolIntUnitNeg _lb_i_un_10_5_lq("Linear::Bool::Int::Unit::Neg::Lq::10::5",10,5,IRT_LQ);
LinearBoolIntUnitNeg _lb_i_un_10_5_le("Linear::Bool::Int::Unit::Neg::Le::10::5",10,5,IRT_LE);
LinearBoolIntUnitNeg _lb_i_un_10_5_gq("Linear::Bool::Int::Unit::Neg::Gq::10::5",10,5,IRT_GQ);
LinearBoolIntUnitNeg _lb_i_un_10_5_gr("Linear::Bool::Int::Unit::Neg::Gr::10::5",10,5,IRT_GR);


IntSet dbv(0,5);

class LinearBoolVarUnitPos : public IntTest {
private:
  const IntRelType irt;
public:
  LinearBoolVarUnitPos(const char* t, int n, IntRelType irt0)
    : IntTest(t,n+1,dbv,false), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size()-1; i++)
      if ((x[i] != 0) && (x[i] != 1))
        return false;
    double m = 0;
    for (int i=0; i<x.size()-1; i++)
      m += x[i];
    return compare(m, irt, static_cast<double>(x[x.size()-1]));
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(x.size()-1);
    for (int i=x.size()-1; i--; )
      b[i] = link(home,x[i]);
    linear(home, b, irt, x[x.size()-1]);
  }
};

LinearBoolVarUnitPos _lb_v_up_1_eq("Linear::Bool::Var::Unit::Pos::Eq::1",1,IRT_EQ);
LinearBoolVarUnitPos _lb_v_up_1_nq("Linear::Bool::Var::Unit::Pos::Nq::1",1,IRT_NQ);
LinearBoolVarUnitPos _lb_v_up_1_lq("Linear::Bool::Var::Unit::Pos::Lq::1",1,IRT_LQ);
LinearBoolVarUnitPos _lb_v_up_1_le("Linear::Bool::Var::Unit::Pos::Le::1",1,IRT_LE);
LinearBoolVarUnitPos _lb_v_up_1_gq("Linear::Bool::Var::Unit::Pos::Gq::1",1,IRT_GQ);
LinearBoolVarUnitPos _lb_v_up_1_gr("Linear::Bool::Var::Unit::Pos::Gr::1",1,IRT_GR);

LinearBoolVarUnitPos _lb_v_up_3_eq("Linear::Bool::Var::Unit::Pos::Eq::3",3,IRT_EQ);
LinearBoolVarUnitPos _lb_v_up_3_nq("Linear::Bool::Var::Unit::Pos::Nq::3",3,IRT_NQ);
LinearBoolVarUnitPos _lb_v_up_3_lq("Linear::Bool::Var::Unit::Pos::Lq::3",3,IRT_LQ);
LinearBoolVarUnitPos _lb_v_up_3_le("Linear::Bool::Var::Unit::Pos::Le::3",3,IRT_LE);
LinearBoolVarUnitPos _lb_v_up_3_gq("Linear::Bool::Var::Unit::Pos::Gq::3",3,IRT_GQ);
LinearBoolVarUnitPos _lb_v_up_3_gr("Linear::Bool::Var::Unit::Pos::Gr::3",3,IRT_GR);

LinearBoolVarUnitPos _lb_v_up_5_eq("Linear::Bool::Var::Unit::Pos::Eq::5",5,IRT_EQ);
LinearBoolVarUnitPos _lb_v_up_5_nq("Linear::Bool::Var::Unit::Pos::Nq::5",5,IRT_NQ);
LinearBoolVarUnitPos _lb_v_up_5_lq("Linear::Bool::Var::Unit::Pos::Lq::5",5,IRT_LQ);
LinearBoolVarUnitPos _lb_v_up_5_le("Linear::Bool::Var::Unit::Pos::Le::5",5,IRT_LE);
LinearBoolVarUnitPos _lb_v_up_5_gq("Linear::Bool::Var::Unit::Pos::Gq::5",5,IRT_GQ);
LinearBoolVarUnitPos _lb_v_up_5_gr("Linear::Bool::Var::Unit::Pos::Gr::5",5,IRT_GR);


class LinearBoolVarUnitNeg : public IntTest {
private:
  const IntRelType irt;
public:
  LinearBoolVarUnitNeg(const char* t, int n, IntRelType irt0)
    : IntTest(t,n+1,dbv,false), irt(irt0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size()-1; i++)
      if ((x[i] != 0) && (x[i] != 1))
        return false;
    double m = 0;
    for (int i=0; i<x.size()-1; i++)
      m -= x[i];
    return compare(m, irt, -static_cast<double>(x[x.size()-1]));
  }
  virtual void post(Space* home, IntVarArray& x) {
    BoolVarArgs b(x.size()-1);
    for (int i=x.size()-1; i--; )
      b[i] = link(home,x[i]);
    IntArgs c(x.size()-1);
    for (int i=x.size()-1; i--; )
      c[i]=-1;
    linear(home, c, b, irt, x[x.size()-1]);
  }
};

LinearBoolVarUnitNeg _lb_v_un_1_eq("Linear::Bool::Var::Unit::Neg::Eq::1",1,IRT_EQ);
LinearBoolVarUnitNeg _lb_v_un_1_nq("Linear::Bool::Var::Unit::Neg::Nq::1",1,IRT_NQ);
LinearBoolVarUnitNeg _lb_v_un_1_lq("Linear::Bool::Var::Unit::Neg::Lq::1",1,IRT_LQ);
LinearBoolVarUnitNeg _lb_v_un_1_le("Linear::Bool::Var::Unit::Neg::Le::1",1,IRT_LE);
LinearBoolVarUnitNeg _lb_v_un_1_gq("Linear::Bool::Var::Unit::Neg::Gq::1",1,IRT_GQ);
LinearBoolVarUnitNeg _lb_v_un_1_gr("Linear::Bool::Var::Unit::Neg::Gr::1",1,IRT_GR);

LinearBoolVarUnitNeg _lb_v_un_3_eq("Linear::Bool::Var::Unit::Neg::Eq::3",3,IRT_EQ);
LinearBoolVarUnitNeg _lb_v_un_3_nq("Linear::Bool::Var::Unit::Neg::Nq::3",3,IRT_NQ);
LinearBoolVarUnitNeg _lb_v_un_3_lq("Linear::Bool::Var::Unit::Neg::Lq::3",3,IRT_LQ);
LinearBoolVarUnitNeg _lb_v_un_3_le("Linear::Bool::Var::Unit::Neg::Le::3",3,IRT_LE);
LinearBoolVarUnitNeg _lb_v_un_3_gq("Linear::Bool::Var::Unit::Neg::Gq::3",3,IRT_GQ);
LinearBoolVarUnitNeg _lb_v_un_3_gr("Linear::Bool::Var::Unit::Neg::Gr::3",3,IRT_GR);

LinearBoolVarUnitNeg _lb_v_un_5_eq("Linear::Bool::Var::Unit::Neg::Eq::5",5,IRT_EQ);
LinearBoolVarUnitNeg _lb_v_un_5_nq("Linear::Bool::Var::Unit::Neg::Nq::5",5,IRT_NQ);
LinearBoolVarUnitNeg _lb_v_un_5_lq("Linear::Bool::Var::Unit::Neg::Lq::5",5,IRT_LQ);
LinearBoolVarUnitNeg _lb_v_un_5_le("Linear::Bool::Var::Unit::Neg::Le::5",5,IRT_LE);
LinearBoolVarUnitNeg _lb_v_un_5_gq("Linear::Bool::Var::Unit::Neg::Gq::5",5,IRT_GQ);
LinearBoolVarUnitNeg _lb_v_un_5_gr("Linear::Bool::Var::Unit::Neg::Gr::5",5,IRT_GR);


class LinearBoolScaleInt : public IntTest {
private:
  const IntRelType irt;
  int* a;
  int c;
public:
  LinearBoolScaleInt(const char* t, 
                     int* a0, int n, IntRelType irt0, int c0)
    : IntTest(t,n,db,false), irt(irt0), a(a0), c(c0) {}
  virtual bool solution(const Assignment& x) const {
    double m = 0;
    for (int i=0; i<x.size(); i++)
      m += a[i]*x[i];
    return compare(m, irt, static_cast<double>(c));
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ia(x.size());
    BoolVarArgs b(x.size());
    for (int i=x.size(); i--; ) {
      b[i] = link(home,x[i]); ia[i] = a[i];
    }
    linear(home, ia, b, irt, c);
  }
};

static int _lbsi_a_51[5] = {1,2,3,4,5};
static int _lbsi_b_51[5] = {-1,-2,-3,-4,-5};
static int _lbsi_c_51[5] = {-1,-2,1,2,4};

class LBSIC {
public:
  void a(char* s, const char* a) {
    while (*s != 0) 
      s++;
    while (*a != 0) {
      *s=*a; s++; a++;
    }
    *s = 0;
  }
  LBSIC(void) {
    for (int r=IRT_EQ; r<=IRT_GR;r++)
      for (int i=0; i<16; i++)
        for (int j=0; j<3; j++) {
          char* t = new char[256];
          t[0]=0;
          a(t,"Linear::Bool::Scale::Int::");
          switch (static_cast<IntRelType>(r)) {
          case IRT_EQ: a(t,"Eq"); break;
          case IRT_NQ: a(t,"Nq"); break;
          case IRT_LQ: a(t,"Lq"); break;
          case IRT_LE: a(t,"Le"); break;
          case IRT_GR: a(t,"Gr"); break;
          case IRT_GQ: a(t,"Gq"); break;
          default: ;
          }
          a(t,"::");
          char sj[2]; sj[0] = 'A'+j; sj[1] = 0;
          a(t,sj);
          a(t,"::");
          char si[2]; si[0] = 'A'+i; si[1] = 0;
          a(t,si);
          switch (j) {
          case 0:
            (void) new LinearBoolScaleInt(t,_lbsi_a_51,5,
                                          static_cast<IntRelType>(r),i);
            break;
          case 1:
            (void) new LinearBoolScaleInt(t,_lbsi_b_51,5,
                                          static_cast<IntRelType>(r),i);
            break;
          case 2:
            (void) new LinearBoolScaleInt(t,_lbsi_c_51,5,
                                          static_cast<IntRelType>(r),i);
            break;
          }
        }
  }
};

static LBSIC _lbsic;

class LinearBoolScaleView : public IntTest {
private:
  const IntRelType irt;
  int* a;
public:
  LinearBoolScaleView(const char* t, 
                     int* a0, int n, const IntSet& is, IntRelType irt0)
    : IntTest(t,n+1,is,false), irt(irt0), a(a0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size()-1; i++)
      if ((x[i] < 0) || (x[i] > 1))
        return false;
    double m = 0;
    for (int i=0; i<x.size()-1; i++)
      m += a[i]*x[i];
    return compare(m, irt, static_cast<double>(x[x.size()-1]));
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs ia(x.size()-1);
    BoolVarArgs b(x.size()-1);
    for (int i=x.size()-1; i--; ) {
      b[i] = link(home,x[i]); ia[i] = a[i];
    }
    linear(home, ia, b, irt, x[x.size()-1]);
  }
};


static IntSet is_010(0,10);
static int a1234[4] = {1,2,3,4};
static IntSet is_100(-10,0);
static int b1234[4] = {-1,-2,-3,-4};
static IntSet is_33(-3,3);
static int a1212[4] = {-1,-2,1,2};
static int a123123[6] = {-1,-2,1,2,-3,3};

LinearBoolScaleView _lbsv_4_1_eq("Linear::Bool::Scale::View::Eq::1",
                                 a1234,4,is_010,IRT_EQ);
LinearBoolScaleView _lbsv_4_2_eq("Linear::Bool::Scale::View::Eq::2",
                                 b1234,4,is_010,IRT_EQ);
LinearBoolScaleView _lbsv_4_3_eq("Linear::Bool::Scale::View::Eq::3",
                                 a1234,4,is_100,IRT_EQ);
LinearBoolScaleView _lbsv_4_4_eq("Linear::Bool::Scale::View::Eq::4",
                                 b1234,4,is_100,IRT_EQ);
LinearBoolScaleView _lbsv_4_5_eq("Linear::Bool::Scale::View::Eq::5",
                                 a1212,4,is_33,IRT_EQ);
LinearBoolScaleView _lbsv_4_6_eq("Linear::Bool::Scale::View::Eq::6",
                                 a123123,6,is_33,IRT_EQ);

LinearBoolScaleView _lbsv_4_1_lq("Linear::Bool::Scale::View::Lq::1",
                                 a1234,4,is_010,IRT_LQ);
LinearBoolScaleView _lbsv_4_2_lq("Linear::Bool::Scale::View::Lq::2",
                                 b1234,4,is_010,IRT_LQ);
LinearBoolScaleView _lbsv_4_3_lq("Linear::Bool::Scale::View::Lq::3",
                                 a1234,4,is_100,IRT_LQ);
LinearBoolScaleView _lbsv_4_4_lq("Linear::Bool::Scale::View::Lq::4",
                                 b1234,4,is_100,IRT_LQ);
LinearBoolScaleView _lbsv_4_5_lq("Linear::Bool::Scale::View::Lq::5",
                                 a1212,4,is_33,IRT_LQ);
LinearBoolScaleView _lbsv_4_6_lq("Linear::Bool::Scale::View::Lq::6",
                                 a123123,6,is_33,IRT_LQ);

LinearBoolScaleView _lbsv_4_1_le("Linear::Bool::Scale::View::Le::1",
                                 a1234,4,is_010,IRT_LE);
LinearBoolScaleView _lbsv_4_2_le("Linear::Bool::Scale::View::Le::2",
                                 b1234,4,is_010,IRT_LE);
LinearBoolScaleView _lbsv_4_3_le("Linear::Bool::Scale::View::Le::3",
                                 a1234,4,is_100,IRT_LE);
LinearBoolScaleView _lbsv_4_4_le("Linear::Bool::Scale::View::Le::4",
                                 b1234,4,is_100,IRT_LE);
LinearBoolScaleView _lbsv_4_5_le("Linear::Bool::Scale::View::Le::5",
                                 a1212,4,is_33,IRT_LE);
LinearBoolScaleView _lbsv_4_6_le("Linear::Bool::Scale::View::Le::6",
                                 a123123,6,is_33,IRT_LE);

LinearBoolScaleView _lbsv_4_1_gr("Linear::Bool::Scale::View::Gr::1",
                                 a1234,4,is_010,IRT_GR);
LinearBoolScaleView _lbsv_4_2_gr("Linear::Bool::Scale::View::Gr::2",
                                 b1234,4,is_010,IRT_GR);
LinearBoolScaleView _lbsv_4_3_gr("Linear::Bool::Scale::View::Gr::3",
                                 a1234,4,is_100,IRT_GR);
LinearBoolScaleView _lbsv_4_4_gr("Linear::Bool::Scale::View::Gr::4",
                                 b1234,4,is_100,IRT_GR);
LinearBoolScaleView _lbsv_4_5_gr("Linear::Bool::Scale::View::Gr::5",
                                 a1212,4,is_33,IRT_GR);
LinearBoolScaleView _lbsv_4_6_gr("Linear::Bool::Scale::View::Gr::6",
                                 a123123,6,is_33,IRT_GR);

LinearBoolScaleView _lbsv_4_1_gq("Linear::Bool::Scale::View::Gq::1",
                                 a1234,4,is_010,IRT_GQ);
LinearBoolScaleView _lbsv_4_2_gq("Linear::Bool::Scale::View::Gq::2",
                                 b1234,4,is_010,IRT_GQ);
LinearBoolScaleView _lbsv_4_3_gq("Linear::Bool::Scale::View::Gq::3",
                                 a1234,4,is_100,IRT_GQ);
LinearBoolScaleView _lbsv_4_4_gq("Linear::Bool::Scale::View::Gq::4",
                                 b1234,4,is_100,IRT_GQ);
LinearBoolScaleView _lbsv_4_5_gq("Linear::Bool::Scale::View::Gq::5",
                                 a1212,4,is_33,IRT_GQ);
LinearBoolScaleView _lbsv_4_6_gq("Linear::Bool::Scale::View::Gq::6",
                                 a123123,6,is_33,IRT_GQ);

LinearBoolScaleView _lbsv_4_1_nq("Linear::Bool::Scale::View::Nq::1",
                                 a1234,4,is_010,IRT_NQ);
LinearBoolScaleView _lbsv_4_2_nq("Linear::Bool::Scale::View::Nq::2",
                                 b1234,4,is_010,IRT_NQ);
LinearBoolScaleView _lbsv_4_3_nq("Linear::Bool::Scale::View::Nq::3",
                                 a1234,4,is_100,IRT_NQ);
LinearBoolScaleView _lbsv_4_4_nq("Linear::Bool::Scale::View::Nq::4",
                                 b1234,4,is_100,IRT_NQ);
LinearBoolScaleView _lbsv_4_5_nq("Linear::Bool::Scale::View::Nq::5",
                                 a1212,4,is_33,IRT_NQ);
LinearBoolScaleView _lbsv_4_6_nq("Linear::Bool::Scale::View::Nq::6",
                                 a123123,6,is_33,IRT_NQ);



// STATISTICS: test-int

