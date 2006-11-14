/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  Last modified:
 *     $Date: 2006-11-13 15:42:58 +0100 (Mon, 13 Nov 2006) $ by $Author: tack $
 *     $Revision: 3932 $
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

/** \brief Board specifications
 *
 * \relates Domino
 */
extern const int *specs[];
/** \brief Number of board specifications
 *
 * \relates Domino
 */
extern const unsigned int n_examples;

/**
 * \brief %Example: Solitaire Domino
 * 
 * The task is to place domino pieces on a board. Each piece covers two 
 * fields and has two numbers. There are 28 pieces, from 0-0 to 6-6.
 * The board is set up with a number in each field that must match the
 * number of the domino piece placed on that field.
 *
 * The instances are taken from
 *   http://www.janko.at/Raetsel/Minesweeper/index.htm
 *
 * \ingroup Example
 *
 */
class Domino : public Example {
private:
  /// Specification of the board
  const int *spec;
  /// Width and height of the board
  int width, height;
  
  /// The board representing the number of the piece at each position
  IntVarArray x;

public:
  /// Construction of the model.
  Domino(const Options& o)
    : spec(specs[o.size]), 
      width(spec[0]), height(spec[1]),
      x(this, (width+1)*height, 0, 28)
  {
    spec+=2; // skip board size information
    
    // Copy spec information to the board
    IntArgs board((width+1)*height);
    for (int i=0; i<width; i++)
      for (int j=0; j<height; j++)
        board[j*(width+1)+i] = spec[j*width+i];    
    
    // Initialize the separator column in the board
    for (int i=0; i<height; i++) {
      board[i*(width+1)+8] = -1;
      post(this, x[i*(width+1)+8]==28);
    }

    // Variables representing the coordinates of the first
    // and second half of a domino piece
    IntVarArray p1(this, 28, 0, (width+1)*height-1);
    IntVarArray p2(this, 28, 0, (width+1)*height-1);
    
    int dominoCount = 0;

    int possibleDiffsA[] = {1, width+1};
    IntSet possibleDiffs(possibleDiffsA, 2);

    for (int i=0; i<=6; i++)
      for (int j=i; j<=6; j++) {

        // The two coordinates must be adjacent.
        // I.e., they may differ by 1 or by the width.
        // The separator column makes sure that a field
        // at the right border is not adjacent to the first field
        // in the next row.
        IntVar diff(this, possibleDiffs);
        abs(this, minus(this, p1[dominoCount], p2[dominoCount]),
            diff, ICL_DOM);

        // Link the current piece to the board
        element(this, board, p1[dominoCount], i);
        element(this, board, p2[dominoCount], j);
        
        // Link the current piece to the array where its
        // number is stored.
        element(this, x, p1[dominoCount], dominoCount);
        element(this, x, p2[dominoCount], dominoCount);
        dominoCount++;
      }

    // Branch by piece
    IntVarArgs ps(28*2);
    for (int i=0; i<28; i++) {
      ps[2*i]   = p1[i];
      ps[2*i+1] = p2[i];
    }

    // Install branchings
    branch(this, ps, BVAR_NONE, BVAL_MIN);
  }

  /// Constructor for cloning \a s
  Domino(bool share, Domino& s) :
    Example(share,s), spec(s.spec), width(s.width), height(s.height) {
      x.update(this, share, s.x);
  }

  /// Copy space during cloning
  virtual Space*
  copy(bool share) {
    return new Domino(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    for (int h = 0; h < height; ++h) {
      std::cout << "\t";
      for (int w = 0; w < width; ++w) {
	      int val =  x[h*(width+1)+w].min();
	      char c = val < 10 ? '0'+val : 'A' + (val-10);
	      std::cout << c;
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};


/** \brief Main-function
 *  \relates Domino
 */
int
main(int argc, char** argv) {
  Options o("Domino");
  o.size  = 0;
  o.naive = true;
  o.parse(argc,argv);
  if (o.size >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
	      << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<Domino,DFS>(o);
  return 0;
}


/** \name Puzzle specifications
 *
 * \relates Domino
 */
//@{

const int domino0[] =
  { // width*height of the board
    8,7,
    // the board itself
    2,1,0,3,0,4,5,5,
    6,2,0,6,3,1,4,0,
    3,2,3,6,2,5,4,3,
    5,4,5,1,1,2,1,2,
    0,0,1,5,0,5,4,4,
    4,6,2,1,3,6,6,1,
    4,2,0,6,5,3,3,6
  };

const int domino1[] =
  { // width*height of the board
    8,7,
    // the board itself
    5,1,2,4,6,2,0,5,
    6,6,4,3,5,0,1,5,
    2,0,4,0,4,0,5,0,
    6,1,3,6,3,5,4,3,
    3,1,0,1,2,2,1,4,
    3,6,6,2,4,0,5,4,
    1,3,6,1,2,3,5,2
  };

const int domino2[] =
  { // width*height of the board
    8,7,
    // the board itself
    4,4,5,4,0,3,6,5,
    1,6,0,1,5,3,4,1,
    2,6,2,2,5,3,6,0,
    1,3,0,6,4,4,2,3,
    3,5,5,2,4,2,2,1,
    2,1,3,3,5,6,6,1,
    5,1,6,0,0,0,4,0
  };

const int domino3[] =
  { // width*height of the board
    8,7,
    // the board itself
    3,0,2,3,3,4,4,3,
    6,5,3,4,2,0,2,1,
    6,5,1,2,3,0,2,0,
    4,5,4,1,6,6,2,5,
    4,3,6,1,0,4,5,5,
    1,3,2,5,6,0,0,1,
    0,5,4,6,2,1,6,1
  };

const int domino4[] =
  { // width*height of the board
    8,7,
    // the board itself
    4,1,5,2,4,4,6,2,
    2,5,6,1,4,6,0,2,
    6,5,1,1,0,1,4,3,
    6,2,1,1,3,2,0,6,
    3,6,3,3,5,5,0,5,
    3,0,1,0,0,5,4,3,
    3,2,4,5,4,2,6,0
  };

const int domino5[] =
  { // width*height of the board
    8,7,
    // the board itself
    4,1,2,1,0,2,4,4,
    5,5,6,6,0,4,6,3,
    6,0,5,1,1,0,5,3,
    3,4,2,2,0,3,1,2,
    3,6,5,6,1,2,3,2,
    2,5,0,6,6,3,3,5,
    4,1,0,0,4,1,4,5
  };

/// List of specifications
const int *specs[] =
  {domino0,domino1,domino2,domino3,domino4,domino5};
/// Number of specifications
const unsigned n_examples = sizeof(specs)/sizeof(int*);
//@}

// STATISTICS: example-any
