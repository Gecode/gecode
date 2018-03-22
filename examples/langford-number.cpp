/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
 *     Mikael Lagerkvist, 2006
 *     Christian Schulte, 2007
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
 * \brief Options taking two additional parameters
 *
 * \relates LangfordNumber
 */
class LangfordNumberOptions : public Options {
public:
  int k, n; /// Parameters to be given on the command line
  /// Initialize options for example with name \a s
  LangfordNumberOptions(const char* s, int k0, int n0)
    : Options(s), k(k0), n(n0) {}
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

/**
 * \brief %Example: Langford's number problem
 *
 * See problem 24 at http://www.csplib.org/.
 *
 * \ingroup Example
 */
class LangfordNumber : public Script {
protected:
  int k, n;      ///< Problem parameters
  IntVarArray y; ///< Sequence variables

public:
  /// Propagation to use for model
  enum {
    PROP_REIFIED,            ///< Use reified constraints
    PROP_EXTENSIONAL,        ///< Use extensional constraints
    PROP_EXTENSIONAL_CHANNEL ///< Use extensional and channel constraints
  };
  /// Construct model
  LangfordNumber(const LangfordNumberOptions& opt)
    : Script(opt), k(opt.k), n(opt.n), y(*this,k*n,1,n) {

    switch (opt.propagation()) {
    case PROP_REIFIED:
      {
        // Position of values in sequence
        IntVarArgs pv(*this,k*n,0,k*n-1);
        Matrix<IntVarArgs> p(pv,n,k);

        /*
         * The occurences of v in the Langford sequence are v numbers apart.
         *
         * Let \#(i, v) denote the position of the i-th occurence of
         * value v in the Langford Sequence. Then
         *
         * \f$ \forall i, j \in \{1, \dots, k\}, i \neq j:
         *     \forall v \in \{1, \dots, n\}: \#(i, v) + (v + 1) = \#(j, v)\f$
         *
         */
        for (int i=0; i<n; i++)
          for (int j=0; j<k-1; j++)
            rel(*this, p(i,j)+i+2 == p(i,j+1));

        distinct(*this, pv, opt.ipl());

        // Channel positions <-> values
        for (int i=0; i<n; i++)
          for (int j=0; j<k; j++)
            element(*this, y, p(i,j), i+1);
      }
      break;
    case PROP_EXTENSIONAL:
      {
        IntArgs a(n-1);
        for (int v=2; v<=n; v++)
          a[v-2]=v;
        for (int v=1; v<=n; v++) {
          // Construct regular expression for all symbols but v
          if (v > 1)
            a[v-2]=v-1;
          REG ra(a), rv(v);
          extensional(*this, y, *ra+rv+(ra(v,v)+rv)(k-1,k-1)+*ra);
        }
      }
      break;
    case PROP_EXTENSIONAL_CHANNEL:
      {
        // Boolean variables for channeling
        BoolVarArgs bv(*this,k*n*n,0,1);
        Matrix<BoolVarArgs> b(bv,k*n,n);

        // Post channel constraints
        for (int i=0; i<n*k; i++)
          channel(*this, b.col(i), y[i], 1);

        // For placing two numbers three steps apart, we construct the
        // regular expression 0*100010*, and apply it to the projection of
        // the sequence on the value.
        REG r0(0), r1(1);
        for (int v=1; v<=n; v++)
          extensional(*this, b.row(v-1),
                      *r0 + r1 + (r0(v,v) + r1)(k-1,k-1) + *r0);
      }
      break;
    }

    // Symmetry breaking
    rel(*this, y[0], IRT_LE, y[n*k-1]);

    // Branching
    branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
  }

  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\t" << y << std::endl;
  }

  /// Constructor for cloning \a l
  LangfordNumber(LangfordNumber& l)
    : Script(l), k(l.k), n(l.n) {
    y.update(*this, l.y);

  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new LangfordNumber(*this);
  }
};


/** \brief Main-function
 *  \relates LangfordNumber
 */
int
main(int argc, char* argv[]) {
  LangfordNumberOptions opt("Langford Numbers",3,9);
  opt.ipl(IPL_DOM);
  opt.propagation(LangfordNumber::PROP_EXTENSIONAL_CHANNEL);
  opt.propagation(LangfordNumber::PROP_REIFIED,
                  "reified");
  opt.propagation(LangfordNumber::PROP_EXTENSIONAL,
                  "extensional");
  opt.propagation(LangfordNumber::PROP_EXTENSIONAL_CHANNEL,
                  "extensional-channel");
  opt.parse(argc, argv);
  if (opt.k < 1) {
    std::cerr << "k must be at least 1!" << std::endl;
    return 1;
  }
  if (opt.k > opt.n) {
    std::cerr << "n must be at least k!" << std::endl;
    return 1;
  }
  Script::run<LangfordNumber,DFS,LangfordNumberOptions>(opt);
  return 0;
}

// STATISTICS: example-any

