/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Contributing author:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *     Mikael Lagerkvist, 2006
 *     Christian Schulte, 2007
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

/**
 * \brief Options taking two additional parameters
 *
 * \relates LangfordNumber
 */
class LangfordNumberOptions : public Options {
public:
  int n, k; /// Parameters to be given on the command line
  /// Initialize options for example with name \a s
  LangfordNumberOptions(const char* s, int n0, int k0) 
    : Options(s), n(n0), k(k0) {}
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 3)
      return;
    n = atoi(argv[1]);
    k = atoi(argv[2]);
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(unsigned int) default: " << n << std::endl
              << "\t\tparameter n" << std::endl
              << "\t(unsigned int) default: " << k << std::endl
              << "\t\tparameter k" << std::endl;
  }
};

/// Propagation to use for model
enum {
  PROP_REIFIED,  ///< Use only binary disequality constraints
  PROP_REGULAR   ///< Use regular constraints
};

/**
 * \brief %Example: Langford's number problem (naive version)
 *
 * See problem 24 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 */
class LangfordNumberReified : public Example {
protected:
  int n, k;        ///< Problem parameters
  IntVarArray pos; ///< Position of values in sequence
  IntVarArray y;   ///< Sequence variables

public:
  /// Returns position of \f$j\f$-th occurence of value \f$ i + 1\f$
  IntVar& p(int i, int j) {
    return pos[i * k + j];
  }
  /// Construct model
  LangfordNumberReified(const LangfordNumberOptions& opt)
    : n(opt.n), k(opt.k), pos(this,k*n,0,k*n-1), y(this,k*n,1,n) {

    /* 
     * The occurences of value v in the Langford sequence are v numbers apart.
     *
     * Let \#(i, v) denote the position of the i-th occurence of 
     * value v in the Langford Sequence. Then the constraint are posted
     * that
     * \f$ \forall i, j \in \{1, \dots, k\}, i \neq j:
     *     \forall v \in \{1, \dots, n\}: \#(i, v) + (v + 1) = \#(j, v)\f$
     *
     */
    for (int i = 0; i < n; i++)
      for (int j = 0; j < k - 1; j++)
        post(this, p(i,j) + ((i+1)+1) == p(i,j+1));

    distinct(this, pos, opt.icl());

    // Symmetry breaking
    rel(this, y[0], IRT_LE, y[n*k-1]);

    // Channeling positions <-> values
    for (int i = 0; i < n; i++)
      for (int j = 0; j < k; j++)
        element(this, y, p(i,j), i+1);

    // Branching
    branch(this, pos, BVAR_SIZE_MIN, BVAL_MIN);
  }

  /// Print solution
  virtual void print(void){
    std::cout << "\t";
    for (int i = 0; i < y.size(); ++i)
      std::cout << y[i] << " ";
    std::cout << std::endl;
  }

  /// Constructor for cloning \a l
  LangfordNumberReified(bool share, LangfordNumberReified& l)
    : Example(share, l), n(l.n), k(l.k) {
    pos.update(this, share, l.pos);
    y.update(this, share, l.y);

  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new LangfordNumberReified(share, *this);
  }
};


/**
 * \brief %Example: Langford's number problem (regular version)
 *
 * See problem 24 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 */
class LangfordNumberRegular : public Example {
protected:
  int n, k;        ///< Problem parameters
  IntVarArray y;   ///< Sequence variables

public:
  /// Construct model
  LangfordNumberRegular(const LangfordNumberOptions& opt)
    : n(opt.n), k(opt.k), y(this,k*n,1,n) {
    // For placing two numbers 3 three steps apart, we construct the
    // regular expression 0*100010*, and apply it to the projection of
    // the sequence on the value.
    for (int i = 1; i <= n; ++i) {
      // Start of regular expression
      REG reg = *REG(0) + REG(1);
      // For each next number to place
      for (int ki = 1; ki < k; ++ki) {
        reg = reg + REG(0)(i, i) + REG(1);
      }
      // End of expression
      reg = reg + *REG(0);
      // Projection for value i
      BoolVarArgs cv(k*n);
      for (int cvi = k*n; cvi--; )
        cv[cvi] = post(this, ~(y[cvi] == i));
      DFA dfa = reg;
      regular(this, cv, dfa);
    } 

    // Symmetry breaking
    post(this, y[0] < y[n*k - 1]);

    // Branching
    branch(this, y, BVAR_SIZE_MIN, BVAL_MAX);
  }

  /// Print solution
  virtual void print(void){
    std::cout << "\t";
    for (int i = 0; i < y.size(); ++i)
      std::cout << y[i] << " ";
    std::cout << std::endl;
  }
  /// Constructor for cloning \a l
  LangfordNumberRegular(bool share, LangfordNumberRegular& l)
    : Example(share, l), n(l.n), k(l.k) {
    y.update(this, share, l.y);
  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new LangfordNumberRegular(share, *this);
  }
};

/** \brief Main-function
 *  \relates LangfordNumber
 */
int 
main(int argc, char* argv[]) {
  LangfordNumberOptions opt("Langford Numbers",8,2);
  opt.icl(ICL_DOM);
  opt.propagation(PROP_REIFIED);
  opt.propagation(PROP_REIFIED, "reified");
  opt.propagation(PROP_REGULAR, "regular");
  opt.parse(argc, argv);
  if (opt.k < 1) {
    std::cerr << "k must be at least 1!" << std::endl;
    return 1;
  }
  if (opt.k > opt.n) {
    std::cerr << "n must be at least k!" << std::endl;
    return 1;
  }
  if (opt.propagation() == PROP_REIFIED)
    Example::run<LangfordNumberReified,DFS,LangfordNumberOptions>(opt);
  else
    Example::run<LangfordNumberRegular,DFS,LangfordNumberOptions>(opt);
  return 0;
}

// STATISTICS: example-any

