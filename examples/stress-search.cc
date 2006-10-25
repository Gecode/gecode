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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "examples/support.hh"

/**
 * \brief %Example: Search stress test
 *
 * \ingroup Example
 *
 */
class StressSearch : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressSearch(const Options& opt)
    : x(this,opt.size,0,opt.size) {
    branch(this, x, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  StressSearch(bool share, StressSearch& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressSearch(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {}
};

/** \brief Main-function
 *  \relates StressSearch
 */
int
main(int argc, char** argv) {
  Options opt("StressSearch");
  opt.iterations = 20;
  opt.size       = 6;
  opt.solutions  = 0;
  opt.parse(argc,argv);
  Example::run<StressSearch,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

