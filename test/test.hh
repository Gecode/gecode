/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Zayenz Lagerkvist, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#ifndef GECODE_TEST_TEST_HH
#define GECODE_TEST_TEST_HH

#include <gecode/kernel.hh>
#include <gecode/search.hh>
#include <gecode/int.hh>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/// General test support
namespace Test {

  /**
   * \defgroup TaskTestTest General test support
   *
   * \ingroup TaskTest
   */
  //@{
  /**
   * \brief Stream used for logging
   *
   * The olog is buffered and printed only if an error has
   * occurred.
   */
  extern std::ostringstream olog;

  /// Simple class for describing indentation
  class ind {
  public:
    /// Which indentation level
    int l;
    /// Indent by level \a i
    ind(int i) : l(i) {}
  };

  /// How to match
  enum MatchType {
    MT_ANY,  //< Positive match anywhere in string
    MT_NOT,  //< Negative match
    MT_FIRST //< Positive match at beginning
  };

  /// Tags for test selection
  enum class TestTag : unsigned int {
    check  = 1U << 0, ///< Basic integrity tests
    normal = 1U << 1, ///< Normal test suite
    sweep  = 1U << 2  ///< Really heavy sweep tests
  };

  /// Set of test tags
  class TestTags {
  private:
    /// Bit mask for tags
    unsigned int _mask;
    /// Initialize from raw bit mask \a m
    explicit TestTags(unsigned int m);
  public:
    /// Initialize with no tags
    TestTags(void);
    /// Initialize with tag \a t
    TestTags(TestTag t);
    /// Initialize with tags \a t0 and \a t1
    TestTags(TestTag t0, TestTag t1);
    /// Return set with all known tags
    static TestTags all(void);
    /// Whether no tags are set
    bool empty(void) const;
    /// Whether this set contains any tag from \a t
    bool overlaps(TestTags t) const;
    /// Add tags \a t
    void add(TestTags t);
    /// Remove tags \a t
    void remove(TestTags t);
  };

  /// Commandline options
  class Options {
  public:
    /// Number of threads to use
    unsigned int threads;
    /// The random seed to be used
    unsigned int seed;
    /// Number of iterations for each test
    unsigned int iter;
    /// Default number of iterations
    static const int defiter = 5;
    /// The probability for computing a fixpoint
    unsigned int fixprob;
    /// Default fixpoint probability
    static const unsigned int deffixprob = 10;
    /// Whether to stop on an error
    bool stop;
    /// Whether to log the tests
    bool log;
    /// Patterns to test against
    std::vector<std::pair<MatchType, const char*> > testpat;
    /// Tags to test against
    TestTags testtags;
    /// Whether test tags have been requested
    bool use_testtags;
    /// Name of first test to start with
    const char* start_from;
    /// Whether to list all tests
    bool list;
    /// Whether to list known tags
    bool list_tags;
    /// Whether to include tags when listing tests
    bool list_with_tags;

    /// Initialize options with defaults
    Options(void);
    /// Parse commandline arguments
    void parse(int argc, char* argv[]);

    /// True iff a test name should be executed according to the patterns. With no patterns, always true.
    bool is_test_name_matching(const std::string& test_name) const;
    /// True iff test tags should be executed according to the requested tags. With no tag request, always true.
    bool is_test_tags_matching(TestTags tags) const;
  };

  /// The options
  extern Options opt;

  /// %Base class for all tests to be run
  class Base {
  private:
    /// Name of the test
    std::string _name;
    /// Tags assigned to the test
    TestTags _tags;
    /// Next test
    Base* _next;
    /// All tests
    static Base* _tests;
    /// How many tests
    static unsigned int _n_tests;
  public:
    /// Create and register a normal test with name \a s
    Base(std::string  s);
    /// Create and register test with name \a s and tags \a t
    Base(std::string s, TestTags t);
    /// Sort tests alphabetically
    static void sort(void);
    /// Return name of test
    const std::string& name(void) const;
    /// Return tags for test
    TestTags tags(void) const;
    /// Replace tags assigned to test with \a t
    void tags(TestTags t);
    /// Add tags \a t to test
    void add_tags(TestTags t);
    /// Remove tags \a t from test
    void remove_tags(TestTags t);
    /// Return all tests
    static Base* tests(void);
    /// Return next test
    Base* next(void) const;
    /// Set next test
    void next(Base* n);
    /// Run test
    virtual bool run(void) = 0;
    /// Throw a coin whether to compute a fixpoint
    bool fixpoint(void);
    /// Throw a coin whether to compute a fixpoint
    static bool fixpoint(Gecode::Support::RandomGenerator& rand);
    /// Destructor
    virtual ~Base(void);

    /// \name Mapping scalar values to strings
    //@{
    /// Map bool to string
    static std::string str(bool b);
    /// Map integer to string
    static std::string str(int i);
    /// Map integer array to string
    static std::string str(const Gecode::IntArgs& i);
    //@}

    /// Random number generator
    mutable Gecode::Support::RandomGenerator _rand;
  };

  /**
   * \brief Run the tests in the process-wide registry
   *
   * Tests register during construction. They must remain alive until every
   * runner call has returned; destruction does not remove a test from the
   * process-wide registry. Runner calls must not overlap. Option parsing exits
   * the process after printing help or reporting a malformed option.
   * \relates Test::Base
   */
  int run_registered_tests(int argc, char* argv[]);
  //@}

}

/**
 * \brief Print indentation
 * \relates Test::ind
 */
std::ostream&
operator<<(std::ostream& os, const Test::ind& i);

#include "test/test.hpp"

#endif

// STATISTICS: test-core
