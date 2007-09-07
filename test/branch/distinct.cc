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

#include "test/branch.hh"

namespace Test {

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

}

// STATISTICS: test-branch

