/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

#include "examples/support.hh"
#include <gecode/minimodel.hh>

#include <iomanip>

/**
 * \brief %Example: Finding optimal %Golomb rulers
 *
 * The script makes use of two lower bounds taken from:
 *   Barbara Smith, Kostas Stergiou, Toby Walsh,
 *   Modelling the Golomb Ruler Problem.
 *   In IJCAI 99 Workshop on Non-binary Constraints,
 *   1999.
 *
 * See also problem 6 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 *
 */
class GolombRuler : public MinimizeExample {
protected:
  /// Number of marks
  const int n;
  /// Array for ruler marks
  IntVarArray m;
public:
  /// Model variants
  enum {
    MODEL_NONE, ///< No lower bound
    MODEL_SUM,  ///< Use sum of ticks as lower bound
    MODEL_RULER ///< Use size of smaller rulers as lower bound
  };
  /// Search variants
  enum {
    SEARCH_DFS,    ///< Use depth first search to find the smallest tick
    SEARCH_BAB,    ///< Use branch and bound to optimize
    SEARCH_RESTART ///< Use restart to optimize
  };
  /// Return index for mark difference between mark \a i and mark \a j
  int
  diag(int i, int j) {
    return (i*(2*n-i-1)) / 2 + j - i - 1;
  }

  /// Actual model
  GolombRuler(const SizeOptions& opt)
    : n(opt.size()), m(*this,n,0,n*n) {
    const int dn = (n*n-n)/2;

    IntVarArgs d(dn);

    // Assume first mark to be zero
    rel(*this, m[0], IRT_EQ, 0);

    // Setup difference constraints
    for (int j=1; j<n; j++)
      d[diag(0,j)] = m[j];
    for (int i=1; i<n-1; i++)
      for (int j=i+1; j<n; j++)
        d[diag(i,j)] = minus(*this, m[j], m[i]);

    // Order marks
    rel(*this, m, IRT_LE);

    switch (opt.model()) {
    case MODEL_NONE:
      break;
    case MODEL_SUM:
      // d[diag(i,j)] must be at least sum of first j-i integers
      for (int i=0; i<n; i++)
        for (int j=i+1; j<n; j++)
          rel(*this, d[diag(i,j)], IRT_GQ, (j-i)*(j-i+1)/2);
      break;
    case MODEL_RULER:
      {
        static const int length[] = {
          // Length 0-9
          0,0,1,3,6,11,17,25,34,44,
          // Length 10-
          55,72,85,106,127};
        // Marks from i to j must be ruler of length j-1+i
        for (int i=0; i<n; i++)
          for (int j=i+1; j<n; j++)
            rel(*this, d[diag(i,j)], IRT_GQ, length[j-i+1]);
      }
      break;
    }

    distinct(*this, d, opt.icl());

    if (n > 2)
      rel(*this, d[diag(0,1)], IRT_LE, d[diag(n-2,n-1)]);

    if (opt.search() == SEARCH_DFS) {
      IntVarArgs max(1);
      max[0]=m[n-1];
      branch(*this, max, INT_VAR_NONE, INT_VAL_SPLIT_MIN);
    }

    branch(*this, m, INT_VAR_NONE, INT_VAL_MIN);
  }

  /// Return cost
  virtual IntVar cost(void) const {
    return m[n-1];
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\tm[" << n << "] = " << m << std::endl;
  }

  /// Constructor for cloning \a s
  GolombRuler(bool share, GolombRuler& s)
    : MinimizeExample(share,s), n(s.n) {
    m.update(*this, share, s.m);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new GolombRuler(share,*this);
  }
};

/** \brief Main-function
 *  \relates GolombRuler
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("GolombRuler");
  opt.solutions(0);
  opt.size(10);
  opt.icl(ICL_BND);
  opt.model(GolombRuler::MODEL_SUM);
  opt.model(GolombRuler::MODEL_NONE, "none",
            "no lower bound");
  opt.model(GolombRuler::MODEL_SUM, "sum",
            "use sum of ticks as lower bound");
  opt.model(GolombRuler::MODEL_RULER, "ruler",
            "use size of smaller rulers as lower bound");
  opt.search(GolombRuler::SEARCH_BAB);
  opt.search(GolombRuler::SEARCH_DFS, "dfs");
  opt.search(GolombRuler::SEARCH_BAB, "bab");
  opt.search(GolombRuler::SEARCH_RESTART, "restart");
  opt.parse(argc,argv);
  if (opt.size() > 0)
    switch (opt.search()) {
    case GolombRuler::SEARCH_DFS:
      MinimizeExample::run<GolombRuler,DFS,SizeOptions>(opt); break;
    case GolombRuler::SEARCH_BAB:
      MinimizeExample::run<GolombRuler,BAB,SizeOptions>(opt); break;
    case GolombRuler::SEARCH_RESTART:
      MinimizeExample::run<GolombRuler,Restart,SizeOptions>(opt); break;
    }
  return 0;
}

// STATISTICS: example-any

