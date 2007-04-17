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
#include "gecode/support/dynamic-array.hh"

/**
 * \brief %Example: Stress linear Boolean inequality
 *
 * \ingroup ExStress
 */
class StressLinearBool : public Example {
protected:
  /// Variables
  BoolVarArray x;
public:
  /// Constructor for the model
  StressLinearBool(const Options& opt)
    : x(this,4*opt.size+1,0,1) {
    
    linear(this, x, IRT_GQ, 2*opt.size);

    for (int i=0; i<opt.size; i++) {
      // Assign a variable
      rel(this, x[opt.size-1-i], IRT_EQ, 0);
      // Propagate
      (void) status();
      // Assign a variable
      rel(this, x[opt.size+i], IRT_EQ, 0);
      // Propagate
      (void) status();
    }

  }
  /// Constructor for cloning \a s
  StressLinearBool(bool share, StressLinearBool& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressLinearBool(share,*this);
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
 *  \relates StressLinearBool
 */
int
main(int argc, char** argv) {
  Options opt("StressLinearBool");
  opt.size = 1000;
  opt.parse(argc,argv);
  Example::run<StressLinearBool,DFS>(opt);
  return 0;
}

// STATISTICS: example-any
