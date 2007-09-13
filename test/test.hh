/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
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

#ifndef __GECODE_TEST_TEST_HH__
#define __GECODE_TEST_TEST_HH__

#include "gecode/kernel.hh"
#include "gecode/search.hh"

#include <iostream>
#include <sstream>
#include <string>

/// General test support
namespace Test {

  /**
   * \defgroup TaskTestTest General test support
   *
   * \ingroup TaskTest
   */
  //@{
  /// Iterator for propagation kinds
  class PropKinds {
  private:
    /// Array of propagation kinds
    static const Gecode::PropKind pks[2];
    /// Current position in array
    int i; 
  public:
    /// Initialize iterator
    PropKinds(void);
    /// Test whether iterator is done
    bool operator()(void) const;
    /// Increment to next propagation kind
    void operator++(void);
    /// Return current propagation kind
    Gecode::PropKind pk(void) const;
  };

  /// Commandline options
  class Options {
  public:
    /// The random seed to be used
    int seed;
    /// Number of iterations for each test
    int iter;
    /// Default number of iterations
    static const int defiter = 5;
    /// The probability for computing a fixpoint
    int fixprob;
    /// Default fixpoint probaibility
    static const int deffixprob = 10;
    /// Whether to stop on an error
    bool stop;
    /// Whether to log the tests
    bool log;

    /// Initialize options with defaults
    Options(void);
    /// Parse commandline arguments
    void parse(int argc, char* argv[]);
  };
  
  /// Base class for all tests to be run
  class Base {
  private:
    /// Name of the test
    std::string _name;
    /// Next test
    Base* _next;
    /// All tests
    static Base* _tests;
  public:
    /// Create and register test with name \a s
    Base(const std::string& s);
    /// Return name of test
    const std::string& name(void) const;
    /// Return all tests
    static Base* tests(void);
    /// Return next test
    Base* next(void) const;
    /// Run test
    virtual bool run(const Options& opt) = 0;
    /// Destructor
    virtual ~Base(void);

    /// Random number generator
    static Gecode::Support::RandomGenerator rand;
  };
  //@}

}

/**
 * \brief Main function 
 * \relates Test::Base
 */
int main(int argc, char* argv[]);

#include "test/test.icc"

#endif

// STATISTICS: test-core
