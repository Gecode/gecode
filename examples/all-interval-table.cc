/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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
#include "examples/assignment.hh"
#include "gecode/minimodel.hh"

#include <cstdlib>
#include <algorithm>

namespace {
  Table diffa;
  Table diffb;
  
  void init_diff(Table& tab, int lo, int hi) {
    const int size = 3;
    IntSet is(lo, hi);
    Assignment ass(size, is);

    do {
      // Valid tuple?
      for (int i = 0; i < size-1; ++i) {
        for (int j = i+1; j < size; ++j) {
          if (ass[i] == ass[j]) goto next;
        }
      }
      { // Add to table
        IntArgs t(size);
        for (int i = 0; i < size; ++i) {
          t[i] = ass[i];
          //std::cerr << t[i] << " ";
        }
        //std::cerr << std::endl;
        tab.add(t);
      }
    next:
      ++ass;
    } while (ass());
    //std::cerr << std::endl;
    tab.finalize();
  }
}



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
 * \ingroup ExProblem
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

    ExtensionalAlgorithm ea = EA_BASIC;
    if (!opt.naive) {
      ea = EA_INCREMENTAL;
    }
    //std::cerr << "Posting for x:" << std::endl;
    int a = x.size();
    for (int i = 0; i < x.size(); ++i) {
      for (int j = i+1; j < x.size(); ++j) {
        do { a = (a-1+x.size())%x.size(); } while (a==i || a==j);
        //std::cerr << i << ", " << j << ", " << a << ", " << b << std::endl;
        IntVarArgs iva(3);
        iva[0] = x[i]; iva[1] = x[j]; 
        iva[2] = x[a];
        extensional(this, iva, diffa, ea);
        // distinct(this, iva, opt.icl);
      }
    }
    //std::cerr << "Posting for d:" << std::endl;
    a = d.size();
    for (int i = 0; i < d.size(); ++i) {
      for (int j = i+1; j < d.size(); ++j) {
        do { a = (a-1+d.size())%d.size(); } while (a==i || a==j);
        //std::cerr << i << ", " << j << ", " << a << ", " << b << std::endl;
        IntVarArgs iva(3);
        iva[0] = d[i]; iva[1] = d[j]; 
        iva[2] = d[a];
        extensional(this, iva, diffb, ea);
        // distinct(this, iva, opt.icl);
      }
    }

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
  opt.size       = 10;
  opt.iterations = 1;
  opt.icl        = ICL_BND;
  opt.parse(argc, argv);
  if (opt.size < 2) {
    std::cerr << "n must be at least 2!" << std::endl;
    return -1;
  }
  init_diff(diffa, 0, opt.size-1);
  init_diff(diffb, 1, opt.size-1);
  Example::run<AllInterval,DFS>(opt);
  return 0;
}

// STATISTICS: example-any

