/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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
 * \brief %Example: partition numbers into two groups
 *
 * \ingroup ExProblem
 */
class Partition : public Example {
protected:
  /// First group of numbers
  IntVarArray x;
  /// Second group of numbers
  IntVarArray y;
public:
  /// Actual model
  Partition(const Options& opt)
    : x(this,opt.size,1,2*opt.size),
      y(this,opt.size,1,2*opt.size) {
    const int n = opt.size;
    // Break symmetries by ordering numbers in each group
    rel(this, x, IRT_LE);
    rel(this, y, IRT_LE);

    rel(this, x[0], IRT_LE, y[0]);

    IntVarArgs xy(2*n);
    for (int i = n; i--; ) {
      xy[i] = x[i]; xy[n+i] = y[i];
    }
    distinct(this, xy, opt.icl());
    IntArgs c(2*n);
    for (int i = n; i--; ) {
      c[i] = 1; c[n+i] = -1;
    }
    linear(this, c, xy, IRT_EQ,0);

    // Array of products
    IntVarArray sxy(this,2*n,1,4*n*n);
    IntVarArgs sx(n);
    IntVarArgs sy(n);

    for (int i = n; i--; ) {
      mult(this,x[i],x[i],sxy[i]);   sx[i] = sxy[i];
      mult(this,y[i],y[i],sxy[n+i]); sy[i] = sxy[n+i];
    }
    linear(this, c,sxy,IRT_EQ,0);

    // Redundant
    linear(this, x, IRT_EQ, 2*n*(2*n+1)/4);
    linear(this, y, IRT_EQ, 2*n*(2*n+1)/4);
    linear(this, sx, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);
    linear(this, sy, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);
    branch(this, xy, BVAR_SIZE_MIN, BVAL_MIN);
  }

  /// Constructor used during cloning \a s
  Partition(bool share, Partition& s) : Example(share,s) {
    x.update(this, share, s.x);
    y.update(this, share, s.y);
  }
  /// Copying during cloning
  virtual Space*
  copy(bool share) {
    return new Partition(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    int a, b;
    a = b = 0;
    for (int i = 0; i < x.size(); i++) {
      a += x[i].val();
      b += x[i].val()*x[i].val();
      std::cout << x[i] << ", ";
    }
    std::cout << " = " << a << ", " << b << std::endl << "\t";
    a = b = 0;
    for (int i = 0; i < y.size(); i++) {
      a += y[i].val();
      b += y[i].val()*y[i].val();
      std::cout << y[i] << ", ";
    }
    std::cout << " = " << a << ", " << b << std::endl;
  }
};

/**
 * \brief Main-functiona
 * \relates Partition
 */
int
main(int argc, char* argv[]) {
  Options opt("Partition");
  opt.size = 32;
  opt.icl(ICL_DOM);
  opt.parse(argc,argv);
  Example::run<Partition,DFS,Options>(opt);
  return 0;
}


// STATISTICS: example-any

