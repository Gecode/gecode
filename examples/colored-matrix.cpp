/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2012
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
  /// Height of matrix
  Driver::UnsignedIntOption _height;
  /// Width of matrix
  Driver::UnsignedIntOption _width;
  /// Size of square matrix
  Driver::UnsignedIntOption _size;
  /// Number of colors to use
  Driver::UnsignedIntOption _colors;
  /// How to implement the not all equal constraint
  Driver::StringOption _not_all_equal;
  /// How to implement the same or 0 constraint
  Driver::StringOption _same_or_0;
  /// How to implement the distinct except 0 constraint
  Driver::StringOption _distinct_except_0;
  /// How to implement the no monochrome rectangle constraint
  Driver::StringOption _no_monochrome_rectangle;

public:
  /// Initialize options for example with name \a n
  ColoredMatrixOptions(const char* n);

  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
  }

  /// Return height
  int height(void) const {
    if (_size.value() == 0U)
      return static_cast<int>(_height.value());
    else
      return static_cast<int>(_size.value());
  }
  /// Return width
  int width(void)  const {
    if (_size.value() == 0U)
      return static_cast<int>(_width.value());
    else
      return static_cast<int>(_size.value());
  }
  /// Return number of colors
  int colors(void) const { return static_cast<int>(_colors.value()); }
  /// Return how to implement not all equals
  int not_all_equal(void) const { return _not_all_equal.value(); }
  /// Return how to implement same or 0
  int same_or_0(void) const { return _same_or_0.value(); }
  /// Return how to implement distinct except 0
  int distinct_except_0(void) const { return _distinct_except_0.value(); }
  /// Return how to implement distinct except 0
  int no_monochrome_rectangle(void) const {
    return _no_monochrome_rectangle.value();
  }
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
  DFA same_or_0_dfa(int colors);

  /** Return tuple set for the same_or_0 constraint.
   *
   * Constraint models the expression \f$(x = y \land z = y) \lor (x
   * \neq y \land z = 0)\f$ for variables \f$\langle x, y,
   * zq\rangle\f$.
   */
  TupleSet same_or_0_tuple_set(int colors);

  /** Return DFA for the distinct_except_0 constraint.
   */
  DFA distinct_except_0_dfa(int colors);

  /** Return DFA for the no monochrome rectangle constraint.
   */
  DFA no_monochrome_rectangle_dfa(int colors);

  /** Return counts for using a global cardninality constraint for the distinct exept 0 constraint.
   */
  IntSetArgs distinct_except_0_counts(int colors, int size);

  /** Return DFA for the not all equals constraint.
   */
  DFA not_all_equal_dfa(int colors);

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
class ColoredMatrix : public IntMinimizeScript {
protected:
  /** \name Instance specification
   */
  //@{
  const ColoredMatrixOptions& opt; ///< Options for model
  const int height;  ///< Height of matrix
  const int width;   ///< Width of matrix
  const int colors;  ///< Number of colors to use
  //@}

  /** \name Problem variables
   */
  //@{
  /// Array for matrix variables
  IntVarArray x;
  /// Maximum color used
  IntVar max_color;
  //@}

  /** Return variable that is zero if a and b differ, or equal to their value if they agree.
   */
  IntVar same_or_0(const IntVar& a, const IntVar& b)
  {
    switch (opt.same_or_0()) {
    case SAME_OR_0_REIFIED: {
      IntVar result(*this, 0, colors);
      BoolVar same = expr(*this, (a == b));
      rel(*this, result, IRT_EQ, a, same);
      // Redundant (implied by previous), but improves efficiency
      rel(*this, result, IRT_NQ, 0, same);
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
        BoolVar viIsZero = expr(*this, v[i] == 0);
        for (int j = i; j--; ) {
          rel(*this, viIsZero || (v[i] != v[j]));
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
      count(*this, v, counts, opt.ipl());
      break;
    }
    }
  }

  /** Post constraint that not all variables in v are equal.
   */
  void not_all_equal(const IntVarArgs& v)
  {
    switch (opt.not_all_equal()) {
    case NOT_ALL_EQUAL_NQ: {
      rel(*this, v, IRT_NQ);
      break;
    }
    case NOT_ALL_EQUAL_NAIVE: {
      // At least one pair must be different.
      // Bad decomposition since too many disjuncts are created.
      BoolVarArgs disequalities;
      for (int i = v.size(); i--; )
        for (int j = i; j--; )
          disequalities << expr(*this, v[i] != v[j]);
      rel(*this, BOT_OR, disequalities, 1);
      break;
    }
    case NOT_ALL_EQUAL_REIFIED: {
      // It must not be the case that all are equal
      BoolVarArgs equalities;
      for (int i = 0; i < v.size()-1; ++i)
        equalities << expr(*this, v[i] == v[i+1]);
      rel(*this, BOT_AND, equalities, 0);
      break;
    }
    case NOT_ALL_EQUAL_NVALUES:
      // More than one number
      nvalues(*this, v, IRT_GR, 1);
      break;
    case NOT_ALL_EQUAL_COUNT:
      // No number in all positions
      count(*this, v, IntSet(0, v.size()-1), IntArgs::create(colors, 1), opt.ipl());
      break;
    case NOT_ALL_EQUAL_DFA: {
      static const DFA automaton = not_all_equal_dfa(colors);
      extensional(*this, v, automaton);
      break;
    }
    }
  }

  /** Post constraint using same_or_0 and distinct_except_0 that enforces
   * rows/columns v1 and v2 to have no monochrome rectangle.
   */
  void no_monochrome_rectangle(IntVarArgs v1, IntVarArgs v2) {
    const int length = v1.size();
    switch (opt.no_monochrome_rectangle()) {
    case NO_MONOCHROME_DECOMPOSITION: {
      IntVarArgs z(length);
      for (int i = 0; i < length; ++i) {
        z[i] = same_or_0(v1[i], v2[i]);
      }
      distinct_except_0(z);
      break;
    }
    case NO_MONOCHROME_DFA: {
      static const DFA automaton = no_monochrome_rectangle_dfa(colors);
      IntVarArgs z(2*length);
      for (int i = length; i--; ) {
        z[2*i + 0] = v1[i];
        z[2*i + 1] = v2[i];
      }
      extensional(*this, z, automaton);
      break;
    }
    }
  }


public:
  /// Search modes
  enum {
    SEARCH_DFS, ///< Find solution
    SEARCH_BAB, ///< Find optimal solution
  };
  /// SYmmetry breaking variants
  enum {
    SYMMETRY_NONE   = 0,   ///< No symmetry breaking
    SYMMETRY_MATRIX = 1, ///< Order rows and columns of matrix
    SYMMETRY_VALUES = 2, ///< Order value occurences
  };
  /// Model variants
  enum {
    MODEL_CORNERS = 1,   ///< Use model on corner combinations
    MODEL_ROWS    = 2,      ///< Use model on pairs of rows
    MODEL_COLUMNS = 4,   ///< Use model on pairs of columns
  };
  /// Not all equal variants
  enum {
    NOT_ALL_EQUAL_NQ,      ///< Use direct constraint for implemeting not all equals
    NOT_ALL_EQUAL_NAIVE,   ///< Use naive reification for implemeting not all equals
    NOT_ALL_EQUAL_REIFIED, ///< Use reification for implemeting not all equals
    NOT_ALL_EQUAL_NVALUES, ///< Use nvalues for implementing not all equals
    NOT_ALL_EQUAL_COUNT,   ///< Use count for implementing not all equals
    NOT_ALL_EQUAL_DFA,     ///< Use dfa for implementing not all equals
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
  /// No monochrome rectangle versions
  enum {
    NO_MONOCHROME_DECOMPOSITION, ///< Use decomposition for no monochrome rectangle
    NO_MONOCHROME_DFA,           ///< Use dfa for no monochrome rectangle
  };


  /// Actual model
  ColoredMatrix(const ColoredMatrixOptions& opt0)
    : IntMinimizeScript(opt0),
      opt(opt0), height(opt.height()), width(opt.width()), colors(opt.colors()),
      x(*this, height*width, 1, colors),
      max_color(*this, 1, colors)
  {

    max(*this, x, max_color);

    Matrix<IntVarArray> m(x, width, height);

    // For each pair of columns and rows, the intersections may not be equal.
    if (opt.model() & MODEL_CORNERS) {
      for (int c1 = 0; c1 < width; ++c1) {
        for (int c2 = c1+1; c2 < width; ++c2) {
          for (int r1 = 0; r1 < height; ++r1) {
            for (int r2 = r1+1; r2 < height; ++r2) {
              not_all_equal(IntVarArgs() << m(c1,r1) << m(c1,r2) << m(c2,r1) << m(c2,r2));
            }
          }
        }
      }
    }
    // Given two rows/columns, construct variables representing if
    // the corresponding places are equal (and if so, what value).
    // For the new values, no non-zero value may appear more than
    // once.
    if (opt.model() & MODEL_ROWS) {
      for (int r1 = 0; r1 < height; ++r1) {
        for (int r2 = r1+1; r2 < height; ++r2) {
          no_monochrome_rectangle(m.row(r1), m.row(r2));
        }
      }
    }
    if (opt.model() & MODEL_COLUMNS) {
      for (int c1 = 0; c1 < width; ++c1) {
        for (int c2 = c1+1; c2 < width; ++c2) {
          no_monochrome_rectangle(m.col(c1), m.col(c2));
        }
      }
    }

    // Symmetry breaking constraints.
    {
      // Lexical order for all columns and rows (all are interchangable)
      if (opt.symmetry() & SYMMETRY_MATRIX) {
        for (int r = 0; r < height-1; ++r) {
          rel(*this, m.row(r), IRT_LE, m.row(r+1));
        }
        for (int c = 0; c < width-1; ++c) {
          rel(*this, m.col(c), IRT_LE, m.col(c+1));
        }
      }

      // Value precedence. Compatible with row/column ordering
      if (opt.symmetry() & SYMMETRY_VALUES) {
        precede(*this, x, IntArgs::create(colors, 1));
      }
    }

    branch(*this, x, tiebreak(INT_VAR_MIN_MIN(), INT_VAR_SIZE_MIN()), INT_VAL_MIN());
  }

  /// Return cost
  virtual IntVar cost(void) const {
    return max_color;
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m(x, width, height);
    for (int r = 0; r < height; ++r) {
      os << "\t";
      for (int c = 0; c < width; ++c) {
        os << m(c, r) << " ";
      }
      os << std::endl;
    }
    os << std::endl;
    os << "\tmax color: " << max_color << std::endl;
    os << std::endl;
  }

  /// Constructor for cloning \a s
  ColoredMatrix(ColoredMatrix& s)
    : IntMinimizeScript(s), opt(s.opt),
      height(s.height), width(s.width), colors(s.colors) {
    x.update(*this, s.x);
    max_color.update(*this, s.max_color);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new ColoredMatrix(*this);
  }
};

ColoredMatrixOptions::ColoredMatrixOptions(const char* n)
  : Options(n),
    _height("height", "Height of matrix", 8),
    _width("width", "Width of matrix", 8),
    _size("size", "If different from 0, used as both width and height", 0),
    _colors("colors", "Maximum number of colors", 4),
    _not_all_equal("not-all-equal", "How to implement the not all equals constraint (used in corners model)",
                   ColoredMatrix::NOT_ALL_EQUAL_NQ),
    _same_or_0("same-or-0", "How to implement the same or 0 constraint (used in the decomposed no monochrome rectangle constraint)",
               ColoredMatrix::SAME_OR_0_DFA),
    _distinct_except_0("distinct-except-0", "How to implement the distinct except 0 constraint (used in the decomposed no monochrome rectangle constraint)",
                       ColoredMatrix::DISTINCT_EXCEPT_0_DFA),
    _no_monochrome_rectangle("no-monochrome-rectangle", "How to implement no monochrome rectangle (used in the rows model)",
                             ColoredMatrix::NO_MONOCHROME_DFA)
{
  add(_height);
  add(_width);
  add(_size);
  add(_colors);
  add(_not_all_equal);
  add(_same_or_0);
  add(_distinct_except_0);
  add(_no_monochrome_rectangle);

  // Add search options
  _search.add(ColoredMatrix::SEARCH_DFS,  "dfs", "Find a solution.");
  _search.add(ColoredMatrix::SEARCH_BAB,  "bab", "Find an optimal solution.");
  _search.value(ColoredMatrix::SEARCH_DFS);

  // Add symmetry options
  _symmetry.add(ColoredMatrix::SYMMETRY_NONE,  "none", "Don't use symmetry breaking.");
  _symmetry.add(ColoredMatrix::SYMMETRY_MATRIX,  "matrix", "Order matrix rows and columns");
  _symmetry.add(ColoredMatrix::SYMMETRY_VALUES,  "values", "Order values");
  _symmetry.add(ColoredMatrix::SYMMETRY_MATRIX | ColoredMatrix::SYMMETRY_VALUES,
                "both", "Order both rows/columns and values");
  _symmetry.value(ColoredMatrix::SYMMETRY_MATRIX);

  // Add model options
  _model.add(ColoredMatrix::MODEL_CORNERS,  "corner", "Use direct corners model with not-all-equal constraints.");
  _model.add(ColoredMatrix::MODEL_ROWS,  "rows", "Use model on pairs of rows (same_or_0 and distinct_except_0 constraints).");
  _model.add(ColoredMatrix::MODEL_ROWS | ColoredMatrix::MODEL_CORNERS,
             "both", "Use both rows and corners model");
  _model.add(ColoredMatrix::MODEL_COLUMNS,  "columns", "Use model on pairs of columns (same_or_0 and distinct_except_0 constraints).");
  _model.add(ColoredMatrix::MODEL_ROWS | ColoredMatrix::MODEL_COLUMNS,
             "matrix", "Use both rows and columns model");
  _model.value(ColoredMatrix::MODEL_CORNERS);

  // Add not all equal variants
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_NQ, "nq", "Use nq constraint.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_NAIVE, "naive", "Use naive reified decomposition.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_REIFIED, "reified", "Use reified decomposition.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_NVALUES, "nvalues", "Use nvalues.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_COUNT, "count", "Use count.");
  _not_all_equal.add(ColoredMatrix::NOT_ALL_EQUAL_DFA, "dfa", "Use dfa.");

  // Add same or 0 variants
  _same_or_0.add(ColoredMatrix::SAME_OR_0_REIFIED, "reified", "Use reified decomposition.");
  _same_or_0.add(ColoredMatrix::SAME_OR_0_TUPLE_SET, "tuple-set", "Use tuple set representation.");
  _same_or_0.add(ColoredMatrix::SAME_OR_0_DFA, "dfa", "Use dfa representation.");

  // Add distinct except 0 variants
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_REIFIED, "reified", "Use reified decomposition.");
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_DFA, "dfa", "Use dfa decomposition.");
  _distinct_except_0.add(ColoredMatrix::DISTINCT_EXCEPT_0_COUNT, "count", "Use global cardinality.");

  // Add no monochrome rectangle variants
  _no_monochrome_rectangle.add(ColoredMatrix::NO_MONOCHROME_DECOMPOSITION,
                               "decompositions",
                               "Use decompositions into same_or_0 and distinct_except_0.");
  _no_monochrome_rectangle.add(ColoredMatrix::NO_MONOCHROME_DFA,
                               "dfa",
                               "Use DFA as direct implementation.");
}

/** \brief Main-function
 *  \relates ColoredMatrix
 */
int
main(int argc, char* argv[]) {
  ColoredMatrixOptions opt("Colored matrix");
  opt.parse(argc,argv);
  if (opt.search() == ColoredMatrix::SEARCH_DFS) {
    Script::run<ColoredMatrix,DFS,ColoredMatrixOptions>(opt);
  } else {
    Script::run<ColoredMatrix,BAB,ColoredMatrixOptions>(opt);
  }
  return 0;
}


namespace {
  DFA same_or_0_dfa(int colors)
  {
    /* DFA over variable sequences (x,y,z) where z equals x/y if x and
     * y are equal, and z equals 0 otherwise.
     *
     * DFA is constructed to contain paths
     *   start -- c --> node -- c --> node' -- c --> end
     * for all colors c representing the case when x and y
     * are equal.
     *
     * For the cases where x and y are non-equal (c and c'), paths
     *   start -- c --> node -- c' --> not-equal -- 0 --> end
     * are added.
     */

    const int start_state = 0;
    const int not_equal_state = 2*colors+1;
    const int final_state = 2*colors+2;

    int n_transitions = colors*colors + 2*colors + 2;
    DFA::Transition* trans = 
      new DFA::Transition[static_cast<size_t>(n_transitions)];
    int current_transition = 0;

    // From start state
    for (int color = 1; color <= colors; ++color) {
      trans[current_transition++] =
        DFA::Transition(start_state, color, color);
    }

    // From first level states (indices [1..color])
    // To second-level if same color, otherwise to not_equal_state
    for (int state = 1; state <= colors; ++state) {
      for (int color = 1; color <= colors; ++color) {
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
    for (int color = 1; color <= colors; ++color) {
      trans[current_transition++] =
        DFA::Transition(colors+color, color, final_state);
    }

    // From not equal state to final state
    trans[current_transition++] =
      DFA::Transition(not_equal_state, 0, final_state);

    // End sentinel
    trans[current_transition++] =
      DFA::Transition(-1, 0, -1);

    int final_states[] = {final_state, -1};

    DFA result(start_state, trans, final_states, true);

    delete[] trans;

    return result;
  }

  TupleSet same_or_0_tuple_set(int colors) {
    TupleSet result(3);
    for (int i = 1; i <= colors; ++i) {
      for (int j = 1; j <= colors; ++j) {
        if (i == j) {
          result.add({i, j, i});
        } else {
          result.add({i, j, 0});
        }
      }
    }
    result.finalize();
    return result;
  }

  DFA distinct_except_0_dfa(int colors)
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

    DFA::Transition* trans =
      new DFA::Transition[static_cast<size_t>(nstates*colors + 1)];
    int current_transition = 0;

    for (int state = nstates; state--; ) {
      trans[current_transition++] = DFA::Transition(state, 0, state);

      for (int color = 1; color <= colors; ++color) {
        const int color_bit = (1 << (color-1));
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

    DFA result(start_state, trans, final_states);

    delete[] trans;
    delete[] final_states;

    return result;
  }

  DFA no_monochrome_rectangle_dfa(int colors)
  {
    /* DFA for a sequence of pairs, where each monochromatic pair may
     * only appear once.
     *
     * For n colors, there are 2^n base states representing which
     * monochromatic pairs are still available. For each base state s,
     * the color seen goes to a new intermediate state. A different
     * color will go back to state s. Seeing the same color will move
     * to the next base state with that color combination removed (if
     * it is still allowed).
     *
     * In essence, this DFA represents the combination of same_or_0
     * and distinct_except_0 as a single constraint.
     */

    const int base_states = 1 << colors;
    const int start_state = base_states-1;
    const int nstates = base_states + colors*base_states;

    DFA::Transition* trans = new DFA::Transition[nstates*colors + 1];
    int current_transition = 0;

    for (int state = base_states; state--; ) {
      for (int color = 1; color <= colors; ++color) {
        const int color_bit = (1 << (color-1));
        const int color_remembered_state = state + color*base_states;

        trans[current_transition++] =
          DFA::Transition(state, color, color_remembered_state);

        for (int next_color = 1; next_color <= colors; ++next_color) {
          if (next_color == color) {
            // Two equal adjacent, only transition if color still allowed
            if (state & color_bit) {
              trans[current_transition++] =
                DFA::Transition(color_remembered_state, color, state & ~color_bit);
            }
          } else {
            trans[current_transition++] =
              DFA::Transition(color_remembered_state, next_color, state);
          }
        }
      }
    }
    trans[current_transition++] = DFA::Transition(-1, 0, -1);
    assert(current_transition <= nstates*colors+1);

    int* final_states = new int[base_states+1];
    final_states[base_states] = -1;
    for (int i = base_states; i--; ) {
      final_states[i] = i;
    }

    DFA result(start_state, trans, final_states);

    delete[] trans;
    delete[] final_states;

    return result;
  }

  IntSetArgs distinct_except_0_counts(int colors, int size)
  {
    IntSetArgs result(colors+1);

    result[0] = IntSet(0, size);

    for (int i = 1; i <= colors; ++i) {
      result[i] = IntSet(0, 1);
    }

    return result;
  }


  DFA not_all_equal_dfa(int colors)
  {
    /* DFA for not all equal.
     *
     * From the start state, there is a transition for each color to
     * that colors state.  As long as the same color is seen, the
     * automaton stays in that state. If a different color is seen,
     * then it goes to the accepting state.
     */

    const int nstates = 1 + colors + 1;
    const int start_state = 0;
    const int final_state = nstates-1;

    DFA::Transition* trans = new DFA::Transition[2*colors + colors*colors + 1];
    int current_transition = 0;

    // Each color to its own state
    for (int color = 1; color <= colors; ++color) {
      trans[current_transition++] = DFA::Transition(start_state, color, color);
    }

    // Each colors state loops on itself, and goes to final on all others
    for (int state = 1; state <= colors; ++state) {
      for (int color = 1; color <= colors; ++color) {
        if (state == color) {
          trans[current_transition++] = DFA::Transition(state, color, state);
        } else {
          trans[current_transition++] = DFA::Transition(state, color, final_state);
        }
      }
    }

    // Loop on all colors in final state
    for (int color = 1; color <= colors; ++color) {
      trans[current_transition++] = DFA::Transition(final_state, color, final_state);
    }

    trans[current_transition++] = DFA::Transition(-1, 0, -1);

    int final_states[] = {final_state, -1};

    DFA result(start_state, trans, final_states);

    delete[] trans;

    return result;
  }

}


// STATISTICS: example-any
