/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

namespace {
  IntSet ds_dense(-2,2);
  const int v_sparse[5] = {-100,-10,0,10,100};
  IntSet ds_sparse(v_sparse,5);
  IntSet ds_rn(-500,500);
  IntSet ds_rnsmall(-50,50);
}

class Distinct : public IntTest {
public:
  Distinct(const char* t, const IntSet& ds, IntConLevel icl)
    : IntTest(t,4,ds,false,icl) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<4; i++)
      for (int j=i+1; j<4; j++)
        if (x[i]==x[j])
          return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    distinct(home, x, icl);
  }
};

namespace {
  Distinct _dist_dom_d("Distinct::Dom::Dense",ds_dense,ICL_DOM);
  Distinct _dist_bnd_d("Distinct::Bnd::Dense",ds_dense,ICL_BND);
  Distinct _dist_val_d("Distinct::Val::Dense",ds_dense,ICL_VAL);
  Distinct _dist_dom_s("Distinct::Dom::Sparse",ds_sparse,ICL_DOM);
  Distinct _dist_bnd_s("Distinct::Bnd::Sparse",ds_sparse,ICL_BND);
  Distinct _dist_val_s("Distinct::Val::Sparse",ds_sparse,ICL_VAL);
}

class DistinctRandom : public IntTest {
private:
  IntConLevel icl;
  int size;
  IntSet ds;
protected:
  virtual Assignment* make_assignment() {
    return new RandomAssignment(size, ds, 100);
  }
  virtual bool do_search_test() { return false; }
public:
  DistinctRandom(const char* t, int size0, const IntSet& ds0, IntConLevel icl0)
    : IntTest(t,size0,ds0,false),
      icl(icl0), size(size0), ds(ds0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<x.size(); i++)
      for (int j=i+1; j<x.size(); j++)
        if (x[i]==x[j])
          return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    distinct(home, x, icl);
  }
};

namespace {
  DistinctRandom _domr("Distinct::Dom::Random", 20, ds_rnsmall, ICL_DOM);
  DistinctRandom _bndr("Distinct::Bnd::Random", 50, ds_rn, ICL_BND);
  DistinctRandom _valr("Distinct::Val::Random", 50, ds_rn, ICL_VAL);
}

class DistinctOffset : public IntTest {
public:
  DistinctOffset(const char* t, const IntSet& ds, IntConLevel icl)
    : IntTest(t,4,ds,false,icl) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<4; i++)
      for (int j=i+1; j<4; j++)
        if (x[i]+i==x[j]+j)
          return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs c(4);
    for (int i=0; i<4; i++)
      c[i]=i;
    distinct(home, c, x, icl);
  }
};

namespace {
  DistinctOffset _disto_dom_d("Distinct::Dom::Offset::Dense",ds_dense,ICL_DOM);
  DistinctOffset _disto_bnd_d("Distinct::Bnd::Offset::Dense",ds_dense,ICL_BND);
  DistinctOffset _disto_val_d("Distinct::Val::Offset::Dense",ds_dense,ICL_VAL);
  DistinctOffset _disto_dom_s("Distinct::Dom::Offset::Sparse",ds_sparse,ICL_DOM);
  DistinctOffset _disto_bnd_s("Distinct::Bnd::Offset::Sparse",ds_sparse,ICL_BND);
  DistinctOffset _disto_val_s("Distinct::Val::Offset::Sparse",ds_sparse,ICL_VAL);
}


// STATISTICS: test-int

