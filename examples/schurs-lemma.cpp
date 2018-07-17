/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2011
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
 * \brief %Options for Schur's Lemma
 *
 */
class SchurOptions : public Options {
public:
  int c, n; ///< Parameters to be given on command line
  /// Initialize options for example with name \a s
  SchurOptions(const char* s, int c0, int n0)
    : Options(s), c(c0), n(n0) {}
  /// Parse options from arguments \a argv (number is \a argc)
  void parse(int& argc, char* argv[]) {
    Options::parse(argc,argv);
    if (argc < 3)
      return;
    c = atoi(argv[1]);
    n = atoi(argv[2]);
  }
  /// Print help message
  virtual void help(void) {
    Options::help();
    std::cerr << "\t(unsigned int) default: " << c << std::endl
              << "\t\tparameter c (number of boxes)" << std::endl
              << "\t(unsigned int) default: " << n << std::endl
              << "\t\tparameter n (number of balls)" << std::endl;
  }
};


/**
 * \brief %Example: Schur's lemma
 *
 * Put \f$n\f$ balls labeled \f${1,\ldots,n}\f$ into \f$c\f$ boxes such
 * that for any triple of balls \f$\langle x, y, z\rangle\f$ with
 * \f$x+y = z\f$, not all are in the same box.
 *
 * This problem has a solution for \f$c=3\f$ if \f$n < 14\f$.
 *
 * See also problem 15 at http://www.csplib.org/.
 *
 * \ingroup Example
 *
 */
class Schur : public Script {
protected:
  /// Array of box per ball
  IntVarArray box;
public:
  /// Actual model
  Schur(const SchurOptions& opt)
    : Script(opt), box(*this,opt.n,1,opt.c) {
    int n = opt.n;

    // Iterate over balls and find triples
    for (int i=1; i<=n; i++)
      for (int j=1; i+j<=n; j++)
        rel(*this, {box[i-1],box[j-1],box[i+j-1]}, IRT_NQ);

    // Break value symmetries
    precede(*this, box, IntArgs::create(opt.c, 1));

    branch(*this, box, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MIN());
  }
  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "\t" << box << std::endl;
  }

  /// Constructor for cloning \a s
  Schur(Schur& s) : Script(s) {
    box.update(*this, s.box);
  }
  /// Copy during cloning
  virtual Space*
  copy(void) {
    return new Schur(*this);
  }
};

/** \brief Main-function
 *  \relates Schur
 */
int
main(int argc, char* argv[]) {
  SchurOptions opt("Schur's Lemma",3,13);
  opt.parse(argc,argv);
  Script::run<Schur,DFS,SchurOptions>(opt);
  return 0;
}

// STATISTICS: example-any

