/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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

using namespace Gecode;

/**
 * \brief %Example: Dominating Queens
 *
 * Place a number of queens on a n times n chessboard such that
 * each squares is either attacked or occupied by a queen.
 *
 * The model is taken from: C. Bessiere, E. Hebrard, B. Hnich,
 * Z. Kiziltan, and T. Walsh, Filtering Algorithms for the NValue
 * Constraint, Constraints, 11(4), 271-293, 2006.
 *
 * \ingroup Example
 *
 */
class DominatingQueens : public IntMinimizeScript {
protected:
  /// Size of the board
  const int n;
  /// Fields on the board
  IntVarArray b;
  /// Number of queens
  IntVar q;
  /// Compute coordinate pair from \a x and \a y
  int xy(int x, int y) const {
    return y*n + x;
  }
  /// Compute x coordinate from pair \a xy
  int x(int xy) const {
    return xy % n;
  }
  /// Compute y coordinate from pair \a xy
  int y(int xy) const {
    return xy / n;
  }
  /// Compute set of fields that can be attacked by \a xy
  IntSet attacked(int xy) {
    IntArgs a;
    for (int i=0; i<n; i++)
      for (int j=0; j<n; j++)
        if ((i == x(xy)) || // Same row
            (j == y(xy)) || // Same column
            (abs(i-x(xy)) == abs(j-y(xy)))) // Same diagonal
          a << DominatingQueens::xy(i,j);
    return IntSet(a);
  }
public:
  /// The actual problem
  DominatingQueens(const SizeOptions& opt)
    : IntMinimizeScript(opt),
      n(opt.size()), b(*this,n*n,0,n*n-1), q(*this,1,n) {

    // Constrain field to the fields that can attack a field
    for (int i=0; i<n*n; i++)
      dom(*this, b[i], attacked(i));

    // At most q queens can be placed
    nvalues(*this, b, IRT_LQ, q);

    /*
     * According to: P. R. J. Östergard and W. D. Weakley, Values
     * of Domination Numbers of the Queen's Graph, Electronic Journal
     * of Combinatorics, 8:1-19, 2001, for n <= 120, the minimal number
     * of queens is either ceil(n/2) or ceil(n/2 + 1).
     */
    if (n <= 120)
      dom(*this, q, (n+1)/2, (n+1)/2 + 1);

    branch(*this, b, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    // Try the easier solution first
    branch(*this, q, INT_VAL_MAX());
  }

  /// Return cost
  virtual IntVar cost(void) const {
    return q;
  }

  /// Constructor for cloning \a s
  DominatingQueens(DominatingQueens& s)
    : IntMinimizeScript(s), n(s.n) {
    b.update(*this, s.b);
    q.update(*this, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new DominatingQueens(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\tNumber of Queens: " << q << std::endl;
    os << "\tBoard: " << b << std::endl;
    if (b.assigned()) {
      // Print a nice board
      bool* placed = new bool[n*n];
      for (int i=0; i<n*n; i++)
        placed[i] = false;
      for (int i=0; i<n*n; i++)
        placed[b[i].val()] = true;
      for (int j=0; j<n; j++) {
        std::cout << "\t\t";
        for (int i=0; i<n; i++)
          std::cout << (placed[xy(i,j)] ? 'Q' : '.') << ' ';
        std::cout << std::endl;
      }
      delete [] placed;
    }
    os << std::endl;
  }
};

/** \brief Main-function
 *  \relates DominatingQueens
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("DominatingQueens");
  opt.size(7);
  opt.solutions(0);
  opt.parse(argc,argv);
  IntMinimizeScript::run<DominatingQueens,BAB,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

