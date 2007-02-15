/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2007
 *
 *  Last modified:
 *     $Date: 2006-10-25 15:21:37 +0200 (Wed, 25 Oct 2006) $ by $Author: tack $
 *     $Revision: 3791 $
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

static IntSet ds_04(0,4);

class Circuit : public IntTest {
public:
  Circuit(void)
    : IntTest("Circuit",5,ds_04,false,ICL_DEF) {}
  virtual bool solution(const Assignment& x) const {
    for (int i=x.size(); i--; )
      for (int j=x.size(); j--; )
        if ((i!=j) && (x[i]==x[j]))
          return false;
    bool reachable[] = {false,false,false,false,false};
    int i0=x[ 0]; reachable[i0] = true;
    int i1=x[i0]; reachable[i1] = true;
    int i2=x[i1]; reachable[i2] = true;
    int i3=x[i2]; reachable[i3] = true;
    int i4=x[i3]; reachable[i4] = true;
    for (int i=x.size(); i--; )
      if (!reachable[i])
        return false;
    return true;
  }
  virtual void post(Space* home, IntVarArray& x) {
    circuit(home, x, icl);
  }
};

Circuit _circuit;

// STATISTICS: test-int

