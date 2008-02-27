/*****************************************************************[warner.cc]
Copyright (c) 2007, Universite d'Orleans - Jeremie Vautard, Marco Benedetti,
Arnaud Lallouet.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*****************************************************************************/
#include "warner.hh"

void Warner::warn(int w) {
  if (bh != NULL) 
    bh->vartouched(w,sp);
}


void Warner::setBH(BranchingHeuristic* b) {
  bh=b;
}


IntWarningProp::IntWarningProp(Space* home, int v, IntView vv, Warner* f, bool fd) : UnaryPropagator<IntView,PC_INT_DOM>(home,vv) {
  var=v;
  toWarn=f;
}

IntWarningProp::IntWarningProp(Space* home, bool share, IntWarningProp& p) : UnaryPropagator<IntView,PC_INT_DOM>(home,share,p) {
  var=p.var;
  toWarn=p.toWarn;
}

IntWarningProp::~IntWarningProp(void) {}

PropCost IntWarningProp::cost(void) const {return PC_CRAZY_HI;}

ExecStatus IntWarningProp::propagate(Space * home,ModEventDelta med) {
  toWarn->warn(var);
  return ES_NOFIX;
}

Actor* IntWarningProp::copy(Space* home,bool share) {return new (home) IntWarningProp(home,share,*this);}

ExecStatus IntWarningProp::warning(Space* home, int v, IntView vv, Warner* f) {
  (void) new (home) IntWarningProp(home,v,vv,f);
  return ES_OK;
}

void IntWarningProp::IntWarning(Space* home, int v, IntVar iv, Warner* f) {
  IntView vv(iv);  
  GECODE_ES_FAIL(home,warning(home,v,vv,f));
}

//////////////////////////////////////////////

BoolWarningProp::BoolWarningProp(Space* home, int v, BoolView vv, Warner* f, bool fd) : UnaryPropagator<BoolView,PC_INT_DOM>(home,vv) {
    var=v;
    toWarn=f;
}

BoolWarningProp::BoolWarningProp(Space* home, bool share, BoolWarningProp& p) : UnaryPropagator<BoolView,PC_INT_DOM>(home,share,p) {
    var=p.var;
    toWarn=p.toWarn;
}

BoolWarningProp::~BoolWarningProp(void) {}

PropCost BoolWarningProp::cost(void) const {return PC_CRAZY_HI;}

ExecStatus BoolWarningProp::propagate(Space * home,ModEventDelta med) {
    toWarn->warn(var);
    return ES_NOFIX;
}

Actor* BoolWarningProp::copy(Space* home,bool share) {return new (home) BoolWarningProp(home,share,*this);}

ExecStatus BoolWarningProp::warning(Space* home, int v, BoolView vv, Warner* f) {
    (void) new (home) BoolWarningProp(home,v,vv,f);
    return ES_OK;
}

void BoolWarningProp::BoolWarning(Space* home, int v, BoolVar iv, Warner* f) {
    BoolView vv(iv);  
    GECODE_ES_FAIL(home,warning(home,v,vv,f));
}



