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

#include "test/test.hh"

#ifdef GECODE_HAS_MTRACE
#include <mcheck.h>
#endif

#include <iostream>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <utility>

namespace Test {

  // Log stream
  std::ostringstream olog;

  /*
   * Base class for tests
   *
   */
  Base::Base(const std::string& s)
    : _name(s), _next(_tests) {
    _tests = this; _n_tests++;
  }

  Base* Base::_tests = NULL;
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
  Base::sort(void) {
    Base** b = Gecode::heap.alloc<Base*>(_n_tests);
    unsigned int i=0;
    for (Base* t = _tests; t != NULL; t = t->next())
      b[i++] = t;
    SortByName sbn;
    Gecode::Support::quicksort(b,_n_tests,sbn);
    i=0;
    _tests = NULL;
    for ( ; i < _n_tests; i++) {
      b[i]->next(_tests); _tests = b[i];
    }
    Gecode::heap.free(b,_n_tests);
  }

  Base::~Base(void) {}

  Gecode::Support::RandomGenerator Base::rand
  = Gecode::Support::RandomGenerator();

  Options opt;

  void report_error(std::string name) {
    std::cout << "Options: -seed " << opt.seed;
    if (opt.fixprob != opt.deffixprob)
      std::cout << " -fixprob " << opt.fixprob;
    std::cout << " -test " << name << std::endl;
    if (opt.log)
      std::cout << olog.str();
  }

  std::vector<std::pair<bool, const char*> > testpat;
  const char* startFrom = NULL;
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
                  << "\t\tprefixing the pattern with \"-\" negates the pattern"
                  << std::endl
                  << "\t\tmultiple pattern-options may be given" << std::endl
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
          seed = static_cast<unsigned int>(time(NULL));
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
        if (argv[i][0] == '-')
          testpat.push_back(std::make_pair(true, argv[i] + 1));
        else
          testpat.push_back(std::make_pair(false, argv[i]));
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

int
main(int argc, char* argv[]) {
  using namespace Test;
#ifdef GECODE_HAS_MTRACE
  mtrace();
#endif

  opt.parse(argc, argv);

  Base::sort();

  if (list) {
    for (Base* t = Base::tests() ; t != NULL; t = t->next() ) {
      std::cout << t->name() << std::endl;
    }
    exit(EXIT_SUCCESS);
  }

  Base::rand.seed(opt.seed);

  bool started = startFrom == NULL ? true : false;

  for (Base* t = Base::tests() ; t != NULL; t = t->next() ) {
    try {
      if (!started) {
        if (t->name().find(startFrom) != std::string::npos)
          started = true;
        else
          goto next;
      }
      if (testpat.size() != 0) {
        bool match_found   = false;
        bool some_positive = false;
        for (unsigned int i = 0; i < testpat.size(); ++i) {
          if (testpat[i].first) { // Negative pattern
            if (t->name().find(testpat[i].second) != std::string::npos)
              goto next;
          } else {               // Positive pattern
            some_positive = true;
            if (t->name().find(testpat[i].second) != std::string::npos)
              match_found = true;
          }
        }
        if (some_positive && !match_found) goto next;
      }
      std::cout << t->name() << " ";
      std::cout.flush();
      for (unsigned int i = opt.iter; i--; ) {
        opt.seed = Base::rand.seed();
        if (t->run()) {
          std::cout << '+';
          std::cout.flush();
        } else {
          std::cout << "-" << std::endl;
          report_error(t->name());
          if (opt.stop)
            return 1;
        }
      }
    std::cout << std::endl;
    } catch (Gecode::Exception e) {
      std::cout << "Exception in \"Gecode::" << e.what()
                << "." << std::endl
                << "Stopping..." << std::endl;
      report_error(t->name());
      if (opt.stop)
        return 1;
    }
  next:;
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
