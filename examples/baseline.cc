/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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
 * \brief %Example: Baseline test
 *
 * \ingroup Example
 *
 */
class Baseline : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  Baseline(const Options& opt)
    : x(this,1,0,1) {
    branch(this, x, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  Baseline(bool share, Baseline& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Baseline(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {}
};

/** \brief Main-function
 *  \relates Baseline
 */
int
main(int argc, char** argv) {
  Options opt("Baseline");
  opt.iterations = 2000;
  opt.solutions  = 0;
  opt.parse(argc,argv);
  Example::run<Baseline,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

