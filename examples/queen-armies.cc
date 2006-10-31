/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
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
#include "gecode/set.hh"
#include "gecode/minimodel.hh"


/** \name Constant sets for attacking queens.
 *
 * \relates QueenArmies
 */
IntSet *A;

/**
 * \brief %Example: Peacable co-existing armies of queens.
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
 * The smart version uses a custom branching implementing a heuristic
 * from the above paper, that helps speeding up the proof of
 * optimality.
 *
 * \ingroup Example
 *
 */
class QueenArmies : public Example {
public:
  const int n;
  SetVar U, ///< Set of un-attacked squares
    W; ///< Set of squares occupied by white queens
  BoolVarArray w, ///< The placement of the white queens
    b; ///< The placement of the black queens
  IntVar q; ///< The number of white queens placed.

  QueenArmies(const Options& o) :
    n(o.size),
    U(this, IntSet::empty, IntSet(0, n*n)),
    W(this, IntSet::empty, IntSet(0, n*n)),
    w(this, n*n, 0, 1),
    b(this, n*n, 0, 1),
    q(this, 0, n*n)
  {
    // Basic rules of the model
    for (int i = n*n; i--; ) {
      // w[i] means that no blacks are allowed on A[i]
      dom(this, U, SRT_DISJ, A[i], w[i]);
      // Make sure blacks and whites are disjoint.
      post(this, tt(!(w[i] && b[i])));
      // If i in U, then b[i] has a piece.
      dom(this, U, SRT_SUP, i, b[i]);
    }

    // Connect optimization variable to number of pieces
    linear(this, w, IRT_EQ, q);
    linear(this, b, IRT_GQ, q);

    // Connect cardinality of U to the number of black pieces.
    IntVar unknowns(this, 0, n*n);
    cardinality(this, U, unknowns);
    post(this, q <= unknowns);
    linear(this, b, IRT_EQ, unknowns);

    if (o.naive) {
      branch(this, w, BVAR_NONE, BVAL_MAX);
      branch(this, b, BVAR_NONE, BVAL_MAX);
    } else {
      QueenBranch::post(this);
      assign(this, b, AVAL_MAX);
    }
  }

  QueenArmies(bool share, QueenArmies& s)
    : Example(share,s), n(s.n)
  {
    U.update(this, share, s.U);
    W.update(this, share, s.W);
    w.update(this, share, s.w);
    b.update(this, share, s.b);
    q.update(this, share, s.q);
  }

  virtual Space*
  copy(bool share) {
    return new QueenArmies(share,*this);
  }

  void
  constrain(Space* s) {
    rel(this, q, IRT_GR, static_cast<QueenArmies*>(s)->q.val());
  }


  virtual void
  print(void) {
    std::cout << '\t';
    for (int i = 0; i < n*n; ++i) {
      if (w[i].val()) std::cout << "W";
      else if (b[i].val()) std::cout << "B";
      else std::cout << ".";
      if ((i+1)%n == 0) std::cout << std::endl << (i!=(n*n-1)?"\t":"");
    }
    std::cout << "Number of white queens: " << q << std::endl << std::endl;
  }

  /** \brief Custom branching for Peacable queens.
   *
   *  Custom branching that tries to place white queens so that they
   *  maximise the amount of un-attacked squares that become attacked.
   */
  class QueenBranch : Branching {
    mutable int pos;
    QueenBranch(Space* home)
      : Branching(home), pos(-1) {}
    QueenBranch(Space* home, bool share, QueenBranch& b)
      : Branching(home, share, b), pos(b.pos) {}

  public:
    virtual bool status(const Space* home) const {
      const QueenArmies *q = static_cast<const QueenArmies*>(home);
      int maxsize = -1;
      pos = -1;

      for (int i = q->n*q->n; i--; ) {
        if (q->w[i].assigned()) continue;
        IntSetRanges ai(A[i]);
        SetVarUnknownRanges qU(q->U);
        Iter::Ranges::Inter<IntSetRanges, SetVarUnknownRanges> r(ai, qU);
        int size = Iter::Ranges::size(r);
        if (size > maxsize) {
          maxsize = size;
          pos = i;
        }
      }
      if (pos == -1) return false;
      return true;
    }
    virtual BranchingDesc* description(const Space*) const {
      assert(pos != -1);
      return new PosValDesc<bool>(this, pos, true);
    }
    virtual ExecStatus commit(Space* home, const BranchingDesc* d, unsigned int a) {
      QueenArmies *q = static_cast<QueenArmies*>(home);
      const PosValDesc<bool> *pvd = static_cast<const PosValDesc<bool>*>(d);
      bool val = a == 0 ? pvd->val() : !pvd->val();
      return me_failed(Int::BoolView(q->w[pvd->pos()]).eq(q, val))
        ? ES_FAILED
        : ES_OK;
    }
    virtual Actor* copy(Space *home, bool share) {
      return new (home) QueenBranch(home, share, *this);
    }

    static void post(QueenArmies* home) {
      (void) new (home) QueenBranch(home);
    }
  };
};

/// Position of a piece in a square board
int pos(int i, int j, int n) {
  return i*n + j;
}

/** \brief Main-function
 *  \relates QueenArmies
 */
int
main(int argc, char** argv) {
  Options o("QueenArmies");
  o.size      = 6;
  o.naive     = false;
  o.solutions = 0;
  o.parse(argc,argv);

  // Set up the A-sets
  // A[i] will contain the values attacked by a queen at position i
  int n = o.size;
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

  Example::run<QueenArmies,BAB>(o);
  return 0;
}

// STATISTICS: example-any
