/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
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
#include <gecode/set.hh>

using namespace Gecode;

/** \name Constant sets for attacking queens.
 *
 * \relates QueenArmies
 */
IntSet* A;

/**
 * \brief %Example: Peaceable co-existing armies of queens
 *
 * The goal of this problem is to place as many white and black queens
 * on a chess-board without any two queens of different color
 * attacking each other. The number of  black queens should
 * be greater than or equal to the number of white queens.
 *
 * This model is based on the one presented in "Models and Symmetry
 * Breaking for 'Peaceable Armies of Queens'", by Barbara M. Smith, Karen
 * E. Petrie, and Ian P. Gent.
 *
 * The smart version uses a custom brancher implementing a heuristic
 * from the above paper, that helps speeding up the proof of
 * optimality.
 *
 * \ingroup Example
 *
 */
class QueenArmies : public IntMaximizeScript {
public:
  const int n;
  SetVar U, ///< Set of un-attacked squares
    W; ///< Set of squares occupied by white queens
  BoolVarArray w, ///< The placement of the white queens
    b; ///< The placement of the black queens
  IntVar q; ///< The number of white queens placed.

  /// Branching to use for model
  enum {
    BRANCH_NAIVE,   ///< Choose variables left to right
    BRANCH_SPECIFIC ///< Choose variable with problem specific strategy
  };

  /// Constructor
  QueenArmies(const SizeOptions& opt) :
    IntMaximizeScript(opt),
    n(opt.size()),
    U(*this, IntSet::empty, IntSet(0, n*n)),
    W(*this, IntSet::empty, IntSet(0, n*n)),
    w(*this, n*n, 0, 1),
    b(*this, n*n, 0, 1),
    q(*this, 0, n*n)
  {
    // Basic rules of the model
    for (int i = n*n; i--; ) {
      // w[i] means that no blacks are allowed on A[i]
      rel(*this, w[i] == (U || A[i]));
      // Make sure blacks and whites are disjoint.
      rel(*this, !w[i] || !b[i]);
      // If i in U, then b[i] has a piece.
      rel(*this, b[i] == (singleton(i) <= U));
    }

    // Connect optimization variable to number of pieces
    linear(*this, w, IRT_EQ, q);
    linear(*this, b, IRT_GQ, q);

    // Connect cardinality of U to the number of black pieces.
    IntVar unknowns = expr(*this, cardinality(U));
    rel(*this, q <= unknowns);
    linear(*this, b, IRT_EQ, unknowns);

    if (opt.branching() == BRANCH_NAIVE) {
      branch(*this, w, BOOL_VAR_NONE(), BOOL_VAL_MAX());
      branch(*this, b, BOOL_VAR_NONE(), BOOL_VAL_MAX());
    } else {
      QueenBranch::post(*this);
      assign(*this, b, BOOL_ASSIGN_MAX());
    }
  }
  /// Constructor for cloning
  QueenArmies(QueenArmies& s)
    : IntMaximizeScript(s), n(s.n) {
    U.update(*this, s.U);
    W.update(*this, s.W);
    w.update(*this, s.w);
    b.update(*this, s.b);
    q.update(*this, s.q);
  }
  /// Return copy during cloning
  virtual Space*
  copy(void) {
    return new QueenArmies(*this);
  }
  /// Return solution cost
  virtual IntVar cost(void) const {
    return q;
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << '\t';
    for (int i = 0; i < n*n; ++i) {
      if (w[i].assigned() && w[i].val()) os << "W";
      else if (b[i].assigned() && b[i].val()) os << "B";
      else if (!w[i].assigned() && !b[i].assigned()) os << " ";
      else os << ".";
      if ((i+1)%n == 0) os << std::endl << (i!=(n*n-1)?"\t":"");
    }
    os << "Number of white queens: " << q << std::endl << std::endl;
  }

  /** \brief Custom brancher for Peacable queens
   *
   *  Custom brancher that tries to place white queens so that they
   *  maximise the amount of un-attacked squares that become attacked.
   *
   * \relates QueenArmies
   */
  class QueenBranch : public Brancher {
  private:
    /// Cache of last computed decision
    mutable int start;
    /// Choice
    class Choice : public Gecode::Choice {
    public:
      /// Position of variable
      int pos;
      /// Value of variable
      bool val;
      /** Initialize choice for brancher \a b, position \a pos0,
       *  and value \a val0.
       */
      Choice(const Brancher& b, int pos0, bool val0)
        : Gecode::Choice(b,2), pos(pos0), val(val0) {}
      /// Archive into \a e
      virtual void archive(Archive& e) const {
        Gecode::Choice::archive(e);
        e << pos << val;
      }
    };

    /// Construct brancher
    QueenBranch(Home home)
      : Brancher(home), start(0) {}
    /// Constructor for cloning
    QueenBranch(Space& home, QueenBranch& b)
      : Brancher(home, b), start(b.start) {}

  public:
    /// Check status of brancher, return true if alternatives left.
    virtual bool status(const Space& home) const {
      const QueenArmies& q = static_cast<const QueenArmies&>(home);
      for (int i = start; i < q.n*q.n; ++i)
        if (!q.w[i].assigned()) {
          start = i;
          return true;
        }
      // No non-assigned orders left
      return false;
    }
    /// Return choice
    virtual Gecode::Choice* choice(Space& home) {
      const QueenArmies& q = static_cast<const QueenArmies&>(home);
      int maxsize = -1;
      int pos = -1;

      for (int i = start; i < q.n*q.n; ++i) {
        if (q.w[i].assigned()) continue;
        IntSetRanges ai(A[i]);
        SetVarUnknownRanges qU(q.U);
        Iter::Ranges::Inter<IntSetRanges, SetVarUnknownRanges> r(ai, qU);
        int size = Iter::Ranges::size(r);
        if (size > maxsize) {
          maxsize = size;
          pos = i;
        }
      }

      assert(pos != -1);
      return new Choice(*this, pos, true);
    }
    /// Return choice
    virtual Choice* choice(const Space&, Archive& e) {
      int pos, val;
      e >> pos >> val;
      return new Choice(*this, pos, val);
    }
    /** \brief Perform commit for choice \a _c and
     * alternative \a a.
     */
    virtual ExecStatus commit(Space& home, const Gecode::Choice& _c,
                              unsigned int a) {
      QueenArmies& q = static_cast<QueenArmies&>(home);
      const Choice& c = static_cast<const Choice&>(_c);
      bool val = (a == 0) ? c.val : !c.val;
      return me_failed(Int::BoolView(q.w[c.pos]).eq(q, val))
        ? ES_FAILED
        : ES_OK;
    }
    /// Print explanation
    virtual void print(const Space&, const Gecode::Choice& _c,
                       unsigned int a,
                       std::ostream& o) const {
      const Choice& c = static_cast<const Choice&>(_c);
      bool val = (a == 0) ? c.val : !c.val;
      o << "w[" << c.pos << "] = " << val;
    }
    /// Copy brancher during cloning
    virtual Actor* copy(Space& home) {
      return new (home) QueenBranch(home, *this);
    }
    /// Post brancher
    static void post(QueenArmies& home) {
      (void) new (home) QueenBranch(home);
    }
    /// Delete brancher and return its size
    virtual size_t dispose(Space&) {
      return sizeof(*this);
    }
  };
};

/** \brief Position of a piece in a square board.
 *
 * \relates QueenArmies
 */
int pos(int i, int j, int n) {
  return i*n + j;
}

/** \brief Main-function
 *  \relates QueenArmies
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("QueenArmies");
  opt.size(6);
  opt.branching(QueenArmies::BRANCH_SPECIFIC);
  opt.branching(QueenArmies::BRANCH_NAIVE, "naive");
  opt.branching(QueenArmies::BRANCH_SPECIFIC, "specific");
  opt.solutions(0);
  opt.parse(argc,argv);

  // Set up the A-sets
  // A[i] will contain the values attacked by a queen at position i
  int n = opt.size();
  A = new IntSet[n*n];
  int *p = new int[std::max(n*n, 25)];
  int pn = 0;
  for (int i = n; i--; ) {
    for (int j = n; j--; ) {
      int dir[][2] = {
        { 0,  1},
        { 1,  1},
        { 1,  0},
        { 0, -1},
        {-1, -1},
        {-1,  0},
        { 1, -1},
        {-1,  1}
      };
      p[pn++] = pos(i, j, n);
      for (int k = 8; k--; ) {
        for (int l = 0; l < n
               && 0 <= (i+l*dir[k][0]) && (i+l*dir[k][0]) < n
               && 0 <= (j+l*dir[k][1]) && (j+l*dir[k][1]) < n; ++l) {
          p[pn++] = pos(i+l*dir[k][0], j+l*dir[k][1], n);
        }
      }

      A[pos(i, j, n)] = IntSet(p, pn);

      pn = 0;
    }
  }
  delete [] p;

  IntMaximizeScript::run<QueenArmies,BAB,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
