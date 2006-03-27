/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *
 *  Last modified:
 *     $Date: 2006-03-21 16:35:03 +0100 (Tue, 21 Mar 2006) $ by $Author: schulte $
 *     $Revision: 3092 $
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

class Channel : public IntTest {
public:
  Channel(const char* t, IntConLevel icl) 
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

Channel _channeldom("Channel::Dom",ICL_DOM);
Channel _channelval("Channel::Val",ICL_VAL);

// STATISTICS: test-int

