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
#include "minimodel.hh"

/**
 * \brief %Example: SEND+MORE=MONEY puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Money : public Example {
protected:
  /// Number of letters
  static const int nl = 8;
  /// Array of letters
  IntVarArray le;
public:
  /// Actual model
  Money(const Options& opt) : le(this,nl,0,9) {
    IntVar
      s(le[0]), e(le[1]), n(le[2]), d(le[3]), 
      m(le[4]), o(le[5]), r(le[6]), y(le[7]);

    rel(this, s, IRT_NQ, 0);
    rel(this, m, IRT_NQ, 0);

    post(this,            1000*s+100*e+10*n+d
                +         1000*m+100*o+10*r+e
	       == 10000*m+1000*o+100*n+10*e+y,
	 opt.icl);

    distinct(this, le, opt.icl);
    branch(this, le, BVAR_SIZE_MIN, BVAL_MIN);
  }
  /// Print solution
  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < nl; i++)
      std::cout << le[i] << " ";
    std::cout << std::endl;
  }

  /// Constructor for cloning \a s
  Money(bool share, Money& s) : Example(share,s) {
    le.update(this, share, s.le);
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
main(int argc, char** argv) {
  Options opt("SEND+MORE=MONEY");
  opt.solutions  = 0;
  opt.iterations = 20000;
  opt.parse(argc,argv);
  Example::run<Money,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

