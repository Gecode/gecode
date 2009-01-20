/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Bugfixes provided by:
 *     Olof Sivertsson <olsi0767@student.uu.se>
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

/**
 * \brief Options for BIBD problems
 *
 */
class BIBDOptions : public Options {
public:
  int v, k, lambda; ///< Parameters to be given on command line
  int b, r;         ///< Derived parameters
  /// Derive additional parameters
  void derive(void) {
    b = (v*(v-1)*lambda)/(k*(k-1));
    r = (lambda*(v-1)) / (k-1);
  }
  /// Initialize options for example with name \a s
  BIBDOptions(const char* s,
              int v0, int k0, int lambda0)
    : Options(s), v(v0), k(k0), lambda(lambda0) {
    derive();
  }
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 4)
      return;
    v = atoi(argv[1]);
    k = atoi(argv[2]);
    lambda = atoi(argv[3]);
    derive();
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(unsigned int) default: " << v << std::endl
              << "\t\tparameter v" << std::endl
              << "\t(unsigned int) default: " << k << std::endl
              << "\t\tparameter k" << std::endl
              << "\t(unsigned int) default: " << lambda << std::endl
              << "\t\tparameter lambda" << std::endl;
  }
};


/**
 * \brief %Example: Balanced Incomplete Block Design (%BIBD)
 *
 * See problem 28 at http://www.csplib.org/.
 *
 * \ingroup ExProblem
 *
 */
class BIBD : public Example {
protected:
  /// Options providing access to parameters
  const BIBDOptions& opt;
  /// Matrix of Boolean variables
  BoolVarArray _p;
public:
  /// Access variable by row and column
  BoolVar& p(int i, int j) {
    // row, column
    return _p[i*opt.b+j];
  }
  /// Actual model
  BIBD(const BIBDOptions& o)
    : opt(o), _p(*this,opt.v*opt.b,0,1) {
    // r ones per row
    for (int i=opt.v; i--; ) {
      BoolVarArgs row(opt.b);
      for (int j=opt.b; j--; )
        row[j] = p(i,j);
      linear(*this, row, IRT_EQ, opt.r);
    }
    // k ones per column
    for (int j=opt.b; j--; ) {
      BoolVarArgs col(opt.v);
      for (int i=opt.v; i--; )
        col[i] = p(i,j);
      linear(*this, col, IRT_EQ, opt.k);
    }
    // Exactly lambda ones in scalar product between two different rows
    for (int i1=0; i1<opt.v; i1++)
      for (int i2=i1+1; i2<opt.v; i2++) {
        BoolVarArgs row(opt.b);
        for (int j=opt.b; j--; ) {
          BoolVar b(*this,0,1);
          rel(*this,p(i1,j),BOT_AND,p(i2,j),b);
          row[j] = b;
        }
        linear(*this, row, IRT_EQ, opt.lambda);
      }

    for (int i=1;i<opt.v;i++) {
      BoolVarArgs row1(opt.b);
      BoolVarArgs row2(opt.b);
      for (int j=opt.b; j--; ) {
        row1[j] = p(i-1,j);
        row2[j] = p(i,j);
      }
      rel(*this, row1, IRT_GQ, row2);
    }
    for (int j=1;j<opt.b;j++) {
      BoolVarArgs col1(opt.v);
      BoolVarArgs col2(opt.v);
      for (int i=opt.v; i--; ) {
        col1[i] = p(i,j-1);
        col2[i] = p(i,j);
      }
      rel(*this, col1, IRT_GQ, col2);
    }

    branch(*this, _p, INT_VAR_NONE, INT_VAL_MIN);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) {
    os << "\tBIBD("
       << opt.v << "," << opt.k << ","
       << opt.lambda << ")" << std::endl;
    for (int i = 0; i < opt.v; i++) {
      os << "\t\t";
      for (int j = 0; j< opt.b; j++)
        os << p(i,j) << " ";
      os << std::endl;
    }
    os << std::endl;
  }

  /// Constructor for cloning \a s
  BIBD(bool share, BIBD& s)
    : Example(share,s), opt(s.opt) {
    _p.update(*this,share,s._p);
  }

  /// Copy during cloning
  virtual Space*
  copy(bool share) {
    return new BIBD(share,*this);
  }

};

/** \brief Main-function
 *  \relates BIBD
 */
int
main(int argc, char* argv[]) {
  BIBDOptions opt("BIBD",7,3,60);
  opt.parse(argc,argv);

  /*
   * Other interesting instances:
   * BIBD(7,3,1), BIBD(6,3,2), BIBD(7,3,20), ...
   */

  Example::run<BIBD,DFS,BIBDOptions>(opt);
  return 0;
}

// STATISTICS: example-any

