/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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
 * \brief %Example: partition numbers into two groups
 *
 * \ingroup Example
 */
class Partition : public Script {
protected:
  /// First group of numbers
  IntVarArray x;
  /// Second group of numbers
  IntVarArray y;
public:
  /// Actual model
  Partition(const SizeOptions& opt)
    : Script(opt),
      x(*this,opt.size(),1,2*opt.size()),
      y(*this,opt.size(),1,2*opt.size()) {
    const int n = opt.size();

    // Break symmetries by ordering numbers in each group
    rel(*this, x, IRT_LE);
    rel(*this, y, IRT_LE);

    rel(*this, x[0], IRT_LE, y[0]);

    IntVarArgs xy(2*n);
    for (int i = n; i--; ) {
      xy[i] = x[i]; xy[n+i] = y[i];
    }
    distinct(*this, xy, opt.ipl());

    IntArgs c(2*n);
    for (int i = n; i--; ) {
      c[i] = 1; c[n+i] = -1;
    }
    linear(*this, c, xy, IRT_EQ, 0);

    // Array of products
    IntVarArgs sxy(2*n), sx(n), sy(n);

    for (int i = n; i--; ) {
      sx[i] = sxy[i] =   expr(*this, sqr(x[i]));
      sy[i] = sxy[n+i] = expr(*this, sqr(y[i]));
    }
    linear(*this, c, sxy, IRT_EQ, 0);

    // Redundant constraints
    linear(*this, x, IRT_EQ, 2*n*(2*n+1)/4);
    linear(*this, y, IRT_EQ, 2*n*(2*n+1)/4);
    linear(*this, sx, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);
    linear(*this, sy, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);

    branch(*this, xy, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN());
  }

  /// Constructor used during cloning \a s
  Partition(Partition& s) : Script(s) {
    x.update(*this, s.x);
    y.update(*this, s.y);
  }
  /// Copying during cloning
  virtual Space*
  copy(void) {
    return new Partition(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t";
    int a, b;
    a = b = 0;
    for (int i = 0; i < x.size(); i++) {
      a += x[i].val();
      b += x[i].val()*x[i].val();
      os << x[i] << ", ";
    }
    os << " = " << a << ", " << b << std::endl << "\t";
    a = b = 0;
    for (int i = 0; i < y.size(); i++) {
      a += y[i].val();
      b += y[i].val()*y[i].val();
      os << y[i] << ", ";
    }
    os << " = " << a << ", " << b << std::endl;
  }
};

/**
 * \brief Main-functiona
 * \relates Partition
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Partition");
  opt.size(32);
  opt.ipl(IPL_BND);
  opt.parse(argc,argv);
  Script::run<Partition,DFS,SizeOptions>(opt);
  return 0;
}


// STATISTICS: example-any

