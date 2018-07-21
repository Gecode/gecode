/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2005
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
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

#include <string>
#include <cmath>
#include <cctype>

using namespace Gecode;

#include <examples/sudoku-instances.hh>

/**
 * \brief %Example: Solving %Sudoku puzzles using integer constraints
 *
 * \ingroup Example
 */
class Sudoku : public Script {
protected:
  /// The size of the problem
  const int n;
  /// Values for the fields
  IntVarArray x;
public:

  /// Constructor
  Sudoku(const SizeOptions& opt)
    : Script(opt),
      n(example_size(examples[opt.size()])),
      x(*this, n*n*n*n, 1, n*n) {

    const int nn = n*n;
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.ipl());
      distinct(*this, m.col(i), opt.ipl());
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n) {
      for (int j=0; j<nn; j+=n) {
        distinct(*this, m.slice(i, i+n, j, j+n), opt.ipl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = sudokuField(examples[opt.size()], nn, i, j))
          rel(*this, m(i,j), IRT_EQ, v );

    branch(*this, x, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Sudoku(Sudoku& s) : Script(s), n(s.n) {
    x.update(*this, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(void) {
    return new Sudoku(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "  ";
    for (int i = 0; i<n*n*n*n; i++) {
      if (x[i].assigned()) {
        if (x[i].val()<10)
          os << x[i] << " ";
        else
          os << (char)(x[i].val()+'A'-10) << " ";
      }
      else
        os << ". ";
      if((i+1)%(n*n) == 0)
        os << std::endl << "  ";
    }
    os << std::endl;
  }

};

/** \brief Main-function
 *  \relates Sudoku
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Sudoku");
  opt.size(0);
  opt.ipl(IPL_DOM);
  opt.solutions(1);
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
  Script::run<Sudoku,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
