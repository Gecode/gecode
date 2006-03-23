/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date: 2006-02-22 16:42:22 +0100 (Wed, 22 Feb 2006) $ by $Author: schulte $
 *     $Revision: 3000 $
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
 * \brief %Example: %Alpha puzzle
 *
 * Well-known cryptoarithmetic puzzle of unknown origin.
 *
 * \ingroup Example
 *
 */
class Alpha : public Example {
protected:
  IntVarArray x;
  IntVarArray y;
public:
  /// Actual model
  Alpha(const Options& opt) : x(this,4,-4,4), y(this,4,-8,8) {
    channel(this, x, y);
    rel(this, x[0], IRT_NQ, 0);
    rel(this, x[0], IRT_NQ, 1);
    rel(this, x[1], IRT_NQ, 1);
    rel(this, x[3], IRT_NQ, 0);
    rel(this, x[3], IRT_NQ, 1);
    //    rel(this, y[3], IRT_NQ, 3);
  }

  /// Constructor for cloning \a s
  Alpha(bool share, Alpha& s) : Example(share,s) {
    x.update(this, share, s.x);
    y.update(this, share, s.y);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Alpha(share,*this);
  }
  /// Print solution
  virtual void 
  print(void) {
    std::cout << "\tx={";
    for (int i = 0; i < 4; i++)
      std::cout << x[i] << ", ";
    std::cout << "}" << std::endl;
    std::cout << "\ty={";
    for (int i = 0; i < 4; i++)
      std::cout << y[i] << ", ";
    std::cout << "}" << std::endl;
  }
};

/** \brief Main-function
 *  \relates Alpha
 */
int
main(int argc, char** argv) {
  Options opt("Alpha");
  opt.solutions  = 0;
  opt.iterations = 10;
  opt.naive      = true;
  opt.parse(argc,argv);
  Example::run<Alpha,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

