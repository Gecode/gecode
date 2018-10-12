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
  IntVarArray x;
  BoolVar b;
public:
  /// Actual model
  Alpha(const Options& opt)
    : Script(opt), x(*this,4,0,6), b(*this,0,1) {
    TupleSet t(3);
    t.add({0,0,0}).add({1,1,1})
      .add({0,1,2}).add({2,1,0}).finalize();
             TupleSet ts(4);
             ts.add({2, 1, 2, 4}).add({2, 2, 1, 4})
               .add({4, 3, 4, 1}).add({1, 3, 2, 3})
               .add({3, 3, 3, 2}).add({5, 1, 4, 4})
               .add({2, 5, 1, 5}).add({4, 3, 5, 1})
               .add({1, 5, 2, 5}).add({5, 3, 3, 2})
               .finalize();
    rel(*this, x[0] == 0);
    rel(*this, x[1] == 0);
    rel(*this, x[2] == 0);
    rel(*this, x[3] == 0);

             extensional(*this, x, ts,b);
    //    rel(*this, b == 0);

    branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Alpha(Alpha& s) : Script(s) {
    x.update(*this, s.x);
    b.update(*this, s.b);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Alpha(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tx=" << x;
    os << ", b=" << b;
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates Alpha
 */
int
main(int argc, char* argv[]) {
  Options opt("Alpha");
  opt.parse(argc,argv);
  Script::run<Alpha,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any

