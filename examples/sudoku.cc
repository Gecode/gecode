/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "examples/support.hh"

#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

#include "gecode/minimodel.hh"

// include the example specifications
#include "examples/sudoku.icc"

#ifdef GECODE_HAVE_SET_VARS
void same(Space* home, int nn, IntVarArgs a, IntVarArgs b) {
  SetVar u(home, IntSet::empty, 1, nn);
  rel(home, SOT_DUNION, a, u);
  rel(home, SOT_DUNION, b, u);
}

IntVarArgs
block_col(MiniModel::Matrix<IntVarArray> m,
          int n, int bc, int i, int j) {
  return m.slice(bc*n+i, bc*n+i+1, j*n, (j+1)*n);
}

IntVarArgs
block_row(MiniModel::Matrix<IntVarArray> m,
          int n, int br, int i, int j) {
  return m.slice(j*n, (j+1)*n, br*n+i, br*n+i+1);
}
#endif

/**
 * \brief %Example: Some %Sudoku puzzles
 *
 * Does not really require any explanation...
 *
 * \ingroup ExProblem
 *
 */
class Sudoku : public Example {
protected:
  const int n;
  IntVarArray x;
public:

  /// Actual model
  Sudoku(const Options& opt)
    : n(example_size(examples[opt.size])),
      x(this,n*n*n*n,1,n*n) {
    const int nn = n*n;
    MiniModel::Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(this, m.row(i), opt.icl);
      distinct(this, m.col(i), opt.icl);
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n)
      for (int j=0; j<nn; j+=n) {
        distinct(this, m.slice(i, i+n, j, j+n), opt.icl);
      }

#ifdef GECODE_HAVE_SET_VARS
    if (!opt.naive) {
      // Implied constraints linking squares and rows
      for (int b=0; b<n; b++) {
        int b1c = 0;
        int b2c = 0;
        IntVarArgs bc1(nn-n);
        IntVarArgs bc2(nn-n);
         IntVarArgs br1(nn-n);
         IntVarArgs br2(nn-n);
        for (int i=0; i<n; i++)
          for (int j=0; j<n; j++) {
            b1c = 0; b2c = 0;
            for (int k=0; k<n; k++) {
              if (k != j) {
                IntVarArgs bc1s = block_col(m, n, b, i, k);
                IntVarArgs br1s = block_row(m, n, b, i, k);
                for (int count=0; count<n; count++) {
                  bc1[b1c] = bc1s[count];
                  br1[b1c] = br1s[count];
                  ++b1c;
                }
              }
              if (k != i) {
                IntVarArgs bc2s = block_col(m, n, b, k, j);
                IntVarArgs br2s = block_row(m, n, b, k, j);
                for (int count=0; count<n; count++) {
                  bc2[b2c] = bc2s[count];
                  br2[b2c] = br2s[count];
                  ++b2c;
                }
              }
            }
            same(this, nn, bc1, bc2);
            same(this, nn, br1, br2);
          }
      }
    }
#endif

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = value_at(examples[opt.size], nn, i, j))
          rel(this, m(i,j), IRT_EQ, v );

    branch(this, x, BVAR_SIZE_MIN, BVAL_SPLIT_MIN);
  }

  /// Constructor for cloning \a s
  Sudoku(bool share, Sudoku& s) : Example(share,s), n(s.n) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Sudoku(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "  ";
    for (int i = 0; i<n*n*n*n; i++) {
      if (x[i].assigned()) {
        if (x[i].val()<10)
          std::cout << x[i] << " ";
        else
          std::cout << (char)(x[i].val()+'A'-10) << " ";        
      }
      else
        std::cout << ". ";
      if((i+1)%(n*n) == 0)
        std::cout << std::endl << "  ";
    }
    std::cout << std::endl;
  }
};


/** \brief Main-function
 *  \relates Sudoku
 */
int
main(int argc, char** argv) {
  Options opt("Sudoku");
  opt.size       = 0;
  opt.icl        = ICL_DOM;
  opt.solutions  = 1;
  opt.naive      = true;
  opt.parse(argc,argv);
  if (opt.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<Sudoku,DFS>(opt);
  return 0;
}


// STATISTICS: example-any
