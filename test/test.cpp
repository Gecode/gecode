/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Mikael Lagerkvist, 2005
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

#include "test/test.hh"

#ifdef GECODE_HAS_MTRACE
#include <mcheck.h>
#endif

#include <iostream>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <utility>
#include <vector>
#include <utility>

namespace Test {

  // Log stream
  std::ostringstream olog;

  /*
   * Base class for tests
   *
   */
  Base::Base(std::string  s)
    : _name(std::move(s)), _next(_tests) {
    _tests = this; _n_tests++;
  }

  Base* Base::_tests = nullptr;
  unsigned int Base::_n_tests = 0;

  /// Sort tests by name
  class SortByName {
  public:
    forceinline bool
    operator()(Base* x, Base* y) {
      return x->name() > y->name();
    }
  };

  void
  Base::sort() {
    Base** b = Gecode::heap.alloc<Base*>(_n_tests);
    unsigned int i=0;
    for (Base* t = _tests; t != nullptr; t = t->next())
      b[i++] = t;
    SortByName sbn;
    Gecode::Support::quicksort(b, _n_tests,sbn);
    i=0;
    _tests = nullptr;
    for ( ; i < _n_tests; i++) {
      b[i]->next(_tests); _tests = b[i];
    }
    Gecode::heap.free(b,_n_tests);
  }

  Base::~Base() = default;

  Gecode::Support::RandomGenerator Base::rand
  = Gecode::Support::RandomGenerator();

  Options opt;

  void report_error(const std::string& name) {
    std::cout << "Options: -seed " << opt.seed;
    if (opt.fixprob != Test::Options::deffixprob)
      std::cout << " -fixprob " << opt.fixprob;
    std::cout << " -test " << name << std::endl;
    if (opt.log)
      std::cout << olog.str();
  }

  /// How to match
  enum MatchType {
    MT_ANY,  //< Positive match anywhere in string
    MT_NOT,  //< Negative match
    MT_FIRST //< Positive match at beginning
  };

  std::vector<std::pair<MatchType, const char*> > testpat;
  const char* startFrom = nullptr;
  bool list = false;

  void
  Options::parse(int argc, char* argv[]) {
    int i = 1;
    while (i < argc) {
      if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help")) {
        std::cerr << "Options for testing:" << std::endl
                  << "\t-seed (unsigned int or \"time\") default: "
                  << seed << std::endl
                  << "\t\tseed for random number generator (unsigned int),"
                  << std::endl
                  << "\t\tor \"time\" for a random seed based on "
                  << "current time" << std::endl
                  << "\t-fixprob (unsigned int) default: "
                  << fixprob << std::endl
                  << "\t\t1/fixprob is the probability of computing a fixpoint"
                  << std::endl
                  << "\t-iter (unsigned int) default: " <<iter<< std::endl
                  << "\t\tthe number of iterations" << std::endl
                  << "\t-test (string) default: (none)" << std::endl
                  << "\t\tsimple pattern for the tests to run" << std::endl
                  << "\t\tprefixing with \"-\" negates the pattern" << std::endl
                  << "\t\tprefixing with \"^\" requires a match at the beginning" << std::endl
                  << "\t\tmultiple pattern-options may be given"
                  << std::endl
                  << "\t-start (string) default: (none)" << std::endl
                  << "\t\tsimple pattern for the first test to run" << std::endl
                  << "\t-log"
                  << std::endl
                  << "\t\tlog execution of tests"
                  << std::endl
                  << "\t\tthe optional argument determines the style of the log"
                  << std::endl
                  << "\t\twith text as the default style"
                  << std::endl
                  << "\t-stop (boolean) default: "
                  << (stop ? "true" : "false") << std::endl
                  << "\t\tstop on first error or continue" << std::endl
                  << "\t-list" << std::endl
                  << "\t\toutput list of all test cases and exit" << std::endl
          ;
        exit(EXIT_SUCCESS);
      } else if (!strcmp(argv[i],"-seed")) {
        if (++i == argc) goto missing;
        if (!strcmp(argv[i],"time")) {
          seed = static_cast<unsigned int>(time(nullptr));
        } else {
          seed = static_cast<unsigned int>(atoi(argv[i]));
        }
      } else if (!strcmp(argv[i],"-iter")) {
        if (++i == argc) goto missing;
        iter = static_cast<unsigned int>(atoi(argv[i]));
      } else if (!strcmp(argv[i],"-fixprob")) {
        if (++i == argc) goto missing;
        fixprob = static_cast<unsigned int>(atoi(argv[i]));
      } else if (!strcmp(argv[i],"-test")) {
        if (++i == argc) goto missing;
        if (argv[i][0] == '^')
          testpat.emplace_back(MT_FIRST, argv[i] + 1);
        else if (argv[i][0] == '-')
          testpat.emplace_back(MT_NOT, argv[i] + 1);
        else
          testpat.emplace_back(MT_ANY, argv[i]);
      } else if (!strcmp(argv[i],"-start")) {
        if (++i == argc) goto missing;
        startFrom = argv[i];
      } else if (!strcmp(argv[i],"-log")) {
        log = true;
      } else if (!strcmp(argv[i],"-stop")) {
        if (++i == argc) goto missing;
        if(argv[i][0] == 't') {
          stop = true;
        } else if (argv[i][0] == 'f') {
          stop = false;
        }
      } else if (!strcmp(argv[i],"-list")) {
        list = true;
      }
      i++;
    }
    return;
  missing:
    std::cerr << "Erroneous argument (" << argv[i-1] << ")" << std::endl
              << "  missing parameter" << std::endl;
    exit(EXIT_FAILURE);
  }

}

/// Check if a test name should be run if there are test name patterns specified
bool is_test_name_matching(const std::string& test_name) {
  if (!Test::testpat.empty()) {
    bool positive_patterns = false;
    bool match_found = false;
    for (const auto& type_pattern: Test::testpat ) {
      const auto& type = type_pattern.first;
      const auto& pattern = type_pattern.second;
      if (type == Test::MT_NOT) { // Negative pattern
        if (test_name.find(pattern) != std::string::npos) {
          // Test matches a negative pattern, should not run
          return false;
        }
      } else {
        // Positive pattern
        positive_patterns = true;
        if (!match_found) {
          // No match found yet, test with current pattern
          if (((type == Test::MT_ANY)   && (test_name.find(pattern) != std::string::npos)) ||
              ((type == Test::MT_FIRST) && (test_name.find(pattern) == 0)))
            match_found = true;
        }
      }
    }

    if (positive_patterns && match_found) {
      // Some positive pattern matched the test name
      return true;
    } else if (positive_patterns && !match_found) {
      // No positive pattern matched the test name
      return false;
    } else {
      // No positive patterns, but no negative pattern ruled the test name out
      return true;
    }
  } else {
    // With no test-patterns, all tests should run.
    return true;
  }
}

int
main(int argc, char* argv[]) {
  using namespace Test;
#ifdef GECODE_HAS_MTRACE
  mtrace();
#endif

  opt.parse(argc, argv);

  Base::sort();

  if (list) {
    for (Base* t = Base::tests() ; t != nullptr; t = t->next() ) {
      std::cout << t->name() << std::endl;
    }
    exit(EXIT_SUCCESS);
  }

  Base::rand.seed(opt.seed);

  std::vector<Base*> tests;
  bool started = startFrom == nullptr ? true : false;
  for (Base* t = Base::tests() ; t != nullptr; t = t->next() ) {
    if (!started) {
      if (t->name().find(startFrom) != std::string::npos) {
        started = true;
      } else {
        continue;
      }
    }
    if (is_test_name_matching(t->name())) {
      tests.emplace_back(t);
    }
  }

  for (auto test : tests) {
    try {
      std::cout << test->name() << " ";
      std::cout.flush();
      for (unsigned int i = opt.iter; i--;) {
        opt.seed = Base::rand.seed();
        if (test->run()) {
          std::cout << '+';
          std::cout.flush();
        } else {
          std::cout << "-" << std::endl;
          report_error(test->name());
          if (opt.stop)
            return 1;
        }
      }
      std::cout << std::endl;
    } catch (Gecode::Exception& e) {
      std::cout << "Exception in \"Gecode::" << e.what()
                << "." << std::endl
                << "Stopping..." << std::endl;
      report_error(test->name());
      if (opt.stop)
        return 1;
    }

  }

  return 0;
}

std::ostream&
operator<<(std::ostream& os, const Test::ind& i) {
  for (int j=i.l; j--; )
    os << "  ";
  return os;
}

// STATISTICS: test-core
