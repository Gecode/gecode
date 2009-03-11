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
#include <gecode/minimodel.hh>

/**
 * \brief %Example: %Alpha puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup ExProblem
 *
 */
class Alpha : public Example {
protected:
  IntVarArray x;
public:
  /// Branching to use for model
  enum {
    BRANCH_NONE, ///< Choose variable left to right
    BRANCH_SIZE  ///< Choose variable with smallest size
  };
  /// Actual model
  Alpha(const Options& opt) : x(*this,4,0,3) {
    rel(*this, x[0], IRT_NQ, 0);
    rel(*this, x[1], IRT_NQ, 1);
    rel(*this, x[1], IRT_NQ, 3);
    rel(*this, x[2], IRT_NQ, 0);
    rel(*this, x[2], IRT_NQ, 1);
    rel(*this, x[3], IRT_NQ, 1);
    rel(*this, x[3], IRT_NQ, 3);
    std::cout << x << std::endl;
    circuit(*this, x);
    status();
    std::cout << x << std::endl;
  }

  /// Constructor for cloning \a s
  Alpha(bool share, Alpha& s) : Example(share,s) {
    x.update(*this, share, s.x);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Alpha(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << x << std::endl;
  }
};

/** \brief Main-function
 *  \relates Alpha
 */
int
main(int argc, char* argv[]) {
  Options opt("Alpha");
  opt.solutions(0);
  opt.iterations(10);
  opt.branching(Alpha::BRANCH_NONE);
  opt.branching(Alpha::BRANCH_NONE, "none");
  opt.branching(Alpha::BRANCH_SIZE, "size");
  opt.parse(argc,argv);
  Example::run<Alpha,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

