/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2005
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

/**
 * \brief %Example: %All-Interval Series
 *
 * This is  prob007 in the category "combinatorial mathematics" of
 * http://www.csplib.org/.
 *
 * \ingroup Example
 */

#include "examples/support.hh"
#include "gecode/minimodel.hh"

class AllInterval : public Example {
private:
  int n;
  IntVarArray x;
  IntVarArray d;
public:

  /**
   * \brief Constrain x to be a permuation of \f$ S_n = \{0, \dots, n - 1\} \f$
   *
   * \f$ \forall i, j\in S_n, i\neq j: x_i \neq x_j\f$
   */
  void adiff_sn(Space* home, IntVarArray& x) {

    // S_n = \{0, \dots, n - 1\}
    IntVarArray sn(home, n);

    for (int i = n; i--; ) {
      sn[i].init(home, i, i);
    }

    sortedness(home, x, sn);
  }

  /**
   * \brief Constrain x to be a permuation of \f$ S^*_n = S_n \setminus \{0\} \f$
   *
   * \f$ \forall i, j\in S^*_n, i\neq j: x_i \neq x_j\f$
   */
  void adiff_sn_star(Space* home, IntVarArray& x) {
    // S^*_n = S_n \setminus \{0\} = \{1, \dots, n - 1\}
    IntVarArray snstar(home, n - 1);

    for (int i = n - 1; i--; ) {
      snstar[i].init(home, i + 1, i + 1);
    }
    sortedness(home, x, snstar);
  }

  /**
   * \brief Constrain d to hold the differences of neighboured x-variables
   *
   * \f$ \forall i \in S^*_n: d_i = |x_i - x_{i - 1}|\f$
   */
  void difference(Space* home, IntVarArray& x, IntVarArray& d) {

    IntVarArray diff(this, n - 1);
    for (int i = 0; i < n - 1; i++) {
      diff[i].init(this, 1 - n, n - 1);
    }

    for (int i = 0; i < n - 1; i++) {
      post(this, x[i + 1] - x[i] == diff[i]);
      abs(this, diff[i], d[i]);
    }
  }

  /**
   * \name Static Symmetrie Breaking
   *
   * This is taken from: I.P. Gent and I. McDonald and B.M. Smith,
   * Conditional Symmetry in the All-Interval Series Problem,
   * Proceedings of the Third International Workshop
   * on Symmetry in Constraint Satisfaction Problems, pages 55-65, 2003.
   *
   */

  //@{
  /*
   * \brief Break negation symmetry
   *
   *  As we are intereseted in the absolute value of the difference of neighbored values in x
   *  the following holds:
   *  If the sequence \f$ \sigma = \langle s_0, \dots, s_{n - 1} \rangle \f$ is a solution to
   *  the problem so is the negated sequence
   *  \f$ \phi = \langle (n - 1 - s_0), \dots, (n - 1) - s_{n - 1}\f$. \n
   */
  void break_negation(Space* home, IntVarArray& x) {
    rel(home, x[0], IRT_LE, x[1]);
  }

  /*
   * \brief Break reveres symmetry
   *
   *  If the sequence \f$ \sigma = \langle s_0, \dots, s_{n - 1} \rangle \f$ is a solution
   *  so is the reverse sequence \f$\sigma^{-1} = \langle s_{n - 1}, \dots, s_0 \rangle \f$
   */

  void break_reversal(Space* home, IntVarArray& d) {
    rel(home, d[0], IRT_LE, d[n - 2]);
  }
  //@}

  AllInterval(const Options& op) :
    n(op.size),
    x(this, n),
    d(this, n - 1) {

    IntSet dom_zn   (0, n - 1);
    IntSet dom_zns  (1, n - 1);

    // initialization of the problem variables
    for (int i = 0; i < n; i++) {
      x[i].init(this, dom_zn);
      if (i < n - 1)
	d[i].init(this, dom_zns);
    }

    difference(this, x, d);

    // breaks negation
    break_negation(this, x);

    // breaks reversal
    break_reversal(this, d);


    adiff_sn(this, x);
    adiff_sn_star(this, d);

    branch(this, x, BVAR_SIZE_MIN, BVAL_MIN);

  }

  AllInterval(bool share, AllInterval& a)
    : Example(share, a),
      n(a.n) {
    x.update(this, share, a.x);
    d.update(this, share, a.d);
  }

  virtual Space*
  copy(bool share) {
    return new AllInterval(share, *this);
  }

  virtual void print(void){
    std::cout << "Sol:\n";
    std::cout << "x: ";
    for (int i = 0; i < n; i++) {
      std::cout << x[i] << " ";
    }
    std::cout <<"\n";

    std::cout << "d: ";
    for (int i = 0; i < n - 1; i++) {
      std::cout << d[i] << " ";
    }
    std::cout <<"\n";

  }
};


int main(int argc, char** argv){
  Options opt("All-Interval Series ");
  opt.size = 12;
  opt.parse(argc, argv);
  if (opt.size < 2) {
    std::cerr << "n must be at least 2!" << std::endl;
    return -1;
  }
  Example::run<AllInterval, DFS>(opt);
  return 0;
}

// STATISTICS: example-any

