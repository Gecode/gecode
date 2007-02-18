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
    Circuit(const char* t, int n, const IntSet& ds, IntConLevel icl)
      : IntTest(t,n,ds,false,icl),
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
        for (int i=n+1; i--; ) {
          j=x[j]; reachable[j]=true;
        }
      }
      for (int i=n; i--; )
        if (!reachable[i])
          return false;
      std::cout << "Solution: ";
      for (int i=0; i<n; i++)
        std::cout << x[i] << ", ";
      std::cout << std::endl;
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

  Circuit c1v("Circuit::Val::1",1,ds_00,ICL_VAL);
  Circuit c2v("Circuit::Val::2",2,ds_01,ICL_VAL);
  Circuit c3v("Circuit::Val::3",3,ds_02,ICL_VAL);
  Circuit c4v("Circuit::Val::4",4,ds_03,ICL_VAL);
  Circuit c5v("Circuit::Val::5",5,ds_04,ICL_VAL);
  Circuit c6v("Circuit::Val::6",6,ds_05,ICL_VAL);

  Circuit c1d("Circuit::Dom::1",1,ds_00,ICL_DOM);
  Circuit c2d("Circuit::Dom::2",2,ds_01,ICL_DOM);
  Circuit c3d("Circuit::Dom::3",3,ds_02,ICL_DOM);
  Circuit c4d("Circuit::Dom::4",4,ds_03,ICL_DOM);
  Circuit c5d("Circuit::Dom::5",5,ds_04,ICL_DOM);
  Circuit c6d("Circuit::Dom::6",6,ds_05,ICL_DOM);

}

// STATISTICS: test-int
