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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel, PropKind) {
    if (home->failed()) return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c);
  }

  void
  linear(Space* home, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel, PropKind) {
    if (home->failed()) return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y);
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c);
  }

  void
  linear(Space* home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel, PropKind) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home->failed()) 
      return;

    int n=x.size();
    GECODE_AUTOARRAY(Linear::Term<BoolView>, t, n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y);
  }

}

// STATISTICS: int-post
