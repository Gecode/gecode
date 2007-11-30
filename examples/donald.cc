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
 * \brief %Example: DONALD+GERALD=ROBERT puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup ExProblem
 *
 */
class Donald : public Example {
private:
  /// Number of letters
  static const int nl = 10;
  /// Array of letters
  IntVarArray le;
public:
  /// Actual model
  Donald(const Options& opt) :
    le(this,nl,0,9) {
    IntVar
      d(le[0]), o(le[1]), n(le[2]), a(le[3]), l(le[4]),
      g(le[5]), e(le[6]), r(le[7]), b(le[8]), t(le[9]);
    rel(this, d, IRT_NQ, 0);
    rel(this, g, IRT_NQ, 0);
    rel(this, r, IRT_NQ, 0);

    post(this,    100000*d+10000*o+1000*n+100*a+10*l+d
                + 100000*g+10000*e+1000*r+100*a+10*l+d
               == 100000*r+10000*o+1000*b+100*e+10*r+t,
         opt.icl());

    distinct(this, le, opt.icl());

    branch(this, le, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }
  /// Constructor for cloning \a s
  Donald(bool share, Donald& s) : Example(share,s) {
    le.update(this, share, s.le);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Donald(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\t" << le << std::endl;;
  }

};


/** \brief Main-function
 *  \relates Donald
 */
int
main(int argc, char* argv[]) {
  Options opt("Donald+Gerald=Robert");
  opt.solutions(0);
  opt.iterations(1500);
  opt.parse(argc,argv);
  Example::run<Donald,DFSE,Options>(opt);
  return 0;
}

// STATISTICS: example-any

