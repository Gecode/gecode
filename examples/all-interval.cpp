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
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <cstdlib>

using namespace Gecode;

/**
 * \brief %Example: All-interval series
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
 * See also problem 7 at http://www.csplib.org/.
 *
 * \ingroup Example
 */
class AllInterval : public Script {
private:
  /// The numbers
  IntVarArray x;
public:
  /// Actual model
  AllInterval(const SizeOptions& opt) :
    x(*this, opt.size(), 0, opt.size() - 1) {
    const int n = x.size();

    IntVarArgs d(n-1);

    // Set up variables for distance
    for (int i=0; i<n-1; i++)
      d[i] = expr(*this, abs(x[i+1]-x[i]), opt.icl());

    // Constrain them to be between 1 and n-1
    dom(*this, d, 1, n-1);

    distinct(*this, x, opt.icl());
    distinct(*this, d, opt.icl());

    // Break mirror symmetry
    rel(*this, x[0], IRT_LE, x[1]);
    // Break symmetry of dual solution
    rel(*this, d[0], IRT_GR, d[n-2]);

    branch(*this, x, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MIN);
  }
  /// Constructor for cloning \a e
  AllInterval(bool share, AllInterval& e)
    : Script(share, e) {
    x.update(*this, share, e.x);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new AllInterval(share, *this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    const int n = x.size();
    os << "\tx[" << n << "] = {";
    for (int i = 0; i < n-1; i++)
      os << x[i] << "(" << abs(x[i+1].val()-x[i].val()) << "),";
    os << x[n-1] << "}" << std::endl;
  }
};


/** \brief Main-function
 *  \relates AllInterval
 */
int
main(int argc, char* argv[]){
  SizeOptions opt("AllInterval");
  opt.size(1000);
  opt.iterations(5);
  opt.icl(ICL_BND);
  opt.parse(argc, argv);
  if (opt.size() < 2) {
    std::cerr << "size must be at least 2!" << std::endl;
    return -1;
  }
  Script::run<AllInterval,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

