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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef __GECODE_TEST_TEST_HH__
#define __GECODE_TEST_TEST_HH__

#include "gecode/kernel.hh"
#include "gecode/search.hh"

using namespace Gecode;

class Options {
public:    
  static const int defiter = 5,
    deffixprob = 10,
    defflushprob = 10;

  int   seed;
  int   iter;
  int   fixprob;
  int   flushprob;
  bool  log, display;
  bool  stop_on_error;
  Options(void) 
    : seed(0), iter(defiter), fixprob(deffixprob), flushprob(defflushprob),
      log(false), display(true), stop_on_error(true)
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
};

/// Return number between 0..m-1
unsigned int random(unsigned int m);
int random(int m);

#endif


// STATISTICS: test-core
