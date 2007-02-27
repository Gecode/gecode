/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
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
 * \brief %Example: Stress test for circuit constraint
 *
 * \ingroup ExStress
 *
 */
class StressCircuit : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressCircuit(const Options& opt)
    : x(this,opt.size,0,opt.size-1) {

    circuit(this, x, opt.icl);
    //	x[]={[3..5], [3..4], 0, 2, 1, [1..4], ;
    rel(this, x[0], IRT_NQ, 0);
    rel(this, x[0], IRT_NQ, 1);
    rel(this, x[0], IRT_NQ, 2);
    rel(this, x[1], IRT_NQ, 0);
    rel(this, x[1], IRT_NQ, 1);
    rel(this, x[1], IRT_NQ, 2);
    rel(this, x[1], IRT_NQ, 5);
    rel(this, x[2], IRT_EQ, 0);
    rel(this, x[3], IRT_EQ, 2);
    rel(this, x[4], IRT_EQ, 1);
    rel(this, x[5], IRT_NQ, 0);
    rel(this, x[5], IRT_NQ, 5);
    //    rel(this, x[1], IRT_EQ; 0);

    //    branch(this, x, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  StressCircuit(bool share, StressCircuit& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressCircuit(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tx: ";
    int i=0;
    do {
      std::cout << i << " -> ";
      i=x[i].val();
    } while (i != 0);
    std::cout << 0 << std::endl;
  }
};

/** \brief Main-function
 *  \relates StressCircuit
 */
int
main(int argc, char** argv) {
  Options opt("StressCircuit");
  opt.size = 6;
  opt.parse(argc,argv);
  Example::run<StressCircuit,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

