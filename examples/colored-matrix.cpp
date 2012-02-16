/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2012
 *
 *  Last modified:
 *     $Date: 2011-05-11 12:44:17 +0200 (Wed, 11 May 2011) $ by $Author: tack $
 *     $Revision: 12001 $
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

#include <fstream>

using namespace Gecode;

/** \brief %ColoredMatrixOptions.
 *
 * Used in the \ref ColoredMatrix example.
 */
class ColoredMatrixOptions : public Options {
private:
  Driver::UnsignedIntOption _height;       ///< Height of matrix
  Driver::UnsignedIntOption _width;        ///< Width of matrix
  Driver::UnsignedIntOption _size;         ///< Size of square matrix
  Driver::UnsignedIntOption _colors;       ///< Number of colors to use
  Driver::StringOption _not_all_equal;     ///< How to implement the not all equal constraint
  Driver::StringOption _same_or_0;         ///< How to implement the same or 0 constraint
  Driver::StringOption _distinct_except_0; ///< How to implement the distinct except 0 constraint

public:
  /// Initialize options for example with name \a n
  ColoredMatrixOptions(const char* n);

  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
  }

  /// Return height
  unsigned int height(void) const { 
    if (_size.value() == 0)
      return _height.value(); 
    else
      return _size.value(); 
  }
  /// Return width
  unsigned int width(void)  const { 
    if (_size.value() == 0)
      return _width.value(); 
    else
      return _size.value(); 
  }
  /// Return number of colors
  unsigned int colors(void) const { return _colors.value(); }
  /// Return how to implement not all equals
  int not_all_equal(void) const { return _not_all_equal.value(); }
  /// Return how to implement same or 0
  int same_or_0(void) const { return _same_or_0.value(); }
  /// Return how to implement distinct except 0
  int distinct_except_0(void) const { return _distinct_except_0.value(); }
};

namespace {
  /** \name Constraint description constructors.
   *
   * These functions implement constructors for descriptions of
   * constraints as extensional specifications.
   *
   * \relates ColoredMatrix
   */
  //@{
  
  /** Return DFA for the same_or_0 constraint.
   *
   * Constraint models the expression \f$(x = y \land z = y) \lor (x
   * \neq y \land z = 0)\f$ for variables \f$\langle x, y,
   * zq\rangle\f$.
   */
  DFA same_or_0_dfa(unsigned int colors);
  
  /** Return tuple set for the same_or_0 constraint.
   *
   * Constraint models the expression \f$(x = y \land z = y) \lor (x
   * \neq y \land z = 0)\f$ for variables \f$\langle x, y,
   * zq\rangle\f$.
   */
  TupleSet same_or_0_tuple_set(unsigned int colors);
  
  /** Return DFA for the distinct_except_0 constraint.
   */
  DFA distinct_except_0_dfa(unsigned int colors);
  
  /** Return counts for using a global cardninality constraint for the distinct exept 0 constraint.
   */
  IntSetArgs distinct_except_0_counts(unsigned int colors, unsigned int size);
  
  //@}
}

/**
 * \brief %Example: Colored matrix example.
 * 
 * An n by m matrix is to be filled with k colors. It is a valid colored matrix iff
 * the corners of each rectangle do not have the same color.
 *
 * An example 5 by 4 matrix with three colors:
 * \code
 *   1 1 1 1 1 
 *   1 2 2 3 3 
 *   1 2 3 2 3 
 *   1 2 3 3 2 
 * \endcode
 *
 * \ingroup Example
 *
 */
class ColoredMatrix : public Script {
protected:
  /** \name Instance specification
   */
  //@{
  const ColoredMatrixOptions& opt; ///< Options for model
  const unsigned int height;  ///< Height of matrix
  const unsigned int width;   ///< Width of matrix
  const unsigned int colors;  ///< Number of colors to use
  //@}

  /** \name Problem variables
   */
  //@{
  /// Array for matrix variables
  IntVarArray x;
  //@}

  /** Return variable that is zero if a and b differ, or equal to their value if they agree.
   */
  IntVar same_or_0(const IntVar& a, const IntVar& b)
  {
    switch (opt.same_or_0()) {
    case SAME_OR_0_REIFIED: {
      IntVar result(*this, 0, colors);
      BoolVar same = expr(*this, (a == b));
      rel(*this, same == (result == a));
      rel(*this, !same == (result == 0));      
      return result;
    }
    case SAME_OR_0_TUPLE_SET: {
      static TupleSet table = same_or_0_tuple_set(colors);
      IntVar result(*this, 0, colors);
      extensional(*this, IntVarArgs() << a << b << result, table);
      return result;
    }
    case SAME_OR_0_DFA: {
      static const DFA automaton = same_or_0_dfa(colors);
      IntVar result(*this, 0, colors);
      extensional(*this, IntVarArgs() << a << b << result, automaton);
      return result;
    }
    default:
      GECODE_NEVER;
      return IntVar(*this, 0, 0);
    }
  }

  /** Post constraint that all values in v different from 0 are distinct.
   */
  void distinct_except_0(const IntVarArgs& v)
  {
    switch (opt.distinct_except_0()) {
    case DISTINCT_EXCEPT_0_REIFIED:
      for (int i = v.size(); i--; ) {
        for (int j = i; j--; ) {
          rel(*this, (v[i] == 0) || (v[j] == 0) || (v[i] != v[j]));
        }
      }
      break;
    case DISTINCT_EXCEPT_0_DFA: {
      static const DFA automaton = distinct_except_0_dfa(colors);
      extensional(*this, v, automaton);
      break;
    }
    case DISTINCT_EXCEPT_0_COUNT: {
      static const IntSetArgs counts = distinct_except_0_counts(colors, std::max(width, height)); 
      count(*this, v, counts, opt.icl());
      break;
    }
    }
  }

  /** Post constraint that not all variables in v are equal.
   */
  void not_all_equal(const IntVarArgs& v)
  {
    switch (opt.not_all_equal()) {
    case NOT_ALL_EQUAL_REIFIED:
      rel(*this, 
          (v[0] != v[1]) || 
          (v[0] != v[2]) || 
          (v[0] != v[3]) || 
          (v[1] != v[2]) || 
          (v[1] != v[3]) || 
          (v[2] != v[3]));
      break;
    case NOT_ALL_EQUAL_NVALUES:
      nvalues(*this, v, IRT_GR, 1);
    }
  }

  /** Post constraint using same_or_0 and distinct_except_0 that enforces
   * rows/columns v1 and v2 to have no monochrome rectangle.
   */
  void no_monochrome_rectangle(IntVarArgs v1, IntVarArgs v2) {
    const unsigned int length = v1.size();
    IntVarArgs z(length);
    for (unsigned int i = 0; i < length; ++i) {
      z[i] = same_or_0(v1[i], v2[i]);
    }
    distinct_except_0(z);
  }


public:
  /// Branching variants
  enum {
    SYMMETRY_NONE,   ///< No symmetry breaking
    SYMMETRY_MATRIX, ///< Order rows and columns of matrix
    SYMMETRY_VALUES, ///< Order value occurences
    SYMMETRY_BOTH,   ///< Combine row/column order with value precedence
  };
  /// Model variants
  enum {
    MODEL_CORNERS,   ///< Use model on corner combinations
    MODEL_ROWS,      ///< Use model on pairs of rows 
    MODEL_COLUMNS,   ///< Use model on pairs of columns 
    MODEL_MATRIX,    ///< Use model on pairs of rows and pairs of columns
  };
  /// Not all equal variants
  enum {
    NOT_ALL_EQUAL_REIFIED, ///< Use reification for implemeting not all equals
    NOT_ALL_EQUAL_NVALUES, ///< Use nvalues for implementing not all equals
  };
  /// Same or 0 variants
  enum {
    SAME_OR_0_REIFIED,   ///< Use reification for same or 0
    SAME_OR_0_DFA,       ///< Use dfa for same or 0
    SAME_OR_0_TUPLE_SET, ///< Use tuple set for same or 0
  };
  /// Distinct except 0 variants
  enum {
    DISTINCT_EXCEPT_0_REIFIED, ///< Use reification for distinct except 0
    DISTINCT_EXCEPT_0_DFA,     ///< Use dfa for distinct except 0
    DISTINCT_EXCEPT_0_COUNT,   ///< Use count for distinct except 0
  };


  /// Actual model
  ColoredMatrix(const ColoredMatrixOptions& opt0)
    : opt(opt0), height(opt.height()), width(opt.width()), colors(opt.colors()),
      x(*this, height*width, 1, colors)
  {
    Matrix<IntVarArray> m(x, width, height);
    
    switch (opt.model()) {
    case MODEL_CORNERS: {
      // For each choice of corners in a rectangle, the variables should not be all equal.
      for (unsigned int c1 = 0; c1 < width; ++c1) {
        for (unsigned int c2 = c1+1; c2 < width; ++c2) {
          for (unsigned int r1 = 0; r1 < height; ++r1) {
            for (unsigned int r2 = r1+1; r2 < height; ++r2) {
              not_all_equal(IntVarArgs() << m(c1,r1) << m(c1,r2) << m(c2,r1) << m(c2,r2));
            }
          }
        }
      }
      break;
    }
    case MODEL_ROWS:
    case MODEL_COLUMNS:
    case MODEL_MATRIX: {
      // Given two rows/columns, construct variables representing if
      // the corresponding places are equal (and if so, what value).
      // For the new values, no non-zero value may appear more than
      // once.
      if (opt.model() == MODEL_ROWS || opt.model() == MODEL_MATRIX) {
        for (unsigned int r1 = 0; r1 < height; ++r1) {
          for (unsigned int r2 = r1+1; r2 < height; ++r2) {
            no_monochrome_rectangle(m.row(r1), m.row(r2));
          }
        }
      }
      if (opt.model() == MODEL_COLUMNS || opt.model() == MODEL_MATRIX) {
        for (unsigned int c1 = 0; c1 < width; ++c1) {
          for (unsigned int c2 = c1+1; c2 < width; ++c2) {
            no_monochrome_rectangle(m.col(c1), m.col(c2));
          }
        }
      }
      break;
    }
    }

    // Symmetry breaking constraints.
    if (opt.symmetry() != SYMMETRY_NONE) {
      // Lexical order for all columns and rows (all are interchangable)
      if (opt.symmetry() == SYMMETRY_MATRIX || opt.symmetry() == SYMMETRY_BOTH) {
        for (unsigned int r = 0; r < height-1; ++r) {
          rel(*this, m.row(r), IRT_LE, m.row(r+1));
        }
        for (unsigned int c = 0; c < width-1; ++c) {
          rel(*this, m.col(c), IRT_LE, m.col(c+1));
        }
      }

      // Value precedence. Compatible with row/column ordering
      if (opt.symmetry() == SYMMETRY_VALUES || opt.symmetry() == SYMMETRY_BOTH) {
        precede(*this, x, IntArgs::create(colors, 1));
      }      
    }

    branch(*this, x, tiebreak(INT_VAR_MIN_MIN, INT_VAR_SIZE_MIN), INT_VAL_MIN);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m(x, width, height);
    for (unsigned int r = 0; r < height; ++r) {
      os << "\t";
      for (unsigned int c = 0; c < width; ++c) {
        os << m(c, r) << " ";
      }
      os << std::endl;
    }
    os << std::endl;
  }

  /// Constructor for cloning \a s
  ColoredMatrix(bool share, ColoredMatrix& s)
    : Script(share,s), opt(s.opt),
      height(s.height), width(s.width), colors(s.colors) {
    x.update(*this, share, s.x);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new ColoredMatrix(share,*this);
  }
};

ColoredMatrixOptions::ColoredMatrixOptions(const char* n)
  : Options(n),
    _height("-height", "Height of matrix", 6),
    _width("-width", "Width of matrix", 6),
    _size("-size", "If different from 0, used as both width and height", 0),
    _colors("-colors", "Number of colors", 4),
    _not_all_equal("-not-all-equal", "How to implement the not all equals constraint (used in corners model)", 
                   ColoredMatrix::NOT_ALL_EQUAL_NVALUES),
    _same_or_0("-same-or-0", "How to implement the same or 0 constraint (used in the rows model)", 
               ColoredMatrix::SAME_OR_0_DFA),
    _distinct_except_0("-distinct-except-0", "How to implement the distinct except 0 constraint (used in the rows model)", 
                       ColoredMatrix::DISTINCT_EXCEPT_0_DFA)
{
  add(_height);
  add(_width);
  add(_size);
  add(_colors);
  add(_not_all_equal);
  add(_same_or_0);
  add(_distinct_except_0);
  
  // Add symmetry options
  _symmetry.add(ColoredMatrix::SYMMETRY_NONE,  "none", "Don't use symmetry breaking.");
  _symmetry.add(ColoredMatrix::SYMMETRY_MATRIX,  "matrix", "Order matrix rows and columns");
  _symmetry.add(ColoredMatrix::SYMMETRY_VALUES,  "values", "Order values");
  _symmetry.add(ColoredMatrix::SYMMETRY_BOTH,  "both", "Order both rows/columns and values");
  _symmetry.value(ColoredMatrix::SYMMETRY_BOTH);

  // Add model options
  _model.add(ColoredMatrix::MODEL_CORNERS,  "corner", "Use direct corners model with not-all-equal constraints.");
  _model.add(ColoredMatrix::MODEL_ROWS,  "rows", "Use model on pairs of rows (same_or_0 and distinct_except_0 constraints).");
  _model.add(ColoredMatrix::MODEL_COLUMNS,  "columns", "Use model on pairs of columns (same_or_0 and distinct_except_0 constraints).");
  _model.add(ColoredMatrix::MODEL_MATRIX,  "matrix", "Use both rows and columns model");
  _model.value(ColoredMatrix::MODEL_ROWS);

  // Add not all equal variants
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_REIFIED, "reified", "Use reified decomposition.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_NVALUES, "nvalues", "Use nvalues.");

  // Add same or 0 variants
  _same_or_0.add(ColoredMatrix::SAME_OR_0_REIFIED, "reified", "Use reified decomposition.");
  _same_or_0.add(ColoredMatrix::SAME_OR_0_TUPLE_SET, "tuple-set", "Use tuple set representation.");
  _same_or_0.add(ColoredMatrix::SAME_OR_0_DFA, "dfa", "Use dfa representation.");

  // Add distinct except 0 variants
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_REIFIED, "reified", "Use reified decomposition.");
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_DFA, "dfa", "Use dfa decomposition.");
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_COUNT, "count", "Use global cardinality.");
}

/** \brief Main-function
 *  \relates ColoredMatrix
 */
int
main(int argc, char* argv[]) {
  ColoredMatrixOptions opt("Colored matrix");
  opt.parse(argc,argv);
  Script::run<ColoredMatrix,DFS,ColoredMatrixOptions>(opt);
  return 0;
}


namespace {
  DFA same_or_0_dfa(unsigned int colors)
  {
    /* DFA over variable sequences (x,y,z) where z equals x/y if x and
     * y are equal, and z equals 0 otherwise.
     * 
     * DFA is constructed to contain paths 
     *   start -- c --> node -- c --> node' -- c --> end
     * for all colors c representing the case when x and y 
     * are equal.
     *
     * For the case where x and y are non-equal (c and c'), paths
     *   start -- c --> node -- c' --> not-equal -- 0 --> end
     * are added.
     */

    const int start_state = 0;
    const int not_equal_state = 2*colors+1;
    const int final_state = 2*colors+2;

    int n_transitions = colors*colors + 2*colors + 2;
    DFA::Transition* trans = new DFA::Transition[n_transitions];
    int current_transition = 0;
    
    // From start state
    for (unsigned int color = 1; color <= colors; ++color) {
      trans[current_transition++] =
        DFA::Transition(start_state, color, color);
    }

    // From first level states (indices [1..color])
    // To second-level if same color, otherwise to not_equal_state
    for (unsigned int state = 1; state <= colors; ++state) {
      for (unsigned int color = 1; color <= colors; ++color) {
        if (color == state) {
          trans[current_transition++] =
            DFA::Transition(state, color, colors+state);
        } else {          
          trans[current_transition++] =
            DFA::Transition(state, color, not_equal_state);
        }
      }
    }

    // From second level states (indices [colors+1..colors+colors])
    // To final state with the same color
    for (unsigned int color = 1; color <= colors; ++color) {
      trans[current_transition++] =
        DFA::Transition(colors+color, color, final_state);
    }

    // From not equal state to final state
    trans[current_transition++] =
      DFA::Transition(not_equal_state, 0, final_state);
    
    // End sentinel
    trans[current_transition++] =
      DFA::Transition(-1, 0, -1);
    
    int* final_states = new int[2];
    final_states[0] = final_state;
    final_states[1] = -1;
    
    DFA result(start_state, trans, final_states, true);

    delete[] trans;
    delete[] final_states;

    return result;
  }
  
  TupleSet same_or_0_tuple_set(unsigned int colors)
  {
    TupleSet result;
    for (unsigned int i = 1; i <= colors; ++i) {
      for (unsigned int j = 1; j <= colors; ++j) {
        if (i == j) {
          result.add(IntArgs(3, i, j, i));
        } else {
          result.add(IntArgs(3, i, j, 0));
        }
      }
    }
    result.finalize();
    return result;
  }  

  DFA distinct_except_0_dfa(unsigned int colors)
  {
    /* DFA for a sequence that may use each color only once (and all
     * others are zero).
     * 
     * For n colors, 2^n nodes are used. For each node, if bit b is one, then 
     * that color has not been used yet. All nodes have self-loops for zero, and 
     * edges for still usable colors to the node with the corresponding bit un-set. 
     * All nodes are final nodes.
     */

    const int nstates = 1 << colors;
    const int start_state = nstates-1;

    DFA::Transition* trans = new DFA::Transition[nstates*colors + 1];
    int current_transition = 0;
    for (int state = start_state+1; state--; ) {
      trans[current_transition++] = DFA::Transition(state, 0, state);

      for (unsigned int color = 1; color <= colors; ++color) {
        const unsigned int color_bit = (1 << (color-1));
        if (state & color_bit) {
          trans[current_transition++] = 
            DFA::Transition(state, color, state & ~color_bit);
        }
      }
    }
    trans[current_transition++] = DFA::Transition(-1, 0, -1);

    int* final_states = new int[nstates+1];
    final_states[nstates] = -1;
    for (int i = nstates; i--; ) {
      final_states[i] = i;
    }

    DFA result(start_state, trans, final_states, false);

    delete[] trans;
    delete[] final_states;

    return result;
  }

  IntSetArgs distinct_except_0_counts(unsigned int colors, unsigned int size)
  {
    IntSetArgs result(colors+1);

    result[0] = IntSet(0, size);

    for (unsigned int i = 1; i <= colors; ++i) {
      result[i] = IntSet(0, 1);
    }

    return result;
  }
}


// STATISTICS: example-any
