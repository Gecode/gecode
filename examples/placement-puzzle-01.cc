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
#include "gecode/minimodel.hh"

/** \brief Specification of one tile
 *
 * This structure can be used to specify  tile with specified width
 * and height, and a char-array tile consiting of spaces and X's
 * showing the tile in row-major order.
 *
 * \relates PlacementPuzzle
 */
struct tilespec {
  int width, height;
  const char *tile;
};

/** \brief Board specifications
 *
 * Each board specification repurposes the first two tilespecs to
 * record width and height of the board (tilespec 0) as well as the
 * number of tiles and wheter the board is filled (tilespec 1).
 *
 * \relates PlacementPuzzle
 */
extern const tilespec *specs[];
/** \brief Number of board specifications
 *
 * \relates PlacementPuzzle
 */
extern const unsigned int n_examples;

namespace {
  /** \name Symmetry functions
   *
   * These functions implement the 8 symmetries of 2D planes. The
   * functions are templatized so that they can be used both for the
   * pieces (defined using C-strings) and for arrays of variables.
   *
   * \relates PlacementPuzzle
   */
  //@{
  /// Return index of (\a h, \a w) in the row-major layout of a matrix with
  /// width w\a w1 and height \a h1.
  int pos(int h, int w, int h1, int w1) {
    if (!(0 <= h && h < h1) ||
        !(0 <= w && w < w1)) {
      std::cerr << "Cannot place (" << h << "," << w << ") on board of size " << h1 << "x" << w1 << std::endl;
    }
    return h * w1 + w;
  }
  /// Type for tile symmetry functions
  typedef void (*tsymmfunc)(const char*, int, int, char*, int&, int&);
  /// Type for board symmetry functions
  typedef void (*bsymmfunc)(const IntVarArgs, int, int, IntVarArgs&, int&, int&);
  /// Identity symmetry
  template <class CArray, class Array>
  void id(CArray t1, int w1, int h1, Array t2, int& w2, int&h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h, w, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  /// Rotate 90 degrees
  template <class CArray, class Array>
  void rot90(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(w, w2-h-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  /// Rotate 180 degrees
  template <class CArray, class Array>
  void rot180(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-h-1, w2-w-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  /// Rotate 270 degrees
  template <class CArray, class Array>
  void rot270(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-w-1, h, h2, w2)] = t1[pos(h, w, h1, w1)];
  }

  /// Flip x-wise
  template <class CArray, class Array>
  void flipx(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h, w2-w-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }

  /// Flip y-wise
  template <class CArray, class Array>
  void flipy(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = w1; h2 = h1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-h-1, w, h2, w2)] = t1[pos(h, w, h1, w1)];
  }

  /// Flip diagonal 1
  template <class CArray, class Array>
  void flipd1(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(w, h, h2, w2)] = t1[pos(h, w, h1, w1)];
  }

  /// Flip diagonal 2
  template <class CArray, class Array>
  void flipd2(CArray t1, int w1, int h1, Array t2, int& w2, int& h2) {
    w2 = h1; h2 = w1;
    for (int h = 0; h < h1; ++h)
      for (int w = 0; w < w1; ++w)
	t2[pos(h2-w-1, w2-h-1, h2, w2)] = t1[pos(h, w, h1, w1)];
  }
  //@}
}

/**
 * \brief %Example: Placement puzzles
 *
 * \section ExamplePlacementPuzzleProblem The Problem
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
 * \section ExamplePlacementPuzzleVariables The Variables
 *
 * The variables for the model is the grid of squares that the  pieces
 * are placed on, where each of the variables for the squares takes
 * the value of the number of the piece which is placed over it.
 * 
 * \section ExamplePlacementPuzzleOnePiece Placing one piece
 *
 * The constraint for each piece placement uses regular expressions
 * (and consequently the \ref regular constraint) for expressing
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
 * described using the regular expression \f$ 0^*11000100010^*\f$. The
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
 * include the extra column, \f$ 0^*1100001000010^*\f$.
 *
 * 
 * \section ExamplePlacementPuzzleRotatingPiece Rotating pieces
 *
 * To handle rotations of the piece, we can use disjunctions of
 * regular expressions for all the relevant rotations. Consider the
 * rotated version of the above piece, depicted below.
 * \code
 *   X
 * XXX
 * \endcode
 * The corresponding regular expression for this piece is
 * \f$ 0^*1001110^*\f$. To combine these two regular expressions, we
 * can simply use disjunction of regular expressions, arriving at the
 * expression \f$ 0^*1100001000010^*|0^*1001110^*\f$ for enforcing
 * the placement of the piece in one of the above two rotations.
 *
 * There are 8 symmetries for the pieces in general. The 8 disjuncts
 * for a particular piece might, however, contain less than 8 distinct
 * expressions (for example, any square has only one distinct
 * disjunct). This is removed when then automaton for the expression
 * is computed, since it is minimized.
 *
 *
 * \section ExamplePlacementPuzzleSeveral Placing several pieces
 *
 * To generalize the above model to several pieces, we let the
 * variables range from \a 0 to \a n, where \a n is the number of pieces to
 * place. For placing piece number \a p, we construct a 0/1-matrix,
 * where each position is 1 if and only if the corresponding place in
 * the original variable matrix is equal to \a p. Then we post the
 * constraint for piece \a p on this new matrix of variables.
 * 
 * \section ExamplePlacementPuzzleHeuristic The Branching
 * 
 * This model does not use any advanced heuristic for the
 * branching. The variables selection is simply left to right, top to
 * bottom, and the value selection minimum first.
 *
 * The static variable ordering means that we try to place the pieces
 * top to bottom. The static value selection allows us to order the
 * pieces in the specification of the problem. We use approximately
 * "largest first" ordering for the pieces.
 *
 *
 * \section ExamplePlacementPuzzleSymmetries Removing board symmetries
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
class PlacementPuzzle : public Example {
private:
  /// Specification of the tiles to place.
  const tilespec *spec;
  /// Width and height of the board
  int width, height;
  /// Number of tiles to place
  int ntiles;

  /// The variables for the board.
  IntVarArray b;

  /// Access position (h,w) in the board matrix.
  IntVar board(int h, int w) {
    return b[h*width + w];
  }

  /// Returns the regular expression for placing a specific tile \a
  /// tile in a specific rotation.
  REG tile_reg(int col, int twidth, int theight, const char* tile) {
    REG res = *REG(0);
    for (int h = 0; h < theight; ++h) {
      for (int w = 0; w < twidth; ++w) {
	int which = tile[h*twidth + w] == 'X';
	res = res + REG(which);
      }
      if (h < theight-1) {
	res = res + REG(0)(width-twidth, width-twidth);
      }
    }
    res = res + *REG(0);

    return res;
  } 

  /// Returns the regular expression for placing tile number \a t in
  /// any rotation.
  REG get_constraint(int t) {
    // Resulting regular expression
    REG res;
    // Color of tile
    int col = t+1;
    // Tile specification for tile under symmetry operations
    char *t2 = new char[width*height];
    int w2, h2;
    // Symmetry functions
    tsymmfunc syms[] = {id, flipx, flipy, flipd1, flipd2, rot90, rot180, rot270};
    int symscnt = sizeof(syms)/sizeof(tsymmfunc);
    for (int i = 0; i < symscnt; ++i) {
      syms[i](spec[t].tile, spec[t].width, spec[t].height, t2, w2, h2);
      res = res | tile_reg(col, w2, h2, t2);
    }
    delete [] t2;

    return res;
  }


public:
  /// Construction of the model.
  PlacementPuzzle(const Options& o)
    : spec(specs[o.size]), 
      width(spec[0].width+1), height(spec[0].height),
      ntiles(spec[1].width),
      b(this, width*height, 0,ntiles) {
    bool filled = spec[1].height;
    spec += 2; // No need for the specification-part any longer
    
    // Set end-of-line markers
    for (int h = 0; h < height; ++h) {
      if (filled)
        for (int w = 0; w < width-1; ++w)
          rel(this, b[h*width + w], IRT_NQ, 0);
      rel(this, b[h*width + width - 1], IRT_EQ, 0);
    }

    // Post constraints
    for (int i = 0; i < ntiles; ++i) {
      // Construct matrix for color i+1
      BoolVarArgs cm(b.size());
      for (int p = b.size(); p--; )
        cm[p] = post(this, ~(b[p] == i+1));
      // Get constraint for color i+1
      REG reg = get_constraint(i);
      DFA dfa = reg;
      // Post constraint
      regular(this, cm, dfa);
    }

    // Remove symmetrical boards
    if (!o.naive) {
      // Storage for board under symmetry operations
      IntVarArgs orig(b.size()-height), symm(b.size()-height);
      int pos = 0;
      for (int i = 0; i < b.size(); ++i) {
        if ((i+1)%width==0) continue;
        orig[pos++] = b[i];
      }
      int w2, h2;
      // Symmetry functions
      bsymmfunc syms[] = {flipx, flipy, flipd1, flipd2, rot90, rot180, rot270};
      int symscnt = sizeof(syms)/sizeof(bsymmfunc);
      for (int i = 0; i < symscnt; ++i) {
        syms[i](orig, width-1, height, symm, w2, h2);
        rel(this, orig, IRT_LQ, symm);
      }
    }

    branch(this, b, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  PlacementPuzzle(bool share, PlacementPuzzle& s) :
    Example(share,s), spec(s.spec), width(s.width), height(s.height) {
    b.update(this, share, s.b);
  }

  /// Copy space during cloning
  virtual Space*
  copy(bool share) {
    return new PlacementPuzzle(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    for (int h = 0; h < height; ++h) {
      std::cout << "\t";
      for (int w = 0; w < width-1; ++w) {
        char c;
        if (board(h,w).assigned()) {
          int val =  board(h,w).val();
          c = val < 10 ? '0'+val : 'A' + (val-10);
        } else {
          c = '_';
        }
	std::cout << c;
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};


/** \brief Main-function
 *  \relates PlacementPuzzle
 */
int
main(int argc, char** argv) {
  Options o("PlacementPuzzle");
  o.size  = 1;
  o.naive = true;
  o.parse(argc,argv);
  if (o.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
	      << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<PlacementPuzzle,DFS>(o);
  return 0;
}


/** \name Puzzle specifications
 *
 * \relates PlacementPuzzle
 */
//@{
/// Small specification
static const tilespec puzzle0[] = 
  {
    // Width and height of board
    {4, 4, ""},
    // Number of tiles and wheter the board is filled
    {5, true, ""},
    {2, 3,
     "XX"
     "X "
     "X "},
    {2, 1,
     "XX"},
    {3, 3,
     " XX"
     "  X"
     "XXX"},
    {1, 1,
     "X"},
    {3, 1,
     "XXX"}
  };
/// Standard specification
static const tilespec puzzle1[] =
  {
    // Width and height of board
    {8, 8, ""},
    // Number of tiles and wheter the board is filled
    {10, true, ""},
    {3, 3,
     "XXX"
     "XXX"
     "XX "},
    {5, 3,
     "  XXX"
     "  X  "
     "XXX  "},
    {3, 4,
     "XXX"
     "XXX"
     "  X"
     "  X"},
    {3, 4,
     "XXX"
     "  X"
     "  X"
     "  X"},
    {2, 5,
     " X"
     " X"
     " X"
     "XX"
     "XX"},
    {4, 2,
     "XX  "
     "XXXX"},
    {3, 3,
     "XXX"
     "  X"
     "  X"},
    {2, 3, 
     "XX"
     "X "
     "X "},
    {2, 4,
     "XX"
     "XX"
     "XX"
     "XX"},
    {3, 2,
     "XX "
     "XXX"}
  };

/// Standard specification with small changes
static const tilespec puzzle2[] =
  {
    // Width and height of board
    {8, 8, ""},
    // Number of tiles and wheter the board is filled
    {10, true, ""},
    {3, 3,
     "XXX"
     "XXX"
     "XX "},
    {5, 3,
     "  XXX"
     "  X  "
     "XXXX "},
    {3, 4,
     "XXX"
     "XXX"
     "  X"
     "  X"},
    {3, 4,
     "XXX"
     "  X"
     " XX"
     "  X"},
    {2, 5,
     " X"
     " X"
     " X"
     "X "
     "XX"},
    {4, 2,
     "XX  "
     "XXXX"},
    {3, 3,
     "XXX"
     "  X"
     "  X"},
    {2, 3, 
     "XX"
     "X "
     "X "},
    {2, 4,
     "XX"
     "XX"
     "X "
     "XX"},
    {3, 2,
     "XX "
     "XXX"}
  };

// Packing number 2 from examples/packing.cc
static const tilespec packing2[] =
  {
    // Width and height of board
    {10, 10, ""},
    // Number of tiles and wheter the board is filled
    {8, true, ""},
    {6, 6,
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
     "XXXXXX"
    },
    {4, 4,
     "XXXX"
     "XXXX"
     "XXXX"
     "XXXX"},
    {4, 4,
     "XXXX"
     "XXXX"
     "XXXX"
     "XXXX"},
    {4, 4,
     "XXXX"
     "XXXX"
     "XXXX"
     "XXXX"},
    {2, 2,
     "XX"
     "XX"},
    {2, 2,
     "XX"
     "XX"},
    {2, 2,
     "XX"
     "XX"},
    {2, 2,
     "XX"
     "XX"}
  };

static const tilespec pentomino6x10[] =
  {
    // Width and height of board
    {10, 6, ""},
    // Number of tiles and wheter the board is filled
    {12, true, ""},
    {2, 4,
     "X "
     "X "
     "X "
     "XX"},
    {3,3,
     "XX "
     " XX"
     " X "},
    {3,3,
     "XXX"
     " X "
     " X "},
    {3,3,
     "  X"
     " XX"
     "XX "},
    {2,4,
     " X"
     "XX"
     " X"
     " X"},
    {5,1,
     "XXXXX"},
    {3,3,
     "X  "
     "XXX"
     "  X"},
    {4,2,
     " XXX"
     "XX  "},
    {2,3,
     "XX"
     "XX"
     " X"},
    {3,2,
     "X X"
     "XXX"},
    {3,3,
     " X "
     "XXX"
     " X "},
    {3,3,
     "  X"
     "  X"
     "XXX"}
  };

static const tilespec pentomino5x12[] =
  {
    // Width and height of board
    {12, 5, ""},
    // Number of tiles and wheter the board is filled
    {12, true, ""},
    {2, 4,
     "X "
     "X "
     "X "
     "XX"},
    {3,3,
     "XX "
     " XX"
     " X "},
    {3,3,
     "XXX"
     " X "
     " X "},
    {3,3,
     "  X"
     " XX"
     "XX "},
    {2,4,
     " X"
     "XX"
     " X"
     " X"},
    {5,1,
     "XXXXX"},
    {3,3,
     "X  "
     "XXX"
     "  X"},
    {4,2,
     " XXX"
     "XX  "},
    {2,3,
     "XX"
     "XX"
     " X"},
    {3,2,
     "X X"
     "XXX"},
    {3,3,
     " X "
     "XXX"
     " X "},
    {3,3,
     "  X"
     "  X"
     "XXX"}
  };

static const tilespec pentomino4x15[] =
  {
    // Width and height of board
    {15, 4, ""},
    // Number of tiles and wheter the board is filled
    {12, true, ""},
    {2, 4,
     "X "
     "X "
     "X "
     "XX"},
    {3,3,
     "XX "
     " XX"
     " X "},
    {3,3,
     "XXX"
     " X "
     " X "},
    {3,3,
     "  X"
     " XX"
     "XX "},
    {2,4,
     " X"
     "XX"
     " X"
     " X"},
    {5,1,
     "XXXXX"},
    {3,3,
     "X  "
     "XXX"
     "  X"},
    {4,2,
     " XXX"
     "XX  "},
    {2,3,
     "XX"
     "XX"
     " X"},
    {3,2,
     "X X"
     "XXX"},
    {3,3,
     " X "
     "XXX"
     " X "},
    {3,3,
     "  X"
     "  X"
     "XXX"}
  };

static const tilespec pentomino3x20[] =
  {
    // Width and height of board
    {20, 3, ""},
    // Number of tiles and wheter the board is filled
    {12, true, ""},
    {2, 4,
     "X "
     "X "
     "X "
     "XX"},
    {3,3,
     "XX "
     " XX"
     " X "},
    {3,3,
     "XXX"
     " X "
     " X "},
    {3,3,
     "  X"
     " XX"
     "XX "},
    {2,4,
     " X"
     "XX"
     " X"
     " X"},
    {5,1,
     "XXXXX"},
    {3,3,
     "X  "
     "XXX"
     "  X"},
    {4,2,
     " XXX"
     "XX  "},
    {2,3,
     "XX"
     "XX"
     " X"},
    {3,2,
     "X X"
     "XXX"},
    {3,3,
     " X "
     "XXX"
     " X "},
    {3,3,
     "  X"
     "  X"
     "XXX"}
  };

/// List of specifications
const tilespec *specs[] = {puzzle0, puzzle1, puzzle2, packing2,
                           pentomino6x10,pentomino5x12,
                           pentomino4x15,pentomino3x20};
/// Number of specifications
const unsigned n_examples = sizeof(specs)/sizeof(tilespec*);
//@}

// STATISTICS: example-any
