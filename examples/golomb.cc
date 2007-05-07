/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
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

/**
 * \brief %Example: Finding optimal %Golomb rulers
 *
 * The script makes use of two lower bounds taken from:
 *   Barbara Smith, Kostas Stergiou, Toby Walsh,
 *   Modelling the Golomb Ruler Problem.
 *   In IJCAI 99 Workshop on Non-binary Constraints,
 *   1999.
 *
 * \ingroup ExProblem
 *
 */
class Golomb : public Example {
protected:
  /// Number of marks
  const int n;
  /// Array for ruler marks
  IntVarArray m;
public:
  /// Return index for mark difference between mark \a i and mark \a j
  int
  diag(int i, int j) {
    return (i*(2*n-i-1)) / 2 + j - i - 1;
  }

  /// Actual model
  Golomb(const Options& opt)
    : n(opt.size), m(this,n,0,n*n) {
    const int dn = (n*n-n)/2;

    IntVarArgs d(dn);

    // Assume first mark to be zero
    rel(this, m[0], IRT_EQ, 0);

    // Setup difference constraints
    for (int j=1; j<n; j++)
      d[diag(0,j)] = m[j];
    for (int i=1; i<n-1; i++)
      for (int j=i+1; j<n; j++)
        d[diag(i,j)] = minus(this, m[j], m[i]);

    // Order marks
    rel(this, m, IRT_LE);

    if (opt.naive) {
      // d[diag(i,j)] must be at least sum of first j-i integers
      for (int i=0; i<n; i++)
        for (int j=i+1; j<n; j++)
          rel(this, d[diag(i,j)], IRT_GQ, (j-i)*(j-i+1)/2);
    } else {
      static const int length[] = {
        // Length 0-9
        0,0,1,3,6,11,17,25,34,44,
        // Length 10-
        55,72,85,106,127};

      // Marks from i to j must be ruler of length j-1+i
      for (int i=0; i<n; i++)
        for (int j=i+1; j<n; j++)
          rel(this, d[diag(i,j)], IRT_GQ, length[j-i+1]);
    }

    distinct(this, d, opt.icl);

    if (n > 2)
      rel(this, d[diag(0,1)], IRT_LE, d[diag(n-2,n-1)]);

    branch(this, m, BVAR_NONE, BVAL_MIN);
  }

  /// Add constraint for next better solution
  void
  constrain(Space* s) {
    rel(this, m[n-1], IRT_LE, static_cast<Golomb*>(s)->m[n-1].val());
  }

  /// Print solution
  virtual void
  print(void) {
    std::cout << "\tm[" << n << "] = {";
    for (int i = 0; i < n; i++)
      std::cout << m[i] << ((i<n-1)?",":"};\n");
  }

  /// Constructor for cloning \a s
  Golomb(bool share, Golomb& s)
    : Example(share,s), n(s.n) {
    m.update(this, share, s.m);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new Golomb(share,*this);
  }

};

/** \brief Main-function
 *  \relates Golomb
 */
int
main(int argc, char** argv) {
  Options o("Golomb");
  o.solutions = 0;
  o.size      = 5;
  o.icl       = ICL_DOM;
  o.naive     = true;
  o.parse(argc,argv);
  if (o.size > 0)
    Example::run<Golomb,BAB>(o);
  return 0;
}

// STATISTICS: example-any

