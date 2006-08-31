/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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
 * \brief %Example: Orthogonal latin squares
 *
 * \ingroup Examples
 */
class OrthoLatinSquare : public Example {
protected:
  /// Size of squares
  const int n;
  /// Fields of first square
  IntVarArray x1;
  /// Fields of second square
  IntVarArray x2;

public:
  /// Access field at position \a i and \a j in first square
  IntVar& y1(int i, int j) {
    return x1[i*n+j];
  }
  /// Access field at position \a i and \a j in second square
  IntVar& y2(int i, int j) {
    return x2[i*n+j];
  }

  /// Actual model
  OrthoLatinSquare(const Options& opt)
    : n(opt.size),
      x1(this,n*n,1,n), x2(this,n*n,1,n) {
    const int nn = n*n;
    IntVarArray z(this,nn,0,n*n-1);

    distinct(this, z, opt.icl);
    // Connect
    {
      IntArgs mod(n*n);
      IntArgs div(n*n);
      for (int i=0; i<n; i++)
	for (int j=0; j<n; j++) {
	  mod[i*n+j] = j+1;
	  div[i*n+j] = i+1;
	}
      for (int i = nn; i--; ) {
	element(this, div, z[i], x2[i]);
	element(this, mod, z[i], x1[i]);
      }
    }

    // Rows
    for (int i = n; i--; ) {
      IntVarArgs ry(n);
      for (int j = n; j--; )
	ry[j] = y1(i,j);
      distinct(this, ry, opt.icl);
      for (int j = n; j--; )
	ry[j] = y2(i,j);
      distinct(this, ry, opt.icl);
    }
    for (int j = n; j--; ) {
      IntVarArgs cy(n);
      for (int i = n; i--; )
	cy[i] = y1(i,j);
      distinct(this, cy, opt.icl);
      for (int i = n; i--; )
	cy[i] = y2(i,j);
      distinct(this, cy, opt.icl);
    }

    for (int i = 1; i<n; i++) {
      IntVarArgs ry1(n);
      IntVarArgs ry2(n);
      for (int j = n; j--; ) {
	ry1[j] = y1(i-1,j);
	ry2[j] = y2(i,j);
      }
      rel(this, ry1, IRT_GQ, ry2);
    }

    branch(this, z, BVAR_SIZE_MIN, BVAL_SPLIT_MIN);
  }

  /// Constructor for cloning \a s
  OrthoLatinSquare(bool share, OrthoLatinSquare& s)
    : Example(share,s), n(s.n) {
      x1.update(this, share, s.x1);
      x2.update(this, share, s.x2);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new OrthoLatinSquare(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    for (int i = 0; i<n; i++) {
      std::cout << "\t";
      for (int j = 0; j<n; j++) {
	std::cout.width(2);
	std::cout << y1(i,j) << "  ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
    for (int i = 0; i<n; i++) {
      std::cout << "\t";
      for (int j = 0; j<n; j++) {
	std::cout.width(2);
	std::cout << y2(i,j) << "  ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

};

/**
 * \brief Main function
 * \relates OrthoLatinSquare
 */
int
main(int argc, char** argv) {
  Options opt("OrthoLatinSquare");
  opt.size = 7;
  opt.icl  = ICL_DOM;
  opt.parse(argc,argv);
  Example::run<OrthoLatinSquare,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

