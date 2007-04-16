/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2006
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

#ifndef __GECODE_ASSIGNMENT_HH__
#define __GECODE_ASSIGNMENT_HH__

#include "gecode/int.hh"
#include "gecode/support/random.hh"

using namespace Gecode;
using namespace Int;

class Assignment {
protected:
  int n;
  IntSetValues* dsv;
  IntSet d;
  bool done;
public:
  Assignment(int, const IntSet&);
  virtual void reset(void);
  virtual bool operator()(void) const {
    return !done;
  }
  virtual void operator++(void);
  virtual int  operator[](int i) const {
    assert((i>=0) && (i<n));
    return dsv[i].val();
  }
  int size(void) const {
    return n;
  }
  virtual ~Assignment(void) {
    delete [] dsv;
  }
};

class RandomAssignment : public Assignment {
protected:
  int *vals;
  int count;
  int left;
  virtual int randval(void);
public:
  RandomAssignment(int, const IntSet&, int);
  virtual void reset(void);
  virtual bool operator()(void) const {
    return left > 0;
  }
  virtual int  operator[](int i) const {
    assert((i>=0) && (i<n));
    return vals[i];
  }
  virtual void operator++(void);
  virtual ~RandomAssignment(void) {
    delete [] vals;
  }
};

std::ostream&
operator<<(std::ostream&, const Assignment&);


Assignment::Assignment(int n0, const IntSet& d0)
  : n(n0), dsv(new IntSetValues[n]), d(d0) {
  reset();
}

void
Assignment::reset(void) {
  done = false;
  for (int i=n; i--; )
    dsv[i].init(d);
}

void
Assignment::operator++(void) {
  int i = n-1;
  while (true) {
    ++dsv[i];
    if (dsv[i]())
      return;
    dsv[i].init(d);
    --i;
    if (i<0) {
      done = true;
      return;
    }
  }
}

RandomAssignment::RandomAssignment(int n0, const IntSet& d0, int count0)
  : Assignment(n0, d0), vals(new int[n0]),
    count(count0), left(count0) {
  reset();
}

void
RandomAssignment::reset(void) {
  left = count;
  for (int i=n; i--; )
    vals[i] = randval();
}

void
RandomAssignment::operator++(void) {
  for (int i = n; i--; )
    vals[i] = randval();
  --left;
}

namespace {
  Support::RandomGenerator randgen;
  
}

int
RandomAssignment::randval(void) {
  int v;
  IntSetRanges it(d);
  unsigned int skip = randgen(d.size());
  while (true) {
    if (it.width() > skip) {
      v = it.min() + skip;
      break;
    }
    skip -= it.width();
    ++it;
  }
  return v;
}

std::ostream&
operator<<(std::ostream& os, const Assignment& a) {
  int n = a.size();
  os << "{";
  for (int i=0; i<n; i++)
    os << a[i] << ((i!=n-1) ? "," : "}");
  return os;
}


#endif

// STATISTICS: examples-int

