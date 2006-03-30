/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
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

//#include <cstdlib>

#include "test/random.hh"

namespace {
  // Simple linear congruential random number generator
  // Instance taken from D.E. Knuth, Seminumerical algorithms, Chapter 3.6.
  const unsigned int m = 2147483647,
    a = 48271,
    q = 44488,
    r = 3399;
  int x = 1;

  void seed(unsigned int val) {
    x = val % m;
    if (x == 0) x = 1;
  }
  
  unsigned int randimp() {
    x = a*(x%q) - r*(x/q);
    if (x<0) x += m;
    return x;
  }
}

unsigned int test_seed() {
  return x;
}

void test_seed(unsigned int val) {
  seed(val);
}

unsigned long test_rand() {
  unsigned int a = randimp() & ((1<<16)-1);
  unsigned int b = randimp() & ((1<<16)-1);
  return (a<<16) | b;
  
}

// STATISTICS: test-core

