/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

static IntSet ds_03(0,3);
static IntSet ds_05(0,5);

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

ChannelFull _channel_full_dom("Channel::Full::Dom",ICL_DOM);
ChannelFull _channel_full_val("Channel::Full::Val",ICL_VAL);

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

ChannelHalf _channel_half_dom("Channel::Half::Dom",ICL_DOM);
ChannelHalf _channel_half_val("Channel::Half::Val",ICL_VAL);

// STATISTICS: test-int

