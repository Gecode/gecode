/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
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

#include "test/branch.hh"

namespace {
  IntSet ds_dense(-2,2);
  const int v_sparse[5] = {-100,-10,0,10,100};
  IntSet ds_sparse(v_sparse,5);

  IntSet ds_large(-2,10);
}

class BDistinct : public BranchCompleteTest {
public:
  BDistinct(const char* t, const IntSet& ds, int size)
    : BranchCompleteTest(t,size,ds) {}
  virtual void post(Space* home, IntVarArray& x) {
    distinct(home, x, randicl());
  }
};

namespace {
  BDistinct _dist_d_3("Distinct::Dense::3",ds_dense,3);
  BDistinct _dist_d_5("Distinct::Dense::5",ds_dense,5);
  BDistinct _dist_s_3("Distinct::Sparse::3",ds_sparse,3);
  BDistinct _dist_s_5("Distinct::Sparse::5",ds_sparse,5);
  BDistinct _dist_l_2("Distinct::Large::2",ds_large,2);
  BDistinct _dist_l_3("Distinct::Large::3",ds_large,3);
}

// STATISTICS: test-branch

