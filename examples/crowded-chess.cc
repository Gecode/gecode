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
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

/// The maximum number of knights placeable
const int kval[] = {
  0,   0,  0,  0,  5,
  9,  15, 21, 29, 37,
  47, 57, 69, 81, 94,
  109
};
const int nkval = 16;

/**
   \brief %Example: Crowded Chessboard

   "You are given a chessboard together with 8 queens, 8 rooks, 14
   bishops, and 21 knights. The puzzle is to arrange the 51 pieces on
   the chessboard so that no queen shall attack another queen, no rook
   attack another rook, no bishop attack another bishop, and no knight
   attack another knight. No notice is to be taken of the intervention
   of pieces of another type from that under consideration - that is,
   two queens will be considered to attack one another although there
   may be, say, a rook, a bishop, and a knight between them. It is not
   difficult to dispose of each type of piece separately; the
   difficulty comes in when you have to find room for all the
   arrangements on the board simultaneously."
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

 \todo Currently this script finds a solution. Instead, it should find
 the maximum number of knights for a given size.

   \ingroup ExProblem
*/
class CrowdedChess : public Example {
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
    MiniModel::Matrix<BoolVarArray> kb(knights, n, n);
    for (int x = n; x--; )
      for (int y = n; y--; )
        for (int i = 4; i--; )
          if (valid_pos(x+kmoves[i][0], y+kmoves[i][1]))
            rel(this, 
                kb(x, y),
                BOT_AND,
                kb(x+kmoves[i][0], y+kmoves[i][1]),
                0);
  }


public:
  /// The model of the problem
  CrowdedChess(const Options& o)
    : n(o.size), s(this, n*n, 0, PMAX-1), queens(this, n, 0, n-1),
      rooks(this, n, 0, n-1), knights(this, n*n, 0, 1) {
    const int nn = n*n, q = n, r = n, b = (2*n)-2,
      k = n <= nkval ? kval[n-1] : kval[nkval-1];
    const int e = nn - (q + r + b + k);

    assert(nn == (e + q + r + b + k));

    MiniModel::Matrix<IntVarArray> m(s, n);

    // ***********************
    // Basic model
    // ***********************

    count(this, s, E, IRT_EQ, e, o.icl);
    count(this, s, Q, IRT_EQ, q, o.icl);
    count(this, s, R, IRT_EQ, r, o.icl);
    count(this, s, B, IRT_EQ, b, o.icl);
    count(this, s, K, IRT_EQ, k, o.icl);

    // Collect rows and columns for handling rooks and queens.
    for (int i = 0; i < n; ++i) {
      IntVarArgs aa = m.row(i), bb = m.col(i);

      count(this, aa, Q, IRT_EQ, 1, o.icl);
      count(this, bb, Q, IRT_EQ, 1, o.icl);
      count(this, aa, R, IRT_EQ, 1, o.icl);
      count(this, bb, R, IRT_EQ, 1, o.icl);

      //Connect (queens|rooks)[i] to the row it is in
      element(this, aa, queens[i], Q, ICL_DOM);
      element(this, aa,  rooks[i], R, ICL_DOM);
    }

    // N-queens constraints
    distinct(this, queens, ICL_DOM);
    IntArgs koff(n);
    for (int i = n; i--; ) koff[i] = i;
    distinct(this, koff, queens, ICL_DOM);
    for (int i = n; i--; ) koff[i] = -i;
    distinct(this, koff, queens, ICL_DOM);

    // N-rooks constraints
    distinct(this,  rooks, ICL_DOM);

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

      count(this, d1, Q, IRT_LQ, 1, o.icl);
      count(this, d2, Q, IRT_LQ, 1, o.icl);
      count(this, d3, Q, IRT_LQ, 1, o.icl);
      count(this, d4, Q, IRT_LQ, 1, o.icl);
      count(this, d1, B, IRT_LQ, 1, o.icl);
      count(this, d2, B, IRT_LQ, 1, o.icl);
      count(this, d3, B, IRT_LQ, 1, o.icl);
      count(this, d4, B, IRT_LQ, 1, o.icl);
    }

    // Handle knigths
    //Connect knigths to board
    for(int i = n*n; i--; )
      knights[i] = post(this, ~(s[i] == K));
    knight_constraints();


    // ***********************
    // Redundant constraints
    // ***********************

    // Queens and rooks not in the same place
    // Faster than going through the channeled board-connection
    for (int i = n; i--; ) {
      rel(this, queens[i], IRT_NQ, rooks[i]);
    }

    // Place bishops in two corners (from Schimpf and Hansens solution)
    // Avoids some symmetries of the problem
    rel(this, m(n-1,   0), IRT_EQ, B);
    rel(this, m(n-1, n-1), IRT_EQ, B);


    // ***********************
    // Branchings
    // ***********************
    //Place each piece in turn
    branch(this, s, BVAR_MIN_MIN, BVAL_MIN);
  }

  /// Constructor for cloning e
  CrowdedChess(bool share, CrowdedChess& e)
    : Example(share,e), n(e.n) {
    s.update(this, share, e.s);
    queens.update(this, share, e.queens);
    rooks.update(this, share, e.rooks);
    knights.update(this, share, e.knights);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new CrowdedChess(share,*this);
  }

  /// Print solution
  virtual void
  print(void) {
    MiniModel::Matrix<IntVarArray> m(s, n);
    char *names = static_cast<char*>(Memory::malloc(PMAX));
    const char *sep   = n < 8 ? "\t\t" : "\t";
    names[E] = '.'; names[Q] = 'Q'; names[R] = 'R';
    names[B] = 'B'; names[K] = 'K';

    for (int r = 0; r < n; ++r){
      // Print main board
      std::cout << '\t';
      for (int c = 0; c < n; ++c) {
        std::cout << names[m(r, c).val()];
      }
      // Print each piece on its own
      for (int p = 0; p < PMAX; ++p) {
        if (p == E) continue;
        std::cout << sep;
        for (int c = 0; c < n; ++c) {
          if (m(r, c).val() == p) std::cout << names[p];
          else                   std::cout << names[E];
        }
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};

/** \brief Main function
 * \relates CrowdedChess
 */
int
main(int argc, char** argv) {
  Options o("CrowdedChess");
  o.icl        = ICL_DOM;
  o.size       = 7;
  o.parse(argc,argv);
  if(o.size < 5) {
    std::cerr << "Error: Size must be at least 5" << std::endl;
    return 1;
  }
  Example::run<CrowdedChess,DFS>(o);
  return 0;
}

// STATISTICS: example-any

