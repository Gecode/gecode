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

static IntSet ds_22(-2,2);

class Distinct : public IntTest {
private:
  IntConLevel icl;
public:
  Distinct(const char* t, IntConLevel icl0) 
    : IntTest(t,4,ds_22,false,1,icl0==ICL_DOM), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<4; i++)
      for (int j=i+1; j<4; j++)
	if (x[i]==x[j])
	  return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    distinct(home, x, icl);
  }
};

Distinct _dom("Distinct::Dom",ICL_DOM);
Distinct _bnd("Distinct::Bnd",ICL_BND);
Distinct _val("Distinct::Val",ICL_VAL);

class DistinctOffset : public IntTest {
private:
  IntConLevel icl;
public:
  DistinctOffset(const char* t, IntConLevel icl0) 
    : IntTest(t,4,ds_22,false,1,icl0==ICL_DOM), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=0; i<4; i++)
      for (int j=i+1; j<4; j++)
	if (x[i]+i==x[j]+j)
	  return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs c(4);
    for (int i=0; i<4; i++)
      c[i]=i;
    distinct(home, c, x, icl);
  }
};

DistinctOffset _domo("Distinct::Dom::Offset",ICL_DOM);
DistinctOffset _bndo("Distinct::Bnd::Offset",ICL_BND);
DistinctOffset _valo("Distinct::Val::Offset",ICL_VAL);

class DistinctShared : public IntTest {
private:
  IntConLevel icl;
public:
  DistinctShared(const char* t, IntConLevel icl0) 
    : IntTest(t,1,ds_22,false), icl(icl0) {}
  virtual bool solution(const Assignment& x) const {
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    IntArgs c(4); IntVarArgs y(4);
    for (int i=0; i<4; i++) {
      c[i]=i; y[i]=x[0];
    }
    distinct(home, c, y, icl);
  }
};

DistinctShared _doms("Distinct::Dom::Shared",ICL_DOM);
DistinctShared _bnds("Distinct::Bnd::Shared",ICL_BND);
DistinctShared _vals("Distinct::Val::Shared",ICL_VAL);


// STATISTICS: test-int

