/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2006
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/int/branch.hh>
#include <gecode/search.hh>

namespace Test { namespace Branch {

  /// Space for testing filtered random tie selection
  class FilteredRndTiesSpace : public Gecode::Space {
  public:
    /// Variables to branch on
    Gecode::IntVarArray x;

    /// Initialize space
    FilteredRndTiesSpace(Gecode::Rnd r)
      : x(*this, 4, 0, 1) {
      Gecode::IntVarArgs xa(x);
      Gecode::ViewArray<Gecode::Int::IntView> xv(*this, xa);
      Gecode::ViewSel<Gecode::Int::IntView>* vs[2] = {
        Gecode::Int::Branch::viewsel(*this, Gecode::INT_VAR_RND(r)),
        Gecode::Int::Branch::viewsel(*this, Gecode::INT_VAR_SIZE_MIN())
      };
      // Post directly so the random selector remains first in the tie chain.
      Gecode::postviewvalbrancher<Gecode::Int::IntView,2,int,2>(
        *this, xv, vs,
        Gecode::Int::Branch::valselcommit(*this, Gecode::INT_VAL_MIN()),
        [](const Gecode::Space&, Gecode::IntVar, int i) {
          return (i == 0) || (i == 2);
        }, nullptr);
    }

    /// Constructor for cloning
    FilteredRndTiesSpace(FilteredRndTiesSpace& s)
      : Gecode::Space(s) {
      x.update(*this, s.x);
    }

    /// Copy space during cloning
    virtual Gecode::Space* copy(void) {
      return new FilteredRndTiesSpace(*this);
    }
  };

  /// Test filtered random tie selection with accepted and rejected views
  class FilteredRndTies : public Base {
  public:
    /// Create and register test
    FilteredRndTies(void)
      : Base("Int::Branch::FilteredRndTies") {}

    /// Run test
    virtual bool run(void) {
      bool selected[2] = {false, false};

      for (unsigned int seed = 1; seed <= 16; seed++) {
        FilteredRndTiesSpace* probe =
          new FilteredRndTiesSpace(Gecode::Rnd(seed));
        if (probe->status() != Gecode::SS_BRANCH) {
          delete probe;
          return false;
        }
        const Gecode::Choice* c = probe->choice();
        const Gecode::PosChoice& pc
          = static_cast<const Gecode::PosChoice&>(*c);
        const int p = pc.pos().pos;
        delete c;
        delete probe;
        if ((p != 0) && (p != 2))
          return false;
        selected[p == 2] = true;

        FilteredRndTiesSpace* root =
          new FilteredRndTiesSpace(Gecode::Rnd(seed));
        Gecode::Search::Options o;
        o.c_d = 2;
        o.a_d = 1;
        Gecode::DFS<FilteredRndTiesSpace> e(root, o);
        delete root;

        int n = 0;
        while (Gecode::Space* s = e.next()) {
          FilteredRndTiesSpace* solution =
            static_cast<FilteredRndTiesSpace*>(s);
          if (solution->x[1].assigned() || solution->x[3].assigned()) {
            delete solution;
            return false;
          }
          delete solution;
          n++;
        }
        if (n != 4)
          return false;
      }

      return selected[0] && selected[1];
    }
  };

  FilteredRndTies filtered_rnd_ties;

  /// %Test brancher with distinct propagator
  class Int : public IntTest {
  public:
    /// Create and register test
    Int(const std::string& s, const Gecode::IntSet& d, int n)
      : IntTest(s,n,d) {}
    /// Post propagators on variables \a x
    virtual void post(Gecode::Space& home, Gecode::IntVarArray& x) {
      Gecode::distinct(home, x);
    }
  };

  Gecode::IntSet d_dense(-2,2);
  const int v_sparse[5] = {-100,-10,0,10,100};
  Gecode::IntSet d_sparse(v_sparse,5);

  Gecode::IntSet d_large(-2,10);

  Int d_3("Dense::3",d_dense,3);
  Int d_5("Dense::5",d_dense,5);
  Int s_3("Sparse::3",d_sparse,3);
  Int s_5("Sparse::5",d_sparse,5);
  Int l_2("Large::2",d_large,2);
  Int l_3("Large::3",d_large,3);

}}

// STATISTICS: test-branch
