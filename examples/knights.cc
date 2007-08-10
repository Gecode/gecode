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
#include "gecode/minimodel.hh"

/// Base-class for Knights tour example
class Knights : public Example {
protected:
  /// Size of board
  const int n;
  /// Maps board field to successor field
  IntVarArray succ;
public:
  /// Propagation to use for model
  enum {
    PROP_REIFIED, ///< Use reified constraints
    PROP_CIRCUIT  ///< Use single circuit constraints
  };
  /// Return field at position \a x, \a y
  int
  field(int x, int y) {
    return x*n+y;
  }
  /// Compute array of neighbours
  void
  neighbours(int f, int nbs[8], int& n_nbs) {
    n_nbs=0;
    int x = f / n;
    int y = f % n;
    for (int i=0;i<8;i++) {
      static const int moves[8][2] = {
        {-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}
      };
      int nx=x+moves[i][0];
      int ny=y+moves[i][1];
      if ((nx >= 0) && (nx < n) && (ny >= 0) && (ny < n))
        nbs[n_nbs++]=field(nx,ny);
    }
  }
  /// Constructor
  Knights(const Options& opt)
    : n(opt.size), succ(this,n*n,0,n*n-1) {}
  /// Constructor for cloning \a s
  Knights(bool share, Knights& s) : Example(share,s), n(s.n) {
    succ.update(this, share, s.succ);
  }
  /// Print board
  virtual void
  print(void) {
    int* jump = new int[n*n];
    {
      int j=0;
      for (int i=0; i<n*n; i++) {
        jump[j]=i; j=succ[j].min();
      }
    }
    std::cout << "\t";
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        std::cout.width(3);
        std::cout << jump[field(i,j)] << " ";
        }
        std::cout << std::endl << "\t";
    }
    std::cout << std::endl;
    delete [] jump;
  }
};

/**
 * \brief %Example: n-%Knights tour (Simple Model)
 *
 * Fill an n times n chess board with knights such that the
 * knights do a full tour by knights move (last knight reaches
 * first knight again). The formulation is due to Gert Smolka.
 *
 * \ingroup ExProblem
 *
 */
class KnightsReified : public Knights {
public:
  KnightsReified(const Options& opt) : Knights(opt) {
    const int nn = n*n;

    // Map knight to its predecessor of succesor on board
    IntVarArgs jump(nn);
    IntVarArgs pred(nn);

    for (int i = nn; i--; ) {
      IntVar p(this,0,nn-1); pred[i]=p;
      IntVar j(this,0,nn-1); jump[i]=j;
    }

    // Place the first two knights
    rel(this, jump[field(0,0)], IRT_EQ, 0);
    rel(this, jump[field(1,2)], IRT_EQ, 1);

    distinct(this, jump, opt.icl());
    channel(this, succ, pred, opt.icl());

    for (int f = 0; f < nn; f++) {
      // Array of neighbours
      int nbs[8]; int n_nbs = 0;
      neighbours(f, nbs, n_nbs);
      for (int i=n_nbs; i--; )
        rel(this,
            post(this, ~(jump[nbs[i]]-jump[f] == 1)),
            BOT_XOR,
            post(this, ~(jump[nbs[i]]-jump[f] == 1-nn)),
            post(this, ~(succ[f] == nbs[i])));

      IntSet ds(nbs, n_nbs);
      dom(this, pred[f], ds);
      dom(this, succ[f], ds);
      rel(this, succ[f], IRT_NQ, pred[f]);
    }
    branch(this, succ, BVAR_NONE, BVAL_MIN);
  }
  /// Constructor for cloning \a s
  KnightsReified(bool share, KnightsReified& s) : Knights(share,s) {}
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new KnightsReified(share,*this);
  }
};

/**
 * \brief %Example: n-%Knights tour (Model using Circuit)
 *
 * Fill an n times n chess board with knights such that the
 * knights do a full tour by knights move (last knight reaches
 * first knight again).
 *
 * \ingroup ExProblem
 *
 */
class KnightsCircuit : public Knights {
public:
  KnightsCircuit(const Options& opt) : Knights(opt) {
    // Fix the first move
    rel(this, succ[0], IRT_EQ, field(1,2));

    circuit(this, succ, opt.icl());

    for (int f = 0; f < n*n; f++) {
      // Array of neighbours
      int nbs[8]; int n_nbs = 0;
      neighbours(f, nbs, n_nbs);
      IntSet ds(nbs, n_nbs);
      dom(this, succ[f], ds);
    }
    branch(this, succ, BVAR_NONE, BVAL_MIN);
  }
  /// Constructor for cloning \a s
  KnightsCircuit(bool share, KnightsCircuit& s) : Knights(share,s) {}
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new KnightsCircuit(share,*this);
  }
};

/** \brief Main-function
 *  \relates Knights
 */
int
main(int argc, char** argv) {
  Options opt("Knights");
  opt.iterations = 100;
  opt.size       = 8;
  opt.propagation(Knights::PROP_CIRCUIT);
  opt.propagation(Knights::PROP_REIFIED, "reified");
  opt.propagation(Knights::PROP_CIRCUIT, "circuit");
  opt.parse(argc,argv);
  if (opt.propagation() == Knights::PROP_REIFIED) {
    Example::run<KnightsReified,DFS>(opt);
  } else {
    Example::run<KnightsCircuit,DFS>(opt);
  }
  return 0;
}

// STATISTICS: example-any

