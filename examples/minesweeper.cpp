/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#include <cctype>

using namespace Gecode;

namespace {
  extern const char *specs[];
  extern const unsigned int n_examples;
  int spec_size(const char *s);
  int mineField(const char *s, int n, int i, int j);
}

/**
 * \brief %Example: Minesweeper
 *
 * This is the classical MineSweeper game.
 *
 * The instances are taken from
 *   http://www.janko.at/Raetsel/Minesweeper/index.htm
 *
 * \ingroup Example
 *
 */
class MineSweeper : public Script {
private:
  const char *spec;
  int size;
  BoolVarArray b;

  /// Access position (\a h,\a w) in the matrix.
  BoolVar pos(int h, int w) {
    return b[h*size + w];
  }
  /// Access position (\a h,\a w) in the matrix.
  const BoolVar pos(int h, int w) const {
    return b[h*size + w];
  }

  /// Return the fields in \a m around position (\a x,\a y)
  BoolVarArgs fieldsAround(Matrix<BoolVarArray>& m,
                           int x, int y) {
    int bvsize=0;
    for (int ix = std::max(0, x-1); ix<=x+1 && ix<size; ix++)
      for (int iy = std::max(0, y-1); iy<=y+1 && iy<size; iy++)
        bvsize++;
    bvsize--; // remove (x,y) itself
    BoolVarArgs b(bvsize);
    int count=0;
    for (int ix = std::max(0, x-1); ix<=x+1 && ix<size; ix++)
      for (int iy = std::max(0, y-1); iy<=y+1 && iy<size; iy++)
        if (ix != x || iy != y)
          b[count++] = m(ix,iy);

    return b;
  }

public:
  /// Actual model
  MineSweeper(const SizeOptions& opt)
    : spec(specs[opt.size()]),
      size(spec_size(spec)),
      b(*this,size*size,0,1) {
    Matrix<BoolVarArray> m(b, size, size);

    // Initialize matrix and post constraints
    for (int h=0; h<size; h++)
      for (int w=0; w<size; w++) {
        int v = mineField(spec, size, h, w);
        if (v != -1) {
          rel(*this, m(w, h), IRT_EQ, 0);
          linear(*this, fieldsAround(m, w, h), IRT_EQ, v);
        }
      }

    // Install branching
    branch(*this, b, INT_VAR_NONE, INT_VAL_MAX);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int h = 0; h < size; ++h) {
      os << '\t';
      for (int w = 0; w < size; ++w) {
        int v = mineField(spec, size, h, w);
        if ( v != -1)
          os << v << " ";
        else if (pos(h,w).val() == 1)
          os << "* ";
        else
          os << ". ";
      }
      os << std::endl;
    }
    os << std::endl;
  }

  /// Constructor for cloning \a s
  MineSweeper(bool share, MineSweeper& s) :
    Script(share,s), spec(s.spec), size(s.size) {
    b.update(*this, share, s.b);
  }
  /// Copy space during cloning
  virtual Space*
  copy(bool share) {
    return new MineSweeper(share,*this);
  }

};


/** \brief Main-function
 *  \relates MineSweeper
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("MineSweeper");
  opt.size(0);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Script::run<MineSweeper,DFS,SizeOptions>(opt);
  return 0;
}


namespace  {

  /** \name Minesweeper specifications
    *
    * A specification is a square matrix of characters. Alphanumeric characters represent
    * the number of mines adjacent to that field. Dots represent fields with an unknown number
    * of mines adjacent to it (or an actual mine).
    *
    * \relates MineSweeper
    */
  //@{

  /// The specifications
  const char* specs[] = {
      // 0
      "..2.3."
      "2....."
      "..24.3"
      "1.34.."
      ".....3"
      ".3.3..",
      // 1
      ".2.211.."
      "..4.2..2"
      "2..2..3."
      "2.22.3.3"
      "..1...4."
      "1...2..3"
      ".2.22.3."
      "1.1..1.1",
      // 2
      "1..2.2.2.."
      ".32...4..1"
      "...13...4."
      "3.1...3..."
      ".21.1..3.2"
      ".3.2..2.1."
      "2..32..2.."
      ".3...32..3"
      "..3.33...."
      ".2.2...22.",
      // 3
      "2...3.1."
      ".5.4...1"
      "..5..4.."
      "2...4.5."
      ".2.4...2"
      "..5..4.."
      "2...5.4."
      ".3.3...2",
      // 4
      "0.0.1..11."
      "1.2.2.22.."
      "......2..2"
      ".23.11...."
      "0......2.1"
      "...22.1..."
      ".....3.32."
      ".5.2...3.1"
      ".3.1..3..."
      ".2...12..0",
      // 5
      ".21.2.2..."
      ".4..3...53"
      "...4.44..3"
      "4.4..5.6.."
      "..45....54"
      "34....55.."
      "..4.4..5.5"
      "2..33.6..."
      "36...3..4."
      "...4.2.21.",
      // 6
      ".32..1.."
      "....1..3"
      "3..2...4"
      ".5...5.."
      "..6...5."
      "3...5..4"
      "2..5...."
      "..2..34.",
      // 7
      ".1.....3."
      "...343..."
      "244...443"
      "...4.4..."
      ".4.4.3.6."
      "...4.3..."
      "123...133"
      "...322..."
      ".2.....3.",
      // 8
      "......."
      ".23435."
      ".1...3."
      "...5..."
      ".1...3."
      ".12234."
      ".......",
      // 9
      "2...2...2"
      ".4.4.3.4."
      "..4...1.."
      ".4.3.3.4."
      "2.......2"
      ".5.4.5.4."
      "..3...3.."
      ".4.3.5.6."
      "2...1...2"
    };

  /// Number of specifications
  const unsigned int n_examples = sizeof(specs)/sizeof(char*);

  /// Compute the size of a specification
  int spec_size(const char *s) {
    int l = std::strlen(s);
    int res = static_cast<int>(std::sqrt(static_cast<float>(l)));
    return res;
  }

  /// Return value at position (\a i,\a j) in the example \a s of size \a n
  int mineField(const char *s, int n, int i, int j) {
    assert(spec_size(s) == n);
    assert(i >= 0 && i < n);
    assert(j >= 0 && j < n);
    char c = s[i*n + j];
    if (!std::isalnum(c))
      return -1;
    if (std::isdigit(c))
      return c - '0';
    if (std::islower(c))
      c = static_cast<char>(std::toupper(c));
    // std::alpha(c) == true
    int res = (c - 'A') + 10;
    if (res > n) return 0;
    else return res;
  }
  //@}
}

// STATISTICS: example-any
