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
 * \ingroup Example
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
    int v, b, r, k, lambda;
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
      linear(this, row, IRT_EQ, par.r);
    }
    // k ones per column
    for (int j=par.b; j--; ) {
      BoolVarArgs col(par.v);
      for (int i=par.v; i--; )
	col[i] = p(i,j);
      linear(this, col, IRT_EQ, par.k);
    }
    // Exactly lambda ones in scalar product between two different rows
    for (int i1=0; i1<par.v; i1++)
      for (int i2=i1+1; i2<par.v; i2++) {
	BoolVarArgs row(par.b);
	for (int j=par.b; j--; ) {
	  BoolVar b(this,0,1);
	  bool_and(this,p(i1,j),p(i2,j),b);
	  row[j] = b;
	}
	linear(this, row, IRT_EQ, par.lambda);
      }

    for (int i=1;i<par.v;i++) {
      BoolVarArgs row1(par.b);
      BoolVarArgs row2(par.b);
      for (int j=par.b; j--; ) {
	row1[j] = p(i-1,j);
	row2[j] = p(i,j);
      }
      lex(this, row1, IRT_GQ, row2);
    }
    for (int j=1;j<par.b;j++) {
      BoolVarArgs col1(par.v);
      BoolVarArgs col2(par.v);
      for (int i=par.v; i--; ) {
	col1[i] = p(i,j-1);
	col2[i] = p(i,j);
      }
      lex(this, col1, IRT_GQ, col2);
    }

    branch(this, _p, BVAR_NONE, BVAL_MAX);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tBIBD("
	      << par.v << "," << par.b << ","
	      << par.r << "," << par.k << ","
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
  opt.solutions  = 1;
  opt.iterations = 25;
  opt.size       = 4;
  opt.c_d        = 10;
  opt.parse(argc,argv);
  BIBD::Par p;
  switch (opt.size) {
  case 0:
    p.v=7; p.b=7; p.r=3; p.k=3; p.lambda=1; break;
  case 1:
    p.v=6; p.b=10; p.r=5; p.k=3; p.lambda=2; break;
  case 2:
    p.v=8; p.b=14; p.r=7; p.k=4; p.lambda=3; break;
  case 3:
    p.v=6; p.b=20; p.r=10; p.k=3; p.lambda=4; break;
  case 4:
    p.v=11; p.b=55; p.r=15; p.k=3; p.lambda=3; break;
  case 5:
    p.v=7; p.b=70; p.r=30; p.k=3; p.lambda=10; break;
  default:
    std::cerr << "Error: size must be between 0 and 5" << std::endl;
    return 1;
  }
  BIBD::par = p;
  
  Example::run<BIBD,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

