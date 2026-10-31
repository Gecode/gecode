/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2005
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <algorithm>
#include <string>
#include <cmath>
#include <cctype>
#include <cstring>

using namespace Gecode;

#include <examples/sudoku-instances.hh>

/**
 * \brief Base class for %Sudoku puzzles
 *
 * The instance is either the number of a built-in instance or a file in
 * the format of the scaled sudoku corpus: a line such as
 * <tt># size=6 box=3x2</tt>, then the rows of the grid with \c . for
 * blank cells and the symbols \c 1-9, \c A-Z, and \c 0 for the values 1
 * to 36.
 */
class Sudoku : public Script {
protected:
  /// The side of the grid
  int nn;
  /// The width of a box
  int bw;
  /// The height of a box
  int bh;
  /// Given values in row-major order (0 for blank)
  IntSharedArray givens;
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
  Sudoku(const InstanceOptions& opt)
    : Script(opt) {
    SudokuInstance si(opt);
    nn = si.nn; bw = si.bw; bh = si.bh;
    givens = IntSharedArray(si.givens);
  }

  /// Constructor for cloning \a s
  Sudoku(Sudoku& s)
    : Script(s), nn(s.nn), bw(s.bw), bh(s.bh), givens(s.givens) {}

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
  SudokuInt(const InstanceOptions& opt)
    : Sudoku(opt), x(*this, nn*nn, 1, nn) {
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.ipl());
      distinct(*this, m.col(i), opt.ipl());
    }

    // Constraints for boxes
    for (int i=0; i<nn; i+=bw) {
      for (int j=0; j<nn; j+=bh) {
        distinct(*this, m.slice(i, i+bw, j, j+bh), opt.ipl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = givens[j*nn+i])
          rel(*this, m(i,j), IRT_EQ, v );

#ifdef GECODE_HAS_SET_VARS
    if (opt.propagation() == PROP_SAME) {
      // Implied constraints linking boxes with columns and rows: the
      // fields of a line outside a box take the values that the fields of
      // the box outside the line take
      for (int l=0; l<nn; l++)
        for (int j=0; j<std::max(bw,bh); j++) {
          if (j < bw)
            same(*this, nn, column_outside_box(m, l, j), box_outside_column(m, l, j));
          if (j < bh)
            same(*this, nn, row_outside_box(m, l, j), box_outside_row(m, l, j));
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
    for (int i = 0; i<nn*nn; i++) {
      if (x[i].assigned()) {
        print_value(os, x[i].val());
        os << " ";
      }
      else
        os << ". ";
      if((i+1)%nn == 0)
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

  /// The fields of column \a c outside box row \a j
  IntVarArgs
  column_outside_box(const Matrix<IntVarArray>& m, int c, int j) const {
    IntVarArgs a;
    for (int k=0; k<bw; k++)
      if (k != j)
        a << m.slice(c, c+1, k*bh, (k+1)*bh);
    return a;
  }

  /// The fields of the box in box row \a j of column \a c outside column \a c
  IntVarArgs
  box_outside_column(const Matrix<IntVarArray>& m, int c, int j) const {
    IntVarArgs a;
    int first = (c / bw) * bw;
    for (int k=first; k<first+bw; k++)
      if (k != c)
        a << m.slice(k, k+1, j*bh, (j+1)*bh);
    return a;
  }

  /// The fields of row \a r outside box column \a j
  IntVarArgs
  row_outside_box(const Matrix<IntVarArray>& m, int r, int j) const {
    IntVarArgs a;
    for (int k=0; k<bh; k++)
      if (k != j)
        a << m.slice(k*bw, (k+1)*bw, r, r+1);
    return a;
  }

  /// The fields of the box in box column \a j of row \a r outside row \a r
  IntVarArgs
  box_outside_row(const Matrix<IntVarArray>& m, int r, int j) const {
    IntVarArgs a;
    int first = (r / bh) * bh;
    for (int k=first; k<first+bh; k++)
      if (k != r)
        a << m.slice(j*bw, (j+1)*bw, k, k+1);
    return a;
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
  SudokuSet(const InstanceOptions& opt)
    : Sudoku(opt),
      y(*this,nn,IntSet::empty,1,nn*nn,
        static_cast<unsigned int>(nn),static_cast<unsigned int>(nn)) {

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

    // Set up the block set constants, for the box in box column i and
    // box row j
    int* dsb_arr = r.alloc<int>(nn);
    for (int i=0; i<bh; i++) {
      for (int j=0; j<bw; j++) {

        for (int ii=0; ii<bw; ii++) {
          for (int jj=0; jj<bh; jj++) {
            dsb_arr[ii*bh+jj] = (j*bh+jj)*nn+i*bw+ii+1;
          }
        }
        block[i*bw+j] = IntSet(dsb_arr, nn);
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
        if (int idx = givens[j*nn+i])
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
    for (int i = 0; i<nn*nn; i++) {
      for (int j=0; j<nn; j++) {
        if (y[j].contains(i+1)) {
          print_value(os, j+1);
          os << " ";
          break;
        }
      }
      if((i+1)%nn == 0)
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
  SudokuMixed(const InstanceOptions& opt)
  : Sudoku(opt), SudokuInt(opt), SudokuSet(opt) {

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
  InstanceOptions opt("Sudoku", instance_spec);
  opt.instance("0");
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
  try {
    if (!SudokuInstance(opt).valid()) {
      std::cerr << "Error: instance must be a number between 0 and " << n_examples-1 << " or a file" << std::endl;
      return 1;
    }
  } catch (const Instance::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
#ifdef GECODE_HAS_SET_VARS
  switch (opt.model()) {
  case Sudoku::MODEL_INT:
    Script::run<SudokuInt,DFS,InstanceOptions>(opt);
    break;
  case Sudoku::MODEL_SET:
    Script::run<SudokuSet,DFS,InstanceOptions>(opt);
    break;
  case Sudoku::MODEL_MIXED:
    Script::run<SudokuMixed,DFS,InstanceOptions>(opt);
    break;
  }
#else
  Script::run<SudokuInt,DFS,InstanceOptions>(opt);
#endif
  return 0;
}

// STATISTICS: example-any
