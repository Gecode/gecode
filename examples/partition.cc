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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "examples/support.hh"

/**
 * \brief %Example: partition numbers into two groups
 *
 * \ingroup Example
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
    for (int i = n-1; i--; ) {
      rel(this, x[i], IRT_LE, x[i+1]);
      rel(this, y[i], IRT_LE, y[i+1]);
    }
    rel(this, x[0], IRT_LE, y[0]);

    IntVarArgs xy(2*n);
    for (int i = n; i--; ) {
      xy[i] = x[i]; xy[n+i] = y[i];
    }
    distinct(this, xy, opt.icl);
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
main(int argc, char** argv) {
  Options o("Partition");
  o.size = 32;
  o.icl  = ICL_DOM;
  o.parse(argc,argv);
  Example::run<Partition,DFS>(o);
  return 0;
}


// STATISTICS: example-any

