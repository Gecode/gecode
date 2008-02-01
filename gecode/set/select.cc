/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
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

#include "gecode/set/select.hh"

namespace Gecode {

  using namespace Gecode::Set;

  void
  selectUnion(Space* home, const SetVarArgs& x, SetVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,
                   (Select::SelectUnion<SetView,SetView>::
                    post(home,z,iv,y)));
  }

  void
  selectUnion(Space* home, const IntSetArgs& s, SetVar y, SetVar z) {
    SharedArray<IntSet> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);
    if (home->failed()) return;
    GECODE_ES_FAIL(home,
                   (Select::SelectUnionConst<SetView,SetView>::
                    post(home,z,x,y)));
  }

  void
  selectInter(Space* home, const SetVarArgs& x, SetVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    IntSet universe(Set::Limits::int_min,
                    Set::Limits::int_max);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
                    post(home,z,iv,y,universe)));
  }

  void
  selectInterIn(Space* home, const SetVarArgs& x, SetVar y, SetVar z,
                const IntSet& universe) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,
                   (Select::SelectIntersection<SetView,SetView>::
                    post(home,z,iv,y,universe)));
  }

  void
  selectSet(Space* home, const SetVarArgs& x, IntVar y, SetVar z) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView > iv(home, x);
    SetView zv(z);

    Int::IntView yv(y);
    SingletonView single(yv);
    GECODE_ES_FAIL(home,(Select::SelectUnion<SetView,
                         SingletonView>::post(home, z, iv, single)));
  }

  void
  selectSet(Space* home, const IntSetArgs& s, IntVar y, SetVar z) {
    if (home->failed()) return;
    SetView zv(z);
    SharedArray<IntSet> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);

    Int::IntView yv(y);
    SingletonView single(yv);
    GECODE_ES_FAIL(home,(Select::SelectUnionConst<SetView,
                         SingletonView>::post(home, z, x, single)));
  }

  void
  selectDisjoint(Space* home, const SetVarArgs& x, SetVar y) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,Select::SelectDisjoint::post(home,iv,y));
  }

  namespace {
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::SingletonView>);
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::SetView>);
    GECODE_REGISTER1(Select::SelectDisjoint);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::SingletonView>);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::SetView>);
    GECODE_REGISTER2(Select::SelectIntersection<Gecode::Set::SetView, Gecode::Set::SetView>);
    
  }
  
}

// STATISTICS: set-post

