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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "examples/support.hh"
#include "gecode/minimodel.hh"

/**
 * \brief %Example: n-%Queens puzzle
 *
 * Place n queens on an n times n chessboard such that they do not
 * attack each other.
 *
 * \ingroup Example
 *
 */
class Queens : public Example {
protected:
  /// Position of queens on boards
  IntVarArray q;
public:
  /// The actual problem
  Queens(const Options& opt)
    : q(this,opt.size,0,opt.size-1) {
    const int n = q.size();
    if (opt.naive) {
      for (int i = 0; i<n; i++)
	for (int j = i+1; j<n; j++) {
	  post(this, q[i] != q[j]);
	  post(this, q[i]+i != q[j]+j);
	  post(this, q[i]-i != q[j]-j);
	}
    } else {
      IntArgs c(n);
      for (int i = n; i--; ) c[i] = i;
      distinct(this, c,q, opt.icl);
      for (int i = n; i--; ) c[i] = -i;
      distinct(this, c,q, opt.icl);
      distinct(this, q, opt.icl);
    }
    branch(this, q, BVAR_SIZE_MIN, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  Queens(bool share, Queens& s) : Example(share,s) {
    q.update(this, share, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Queens(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < q.size(); i++) {
      std::cout << q[i] << ", ";
      if ((i+1) % 10 == 0)
	std::cout << std::endl << "\t";
    }
    std::cout << std::endl;
    }
};

/** \brief Main-function
 *  \relates Alpha
 */
int
main(int argc, char** argv) {
  Options opt("Queens");
  opt.iterations = 200;
  opt.size       = 100;
  opt.c_d        = 5;
  opt.parse(argc,argv);
  Example::run<Queens,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

