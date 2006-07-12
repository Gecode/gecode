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
 * \brief %Example: Failure stress test
 *
 * \ingroup Example
 *
 */
class StressFailure : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressFailure(const Options& opt)
    : x(this,2,0,opt.size) {

    rel(this, x[0], IRT_LE, x[1]);
    rel(this, x[1], IRT_LE, x[0]);

  }

  /// Constructor for cloning \a s
  StressFailure(bool share, StressFailure& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressFailure(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {}
};

/** \brief Main-function
 *  \relates StressFailure
 */
int
main(int argc, char** argv) {
  Options opt("StressFailure");
  opt.iterations = 20;
  opt.size       = 1000000;
  opt.c_d        = 5;
  opt.parse(argc,argv);
  Example::run<StressFailure,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

