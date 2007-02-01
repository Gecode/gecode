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

#include "examples/support.hh"

/**
 * \brief %Example: Stress distinct propagator
 *
 * Performs propagation on a pathological example. The example is taken
 * from: Jean-Francois Puget, A fast algorithm for the bound consistency
 * of alldiff constraints, AAAI-98, pages 359-366, July 1998,
 * Madison, WI, USA.
 *
 * \ingroup ExStress
 */
class StressDistinct : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressDistinct(const Options& opt)
    : x(this,opt.size+1,0,opt.size) {
    distinct(this, x, opt.icl);
    for (int i=0; i<opt.size; i++)
      rel(this, x[i], IRT_LQ, i);
  }
  /// Constructor for cloning \a s
  StressDistinct(bool share, StressDistinct& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressDistinct(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    int n = x.size();
    std::cout << "\tx[" << n << "] = {";
    for (int i = 0; i < n; i++)
      std::cout << x[i] << ((i<n-1)?",":"};\n");
  }
};

/** \brief Main-function
 *  \relates StressDistinct
 */
int
main(int argc, char** argv) {
  Options opt("StressDistinct");
  opt.size = 1000;
  opt.parse(argc,argv);
  Example::run<StressDistinct,DFS>(opt);
  return 0;
}

// STATISTICS: example-any
