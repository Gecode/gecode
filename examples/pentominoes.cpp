/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
 *     Guido Tack, 2006
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

/** \brief Specification of one tile
 *
 * This structure can be used to specify a tile with specified width
 * and height, number of such tiles (all with unique values), and a
 * char-array tile showing the tile in row-major order.
 *
 * \relates Pentominoes
 */
class TileSpec {
public:
  int width;  ///< Width of tile
  int height; ///< Height of tile
  int amount; ///< Number of tiles
  const char *tile; ///< Picture of tile
};

/** \brief Board specifications
 *
 * Each board specification repurposes the first two TileSpecs to
 * record width and height of the board (TileSpec 0) as well as the
 * number of tiles and whether the board is filled (TileSpec 1).
 *
 * \relates Pentominoes
 */
extern const TileSpec *examples[];
/** \brief Board specification sizes
 *
 * \relates Pentominoes
 */
extern const int examples_size[];
/** \brief Number of board specifications
 *
 * \relates Pentominoes
 */
extern const unsigned int n_examples;

namespace {
  /** \name Symmetry functions
   *
   * These functions implement the 8 symmetries of 2D planes. The
   * functions are templatized so that they can be used both for the
   * pieces (defined using C-strings) and for arrays of variables.
   *
   * \relates Pentominoes
   */
  //@{
  /** Return index of (\a h, \a w) in the row-major layout of a matrix with
   * width \a w1 and height \a h1.
   */
  int pos(int h, int w, int h1, int w1);
  /// Type for tile symmetry functions
  typedef void (*tsymmfunc)(const char*, int, int, char*, int&, int&);
  /// Type for board symmetry functions
  typedef void (*bsymmfunc)(const IntVarArgs, int, int, IntVarArgs&, int&, int&);
  /// Identity symmetry
  template<class CArray, class Array>
  void id(CArray t1, int w1, int h1, Array t2, int& w2, int&h2);
  /// Rotate 90 degrees
  template<class CArray, class Array>
  void rot90(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Rotate 180 degrees
  template<class CArray, class Array>
  void rot180(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Rotate 270 degrees
  template<class CArray, class Array>
  void rot270(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Flip x-wise
  template<class CArray, class Array>
  void flipx(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Flip y-wise
  template<class CArray, class Array>
  void flipy(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Flip diagonal 1
  template<class CArray, class Array>
  void flipd1(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  /// Flip diagonal 2
  template<class CArray, class Array>
  void flipd2(CArray t1, int w1, int h1, Array t2, int& w2, int& h2);
  //@}
}

/**
 * \brief %Example: %Pentominoes
 *
 * \section ScriptPentominoesProblem The Problem
 *
 * This example places pieces of a puzzle, where each piece is
 * composed of a collection of squares, onto a grid. The pieces may
 * all be rotated and flipped freely. The goal is to place all the
 * pieces on the grid, without any overlaps. An example piece to be
 * placed looks like
 * \code
 *   XXX
 *   X
 * XXX
 * \endcode
 * in one of its rotations.
 *
 * The most famous instance of such a puzzle is the Pentominoes
 * puzzle, where the pieces are all pieces formed by 5 four-connected
 * squares.
 *
 *
 * \section ScriptPentominoesVariables The Variables
 *
 * The variables for the model is the grid of squares that the  pieces
 * are placed on, where each of the variables for the squares takes
 * the value of the number of the piece which is placed overonto it.
 *
 *
 * \section ScriptPentominoesOnePiece Placing one piece
 *
 * The constraint for each piece placement uses regular expressions
 * (and consequently the extensional constraint) for expressing
 * placement of (rotated) pieces on the grid. Consider the simple
 * example of placing the piece
 * \code
 * XX
 *  X
 *  X
 * \endcode
 * onto the 4 by 4 board
 * \code
 * 0123
 * 4567
 * 89AB
 * CDEF
 * \endcode
 *
 * Let the variables 0-F be 0/1-variables indicating if the piece is
 * placed on that position or not. First consider placing the piece on
 * some location, say covering 1,2,6, and A. Then, writing the
 * sequence of values for the variables 0-F out, we get the string
 * 0110001000100000. This string and all other strings corresponding
 * to placing the above piece in that particular rotation can be
 * described using the regular expression \f$0^*11000100010^*\f$. The
 * expression indicates that first comes some number of zeroes, then
 * two ones in a row (covering places 1 and 2 in our example
 * placement), then comes exactly three 0's (not covering places 3, 4,
 * and 5), and so on. The variable number of 0's at the beginning and at the end
 * makes the expression match any placement of the piece on the board.
 *
 * There is one problem with the above constraint, since it allows
 * placing the piece covering places 3, 4, 8, and C. That is, the
 * piece may wrap around the board. To prohibit this, we add a new
 * dummy-column to the board, so that it looks like
 * \code
 * 0123G
 * 4567H
 * 89ABI
 * CDEFJ
 * \endcode
 * The variables for places G to J are all set to zero initially, and the
 * regular expression for  the placement of the piece is modified to
 * include the extra column, \f$0^*1100001000010^*\f$.
 *
 *
 * \section ScriptPentominoesRotatingPiece Rotating pieces
 *
 * To handle rotations of the piece, we can use disjunctions of
 * regular expressions for all the relevant rotations. Consider the
 * rotated version of the above piece, depicted below.
 * \code
 *   X
 * XXX
 * \endcode
 * The corresponding regular expression for this piece is
 * \f$0^*1001110^*\f$. To combine these two regular expressions, we
 * can simply use disjunction of regular expressions, arriving at the
 * expression \f$0^*1100001000010^*|0^*1001110^*\f$ for enforcing
 * the placement of the piece in one of the above two rotations.
 *
 * There are 8 symmetries for the pieces in general. The 8 disjuncts
 * for a particular piece might, however, contain less than 8 distinct
 * expressions (for example, any square has only one distinct
 * disjunct). This is removed when then automaton for the expression
 * is computed, since it is minimized.
 *
 *
 * \section ScriptPentominoesSeveral Placing several pieces
 *
 * To generalize the above model to several pieces, we let the
 * variables range from 0 to n, where n is the number of pieces to
 * place. Given that we place three pieces, and that the above shown
 * piece is number one, we will replace each \f$0\f$-expression with
 * the expression \f$(0|2|3)\f$. Thus, the second regular expression
 * becomes \f$(0|2|3)^*1(0|2|3)(0|2|3)111(0|2|3)^*\f$. Additionaly,
 * the end of line marker gets its own value.
 *
 * This generalization suffers from the fact that the automata become
 * much more complex. This problem can be solved by instead
 * projecting out each component, which gives a new board of
 * 0/1-variables for each piece to place.
 *
 *
 * \section ScriptPentominoesHeuristic The Branching
 *
 * This model does not use any advanced heuristic for the
 * branching. The variables selection is simply in order, and the
 * value selection is minimum value first.
 *
 * The static value selection allows us to order the pieces in the
 * specification of the problem. The pieces are approximately ordered by
 * largness or hardness to place.
 *
 *
 * \section ScriptPentominoesSymmetries Removing board symmetries
 *
 * Especially when searching for all solutions of a puzzle instance,
 * we might want to remove the symmetrical boards from the
 * solutions-space. This is done using the same symmetry functions as
 * for the piece symmetries and lexicographical order constraints.
 *
 *
 * \ingroup Example
 *
 */
class Pentominoes : public Script {
public:
  /// Choice of propagators
  enum {
    PROPAGATION_INT,       ///< Use integer propagators
    PROPAGATION_BOOLEAN,   ///< Use Boolean propagators
  };
  /// Choice of symmetry breaking
  enum {
    SYMMETRY_NONE, ///< Do not remove symmetric solutions
    SYMMETRY_FULL, ///< Remove symmetric solutions
  };
private:
  /// Specification of the tiles to place.
  const TileSpec *spec;
  /// Width and height of the board
  int width, height;
  /// Whether the board is filled or not
  bool filled;
  /// Number of specifications of tiles to place
  int nspecs;
  /// Number of tiles to place
  int ntiles;

  /// The variables for the board.
  IntVarArray board;

  /// Compute number of tiles
  int compute_number_of_tiles(const TileSpec* ts, int nspecs) const {
    int res = 0;
    for (int i=0; i<nspecs; i++)
      res += ts[i].amount;
    return res;
  }

  /// Returns the regular expression for placing a specific tile \a
  /// tile in a specific rotation.
  REG tile_reg(int twidth, int theight, const char* tile,
               REG mark, REG other, REG eol) const {
    REG oe = other | eol;
    REG res = *oe;
    REG color[] = {other, mark};
    for (int h = 0; h < theight; ++h) {
      for (int w = 0; w < twidth; ++w) {
	int which = tile[h*twidth + w] == 'X';
	res += color[which];
      }
      if (h < theight-1) {
	res += oe(width-twidth, width-twidth);
      }
    }
    res += *oe + oe;
    return res;
  }

  /// Returns the regular expression for placing tile number \a t in
  /// any rotation.
  REG get_constraint(int t, REG mark, REG other, REG eol) {
    // This should be done for all rotations
    REG res;
    char *t2 = new char[width*height];
    int w2, h2;
    tsymmfunc syms[] = {id, flipx, flipy, flipd1, flipd2, rot90, rot180, rot270};
    int symscnt = sizeof(syms)/sizeof(tsymmfunc);
    for (int i = 0; i < symscnt; ++i) {
      syms[i](spec[t].tile, spec[t].width, spec[t].height, t2, w2, h2);
      res = res | tile_reg(w2, h2, t2, mark, other, eol);
    }
    delete [] t2;

    return res;
  }


public:
  /// Construction of the model.
  Pentominoes(const SizeOptions& opt)
    : Script(opt), spec(examples[opt.size()]),
      width(spec[0].width+1), // Add one for extra row at end.
      height(spec[0].height),
      filled(spec[0].amount),
      nspecs(examples_size[opt.size()]-1),
      ntiles(compute_number_of_tiles(spec+1, nspecs)),
      board(*this, width*height, filled,ntiles+1) {
    spec += 1; // No need for the specification-part any longer

    // Set end-of-line markers
    for (int h = 0; h < height; ++h) {
      for (int w = 0; w < width-1; ++w)
	rel(*this, board[h*width + w], IRT_NQ, ntiles+1);
      rel(*this, board[h*width + width - 1], IRT_EQ, ntiles+1);
    }

    // Post constraints
    if (opt.propagation() == PROPAGATION_INT) {
      int tile = 0;
      for (int i = 0; i < nspecs; ++i) {
        for (int j = 0; j < spec[i].amount; ++j) {
          // Color
          int col = tile+1;
          // Expression for color col
          REG mark(col);
          // Build expression for complement to color col
          REG other;
          bool first = true;
          for (int j = filled; j <= ntiles; ++j) {
            if (j == col) continue;
            if (first) {
              other = REG(j);
              first = false;
            } else {
              other |= REG(j);
            }
          }
          // End of line marker
          REG eol(ntiles+1);
          extensional(*this, board, get_constraint(i, mark, other, eol));
          ++tile;
        }
      }
    } else { // opt.propagation() == PROPAGATION_BOOLEAN
      int ncolors = ntiles + 2;
      // Boolean variables for channeling
      BoolVarArgs p(*this,ncolors * board.size(),0,1);

      // Post channel constraints
      for (int i=board.size(); i--; ) {
        BoolVarArgs c(ncolors);
        for (int j=ncolors; j--; )
          c[j]=p[i*ncolors+j];
        channel(*this, c, board[i]);
      }

      // For placing tile i, we construct the expression over
      // 0/1-variables and apply it to the projection of
      // the board on the color for the tile.
      REG other(0), mark(1);
      int tile = 0;
      for (int i = 0; i < nspecs; ++i) {
        for (int j = 0; j < spec[i].amount; ++j) {
          int col = tile+1;
          // Projection for color col
          BoolVarArgs c(board.size());

          for (int k = board.size(); k--; ) {
            c[k] = p[k*ncolors+col];
          }

          extensional(*this, c, get_constraint(i, mark, other, other));
          ++tile;
        }
      }
    }

    if (opt.symmetry() == SYMMETRY_FULL) {
      // Remove symmetrical boards
      IntVarArgs orig(board.size()-height), symm(board.size()-height);
      int pos = 0;
      for (int i = 0; i < board.size(); ++i) {
        if ((i+1)%width==0) continue;
        orig[pos++] = board[i];
      }

      int w2, h2;
      bsymmfunc syms[] = {flipx, flipy, flipd1, flipd2, rot90, rot180, rot270};
      int symscnt = sizeof(syms)/sizeof(bsymmfunc);
      for (int i = 0; i < symscnt; ++i) {
        syms[i](orig, width-1, height, symm, w2, h2);
        if (width-1 == w2 && height == h2)
          rel(*this, orig, IRT_LQ, symm);
      }
    }

    // Install branching
    branch(*this, board, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Pentominoes(Pentominoes& s) :
    Script(s), spec(s.spec), width(s.width), height(s.height),
    filled(s.filled), nspecs(s.nspecs) {
    board.update(*this, s.board);
  }

  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Pentominoes(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int h = 0; h < height; ++h) {
      os << "\t";
      for (int w = 0; w < width-1; ++w) {
        int val =  board[h*width + w].val();
        char c = val < 10 ? '0'+val : 'A' + (val-10);
        os << c;
      }
      os << std::endl;
    }
    os << std::endl;
  }
};


/** \brief Main-function
 *  \relates Pentominoes
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Pentominoes");
  opt.size(1);
  opt.symmetry(Pentominoes::SYMMETRY_FULL);
  opt.symmetry(Pentominoes::SYMMETRY_NONE,
            "none", "do not remove symmetric solutions");
  opt.symmetry(Pentominoes::SYMMETRY_FULL,
            "full", "remove symmetric solutions");

  opt.propagation(Pentominoes::PROPAGATION_BOOLEAN);
  opt.propagation(Pentominoes::PROPAGATION_INT,
                  "int", "use integer propagators");
  opt.propagation(Pentominoes::PROPAGATION_BOOLEAN,
                  "bool", "use Boolean propagators");

  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
	      << n_examples-1 << std::endl;
    return 1;
  }
  Script::run<Pentominoes,DFS,SizeOptions>(opt);
  return 0;
}


/** \name Puzzle specifications
 *
 * \relates Pentominoes
 */
//@{
/// Small specification
static const TileSpec puzzle0[] =
  {
    // Width and height of board
    {4, 4, true, ""},
    {2, 3, 1,
     "XX"
     "X "
     "X "},
    {2, 1, 1,
     "XX"},
    {3, 3, 1,
     " XX"
     "  X"
     "XXX"},
    {1, 1, 1,
     "X"},
    {3, 1, 1,
     "XXX"}
  };
/// Standard specification
static const TileSpec puzzle1[] =
  {
    // Width and height of board
    {8, 8, true, ""},
    {3, 3, 1,
     "XXX"
     "XXX"
     "XX "},
    {5, 3, 1,
     "  XXX"
     "  X  "
     "XXX  "},
    {3, 4, 1,
     "XXX"
     "XXX"
     "  X"
     "  X"},
    {3, 4, 1,
     "XXX"
     "  X"
     "  X"
     "  X"},
    {2, 5, 1,
     " X"
     " X"
     " X"
     "XX"
     "XX"},
    {4, 2, 1,
     "XX  "
     "XXXX"},
    {3, 3, 1,
     "XXX"
     "  X"
     "  X"},
    {2, 3, 1,
     "XX"
     "X "
     "X "},
    {2, 4, 1,
     "XX"
     "XX"
     "XX"
     "XX"},
    {3, 2, 1,
     "XX "
     "XXX"}
  };

// Perfect square number 2 from examples/perfect-square.cc
static const TileSpec square2[] =
  {
    // Width and height of board
    {10, 10, true, ""},
    {6, 6, 1,
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
    },
    {4, 4, 3,
     "XXXX"
     "XXXX"
     "XXXX"
     "XXXX"},
    {2, 2, 4,
     "XX"
     "XX"}
  };

// Perfect square number 3 from examples/perfect-square.cc
static const TileSpec square3[] =
  {
    // Width and height of board
    {20, 20, true, ""},
    {9, 9, 1,
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
     "XXXXXXXXX"
    },
    {8, 8, 2,
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
     "XXXXXXXX"
    },
    {7, 7, 1,
     "XXXXXXX"
     "XXXXXXX"
     "XXXXXXX"
     "XXXXXXX"
     "XXXXXXX"
     "XXXXXXX"
     "XXXXXXX"
    },
    {5, 5, 1,
     "XXXXX"
     "XXXXX"
     "XXXXX"
     "XXXXX"
     "XXXXX"
    },
    {4, 4, 5,
     "XXXX"
     "XXXX"
     "XXXX"
     "XXXX"},
    {3, 3, 3,
     "XXX"
     "XXX"
     "XXX"},
    {2, 2, 2,
     "XX"
     "XX"},
    {1, 1, 2,
     "X"}
  };

static const TileSpec pentomino6x10[] =
  {
    // Width and height of board
    {10, 6, true, ""},
    {2, 4, 1,
     "X "
     "X "
     "X "
     "XX"},
    {3,3, 1,
     "XX "
     " XX"
     " X "},
    {3,3, 1,
     "XXX"
     " X "
     " X "},
    {3,3, 1,
     "  X"
     " XX"
     "XX "},
    {2,4, 1,
     " X"
     "XX"
     " X"
     " X"},
    {5,1, 1,
     "XXXXX"},
    {3,3, 1,
     "X  "
     "XXX"
     "  X"},
    {4,2, 1,
     " XXX"
     "XX  "},
    {2,3, 1,
     "XX"
     "XX"
     " X"},
    {3,2, 1,
     "X X"
     "XXX"},
    {3,3, 1,
     " X "
     "XXX"
     " X "},
    {3,3, 1,
     "  X"
     "  X"
     "XXX"}
  };

static const TileSpec pentomino5x12[] =
  {
    // Width and height of board
    {12, 5, true, ""},
    {2, 4, 1,
     "X "
     "X "
     "X "
     "XX"},
    {3,3, 1,
     "XX "
     " XX"
     " X "},
    {3,3, 1,
     "XXX"
     " X "
     " X "},
    {3,3, 1,
     "  X"
     " XX"
     "XX "},
    {2,4, 1,
     " X"
     "XX"
     " X"
     " X"},
    {5,1, 1,
     "XXXXX"},
    {3,3, 1,
     "X  "
     "XXX"
     "  X"},
    {4,2, 1,
     " XXX"
     "XX  "},
    {2,3, 1,
     "XX"
     "XX"
     " X"},
    {3,2, 1,
     "X X"
     "XXX"},
    {3,3, 1,
     " X "
     "XXX"
     " X "},
    {3,3, 1,
     "  X"
     "  X"
     "XXX"}
  };

static const TileSpec pentomino4x15[] =
  {
    // Width and height of board
    {15, 4, true, ""},
    {2, 4, 1,
     "X "
     "X "
     "X "
     "XX"},
    {3,3, 1,
     "XX "
     " XX"
     " X "},
    {3,3, 1,
     "XXX"
     " X "
     " X "},
    {3,3, 1,
     "  X"
     " XX"
     "XX "},
    {2,4, 1,
     " X"
     "XX"
     " X"
     " X"},
    {5,1, 1,
     "XXXXX"},
    {3,3, 1,
     "X  "
     "XXX"
     "  X"},
    {4,2, 1,
     " XXX"
     "XX  "},
    {2,3, 1,
     "XX"
     "XX"
     " X"},
    {3,2, 1,
     "X X"
     "XXX"},
    {3,3, 1,
     " X "
     "XXX"
     " X "},
    {3,3, 1,
     "  X"
     "  X"
     "XXX"}
  };

static const TileSpec pentomino3x20[] =
  {
    // Width and height of board
    {20, 3, true, ""},
    {2, 4, 1,
     "X "
     "X "
     "X "
     "XX"},
    {3,3, 1,
     "XX "
     " XX"
     " X "},
    {3,3, 1,
     "XXX"
     " X "
     " X "},
    {3,3, 1,
     "  X"
     " XX"
     "XX "},
    {2,4, 1,
     " X"
     "XX"
     " X"
     " X"},
    {5,1, 1,
     "XXXXX"},
    {3,3, 1,
     "X  "
     "XXX"
     "  X"},
    {4,2, 1,
     " XXX"
     "XX  "},
    {2,3, 1,
     "XX"
     "XX"
     " X"},
    {3,2, 1,
     "X X"
     "XXX"},
    {3,3, 1,
     " X "
     "XXX"
     " X "},
    {3,3, 1,
     "  X"
     "  X"
     "XXX"}
  };

/// List of specifications
const TileSpec *examples[] = {puzzle0, puzzle1, square2, square3,
                           pentomino6x10,pentomino5x12,
                           pentomino4x15,pentomino3x20};
const int examples_size[] = {sizeof(puzzle0)/sizeof(TileSpec),
                             sizeof(puzzle1)/sizeof(TileSpec),
                             sizeof(square2)/sizeof(TileSpec),
                             sizeof(square3)/sizeof(TileSpec),
                             sizeof(pentomino6x10)/sizeof(TileSpec),
                             sizeof(pentomino5x12)/sizeof(TileSpec),
                             sizeof(pentomino4x15)/sizeof(TileSpec),
                             sizeof(pentomino3x20)/sizeof(TileSpec)};

/// Number of specifications
const unsigned n_examples = sizeof(examples)/sizeof(TileSpec*);
//@}

// Symmetry functions
namespace {
  int pos(int h, int w, int h1, int w1) {
    if (!(0 <= h && h < h1) ||
        !(0 <= w && w < w1)) {
      std::cerr << "Cannot place (" << h << "," << w
                << ") on board of size " << h1 << "x" << w1 << std::endl;
    }
    return h * w1 + w;
  }
  template<class CArray, class Array>
  void id(CArray t1, int w1, int h1, Array t2, int& w2, int&h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h, w, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void rot90(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(w, w2-h-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void rot180(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-h-1, w2-w-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void rot270(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-w-1, h, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void flipx(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h, w2-w-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void flipy(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-h-1, w, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void flipd1(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(w, h, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  template<class CArray, class Array>
  void flipd2(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-w-1, w2-h-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
}

// STATISTICS: example-any
