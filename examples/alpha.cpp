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

#include <gecode/driver.hh>

#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

/**
 * \brief %Example: %Alpha puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Alpha : public Script {
protected:
  /// Alphabet has 26 letters
  static const int n = 26;
  /// Array for letters
  IntVarArray le;
public:
  /// Branching to use for model
  enum {
    BRANCH_NONE,    ///< Choose variable left to right
    BRANCH_INVERSE, ///< Choose variable right to left
    BRANCH_SIZE     ///< Choose variable with smallest size
  };
  /// Actual model
  Alpha(const Options& opt) : le(*this,n,1,n) {
    IntVar
      a(le[ 0]), b(le[ 1]), c(le[ 2]), e(le[ 4]), f(le[ 5]),
      g(le[ 6]), h(le[ 7]), i(le[ 8]), j(le[ 9]), k(le[10]),
      l(le[11]), m(le[12]), n(le[13]), o(le[14]), p(le[15]),
      q(le[16]), r(le[17]), s(le[18]), t(le[19]), u(le[20]),
      v(le[21]), w(le[22]), x(le[23]), y(le[24]), z(le[25]);

    rel(*this, b+a+l+l+e+t       == 45,  opt.icl());
    rel(*this, c+e+l+l+o         == 43,  opt.icl());
    rel(*this, c+o+n+c+e+r+t     == 74,  opt.icl());
    rel(*this, f+l+u+t+e         == 30,  opt.icl());
    rel(*this, f+u+g+u+e         == 50,  opt.icl());
    rel(*this, g+l+e+e           == 66,  opt.icl());
    rel(*this, j+a+z+z           == 58,  opt.icl());
    rel(*this, l+y+r+e           == 47,  opt.icl());
    rel(*this, o+b+o+e           == 53,  opt.icl());
    rel(*this, o+p+e+r+a         == 65,  opt.icl());
    rel(*this, p+o+l+k+a         == 59,  opt.icl());
    rel(*this, q+u+a+r+t+e+t     == 50,  opt.icl());
    rel(*this, s+a+x+o+p+h+o+n+e == 134, opt.icl());
    rel(*this, s+c+a+l+e         == 51,  opt.icl());
    rel(*this, s+o+l+o           == 37,  opt.icl());
    rel(*this, s+o+n+g           == 61,  opt.icl());
    rel(*this, s+o+p+r+a+n+o     == 82,  opt.icl());
    rel(*this, t+h+e+m+e         == 72,  opt.icl());
    rel(*this, v+i+o+l+i+n       == 100, opt.icl());
    rel(*this, w+a+l+t+z         == 34,  opt.icl());

    distinct(*this, le, opt.icl());

    switch (opt.branching()) {
    case BRANCH_NONE:
      branch(*this, le, INT_VAR_NONE, INT_VAL_MIN);
      break;
    case BRANCH_INVERSE:
      branch(*this, le.slice(le.size()-1,-1), INT_VAR_NONE, INT_VAL_MIN);
      break;
    case BRANCH_SIZE:
      branch(*this, le, INT_VAR_SIZE_MIN, INT_VAL_MIN);
      break;
    }
  }

  /// Constructor for cloning \a s
  Alpha(bool share, Alpha& s) : Script(share,s) {
    le.update(*this, share, s.le);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Alpha(share,*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t";
    for (int i = 0; i < n; i++) {
      os << ((char) (i+'a')) << '=' << le[i] << ((i<n-1)?", ":"\n");
      if ((i+1) % 8 == 0)
        os << std::endl << "\t";
    }
    os << std::endl;
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
  opt.branching(Alpha::BRANCH_INVERSE, "inverse");
  opt.branching(Alpha::BRANCH_SIZE, "size");
  opt.parse(argc,argv);
  Script::run<Alpha,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

