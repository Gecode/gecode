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

/**
 * \brief %Example: Execution stress test
 *
 * The size argument defines how many duplicate propagators
 * are created.
 *
 * \ingroup ExStress
 *
 */
class StressExec : public Example {
protected:
  /// Variables
  IntVarArray x;
  /// Initial domain size
  static const int n = 1000000;
public:
  /// The actual problem
  StressExec(const Options& opt)
    : x(this,2,0,n) {

    for (unsigned int i=0; i<opt.size; i++) {
      rel(this, x[0], IRT_LE, x[1]);
      rel(this, x[1], IRT_LE, x[0]);
    }

  }

  /// Constructor for cloning \a s
  StressExec(bool share, StressExec& s) : Example(share,s) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressExec(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {}
};

/** \brief Main-function
 *  \relates StressExec
 */
int
main(int argc, char** argv) {
  Options opt("StressExec");
  opt.iterations = 20;
  opt.size       = 1;
  opt.parse(argc,argv);
  Example::run<StressExec,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

