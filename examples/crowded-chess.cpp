/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *     Mikael Lagerkvist, 2005
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

/** The maximum number of knights placeable
 *
 * \relates QueensArmies
 */
const int kval[] = {
  0,   0,  0,  0,  5,
  9,  15, 21, 29, 37,
  47, 57, 69, 81, 94,
  109
};

namespace {
  /** \brief Set of valid positions for the bishops
   * \relates CrowdedChess
   */
  TupleSet bishops;
  /** \brief Class for solving the bishops sub-problem
   * \relates CrowdedChess
   */
  class Bishops : public Space {
  public:
    const int n;
    BoolVarArray k;
    bool valid_pos(int i, int j) {
      return (i >= 0 && i < n) && (j >= 0 && j < n);
    }
    Bishops(int size)
      : n(size), k(*this,n*n,0,1) {
      Matrix<BoolVarArray> kb(k, n, n);
      for (int l = n; l--; ) {
        const int il = (n-1) - l;
        BoolVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
        for (int i = 0; i <= l; ++i) {
          d1[i] = kb(i+il, i);
          d2[i] = kb(i, i+il);
          d3[i] = kb((n-1)-i-il, i);
          d4[i] = kb((n-1)-i, i+il);
        }

        linear(*this, d1, IRT_LQ, 1);
        linear(*this, d2, IRT_LQ, 1);
        linear(*this, d3, IRT_LQ, 1);
        linear(*this, d4, IRT_LQ, 1);
      }

      linear(*this, k, IRT_EQ, 2*n - 2);
      // Forced bishop placement from crowded chess model
      rel(*this, kb(n-1,   0), IRT_EQ, 1);
      rel(*this, kb(n-1, n-1), IRT_EQ, 1);
      branch(*this, k, BOOL_VAR_DEGREE_MAX(), BOOL_VAL_MAX());
    }
    Bishops(Bishops& s) : Space(s), n(s.n) {
      k.update(*this, s.k);
    }
    virtual Space* copy(void) {
      return new Bishops(*this);
    }
  };
  /** \brief Initialize bishops
   * \relates CrowdedChess
   */
  void init_bishops(int size) {
    Bishops* prob = new Bishops(size);
    DFS<Bishops> e(prob); 
    IntArgs ia(size*size);
    delete prob;

    bishops.init(size*size);

    while (Bishops* s = e.next()) {
      for (int i = size*size; i--; )
        ia[i] = s->k[i].val();
      bishops.add(ia);
      delete s;
    }

    bishops.finalize();
  }
}
/**
   \brief %Example: Crowded chessboard

   You are given a chessboard together with 8 queens, 8 rooks, 14
   bishops, and 21 knights. The puzzle is to arrange the 51 pieces on
   the chessboard so that no queen shall attack another queen, no rook
   attack another rook, no bishop attack another bishop, and no knight
   attack another knight. No notice is to be taken of the intervention
   of pieces of another type from that under consideration - that is,
   two queens will be considered to attack one another although there
   may be, say, a rook, a bishop, and a knight between them. It is not
   difficult to dispose of each type of piece separately; the
   difficulty comes in when you have to find room for all the
   arrangements on the board simultaneously.
   <em>Dudeney, H.E., (1917), Amusements in Mathematics,
   Thomas Nelson and Sons.</em>

   This puzzle can be generalized to chess-boards of size \f$n\f$, where the
   number of pieces to place are:
     - \f$ n \f$ queens
     - \f$ n \f$ rooks
     - \f$ 2n-1 \f$ bishops
     - \f$ k \f$ knights
   where k is a number to maximize.

   The maximum k for some different values of \f$ n \f$ are presented
   below (from Jesper Hansen and Joachim Schimpf, <a
   href="http://www.icparc.ic.ac.uk/eclipse/examples/crowded_chess.ecl.txt">
   ECLiPSe solution</a>
   <TABLE>
   <TR> <TD> n </TD> <TD> k </TD> </TR>
   <TR> <TD> 8 </TD> <TD> 21 </TD> </TR>
   <TR> <TD> 9 </TD> <TD> 29 </TD> </TR>
   <TR> <TD> 10 </TD> <TD> 37 </TD> </TR>
   <TR> <TD> 11 </TD> <TD> 47 </TD> </TR>
   <TR> <TD> 12 </TD> <TD> 57 </TD> </TR>
   <TR> <TD> 13 </TD> <TD> 69 </TD> </TR>
   <TR> <TD> 14 </TD> <TD> 81 </TD> </TR>
   <TR> <TD> 15 </TD> <TD> 94 </TD> </TR>
   <TR> <TD> 16 </TD> <TD> 109 </TD> </TR>
   </TABLE>

   A solution for n = 8 is:
   <TABLE>
   <TR><TD>Q</TD><TD>B</TD><TD>K</TD><TD>.</TD>
   <TD>K</TD><TD>B</TD><TD>K</TD><TD>R</TD></TR>
   <TR><TD>.</TD><TD>K</TD><TD>.</TD><TD>K</TD>
   <TD>Q</TD><TD>K</TD><TD>R</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>.</TD><TD>K</TD><TD>R</TD>
   <TD>K</TD><TD>.</TD><TD>K</TD><TD>Q</TD></TR>
   <TR><TD>B</TD><TD>K</TD><TD>R</TD><TD>K</TD>
   <TD>.</TD><TD>Q</TD><TD>.</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>R</TD><TD>Q</TD><TD>.</TD>
   <TD>K</TD><TD>.</TD><TD>K</TD><TD>B</TD></TR>
   <TR><TD>R</TD><TD>K</TD><TD>.</TD><TD>K</TD>
   <TD>.</TD><TD>K</TD><TD>Q</TD><TD>B</TD></TR>
   <TR><TD>B</TD><TD>Q</TD><TD>K</TD><TD>.</TD>
   <TD>K</TD><TD>R</TD><TD>K</TD><TD>.</TD></TR>
   <TR><TD>B</TD><TD>K</TD><TD>B</TD><TD>Q</TD>
   <TD>R</TD><TD>K</TD><TD>B</TD><TD>B</TD></TR>
 </TABLE>

   \ingroup Example
*/
class CrowdedChess : public Script {
protected:
  const int n;          ///< Board-size
  IntVarArray s;        ///< The board
  IntVarArray queens,   ///< Row of queen in column x
    rooks;              ///< Row of rook in column x
  BoolVarArray knights; ///< True iff the corresponding place has a knight

  /** Symbolic names of pieces. The order determines which piece will
   * be placed first.
   */
  enum
    {Q,   ///< Queen
     R,   ///< Rook
     B,   ///< Bishop
     K,   ///< Knight
     E,   ///< Empty square
     PMAX ///< Number of pieces (including empty squares)
    } piece;

  // Is (i,j) a valid position on the board?
  bool valid_pos(int i, int j) {
    return (i >= 0 && i < n) &&
      (j >= 0 && j < n);
  }

  /// Post knight-constraints
  void knight_constraints(void) {
    static const int kmoves[4][2] = {
      {-1,2}, {1,2}, {2,-1}, {2,1}
    };
    Matrix<BoolVarArray> kb(knights, n, n);
    for (int x = n; x--; )
      for (int y = n; y--; )
        for (int i = 4; i--; )
          if (valid_pos(x+kmoves[i][0], y+kmoves[i][1]))
            rel(*this,
                kb(x, y),
                BOT_AND,
                kb(x+kmoves[i][0], y+kmoves[i][1]),
                0);
  }


public:
  enum {
    PROP_TUPLE_SET, ///< Propagate bishops placement extensionally
    PROP_DECOMPOSE  ///< Propagate bishops placement with decomposition
  };

  /// The model of the problem
  CrowdedChess(const SizeOptions& opt)
    : Script(opt),
      n(opt.size()),
      s(*this, n*n, 0, PMAX-1),
      queens(*this, n, 0, n-1),
      rooks(*this, n, 0, n-1),
      knights(*this, n*n, 0, 1) {
    const int nkval = sizeof(kval)/sizeof(int);
    const int nn = n*n, q = n, r = n, b = (2*n)-2,
      k = n <= nkval ? kval[n-1] : kval[nkval-1];
    const int e = nn - (q + r + b + k);

    assert(nn == (e + q + r + b + k));

    Matrix<IntVarArray> m(s, n);

    // ***********************
    // Basic model
    // ***********************

    count(*this, s, E, IRT_EQ, e, opt.ipl());
    count(*this, s, Q, IRT_EQ, q, opt.ipl());
    count(*this, s, R, IRT_EQ, r, opt.ipl());
    count(*this, s, B, IRT_EQ, b, opt.ipl());
    count(*this, s, K, IRT_EQ, k, opt.ipl());

    // Collect rows and columns for handling rooks and queens
    for (int i = 0; i < n; ++i) {
      IntVarArgs aa = m.row(i), bb = m.col(i);

      count(*this, aa, Q, IRT_EQ, 1, opt.ipl());
      count(*this, bb, Q, IRT_EQ, 1, opt.ipl());
      count(*this, aa, R, IRT_EQ, 1, opt.ipl());
      count(*this, bb, R, IRT_EQ, 1, opt.ipl());

      // Connect (queens|rooks)[i] to the row it is in
      element(*this, aa, queens[i], Q, IPL_DOM);
      element(*this, aa,  rooks[i], R, IPL_DOM);
    }

    // N-queens constraints
    distinct(*this, queens, IPL_DOM);
    distinct(*this, IntArgs::create(n,0,1), queens, IPL_DOM);
    distinct(*this, IntArgs::create(n,0,-1), queens, IPL_DOM);

    // N-rooks constraints
    distinct(*this,  rooks, IPL_DOM);

    // Collect diagonals for handling queens and bishops
    for (int l = n; l--; ) {
      const int il = (n-1) - l;
      IntVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
      for (int i = 0; i <= l; ++i) {
        d1[i] = m(i+il, i);
        d2[i] = m(i, i+il);
        d3[i] = m((n-1)-i-il, i);
        d4[i] = m((n-1)-i, i+il);
      }

      count(*this, d1, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d2, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d3, Q, IRT_LQ, 1, opt.ipl());
      count(*this, d4, Q, IRT_LQ, 1, opt.ipl());
      if (opt.propagation() == PROP_DECOMPOSE) {
        count(*this, d1, B, IRT_LQ, 1, opt.ipl());
        count(*this, d2, B, IRT_LQ, 1, opt.ipl());
        count(*this, d3, B, IRT_LQ, 1, opt.ipl());
        count(*this, d4, B, IRT_LQ, 1, opt.ipl());
      }
    }
    if (opt.propagation() == PROP_TUPLE_SET) {
      IntVarArgs b(s.size());
      for (int i = s.size(); i--; )
        b[i] = channel(*this, expr(*this, (s[i] == B)));
      extensional(*this, b, bishops, opt.ipl());
    }

    // Handle knigths
    // Connect knigths to board
    for(int i = n*n; i--; )
      knights[i] = expr(*this, (s[i] == K));
    knight_constraints();


    // ***********************
    // Redundant constraints
    // ***********************

    // Queens and rooks not in the same place
    // Faster than going through the channelled board-connection
    for (int i = n; i--; )
      rel(*this, queens[i], IRT_NQ, rooks[i]);

    // Place bishops in two corners (from Schimpf and Hansens solution)
    // Avoids some symmetries of the problem
    rel(*this, m(n-1,   0), IRT_EQ, B);
    rel(*this, m(n-1, n-1), IRT_EQ, B);


    // ***********************
    // Branching
    // ***********************
    // Place each piece in turn
    branch(*this, s, INT_VAR_MIN_MIN(), INT_VAL_MIN());
  }

  /// Constructor for cloning e
  CrowdedChess(CrowdedChess& e)
    : Script(e), n(e.n) {
    s.update(*this, e.s);
    queens.update(*this, e.queens);
    rooks.update(*this, e.rooks);
    knights.update(*this, e.knights);
  }

  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new CrowdedChess(*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> m(s, n);
    char names[PMAX];
    names[E] = '.'; names[Q] = 'Q'; names[R] = 'R';
    names[B] = 'B'; names[K] = 'K';
    const char* sep   = n < 8 ? "\t\t" : "\t";

    for (int r = 0; r < n; ++r){
      // Print main board
      os << '\t';
      for (int c = 0; c < n; ++c) {
        if (m(r, c).assigned()) {
          os << names[m(r, c).val()];
        } else {
          os << " ";
        }
      }
      // Print each piece on its own
      for (int p = 0; p < PMAX; ++p) {
        if (p == E) continue;
        os << sep;
        for (int c = 0; c < n; ++c) {
          if (m(r, c).assigned()) {
            if (m(r, c).val() == p)
              os << names[p];
            else
              os << names[E];
          } else {
            os << " ";
          }
        }
      }
      os << std::endl;
    }
    os << std::endl;
  }
};

/** \brief Main function
 * \relates CrowdedChess
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("CrowdedChess");
  opt.propagation(CrowdedChess::PROP_TUPLE_SET);
  opt.propagation(CrowdedChess::PROP_TUPLE_SET,
                  "extensional",
                  "Use extensional propagation for bishops-placement");
  opt.propagation(CrowdedChess::PROP_DECOMPOSE,
                  "decompose",
                  "Use decomposed propagation for bishops-placement");
  opt.ipl(IPL_DOM);
  opt.size(8);
  opt.parse(argc,argv);
  if (opt.size() < 5) {
    std::cerr << "Error: size must be at least 5" << std::endl;
    return 1;
  }
  init_bishops(opt.size());
  Script::run<CrowdedChess,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

