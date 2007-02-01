/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

static const int n = 15;
static const int m = 90;
static const int p[n] = {16,35,90,42,88,6,40,42,64,48,46,5,90,29,70};

/**
 * \brief %Example: Stress test for element constraint (involving integers)
 *
 * This stress test is an adaption of the benchmark for element posted
 * by Neng-Fa Zhou for B-Prolog, November, 2005.
 *
 * \ingroup ExStress
 *
 */
class StressElement : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressElement(const Options& opt)
    : x(this,n,0,n-1) {

    IntVarArgs s(n);
    for (int i=0; i<n; i++) {
      IntVar si(this,0,m); s[i]=si;
    }

    for (int i=0; i<n-1; i++)
      rel(this, s[i], IRT_LQ, s[i+1]);

    IntArgs e(n,p);

    for (int i=0; i<n; i++)
      element(this, e, x[i], s[i]);

    distinct(this, x);
    branch(this, x, BVAR_SIZE_MIN, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  StressElement(bool share, StressElement& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressElement(share,*this);
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
 *  \relates StressElement
 */
int
main(int argc, char** argv) {
  Options opt("StressElement");
  opt.parse(argc,argv);
  Example::run<StressElement,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

