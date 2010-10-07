/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2009
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

using namespace Gecode;

/**
 * \brief %Options for %EFPA problems
 *
 * \relates EFPA
 */
class EFPAOptions : public Options {
private:
  Driver::UnsignedIntOption _v;   ///< Parameter v
  Driver::UnsignedIntOption _q;   ///< Parameter q
  Driver::UnsignedIntOption _l;   ///< Parameter lambda
  Driver::UnsignedIntOption _d;   ///< Parameter d
  Driver::StringOption _permutation; ///< Use permutation constraints if d=4

public:
  /// Initialize options for example with name \a s
  EFPAOptions(const char* s,
              int v0 = 5, int q0 = 3, int lambda0 = 2, int d0 = 4)
    : Options(s),
      _v("-v", "number of sequences",                        v0     ),
      _q("-q", "number of symbols",                          q0     ),
      _l("-l", "sets of symbols per sequence (lambda)",      lambda0),
      _d("-d", "Hamming distance between sequences",         d0     ),
      _permutation("-permutation", "use permutation constraints if d=4", 
                   false)
  {
    // Add options
    add(_d);
    add(_l);
    add(_q);
    add(_v);
    add(_permutation);
    add(_symmetry);

    // Add permutation options
    _permutation.add(true,  "full" );
    _permutation.add(false, "none");
    // Add symmetry options
    _symmetry.add(true,  "true" );
    _symmetry.add(false, "false");
  }
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
  }
  /// Get v, number of sequences
  int v(void) const { return _v.value(); }
  /// Get q, number of symbols
  int q(void) const { return _q.value(); }
  /// Get lambda, sets of symbols per sequence
  int l(void) const { return _l.value(); }
  /// Get d, Hamming distance between sequences
  int d(void) const { return _d.value(); }

  /// Whether to use permutation constraints. Only active if d=4
  bool permutation(void) const { return d() == 4 && _permutation.value(); }
  /// Whether to use symmetry breaking.
  bool symmetry(void) const { return _symmetry.value(); }
};


/**
 * \brief %Example: Equidistant Frequency Permutation Arrays
 *
 * This example solves instances of the equidistant frequency
 * permutation arrays problem.
 *
 * The model of the problem is mostly taken from "Modelling
 * Equidistant Frequency Permutation Arrays in Constraints", by Ian
 * P. Gent, Paul McKay, Peter Nightingale, and Sophie Huczynska. It
 * implements the non-Boolean model without SAC.
 *
 * \ingroup Example
 *
 */
class EFPA : public Script {
protected:
  int v; ///< Number of sequences
  int q; ///< Number of symbols
  int l; ///< Number of sets of symbols for a sequence (\f$\lambda\f$)
  int d; ///< Hamming distance between any pair of sequences
  int n; ///< Length of sequence (\f$q\cdot\lambda\f$)
  int nseqpair;  ///< Number of sequence pairs (\f$\frac{v(v-1)}{2}\f$)
  IntVarArray  c; ///< Variables for sequences
  BoolVarArray diff; ///< Differences between sequences

public:
  /// Actual model
  EFPA(const EFPAOptions& opt)
    : v(opt.v()),
      q(opt.q()),
      l(opt.l()),
      d(opt.d()),
      n(q*l),
      nseqpair((v*(v-1))/2),
      c(*this, n*v, 1,q),
      diff(*this, n*nseqpair, 0, 1)
  {
    // Matrix access
    // q*lambda=n columns, and v rows
    Matrix<IntVarArray> cm(c, n, v);
    // q*lambda=n columns, and nseqpair rows
    Matrix<BoolVarArray> diffm(diff, n, nseqpair);

    // Counting symbols in rows
    {
      IntArgs values(q);
      for (int i = q; i--; ) values[i] = i+1;
      IntSet cardinality(l, l);
      for (int i = v; i--; )
        count(*this, cm.row(i), cardinality, values, opt.icl());
    }
    
    // Difference variables
    {
      int nseqi = 0;
      for (int a = 0; a < v; ++a) {
        for (int b = a+1; b < v; ++b) {
          for (int i = n; i--; ) {
            rel(*this, cm(i, a), IRT_NQ, cm(i, b), diffm(i, nseqi));
          }
          ++nseqi;
        }
      }
      assert(nseqi == nseqpair);
    }

    // Counting the Hamming difference
    {
      for (int i = nseqpair; i--; ) {
        linear(*this, diffm.row(i), IRT_EQ, d);
      }
    }

    // Symmetry breaking
    if (opt.symmetry()) {
      IntRelType row_less = d==0 ? IRT_EQ : IRT_LE;
      // order rows
      for (int r = 0; r<v-1; ++r) {
        rel(*this, cm.row(r), row_less, cm.row(r+1));
      }
      // order columns
      for (int c = 0; c<n-1; ++c) {
        rel(*this, cm.col(c), IRT_LQ, cm.col(c+1));
      }
      // Set first row according to symmetry breaking
      int color = 1;
      int ncolor = 0;
      for (int c = 0; c < n; ++c) {
        rel(*this, cm(c, 0), IRT_EQ, color);
        if (++ncolor == l) {
          ncolor = 0;
          ++color;
        }
      }
    }

    // Permutation constraints
    if (opt.permutation()) {
      const int k[][4] = { // inverse indexing of the permutation
        {0, 1, 3, 2}, // cform == 0, ((1, 2)(3, 4))
        {1, 2, 3, 0}, // cform == 1, ((1, 2, 3, 4))
      };
      assert(d == 4);
      // Constraint on each pair of rows
      for (int r1 = 0; r1 < v; ++r1) {
        for (int r2 = r1+1; r2 < v; ++r2) {
          IntVarArgs row1 = cm.row(r1);
          IntVarArgs row2 = cm.row(r2);
          // Perm is the 
          IntVarArgs perm(d);
          for (int i = d; i--; ) perm[i] = IntVar(*this, 0, n-1);
          // cform is the cycle-form of the permutation
          IntVar  cform(*this, 0, 1);
          BoolVar cformb = channel(*this, cform);
          
          /* Permutation mapping*/
          // Values from row1...
          IntVarArgs _p(2*d);
          for (int i = 2*d; i--; ) _p[i] = IntVar(*this, 1, q);
          Matrix<IntVarArgs> p(_p, d, 2);
          for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < d; ++j) {
              element(*this, row1, perm[k[i][j]], p(j, i));
            }
          }

          // ...into values in row2
          for (int i = 0; i < d; ++i) {
            IntVar index(*this, 0, 2*d);
            rel(*this, cform*d + i == index);
            IntVar value(*this, 1, q);
            element(*this, _p, index, value);
            element(*this, row2, perm[i], value);
          }

          /* Rows r1 and r2 are equal at indices not in perm */
          // uses Boolean representations pib for perm[i]
          BoolVarArgs p1b(*this, n, 0, 1);
          channel(*this, p1b, perm[0]);
          BoolVarArgs p2b(*this, n, 0, 1);
          channel(*this, p2b, perm[1]);
          BoolVarArgs p3b(*this, n, 0, 1);
          channel(*this, p3b, perm[2]);
          BoolVarArgs p4b(*this, n, 0, 1);
          channel(*this, p4b, perm[3]);
          for (int i = n; i--; ) {
            // No perm-variable uses i is equivalent to the reows
            // being equal at i
            rel(*this, (!p1b[i] && !p2b[i] && !p3b[i] && !p4b[i]) ==
                       (row1[i] == row2[i]));
          }
          
          /* Constraints for fixing the permutation */
          // Common non-equality constraints - derangements
          rel(*this, perm[0], IRT_NQ, perm[1]);
          rel(*this, perm[2], IRT_NQ, perm[3]);
          // Conditional non-equality constraints - derangment of cform 1
          // Implements distinct(*this, perm, cformb);
          rel(*this, perm[0], IRT_NQ, perm[2], cformb);
          rel(*this, perm[0], IRT_NQ, perm[3], cformb);
          rel(*this, perm[1], IRT_NQ, perm[2], cformb);
          rel(*this, perm[1], IRT_NQ, perm[3], cformb);
          // Common ordering-constraints - symmetry breaking
          rel(*this, perm[0], IRT_LE, perm[1]);
          rel(*this, perm[0], IRT_LE, perm[2]);
          rel(*this, perm[0], IRT_LE, perm[3]);
          // Conditional ordering constraint - symmetry breaking for cform 0
          rel(*this, (!cformb) >> (perm[2] < perm[3]));
        }
      }
    }

    branch(*this, c, INT_VAR_NONE, INT_VAL_MIN);
  }

  /// Print instance and solution
  virtual void
  print(std::ostream& os) const {
    Matrix<IntVarArray> cm(c, n, v);
    for (int i = 0; i < v; ++i) {
      IntVarArgs r = cm.row(i);
      os << r << std::endl;
    }
    os << std::endl;
  }

  /// Constructor for cloning \a s
  EFPA(bool share, EFPA& s) 
    : Script(share,s),
      v(s.v),
      q(s.q),
      l(s.l),
      d(s.d),
      n(s.n),
      nseqpair(s.nseqpair)
  {
    c.update(*this, share, s.c);
    diff.update(*this, share, s.diff);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new EFPA(share,*this);
  }
};

/** \brief Main-function
 *  \relates EFPA
 */
int
main(int argc, char* argv[]) {
  EFPAOptions opt("Equidistant Frequency Permutation Arrays");
  opt.icl(ICL_DOM);
  opt.parse(argc,argv);

  Script::run<EFPA,DFS,EFPAOptions>(opt);
  return 0;
}

// STATISTICS: example-any
