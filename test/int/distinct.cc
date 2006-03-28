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

static IntSet ds_dense(-2,2);
static const int v_sparse[5] = {-100,-10,0,10,100};
static IntSet ds_sparse(v_sparse,5);

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

Distinct _dist_dom_d("Distinct::Dom::Dense",ds_dense,ICL_DOM);
Distinct _dist_bnd_d("Distinct::Bnd::Dense",ds_dense,ICL_BND);
Distinct _dist_val_d("Distinct::Val::Dense",ds_dense,ICL_VAL);
Distinct _dist_dom_s("Distinct::Dom::Sparse",ds_sparse,ICL_DOM);
Distinct _dist_bnd_s("Distinct::Bnd::Sparse",ds_sparse,ICL_BND);
Distinct _dist_val_s("Distinct::Val::Sparse",ds_sparse,ICL_VAL);

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

DistinctOffset _disto_dom_d("Distinct::Dom::Offset::Dense",ds_dense,ICL_DOM);
DistinctOffset _disto_bnd_d("Distinct::Bnd::Offset::Dense",ds_dense,ICL_BND);
DistinctOffset _disto_val_d("Distinct::Val::Offset::Dense",ds_dense,ICL_VAL);
DistinctOffset _disto_dom_s("Distinct::Dom::Offset::Sparse",ds_sparse,ICL_DOM);
DistinctOffset _disto_bnd_s("Distinct::Bnd::Offset::Sparse",ds_sparse,ICL_BND);
DistinctOffset _disto_val_s("Distinct::Val::Offset::Sparse",ds_sparse,ICL_VAL);

// STATISTICS: test-int

