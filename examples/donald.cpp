/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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
 * \brief %Example: DONALD+GERALD=ROBERT puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Donald : public Script {
private:
  /// Number of letters
  static const int nl = 10;
  /// Array of letters
  IntVarArray le;
public:
  /// Model variants
  enum {
    MODEL_SINGLE, ///< Use single linear equation
    MODEL_CARRY   ///< Use carries
  };
  /// Actual model
  Donald(const Options& opt)
    : Script(opt), le(*this,nl,0,9) {
    IntVar
      d(le[0]), o(le[1]), n(le[2]), a(le[3]), l(le[4]),
      g(le[5]), e(le[6]), r(le[7]), b(le[8]), t(le[9]);
    rel(*this, d, IRT_NQ, 0);
    rel(*this, g, IRT_NQ, 0);
    rel(*this, r, IRT_NQ, 0);

    distinct(*this, le, opt.ipl());

    switch (opt.model()) {
    case MODEL_SINGLE:
      rel(*this,    100000*d+10000*o+1000*n+100*a+10*l+d
                  + 100000*g+10000*e+1000*r+100*a+10*l+d
                 == 100000*r+10000*o+1000*b+100*e+10*r+t,
           opt.ipl());
      break;
    case MODEL_CARRY:
      {
        IntVar c0(*this,0,1), c1(*this,0,1), c2(*this,0,1),
          c3(*this,0,1), c4(*this,0,1);
        rel(*this,    d+d == t+10*c0, opt.ipl());
        rel(*this, c0+l+l == r+10*c1, opt.ipl());
        rel(*this, c1+a+a == e+10*c2, opt.ipl());
        rel(*this, c2+n+r == b+10*c3, opt.ipl());
        rel(*this, c3+o+e == o+10*c4, opt.ipl());
        rel(*this, c4+d+g == r,       opt.ipl());
      }
      break;
    default: GECODE_NEVER;
    }

    branch(*this, le, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
  }
  /// Constructor for cloning \a s
  Donald(Donald& s) : Script(s) {
    le.update(*this, s.le);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Donald(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << le << std::endl;;
  }

};


/** \brief Main-function
 *  \relates Donald
 */
int
main(int argc, char* argv[]) {
  Options opt("Donald+Gerald=Robert");
  opt.model(Donald::MODEL_SINGLE);
  opt.model(Donald::MODEL_SINGLE, "single", "use single linear equation");
  opt.model(Donald::MODEL_CARRY, "carry", "use carry");
  opt.solutions(0);
  opt.iterations(1500);
  opt.parse(argc,argv);
  Script::run<Donald,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

