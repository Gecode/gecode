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
 * \brief %Example: DONALD+GERALD=ROBERT puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Donald : public Example {
private:
  /// Number of letters
  static const int nl = 10;
  /// Array of letters
  IntVarArray le;
public:
  Donald(const Options& opt) : 
    le(this,nl,0,9) {
    IntVar
      d(le[0]), o(le[1]), n(le[2]), a(le[3]), l(le[4]),
      g(le[5]), e(le[6]), r(le[7]), b(le[8]), t(le[9]);
    rel(this, d, IRT_NQ, 0);
    rel(this, g, IRT_NQ, 0);
    rel(this, r, IRT_NQ, 0);

    post(this,    100000*d+10000*o+1000*n+100*a+10*l+d
                + 100000*g+10000*e+1000*r+100*a+10*l+d
	       == 100000*r+10000*o+1000*b+100*e+10*r+t,
	 opt.icl);

    distinct(this, le, opt.icl);

    branch(this, le, BVAR_SIZE_MIN, BVAL_MIN);
  }

  Donald(bool share, Donald& s) : Example(share,s) {
    le.update(this, share, s.le);
  }

  virtual Space*
  copy(bool share) {
    return new Donald(share,*this);
  }

  virtual void
  print(void) {
    std::cout << "\t";
    for (int i = 0; i < nl; i++)
      std::cout << le[i] << ' ';
    std::cout << std::endl;
  }

};


/** \brief Main-function
 *  \relates Donald
 */
int
main(int argc, char** argv) {
  Options opt("Donald+Gerald=Robert");
  opt.solutions  = 0;
  opt.iterations = 1500;
  opt.parse(argc,argv);
  Example::run<Donald,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

