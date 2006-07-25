/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

#include "examples/support.hh"

/**
 * \brief %Example: Stress test for the minimum constraint
 *
 * \ingroup Example
 *
 */
class StressMin : public Example {
protected:
  /// Size of problem
  const int n;
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressMin(const Options& opt)
    : n(opt.size), x(this,n,0,2*n-1) {
    for (int i=1; i<n; i++) {
      IntVarArgs y(i);
      for (int j=0; j<i; j++)
	y[j]=x[j];
      IntVar m(this,0,2*n);
      min(this, y, m);
      rel(this, m, IRT_GR, x[i]);
    }
    branch(this, x, BVAR_NONE, BVAL_SPLIT_MAX);
  }

  /// Constructor for cloning \a s
  StressMin(bool share, StressMin& s) : Example(share,s), n(s.n) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressMin(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tx[" << n << "] = {";
    for (int i = 0; i < n; i++)
      std::cout << x[i] << ((i<n-1)?",":"};\n");
  }
};


/** \brief Main-function
 *  \relates StressMin
 */
int
main(int argc, char** argv) {
  Options opt("StressMin");
  opt.parse(argc,argv);
  opt.size = 200;
  Example::run<StressMin,DFS>(opt);
  return 0;
}

// STATISTICS: example-any
