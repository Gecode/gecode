/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, 
         IntConLevel icl, PropKind) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, BoolVar b, 
         IntConLevel icl, PropKind) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,b);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, int c,
         IntConLevel icl, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, int c, BoolVar b,
         IntConLevel, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,b);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, IntVar y, 
         IntConLevel icl, PropKind) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, IntVar y, BoolVar b,
         IntConLevel, PropKind) {
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, IntVar y,
         IntConLevel icl, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
  }

  void
  linear(Space* home,
         const IntArgs& a, const IntVarArgs& x, IntRelType r, IntVar y,
         BoolVar b, IntConLevel, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) return;
    GECODE_AUTOARRAY(Linear::Term<IntView>, t, x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }

}

// STATISTICS: int-post

