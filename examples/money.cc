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
#include "gecode/minimodel.hh"

/**
 * \brief %Example: SEND+MORE=MONEY puzzle
 *
 * Well-known cryptoarithmetic puzzle.
 * Henry Dudeney, Strand Magazine, July 1924.
 *
 * \ingroup ExProblem
 *
 */
class Money : public Example {
protected:
  /// Number of letters
  static const int nl = 8;
  /// Array of letters
  IntVarArray le;
public:
  /// Actual model
  Money(const Options& opt) : le(this,nl,0,9) {
    IntVar
      s(le[0]), e(le[1]), n(le[2]), d(le[3]),
      m(le[4]), o(le[5]), r(le[6]), y(le[7]);

    rel(this, s, IRT_NQ, 0);
    rel(this, m, IRT_NQ, 0);

    post(this,            1000*s+100*e+10*n+d
                +         1000*m+100*o+10*r+e
               == 10000*m+1000*o+100*n+10*e+y,
         opt.icl());

    distinct(this, le, opt.icl());
    branch(this, le, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }
  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < nl; i++)
      std::cout << le[i] << " ";
    std::cout << std::endl;
  }

  /// Constructor for cloning \a s
  Money(bool share, Money& s) : Example(share,s) {
    le.update(this, share, s.le);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Money(share,*this);
  }
};

/** \brief Main-function
 *  \relates Money
 */
int
main(int argc, char* argv[]) {
  Options opt("SEND+MORE=MONEY");
  opt.solutions(0);
  opt.iterations(20000);
  opt.parse(argc,argv);
  Example::run<Money,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

