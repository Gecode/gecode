/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>

 *  Copyright:
 *     Christian Schulte, 2001
 *     Samuel Gagnon, 2018
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
  /// List of specifications
  extern const int *specs[];
  /// Number of specifications
  extern const unsigned int n_examples;
}

/**
 * \brief %Example: Magic squares
 *
 * Compute magic squares of arbitrary size
 *
 * See problem 19 at http://www.csplib.org/.
 *
 * \ingroup Example
 *
 */
class MagicSquare : public Script {
private:
  /// Specification to be used
  const int* spec;
  /// Number of filled squares
  const int n_filled;
  /// Size of magic square
  const int n;
  /// Fields of square
  IntVarArray x;

public:
  /// Branching to use for model
  enum {
    BRANCH_SIZE,    ///< Branch by size
    BRANCH_AFC_SIZE, ///< Branch by size over AFC
    BRANCH_CBS_MAX_SD ///< Use maximum solution density
  };
  /// Post constraints
  MagicSquare(const SizeOptions& opt)
    : Script(opt), spec(specs[opt.size()]),
      n_filled(spec[1]), n(spec[0]), x(*this,n*n,1,n*n) {
    // Number of fields on square
    const int nn = n*n;

    // Sum of all a row, column, or diagonal
    const int s  = nn*(nn+1) / (2*n);

    // Matrix-wrapper for the square
    Matrix<IntVarArray> m(x, n, n);

    for (int i=0; i<n_filled; i++) {
      int row, col, num;
      {
        int idx = 3 * i + 2;
        row = spec[idx] - 1;
        col = spec[idx + 1] - 1;
        num = spec[idx + 2];
      }
      rel(*this, m(col,row), IRT_EQ, num);
    }

    for (int i = n; i--; ) {
      linear(*this, m.row(i), IRT_EQ, s, opt.ipl());
      linear(*this, m.col(i), IRT_EQ, s, opt.ipl());
    }
    // Both diagonals must have sum s
    {
      IntVarArgs d1y(n);
      IntVarArgs d2y(n);
      for (int i = n; i--; ) {
        d1y[i] = m(i,i);
        d2y[i] = m(n-i-1,i);
      }
      linear(*this, d1y, IRT_EQ, s, opt.ipl());
      linear(*this, d2y, IRT_EQ, s, opt.ipl());
    }

    // All fields must be distinct
    distinct(*this, x, opt.ipl());

    switch (opt.branching()) {
    case BRANCH_CBS_MAX_SD:
#ifdef GECODE_HAS_CBS
      cbsbranch(*this, x);
#endif
    case BRANCH_SIZE:
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
      break;
    case BRANCH_AFC_SIZE:
      branch(*this, x, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
      break;
    }
  }

  /// Constructor for cloning \a s
  MagicSquare(MagicSquare& s)
    : Script(s), spec(s.spec), n_filled(s.n_filled), n(s.n) {
    x.update(*this, s.x);
  }

  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new MagicSquare(*this);
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    // Matrix-wrapper for the square
    Matrix<IntVarArray> m(x, n, n);
    for (int i = 0; i<n; i++) {
      os << "\t";
      for (int j = 0; j<n; j++) {
        os.width(2);
        os << m(i,j) << "  ";
      }
      os << std::endl;
    }
  }

};

/** \brief Main-function
 *  \relates MagicSquare
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("MagicSquare");
  opt.iterations(1);
  opt.size(0);
  opt.branching(MagicSquare::BRANCH_SIZE);
  opt.branching(MagicSquare::BRANCH_SIZE, "size");
  opt.branching(MagicSquare::BRANCH_AFC_SIZE, "afc-size");
#ifdef GECODE_HAS_CBS
  opt.branching(MagicSquare::BRANCH_CBS_MAX_SD, "maxSD");
#endif
  opt.parse(argc,argv);
  Script::run<MagicSquare,DFS,SizeOptions>(opt);
  return 0;
}

namespace {

  /** \name Magic-square specifications
   *
   * A specification is given by a list of integers. The first two integers (n
   * and n_filled) specify the dimension of the magic square and the number of
   * pre-filled square respectively. Then n_filled triplets <i, j, v> of
   * integers follows, with i and j representing the row and the column of the
   * value v.
   *
   * \relates Magic-square
   */
  //@{
  const int magicSquare5_filled10_10[] = {
    5,10,
    1,2,18,
    1,3,9,
    1,5,2,
    2,2,5,
    3,1,1,
    3,5,24,
    4,2,21,
    4,3,6,
    5,2,13,
    5,4,10,
  };

  const int magicSquare5_filled10_11[] = {
    5,10,
    1,3,6,
    1,5,2,
    2,2,8,
    2,5,25,
    3,1,1,
    3,4,17,
    3,5,24,
    4,1,9,
    4,2,21,
    5,3,16,
  };

  const int magicSquare5_filled10_12[] = {
    5,10,
    1,2,5,
    1,4,21,
    2,1,10,
    2,2,16,
    2,3,8,
    3,4,12,
    4,2,20,
    4,5,11,
    5,1,18,
    5,5,3,
  };

  const int magicSquare5_filled10_13[] = {
    5,10,
    1,5,2,
    1,2,16,
    2,3,8,
    3,3,21,
    3,1,1,
    4,1,12,
    5,2,6,
    5,1,20,
    5,3,14,
    5,5,3,
  };

  const int magicSquare5_filled10_14[] = {
    5,10,
    1,3,14,
    1,2,7,
    2,3,12,
    2,4,4,
    3,3,21,
    3,5,24,
    4,5,11,
    4,4,6,
    4,3,13,
    5,4,22,
  };

  const int magicSquare5_filled10_15[] = {
    5,10,
    1,5,2,
    1,1,21,
    2,1,15,
    2,2,5,
    4,2,18,
    4,5,11,
    5,4,20,
    5,5,3,
    5,2,12,
    5,3,8,
  };

  const int magicSquare5_filled10_16[] = {
    5,10,
    1,1,22,
    1,5,2,
    1,4,18,
    2,5,25,
    2,4,7,
    3,4,15,
    3,1,1,
    3,2,9,
    4,4,4,
    5,1,23,
  };

  const int magicSquare5_filled10_17[] = {
    5,10,
    1,2,5,
    2,4,4,
    2,5,25,
    3,4,16,
    3,5,24,
    3,3,14,
    3,1,1,
    4,2,22,
    5,3,13,
    5,4,7,
  };

  const int magicSquare5_filled10_18[] = {
    5,10,
    1,3,7,
    1,4,22,
    2,5,25,
    2,4,4,
    2,1,10,
    3,1,1,
    4,4,6,
    4,3,20,
    5,3,8,
    5,2,12,
  };

  const int magicSquare5_filled10_19[] = {
    5,10,
    1,2,6,
    1,5,2,
    2,2,18,
    2,1,13,
    3,5,24,
    3,3,17,
    5,2,7,
    5,1,22,
    5,4,21,
    5,3,12,
  };

  const int magicSquare5_filled10_1[] = {
    5,10,
    1,4,16,
    1,5,2,
    1,3,20,
    2,5,25,
    2,4,4,
    3,5,24,
    3,1,1,
    4,4,9,
    5,2,12,
    5,1,23,
  };

  const int magicSquare5_filled10_20[] = {
    5,10,
    1,1,22,
    2,5,25,
    2,2,5,
    3,5,24,
    3,1,1,
    4,4,19,
    4,5,11,
    5,1,23,
    5,2,8,
    5,3,10,
  };

  const int magicSquare5_filled10_2[] = {
    5,10,
    1,2,19,
    1,4,9,
    2,5,25,
    2,1,15,
    3,2,6,
    4,2,20,
    4,1,5,
    5,5,3,
    5,3,10,
    5,1,23,
  };

  const int magicSquare5_filled10_3[] = {
    5,10,
    1,1,15,
    1,5,2,
    2,1,20,
    2,3,7,
    3,4,5,
    4,1,6,
    4,4,16,
    5,1,23,
    5,2,12,
    5,3,8,
  };

  const int magicSquare5_filled10_4[] = {
    5,10,
    1,2,15,
    1,5,2,
    1,1,22,
    1,4,16,
    2,5,25,
    3,5,24,
    3,1,1,
    4,1,23,
    4,2,18,
    4,3,9,
  };

  const int magicSquare5_filled10_5[] = {
    5,10,
    1,1,14,
    1,4,21,
    2,2,12,
    2,5,25,
    3,2,5,
    3,3,19,
    3,5,24,
    4,2,18,
    5,1,22,
    5,2,10,
  };

  const int magicSquare5_filled10_6[] = {
    5,10,
    1,1,19,
    1,2,20,
    2,1,7,
    3,3,23,
    4,5,11,
    4,4,4,
    5,3,13,
    5,5,3,
    5,4,22,
    5,2,10,
  };

  const int magicSquare5_filled10_7[] = {
    5,10,
    1,2,10,
    2,4,4,
    3,5,24,
    3,4,17,
    4,5,11,
    4,2,23,
    4,4,5,
    4,1,14,
    5,2,6,
    5,5,3,
  };

  const int magicSquare5_filled10_8[] = {
    5,10,
    1,1,22,
    1,3,5,
    2,5,25,
    2,4,4,
    3,5,24,
    3,2,8,
    4,1,10,
    4,2,21,
    4,3,16,
    4,5,11,
  };

  const int magicSquare5_filled10_9[] = {
    5,10,
    1,5,2,
    2,2,10,
    2,5,25,
    3,3,23,
    3,1,1,
    4,2,15,
    4,4,13,
    4,3,6,
    5,5,3,
    5,3,5,
  };

  const int magicSquare5_filled11_3_1[] = {
    5,11,
    1,1,15,
    1,5,2,
    2,1,20,
    2,3,7,
    3,2,13,
    3,4,5,
    4,1,6,
    4,4,16,
    5,1,23,
    5,2,12,
    5,3,8,
  };

  const int magicSquare5_filled11_5_1[] = {
    5,11,
    1,1,14,
    1,4,21,
    2,2,12,
    2,5,25,
    3,2,5,
    3,3,19,
    3,4,16,
    3,5,24,
    4,2,18,
    5,1,22,
    5,2,10,
  };

  const int magicSquare5_filled11_5_2[] = {
    5,11,
    1,1,14,
    1,4,21,
    2,2,12,
    2,5,25,
    3,2,5,
    3,3,19,
    3,5,24,
    4,2,18,
    5,1,22,
    5,2,10,
    5,5,3,
  };

  const int magicSquare5_filled11_5_3[] = {
    5,11,
    1,1,14,
    1,3,8,
    1,4,21,
    2,2,12,
    2,5,25,
    3,2,5,
    3,3,19,
    3,5,24,
    4,2,18,
    5,1,22,
    5,2,10,
  };

  const int magicSquare5_filled12_10_1[] = {
    5,12,
    1,2,18,
    1,3,9,
    1,5,2,
    2,2,5,
    3,1,1,
    3,3,15,
    3,5,24,
    4,1,7,
    4,2,21,
    4,3,6,
    5,2,13,
    5,4,10,
  };

  const int magicSquare5_filled12_1_1[] = {
    5,12,
    1,1,21,
    1,3,20,
    1,4,16,
    1,5,2,
    2,5,25,
    2,4,4,
    3,1,1,
    3,3,14,
    3,5,24,
    4,4,9,
    5,2,12,
    5,1,23,
  };

  const int magicSquare5_filled12_1_2[] = {
    5,12,
    1,3,20,
    1,4,16,
    1,5,2,
    2,2,18,
    2,4,4,
    2,5,25,
    3,1,1,
    3,3,14,
    3,5,24,
    4,4,9,
    5,2,12,
    5,1,23,
  };

  const int magicSquare5_filled12_1_3[] = {
    5,12,
    1,3,20,
    1,4,16,
    1,5,2,
    2,2,18,
    2,4,4,
    2,5,25,
    3,1,1,
    3,5,24,
    4,4,9,
    5,1,23,
    5,2,12,
    5,3,10,
  };

  const int magicSquare5_filled12_2_1[] = {
    5,12,
    1,1,21,
    1,2,19,
    1,4,9,
    2,1,15,
    2,5,25,
    3,2,6,
    3,4,18,
    4,1,5,
    4,2,20,
    5,1,23,
    5,3,10,
    5,5,3,
  };

  const int magicSquare5_filled12_2_2[] = {
    5,12,
    1,2,19,
    1,4,9,
    2,1,15,
    2,3,8,
    2,5,25,
    3,2,6,
    3,4,18,
    4,1,5,
    4,2,20,
    5,1,23,
    5,3,10,
    5,5,3,
  };

  const int magicSquare5_filled12_2_3[] = {
    5,12,
    1,1,21,
    1,2,19,
    1,4,9,
    2,1,15,
    2,5,25,
    3,2,6,
    4,1,5,
    4,2,20,
    4,4,12,
    5,1,23,
    5,3,10,
    5,5,3,
  };

  const int magicSquare5_filled12_3_1[] = {
    5,12,
    1,1,15,
    1,4,21,
    1,5,2,
    2,1,20,
    2,3,7,
    3,4,5,
    4,1,6,
    4,4,16,
    5,1,23,
    5,2,12,
    5,3,8,
    5,5,3,
  };

  const int magicSquare5_filled12_3_2[] = {
    5,12,
    1,1,15,
    1,2,17,
    1,3,10,
    1,5,2,
    2,1,20,
    2,3,7,
    3,4,5,
    4,1,6,
    4,4,16,
    5,1,23,
    5,2,12,
    5,3,8,
  };

  const int magicSquare9_filled10_10[] = {
    9,10,
    8,2,19,
    6,7,25,
    7,3,15,
    8,4,77,
    7,1,29,
    4,9,63,
    4,6,53,
    7,1,29,
    3,9,36,
    5,4,74,
  };

  const int magicSquare9_filled10_11[] = {
    9,10,
    5,5,78,
    6,5,56,
    9,8,30,
    3,3,38,
    2,3,9,
    1,2,23,
    3,5,80,
    9,7,52,
    7,5,1,
    1,1,32,
  };

  const int magicSquare9_filled10_12[] = {
    9,10,
    6,7,25,
    1,4,2,
    5,1,54,
    3,3,38,
    7,2,71,
    1,5,28,
    3,2,50,
    2,7,59,
    7,7,42,
    5,4,74,
  };

  const int magicSquare9_filled10_13[] = {
    9,10,
    7,2,71,
    8,9,22,
    2,2,17,
    3,3,38,
    1,8,72,
    6,5,56,
    5,6,41,
    9,1,8,
    7,8,37,
    3,3,38,
  };

  const int magicSquare9_filled10_14[] = {
    9,10,
    9,4,35,
    1,3,48,
    9,6,69,
    7,6,70,
    6,1,46,
    5,1,54,
    4,2,62,
    2,8,67,
    7,6,70,
    7,3,15,
  };

  const int magicSquare9_filled10_15[] = {
    9,10,
    9,8,30,
    1,6,4,
    9,9,45,
    9,2,49,
    5,6,41,
    6,5,56,
    5,9,7,
    2,6,3,
    3,2,50,
    1,2,23,
  };

  const int magicSquare9_filled10_16[] = {
    9,10,
    1,9,81,
    3,8,18,
    8,4,77,
    6,5,56,
    7,9,31,
    1,9,81,
    7,3,15,
    6,4,5,
    6,8,61,
    4,6,53,
  };

  const int magicSquare9_filled10_17[] = {
    9,10,
    4,4,12,
    9,5,26,
    7,6,70,
    8,2,19,
    7,2,71,
    8,1,68,
    8,2,19,
    5,5,78,
    6,3,21,
    5,3,75,
  };

  const int magicSquare9_filled10_18[] = {
    9,10,
    7,2,71,
    4,1,47,
    6,9,24,
    8,8,39,
    3,9,36,
    2,9,60,
    4,3,64,
    5,8,11,
    4,2,62,
    3,7,43,
  };

  const int magicSquare9_filled10_19[] = {
    9,10,
    7,4,73,
    8,3,44,
    8,1,68,
    1,2,23,
    1,6,4,
    5,7,16,
    3,6,6,
    8,2,19,
    4,5,14,
    3,3,38,
  };

  const int magicSquare9_filled10_1[] = {
    9,10,
    4,5,14,
    6,7,25,
    8,8,39,
    6,7,25,
    3,8,18,
    4,4,12,
    8,2,19,
    4,1,47,
    4,6,53,
    8,5,10,
  };

  const int magicSquare9_filled10_20[] = {
    9,10,
    1,4,2,
    8,6,57,
    1,5,28,
    1,8,72,
    6,1,46,
    4,1,47,
    8,7,33,
    4,6,53,
    1,5,28,
    5,8,11,
  };

  const int magicSquare9_filled10_2[] = {
    9,10,
    2,5,76,
    9,1,8,
    2,4,51,
    1,2,23,
    9,7,52,
    1,8,72,
    3,3,38,
    6,6,66,
    3,7,43,
    7,6,70,
  };

  const int magicSquare9_filled10_3[] = {
    9,10,
    5,6,41,
    5,3,75,
    6,1,46,
    3,2,50,
    3,8,18,
    1,5,28,
    3,2,50,
    5,9,7,
    4,8,34,
    1,4,2,
  };

  const int magicSquare9_filled10_4[] = {
    9,10,
    4,9,63,
    9,4,35,
    7,5,1,
    2,6,3,
    8,4,77,
    7,6,70,
    5,8,11,
    2,5,76,
    9,4,35,
    3,6,6,
  };

  const int magicSquare9_filled10_5[] = {
    9,10,
    2,5,76,
    1,4,2,
    9,3,55,
    1,2,23,
    3,3,38,
    2,5,76,
    3,4,40,
    3,4,40,
    1,2,23,
    3,6,6,
  };

  const int magicSquare9_filled10_6[] = {
    9,10,
    7,5,1,
    4,7,20,
    8,7,33,
    5,5,78,
    5,2,13,
    3,1,58,
    1,9,81,
    5,1,54,
    3,1,58,
    7,5,1,
  };

  const int magicSquare9_filled10_7[] = {
    9,10,
    7,5,1,
    7,5,1,
    1,5,28,
    4,8,34,
    4,1,47,
    4,2,62,
    2,6,3,
    2,8,67,
    1,9,81,
    5,4,74,
  };

  const int magicSquare9_filled10_8[] = {
    9,10,
    9,4,35,
    4,9,63,
    1,6,4,
    5,5,78,
    7,5,1,
    9,4,35,
    5,6,41,
    6,3,21,
    9,3,55,
    3,5,80,
  };

  const int magicSquare9_filled10_9[] = {
    9,10,
    4,8,34,
    4,1,47,
    3,9,36,
    8,6,57,
    9,2,49,
    9,9,45,
    1,6,4,
    4,6,53,
    3,6,6,
    2,2,17,
  };

  const int magicSquare9_filled50_10[] = {
    9,50,
    4,3,64,
    3,9,36,
    8,3,44,
    6,9,24,
    2,7,59,
    7,2,71,
    8,2,19,
    2,8,67,
    6,7,25,
    6,6,66,
    3,8,18,
    1,9,81,
    8,4,77,
    5,4,74,
    8,2,19,
    6,9,24,
    4,9,63,
    9,3,55,
    2,5,76,
    9,2,49,
    3,7,43,
    3,8,18,
    6,3,21,
    6,2,65,
    9,2,49,
    7,9,31,
    8,6,57,
    8,6,57,
    9,3,55,
    7,5,1,
    2,4,51,
    5,4,74,
    3,4,40,
    4,2,62,
    6,3,21,
    3,6,6,
    7,4,73,
    5,4,74,
    4,8,34,
    3,3,38,
    8,1,68,
    3,4,40,
    6,8,61,
    7,3,15,
    9,5,26,
    7,1,29,
    6,9,24,
    2,6,3,
    3,5,80,
    7,9,31,
  };

  const int magicSquare9_filled50_11[] = {
    9,50,
    6,8,61,
    3,3,38,
    9,7,52,
    6,3,21,
    4,9,63,
    3,9,36,
    7,3,15,
    1,1,32,
    2,7,59,
    1,8,72,
    2,5,76,
    8,7,33,
    5,1,54,
    3,5,80,
    3,8,18,
    4,8,34,
    4,5,14,
    2,3,9,
    9,5,26,
    3,3,38,
    2,5,76,
    9,8,30,
    8,9,22,
    6,7,25,
    7,6,70,
    5,6,41,
    2,1,27,
    4,4,12,
    8,4,77,
    8,2,19,
    9,1,8,
    7,3,15,
    5,6,41,
    3,4,40,
    2,5,76,
    5,1,54,
    8,4,77,
    7,4,73,
    2,3,9,
    1,8,72,
    7,3,15,
    4,6,53,
    3,5,80,
    9,2,49,
    7,7,42,
    1,6,4,
    7,7,42,
    7,2,71,
    4,7,20,
    4,3,64,
  };

  const int magicSquare9_filled50_12[] = {
    9,50,
    1,3,48,
    3,5,80,
    4,5,14,
    6,1,46,
    8,8,39,
    2,8,67,
    8,8,39,
    9,2,49,
    2,2,17,
    6,1,46,
    9,9,45,
    8,9,22,
    3,4,40,
    2,3,9,
    4,6,53,
    5,3,75,
    6,6,66,
    7,1,29,
    8,1,68,
    1,7,79,
    7,9,31,
    5,3,75,
    6,4,5,
    5,7,16,
    3,1,58,
    5,3,75,
    7,4,73,
    9,8,30,
    5,8,11,
    1,7,79,
    4,4,12,
    9,8,30,
    7,4,73,
    6,5,56,
    4,4,12,
    2,8,67,
    1,4,2,
    2,6,3,
    6,4,5,
    1,8,72,
    2,6,3,
    8,9,22,
    7,7,42,
    5,9,7,
    5,5,78,
    6,7,25,
    6,5,56,
    3,3,38,
    8,8,39,
    8,1,68,
  };

  const int magicSquare9_filled50_13[] = {
    9,50,
    2,7,59,
    6,2,65,
    9,5,26,
    6,5,56,
    8,6,57,
    1,1,32,
    9,9,45,
    7,6,70,
    4,2,62,
    4,9,63,
    4,9,63,
    4,8,34,
    3,6,6,
    1,1,32,
    2,6,3,
    8,3,44,
    2,3,9,
    2,1,27,
    7,7,42,
    3,6,6,
    2,3,9,
    4,1,47,
    2,1,27,
    5,4,74,
    9,8,30,
    3,1,58,
    7,4,73,
    6,9,24,
    7,5,1,
    8,8,39,
    1,4,2,
    1,9,81,
    6,9,24,
    7,2,71,
    5,9,7,
    5,6,41,
    1,8,72,
    4,2,62,
    6,8,61,
    3,3,38,
    6,5,56,
    1,2,23,
    6,7,25,
    8,3,44,
    2,4,51,
    1,9,81,
    8,8,39,
    9,2,49,
    6,6,66,
    3,1,58,
  };

  const int magicSquare9_filled50_14[] = {
    9,50,
    1,4,2,
    6,7,25,
    3,2,50,
    6,6,66,
    2,8,67,
    5,1,54,
    2,6,3,
    7,7,42,
    6,2,65,
    2,8,67,
    8,3,44,
    2,9,60,
    4,5,14,
    9,5,26,
    4,1,47,
    5,2,13,
    4,9,63,
    6,5,56,
    1,9,81,
    1,2,23,
    8,4,77,
    1,9,81,
    9,7,52,
    5,3,75,
    8,6,57,
    1,7,79,
    8,9,22,
    4,9,63,
    5,1,54,
    3,6,6,
    1,7,79,
    5,3,75,
    6,2,65,
    5,6,41,
    1,3,48,
    6,8,61,
    6,6,66,
    6,3,21,
    3,1,58,
    6,8,61,
    6,4,5,
    5,4,74,
    4,7,20,
    1,6,4,
    7,3,15,
    2,6,3,
    1,6,4,
    6,4,5,
    5,1,54,
    8,4,77,
  };

  const int magicSquare9_filled50_15[] = {
    9,50,
    1,4,2,
    2,7,59,
    7,5,1,
    7,7,42,
    5,3,75,
    5,1,54,
    7,8,37,
    2,8,67,
    3,3,38,
    4,9,63,
    3,3,38,
    3,1,58,
    6,8,61,
    5,2,13,
    6,1,46,
    5,5,78,
    4,4,12,
    2,8,67,
    9,8,30,
    5,2,13,
    9,7,52,
    3,4,40,
    5,2,13,
    9,7,52,
    2,3,9,
    5,5,78,
    3,5,80,
    5,8,11,
    2,7,59,
    9,7,52,
    7,2,71,
    9,8,30,
    4,1,47,
    6,1,46,
    7,8,37,
    2,4,51,
    6,2,65,
    5,8,11,
    2,4,51,
    6,3,21,
    6,8,61,
    5,8,11,
    3,8,18,
    4,4,12,
    5,4,74,
    9,1,8,
    3,8,18,
    8,4,77,
    7,3,15,
    4,2,62,
  };

  const int magicSquare9_filled50_16[] = {
    9,50,
    1,4,2,
    3,6,6,
    3,7,43,
    5,4,74,
    1,8,72,
    5,6,41,
    4,7,20,
    2,6,3,
    3,5,80,
    8,8,39,
    6,5,56,
    6,7,25,
    3,4,40,
    1,7,79,
    4,3,64,
    8,5,10,
    4,8,34,
    8,6,57,
    5,3,75,
    8,5,10,
    8,1,68,
    8,2,19,
    5,9,7,
    8,8,39,
    3,4,40,
    4,6,53,
    8,9,22,
    3,8,18,
    1,2,23,
    6,5,56,
    4,2,62,
    9,7,52,
    1,7,79,
    1,5,28,
    8,8,39,
    1,6,4,
    6,8,61,
    8,2,19,
    6,4,5,
    9,6,69,
    5,3,75,
    2,3,9,
    2,3,9,
    1,9,81,
    4,4,12,
    4,5,14,
    6,3,21,
    2,4,51,
    8,9,22,
    8,6,57,
  };

  const int magicSquare9_filled50_17[] = {
    9,50,
    8,6,57,
    9,2,49,
    3,5,80,
    3,6,6,
    8,2,19,
    9,1,8,
    2,1,27,
    3,1,58,
    1,2,23,
    1,2,23,
    5,2,13,
    6,8,61,
    5,5,78,
    2,1,27,
    5,8,11,
    4,1,47,
    4,3,64,
    2,4,51,
    8,3,44,
    9,4,35,
    2,8,67,
    5,4,74,
    7,5,1,
    2,7,59,
    6,2,65,
    9,9,45,
    4,3,64,
    7,6,70,
    8,9,22,
    4,3,64,
    7,7,42,
    1,1,32,
    7,2,71,
    3,3,38,
    2,2,17,
    5,4,74,
    8,7,33,
    7,3,15,
    2,8,67,
    7,6,70,
    8,4,77,
    5,9,7,
    7,2,71,
    3,3,38,
    1,6,4,
    5,5,78,
    1,3,48,
    4,7,20,
    2,6,3,
    8,4,77,
  };

  const int magicSquare9_filled50_18[] = {
    9,50,
    5,3,75,
    7,1,29,
    9,4,35,
    3,8,18,
    9,1,8,
    4,5,14,
    2,6,3,
    2,8,67,
    8,4,77,
    1,6,4,
    7,3,15,
    2,5,76,
    5,5,78,
    3,7,43,
    1,8,72,
    8,5,10,
    1,5,28,
    6,7,25,
    6,6,66,
    5,3,75,
    6,7,25,
    8,8,39,
    3,9,36,
    4,1,47,
    2,3,9,
    7,8,37,
    5,6,41,
    4,8,34,
    1,4,2,
    3,8,18,
    9,1,8,
    3,9,36,
    5,6,41,
    6,8,61,
    3,8,18,
    1,6,4,
    5,8,11,
    2,8,67,
    6,4,5,
    6,5,56,
    6,1,46,
    1,8,72,
    6,2,65,
    6,6,66,
    5,6,41,
    4,2,62,
    6,5,56,
    1,8,72,
    8,4,77,
    4,8,34,
  };

  const int magicSquare9_filled50_19[] = {
    9,50,
    1,5,28,
    3,5,80,
    1,2,23,
    1,4,2,
    5,7,16,
    8,8,39,
    5,9,7,
    7,2,71,
    1,1,32,
    5,4,74,
    7,9,31,
    5,1,54,
    2,4,51,
    7,9,31,
    7,1,29,
    6,5,56,
    3,8,18,
    1,4,2,
    9,8,30,
    7,3,15,
    3,4,40,
    1,8,72,
    3,5,80,
    9,3,55,
    6,4,5,
    6,1,46,
    1,9,81,
    1,2,23,
    3,5,80,
    9,7,52,
    4,5,14,
    3,6,6,
    1,1,32,
    7,9,31,
    8,3,44,
    2,2,17,
    6,1,46,
    9,6,69,
    5,6,41,
    8,1,68,
    7,3,15,
    1,8,72,
    9,9,45,
    7,2,71,
    2,4,51,
    8,5,10,
    6,1,46,
    2,6,3,
    1,6,4,
    4,7,20,
  };

  const int magicSquare9_filled50_1[] = {
    9,50,
    7,1,29,
    2,9,60,
    2,4,51,
    1,8,72,
    8,5,10,
    4,6,53,
    3,4,40,
    6,2,65,
    8,2,19,
    3,6,6,
    6,2,65,
    2,8,67,
    2,5,76,
    8,9,22,
    6,2,65,
    6,1,46,
    2,8,67,
    1,4,2,
    9,8,30,
    2,5,76,
    3,3,38,
    1,3,48,
    4,6,53,
    5,1,54,
    7,5,1,
    6,1,46,
    4,6,53,
    6,5,56,
    1,5,28,
    3,6,6,
    4,8,34,
    7,6,70,
    4,5,14,
    7,3,15,
    1,6,4,
    8,2,19,
    7,6,70,
    4,8,34,
    3,8,18,
    8,7,33,
    1,5,28,
    6,5,56,
    8,2,19,
    9,8,30,
    4,2,62,
    3,8,18,
    8,7,33,
    4,2,62,
    9,8,30,
    5,9,7,
  };

  const int magicSquare9_filled50_20[] = {
    9,50,
    8,3,44,
    8,2,19,
    3,5,80,
    7,8,37,
    8,4,77,
    6,5,56,
    4,5,14,
    1,3,48,
    2,6,3,
    4,3,64,
    9,9,45,
    6,6,66,
    8,5,10,
    2,6,3,
    1,5,28,
    3,7,43,
    8,8,39,
    8,8,39,
    1,4,2,
    4,8,34,
    5,8,11,
    2,8,67,
    1,2,23,
    1,2,23,
    7,2,71,
    2,7,59,
    1,7,79,
    1,6,4,
    2,9,60,
    3,1,58,
    3,3,38,
    7,8,37,
    9,3,55,
    6,9,24,
    4,1,47,
    8,8,39,
    6,9,24,
    6,6,66,
    8,4,77,
    5,6,41,
    5,6,41,
    1,4,2,
    2,8,67,
    9,1,8,
    7,9,31,
    1,9,81,
    3,7,43,
    7,2,71,
    8,2,19,
    8,2,19,
  };

  const int magicSquare9_filled50_2[] = {
    9,50,
    2,1,27,
    8,8,39,
    6,3,21,
    7,6,70,
    8,5,10,
    2,9,60,
    7,7,42,
    4,5,14,
    8,1,68,
    2,3,9,
    3,2,50,
    8,1,68,
    8,9,22,
    9,5,26,
    7,2,71,
    2,8,67,
    2,1,27,
    5,6,41,
    3,2,50,
    9,8,30,
    4,1,47,
    7,9,31,
    7,1,29,
    4,4,12,
    2,3,9,
    4,2,62,
    4,9,63,
    9,9,45,
    8,8,39,
    2,5,76,
    1,4,2,
    1,9,81,
    4,5,14,
    3,4,40,
    4,3,64,
    2,8,67,
    1,9,81,
    7,8,37,
    7,8,37,
    9,6,69,
    9,1,8,
    5,1,54,
    9,2,49,
    1,7,79,
    1,9,81,
    9,8,30,
    3,7,43,
    5,4,74,
    3,8,18,
    7,4,73,
  };

  const int magicSquare9_filled50_3[] = {
    9,50,
    8,7,33,
    9,8,30,
    4,4,12,
    4,1,47,
    3,3,38,
    5,9,7,
    1,9,81,
    9,7,52,
    9,7,52,
    2,7,59,
    7,1,29,
    5,9,7,
    8,7,33,
    2,8,67,
    3,6,6,
    2,1,27,
    3,9,36,
    9,6,69,
    3,3,38,
    5,3,75,
    4,9,63,
    2,4,51,
    6,2,65,
    2,5,76,
    8,1,68,
    9,5,26,
    2,2,17,
    3,7,43,
    9,2,49,
    3,2,50,
    7,2,71,
    3,1,58,
    1,2,23,
    6,4,5,
    4,1,47,
    6,5,56,
    7,8,37,
    9,2,49,
    9,8,30,
    4,5,14,
    8,4,77,
    8,7,33,
    5,8,11,
    2,4,51,
    9,4,35,
    4,6,53,
    4,6,53,
    6,4,5,
    5,1,54,
    7,6,70,
  };

  const int magicSquare9_filled50_4[] = {
    9,50,
    8,2,19,
    2,6,3,
    9,8,30,
    5,6,41,
    4,8,34,
    8,9,22,
    9,6,69,
    5,5,78,
    4,6,53,
    6,3,21,
    8,7,33,
    7,2,71,
    1,3,48,
    3,5,80,
    1,8,72,
    9,9,45,
    9,8,30,
    3,6,6,
    6,7,25,
    9,7,52,
    3,5,80,
    6,3,21,
    2,1,27,
    5,3,75,
    5,8,11,
    4,1,47,
    6,1,46,
    9,6,69,
    1,2,23,
    9,2,49,
    7,8,37,
    1,4,2,
    4,3,64,
    7,7,42,
    7,6,70,
    4,9,63,
    2,9,60,
    9,1,8,
    8,4,77,
    1,1,32,
    3,4,40,
    1,6,4,
    2,9,60,
    2,3,9,
    8,8,39,
    2,4,51,
    4,2,62,
    7,7,42,
    2,5,76,
    4,8,34,
  };

  const int magicSquare9_filled50_5[] = {
    9,50,
    8,7,33,
    7,7,42,
    5,7,16,
    5,1,54,
    8,6,57,
    1,1,32,
    9,8,30,
    4,2,62,
    5,6,41,
    2,1,27,
    2,3,9,
    4,6,53,
    2,9,60,
    3,3,38,
    4,5,14,
    1,9,81,
    9,5,26,
    7,2,71,
    9,2,49,
    2,8,67,
    7,9,31,
    6,5,56,
    5,1,54,
    4,7,20,
    4,5,14,
    8,5,10,
    5,9,7,
    1,6,4,
    8,2,19,
    8,9,22,
    6,6,66,
    9,3,55,
    1,6,4,
    5,1,54,
    5,6,41,
    8,3,44,
    4,2,62,
    7,8,37,
    2,8,67,
    4,5,14,
    3,2,50,
    1,7,79,
    1,8,72,
    3,7,43,
    9,8,30,
    6,3,21,
    4,5,14,
    6,2,65,
    1,1,32,
    2,6,3,
  };

  const int magicSquare9_filled50_6[] = {
    9,50,
    4,7,20,
    6,7,25,
    9,1,8,
    4,1,47,
    8,7,33,
    4,8,34,
    6,4,5,
    5,4,74,
    9,5,26,
    1,9,81,
    1,7,79,
    9,4,35,
    2,5,76,
    5,1,54,
    3,5,80,
    4,7,20,
    9,7,52,
    2,8,67,
    7,3,15,
    7,3,15,
    7,1,29,
    1,1,32,
    2,3,9,
    5,1,54,
    5,5,78,
    7,5,1,
    9,5,26,
    6,2,65,
    7,8,37,
    9,1,8,
    1,1,32,
    5,1,54,
    7,4,73,
    8,2,19,
    5,3,75,
    4,2,62,
    1,2,23,
    3,2,50,
    2,5,76,
    1,6,4,
    9,5,26,
    8,9,22,
    9,2,49,
    8,5,10,
    9,5,26,
    5,1,54,
    5,7,16,
    8,2,19,
    1,4,2,
    3,3,38,
  };

  const int magicSquare9_filled50_7[] = {
    9,50,
    6,3,21,
    7,6,70,
    3,4,40,
    5,2,13,
    6,6,66,
    7,8,37,
    8,6,57,
    8,1,68,
    2,9,60,
    4,8,34,
    7,3,15,
    2,3,9,
    4,3,64,
    2,2,17,
    7,2,71,
    3,3,38,
    4,9,63,
    8,4,77,
    2,4,51,
    3,5,80,
    7,1,29,
    3,5,80,
    6,1,46,
    4,7,20,
    1,5,28,
    3,5,80,
    8,4,77,
    7,2,71,
    6,8,61,
    3,3,38,
    8,4,77,
    3,9,36,
    1,2,23,
    4,2,62,
    3,6,6,
    4,7,20,
    4,7,20,
    9,9,45,
    5,1,54,
    4,3,64,
    6,6,66,
    8,2,19,
    8,5,10,
    3,2,50,
    2,4,51,
    3,7,43,
    5,5,78,
    4,5,14,
    4,7,20,
    5,4,74,
  };

  const int magicSquare9_filled50_8[] = {
    9,50,
    2,8,67,
    1,3,48,
    5,8,11,
    3,8,18,
    8,6,57,
    1,2,23,
    1,8,72,
    3,8,18,
    2,4,51,
    7,1,29,
    5,9,7,
    5,9,7,
    5,8,11,
    4,6,53,
    4,8,34,
    1,3,48,
    5,8,11,
    5,7,16,
    6,5,56,
    5,3,75,
    9,4,35,
    4,9,63,
    9,5,26,
    5,8,11,
    8,2,19,
    6,3,21,
    9,1,8,
    2,4,51,
    7,3,15,
    9,1,8,
    9,7,52,
    1,2,23,
    6,5,56,
    8,2,19,
    8,2,19,
    2,7,59,
    5,6,41,
    6,4,5,
    8,8,39,
    3,6,6,
    7,8,37,
    6,6,66,
    7,5,1,
    7,2,71,
    5,5,78,
    2,2,17,
    2,9,60,
    3,7,43,
    2,2,17,
    7,9,31,
  };

  const int magicSquare9_filled50_9[] = {
    9,50,
    3,8,18,
    6,7,25,
    2,9,60,
    8,9,22,
    7,8,37,
    3,3,38,
    5,6,41,
    8,2,19,
    8,4,77,
    3,4,40,
    8,2,19,
    5,9,7,
    8,8,39,
    5,9,7,
    7,9,31,
    9,9,45,
    5,5,78,
    4,7,20,
    3,9,36,
    4,9,63,
    8,5,10,
    9,3,55,
    8,6,57,
    2,7,59,
    9,2,49,
    1,7,79,
    1,3,48,
    4,8,34,
    8,6,57,
    7,5,1,
    5,6,41,
    2,1,27,
    9,3,55,
    5,2,13,
    3,6,6,
    8,1,68,
    8,8,39,
    1,7,79,
    2,2,17,
    4,1,47,
    1,2,23,
    5,1,54,
    2,6,3,
    8,1,68,
    3,5,80,
    3,5,80,
    9,3,55,
    3,8,18,
    5,7,16,
    7,5,1,
  };

  const int *specs[] = {
    magicSquare5_filled10_1,    //0
    magicSquare5_filled10_2,    //1
    magicSquare5_filled10_3,    //2
    magicSquare5_filled10_4,    //3
    magicSquare5_filled10_5,    //4
    magicSquare5_filled10_6,    //5
    magicSquare5_filled10_7,    //6
    magicSquare5_filled10_8,    //7
    magicSquare5_filled10_9,    //8
    magicSquare5_filled10_10,   //9
    magicSquare5_filled10_11,   //10
    magicSquare5_filled10_12,   //11
    magicSquare5_filled10_13,   //12
    magicSquare5_filled10_14,   //13
    magicSquare5_filled10_15,   //14
    magicSquare5_filled10_16,   //15
    magicSquare5_filled10_17,   //16
    magicSquare5_filled10_18,   //17
    magicSquare5_filled10_19,   //18
    magicSquare5_filled10_20,   //19
    magicSquare5_filled11_3_1,  //20
    magicSquare5_filled11_5_1,  //21
    magicSquare5_filled11_5_2,  //22
    magicSquare5_filled11_5_3,  //23
    magicSquare5_filled12_10_1, //24
    magicSquare5_filled12_1_1,  //25
    magicSquare5_filled12_1_2,  //26
    magicSquare5_filled12_1_3,  //27
    magicSquare5_filled12_2_1,  //28
    magicSquare5_filled12_2_2,  //29
    magicSquare5_filled12_2_3,  //30
    magicSquare5_filled12_3_1,  //31
    magicSquare5_filled12_3_2,  //32
    magicSquare9_filled10_1,    //33
    magicSquare9_filled10_2,    //34
    magicSquare9_filled10_3,    //35
    magicSquare9_filled10_4,    //36
    magicSquare9_filled10_5,    //37
    magicSquare9_filled10_6,    //38
    magicSquare9_filled10_7,    //39
    magicSquare9_filled10_8,    //40
    magicSquare9_filled10_9,    //41
    magicSquare9_filled10_10,   //42
    magicSquare9_filled10_11,   //43
    magicSquare9_filled10_12,   //44
    magicSquare9_filled10_13,   //45
    magicSquare9_filled10_14,   //46
    magicSquare9_filled10_15,   //47
    magicSquare9_filled10_16,   //48
    magicSquare9_filled10_17,   //49
    magicSquare9_filled10_18,   //50
    magicSquare9_filled10_19,   //51
    magicSquare9_filled10_20,   //52
    magicSquare9_filled50_1,    //53
    magicSquare9_filled50_2,    //54
    magicSquare9_filled50_3,    //55
    magicSquare9_filled50_4,    //56
    magicSquare9_filled50_5,    //57
    magicSquare9_filled50_6,    //58
    magicSquare9_filled50_7,    //59
    magicSquare9_filled50_8,    //60
    magicSquare9_filled50_9,    //61
    magicSquare9_filled50_10,   //62
    magicSquare9_filled50_11,   //63
    magicSquare9_filled50_12,   //64
    magicSquare9_filled50_13,   //65
    magicSquare9_filled50_14,   //66
    magicSquare9_filled50_15,   //67
    magicSquare9_filled50_16,   //68
    magicSquare9_filled50_17,   //69
    magicSquare9_filled50_18,   //70
    magicSquare9_filled50_19,   //71
    magicSquare9_filled50_20    //72
  };

  const unsigned n_examples = sizeof(specs)/sizeof(int*);
  //@}

}

// STATISTICS: example-any

