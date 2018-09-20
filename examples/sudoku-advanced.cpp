/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
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

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

#include <string>
#include <cmath>
#include <cctype>

using namespace Gecode;

#include <examples/sudoku-instances.hh>

/// Base class for %Sudoku puzzles
class Sudoku : public Script {
protected:
  /// The size of the problem
  const int n;
public:
#ifdef GECODE_HAS_SET_VARS
  /// Model variants
  enum {
    MODEL_INT,   ///< Use integer constraints
    MODEL_SET,   ///< Use set constraints
    MODEL_MIXED  ///< Use both integer and set constraints
  };
#endif
  // Branching variants
  enum {
    BRANCH_NONE,        ///< Use lexicographic ordering
    BRANCH_SIZE,        ///< Use minimum size
    BRANCH_SIZE_DEGREE, ///< Use minimum size over degree
    BRANCH_SIZE_AFC,    ///< Use minimum size over afc
    BRANCH_AFC          ///< Use maximum afc
  };

  /// Constructor
  Sudoku(const SizeOptions& opt)
    : Script(opt),
      n(example_size(examples[opt.size()])) {}

  /// Constructor for cloning \a s
  Sudoku(Sudoku& s) : Script(s), n(s.n) {}

};

/**
 * \brief %Example: Solving %Sudoku puzzles using integer constraints
 *
 * \ingroup Example
 */
class SudokuInt : virtual public Sudoku {
protected:
  /// Values for the fields
  IntVarArray x;
public:
#ifdef GECODE_HAS_SET_VARS
  /// Propagation variants
  enum {
    PROP_NONE, ///< No additional constraints
    PROP_SAME, ///< Use "same" constraint with integer model
  };
#endif
  /// Constructor
  SudokuInt(const SizeOptions& opt)
    : Sudoku(opt), x(*this, n*n*n*n, 1, n*n) {
    const int nn = n*n;
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.ipl());
      distinct(*this, m.col(i), opt.ipl());
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n) {
      for (int j=0; j<nn; j+=n) {
        distinct(*this, m.slice(i, i+n, j, j+n), opt.ipl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = sudokuField(examples[opt.size()], nn, i, j))
          rel(*this, m(i,j), IRT_EQ, v );

#ifdef GECODE_HAS_SET_VARS
    if (opt.propagation() == PROP_SAME) {
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
                IntVarArgs bc1s = block_col(m, b, i, k);
                IntVarArgs br1s = block_row(m, b, i, k);
                for (int count=0; count<n; count++) {
                  bc1[b1c] = bc1s[count];
                  br1[b1c] = br1s[count];
                  ++b1c;
                }
              }
              if (k != i) {
                IntVarArgs bc2s = block_col(m, b, k, j);
                IntVarArgs br2s = block_row(m, b, k, j);
                for (int count=0; count<n; count++) {
                  bc2[b2c] = bc2s[count];
                  br2[b2c] = br2s[count];
                  ++b2c;
                }
              }
            }
            same(*this, nn, bc1, bc2);
            same(*this, nn, br1, br2);
          }
      }
    }
#endif
    if (opt.branching() == BRANCH_NONE) {
      branch(*this, x, INT_VAR_NONE(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
      branch(*this, x, INT_VAR_DEGREE_SIZE_MAX(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, x, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, x, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
    }
  }

  /// Constructor for cloning \a s
  SudokuInt(SudokuInt& s) : Sudoku(s) {
    x.update(*this, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new SudokuInt(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "  ";
    for (int i = 0; i<n*n*n*n; i++) {
      if (x[i].assigned()) {
        if (x[i].val()<10)
          os << x[i] << " ";
        else
          os << (char)(x[i].val()+'A'-10) << " ";
      }
      else
        os << ". ";
      if((i+1)%(n*n) == 0)
        os << std::endl << "  ";
    }
    os << std::endl;
  }

#ifdef GECODE_HAS_SET_VARS
private:
  /// Post the constraint that \a a and \a b take the same values
  void same(Space& home, int nn, IntVarArgs a, IntVarArgs b) {
    SetVar u(home, IntSet::empty, 1, nn);
    rel(home, SOT_DUNION, a, u);
    rel(home, SOT_DUNION, b, u);
  }

  /// Extract column \a bc from block starting at (\a i,\a j)
  IntVarArgs
  block_col(Matrix<IntVarArray> m, int bc, int i, int j) {
    return m.slice(bc*n+i, bc*n+i+1, j*n, (j+1)*n);
  }

  /// Extract row \a br from block starting at (\a i,\a j)
  IntVarArgs
  block_row(Matrix<IntVarArray> m, int br, int i, int j) {
    return m.slice(j*n, (j+1)*n, br*n+i, br*n+i+1);
  }
#endif
};

#ifdef GECODE_HAS_SET_VARS
/**
 * \brief %Example: Solving %Sudoku puzzles using set constraints
 *
 * \ingroup Example
 */
class SudokuSet : virtual public Sudoku {
protected:
  /// The fields occupied by a certain number
  SetVarArray y;
public:
  /// Constructor
  SudokuSet(const SizeOptions& opt)
    : Sudoku(opt),
      y(*this,n*n,IntSet::empty,1,n*n*n*n,
        static_cast<unsigned int>(n*n),static_cast<unsigned int>(n*n)) {

    const int nn = n*n;

    Region r;
    IntSet* row = r.alloc<IntSet>(nn);
    IntSet* col = r.alloc<IntSet>(nn);
    IntSet* block = r.alloc<IntSet>(nn);

    // Set up the row and column set constants
    int* dsc = r.alloc<int>(nn);
    for (int i=0; i<nn; i++) {
      row[i] = IntSet((i*nn)+1, (i+1)*nn);

      for (int j=0; j<nn; j++) {
        dsc[j] = (j*nn)+1+i;
      }
      col[i] = IntSet(dsc, nn);
    }

    // Set up the block set constants
    int* dsb_arr = r.alloc<int>(nn);
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {

        for (int ii=0; ii<n; ii++) {
          for (int jj=0; jj<n; jj++) {
            dsb_arr[ii*n+jj] = j*nn*n+i*n+jj*nn+ii+1;
          }
        }
        block[i*n+j] = IntSet(dsb_arr, nn);
      }
    }

    IntSet full(1, nn*nn);
    // All x must be pairwise disjoint and partition the field indices
    rel(*this, SOT_DUNION, y, SetVar(*this, full, full));

    // The x must intersect in exactly one element with each
    // row, column, and block
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++) {
        SetVar inter_row(*this, IntSet::empty, full, 1U, 1U);
        rel(*this, y[i], SOT_INTER, row[j], SRT_EQ, inter_row);
        SetVar inter_col(*this, IntSet::empty, full, 1U, 1U);
        rel(*this, y[i], SOT_INTER, col[j], SRT_EQ, inter_col);
        SetVar inter_block(*this, IntSet::empty, full, 1U, 1U);
        rel(*this, y[i], SOT_INTER, block[j], SRT_EQ, inter_block);
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int idx = sudokuField(examples[opt.size()], nn, i, j))
          dom(*this, y[idx-1], SRT_SUP, (i+1)+(j*nn) );

    if (opt.branching() == BRANCH_NONE) {
      branch(*this, y, SET_VAR_NONE(), SET_VAL_MIN_INC());
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, y, SET_VAR_SIZE_MIN(), SET_VAL_MIN_INC());
    } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
      branch(*this, y, SET_VAR_DEGREE_SIZE_MAX(), SET_VAL_MIN_INC());
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, y, SET_VAR_AFC_SIZE_MAX(opt.decay()), SET_VAL_MIN_INC());
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, y, SET_VAR_AFC_MAX(opt.decay()), SET_VAL_MIN_INC());
    }
  }

  /// Constructor for cloning \a s
  SudokuSet(SudokuSet& s) : Sudoku(s) {
    y.update(*this, s.y);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new SudokuSet(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      for (int j=0; j<n*n; j++) {
        if (y[j].contains(i+1)) {
          if (j+1<10)
            os << j+1 << " ";
          else
            os << (char)(j+1+'A'-10) << " ";
          break;
        }
      }
      if((i+1)%(n*n) == 0)
        os << std::endl << '\t';
    }
    os << std::endl;
  }
};


/**
 * \brief %Example: Solving %Sudoku puzzles using both set and integer
 * constraints
 *
 * \ingroup Example
 */
class SudokuMixed : public SudokuInt, public SudokuSet {
public:
  /// Constructor
  SudokuMixed(const SizeOptions& opt)
  : Sudoku(opt), SudokuInt(opt), SudokuSet(opt) {
    const int nn = n*n;

    IntSet is0(0,0);
    SetVar dummySet0(*this, is0, is0);
    IntVar dummyInt0(*this, 0, 0);
    SetVarArgs ys(nn+1);
    ys[0] = dummySet0;
    for (int i=0; i<nn; i++)
      ys[i+1] = y[i];
    IntVarArgs xs(nn*nn+1);
    xs[0] = dummyInt0;
    for (int i=0; i<nn*nn; i++)
      xs[i+1] = x[i];

    channel(*this, xs, ys);

    IntArgs values(nn);
    for (int i=0; i<nn; i++)
      values[i] = i+1;
    count(*this, x, IntSet(nn,nn), values, IPL_DOM);
  }

  /// Constructor for cloning \a s
  SudokuMixed(SudokuMixed& s)
  : Sudoku(s), SudokuInt(s), SudokuSet(s) {}

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new SudokuMixed(*this);
  }

  /// Print solution
  virtual void print(std::ostream& os) const { SudokuInt::print(os); }

};

#endif

/** \brief Main-function
 *  \relates Sudoku
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Sudoku");
  opt.size(0);
  opt.ipl(IPL_DOM);
  opt.solutions(1);
#ifdef GECODE_HAS_SET_VARS
  opt.model(Sudoku::MODEL_INT);
  opt.model(Sudoku::MODEL_INT, "int", "use integer constraints");
  opt.model(Sudoku::MODEL_SET, "set", "use set constraints");
  opt.model(Sudoku::MODEL_MIXED, "mixed",
            "use both integer and set constraints");
  opt.propagation(SudokuInt::PROP_NONE);
  opt.propagation(SudokuInt::PROP_NONE, "none", "no additional constraints");
  opt.propagation(SudokuInt::PROP_SAME, "same",
                  "additional \"same\" constraint for integer model");
#endif
  opt.branching(Sudoku::BRANCH_SIZE_AFC);
  opt.branching(Sudoku::BRANCH_NONE, "none", "none");
  opt.branching(Sudoku::BRANCH_SIZE, "size", "min size");
  opt.branching(Sudoku::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
  opt.branching(Sudoku::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
  opt.branching(Sudoku::BRANCH_AFC, "afc", "maximum afc");
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
#ifdef GECODE_HAS_SET_VARS
  switch (opt.model()) {
  case Sudoku::MODEL_INT:
    Script::run<SudokuInt,DFS,SizeOptions>(opt);
    break;
  case Sudoku::MODEL_SET:
    Script::run<SudokuSet,DFS,SizeOptions>(opt);
    break;
  case Sudoku::MODEL_MIXED:
    Script::run<SudokuMixed,DFS,SizeOptions>(opt);
    break;
  }
#else
  Script::run<SudokuInt,DFS,SizeOptions>(opt);
#endif
  return 0;
}

// STATISTICS: example-any
