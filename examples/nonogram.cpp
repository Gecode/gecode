/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
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

#include "examples/support.hh"
#include <gecode/minimodel.hh>

namespace {

  /// List of specifications
  extern const int* specs[];
  /// Number of specifications
  extern const unsigned int n_examples;

}

/**
 * \brief %Example: Nonogram
 *
 * This example solves nonograms. A nonogram is composed of a matrix of
 * markers. For each row/column there is a specification on how many groups
 * of markers (separated by one or more unmarked spots) and their length.
 * The objective is to find a valid assignment, which incidentally may also
 * produce a pretty picture.
 *
 * See problem 12 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 *
 */
class Nonogram : public Example {
protected:
  /// Specification to be used
  const int* spec;
  /// Fields of board
  BoolVarArray b;

  /// Return width of board
  int width(void) const {
    return spec[0];
  }
  /// Return height of board
  int height(void) const {
    return spec[1];
  }

  /// Returns next regular expression for line starting from spos
  DFA line(int& spos) {
    REG r0(0), r1(1);
    REG border = *r0;
    REG between = +r0;
    int hints = spec[spos++];
    REG r = border + r1(spec[spos],spec[spos]);
    spos++;
    for (int i=hints-1; i--; spos++)
      r += between + r1(spec[spos],spec[spos]);
    return r + border;
  }


public:
  /// Construction of the model.
  Nonogram(const SizeOptions& opt)
    : spec(specs[opt.size()]), b(*this,width()*height(),0,1) {
    Matrix<BoolVarArray> m(b, width(), height());

    {
      int spos = 2;
      // Post constraints for columns
      for (int w=0; w<width(); w++)
        extensional(*this, m.col(w), line(spos));
      // Post constraints for rows
      for (int h=0; h<height(); h++)
        extensional(*this, m.row(h), line(spos));
    }

    // Number of hints for columns
    int cols = 0;
    // Number of hints for rows
    int rows = 0;
    // Compute number of hints
    {
      int spos = 2;
      for (int w=0; w<width(); w++) {
        int hint = spec[spos++];
        cols += hint; spos += hint;
      }
      for (int h=0; h<height(); h++) {
        int hint = spec[spos++];
        rows += hint; spos += hint;
      }
    }

    /*
     * The following branches either by columns or rows, depending on
     * whether there are more hints relative to the height or width
     * for columns or rows.
     *
     * This idea is due to Pascal Van Hentenryck and has been suggested
     * to us by Hakan Kjellerstrand.
     */
    if (rows*width() > cols*height()) {
      for (int w=0; w<width(); w++)
        branch(*this, m.col(w), INT_VAR_NONE, INT_VAL_MAX);
    } else {
      for (int h=0; h<height(); h++)
        branch(*this, m.row(h), INT_VAR_NONE, INT_VAL_MAX);
    }
  }

  /// Constructor for cloning \a s
  Nonogram(bool share, Nonogram& s) : Example(share,s), spec(s.spec) {
    b.update(*this, share, s.b);
  }

  /// Copy space during cloning
  virtual Space*
  copy(bool share) {
    return new Nonogram(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<BoolVarArray> m(b, width(), height());
    for (int h = 0; h < height(); ++h) {
      os << '\t';
      for (int w = 0; w < width(); ++w)
        os << ((m(w,h).val() == 1) ? '#' : ' ');
      os << std::endl;
    }
    os << std::endl;
  }
};


/** \brief Main-function
 *  \relates Nonogram
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Nonogram");
  opt.size(8);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Example::run<Nonogram,DFS,SizeOptions>(opt);
  return 0;
}

namespace {

  /** \name Picture specifications
   *
   * A specification is given by a list of integers. The first two
   * integers (w and h) specify the number of columns and rows
   * respectively. Then w + h groups of integers follows. Each group is
   * started by the number of integers it contains (n), followed by n integers
   * specifying the sizes of the stretches of markers in that row/column.
   *
   * \relates Nonogram
   */
  //@{
  /// Specification for a heart-shaped picture.
const int heart[] =
  { 9, 9,
    // Column constraints.
    1, 3,
    2, 2, 3,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 2,
    2, 2, 3,
    1, 3,
    // Row constraints
    2, 2, 2,
    2, 4, 4,
    3, 1, 3, 1,
    3, 2, 1, 2,
    2, 1, 1,
    2, 2, 2,
    2, 2, 2,
    1, 3,
    1, 1
  };

  /// Specification for a bear/bunny-shaped picture.
const int bear[] =
  { 13, 8,
    // Column constraints
    1, 2,
    2, 2, 1,
    2, 3, 2,
    1, 6,
    2, 1, 4,
    1, 3,
    1, 4,
    1, 4,
    1, 4,
    1, 5,
    1, 4,
    2, 1, 3,
    1, 2,
    // Row constraints
    1, 1,
    1, 2,
    2, 4, 4,
    1, 12,
    1, 8,
    1, 9,
    2, 3, 4,
    2, 2, 2
  };

  /// Specification for a crocodile-shaped picture
const int crocodile[] =
  { 15, 9,
    // Column constraints
    1, 3,
    1, 4,
    2, 2, 2,
    2, 3, 1,
    2, 2, 3,
    2, 3, 2,
    2, 2, 3,
    2, 4, 2,
    2, 3, 2,
    1, 6,
    2, 1, 3,
    2, 1, 3,
    2, 1, 4,
    1, 5,
    1, 5,
    // Row constraints
    1, 3,
    3, 2, 3, 2,
    2, 10, 3,
    1, 15,
    5, 1, 1, 1, 1, 6,
    2, 1, 7,
    2, 1, 4,
    2, 1, 4,
    1, 4
  };

  /// Specification for an unknown picture
const int unknown[] =
  { 10, 10,
    // Column constraints
    1, 3,
    2, 2, 1,
    2, 2, 2,
    2, 2, 1,
    3, 1, 2, 1,
    2, 1, 1,
    3, 1, 4, 1,
    3, 1, 1, 2,
    2, 3, 1,
    1, 4,
    // Row constraints
    1, 3,
    2, 2, 1,
    2, 1, 1,
    2, 1, 4,
    4, 1, 1, 1, 1,
    4, 2, 1, 1, 1,
    3, 2, 1, 1,
    2, 1, 2,
    2, 2, 3,
    1, 3
  };

  /// Specification for a pinwheel-picture
const int pinwheel[] =
  { 6, 6,
    // Column constraints
    2, 1, 2,
    1, 1,
    1, 2,
    1, 2,
    1, 1,
    2, 2, 1,
    // Row constraints
    2, 2, 1,
    1, 1,
    1, 2,
    1, 2,
    1, 1,
    2, 1, 2
  };

  /// Specification for a more difficult picture.
const int difficult[] =
  { 15, 15,
    // Column constraints
    1, 3,
    1, 2,
    1, 2,
    1, 1,
    1, 2,
    1, 3,
    1, 2,
    1, 4,
    1, 3,
    1, 4,
    2, 2, 1,
    2, 1, 1,
    2, 1, 1,
    2, 1, 1,
    1, 3,
    // Row constraints
    1, 3,
    2, 1, 1,
    2, 1, 1,
    2, 1, 1,
    2, 1, 2,
    1, 5,
    1, 1,
    1, 2,
    1, 1,
    1, 1,
    2, 1, 2,
    2, 1, 2,
    2, 2, 1,
    2, 2, 2,
    1, 3
  };

  /// Specification for a non-unique picture
const int non_unique[] =
  { 11, 15,
    // Column constraints
    1, 5,
    3, 1, 2, 4,
    3, 2, 1, 3,
    4, 2, 2, 1, 1,
    4, 1, 1, 1, 1,
    2, 1, 5,
    5, 2, 1, 1, 3, 2,
    5, 2, 1, 1, 1, 1,
    3, 1, 4, 1,
    2, 1, 1,
    1, 1,
    // Row constraints
    2, 2, 2,
    2, 2, 2,
    1, 4,
    2, 1, 1,
    2, 1, 1,
    4, 1, 1, 1, 1,
    2, 1, 1,
    2, 1, 4,
    3, 1, 1, 1,
    3, 1, 1, 4,
    2, 1, 3,
    2, 1, 2,
    1, 5,
    2, 2, 2,
    2, 3, 3
  };

  /** \brief Specification for a dragonfly-picture.
   *
   * From http://www.oberlin.edu/math/faculty/bosch/pbn-page.html, where
   * it is claimed that it is hard.
   */
  const int dragonfly[] =
    { 20, 20,
      // Column constraints.
      4, 1, 1, 1, 2,
      5, 3, 1, 2, 1, 1,
      5, 1, 4, 2, 1, 1,
      4, 1, 3, 2, 4,
      4, 1, 4, 6, 1,
      3, 1, 11, 1,
      4, 5, 1, 6, 2,
      1, 14,
      2, 7, 2,
      2, 7, 2,
      3, 6, 1, 1,
      2, 9, 2,
      4, 3, 1, 1, 1,
      3, 3, 1, 3,
      3, 2, 1, 3,
      3, 2, 1, 5,
      3, 3, 2, 2,
      3, 3, 3, 2,
      3, 2, 3, 2,
      2, 2, 6,
      // Row constraints
      2, 7, 1,
      3, 1, 1, 2,
      3, 2, 1, 2,
      3, 1, 2, 2,
      3, 4, 2, 3,
      3, 3, 1, 4,
      3, 3, 1, 3,
      3, 2, 1, 4,
      2, 2, 9,
      3, 2, 1, 5,
      2, 2, 7,
      1, 14,
      2, 8, 2,
      3, 6, 2, 2,
      4, 2, 8, 1, 3,
      4, 1, 5, 5, 2,
      5, 1, 3, 2, 4, 1,
      5, 3, 1, 2, 4, 1,
      5, 1, 1, 3, 1, 3,
      4, 2, 1, 1, 2
    };

  /// From http://www.cs.kuleuven.be/~bmd/nonogram.pl
  const int castle[]  = {
    60, 35,
    // Column constraints
    7, 2,3,1,5,1,7,1,
    7, 2,4,2,3,2,3,5,
    8, 2,6,3,1,1,5,1,5,
    10, 2,4,2,1,1,1,4,1,1,2,
    7, 2,8,2,1,5,2,5,
    7, 3,1,6,2,5,1,5,
    9, 3,3,3,1,1,6,1,1,1,
    9, 3,2,2,2,2,8,1,1,3,
    7, 1,4,4,3,7,1,1,
    7, 1,2,2,2,3,7,9,
    8, 1,2,3,1,1,5,2,2,
    7, 2,2,3,1,1,6,1,
    6, 1,3,1,5,4,1,
    8, 1,3,1,1,6,1,3,1,
    8, 3,3,4,5,1,4,2,1,
    6, 2,3,3,9,7,1,
    8, 2,3,2,2,1,1,3,5,
    8, 4,2,1,1,1,1,2,3,
    7, 4,2,2,1,4,3,2,
    4, 4,3,16,2,
    5, 1,2,5,7,1,      
    6, 4,3,2,2,7,1,      
    5, 2,3,1,10,1,      
    6, 2,4,2,1,4,1,      
    5, 1,6,7,3,1,      
    4, 3,11,3,1,      
    5, 7,1,11,2,1,      
    7, 2,2,2,2,2,2,2,      
    7, 3,1,1,1,1,2,1,      
    7, 2,2,2,2,1,1,1,      
    7, 1,1,1,1,2,1,2,      
    8, 2,2,2,2,1,1,1,1,      
    5, 4,1,1,2,2,      
    5, 5,2,17,2,1,      
    6, 9,2,3,1,4,2,      
    6, 9,4,2,1,1,1,      
    5, 5,4,2,1,4,      
    7, 11,1,2,1,4,1,2,      
    5, 3,4,2,4,4,      
    8, 2,1,4,1,2,1,5,2,      
    5, 8,4,1,1,2,      
    5, 1,1,3,2,3,      
    6, 1,3,1,8,1,6,      
    4, 2,1,7,14,      
    7, 1,2,4,4,1,2,3,      
    10, 1,1,4,2,1,1,1,1,1,4,      
    6, 3,5,3,1,1,4,      
    6, 2,4,2,2,1,2,      
    5, 4,2,3,8,4,      
    5, 4,15,2,2,4,      
    6, 4,1,10,2,1,2,      
    6, 2,12,6,1,2,4,      
    7, 3,1,3,1,3,3,4,      
    6, 3,1,2,3,4,1,      
    7, 5,2,2,2,3,3,3,      
    9, 1,2,2,2,2,4,1,1,3,      
    7, 2,1,4,2,7,1,1,      
    6, 5,2,2,3,6,3,      
    7, 3,3,2,2,3,2,3,      
    7, 4,1,2,1,1,2,1,

    // Row constraints
    4, 12,1,1,1,                                
    5, 8,6,3,1,3,                                
    6, 5,8,4,3,1,5,                                
    8, 7,3,4,1,3,5,1,7,                                
    13, 2,2,4,9,1,5,1,1,1,1,1,1,1,                                
    8, 4,5,10,2,1,8,7,1,                                
    7, 5,1,3,3,16,1,2,                                
    8, 8,5,1,2,4,9,1,3,                                
    12, 4,5,3,14,1,1,1,1,4,1,1,3,                                
    19, 3,3,2,2,2,4,1,1,1,1,1,1,1,1,3,1,1,3,2,                                
    11, 8,2,7,2,1,1,2,1,1,3,3,                                
    13, 1,5,9,12,2,1,1,3,1,1,2,2,1,                                
    17, 3,2,2,1,1,1,1,4,1,1,1,3,3,1,1,2,2,                                
    12, 5,2,2,2,2,1,5,2,1,1,2,5,                                
    12, 3,5,9,2,1,1,6,3,1,3,2,3,                                
    12, 1,4,1,1,1,4,1,5,5,3,3,3,                                
    10, 4,1,1,1,1,3,4,6,6,3,                                
    12, 3,1,3,1,1,3,3,1,1,4,6,1,                                
    11, 3,1,5,1,1,3,1,1,9,4,1,                                
    14, 2,1,1,7,1,4,1,1,1,1,1,1,3,5,                                
    11, 9,2,1,3,1,1,1,1,4,2,1,                                
    10, 1,14,1,1,2,2,2,10,1,2,                                
    10, 1,9,2,1,2,6,1,5,3,2,                                
    12, 1,9,9,1,2,2,3,1,1,4,3,1,                                
    10, 10,1,3,4,1,3,2,1,2,8,                                
    9, 9,1,3,5,1,1,1,2,7,                                
    12, 4,5,1,2,5,1,3,1,1,2,1,3,                                
    14, 1,1,1,1,2,6,2,3,2,1,1,2,3,1,                                
    11, 1,6,1,5,7,1,3,3,2,4,3,                                
    10, 1,2,1,2,9,1,5,2,6,2,                                
    8, 10,2,2,13,1,3,3,1,                                
    11, 2,2,1,6,2,3,3,2,2,2,1,                                
    12, 2,2,1,1,12,2,2,9,2,2,2,2,                                
    9, 5,1,2,4,1,5,11,2,2,                                
    3, 15,6,18,
  };

  /** \brief Specification for a picture of cupid.
   *
   * From http://www.icparc.ic.ac.uk/eclipse/examples/nono.ecl.txt, the
   * hardest instance.
   */
  const int p200[] =
    { 25, 25,
      // Column constraints
      4, 1,1,2,2,
      3, 5,5,7,
      4, 5,2,2,9,
      4, 3,2,3,9,
      5, 1,1,3,2,7,
      3, 3,1,5,
      5, 7,1,1,1,3,
      6, 1,2,1,1,2,1,
      3, 4,2,4,
      4, 1,2,2,2,
      3, 4,6,2,
      4, 1,2,2,1,
      4, 3,3,2,1,
      3, 4,1,15,
      6, 1,1,1,3,1,1,
      6, 2,1,1,2,2,3,
      4, 1,4,4,1,
      4, 1,4,3,2,
      4, 1,1,2,2,
      5, 7,2,3,1,1,
      5, 2,1,1,1,5,
      3, 1,2,5,
      4, 1,1,1,3,
      3, 4,2,1,
      1, 3,
      // Row constraints
      3, 2,2,3,
      5, 4,1,1,1,4,
      5, 4,1,2,1,1,
      7, 4,1,1,1,1,1,1,
      6, 2,1,1,2,3,5,
      6, 1,1,1,1,2,1,
      5, 3,1,5,1,2,
      6, 3,2,2,1,2,2,
      7, 2,1,4,1,1,1,1,
      6, 2,2,1,2,1,2,
      6, 1,1,1,3,2,3,
      5, 1,1,2,7,3,
      5, 1,2,2,1,5,
      5, 3,2,2,1,2,
      4, 3,2,1,2,
      3, 5,1,2,
      4, 2,2,1,2,
      4, 4,2,1,2,
      4, 6,2,3,2,
      4, 7,4,3,2,
      3, 7,4,4,
      3, 7,1,4,
      3, 6,1,4,
      3, 4,2,2,
      2, 2,1
    };


  const int *specs[] = {heart, bear, crocodile, unknown,
                        pinwheel, difficult, non_unique, dragonfly, 
                        castle, p200};
  const unsigned n_examples = sizeof(specs)/sizeof(int*);
  //@}

}

// STATISTICS: example-any
