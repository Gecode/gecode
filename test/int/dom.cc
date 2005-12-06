/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
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

#include "test/int.hh"

static const int d1r[4][2] = {
  {-4,-3},{-1,-1},{1,1},{3,5}
};
static IntSet d1(d1r,4);

static IntSet ds_66(-6,6);

class DomRange : public IntTest {
public:
  DomRange(const char* t) 
    : IntTest(t,1,ds_66,true) {}
  virtual bool solution(const Assignment& x) const {
    return (x[0] >= -2) && (x[0] <= 2);
  }
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::dom(home, x[0], -2, 2);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    Gecode::dom(home, x[0], -2, 2, b);
  }
};
DomRange _domrange("Dom::Range");


class DomDom : public IntTest {
public:
  DomDom(const char* t) 
    : IntTest(t,1,ds_66,true) {}
  virtual bool solution(const Assignment& x) const {
    return 
      (((x[0] >= -4) && (x[0] <= -3)) ||
       ((x[0] >= -1) && (x[0] <= -1)) ||
       ((x[0] >=  1) && (x[0] <=  1)) ||
       ((x[0] >=  3) && (x[0] <=  5)));
  }
  virtual void post(Space* home, IntVarArray& x) {
    Gecode::dom(home, x[0], d1);
  }
  virtual void post(Space* home, IntVarArray& x, BoolVar b) {
    Gecode::dom(home, x[0], d1, b);
  }
};
DomDom _domdom("Dom::Dom");

// STATISTICS: test-int

