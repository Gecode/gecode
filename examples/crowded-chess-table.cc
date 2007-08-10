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
#include "gecode/minimodel.hh"

/// The maximum number of knights placeable
const int kval[] = {
  0,   0,  0,  0,  5,
  9,  15, 21, 29, 37,
  47, 57, 69, 81, 94,
  109
};
const int nkval = 16;

namespace {
  Table knight_table;
  class Knights : public Space {
  public:
    const int n;
    BoolVarArray k;
    bool valid_pos(int i, int j) {
      return (i >= 0 && i < n) && (j >= 0 && j < n);
    }
    Knights(int size)
      : n(size), k(this,n*n,0,1) {
      static const int kmoves[4][2] = {
        {-1,2}, {1,2}, {2,-1}, {2,1}
      };
      MiniModel::Matrix<BoolVarArray> kb(k, n, n);
      for (int x = n; x--; )
        for (int y = n; y--; )
          for (int i = 4; i--; )
            if (valid_pos(x+kmoves[i][0], y+kmoves[i][1]))
              rel(this, 
                  kb(x, y),
                  BOT_AND,
                  kb(x+kmoves[i][0], y+kmoves[i][1]),
                  0);
      linear(this, k, IRT_EQ, n <= nkval ? kval[n-1] : kval[nkval-1]);
      // Incorporate bishop-negation
      rel(this, kb(n-1,   0), IRT_NQ, 1);
      rel(this, kb(n-1, n-1), IRT_NQ, 1);
      branch(this, k, BVAR_DEGREE_MAX, BVAL_MAX);
    }
    Knights(bool share, Knights& s) : Space(share,s), n(s.n) {
      k.update(this, share, s.k);
    }
    virtual Space* copy(bool share) {
      return new Knights(share,*this);
    }
  };
  void init_knight(int size) {
    Knights* prob = new Knights(size);
    DFS<Knights> e(prob); IntArgs ia(size*size);
    delete prob;
    int cnt = 0;
    while (Knights* s = e.next()) {
      for (int i = 0; i < size*size; ++i) { //size*size; i--; ) {
        ia[i] = s->k[i].val();
        //std::cerr << (ia[i] ? "K" : "·");
        //if ((i+1)%size == 0) std::cerr << std::endl;
      }
      ++cnt;
      knight_table.add(ia);
      delete s;
    }
    std::cerr << "Count == " << cnt << std::endl;
    knight_table.finalize();
  }


  Table bishop_table;
  class Bishops : public Space {
  public:
    const int n;
    BoolVarArray k;
    bool valid_pos(int i, int j) {
      return (i >= 0 && i < n) && (j >= 0 && j < n);
    }
    Bishops(int size)
      : n(size), k(this,n*n,0,1) {
      MiniModel::Matrix<BoolVarArray> kb(k, n, n);
      for (int l = n; l--; ) {
        const int il = (n-1) - l;
        BoolVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
        for (int i = 0; i <= l; ++i) {
          d1[i] = kb(i+il, i);
          d2[i] = kb(i, i+il);
          d3[i] = kb((n-1)-i-il, i);
          d4[i] = kb((n-1)-i, i+il);
        }

        linear(this, d1, IRT_LQ, 1);
        linear(this, d2, IRT_LQ, 1);
        linear(this, d3, IRT_LQ, 1);
        linear(this, d4, IRT_LQ, 1);
      }

      linear(this, k, IRT_EQ, 2*n - 2);
      // Incorporate bishop-placement
      rel(this, kb(n-1,   0), IRT_EQ, 1);
      rel(this, kb(n-1, n-1), IRT_EQ, 1);
      branch(this, k, BVAR_DEGREE_MAX, BVAL_MAX);
    }
    Bishops(bool share, Bishops& s) : Space(share,s), n(s.n) {
      k.update(this, share, s.k);
    }
    virtual Space* copy(bool share) {
      return new Bishops(share,*this);
    }
  };
  void init_bishop(int size) {
    Bishops* prob = new Bishops(size);
    DFS<Bishops> e(prob); IntArgs ia(size*size);
    delete prob;
    int cnt = 0;
    while (Bishops* s = e.next()) {
      for (int i = 0; i < size*size; ++i) { //size*size; i--; ) {
        ia[i] = s->k[i].val();
        //std::cerr << (ia[i] ? "K" : "·");
        //if ((i+1)%size == 0) std::cerr << std::endl;
      }
      ++cnt;
      bishop_table.add(ia);
      delete s;
    }
    std::cerr << "Count == " << cnt << std::endl;
    bishop_table.finalize();
  }
}

/**
   \brief %Example: Crowded Chessboard using tables
   \ingroup ExProblem
*/
class CrowdedChessTable : public Example {
protected:
  const int n;          ///< Board-size
  IntVarArray s;        ///< The board
  IntVarArray queens,   ///< Row of queen in column x
    rooks;              ///< Row of rook in column x
  IntVarArray knights; ///< True iff the corresponding place has a knight

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

public:
  /// The model of the problem
  CrowdedChessTable(const Options& o)
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
      element(this, aa, queens[i], Q, 0, ICL_DOM);
      element(this, aa,  rooks[i], R, 0, ICL_DOM);
    }

    // N-queens constraints
    ExtensionalAlgorithm ea = EA_BASIC;
    if (!o.naive) {
      ea = EA_INCREMENTAL;
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
    if (false && o.naive) {
      for (int l = n; l--; ) {
        const int il = (n-1) - l;
        IntVarArgs d1(l+1), d2(l+1), d3(l+1), d4(l+1);
        for (int i = 0; i <= l; ++i) {
          d1[i] = m(i+il, i);
          d2[i] = m(i, i+il);
          d3[i] = m((n-1)-i-il, i);
          d4[i] = m((n-1)-i, i+il);
        }
        
        count(this, d1, B, IRT_LQ, 1, o.icl);
        count(this, d2, B, IRT_LQ, 1, o.icl);
        count(this, d3, B, IRT_LQ, 1, o.icl);
        count(this, d4, B, IRT_LQ, 1, o.icl);
      }
    } else {
      IntVarArgs iva(n*n);
      for (int i = iva.size(); i--; ) 
        iva[i] = channel(this, post(this, ~(s[i] == B)));
      extensional(this, iva, bishop_table);
    }

    // Handle knigths
    //Connect knigths to board
    for(int i = n*n; i--; )
      knights[i] = channel(this, post(this, ~(s[i] == K)));
    //knight_constraints();
    extensional(this, knights, knight_table, ea);


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
  CrowdedChessTable(bool share, CrowdedChessTable& e)
    : Example(share,e), n(e.n) {
    s.update(this, share, e.s);
    queens.update(this, share, e.queens);
    rooks.update(this, share, e.rooks);
    knights.update(this, share, e.knights);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new CrowdedChessTable(share,*this);
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
 * \relates CrowdedChessTable
 */
int
main(int argc, char** argv) {
  Options o("CrowdedChessTable");
  o.icl        = ICL_DOM;
  o.size       = 6;
  o.parse(argc,argv);
  if(o.size < 5) {
    std::cerr << "Error: Size must be at least 5" << std::endl;
    return 1;
  }

  init_knight(o.size);
  init_bishop(o.size);

  Example::run<CrowdedChessTable,DFS>(o);
  return 0;
}

// STATISTICS: example-any

