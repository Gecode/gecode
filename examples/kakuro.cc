/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  Last modified:
 *     $Date: 2007-09-14 14:50:52 +0200 (Fri, 14 Sep 2007) $ by $Author: schulte $
 *     $Revision: 5033 $
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

namespace {

  /** \name %Kakuro specifications
   *
   * Each specification starts with two integers for width and height,
   * followed by entries for horizontal constraints, an integer -1
   * (signalling the end of the horizontal constraints), entries
   * for the vertical constraints, and, finally, an integer -1.
   *
   * Each entry consists of four integers:
   *  - the y-coordinate of the hint
   *  - the x-coordinate of the hint
   *  - the number of fields in the respective direction
   *  - the sum of the fields
   *
   * The example are taken from the website of Nikoli (from the free 
   * section). Thanks to Nikoli for their great puzzles and their
   * brilliant website: www.nikoli.com.
   *
   * \relates Kakuro
   */
  //@{

  // Easy, Author: Casty
  const int k0[] = {
    12,10,
    // Horizontal constraints
    0,2,5,16, 0,3,2,4, 0,5,3,6, 0,6,2,4, 0,7,5,15, 0,10,3,6, 0,11,3,7,
    1,1,3,7, 1,9,5,16,
    2,4,2,5, 2,8,2,3,
    3,3,5,16, 3,8,2,3,
    4,5,5,15, 4,10,5,15,
    5,4,2,3, 5,8,2,4, 5,11,3,7,
    6,1,3,6, 6,2,3,7, 6,7,3,7,
    7,6,2,3, 7,9,2,4,
    -1,
    // Vertical constraints
    1,1,2,7, 1,4,3,9, 1,9,2,4,
    2,0,3,7, 2,4,3,7, 2,8,3,6,
    3,0,2,3, 3,3,2,4, 3,6,5,16,
    4,0,4,10, 4,5,4,10,
    5,1,2,10, 5,4,3,6, 5,8,2,5,
    6,2,4,10, 6,7,4,12,
    7,0,5,16, 7,6,2,4, 7,9,2,4,
    8,0,3,7, 8,4,3,8, 8,8,3,6,
    9,0,2,3, 9,4,3,7, 9,8,2,3,
    -1
  };

  // Hard, Author: TAKEI Daisuke
  const int k1[] = {
    22,14,
    // Horizontal constraints
    0,1,4,10, 0,2,4,24, 0,3,3,7, 0,7,3,8, 0,8,2,17, 0,9,3,13, 0,13,3,22,
    0,14,2,12, 0,15,3,24, 0,19,3,19, 0,20,4,28, 0,21,4,14,
    1,4,5,16, 1,6,5,17, 1,10,5,32, 1,12,5,34, 1,16,5,35, 1,18,5,31,
    2,5,3,9, 2,11,3,7, 2,17,3,7,
    4,3,5,27, 4,7,5,16, 4,9,5,20, 4,13,5,30, 4,15,5,30, 4,19,5,26,
    5,1,3,12, 5,2,3,20, 5,8,3,22, 5,14,3,9, 5,20,3,10, 5,21,3,20,
    7,4,5,31, 7,6,5,16, 7,10,5,17, 7,12,5,32, 7,16,5,19, 7,18,5,34,
    8,5,3,8, 8,11,3,24, 8,17,3,24,
    9,1,4,10, 9,2,4,15, 9,20,4,30, 9,21,4,12,
    10,3,3,20, 10,7,3,6, 10,9,3,9, 10,13,3,6, 10,15,3,7, 10,19,3,24,
    11,8,2,8, 11,14,2,7,
    -1,
    // Vertical constraints
    1,0,3,7, 1,6,3,12, 1,12,3,23, 1,18,3,23,
    2,0,4,11, 2,5,5,19, 2,11,5,33, 2,17,4,28,
    3,0,7,28, 3,8,5,34, 3,14,7,29,
    4,0,2,12, 4,3,3,7, 4,9,3,16, 4,15,3,10, 4,19,2,10,
    5,2,5,18, 5,8,5,20, 5,14,5,29,
    6,0,4,24, 6,5,5,35, 6,11,5,23, 6,17,4,26,
    7,0,3,23, 7,6,3,9, 7,12,3,10, 7,18,3,23,
    8,0,4,15, 8,5,5,19, 8,11,5,33, 8,17,4,10,
    9,2,5,19, 9,8,5,35, 9,14,5,31,
    10,0,2,4, 10,3,3,10, 10,9,3,18, 10,15,3,24, 10,19,2,12,
    11,0,7,41, 11,8,5,23, 11,14,7,36,
    12,0,4,14, 12,5,5,17, 12,11,5,15, 12,17,4,26,
    13,0,3,7, 13,6,3,7, 13,12,3,6, 13,18,3,23,
    -1
  };
  //@}

  /// An array of all examples
  const int* examples[] = {
    &k0[0], &k1[0]
  };
  /// The number of examples
  const unsigned int n_examples = sizeof(examples)/sizeof(char*);

}

/**
 * \brief %Example: Kakuro
 *
 * Another puzzle in the style of Sudoku.
 *
 * \ingroup ExProblem
 */
class Kakuro : public Example {
protected:
  const int w; ///< Width of board
  const int h; ///< Height of board
  IntVarArray _b; ///< Variables for board
public:
  /// Access the field at position \a x, \a y
  IntVar& b(int x, int y) {
    assert((x >= 0) && (x < w));
    assert((y >= 0) && (y < h));
    return _b[x+y*w];
  }
  /// Init the field at position \a x, \a y if necessary
  void init(int x, int y) {
    if (b(x,y).min() > 0)
      return;
    b(x,y).init(this,1,9);
  }
  /// The actual problem
  Kakuro(const SizeOptions& opt)
    : w(examples[opt.size()][0]), 
      h(examples[opt.size()][1]), 
      _b(this,w*h) {
    IntVar black(this,0,0);
    // Initialize all fields as black (unused). Only if a field
    // is actually used in a constraint, create a fresh variable
    // for it.
    for (int i=w*h; i--; )
      _b[i] = black;
    const int* k = &examples[opt.size()][2];
    // Process horizontal constraints
    while (*k >= 0) {
      int y=*k++; int x=*k++; int n=*k++; int s=*k++;
      IntVarArgs col(n);
      for (int i=0; i<n; i++) {
        init(x,y+i+1); col[i]=b(x,y+i+1);
      }
      distinct(this, col, opt.icl());
      linear(this, col, IRT_EQ, s, opt.icl());
    }
    k++;
    // Process vertical constraints
    while (*k >= 0) {
      int y=*k++; int x=*k++; int n=*k++; int s=*k++;
      IntVarArgs row(n);
      for (int i=0; i<n; i++) {
        init(x+i+1,y); row[i]=b(x+i+1,y);
      }
      distinct(this, row, opt.icl());
      linear(this, row, IRT_EQ, s, opt.icl());
    }
    branch(this, _b, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MIN); 
  }
  /// Constructor for cloning \a s
  Kakuro(bool share, Kakuro& s) : Example(share,s), w(s.w), h(s.h) {
    _b.update(this, share, s._b);
  }
  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Kakuro(share,*this);
  }
  /// Print solution
  virtual void
  print(void) {
    for (int y=0; y<h; y++) {
      std::cout << '\t';
      for (int x=0; x<w; x++)
        std::cout << b(x,y) << ' ';
      std::cout << std::endl;
    }
  }
};


/** \brief Main-function
 *  \relates Kakuro
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Kakuro");
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<Kakuro,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
