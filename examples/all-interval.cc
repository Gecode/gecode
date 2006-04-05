/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2005-11-28 13:56:08 +0100 (Mon, 28 Nov 2005) $ by $Author: schulte $
 *     $Revision: 2654 $
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

#include <cstdlib>

/**
 * \brief %Example: All-interval Series
 *
 * An all-interval series of length \f$n\f$ is a sequence
 * \f[
 * (x_0,x_1,\ldots,n_{n-1})
 * \f]
 * where each \f$x_i\f$ is an integer between \f$0\f$ and \f$n-1\f$ 
 * such that the following conditions hold:
 *  - the \f$x_i\f$ are a permutation of \f$\{0,1,\ldots,n-1\}\f$
 *    (that is, they are pairwise distinct and take values from
 *    \f$\{0,1,\ldots,n-1\}\f$).
 *  - the differences between adjacent values \f$(d_1,d_2,\ldots,d_{n-1})\f$
 *    with \f$d_i=\operatorname{abs}(x_i-x_{i-1})\f$ form a permutation of 
 *    \f$\{1,2,\ldots,n-1\}\f$.
 *
 * Read more about this problem as prob007 in the category 
 * "combinatorial mathematics" of http://www.csplib.org/. 
 *
 * \ingroup Example
 */
class AllInterval : public Example {
private:
  /// The numbers
  IntVarArray x;
public:
  /// Actual model
  AllInterval(const Options& opt) :
    x(this, opt.size, 0, opt.size - 1) {
    const int n = x.size();

    IntVarArgs d(n-1);

    // Set up variables for distance
    for (int i=0; i<n-1; i++)
      d[i] = abs(this, minus(this,x[i+1],x[i],opt.icl),opt.icl);
   
    // Constrain them to be between 1 and n-1
    dom(this, d, 1, n-1);
    
    distinct(this, x, opt.icl);
    distinct(this, d, opt.icl);

    // Break mirror symmetry
    rel(this, x[0], IRT_LE, x[1]);
    // Break symmetry of dual solution
    rel(this, d[0], IRT_GR, d[n-2]);

    branch(this, x, BVAR_SIZE_MIN, BVAL_SPLIT_MIN);
  }
  /// Constructor for cloning \a e
  AllInterval(bool share, AllInterval& e)
    : Example(share, e) {
    x.update(this, share, e.x);
  }
  /// Copy during cloning
  virtual Space* 
  copy(bool share) {
    return new AllInterval(share, *this);
  }
  /// Print solution
  virtual void
  print(void) {
    const int n = x.size();
    std::cout << "\tx[" << n << "] = {";
    for (int i = 0; i < n-1; i++)
      std::cout << x[i] << "(" << abs(x[i+1].val()-x[i].val()) << "),";
    std::cout << x[n-1] << "}" << std::endl;
  }
};


int main(int argc, char** argv){
  Options opt("All-interval Series");
  opt.size = 12;
  opt.icl  = ICL_BND;
  opt.parse(argc, argv);
  if (opt.size < 2) {
    std::cerr << "n must be at least 2!" << std::endl;
    return -1;
  }
  Example::run<AllInterval,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

