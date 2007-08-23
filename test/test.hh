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

#include "gecode/support/random.hh"

#include "test/log.hh"

#include <iostream>
#include <sstream>

using namespace Gecode;

class Options {
public:
  static const int defiter = 5,
    deffixprob = 10,
    defnoofmachines = 1,
    defmachine = 1,
    defskip = 0;

  int   seed;
  int   iter;
  int   fixprob;
  bool  log, display;
  bool  stop_on_error;
  unsigned int   noofmachines;
  unsigned int   machine;
  unsigned int   skip;
  Options(void)
    : seed(0), iter(defiter), fixprob(deffixprob), 
      log(false), display(true), stop_on_error(true),
      noofmachines(defnoofmachines), machine(defmachine), skip(defskip)
  {}

  void parse(int argc, char **argv);
};

class Test {
private:
  const char* m;
  const char* t;
  Test* n;
  static Test* all;
public:
  /// Return number between 0..m-1
  static Support::RandomGenerator randgen;

  Test(const char* module, const char* test)
    : m(module), t(test) {
    if (all == NULL) {
      all = this; n = NULL;
    } else {
      // Search alphabetically
      Test* p = NULL;
      Test* c = all;
      while ((c != NULL) &&
             ((strcmp(m,c->m)>0)
              || ((strcmp(m,c->m)==0) && (strcmp(t,c->t)>0)))) {
        p = c; c = c->n;
      }
      if (c == NULL) {
        p->n = this; n = NULL;
      } else if (c == all) {
        n = all; all = this;
      } else {
        p->n = this; n = c;
      }
    }
  }
  static Test* tests(void) {
    return all;
  }
  Test* next(void) const {
    return n;
  }
  const char* module(void) const {
    return m;
  }
  const char* test(void) const {
    return t;
  }
  virtual bool run(const Options& opt) = 0;
  virtual ~Test(void) {}

protected:
  /** \brief Log start of test.
   */
  void log_posting(void) {
    if (Log::logging()) {
      std::ostringstream h, c;
      Log::log(h.str(), c.str());
    }
  }
};

#endif

// STATISTICS: test-core
