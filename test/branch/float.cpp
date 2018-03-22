/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
 *     Vincent Barichard, 2012
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

#ifdef GECODE_HAS_FLOAT_VARS

namespace Test { namespace Branch {

  /// %Test brancher with distinct propagator
  class Float : public FloatTest {
  public:
    /// Create and register test
    Float(const std::string& s, const Gecode::FloatVal& d, int n, int nbSols)
      : FloatTest(s,n,d,nbSols) {}
    /// Post propagators on variables \a x
    virtual void post(Gecode::Space& home, Gecode::FloatVarArray& x) {
      for (int i=0; i<x.size()-1; i++)
        Gecode::rel(home, x[i], Gecode::FRT_LQ, x[i+1]);
    }
  };

  namespace {
    Gecode::FloatVal d_dense(-2,2);
    Gecode::FloatVal d_large(-2,10);

    Float d_3("Dense::3",d_dense,3,10);
    Float d_5("Dense::5",d_dense,5,10);
    Float l_2("Large::2",d_large,2,10);
    Float l_3("Large::3",d_large,3,10);
  }

}}

#endif

// STATISTICS: test-branch

