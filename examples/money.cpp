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
 * \brief %Example: SEND+MORE=MONEY puzzle
 *
 * Well-known cryptoarithmetic puzzle.
 * Henry Dudeney, Strand Magazine, July 1924.
 *
 * \ingroup Example
 *
 */
class Money : public Script {
protected:
  /// Number of letters
  static const int nl = 8;
  /// Array of letters
  IntVarArray le;
public:
  /// Model variants
  enum {
    MODEL_SINGLE, ///< Use single linear equation
    MODEL_CARRY   ///< Use carries
  };
  /// Actual model
  Money(const Options& opt) : le(*this,nl,0,9) {
    IntVar
      s(le[0]), e(le[1]), n(le[2]), d(le[3]),
      m(le[4]), o(le[5]), r(le[6]), y(le[7]);

    rel(*this, s, IRT_NQ, 0);
    rel(*this, m, IRT_NQ, 0);

    distinct(*this, le, opt.icl());

    switch (opt.model()) {
    case MODEL_SINGLE:
      rel(*this,            1000*s+100*e+10*n+d
                  +         1000*m+100*o+10*r+e
                 == 10000*m+1000*o+100*n+10*e+y,
           opt.icl());
      break;
    case MODEL_CARRY:
      {
        IntVar c0(*this,0,1), c1(*this,0,1), c2(*this,0,1), c3(*this,0,1);
        rel(*this,    d+e == y+10*c0, opt.icl());
        rel(*this, c0+n+r == e+10*c1, opt.icl());
        rel(*this, c1+e+o == n+10*c2, opt.icl());
        rel(*this, c2+s+m == o+10*c3, opt.icl());
        rel(*this, c3     == m,       opt.icl());
      }
      break;
    default: GECODE_NEVER;
    }

    branch(*this, le, INT_VAR_SIZE_MIN, INT_VAL_MIN);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << le << std::endl;
  }

  /// Constructor for cloning \a s
  Money(bool share, Money& s) : Script(share,s) {
    le.update(*this, share, s.le);
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
  opt.model(Money::MODEL_SINGLE);
  opt.model(Money::MODEL_SINGLE, "single", "use single linear equation");
  opt.model(Money::MODEL_CARRY, "carry", "use carry");
  opt.solutions(0);
  opt.iterations(20000);
  opt.parse(argc,argv);
  Script::run<Money,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

