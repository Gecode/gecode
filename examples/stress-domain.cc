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
 * \brief %Example: Domain stress test
 *
 * \ingroup ExStress
 *
 */
class StressDomain : public Example {
protected:
  /// Variables
  IntVarArray x;
public:
  /// The actual problem
  StressDomain(const SizeOptions& opt)
    : x(this,5,0,5*opt.size()) {

    // Cut holes: expand
    for (int i = 5; i--; ) {
      for (unsigned int j = 0; j <= 5*opt.size(); j++)
        rel(this, x[i], IRT_NQ, 5*j);
      for (unsigned int j = 0; j <= 5*opt.size(); j++)
        rel(this, x[i], IRT_NQ, 5*j+2);
      for (unsigned int j = 0; j <= 5*opt.size(); j++)
        rel(this, x[i], IRT_NQ, 5*j+4);
    }
    // Contract
    for (unsigned int j = 0; j <= 5*opt.size()/2; j++)
      for (unsigned int i = 5; i--; ) {
        rel(this, x[i], IRT_GQ, 5*j);
        rel(this, x[i], IRT_LQ, 5*(j + (5*opt.size()/2)));
      }
  }
  /// Constructor for cloning \a s
  StressDomain(bool share, StressDomain& s) : Example(share,s) {
    x.update(this, share, s.x);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new StressDomain(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream&) {}
};

/** \brief Main-function
 *  \relates StressDomain
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("StressDomain");
  opt.iterations(200);
  opt.size(1000);
  opt.parse(argc,argv);
  Example::run<StressDomain,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

