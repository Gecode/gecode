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

namespace {

  class Circuit : public IntTest {
  private:
    bool* reachable;
  public:
    Circuit(const char* t, int n, const IntSet& ds)
      : IntTest(t,n,ds,false,ICL_DEF),
        reachable(new bool[n]) {}
    virtual bool solution(const Assignment& x) const {
      int n = x.size();
      for (int i=n; i--; ) {
        if ((x[i] < 0) || (x[i] > n-1))
          return false;
        for (int j=n; j--; )
          if ((i!=j) && (x[i]==x[j]))
            return false;
      }
      for (int i=n; i--; )
        reachable[i]=false;
      {
        int j=0;
        for (int i=n; i--; ) {
          j=x[j]; reachable[j]=true;
        }
      }
      for (int i=n; i--; )
        if (!reachable[i])
          return false;
      return true;
    }
    virtual void post(Space* home, IntVarArray& x) {
      circuit(home, x, icl);
    }
    virtual ~Circuit(void) {
      delete [] reachable;
    }
  };

  IntSet ds_00(0,0);
  IntSet ds_01(0,1);
  IntSet ds_02(0,2);
  IntSet ds_03(0,3);
  IntSet ds_04(0,4);
  IntSet ds_05(0,5);

  Circuit c1("Circuit::1",1,ds_00);
  Circuit c2("Circuit::2",2,ds_01);
  Circuit c3("Circuit::3",3,ds_02);
  Circuit c4("Circuit::4",4,ds_03);
  Circuit c5("Circuit::5",5,ds_04);
  Circuit c6("Circuit::6",6,ds_05);

}

// STATISTICS: test-int
