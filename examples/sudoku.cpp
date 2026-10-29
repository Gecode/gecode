/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2005
 *     Guido Tack, 2005
 *     Christian Schulte, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
#include <cstring>

using namespace Gecode;

#include <examples/sudoku-instances.hh>

/**
 * \brief %Example: Solving %Sudoku puzzles using integer constraints
 *
 * The instance is either the number of a built-in instance or a file in
 * the format of the scaled sudoku corpus: a line such as
 * <tt># size=6 box=3x2</tt>, then the rows of the grid with \c . for
 * blank cells and the symbols \c 1-9, \c A-Z, and \c 0 for the values 1
 * to 36.
 *
 * \ingroup Example
 */
class Sudoku : public Script {
protected:
  /// The side of the grid
  int nn;
  /// Values for the fields
  IntVarArray x;
public:

  /// Constructor
  Sudoku(const InstanceOptions& opt) : Script(opt) {
    SudokuInstance si(opt);
    nn = si.nn;
    x = IntVarArray(*this, nn*nn, 1, nn);
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.ipl());
      distinct(*this, m.col(i), opt.ipl());
    }

    // Constraints for boxes
    for (int i=0; i<nn; i+=si.bw) {
      for (int j=0; j<nn; j+=si.bh) {
        distinct(*this, m.slice(i, i+si.bw, j, j+si.bh), opt.ipl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = si.givens[j*nn+i])
          rel(*this, m(i,j), IRT_EQ, v );

    branch(*this, x, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN());
  }

  /// Constructor for cloning \a s
  Sudoku(Sudoku& s) : Script(s), nn(s.nn) {
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
    for (int i = 0; i<nn*nn; i++) {
      if (x[i].assigned()) {
        print_value(os, x[i].val());
        os << " ";
      }
      else
        os << ". ";
      if((i+1)%nn == 0)
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
  InstanceOptions opt("Sudoku", instance_spec);
  opt.instance("0");
  opt.ipl(IPL_DOM);
  opt.solutions(1);
  opt.parse(argc,argv);
  try {
    if (!SudokuInstance(opt).valid()) {
      std::cerr << "Error: instance must be a number between 0 and " << n_examples-1 << " or a file" << std::endl;
      return 1;
    }
  } catch (const Instance::Error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  Script::run<Sudoku,DFS,InstanceOptions>(opt);
  return 0;
}

// STATISTICS: example-any
