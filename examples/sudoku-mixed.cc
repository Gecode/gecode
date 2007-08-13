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

#include "gecode/set.hh"
#include "examples/support.hh"
#include "gecode/minimodel.hh"

#include "examples/sudoku.icc"

/**
 * \brief Implements the "same" constraint
 *
 * Posts the constraint \f$\biguplus_i \{a_i\}=\biguplus_i \{b_i\}\f$
 * where both are a subset of \f$1\dots\mathit{nn}\f$
 */
void same(Space* home, int nn, IntVarArgs a, IntVarArgs b) {
  SetVar u(home, IntSet::empty, 1, nn);
  rel(home, SOT_DUNION, a, u);
  rel(home, SOT_DUNION, b, u);
}

MiniModel::Matrix<IntVarArray>::Slice
block_col(MiniModel::Matrix<IntVarArray> m,
          int n, int bc, int i, int j) {
  return m.slice(bc*n+i, bc*n+i+1, j*n, (j+1)*n);
}

MiniModel::Matrix<IntVarArray>::Slice
block_row(MiniModel::Matrix<IntVarArray> m,
          int n, int br, int i, int j) {
  return m.slice(j*n, (j+1)*n, br*n+i, br*n+i+1);
}

/**
 * \brief %Example: Some %Sudoku puzzles using finite domain and set constraints
 *
 * The problem does not really require any explanation...
 *
 * This implementation combines both the finite domain and the finite set
 * model using channelling constraints.
 *
 * \ingroup ExProblem
 *
 */
class SudokuMixed : public Example {
protected:
  const int n;
  SetVarArray x;
public:
  /// Actual model
  SudokuMixed(const SizeOptions& opt)
    : n(example_size(examples[opt.size()])),
      x(this,n*n,IntSet::empty,1,n*n*n*n,9,9) {

    const int nn = n*n;

    /********************************************************
     *
     * Finite Domain model
     *
     */
    IntVarArray y(this,n*n*n*n,1,n*n);

    MiniModel::Matrix<IntVarArray> m(y, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(this, m.row(i), opt.icl());
      distinct(this, m.col(i), opt.icl());
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n)
      for (int j=0; j<nn; j+=n) {
        distinct(this, m.slice(i, i+n, j, j+n), opt.icl());
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = value_at(examples[opt.size()], nn, i, j))
          rel(this, m(i,j), IRT_EQ, v );

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

    /********************************************************
     *
     * Channelling between the models
     *
     */

    IntSet is0(0,0);
    SetVar dummySet0(this, is0, is0);
    IntVar dummyInt0(this, 0, 0);
    SetVarArgs xs(nn+1);
    xs[0] = dummySet0;
    for (int i=0; i<nn; i++)
      xs[i+1] = x[i];
    IntVarArgs ys(nn*nn+1);
    ys[0] = dummyInt0;
    for (int i=0; i<nn*nn; i++)
      ys[i+1] = y[i];

    channel(this, ys, xs);

    gcc(this, y, 9, ICL_DOM);


    /********************************************************
     *
     * Finite set model
     *
     */

    IntSet row[9];
    IntSet col[9];
    IntSet block[9];

    // Set up the row and column set constants
    for (int i=0; i<9; i++) {
      IntSet dsr((i*nn)+1, (i*nn)+9);
      row[i] = dsr;

      int dsc_arr[9] = { 1+i, 10+i, 19+i, 28+i, 37+i, 46+i, 55+i, 64+i, 73+i };
      IntSet dsc(dsc_arr, 9);

      col[i] = dsc;
    }

    // Set up the block set constants
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++) {
        int dsb_arr[9] = {
          (j*27)+(i*3)+1, (j*27)+(i*3)+2, (j*27)+(i*3)+3,
          (j*27)+(i*3)+10, (j*27)+(i*3)+11, (j*27)+(i*3)+12,
          (j*27)+(i*3)+19, (j*27)+(i*3)+20, (j*27)+(i*3)+21
        };
        IntSet dsb(dsb_arr, 9);
         block[i*3+j]=dsb;
      }

    // All x must be pairwise disjoint
    for (int i=0; i<nn-1; i++)
      for (int j=i+1; j<nn; j++)
         rel(this, x[i], SRT_DISJ, x[j]);
    distinct(this, x, nn);

    // The x must intersect in exactly one element with each
    // row, column, and block
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++) {
        SetVar inter_row(this, IntSet::empty, 1, 9*9, 1, 1);
        rel(this, x[i], SOT_INTER, row[j], SRT_EQ, inter_row);
        SetVar inter_col(this, IntSet::empty, 1, 9*9, 1, 1);
        rel(this, x[i], SOT_INTER, col[j], SRT_EQ, inter_col);
        SetVar inter_block(this, IntSet::empty, 1, 9*9, 1, 1);
        rel(this, x[i], SOT_INTER, block[j], SRT_EQ, inter_block);
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int idx = value_at(examples[opt.size()], nn, i, j))
          dom(this, x[idx-1], SRT_SUP, (i+1)+(j*nn) );

    branch(this, x, SETBVAR_NONE, SETBVAL_MIN);
  }

  /// Constructor for cloning \a s
  SudokuMixed(bool share, SudokuMixed& s) : Example(share,s), n(s.n) {
    x.update(this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuMixed(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      for (int j=0; j<n*n; j++) {
        if (x[j].contains(i+1)) {
          if (j+1<10)
            std::cout << j+1 << " ";
          else
            std::cout << (char)(j+1+'A'-10) << " ";        
          break;
        }
      }
      if((i+1)%(n*n) == 0)
        std::cout << std::endl << '\t';
    }
    std::cout << std::endl;
  }

};


/** \brief Main-function
 *  \relates SudokuMixed
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Sudoku (Mixed Model)");
  opt.iterations(200);
  opt.size(0);
  opt.icl(ICL_DOM);
  opt.solutions(1);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  if (example_size(examples[opt.size()]) != 3) {
    std::cerr << "Set-based version only available with exmples of size 9*9"
              << std::endl;
    return 2;
  }
  Example::run<SudokuMixed,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

