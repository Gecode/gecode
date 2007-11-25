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
  int k, n; /// Parameters to be given on the command line
  /// Initialize options for example with name \a s
  LangfordNumberOptions(const char* s, int k0, int n0) 
    : Options(s), k(k0), n(n0) {}
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 3)
      return;
    k = atoi(argv[1]);
    n = atoi(argv[2]);
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
 * \ingroup ExProblem
 */
class LangfordNumber : public Example {
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
    : k(opt.k), n(opt.n), y(this,k*n,1,n) {

    switch (opt.propagation()) {
    case PROP_REIFIED:
      {
        /// Position of values in sequence
        IntVarArgs p(k*n);
        for (int i=k*n; i--; )
          p[i].init(this,0,k*n-1);
        
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
        for (int i=n; i--; )
          for (int j=k-1; j--; )
            post(this, p[i*k+j] + (i+2) == p[i*k+j+1]);
        
        distinct(this, p, opt.icl());
        
        // Channel positions <-> values
        for (int i=n; i--; )
          for (int j=k; j--; )
            element(this, y, p[i*k+j], i+1);
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
          extensional(this, y, *ra+rv+(ra(v,v)+rv)(k-1,k-1)+*ra);
        }
      }
      break;
    case PROP_EXTENSIONAL_CHANNEL:
      {
        // Boolean variables for channeling
        BoolVarArgs b(k*n*n);
        for (int i=n*n*k; i--; )
          b[i].init(this,0,1);
        
        // Post channel constraints
        for (int i=n*k; i--; ) {
          BoolVarArgs c(n);
          for (int j=n; j--; ) 
            c[j]=b[i*n+j];
          channel(this, c, y[i], 1);
        }
        
        // For placing two numbers three steps apart, we construct the
        // regular expression 0*100010*, and apply it to the projection of
        // the sequence on the value.
        REG r0(0), r1(1);
        for (int v=1; v<=n; v++) {
          // Projection for value v
          BoolVarArgs c(k*n);
          for (int i = k*n; i--; )
            c[i] = b[i*n+(v-1)];
          extensional(this, c, *r0 + r1 + (r0(v,v) + r1)(k-1,k-1) + *r0);
        }
      }
      break;
    }
      
    // Symmetry breaking
    rel(this, y[0], IRT_LE, y[n*k-1]);
    
    // Branching
    branch(this, y, INT_VAR_SIZE_MIN, INT_VAL_MAX);
  }

  /// Print solution
  virtual void print(std::ostream& os){
    os << "\t" << y << std::endl;
  }

  /// Constructor for cloning \a l
  LangfordNumber(bool share, LangfordNumber& l)
    : Example(share, l), k(l.k), n(l.n) {
    y.update(this, share, l.y);

  }
  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new LangfordNumber(share, *this);
  }
};


/** \brief Main-function
 *  \relates LangfordNumber
 */
int 
main(int argc, char* argv[]) {
  LangfordNumberOptions opt("Langford Numbers",3,9);
  opt.icl(ICL_DOM);
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
  Example::run<LangfordNumber,DFS,LangfordNumberOptions>(opt);
  return 0;
}

// STATISTICS: example-any

