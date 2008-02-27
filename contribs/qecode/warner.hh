/*****************************************************************[warner.hh]
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
#ifndef __QECODE_WARNER__
#define __QECODE_WARNER__

#include "qecode.hh"
#include <iostream>
#include <cstdlib>
#include "gecode/minimodel.hh"
#include "qecore.hh"


using namespace Gecode;
using namespace Gecode::Int;

class QECODE_VTABLE_EXPORT Warner {
private : 
  BranchingHeuristic* bh;
  QSpace* sp;
public : 
  QECODE_EXPORT Warner(QSpace* qs) {bh=NULL;sp=qs;}
  QECODE_EXPORT void update(QSpace* qs) {sp=qs;}
  QECODE_EXPORT void setBH(BranchingHeuristic* b);
  QECODE_EXPORT void warn(int w);
};


class QECODE_VTABLE_EXPORT IntWarningProp : public UnaryPropagator<IntView,PC_INT_DOM> {

private : 
  int var;
  Warner* toWarn;

public : 
  QECODE_EXPORT IntWarningProp (Space* home, int v, IntView vv, Warner* f, bool fd=false);
  QECODE_EXPORT IntWarningProp (Space* home,bool share, IntWarningProp& p);
  QECODE_EXPORT ~IntWarningProp (void);
  QECODE_EXPORT virtual PropCost cost(void) const;
  QECODE_EXPORT virtual ExecStatus propagate(Space * home,ModEventDelta med);
  QECODE_EXPORT virtual Actor* copy(Space* home,bool share);
  QECODE_EXPORT static ExecStatus warning(Space* home, int v, IntView vv, Warner* f);
  QECODE_EXPORT static void IntWarning(Space* home, int v, IntVar iv, Warner* f);
};

class QECODE_VTABLE_EXPORT BoolWarningProp : public UnaryPropagator<BoolView,PC_INT_DOM> {
    
private : 
    int var;
    Warner* toWarn;
    
public : 
    QECODE_EXPORT BoolWarningProp (Space* home, int v, BoolView vv, Warner* f, bool fd=false);
    QECODE_EXPORT BoolWarningProp (Space* home,bool share, BoolWarningProp& p);
    QECODE_EXPORT ~BoolWarningProp (void);
    QECODE_EXPORT virtual PropCost cost(void) const;
    QECODE_EXPORT virtual ExecStatus propagate(Space * home,ModEventDelta med);
    QECODE_EXPORT virtual Actor* copy(Space* home,bool share);
    QECODE_EXPORT static ExecStatus warning(Space* home, int v, BoolView vv, Warner* f);
    QECODE_EXPORT static void BoolWarning(Space* home, int v, BoolVar iv, Warner* f);
};


#endif
