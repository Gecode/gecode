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

#ifdef GECODE_HAVE_MTRACE
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
   * Iterator for propagation kinds
   *
   */
  const Gecode::PropKind PropKinds::pks[] =
    {Gecode::PK_MEMORY,Gecode::PK_SPEED};
  

  /*
   * Base class for tests
   *
   */
  Base::Base(const std::string& s)
    : _name(s) {
    if (_tests == NULL) {
      _tests = this; _next = NULL;
    } else {
      // Search alphabetic_testsy
      Base* p = NULL;
      Base* c = _tests;
      while ((c != NULL) && (c->name() < s)) {
        p = c; c = c->_next;
      }
      if (c == NULL) {
        p->_next = this; _next = NULL;
      } else if (c == _tests) {
        _next = _tests; _tests = this;
      } else {
        p->_next = this; _next = c;
      }
    }
  }

  Base* Base::_tests = NULL;
  
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
    
  void
  Options::parse(int argc, char* argv[]) {
    using namespace std;
    using std::vector;
    using std::make_pair;
    using std::pair;
  
    static const char* bool2str[] =
      { "false", "true" };
    int i = 1;
    const char* e = NULL;
    while (i < argc) {
      if (!strcmp(argv[i],"-help") || !strcmp(argv[i],"--help")) {
        std::cerr << "Options for testing:" << std::endl
                  << "\t-seed (unsigned int or \"time\") default: " 
                  << seed << std::endl
                  << "\t\tthe seed for the random numbers (an integer),"
                  << std::endl
                  << "\t\tor the word time for a random seed based on "
                  << "current time" << std::endl
                  << "\t-fixprob (unsigned int) default: " <<fixprob<< std::endl
                  << "\t\t1/fixprob is the probability of computing a fixpoint"
                  << std::endl
                  << "\t-iter (unsigned int) default: " <<iter<< std::endl
                  << "\t\tthe number of iterations" << std::endl
                  << "\t-test (string) default: (none)" << std::endl
                  << "\t\tsimple pattern for the tests to run" << std::endl
                  << "\t\tprefixing the pattern by a - negates the pattern"
                  << std::endl
                  << "\t\tmultiple pattern-options may be given" << std::endl
                  << "\t-log"
                  << std::endl
                  << "\t\tlog execution of tests"
                  << std::endl
                  << "\t\tthe optional argument determines the style of the log"
                  << std::endl
                  << "\t\twith text as the default style"
                  << std::endl
                  << "\t-stop (boolean) default: "
                  << bool2str[stop] << std::endl
                  << "\t\tstop on first error or continue" << std::endl
          ;
        exit(EXIT_SUCCESS);
      } else if (!strcmp(argv[i],"-seed")) {
        if (++i == argc) goto missing;
        if (!strcmp(argv[i],"time")) {
          seed = static_cast<int>(time(NULL));
        } else {
          seed = atoi(argv[i]);
        }
      } else if (!strcmp(argv[i],"-iter")) {
        if (++i == argc) goto missing;
        iter = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-fixprob")) {
        if (++i == argc) goto missing;
        fixprob = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-test")) {
        if (++i == argc) goto missing;
        int offset = 0;
        bool negative = false;
        if (argv[i][0] == '-') {
          negative = true;
          offset = 1;
        }
        testpat.push_back(make_pair(negative, argv[i] + offset));
      } else if (!strcmp(argv[i],"-log")) {
        log = true;
      } else if (!strcmp(argv[i],"-stop")) {
        if (++i == argc) goto missing;
        if(argv[i][0] == 't') {
          stop = true;
        } else if (argv[i][0] == 'f') {
          stop = false;
        }
      }
      i++;
    }
    return;
  missing:
    e = "missing parameter";
  error:
    std::cerr << "Erroneous argument (" << argv[i-1] << ")" << std::endl
              << e << std::endl;
    exit(EXIT_FAILURE);
  }

}

int
main(int argc, char* argv[]) {
  using namespace Test;
#ifdef GECODE_HAVE_MTRACE
  mtrace();
#endif

  opt.parse(argc, argv);
  Base::rand.seed(opt.seed);

  for (Base* t = Base::tests() ; t != NULL; t = t->next() ) {
    try {
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
      for (int i = opt.iter; i--; ) {
        opt.seed = Base::rand.seed();
        if (t->run()) {
          std::cout << "+";
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
