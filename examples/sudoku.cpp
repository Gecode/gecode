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

#ifdef GECODE_HAS_SET_VARS
#include <gecode/set.hh>
#endif

#include <string>
#include <cmath>
#include <cctype>

using namespace Gecode;

namespace {
  extern const char* examples[];
  extern const unsigned int n_examples;
  int example_size(const char *s);
  int sudokuField(const char *s, int n, int i, int j);
}

/// Base class for %Sudoku puzzles
class Sudoku : public Script {
protected:
  /// The size of the problem
  const int n;
public:
#ifdef GECODE_HAS_SET_VARS
  /// Model variants
  enum {
    MODEL_INT,   ///< Use integer constraints
    MODEL_SET,   ///< Use set constraints
    MODEL_MIXED  ///< Use both integer and set constraints
  };
#endif
  // Branching variants
  enum {
    BRANCH_NONE,        ///< Use lexicographic ordering
    BRANCH_SIZE,        ///< Use minimum size
    BRANCH_SIZE_DEGREE, ///< Use minimum size over degree
    BRANCH_SIZE_AFC,    ///< Use minimum size over afc
    BRANCH_AFC          ///< Use maximum afc
  };

  /// Constructor
  Sudoku(const SizeOptions& opt) : n(example_size(examples[opt.size()])) {}

  /// Constructor for cloning \a s
  Sudoku(bool share, Sudoku& s) : Script(share,s), n(s.n) {}

};

/**
 * \brief %Example: Solving %Sudoku puzzles using integer constraints
 *
 * \ingroup Example
 */
class SudokuInt : virtual public Sudoku {
protected:
  /// Values for the fields
  IntVarArray x;
public:
#ifdef GECODE_HAS_SET_VARS
  /// Propagation variants
  enum {
    PROP_NONE, ///< No additional constraints
    PROP_SAME, ///< Use "same" constraint with integer model
  };
#endif
  /// Constructor
  SudokuInt(const SizeOptions& opt)
    : Sudoku(opt), x(*this, n*n*n*n, 1, n*n) {
    const int nn = n*n;
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.icl());
      distinct(*this, m.col(i), opt.icl());
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n) {
      for (int j=0; j<nn; j+=n) {
        distinct(*this, m.slice(i, i+n, j, j+n), opt.icl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = sudokuField(examples[opt.size()], nn, i, j))
          rel(*this, m(i,j), IRT_EQ, v );

#ifdef GECODE_HAS_SET_VARS
    if (opt.propagation() == PROP_SAME) {
      // Implied constraints linking squares and rows
      for (int b=0; b<n; b++) {
        int b1c = 0;
        int b2c = 0;
        IntVarArgs bc1(nn-n);
        IntVarArgs bc2(nn-n);
        IntVarArgs br1(nn-n);
        IntVarArgs br2(nn-n);
        for (int i=0; i<n; i++)
          for (int j=0; j<n; j++) {
            b1c = 0; b2c = 0;
            for (int k=0; k<n; k++) {
              if (k != j) {
                IntVarArgs bc1s = block_col(m, b, i, k);
                IntVarArgs br1s = block_row(m, b, i, k);
                for (int count=0; count<n; count++) {
                  bc1[b1c] = bc1s[count];
                  br1[b1c] = br1s[count];
                  ++b1c;
                }
              }
              if (k != i) {
                IntVarArgs bc2s = block_col(m, b, k, j);
                IntVarArgs br2s = block_row(m, b, k, j);
                for (int count=0; count<n; count++) {
                  bc2[b2c] = bc2s[count];
                  br2[b2c] = br2s[count];
                  ++b2c;
                }
              }
            }
            same(*this, nn, bc1, bc2);
            same(*this, nn, br1, br2);
          }
      }
    }
#endif
    if (opt.branching() == BRANCH_NONE) {
      branch(*this, x, INT_VAR_NONE, INT_VAL_SPLIT_MIN);
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, x, INT_VAR_SIZE_MIN, INT_VAL_SPLIT_MIN);
    } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
      branch(*this, x, INT_VAR_SIZE_DEGREE_MIN, INT_VAL_SPLIT_MIN);
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, x, INT_VAR_SIZE_AFC_MIN, INT_VAL_SPLIT_MIN);
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, x, INT_VAR_AFC_MAX, INT_VAL_SPLIT_MIN);
    }
  }

  /// Constructor for cloning \a s
  SudokuInt(bool share, SudokuInt& s) : Sudoku(share, s) {
    x.update(*this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuInt(share,*this);
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

#ifdef GECODE_HAS_SET_VARS
private:
  /// Post the constraint that \a a and \a b take the same values
  void same(Space& home, int nn, IntVarArgs a, IntVarArgs b) {
    SetVar u(home, IntSet::empty, 1, nn);
    rel(home, SOT_DUNION, a, u);
    rel(home, SOT_DUNION, b, u);
  }

  /// Extract column \a bc from block starting at (\a i,\a j)
  IntVarArgs
  block_col(Matrix<IntVarArray> m, int bc, int i, int j) {
    return m.slice(bc*n+i, bc*n+i+1, j*n, (j+1)*n);
  }

  /// Extract row \a br from block starting at (\a i,\a j)
  IntVarArgs
  block_row(Matrix<IntVarArray> m, int br, int i, int j) {
    return m.slice(j*n, (j+1)*n, br*n+i, br*n+i+1);
  }
#endif
};

#ifdef GECODE_HAS_SET_VARS
/**
 * \brief %Example: Solving %Sudoku puzzles using set constraints
 *
 * \ingroup Example
 */
class SudokuSet : virtual public Sudoku {
protected:
  /// The fields occupied by a certain number
  SetVarArray y;
public:
  /// Constructor
  SudokuSet(const SizeOptions& opt)
    : Sudoku(opt),
      y(*this,n*n,IntSet::empty,1,n*n*n*n,n*n,n*n) {

    const int nn = n*n;

    Region r(*this);
    IntSet* row = r.alloc<IntSet>(nn);
    IntSet* col = r.alloc<IntSet>(nn);
    IntSet* block = r.alloc<IntSet>(nn);

    // Set up the row and column set constants
    int* dsc = r.alloc<int>(nn);
    for (int i=0; i<nn; i++) {
      row[i] = IntSet((i*nn)+1, (i+1)*nn);

      for (int j=0; j<nn; j++) {
        dsc[j] = (j*nn)+1+i;
      }
      col[i] = IntSet(dsc, nn);
    }

    // Set up the block set constants
    int* dsb_arr = r.alloc<int>(nn);
    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {

        for (int ii=0; ii<n; ii++) {
          for (int jj=0; jj<n; jj++) {
            dsb_arr[ii*n+jj] = j*nn*n+i*n+jj*nn+ii+1;
          }
        }
        block[i*n+j] = IntSet(dsb_arr, nn);
      }
    }

    IntSet full(1, nn*nn);
    // All x must be pairwise disjoint and partition the field indices
    rel(*this, SOT_DUNION, y, SetVar(*this, full, full));

    // The x must intersect in exactly one element with each
    // row, column, and block
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++) {
        SetVar inter_row(*this, IntSet::empty, full, 1, 1);
        rel(*this, y[i], SOT_INTER, row[j], SRT_EQ, inter_row);
        SetVar inter_col(*this, IntSet::empty, full, 1, 1);
        rel(*this, y[i], SOT_INTER, col[j], SRT_EQ, inter_col);
        SetVar inter_block(*this, IntSet::empty, full, 1, 1);
        rel(*this, y[i], SOT_INTER, block[j], SRT_EQ, inter_block);
      }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int idx = sudokuField(examples[opt.size()], nn, i, j))
          dom(*this, y[idx-1], SRT_SUP, (i+1)+(j*nn) );

    if (opt.branching() == BRANCH_NONE) {
      branch(*this, y, SET_VAR_NONE, SET_VAL_MIN_INC);
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, y, SET_VAR_SIZE_MIN, SET_VAL_MIN_INC);
    } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
      branch(*this, y, SET_VAR_SIZE_DEGREE_MIN, SET_VAL_MIN_INC);
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, y, SET_VAR_SIZE_AFC_MIN, SET_VAL_MIN_INC);
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, y, SET_VAR_AFC_MAX, SET_VAL_MIN_INC);
    }
  }

  /// Constructor for cloning \a s
  SudokuSet(bool share, SudokuSet& s) : Sudoku(share,s) {
    y.update(*this, share, s.y);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuSet(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << '\t';
    for (int i = 0; i<n*n*n*n; i++) {
      for (int j=0; j<n*n; j++) {
        if (y[j].contains(i+1)) {
          if (j+1<10)
            os << j+1 << " ";
          else
            os << (char)(j+1+'A'-10) << " ";
          break;
        }
      }
      if((i+1)%(n*n) == 0)
        os << std::endl << '\t';
    }
    os << std::endl;
  }
};


/**
 * \brief %Example: Solving %Sudoku puzzles using both set and integer
 * constraints
 *
 * \ingroup Example
 */
class SudokuMixed : public SudokuInt, public SudokuSet {
public:
  /// Constructor
  SudokuMixed(const SizeOptions& opt)
  : Sudoku(opt), SudokuInt(opt), SudokuSet(opt) {
    const int nn = n*n;

    IntSet is0(0,0);
    SetVar dummySet0(*this, is0, is0);
    IntVar dummyInt0(*this, 0, 0);
    SetVarArgs ys(nn+1);
    ys[0] = dummySet0;
    for (int i=0; i<nn; i++)
      ys[i+1] = y[i];
    IntVarArgs xs(nn*nn+1);
    xs[0] = dummyInt0;
    for (int i=0; i<nn*nn; i++)
      xs[i+1] = x[i];

    channel(*this, xs, ys);

    IntArgs values(nn);
    for (int i=nn; i--;)
      values[i] = i+1;
    count(*this, x, IntSet(nn,nn), values, ICL_DOM);
  }

  /// Constructor for cloning \a s
  SudokuMixed(bool share, SudokuMixed& s)
  : Sudoku(share, s), SudokuInt(share, s), SudokuSet(share, s) {}

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuMixed(share,*this);
  }

  /// Print solution
  virtual void print(std::ostream& os) const { SudokuInt::print(os); }

};

#endif

/** \brief Main-function
 *  \relates Sudoku
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Sudoku");
  opt.size(0);
  opt.icl(ICL_DOM);
  opt.solutions(1);
#ifdef GECODE_HAS_SET_VARS
  opt.model(Sudoku::MODEL_INT);
  opt.model(Sudoku::MODEL_INT, "int", "use integer constraints");
  opt.model(Sudoku::MODEL_SET, "set", "use set constraints");
  opt.model(Sudoku::MODEL_MIXED, "mixed",
            "use both integer and set constraints");
  opt.propagation(SudokuInt::PROP_NONE);
  opt.propagation(SudokuInt::PROP_NONE, "none", "no additional constraints");
  opt.propagation(SudokuInt::PROP_SAME, "same",
                  "additional \"same\" constraint for integer model");
#endif
  opt.branching(Sudoku::BRANCH_SIZE_AFC);
  opt.branching(Sudoku::BRANCH_NONE, "none", "none");
  opt.branching(Sudoku::BRANCH_SIZE, "size", "min size");
  opt.branching(Sudoku::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
  opt.branching(Sudoku::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
  opt.branching(Sudoku::BRANCH_AFC, "afc", "maximum afc");
  opt.parse(argc,argv);
  if (opt.size() >= n_examples) {
    std::cerr << "Error: size must be between 0 and "
              << n_examples-1 << std::endl;
    return 1;
  }
#ifdef GECODE_HAS_SET_VARS
  switch (opt.model()) {
  case Sudoku::MODEL_INT:
    Script::run<SudokuInt,DFS,SizeOptions>(opt);
    break;
  case Sudoku::MODEL_SET:
    Script::run<SudokuSet,DFS,SizeOptions>(opt);
    break;
  case Sudoku::MODEL_MIXED:
    Script::run<SudokuMixed,DFS,SizeOptions>(opt);
    break;
  }
#else
  Script::run<SudokuInt,DFS,SizeOptions>(opt);
#endif
  return 0;
}

namespace {

  /** \name %Sudoku specifications
   *
   * Each specification gives the initial positions that are filled in,
   * with blank squares represented as zeroes.
   *
   * \relates Sudoku
   */
  //@{

  /// The specifications
  const char* examples[] = {
    // 0
    "...2.5..."
    ".9....73."
    "..2..9.6."
    "2.....4.9"
    "....7...."
    "6.9.....1"
    ".8.4..1.."
    ".63....8."
    "...6.8..."
    ,
    // 1
    "3..9.4..1"
    "..2...4.."
    ".61...79."
    "6..247..5"
    "........."
    "2..836..4"
    ".46...23."
    "..9...6.."
    "5..3.9..8"
    ,
    // 2
    "....1...."
    "3.14..86."
    "9..5..2.."
    "7..16...."
    ".2.8.5.1."
    "....97..4"
    "..3..4..6"
    ".48..69.7"
    "....8...."
    ,
    // 3
    // Fiendish puzzle April 21 2005 Times London
    "..4..3.7."
    ".8..7...."
    ".7...82.5"
    "4.....31."
    "9.......8"
    ".15.....4"
    "1.69...3."
    "....2..6."
    ".2.4..5.."
    ,
    // 4
    // This one requires search
    ".43.8.25."
    "6........"
    ".....1.94"
    "9....4.7."
    "...6.8..."
    ".1.2....3"
    "82.5....."
    "........5"
    ".34.9.71."
    ,
    // 5
    // Hard one from http://www.cs.mu.oz.au/671/proj3/node5.html
    ".....3.6."
    ".......1."
    ".975...8."

    "....9.2.."
    "..8.7.4.."
    "..3.6...."

    ".1...289."
    ".4......."
    ".5.1....."
    , // Puzzle 1 from http://www.sudoku.org.uk/bifurcation.htm
    // 6
    "1..9.7..3"
    ".8.....7."
    "..9...6.."
    "..72.94.."
    "41.....95"
    "..85.43.."
    "..3...7.."
    ".5.....4."
    "2..8.6..9"
    , // Puzzle 2 from http://www.sudoku.org.uk/bifurcation.htm
    // 7
    "...3.2..."
    ".5.798.3."
    "..7...8.."
    "..86.73.."
    ".7.....6."
    "..35.41.."
    "..5...6.."
    ".2.419.5."
    "...8.6..."
    , // Puzzle 3 from http://www.sudoku.org.uk/bifurcation.htm
    // 8
    "...8....6"
    "..162.43."
    "4...71..2"
    "..72...8."
    "....1...."
    ".1...62.."
    "1..73...4"
    ".26.481.."
    "3....5..."
    , // Puzzle 4 from http://www.sudoku.org.uk/bifurcation.htm
    // 9
    "3.5..4.7."
    ".7......1"
    ".4.9...3."
    "4...51..6"
    ".9.....4."
    "2..84...7"
    ".2...7.6."
    "8......9."
    ".6.4..2.8"
    , // Puzzle 5 from http://www.sudoku.org.uk/bifurcation.htm
    // 10
    "...7..3.."
    ".6....57."
    ".738..41."
    "..928...."
    "5.......9"
    "....936.."
    ".98..715."
    ".54....6."
    "..1..9..."
    , // Puzzle 6 from http://www.sudoku.org.uk/bifurcation.htm
    // 11
    "...6....4"
    ".3..9..2."
    ".6.8..7.."
    "..5.6...1"
    "67.3.1.58"
    "9...5.4.."
    "..6..3.9."
    ".1..8..6."
    "2....6..."
    , // Puzzle 7 from http://www.sudoku.org.uk/bifurcation.htm
    // 12
    "8....1.4."
    "2.6.9..1."
    "..9..6.8."
    "124.....9"
    "........."
    "9.....824"
    ".5.4..1.."
    ".8..7.2.5"
    ".9.5....7"
    , // Puzzle 8 from http://www.sudoku.org.uk/bifurcation.htm
    // 13
    "652.48..7"
    ".7.2.54.."
    "........."
    ".641...7."
    "....8...."
    ".8...456."
    "........."
    "..86.7.2."
    "2..89.751"
    , // Puzzle 9 from http://www.sudoku.org.uk/bifurcation.htm
    // 14
    "..6..2..9"
    "1..5...2."
    ".473.6..1"
    ".....8.4."
    ".3.....7."
    ".1.6....."
    "4..8.321."
    ".6...1..4"
    "3..4..9.."
    , // Puzzle 10 from http://www.sudoku.org.uk/bifurcation.htm
    // 15
    "..4.5.9.."
    "....7...6"
    "37......2"
    "..95...8."
    "..12.43.."
    ".6...92.."
    "2......93"
    "1...4...."
    "..6.2.7.."
    , // Puzzle 11 from http://www.sudoku.org.uk/bifurcation.htm
    // 16
    "....3.79."
    "3.......5"
    "...4.73.6"
    ".53.94.7."
    "....7...."
    ".1.82.64."
    "7.19.8..."
    "8.......1"
    ".94.1...."
    , // From http://www.sudoku.org.uk/discus/messages/29/51.html?1131034031
    // 17
    "2581.4.37"
    "936827514"
    "47153.28."
    "7152.3.4."
    "849675321"
    "36241..75"
    "1249..753"
    "593742168"
    "687351492"
    ,
    // 18
    // The following minimal 25*25 Sudokus are from Alain Frisch
    // Sudoku website http://www.eleves.ens.fr/home/frisch/sudoku.html
    "...G...9..4.....6F..L8..."
    "CEIN.HDM.OF.1L..A..9PJ.4."
    ".....A...L..JBN.2.D.1...H"
    "P49...JB23.AD..7E..C5F..."
    "A1H....F.N5....I.BL...26."
    "....7..C.6...H4B..1....I5"
    ".F.P...I..B..7.5.L...9..."
    ".L6A...5OF.8P...K.NE..734"
    "B2.E..L...1J.5....O7.K.AP"
    "O.5.CB1.P....3EM....2L.H."
    "2..MJ.A...9.3.7......P.8C"
    ".....CF.DPG62N.E...OH.M.J"
    ".DL..OM..IE.B8..NH...3..K"
    ".CO1F.B.N.AH..P.78.JE...D"
    "E..6.....H......4M.KIB9.."
    "N.J..6......C..1.5.G..H.."
    "...75LG...6..1..CI..4.E.."
    "9K..6.....HGN.O2P.4......"
    ".OA..IP849...2.K3...7GN.."
    "..G....N...P.D9....A...C1"
    "J...M.NAFE.4..23.7....8L."
    ".....J.H9CD1LP..GO....4ME"
    "48NK..5.M......JL.......9"
    ".I.OG....835.A.DH..P....."
    "35......L.J..E....8IG.67B"
    ,
    // 19
    ".N..JG..O7591...8I....L.."
    "FG.M.B8...P.E...CJ..H...."
    "...........G.4.H.D.O.NJA2"
    ".....J.EN4.L6MA.B.2......"
    "HE..2..DC.....F4KMA.B.9O8"
    "M....62...47C19......E5.."
    ".I2.8M.JGL...ADN..K..3.F7"
    "..H3.5..89....I.J.....NL."
    "1B..9.FAP.6.N....537.H..O"
    "......1..N...O...LC.68.PG"
    "KOA.FNBH.....7.C.....M..6"
    "45.ECP.I..N.F.J1...MK.79."
    "I.L..8.O..9.P...A...2.1J."
    "..621.D.M.....B8LG..P..CH"
    ".HP.N7E.L1....3..B..O.G45"
    "....BIO....5.C.P...FN48E."
    "...FL.....2.DH..17..59O.."
    "..I.MF..2G.N...A6O.HC.PB."
    "72.1..L...IM.96E.45G....."
    "..9...7M..A.O...I...L...."
    ".C.JA.........1.....E.48."
    "O.BI.......PHL.6..1....5C"
    "G6M...N4FI8...K..H.E....."
    ".L..4.917....BE.G8F.M.I.."
    "8F......5.O3..4...9.....K"
    ,
    // 20
    "5.PC..7..J..I...3..H.M.2L"
    "H.......F..DA..N.G...9..P"
    "...3LCP2.54.71.B....J.8.."
    "76M8...3A...H..C.D.FO...."
    "...DK..GI..B.L.6.8.14...."
    "A......M....D.6.N.P....O2"
    "...E5BLF..92..3AJC..6I..."
    ".PNJ.6..E7A.8..IM.OL1.G.C"
    ".L3...O.N.5.KI.46.....9E."
    ".I.G.A..28.MBP...E..HJ3.7"
    "J.74.L..D1O96.A3.M...GI.."
    "E...1...K....J...P6.7.C.9"
    "8M..A9JO.F.P..1...4.E3N6."
    "...I.3.7.......EL.CD..H.."
    "....DE2..P...N.........K."
    ".O.7.FKI1..GJ.N.....9.P8."
    ".89..H..BNM73D.KFJ..I.6.A"
    "PDBN...9M..C.....O.6..7.."
    ".F.K...4..LA9B..C.E75..GN"
    "G.A.....7.8.....H....O.3."
    "B...C...4......8K.3.P...."
    "DHE5..FA...J.3..B.2.KC.98"
    "...F...5....N..J9......I."
    ".J...P..O.BK....I.M.3..5."
    "9...8.B.6.D.M.I..H.5G.J4."
    ,
    // 21
    "..6F.....5..3.....H..A.M2"
    ".....3....I8.A.MC.KJ.LNG."
    ".I.7N..K.2..6...3D..B.O85"
    ".3A.PF.D8OBK7.2....L6.9H4"
    ".K..CBML.....O.A8.G4.D..."
    "1.4.AGL..M5..F.O.9....P2."
    "..I.........8......N4E..."
    "J.C8.1.6.3.LO.K7AG2P9.H.."
    ".2.3BH..9.A...G......O..L"
    ".H..M8.J.....NI1.LEF....B"
    "I5..4.....G.27..K..3.M..H"
    "P..E..I.A..3B.8....G.2..."
    "AJ......NFK.I.O9476..G.1."
    ".976...E3H....M5.F......O"
    ".F.M3.5.GKC4.HJ.N.....ID7"
    "..L1K..9.J37.ID..B..E6..."
    ".82OH.1..PNML...E..C...J."
    "....JLFN.B..G..6M..H..D.9"
    "...C.A...I.6.......K.5..."
    "E.G.I.....O....J.8F......"
    "..M4..9.D.7.K.FE.3O......"
    "H.N.....14E..B3L..8I...AG"
    "K.O.62.PM...N..H...18C.9."
    "LC..8.3...2...H.G...J..4E"
    ".B..9NK.E....C6.P.4D.71OI"
    ,
    // 22
    "A..F.N......M28DC.LI7..OJ"
    "..B...D.M.6....9.....2A.."
    ".2..I.56.B...J.MEH......."
    "O7...HE...BA..G2........L"
    ".H6J.2...G.7NDP.A...8..C."
    ".PN35..B..869.2.GA..K.C.."
    "....EM13OD.N...49K...7..."
    "9G..C....IJF5.B..7.3....."
    "HD.....JN....7...EF....9."
    "1.OA..G.KL....H..B.CP...."
    "..CE.......2..9I....3...."
    "FJ..83P.E..K7.NL15H..I2.."
    ".4..GJ...6DIB...P...AHL.C"
    ".1I.2.M....83.F..4.NBE..."
    "L3M.OD.H.AG..4.....69...F"
    ".8...5H.3.....M...D..K..4"
    "3..4..AEDO7J...5..9..G1.."
    "..2N9.8F.P.OIGC.L6..E.H.."
    "C...1.7..K.L6.4EO.8.5...N"
    ".IG.H..M..E...1A2N4..8.F."
    "6..5J..N1...2..H.IGA...P8"
    ".L...O...H...C....M5G..A."
    "..F...3C.7.P..5N..B..DMH9"
    "291D..6..M..H.7.3.J.N..B."
    "....MK..29F.G.DO4...6.E35"
    ,
    // 23
    "....B12O3..D..F..KPL.E4.7"
    "1M..GL..H..K.A.....39.P.."
    ".8.3.4.....7..6.F.....CK."
    "P.O...758.2..MC.....1L.A."
    "...HFK6.A..8...9B...2...J"
    "91.KJE..L.5O..GD.....4..."
    ".I..3..D.2.....C4MLAK..N."
    ".4..6IA.P7.....B9......3."
    "M.F...4.J...8....N.H.1G7."
    "..5P.N.....C.731.I.E.9A.."
    ".....GO.KDL......BA......"
    ".3A..F...9..K.EI5.7..6.DN"
    ".G.54.L.....PH..3F6....2."
    "....P.....G..2D.OH.1B...C"
    "6.EM.7N....3.B4..DC..K1P."
    "B.9.....I..5N...7OGK...46"
    "OF.GD6HP..JM..BA8.I.C...."
    "8.L7...........P.3M5...92"
    ".EM.....45IF7....2.CJ8DL."
    ".6.4.8..NA.2......9....GI"
    "2..A1DCN..3G.F5.L..I..O6H"
    "..BIO..1.H.L...G........."
    "C.P.....M8.9O..5A..N.JK.D"
    "H...N...FOA4..73....5..CM"
    ".D4..3...6BE..N.2JH8..L.."
    ,
    // 24
    "LJ.FH2...K....359.E.B6.N."
    "...EML1FC.PJ.A.8I....2.K4"
    "9........G1..6..K...PAL3C"
    "P...C34..8.N7.........DE."
    "...7....E..IO...124.J5.F."
    ".8.L.PA.2..31FG....N..C.I"
    "F.G......M........2.K..1."
    "....2KH6.JOD..9.L..G...B."
    ".6O3..G...4.NJ.H.PB...5.9"
    "I...K..LBN...E.76.A....8."
    "D.B.L......8.3.C.KM.6...."
    ".C593.I.N4.26MB..1..L.K.."
    "M.F6......DA45.9..N.I3..B"
    ".G..79.H...KJ...4...AN.2."
    ".KH...BC....LON..7..D.8.F"
    "2..B...MP....7OEJ4....6.1"
    ".4....2O.9......N..1....G"
    ".....G.JF.2..L.6.5......."
    "C9A.G...H1...PJ..L.3.8.MN"
    "...1......M.....O.P8.K3J."
    "...N.HP.K25G3..JC8..1M..."
    "..2K.FC34.A....N7..9.LB.."
    "..65DJ..L..C.4.1...BGF..."
    "OF..E..7.B.......634...D."
    "G.3..N.I9D7PM8KF2....4..."
    ,
    // 25
    ".1..I8.P..6.N...BD...3O.."
    ".9.6...E.M43.8..AK.2J.1.."
    "...JKL...FA...P.I4...DB.."
    "E...P.I.B.7.1..J.HC...98."
    "..5M8GJ.KD.O...N3..1AI..."
    "K.2C4.M.N..J..I.......H.5"
    "N..B9O.D...KH6.E...C...7I"
    "DE..JK4.....L.1B7..68PN2."
    ".....P...C..F.7...L.O9..."
    "...35.H......92..M.4.EC.1"
    "P..IL..H.........1..3.D.."
    "....B9..8.3I5.C...K...F1."
    "FH....2O..D.4M.P...A...GC"
    "JA.......KF.......4.EOMIP"
    "3.7GNF...A.2OB9C.E5.HJ..."
    "2.I.1.......AO5.P...K.3.."
    "..H....L..M.CIJ..7...4..."
    "GO.9..KF.I.P3...E.HJ...N7"
    "5.37...B.E...4N..O.8....."
    "4B...7.9O.HL..E2C3.K51.M."
    "L.O42...D..AJ.8...GH.N..E"
    "9M.8.H..LG1..N..5.E..F7B."
    "......FAC..5M..I6J.B4...G"
    "...F..82..P.E......36HK.L"
    "B.J.G5...O.H2..98.7......"
    ,
    // 26
    ".CNPHK.53.O9F..D.78..J..."
    "J.....F.DB...7.G..PAE...L"
    ".6.........5.4A......I..."
    "A..L.6.E.1JG....5.H.2...9"
    "..G4..P...E2N.M...C..FB.1"
    "..PKJ...G41O..C.N..H....."
    ".M..I5L97J.3H...E2.8...D."
    "1.4..ON.83G.P.D59..C.BH.."
    "..2.9.......4.F.3.6..O.7."
    "3F.7.MEC..5...2..4K.LN8.."
    "...I........MF7.6AOG..LE."
    "C1.3..JG..D.9...4..N...68"
    "...M7L9..NHA...FJ.I..3.C."
    ".A...P...5...E3.8M..K4OFG"
    ".....C.6KIP...8..3.DJ...."
    ".2.J...3C..7.D.9A.EF6L..."
    "4...3.6N.....L9.H..P8...2"
    ".9.C.4H.......P....1..FJ3"
    ".LD.K87.1.BM5AJN....4H.G."
    ".B..MAI........47O.LD...."
    ".G....3....F..1..9..M..K6"
    "P7.A..B8.......2....I.3.."
    "M..O....9K2..6..1.N.FE.L."
    "....E...A..N.J.IG8...C.9."
    "6KL.4...FCI.A.....5J.2D.N"
    ,
    // 27
    "E..I...M......L....D...BK"
    "F..BH9.K.A2.7..E4.P.6..M."
    ".6.J..PD8FE.IM...K...54.."
    "L...8E...IA..HC........7J"
    ".....7H.4.JK..DOFC....9I."
    "9...7A.5.B.M34E.KDJ8....."
    "8...BD....O..7....CP.E.6."
    ".1.3..8....D...2.ML.B..5A"
    ".E2.A.O.7..1..I.659..8LD."
    "KF..M2...PL......A.G..N3."
    "...76N.1..CBG...DPK..O.J2"
    ".JK...6B.9..P.7.N.EMFDG.5"
    "N....G.F.8....OH9.2...E.7"
    "....4.3....F9.....5.NC.A."
    "M.A.GL.J......5..4.7.91.."
    "C.8NE.5..6..M.....BJ17..H"
    "7.6...NL.41.AC.I8...GJ3.."
    "...53P.GM2.L..F.....K.68."
    "J.4.D..H..IG..8K..35.N.FL"
    "P.F...93.D.....A.1......M"
    "..7.I...1...DF..PJ.4.MA.."
    ".KN.....5H..O.63.E.2....4"
    "....1...O..A2..DCH8B..KE."
    "3.HA.6BP.....J1957O..28.."
    "42JO.I...K5C......6.P.B.."
    ,
    // 28
    "...G87..O..F.N..CH6..D..2"
    "C16..N....DL.3.E........."
    ".LN.EK..D..O.G6.412....5H"
    "K...2........59M...P..3.."
    "..A9.M.6....8.E7O3..K..LB"
    "7..8B.1.EP.....4.L.6.C.9."
    ".3.6...9.85.A2F.....B.EP."
    ".D4K.L..NA....C.M.E..7..."
    "L..P.3H.CG.7....DKF..I6.."
    "5EH.G.7.6.1....J....D3K.O"
    ".6.....G.K.E..I2.4J......"
    "..ICFP.8H7.2.O.B.NM5..G.."
    ".M..D9..BE..J..F..I7..LAK"
    ".BE.L...31.M7FK..C9..8.DN"
    ".2O......D38C...E...F.P.."
    ".A.....M..NB..3.J.7E8..2."
    "...O...B..6..C.8KG4..5D7M"
    "....M8..I..9..AL1.O.3HBNG"
    ".C..4....L.....D.F......."
    "J..5NF..G...H..6..C..1..."
    "D..N..O....G9.J.A..I...8."
    ".F.H1BN.K.O.4.8.6.3.9.M.."
    "B...I....9K..6..2...G.H1."
    "O...7.CJMI25N...HDKBPFAE."
    ".....28.F..C........O.JK."
    ,
    // 29
    "9.K..6D.I5...........H..."
    ".I.E..BK...GN...6.L...3.."
    "7....2..L8E.K.D1.P5I.6..A"
    "..N.LEH.A32..CM9........."
    "..2....9N...3.IC...J.KF8."
    "......GA..C.7JPNI3....6L."
    "....E.J.8K..I.....97NG2.B"
    "O7..3HI...M.....DCF5..914"
    "L.M.4.3.19.D..2......F.K."
    "B....F...O9.6ANG.2.PH.E.5"
    "8KDM...5.1....G......P..6"
    ".4.H.L.C..J2....G..3ION.."
    "C.A..J..E...1K......92.5."
    ".9O.NP.2...IA....H..G..7."
    "...I5..O.N4.H...2DCKJ...E"
    "56....1.D..AJ.....7.L.O.."
    "K.8.H.7.9.5....AC..O..G6F"
    "3..........O..C.F.P6K.5.."
    "...OC.4J2.3E.9...N.H7..P1"
    ".B.7.K.....6.MH.LJ..A...."
    "I..4.........P..E7D9O.B.H"
    "E.3G.OP...I1CBL..FN4..82."
    ".A..9N.8.E...73O..H....4."
    ".M.C.3....DK..EI......JG."
    ".HP..D..FB...NO.1KJ8.AL.."
    ,
    // 30
    "....L.1.8H.CO..P....FA..."
    "P1.2.4CO...K..A..3H8...7."
    "I..H.G.5.B.6.M..A24.K..9."
    "....6I.KF9..5.P.J1.BDCE.M"
    ".A....2...F..L..E.M.6.5O3"
    "7..1.C...8.L9..4P.5.G.K.."
    "JO.5H.M2.I.7.F.6D..A4..NE"
    "E.3.NO....K2...LG..7JM..."
    ".9F.CJ.H....A.NKB.1..2.5."
    "2LB...5..7.P36H.M..N...1D"
    ".....P..B....46.HJ......."
    "B4..7LI.C.GD2...KA.3H...."
    "K5......6J1.........27O8P"
    "LP...5K.G..JB3.......1.M."
    "..EG...4.F.NKC...O..9.B.."
    "..MI...D32AGP..5O...74.BN"
    "FC74.....1..LO..8...5.D.J"
    "..JN....H...E.I...DPA3.6."
    "AE.......O3F..5.........H"
    "...8..6......24.9G....I.1"
    ".M....8.9..O1.F.C.EL3...A"
    ".N...E.3.....I7M1...ODG.K"
    "1......P.N9..D..5........"
    ".I.P..FGOCMA.......J.5..2"
    ".GK....7...3.....92..B.L4"
    ,
    // 31
    "...H....GKM.43..B.D......"
    "15I.C....8B.6D.7G....A.H4"
    ".7...F..B...J.E16.N....3."
    "...D.1...6L7.H.5K....P98F"
    "KJ.AF.5CHO...NP2M.....B.."
    "4..K..I9.M.DNP..A.3..E5O6"
    "PGC...HE.....I5..M...7..J"
    ".B.EJ..G.5..L....D.48.2.."
    ".D....N.4.J.2A..H..5..F1."
    "8....AJLD.7..OM...1B....P"
    "CH...N..7.........B3....."
    "I..O.JL..P5...3......KH.8"
    "D.259.E......6.......L3.."
    "...3K6.B.AE..74N25H..G1.."
    ".L.B..41.HC...G.OIP8..MN7"
    ".........2.C.4.L....69.I5"
    "N...7.C...69.5.A......PKL"
    "....L.G...13.BOMP.82..D7H"
    "...C.H..5.P.E.K...I7....."
    "JO..P.96ABI8.....C..N..4G"
    ".NA8H.32..O4..IB..L9G..J."
    "..4.GD..M.3..27INJ.O....."
    ".C9L..A..J.....6.2..I..F."
    "BK...4..F.A.CL..87.1.O.M."
    "6..P.I.5...M..N....F....D"
    ,
    // 32
    ".6OL.3GB.I.1F.EP..9..24.."
    "..1.P.....K....N.2.....ID"
    "2.8.K7..J...6..1F.......A"
    "...D.8.4..29..LJ7.HMKPF.."
    "..I.9N...A..P.M.DK...E5.."
    "94.N...5..F..E.C182..BG3."
    "8D..3HC....6G.....P.F.72I"
    "C..I.E.7D....8BG.F...6..."
    "......L....N.24..I..1K.DJ"
    ".27HG.NF......I..L..E.A.."
    "L...N.I8F6.H.9K.3O..B..5P"
    ".9...L..25.B14F.N.JE...7K"
    ".1...9B....GJ.8.A.5C....."
    "....CPK..J....5.M.7.6.H.3"
    "..F7..3..O..N..B8....421."
    "AM.8....P...71..26N...3G."
    "P....D.2.78.O..5CJ.GA.BH."
    "6..4.F...3...K9A..1...J.M"
    ".I2.E......JM.N...B98..P."
    "FC..7..6.EI.D.G...3H5..KN"
    "I8NO.4F.9..DB....5......."
    "5..1...G3.MIH.A.9..N....."
    "KG.62..1.....L3.....IJE.F"
    "....D.6.AC4.9.....OL.HP.."
    "..EA.5O.L.6.......D...9N."
    ,
    // 33
    ".CH3.L..D4F..I.........1."
    "G..K.....7.O.......L.42N."
    "A.D...5O8B...39G.4..I...L"
    "1E2OB....A8N..6FCD9.H...G"
    "IN..4..3.....J..5OM....9."
    "...JEMP..9G8..D7......BA."
    "B.M9..L....6...2.FI.KJ..."
    ".LN.386A..BH....O.J.2..7."
    "7A6..2..3N.....4KP..G8HIC"
    ".1.....J.CK3....HB..5L4.."
    ".H......P.I.57....G..21B4"
    "....7.....EP1....3.2..A.."
    ".59.1FI.L8.MJG.B..AH....E"
    "NI..D....O3K..A.P9..8F..."
    ".O....A..E2.8.HN.J7.P...."
    ".MC..P....H2...A7..ID...1"
    ".6.....M.J.14.BDG.3.O9F2."
    ".......CG.........4...M.."
    "HDL...8N7.......FK2C.E..B"
    "..KP2I.F.3...N7J.E.6..L85"
    "D.E...C.4.7B..I....A.1..M"
    "J..M..7..D.A..4...ONBG..2"
    "9.7.6..K..P..O....D...8.."
    ".2IN..GE1.D..8...MK.CPJH."
    "FG..8B.2.L..KH.....1.5E..",

    // 55 16x16 instances (mostly harmless) from janko.at

    // http://janko.at/Raetsel/Sudoku/301.a.htm
    // 34
    "D92.....G...43.."
    "4CF.....9D.2.6EB"
    ".E.1....F.8BC..A"
    "G56.....A3C....1"
    ".7G5A8....61...."
    "2...C....B7....."
    "..AE.964..G....."
    ".F9.5.7.4......."
    ".....29....A.C.."
    "........645D.1.."
    "....D....1.C.B7F"
    ".....E.C2G...8A9"
    "B..9.G52.....EF6"
    ".256..F.....D.B."
    "E13.6.D........7"
    "A...8BC3....954."
    ,

    // http://janko.at/Raetsel/Sudoku/302.a.htm
    // 35
    ".DG1.C.BE..3.4.A"
    "..7B.62..41...59"
    ".......D.....G.."
    "..49.7.3.B6..FD."
    ".9..G...C......4"
    "G..46....9F.3.B."
    ".C5.1...BE.86..G"
    ".B.....E2G...D.."
    "..3...596.....1."
    "F..C2.76...B.E3."
    ".1.8.4D....7F..5"
    "E......F...D..9."
    ".AB..FG.1.3.C8.."
    "..2.....F......."
    "8F...BC..62.97.."
    "1.6.A..59.C.GB2."
    ,

    // http://janko.at/Raetsel/Sudoku/303.a.htm
    // 36
    ".....D.3..7F.A.."
    "...B1.F8....26.."
    ".F.3...6D..AC..."
    "AGC.9.5..8....BD"
    "E.FG5...7...A.9."
    "2.7.....89A36.F5"
    "...1.9...CBE...."
    ".3....A.....BG2."
    ".1B2.....7....6."
    "....B16...3.9..."
    "5D.4F3EA.....2.B"
    ".E.A...9...D83.C"
    "4A....B..E.8.FC9"
    "...EA..G1...D.4."
    "..GC....FD.B1..."
    "..D.47..6.C....."
    ,

    // http://janko.at/Raetsel/Sudoku/304.a.htm
    // 37
    "....9...5...3B.."
    "...D13.7.4.....F"
    "637...2..81AC9.."
    ".2G..5.......18D"
    "...F4...3...8C.."
    "E.D.7..6..G...A5"
    "C5.6..3....F.2.."
    "4.A...1D72.9..B."
    ".E..D.9CA6...F.1"
    "..9.5....E..D.26"
    "B6...4..D..5.7.A"
    "..F4...A...C9..."
    "AB4.......2..G6."
    "..68FBD..5...A47"
    "1.....6.9.E4B..."
    "..32...5...B...."
    ,

    // http://janko.at/Raetsel/Sudoku/305.a.htm
    // 38
    "..16.E.8.BF.4..."
    ".5...9D......A73"
    "..3...B.78.D.6.."
    "......1...9...BE"
    "C..1D..6B5..7.A4"
    "...5.F.98..32.DG"
    "3.....C..D.A5.E."
    "....3B5.F7...9.."
    "..5...6C.2AE...."
    ".3.BE.2..4.....9"
    "F9.2A..B5.7.G..."
    "EA.G..756..BD..1"
    "6C...8...9......"
    "..9.6.47.E...D.."
    "2GE......C6...F."
    "...3.CG.2.D.65.."
    ,

    // http://janko.at/Raetsel/Sudoku/306.a.htm
    // 39
    ".....D5..7.169.."
    "..4..2BE8.G..A.."
    "8DA....752..BGF."
    "G.9EA.86..3F2..."
    "C..4G1.....7F..."
    ".G.A...315.6..C8"
    "E.5..F7.4G..1.A2"
    ".91..BE...D.54G."
    ".68D.3...C5..B9."
    "4E.5..9B.31..F.G"
    "3B..E.G1A...C.4."
    "...95.....F83..7"
    "...C9G..F1.5D8.B"
    ".468..DFC....3E5"
    "..2..E.A7DB..C.."
    "..EFB.3..86....."
    ,

    // http://janko.at/Raetsel/Sudoku/307.a.htm
    // 40
    ".BC..32...9..D.."
    ".3..CB...52A.9E4"
    "7E.A...D8.6B2..5"
    "..9F..A.DC7.B6.."
    ".15..2.E..3...4."
    "4GD81.3C...7..6F"
    ".C.9....E.4.G..1"
    "..E3..59G.FD.B.."
    "..A.2E.FC9..84.."
    "B..E.9.4....F.A."
    "16..A...57.F3E9B"
    ".9...6..4.E..7G."
    "..64.C85.2..DA.."
    "E..DB1.23...6.59"
    "C5G.9D4...1E..2."
    "..2..F...DA..CB."
    ,

    // http://janko.at/Raetsel/Sudoku/308.a.htm
    // 41
    "G.E37..1.....6.."
    "96..E..3..G5D.F."
    ".7..64.CF31..29E"
    "....F...8.9E437."
    "6AF...D.3..1...."
    "..1...B5.8F47..3"
    ".83B2.47.G....61"
    "..79.6...EC..8.G"
    "E.C..2A...8.FG.."
    "25....C.GA.78B4."
    "7..AD3F.24...E.."
    "....G..B.1...C52"
    ".4A2B5.D...8...."
    "FE8..G2A1.73..C."
    ".C.78F..4..2..E5"
    "..9.....E..G34.8"
    ,

    // http://janko.at/Raetsel/Sudoku/309.a.htm
    // 42
    ".G.4..1E6.9...2."
    ".D..4G.C.....9F7"
    "..79.D...5C.B.G."
    ".EFC7...G..D.5.3"
    "5...C...E6BFD..."
    "...1...5..D.CB.2"
    "7.CG29.D3...E8.F"
    "94...EGB.2.C...."
    "....E.2.583...CD"
    "3.D5...89.FB7G.E"
    "4.1E.F..A...3..."
    "...F1B3G...E...9"
    "F.9.8..1...G23D."
    ".A.B.4D...7.5F.."
    "8CE.....1.25..7."
    ".7...5.3F9..G.8."
    ,

    // http://janko.at/Raetsel/Sudoku/310.a.htm
    // 43
    "...4.1.9..7...B5"
    "6E..2.8....CGA.."
    ".51....B.D....6."
    "B.9..E..G..A...7"
    "..7.5F9G..48...."
    "..29..3..F..5.7."
    "G.BD...837.E..94"
    "......7E.1.6A2G3"
    "94GA7.2.63......"
    "5B..A.E18...79.2"
    ".1.8..6..4..BD.."
    "....G8..EB92.4.."
    "D...9..A..1..G.6"
    ".3....4.C....1E."
    "..4B6....E.7..2A"
    "F9...5..A.3.4..."
    ,

    // http://janko.at/Raetsel/Sudoku/361.a.htm
    // 44
    ".G..5.2.E.FA.4C."
    "AB..G....35.17D9"
    "53..4AC..DB....."
    ".6.C3B..2.8.5..."
    "1...A....E.D9CG3"
    "GDA9.4..B.1.6E.."
    ".FB.1..E9....D.8"
    "C..3.59G48......"
    "......F38G2.7..4"
    "6.3....A7..9.8E."
    "..CE.9.1..4.DG35"
    "89GD2.4....6...C"
    "...G.C.4..958.7."
    ".....1A..F78..42"
    "4871.3G....2..9A"
    ".C9.72.8.6.4..1."
    ,

    // http://janko.at/Raetsel/Sudoku/362.a.htm
    // 45
    "A...18..79..C..6"
    ".37.A2...FD...E9"
    "G14D..5...8C.A.."
    ".268.E7.3A....D5"
    "3...D1...7A2.8.4"
    "7CF.9..4....D.2."
    "5..63.A28...1CF."
    "...1.6...3FD..5."
    ".A..FC6...9.3..."
    ".D2F...3A5.1E..8"
    ".9.G....D..7.F1C"
    "E.C.G9D...3F...7"
    "17....9B.2E.43C."
    "..9.E3...C..5D7F"
    "FE...AC...G5.29."
    "D..C..2F..73...E"
    ,

    // http://janko.at/Raetsel/Sudoku/363.a.htm
    // 46
    "3.8.BD..5F72E..6"
    "..G..4.7E..6F5.."
    ".AF.2..C....9G73"
    "59C...FE.AG....."
    "C5..1..F.4.G.E.7"
    "F..2.C..B13...GD"
    "4..B73.D....C..."
    "GD...A...C.7B48."
    ".G5FD.3...9...BE"
    "...C....6.E1G..9"
    "28...7E1..B.6..4"
    "1.B.5.C.3..8..AF"
    ".....ED.16...345"
    "9C21....7..A.8E."
    "..EA9..3F.8..7.."
    "6..5827A..4D.F.C"
    ,

    // http://janko.at/Raetsel/Sudoku/364.a.htm
    // 47
    "1.....6C.4G.9BA."
    ".G6..F.9A.D1...2"
    "...D7G.3F...41.."
    ".3A.2..1.759.E.G"
    "B8..D.F.C.2.A..."
    "C....1..DBFA2..5"
    "..FG.E.....58C9."
    ".25A3.C.G..E...1"
    "3...4..B.G.DE78."
    ".C78A.....3.12.."
    "5..1FC37..E....9"
    "...4.2.8.F.B..CA"
    "A.4.6B7.5..F.9D."
    "..8C...DB.975..."
    "9...85.E3.A..46."
    ".5D7.A9.E2.....8"
    ,

    // http://janko.at/Raetsel/Sudoku/365.a.htm
    // 48
    "..D.2...5A1....F"
    "E21F..9..6.D...G"
    "7A.9G1..2E.4D..8"
    ".B64.3FA...8.E.2"
    "...3...F...G2A.."
    ".F7...5.8D4.B.3."
    "4....C.....F8.D1"
    ".G....B3A2.....6"
    "3.....4CF8....6."
    "94.B1.....D....E"
    ".C.A.E8D.B...15."
    "..G8B...1...C..."
    "B.A.8...6F2.3DE."
    "2..16.E5..A39.84"
    "6...9.3..5..1C2B"
    "8....D12...9.6.."
    ,

    // http://janko.at/Raetsel/Sudoku/366.a.htm
    // 49
    "3....2A.4F.6..G1"
    "ADF4..3..5....E."
    "..5G..1E....FAB."
    "..E.GF75..B..93."
    "B...17...A629..."
    "...EFG..7.516..C"
    "63..D...G...E42F"
    "2....8639...1G.."
    "..B8...76G2....E"
    "5C32...4...E..1G"
    "G..62E.9..D4B..."
    "...D5CG...A3...7"
    ".G7..5..84F9.B.."
    ".F6B....5E..21.."
    ".5....B..6..7EF9"
    "E4..A.9F.BC....5"
    ,

    // http://janko.at/Raetsel/Sudoku/367.a.htm
    // 50
    "B........45DC.6A"
    "4.F...639.CA.E.."
    ".9A...CD26.8F1B."
    "6.C3.7.8.F..9..."
    "D68.E..B...5...."
    "73...8A5..9.2..."
    "A.G1..9..26..D8."
    ".C9.......81A6E."
    ".574FA.......G1."
    ".AD..37..G..4F.E"
    "...9.G..452...CD"
    "....B...F..9.725"
    "...B..G.C.F.12.9"
    ".16AC.5FG3...8D."
    "..2.36.759...C.F"
    "F8.C194........6"
    ,

    // http://janko.at/Raetsel/Sudoku/368.a.htm
    // 51
    "C..B61..G...F.AE"
    "4E8F...A.....DG."
    ".D..F3.E1.5A..6."
    "5.....GBE9FC..82"
    "..5EB.D8...1.F.6"
    "..AD.72...6..3.8"
    "...3..E.9FB85..."
    "6.B4..1....2CAE."
    ".3E9C....G..D1.7"
    "...CEG3D.7..A..."
    "A.F..B...CD.EG.."
    "D.7.1...B2.348.."
    "FG..8EB17A.....3"
    ".4..D6.G3.CE..5."
    ".62.....D...74CA"
    "3B.A...4..9F8..G"
    ,

    // http://janko.at/Raetsel/Sudoku/369.a.htm
    // 52
    "...E.6DB...2.8.."
    "....5G.3.9F8C1.."
    "981....FG....7.."
    ".3.F8....65.2..9"
    "3G....4A5D..7.F."
    ".A.D...2......64"
    ".2.4C.F..A.G...3"
    "..F.D.....6C.21E"
    "1F9.B2.....E.D.."
    "4...E.3..B.DF.2."
    "5E......9...3.C."
    ".D.3..8142....5A"
    "2..5.D6....F1.A."
    "..4....86....EDG"
    "..3CGFB.E.DA...."
    "..G.3...279.6..."
    ,

    // http://janko.at/Raetsel/Sudoku/370.a.htm
    // 53
    "3....1G..5.7.A4."
    "FE7C.3.9.......G"
    ".8.....5D9G.C..3"
    "5G.A6....38.FD7."
    "...5G.94.8.27C.."
    ".98.E.5C.G......"
    "4....7.25.CB.6.A"
    "2A.F.......6.5G."
    ".F3.5.......4.DB"
    "C.B.98.AF.7....6"
    "......6.2D.C.9E."
    "..6G4.B.81.9A..."
    ".D53.C8....E6.97"
    "A..2.D467.....5."
    "8.......4.9.32B1"
    ".4F.2.3..61....C"
    ,

    // http://janko.at/Raetsel/Sudoku/501.a.htm
    // 54
    "..E3.7.1.5.6.B.."
    "....G85B92.FE..."
    "C...4.36A......2"
    ".4.BA......G7..C"
    "48.2E...5G.9ADB."
    ".....B...C4...9E"
    "9A..F42.E1...5C."
    ".5C.7.9G8....413"
    "BE8....26A.C.G3."
    ".FD...7E.931..56"
    "AC...G6...2....."
    ".1369.85...BD.A7"
    "E..A6......5C.7."
    "3......A7F.E...5"
    "...51.G7CDA2...."
    "..G.C.B.3.1.9A.."
    ,

    // http://janko.at/Raetsel/Sudoku/502.a.htm
    // 55
    "G..B..12....73.C"
    "..8DB.7CG9A....."
    "6.3...D...4E.8B."
    "4...38G.21....AD"
    "..F.........E.6."
    ".E6..75DFG3.B..."
    ".7.G.F916EB.4583"
    ".B.3.E2..89...F1"
    "74...3E..62.5.1."
    "9852.CB7DFE.3.G."
    "...A.56F431..C9."
    ".F.6.........E.."
    "C1....F4.B5G...E"
    ".D4.EG...2...9.8"
    ".....D8974.1CA.."
    "B.A7....ED..F..4"
    ,

    // http://janko.at/Raetsel/Sudoku/503.a.htm
    // 56
    "7B..9C.3..6.A.2E"
    "4.2...67A..53..D"
    ".....ADE.CB.4.5."
    "AD8....B7.F....."
    ".C...1.A.9.....8"
    "F.E8...C.4.D.6.2"
    "..D.59.......1A."
    ".1.2......7FBDC3"
    "BF6EC4......2.7."
    ".53.......C2.E.."
    "D.G.2.A.5...F3.C"
    "2.....1.B.3...8."
    ".....F.43....CD7"
    ".8.G.E7.C25....."
    "C..DG..1F7...2.A"
    "E7.A.6..9.18..B5"
    ,

    // http://janko.at/Raetsel/Sudoku/504.a.htm
    // 57
    ".21..BD...EF6G.."
    "..6.5.F....D.8.E"
    "G4.....EB.7.1.D3"
    "C.D..37G..61...."
    "AB.D8..9.1.E..F."
    "6.F4..G..D..8..B"
    "....BD.1F.8.7.C9"
    "..3...6..G.BED.."
    "..B91.C..E...A.."
    "47.2.6.A3.1G...."
    "8..3..E..B..F1.G"
    ".1..7.3.D..C9.25"
    "....GF..28B..4.A"
    "5A.8.4.B1.....GF"
    "D.2.C....F.6.7.."
    "..41DA...7G..C9."
    ,

    // http://janko.at/Raetsel/Sudoku/505.a.htm
    // 58
    ".B...7..4D..E13."
    "D......6G.E7...F"
    "8.6F....B....7.."
    "7...91D..F8C.B.."
    ".F.G8.1.3..27..."
    ".1.6.4329.7.F..D"
    "3..7..5..GBD8..."
    "48D.CE....A...9."
    ".3...G....C4.9EB"
    "...148C..3..A..2"
    "9..4.6.7FA5.C.G."
    "...EF..5.1.B6.D."
    "..5.ABG..7F9...6"
    "..3....F....18.E"
    "G...E5.32......9"
    ".6FB..89..3...C."
    ,

    // http://janko.at/Raetsel/Sudoku/506.a.htm
    // 59
    "...FDAE..6.1.3B."
    "1..C...9.D34.6.."
    "8A3.4.....2E7C.."
    "..DG.3......8.5E"
    "3F2.C.....8B.5.9"
    ".DE.8..B2C..A..4"
    "C8...1.........6"
    ".....D.31.5...E."
    ".G...E.84.D....."
    "A.........1...43"
    "D..E..17C..2.FG."
    "2.5.AF.....9.D7B"
    "73.2......6.B4.."
    "..483B.....D.A61"
    "..A.14F.B...5..C"
    ".1C.6.D..24A9..."
    ,

    // http://janko.at/Raetsel/Sudoku/507.a.htm
    // 60
    "4.8CB9G...D..FA2"
    "G..18..A96...E.."
    "F9....ED..8....B"
    "...D73...2F.G.81"
    ".......2....C.F9"
    "3.D7.E6...9.4..A"
    ".C.4..D9.GA..3.7"
    ".2.......7.3.65."
    ".3B.5.2.......D."
    "6.4..G8.FC..B.2."
    "E..G.B...D2.18.F"
    "CD.2....3......."
    "D1.B.8F...C9E..."
    "2....D..1B....G5"
    "..5...17D..G9..C"
    "7E9..C...468FD.3"
    ,

    // http://janko.at/Raetsel/Sudoku/508.a.htm
    // 61
    "D.B.8.4..5G..2.9"
    ".C..6.3...D1.7B."
    "GE4.B..52A..FD.C"
    "..7.2..E.F9....."
    ".2..4....3.D9GEF"
    "43.7..A..8......"
    "5.ABGD.F..1...37"
    "..E......7..6B.."
    "..2E..G......3.."
    "C5...B..D.F971.8"
    "......5..2..C.GE"
    "348GD.C....7..5."
    ".....91.8..F.6.."
    "1.G2..F65..E.89B"
    ".8C.54...1.G..F."
    "F.5..87..9.A.C.1"
    ,

    // http://janko.at/Raetsel/Sudoku/509.a.htm
    // 62 // problems
    "..EF96...8.5B.C."
    "D...FE....1.2A.."
    ".86..2..C...57.1"
    "C21...BD9..E...3"
    "5..1.C...6....DA"
    ".G.....7.E.1.5BC"
    "B...D1..8...7..."
    "..9D..A27.3.E..."
    "...3.A.E5B..6F.."
    "...5...C..83...B"
    "FDB.2.9.6.....5."
    "61....5...E.G..9"
    "4...8..3B7...EA2"
    "E.26...A..G..CF."
    "..58.D....4F...G"
    ".B.7C.2...5AD9.."
    ,

    // http://janko.at/Raetsel/Sudoku/510.a.htm
    // 63
    "..2..DA6.38.14.."
    "...5.F..2......."
    "6.8.2.G.....7A.B"
    "A.F.....65.G..D."
    "...6.8E.52...B.."
    "7....2.F.G39..8E"
    "8..3679..4.C.1.G"
    ".B.EG..18.A....7"
    "1....A.8C..FG.3."
    "E.A.1.3..D42B..5"
    "97..CG6.1.B....4"
    "..3...2D.E6.A..."
    ".C..B.74.....5.A"
    "B.68.....C.7.D.F"
    ".......A..1.2..."
    "..4A.C8.E6G..7.."
    ,

    // http://janko.at/Raetsel/Sudoku/511.a.htm
    // 64
    "A..95.B.G...8.FD"
    "G.D...6FB...A5.."
    ".B...1D83.A9G.7."
    "F68....G5..1...C"
    "..28DA9..5FC...G"
    "..G.87.2.A4D.6.."
    "....3F.....E.CD9"
    "9C5D......G.FA8."
    ".71B.6......5842"
    "D5F.B.....26...."
    "..9.127.F.85.D.."
    "2...E35..CB719.."
    "6...F..BE....734"
    ".F.ECD.3479...2."
    "..AG...9C1...F.8"
    "39.5...1.D.FB..A"
    ,

    // http://janko.at/Raetsel/Sudoku/512.a.htm
    // 65
    "....G5D.C1...B2."
    "6...E.BC.G..D8.."
    "D13C..7..4..5G.."
    ".72B48..5.6.C9.."
    "....B9E....F4.12"
    "...A..FD7B.C8..3"
    "E6F..1....G3.DB9"
    "B..4.2.89.1...G."
    ".5...7.ED.C.G..8"
    "C9D.14....E..A35"
    "8..2D.591A..6..."
    "7G.36....259...."
    "..6D.E.5..B4A27."
    "..95..2..8..B3D4"
    "..7G..4.6E.5...F"
    ".B8...97.C32...."
    ,

    // http://janko.at/Raetsel/Sudoku/513.a.htm
    // 66
    "D...9A..6.F4.3.C"
    "....B6..5A.E9D.."
    "6E5......D..7F.."
    ".3G9..FDC...4..."
    "16..AF4..C.7..58"
    "G....1..A.B8..F9"
    ".7C.3..8...F6..."
    "A8.F.G.C43..2..."
    "...7..9E3.D.8.4F"
    "...8G...9..5.6C."
    "59..F3.4..C....G"
    "4F..6.D..B7A..2E"
    "...D...BE9..G86."
    "..2G..3......BE5"
    "..E48.6A..2C...."
    "3.8.E5.F..AD...4"
    ,

    // http://janko.at/Raetsel/Sudoku/514.a.htm
    // 67
    "...B562E.1G....."
    ".D27A...4...56B."
    ".G6..B.C..2..E7."
    ".1.C..7.DB..3.42"
    "....37.2E..G..64"
    "D.3..5..C.A8.G.1"
    "C..A...F9...D..3"
    ".2.FDG8..3.4.5.E"
    "2.8.F.4..CEBG.5."
    "E..D...G5...C..B"
    "1.5.2C.D..9..F.8"
    "FC..E..5G.81...."
    "A3.5..G8.9..6.E."
    ".F4..A..2.1..39."
    ".E16...3...C42G."
    ".....E1.763AF..."
    ,

    // http://janko.at/Raetsel/Sudoku/515.a.htm
    // 68
    "A.5.FB.C...7..3."
    "G38....D.C.EB5.."
    "..F...3.9G8..D7."
    "...E.2.4.A.59.FG"
    "8..A..6.3F7D5..."
    "B..4....5...DEA."
    "61..BD75..E....."
    ".5C.1E.A.8....62"
    "C8....E.7.62.GD."
    ".....A..D95F..84"
    ".D41...6....2..7"
    "...928D1.E..3..C"
    "57.3E.A.8.9.C..."
    ".21..C58.4...F.."
    "..6C9.1.2....AEB"
    ".A..D...G.1C.4.5"
    ,

    // http://janko.at/Raetsel/Sudoku/516.a.htm
    // 69
    "...E9.5..6.G...F"
    ".6.4.3.G...7.1.B"
    ".37A.E..49.5C..."
    "9....C76.32E.548"
    ".E...4..DG9.2..."
    ".4...562C...G8.."
    ".G93..1B5F.2.C.7"
    "C1.639.A.......5"
    "D.......2.496.8G"
    "6.3.F.9EG5..B2C."
    "..AB...86EC...3."
    "...9.2C1..B...D."
    "7A1.462.3B5....D"
    "...D5.89..G.1BA."
    "E.5.G...9.6.F.2."
    "4...B.E..D.87..."
    ,

    // http://janko.at/Raetsel/Sudoku/517.a.htm
    // 70
    ".FE6.A.8......1."
    "145..7.E..F...68"
    "...C.4..EG82..5F"
    "....5FD.7B1..C.4"
    "..4.82A.C..1B..."
    ".8C7..5....AD24G"
    "..1F..9.G83BA..."
    "..A9F.E6D.....71"
    "F9.....53C.718.."
    "...321CD.6..74.."
    "A1726....4..GFC."
    "...49..F.1A8.E.."
    "D.F..62B.593...."
    "4B..D83A..2.5..."
    "87...5..B.4..936"
    ".6......8.7.CBD."
    ,

    // http://janko.at/Raetsel/Sudoku/518.a.htm
    // 71
    "...84......D.1F7"
    ".6.C..A.4G.....9"
    "..945.G.8.F..3A."
    "...7CFD2.3...E.G"
    "..6B..58..G....."
    "8.7..G.C9.4A1..E"
    "C..EA3.9...5.GD."
    ".F...2.....B3.8A"
    "AB.6F.....C...9."
    ".7E.B...5.16G..3"
    "1..G79.3A.D..C.5"
    ".....1..7E..AB.."
    "3.4...8.E792B..."
    ".E8..D.B.A.359.."
    "5.....1A.D..E.3."
    "69D.2......C7..."
    ,

    // http://janko.at/Raetsel/Sudoku/519.a.htm
    // 72// problems
    "CD.E9.8......6.."
    ".62..3...E..85.7"
    ".G...6...AF5...D"
    "....D54.39.8...E"
    "6FB..ED4...G.1.."
    "4.A.5..2D......G"
    ".C.1...GF53A2..6"
    ".2.3.A.1....F..."
    "...2....4.B.9.6."
    "3..6G8E95...4.2."
    "G......D9..3.F.8"
    "..4.2...68A..GC3"
    "A...1.3E.D9C...."
    "E...49C...5...1."
    "2.9D..A...8..3F."
    "..8......2.7A.E5"
    ,

    // http://janko.at/Raetsel/Sudoku/520.a.htm
    // 73
    "D..6..1.2C3....8"
    "..1..386..5..9.."
    ".8C2......DGBF1."
    "..5.G......1.A.D"
    "..A876..4..C5..."
    "64F...AD.2.5..C."
    "E....1B9.6A...24"
    "B...4....8G...7."
    ".7...BD....6...9"
    "FG...C9.1D4....A"
    ".5..E.6.G7...1DC"
    "...BF..G..2A37.."
    "5.8.6......D.3.."
    ".E2D5A......7CF."
    "..7..9..A51..4.."
    "9....DC1.4..E..5"
    ,

    // http://janko.at/Raetsel/Sudoku/586.a.htm
    // 74
    "8D...6E...A2...."
    "E9.6....4..D.537"
    "..539C.2..7.A4.."
    "..B2G..5......6D"
    ".F....G9CB84...."
    "21.D.F5..7.EB.G6"
    "983..47.65...A.2"
    ".7.C6....A..3954"
    "C698..1....54.7."
    "F.A...46.82..D9G"
    "42.5B.C..3G.6.8F"
    "....5E28F9....A."
    "7B......G..1C3.."
    "..F4.5..B.3AG6.."
    "35E.C..A....7.49"
    "....D1...FC...25"
    ,

    // http://janko.at/Raetsel/Sudoku/587.a.htm
    // 75
    "6D.5F.B83..7...."
    ".F.761...854.A.."
    "39.8.D..B..EF.6."
    "..E.3.956F...CB1"
    "F65......GBA...2"
    "...3.6C7D9...8.B"
    "..DE..82F.7...5."
    "B.7.DF....3.619."
    ".A9B.5....81.F.6"
    ".3...E.BG5..12.."
    "C.G...DF7B9.A..."
    "5...1C6......B84"
    "7E6...F91A.B.3.."
    ".5.9A..D..6.B.2E"
    "..A.BG5...F97.1."
    "....2..65E.8C.AF"
    ,

    // http://janko.at/Raetsel/Sudoku/588.a.htm
    // 76
    "F.....8.43..BGD6"
    "6E..G29....C3.7."
    "2.1..D7.5.E..C.."
    "89.4...C7.612..."
    ".1.F..DAEB5...8."
    "..6AF34...D..E1."
    "E...B..5..8G.493"
    "7.892....4.3D..."
    "...1E.A....F63.C"
    "D6E.89..G..5...1"
    ".G3..F...91E78.."
    ".F...1GB36..E.A."
    "...87G.6C...4.FA"
    "..7..8.1.FB..6.2"
    ".D.GA....134..CE"
    "C2A6..F3.5.....D"
    ,

    // http://janko.at/Raetsel/Sudoku/589.a.htm
    // 77
    "...439.27..5..G."
    "5C67...A93...2D."
    ".3B...1D...2.78."
    "...2.G7.8EA.3.5F"
    "E.C.A23....D...7"
    "...D.B..G7F81..6"
    ".G.1.68.2A.ECD.."
    "F8.6.4G......AE5"
    "643......DB.F.2E"
    "..586.4C.F2.D.B."
    "B..F8A2G..3.7..."
    "7...E....894.C.A"
    "D7.3.1B4.CE.9..."
    ".F4.9...B2...6C."
    ".BG...67D...4E13"
    ".6..F..31.4AB..."
    ,

    // http://janko.at/Raetsel/Sudoku/590.a.htm
    // 78
    ".1B.....2.59F.6G"
    "G.2341A...BF...."
    "C.E8....3..D.24."
    "F....9E..1...B38"
    "..1.F45.6.3.2..9"
    "3..E.8C.5D..1..."
    "....DB..A8...FE3"
    ".4.93.1.E..G.8D."
    ".GF.C..7.5.69.8."
    "BDC...F1..A8...."
    "...5..94.21.G..C"
    "E..1.5.8.FC3.D.."
    "1B4...8..92....6"
    ".A8.9..C....45.B"
    "....12...6853A.F"
    "5C.27A.B.....91."
    ,

    // http://janko.at/Raetsel/Sudoku/591.a.htm
    // 79
    "A57...8E4......9"
    ".4C85.6....9.E3B"
    ".E1...3G657..AC8"
    "...F.1.9.8EA5.7."
    ".8.5.7...4F...2."
    "..9316....BG8..."
    "..E2A..4.....D97"
    "F.4....856..G1.3"
    "6.84..ECB....F.5"
    "5FA.....C..637.."
    "...7F4....1528.."
    ".2...35...8.C.1."
    ".9.13E2.8.4.A..."
    "BD2..G4FAC...98."
    "8G5.C....D.EF3B."
    "4......57B...2G6"
    ,

    // http://janko.at/Raetsel/Sudoku/592.a.htm
    // 80
    "..8.A.F.B97.1.6."
    "1B79...6.....8E."
    "..E3..928...D.5."
    "26..8.B....17..."
    "91..6A2..B3..D.."
    ".F..37.52.GD.4.."
    "3.679...5.EF...A"
    "4.B..FC..16...G5"
    "73...8A..5D..F.1"
    "5...FC.3...98G.E"
    "..F.D5.13.A8..B."
    "..A..B6..2FG..7D"
    "...6B....E.2..1C"
    ".4.F...A1D..GE.."
    ".A2.....G...4638"
    ".7.G.28F.6.3.A.."
    ,

    // http://janko.at/Raetsel/Sudoku/593.a.htm
    // 81
    ".2ED.4...C...F.."
    ".8.FE.6.1.....A4"
    "A..7.8F.29.B...C"
    "....3GCB...5.87D"
    "..45DA..B7F3C.6."
    "....G...4..CA2.5"
    "2.7.F..CG...34B."
    ".ED.B54381..G..."
    "...4..825GBE.3F."
    ".B5G...63..F.C.1"
    "1.F27..E...4...."
    ".D.84FG5..167B.."
    "3C1.2...DB8G...."
    "D...6.59.F3.8..2"
    "4G.....F.A.71.C."
    "..8...B...9.ED3."
    ,

    // http://janko.at/Raetsel/Sudoku/594.a.htm
    // 82
    "5....E...7B..CF2"
    "AF.467....3...D."
    "D.E.C.3....8.7.."
    "....8..DA6.E..5."
    "..3B....4.A.EF1."
    "69..B.D.3.....C7"
    "1..G...49.C..6.."
    "...D12G5FE..B..."
    "...7..93285AF..."
    "..8..4.76...2..G"
    "96.....F.3.B..84"
    ".AB3.G.6....95.."
    ".1..F.69E..2...."
    "..9.G....F.3.2.E"
    ".E...D....95G.BF"
    "2G7..C5...4....3"
    ,

    // http://janko.at/Raetsel/Sudoku/595.a.htm
    // 83
    ".3.B8..C61...2.."
    ".E.2...F..4.1A67"
    "71D...A...C..B.."
    ".6...92D.B.3..45"
    "...8A...C.F4...3"
    ".F9.75E4..B.6..."
    "5..1...8..6.4F.."
    "4...C....EA.B.2G"
    "2D.7.E5....6...1"
    "..BA.D..9...G..8"
    "...E.1..G2DF.59."
    "6...4C.B...73..."
    "A2..B.C.46E...7."
    "..1..2...9...8AD"
    "DC75.A..3...E.B."
    "..8...GEA..C2.5."
    ,

    // http://janko.at/Raetsel/Sudoku/596.a.htm
    // 84
    "B62...9...1...G7"
    "C.....7.6..A.1.9"
    ".1....A8..7.2..3"
    "..A831.CG2..E..."
    ".8..92.A..GD4..."
    "G.C.D8..F.529..."
    "...4.........D6A"
    ".5.B.C.17..3G8.."
    "..E12..9D.B.6.8."
    "9C7.........5..."
    "...A54.7..2C.F.G"
    "...FCD..4.A5..9."
    "...9..1E2.F83C.."
    "1..7.9..A3....D."
    "2.3.8..B.5.....6"
    "EB...3...7...G12"
    ,

    // http://janko.at/Raetsel/Sudoku/597.a.htm
    // 85
    "..6..1..4..F3.A."
    "7E..6G.3....FC.."
    "BC3..FD.6.9.G..2"
    ".....4E.CG3.B..."
    "1.E..C.....2.A.."
    "...AE6...4F..9.3"
    ".8.2..3F.C.1...."
    "34..8...B57..EC."
    ".B9..3CD...8..E5"
    "....9.F.GA..4.1."
    "A.4..E2...6BC..."
    "..8.A.....E..7.B"
    "...C.2A6.EB....."
    "E..8.9.5.3C..62G"
    "..B9....2.GD..5C"
    ".3.5C..E..1..4.."
    ,

    // http://janko.at/Raetsel/Sudoku/598.a.htm
    // 86
    ".FA..5...B7..E.6"
    "18....4B...C..G."
    "..G.7..CF....8.5"
    ".B9CG8.........."
    ".D..A.G..85....E"
    "69..3...1...C5.."
    "7.4B....G.A.2..F"
    "..8.5B6D..27...."
    "....8G..EC61.D.."
    "3..6.C.7....91.8"
    "..DF...5...9..73"
    "8....6B..2.5..E."
    "..........14EFD."
    "9.6....AD..F.7.."
    ".E..6...29....1C"
    "G.2..EF...C..4B."
    ,

    // http://janko.at/Raetsel/Sudoku/599.a.htm
    // 87
    ".3..6GF..C8....."
    "G..9.B.8...5.A74"
    "7..8.2..B..D...."
    ".EA.....3.6..9B."
    "..F.2....3AG...."
    "E7...........39G"
    ".9.A..31E6..FC.."
    "4.3..D.9C.B....E"
    "6....8.4A.2..G.C"
    "..G3..CFD9..4.A."
    "A85...........FB"
    "....5A7....6.2.."
    ".24..7.D.....16."
    "....1..3..C.2..8"
    "1A7.C...6.3.E..9"
    ".....6E..1F2..3."
    ,

    // http://janko.at/Raetsel/Sudoku/600.a.htm
    // 88
    "....47.....9B.1."
    "..5..9F2.6....43"
    "B..3.....E24..9."
    "..9.....A57..2.."
    ".DA4E..7...C.F.."
    ".1.6..4.E....8.B"
    "....81BF..4.5..7"
    "...F9A....D..E.4"
    "5.2..3....A71..."
    "6..C.B..G9FE...."
    "7.8....9.D..C.3."
    "..G.6...1..3E45."
    "..F..E8D.....A.."
    ".2..741.....3..8"
    "D6....5.92B..1.."
    ".A.EB.....C6....",
    // 89
    //".N..JG..O7591...8I....L.."
    "BNDAJG62O7591KHF8IP34CLME"
    "FG.M.B8...P.E...CJ..H...."
    "...........G.4.H.D.O.NJA2"
    ".....J.EN4.L6MA.B.2......"
    "HE..2..DC.....F4KMA.B.9O8"
    "M....62...47C19......E5.."
    ".I2.8M.JGL...ADN..K..3.F7"
    "..H3.5..89....I.J.....NL."
    "1B..9.FAP.6.N....537.H..O"
    "......1..N...O...LC.68.PG"
    "KOA.FNBH.....7.C.....M..6"
    "45.ECP.I..N.F.J1...MK.79."
    "I.L..8.O..9.P...A...2.1J."
    "..621.D.M.....B8LG..P..CH"
    ".HP.N7E.L1....3..B..O.G45"
    "....BIO....5.C.P...FN48E."
    "...FL.....2.DH..17..59O.."
    "..I.MF..2G.N...A6O.HC.PB."
    "72.1..L...IM.96E.45G....."
    "..9...7M..A.O...I...L...."
    ".C.JA.........1.....E.48."
    "O.BI.......PHL.6..1....5C"
    "G6M...N4FI8...K..H.E....."
    ".L..4.917....BE.G8F.M.I.."
    "8F......5.O3..4...9.....K",
    // 90
    //".N..JG..O7591...8I....L.."
    ".ND.JG6.O7591..F8IP.4.LM."
    "FG.M.B8...P.E...CJ..H...."
    "...........G.4.H.D.O.NJA2"
    ".....J.EN4.L6MA.B.2......"
    "HE..2..DC.....F4KMA.B.9O8"
    "M....62...47C19......E5.."
    ".I2.8M.JGL...ADN..K..3.F7"
    "..H3.5..89....I.J.....NL."
    "1B..9.FAP.6.N....537.H..O"
    "......1..N...O...LC.68.PG"
    "KOA.FNBH.....7.C.....M..6"
    "45.ECP.I..N.F.J1...MK.79."
    "I.L..8.O..9.P...A...2.1J."
    "..621.D.M.....B8LG..P..CH"
    ".HP.N7E.L1....3..B..O.G45"
    "....BIO....5.C.P...FN48E."
    "...FL.....2.DH..17..59O.."
    "..I.MF..2G.N...A6O.HC.PB."
    "72.1..L...IM.96E.45G....."
    "..9...7M..A.O...I...L...."
    ".C.JA.........1.....E.48."
    "O.BI.......PHL.6..1....5C"
    "G6M...N4FI8...K..H.E....."
    ".L..4.917....BE.G8F.M.I.."
    "8F......5.O3..4...9.....K"
  };

  /// The number of instances
  const unsigned int n_examples = sizeof(examples)/sizeof(char*);

  /// The size of an instance
  int example_size(const char *s) {
    int l = std::strlen(s);
    int res = static_cast<int>(std::sqrt(std::sqrt(static_cast<float>(l))));
    assert(res*res*res*res == l);
    return res;
  }

  /// Return value at position (\a i,\a j) in the example \a s of size \a n
  int sudokuField(const char *s, int n, int i, int j) {
    assert(example_size(s) == std::sqrt(static_cast<float>(n)));
    assert(i >= 0 && i < n);
    assert(j >= 0 && j < n);
    char c = s[j*n + i];
    if (!std::isalnum(c))
      return 0;
    if (std::isdigit(c))
      return c - '0';
    if (std::islower(c))
      c = static_cast<char>(std::toupper(c));
    // std::alpha(c) == true && std::isupper(c)
    int res = (c - 'A') + 10;
    if (res > n) return 0;
    else return res;
  }
  //@}
}

// STATISTICS: example-any
