/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Bugfixes provided by:
 *     Olof Sivertsson <olsi0767@student.uu.se>
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
 * \brief %Example: Balanced Incomplete Block Design (%BIBD)
 *
 * See problem 28 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 *
 */
class BIBD : public Example {
protected:
  /// Matrix of Boolean variables
  BoolVarArray _p;
public:
  /// Class for BIBD parameters
  class Par {
  public:
    int v, k, lambda, b, r;
    Par(int v0, int k0, int l0)
      : v(v0), k(k0), lambda(l0),
        b((v*(v-1)*lambda)/(k*(k-1))),
        r((lambda*(v-1)) / (k-1)) {}
    Par(void) {}
  };
  static Par par;

  /// Access variable by row and column
  BoolVar& p(int i, int j) {
    // row, column
    return _p[i*par.b+j];
  }

  /// Actual model
  BIBD(const Options& opt)
    : _p(this,par.v*par.b,0,1) {
    // r ones per row
    for (int i=par.v; i--; ) {
      BoolVarArgs row(par.b);
      for (int j=par.b; j--; )
        row[j] = p(i,j);
      if (opt.naive) {
        linear(this, row, IRT_LQ, par.r);
        linear(this, row, IRT_GQ, par.r);
      } else {
        linear(this, row, IRT_EQ, par.r);
      }
    }
    // k ones per column
    for (int j=par.b; j--; ) {
      BoolVarArgs col(par.v);
      for (int i=par.v; i--; )
        col[i] = p(i,j);
      if (opt.naive) {
        linear(this, col, IRT_LQ, par.k);
        linear(this, col, IRT_GQ, par.k);
      } else {
        linear(this, col, IRT_EQ, par.k);
      }
    }
    // Exactly lambda ones in scalar product between two different rows
    for (int i1=0; i1<par.v; i1++)
      for (int i2=i1+1; i2<par.v; i2++) {
        BoolVarArgs row(par.b);
        for (int j=par.b; j--; ) {
          BoolVar b(this,0,1);
          rel(this,p(i1,j),BOT_AND,p(i2,j),b);
          row[j] = b;
        }
        if (opt.naive) {
          linear(this, row, IRT_LQ, par.lambda);
          linear(this, row, IRT_GQ, par.lambda);
        } else {
          linear(this, row, IRT_EQ, par.lambda);
        }
      }

    for (int i=1;i<par.v;i++) {
      BoolVarArgs row1(par.b);
      BoolVarArgs row2(par.b);
      for (int j=par.b; j--; ) {
        row1[j] = p(i-1,j);
        row2[j] = p(i,j);
      }
      rel(this, row1, IRT_GQ, row2);
    }
    for (int j=1;j<par.b;j++) {
      BoolVarArgs col1(par.v);
      BoolVarArgs col2(par.v);
      for (int i=par.v; i--; ) {
        col1[i] = p(i,j-1);
        col2[i] = p(i,j);
      }
      rel(this, col1, IRT_GQ, col2);
    }

    branch(this, _p, BVAR_NONE, BVAL_MIN);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tBIBD("
              << par.v << "," << par.k << ","
              << par.lambda << ")" << std::endl;
    for (int i = 0; i < par.v; i++) {
      std::cout << "\t\t";
      for (int j = 0; j< par.b; j++)
        std::cout << p(i,j) << " ";
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  /// Constructor for cloning \a s
  BIBD(bool share, BIBD& s)
    : Example(share,s) {
    _p.update(this,share,s._p);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new BIBD(share,*this);
  }

};


BIBD::Par BIBD::par;

/** \brief Main-function
 *  \relates BIBD
 */
int
main(int argc, char** argv) {
  Options opt("BIBD");
  opt.solutions = 1;
  opt.size      = 9;
  opt.naive     = false;
  opt.parse(argc,argv);
  switch (opt.size) {
  case 0: { BIBD::Par p(7,3,1);  BIBD::par = p; break; }
  case 1: { BIBD::Par p(6,3,2);  BIBD::par = p; break; }
  case 2: { BIBD::Par p(8,4,3);  BIBD::par = p; break; }
  case 3: { BIBD::Par p(7,3,20); BIBD::par = p; break; }
  case 4: { BIBD::Par p(7,3,30); BIBD::par = p; break; }
  case 5: { BIBD::Par p(7,3,40); BIBD::par = p; break; }
  case 6: { BIBD::Par p(7,3,45); BIBD::par = p; break; }
  case 7: { BIBD::Par p(7,3,50); BIBD::par = p; break; }
  case 8: { BIBD::Par p(7,3,55); BIBD::par = p; break; }
  case 9: { BIBD::Par p(7,3,60); BIBD::par = p; break; }
  case 10: { BIBD::Par p(7,3,100); BIBD::par = p; break; }
  case 11: { BIBD::Par p(7,3,200); BIBD::par = p; break; }
  default:
    std::cerr << "Error: size must be between 0 and 11" << std::endl;
    return 1;
  }

  Example::run<BIBD,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

