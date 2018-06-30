/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *     Mikael Lagerkvist, 2006
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

namespace {

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

}

/**
 * \brief %Example: Solitaire domino
 *
 * The task is to place domino pieces on a board. Each piece covers two
 * fields and has two numbers. There are 28 pieces, from 0-0 to 6-6.
 * The board is set up with a number in each field that must match the
 * number of the domino piece placed on that field.
 *
 * \ingroup Example
 *
 */
class Domino : public Script {
private:
  /// Specification of the board
  const int *spec;
  /// Width of the board
  int width;
  /// Height of the board
  int height;

  /// The board representing the number of the piece at each position
  IntVarArray x;

public:
  /// Propagation to use for model
  enum {
    PROP_ELEMENT,    ///< Use element constraints
    PROP_EXTENSIONAL ///< Use extensional constraints
  };

  /// Actual model
  Domino(const SizeOptions& opt)
    : Script(opt),
      spec(specs[opt.size()]),
      width(spec[0]), height(spec[1]),
      x(*this, (width+1)*height, 0, 28) {
    spec+=2; // skip board size information

    // Copy spec information to the board
    IntArgs board((width+1)*height);
    for (int i=0; i<width; i++)
      for (int j=0; j<height; j++)
        board[j*(width+1)+i] = spec[j*width+i];

    // Initialize the separator column in the board
    for (int i=0; i<height; i++) {
      board[i*(width+1)+8] = -1;
      rel(*this, x[i*(width+1)+8]==28);
    }

    // Variables representing the coordinates of the first
    // and second half of a domino piece
    IntVarArgs p1(*this, 28, 0, (width+1)*height-1);
    IntVarArgs p2(*this, 28, 0, (width+1)*height-1);


    if (opt.propagation() == PROP_ELEMENT) {
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
          IntVar diff(*this, possibleDiffs);
          abs(*this, expr(*this, p1[dominoCount]-p2[dominoCount]),
              diff, IPL_DOM);

          // If the piece is symmetrical, order the locations
          if (i == j)
            rel(*this, p1[dominoCount], IRT_LE, p2[dominoCount]);

          // Link the current piece to the board
          element(*this, board, p1[dominoCount], i);
          element(*this, board, p2[dominoCount], j);

          // Link the current piece to the array where its
          // number is stored.
          element(*this, x, p1[dominoCount], dominoCount);
          element(*this, x, p2[dominoCount], dominoCount);
          dominoCount++;
        }
    } else {
      int dominoCount = 0;

      for (int i=0; i<=6; i++)
        for (int j=i; j<=6; j++) {
          // Find valid placements for piece i-j
          // Extensional is used as a table-constraint listing all valid
          // tuples.
          // Note that when i == j, only one of the orientations are used.
          REG valids;
          for (int pos = 0; pos < (width+1)*height; ++pos) {
            if ((pos+1) % (width+1) != 0) { // not end-col
              if (board[pos] == i && board[pos+1] == j)
                valids |= REG(pos) + REG(pos+1);
              if (board[pos] == j && board[pos+1] == i && i != j)
                valids |= REG(pos+1) + REG(pos);
            }
            if (pos/(width+1) < height-1) { // not end-row
              if (board[pos] == i && board[pos+width+1] == j)
                valids |= REG(pos) + REG(pos+width+1);
              if (board[pos] == j && board[pos+width+1] == i && i != j)
                valids |= REG(pos+width+1) + REG(pos);
            }
          }
          extensional(*this, IntVarArgs({p1[dominoCount],p2[dominoCount]}),
                      valids);


          // Link the current piece to the array where its
          // number is stored.
          element(*this, x, p1[dominoCount], dominoCount);
          element(*this, x, p2[dominoCount], dominoCount);
          dominoCount++;
        }
    }

    // Branch by piece
    IntVarArgs ps(28*2);
    for (int i=0; i<28; i++) {
      ps[2*i]   = p1[i];
      ps[2*i+1] = p2[i];
    }

    branch(*this, ps, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int h = 0; h < height; ++h) {
      os << "\t";
      for (int w = 0; w < width; ++w) {
        int val =  x[h*(width+1)+w].min();
        char c = val < 10 ? '0'+val : 'A' + (val-10);
        os << c;
      }
      os << std::endl;
    }
    os << std::endl;
  }
  /// Constructor for cloning \a s
  Domino(Domino& s) :
    Script(s), spec(s.spec), width(s.width), height(s.height) {
      x.update(*this, s.x);
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Domino(*this);
  }

};


/** \brief Main-function
 *  \relates Domino
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Domino");
  opt.size(0);
  opt.propagation(Domino::PROP_ELEMENT);
  opt.propagation(Domino::PROP_ELEMENT, "element");
  opt.propagation(Domino::PROP_EXTENSIONAL, "extensional");
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
	      << n_examples-1 << std::endl;
    return 1;
  }
  Script::run<Domino,DFS,SizeOptions>(opt);
  return 0;
}


namespace {

  /** \name Puzzle specifications
   *
   * \relates Domino
   */
  //@{

  /// %Example 0
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

  /// %Example 1
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

  /// %Example 2
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

  /// %Example 3
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

  /// %Example 4
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

  /// %Example 5
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

}

// STATISTICS: example-any
