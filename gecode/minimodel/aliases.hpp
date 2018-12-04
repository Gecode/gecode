/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2018
 *     Guido Tack, 2018
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

namespace Gecode {

  inline void
  atmost(Home home, const IntVarArgs& x, int n, int m,
         IntPropLevel ipl) {
    count(home,x,n,IRT_LQ,m,ipl);
  }
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, int m,
         IntPropLevel ipl) {
    count(home,x,y,IRT_LQ,m,ipl);
  }
  inline void
  atmost(Home home, const IntVarArgs& x, const IntArgs& y, int m,
         IntPropLevel ipl) {
    count(home,x,y,IRT_LQ,m,ipl);
  }
  inline void
  atmost(Home home, const IntVarArgs& x, int n, IntVar z,
         IntPropLevel ipl) {
    count(home,x,n,IRT_LQ,z,ipl);
  }
  inline void
  atmost(Home home, const IntVarArgs& x, IntVar y, IntVar z,
         IntPropLevel ipl) {
    count(home,x,y,IRT_LQ,z,ipl);
  }
  inline void
  atmost(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
         IntPropLevel ipl) {
    count(home,x,y,IRT_LQ,z,ipl);
  }

  inline void
  atleast(Home home, const IntVarArgs& x, int n, int m,
          IntPropLevel ipl) {
    count(home,x,n,IRT_GQ,m,ipl);
  }
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, int m,
          IntPropLevel ipl) {
    count(home,x,y,IRT_GQ,m,ipl);
  }
  inline void
  atleast(Home home, const IntVarArgs& x, const IntArgs& y, int m,
          IntPropLevel ipl) {
    count(home,x,y,IRT_GQ,m,ipl);
  }
  inline void
  atleast(Home home, const IntVarArgs& x, int n, IntVar z,
          IntPropLevel ipl) {
    count(home,x,n,IRT_GQ,z,ipl);
  }
  inline void
  atleast(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntPropLevel ipl) {
    count(home,x,y,IRT_GQ,z,ipl);
  }
  inline void
  atleast(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntPropLevel ipl) {
    count(home,x,y,IRT_GQ,z,ipl);
  }

  inline void
  exactly(Home home, const IntVarArgs& x, int n, int m,
          IntPropLevel ipl) {
    count(home,x,n,IRT_EQ,m,ipl);
  }
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, int m,
          IntPropLevel ipl) {
    count(home,x,y,IRT_EQ,m,ipl);
  }
  inline void
  exactly(Home home, const IntVarArgs& x, const IntArgs& y, int m,
          IntPropLevel ipl) {
    count(home,x,y,IRT_EQ,m,ipl);
  }
  inline void
  exactly(Home home, const IntVarArgs& x, int n, IntVar z,
          IntPropLevel ipl) {
    count(home,x,n,IRT_EQ,z,ipl);
  }
  inline void
  exactly(Home home, const IntVarArgs& x, IntVar y, IntVar z,
          IntPropLevel ipl) {
    count(home,x,y,IRT_EQ,z,ipl);
  }
  inline void
  exactly(Home home, const IntVarArgs& x, const IntArgs& y, IntVar z,
          IntPropLevel ipl) {
    count(home,x,y,IRT_EQ,z,ipl);
  }

  inline void
  lex(Home home, const IntVarArgs& x, IntRelType r, const IntVarArgs& y,
      IntPropLevel ipl) {
    rel(home,x,r,y,ipl);
  }
  inline void
  lex(Home home, const BoolVarArgs& x, IntRelType r, const BoolVarArgs& y,
      IntPropLevel ipl) {
    rel(home,x,r,y,ipl);
  }

  inline void
  values(Home home, const IntVarArgs& x, IntSet y,
         IntPropLevel ipl) {
    dom(home,x,y,ipl);
    nvalues(home,x,IRT_EQ,static_cast<int>(y.size()),ipl);
  }

#ifdef GECODE_HAS_SET_VARS

  inline void
  channel(Home home, const IntVarArgs& x, SetVar y) {
    rel(home,SOT_UNION,x,y);
    nvalues(home,x,IRT_EQ,expr(home,cardinality(y)));
  }

  inline void
  range(Home home, const IntVarArgs& x, SetVar y, SetVar z) {
    element(home,SOT_UNION,x,y,z);
  }

  inline void
  roots(Home home, const IntVarArgs& x, SetVar y, SetVar z) {
    SetVarArgs xiv(home,z.lubMax()+1,IntSet::empty,0,x.size()-1);
    channel(home,x,xiv);
    element(home,SOT_UNION,xiv,z,y);
  }

#endif

}

// STATISTICS: minimodel-any

