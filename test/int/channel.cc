/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

#include "test/int.hh"
#include "gecode/minimodel.hh"

namespace {

  IntSet ds_12(-1,2);
  IntSet ds_03(0,3);
  IntSet ds_05(0,5);

  class ChannelFull : public IntTest {
  public:
    ChannelFull(const char* t, IntConLevel icl)
      : IntTest(t,8,ds_03,false,icl) {}
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<4; i++)
        if (x[4+x[i]] != i)
          return false;
      return true;
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArgs xa(4);
      IntVarArgs ya(4);
      for (int i=4; i--; ) {
        xa[i] = x[i];
        ya[i] = x[4+i];
      }
      channel(home, xa, ya, icl);
    }
  };
  
  class ChannelHalf : public IntTest {
  public:
    ChannelHalf(const char* t, IntConLevel icl)
      : IntTest(t,6,ds_05,false,icl) {}
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<6; i++)
        for (int j=i+1; j<6; j++)
          if (x[i] == x[j])
            return false;
      return true;
    }
    virtual void post(Space* home, IntVarArray& x) {
      IntVarArray y(home,6,dom);
      for (int i=0; i<6; i++) {
        for (int j=0; j<6; j++) {
          BoolVar b(home,0,1);
          rel(home, x[i], IRT_EQ, j, b);
          rel(home, y[j], IRT_EQ, i, b);
        }
      }
      channel(home, x, y, icl);
    }
  };
  
  class ChannelShared : public IntTest {
  public:
    ChannelShared(const char* t, IntConLevel icl)
      : IntTest(t,6,ds_05,false,icl) {
      testdomcon = false;
    }
    virtual bool solution(const Assignment& x) const {
      for (int i=0; i<6; i++)
        if (x[x[i]] != i)
          return false;
      return true;
    }
    virtual void post(Space* home, IntVarArray& x) {
      channel(home, x, x, icl);
    }
  };
  
  class ChannelLinkSingle : public IntTest {
  public:
    ChannelLinkSingle(void)
      : IntTest("Channel::Bool::Single",2,ds_12,false,ICL_DEF) {
      testdomcon = false;
    }
    virtual bool solution(const Assignment& x) const {
      return ((x[0]==0) || (x[0]==1)) && (x[0]==x[1]);
    }
    virtual void post(Space* home, IntVarArray& x) {
      BoolVar b(home,0,1);
      channel(home, x[0], b);
      channel(home, x[1], b);
    }
  };
  
  class ChannelLinkMulti : public IntTest {
  private:
    int o;
  public:
    ChannelLinkMulti(const char* t, IntSet& ds, int o0)
      : IntTest(t,7,ds,false,ICL_DEF), o(o0) {
      testdomcon = false;
    }
    virtual bool solution(const Assignment& x) const {
      int n = x.size()-1;
      for (int i=n; i--; )
        if ((x[i] != 0) && (x[i] != 1))
          return false;
      int k=x[n]-o;
      if ((k<0) || (k>=n))
        return false;
      for (int i=0; i<k; i++)
        if (x[i] != 0)
          return false;
      for (int i=k+1; i<n; i++)
        if (x[i] != 0)
          return false;
      return x[k] == 1;
    }
    virtual void post(Space* home, IntVarArray& x) {
      int n=x.size()-1;
      BoolVarArgs b(n);
      for (int i=n; i--; )
        b[i]=channel(home,x[i]);
      channel(home, b, x[n], o);
    }
  };
  
  ChannelFull cfd("Channel::Int::Full::Dom",ICL_DOM);
  ChannelFull cfv("Channel::Int::Full::Val",ICL_VAL);

  ChannelHalf chd("Channel::Int::Half::Dom",ICL_DOM);
  ChannelHalf chv("Channel::Int::Half::Val",ICL_VAL);

  ChannelShared csd("Channel::Int::Shared::Dom",ICL_DOM);
  ChannelShared csv("Channel::Int::Shared::Val",ICL_VAL);

  ChannelLinkSingle cls;

  IntSet dlm_05(0,5);
  IntSet dlm_16(1,6);
  IntSet dlm_14(-1,4);
  ChannelLinkMulti clma("Channel::Bool::Multi::A",dlm_05, 0);
  ChannelLinkMulti clmb("Channel::Bool::Multi::B",dlm_16, 1);
  ChannelLinkMulti clmc("Channel::Bool::Multi::C",dlm_14,-1);

}

// STATISTICS: test-int

