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
  selectUnion(Space* home, const SetVarArgs& x, SetVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectUnion<SetView,SetView>::
                      post(home,z,iv,y)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectUnion<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy)));
    }
  }

  void
  selectUnion(Space* home, const IntSetArgs& s, SetVar y, SetVar z, int offset) {
    SharedArray<IntSet> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);
    if (home->failed()) return;
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectUnionConst<SetView,SetView>::
                      post(home,z,x,y)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectUnionConst<SetView,OffsetSetView<SetView> >::
                      post(home,z,x,oy)));
    }
  }

  void
  selectInter(Space* home, const SetVarArgs& x, SetVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    IntSet universe(Limits::Set::int_min,
                    Limits::Set::int_max);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,SetView>::
                      post(home,z,iv,y,universe)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy,universe)));
    }
  }

  void
  selectInterIn(Space* home, const SetVarArgs& x, SetVar y, SetVar z,
                const IntSet& universe, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    if (offset == 0) {
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,SetView>::
                      post(home,z,iv,y,universe)));
    } else {
      SetView vy(y);
      OffsetSetView<SetView> oy(vy, offset);
      GECODE_ES_FAIL(home,
                     (Select::SelectIntersection<SetView,OffsetSetView<SetView> >::
                      post(home,z,iv,oy,universe)));
    }
  }

  void
  selectSet(Space* home, const SetVarArgs& x, IntVar y, SetVar z, int offset) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView > iv(home, x);
    SetView zv(z);

    if (offset == 0) {
      Int::IntView yv(y);
      SingletonView single(yv);
      GECODE_ES_FAIL(home,(Select::SelectUnion<SetView,
                           SingletonView>::post(home, z, iv, single)));
    } else {
      Int::IntView yv(y);
      SingletonView single(yv);
      OffsetSetView<SingletonView> osingle(single, offset);
      GECODE_ES_FAIL(home,(Select::SelectUnion<SetView,
                           OffsetSetView<SingletonView> >::post(home, z, iv,
                                                                osingle)));
      
    }
  }

  void
  selectSet(Space* home, const IntSetArgs& s, IntVar y, SetVar z, int offset) {
    if (home->failed()) return;
    SetView zv(z);
    SharedArray<IntSet> x(s.size());
    for (int i=s.size(); i--;)
      new (&x[i]) IntSet(s[i]);

    if (offset == 0) {
      Int::IntView yv(y);
      SingletonView single(yv);
      GECODE_ES_FAIL(home,(Select::SelectUnionConst<SetView,
                           SingletonView>::post(home, z, x, single)));
    } else {
      Int::IntView yv(y);
      SingletonView single(yv);
      OffsetSetView<SingletonView> osingle(single, offset);
      GECODE_ES_FAIL(home,(Select::SelectUnionConst<SetView,
                           OffsetSetView<SingletonView> >::post(home, z, x,
                                                                osingle)));
      
    }
  }

  void
  selectDisjoint(Space* home, const SetVarArgs& x, SetVar y) {
    if (home->failed()) return;
    Set::Select::IdxViewArray<SetView> iv(home, x);
    GECODE_ES_FAIL(home,Select::SelectDisjoint::post(home,iv,y));
  }

  namespace {
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::OffsetSetView<Gecode::Set::SingletonView> >);
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::OffsetSetView<Gecode::Set::SetView> >);
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::SingletonView>);
    GECODE_REGISTER2(Select::SelectUnion<Gecode::Set::SetView, Gecode::Set::SetView>);
    GECODE_REGISTER1(Select::SelectDisjoint);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::OffsetSetView<Gecode::Set::SingletonView> >);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::OffsetSetView<Gecode::Set::SetView> >);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::SingletonView>);
    GECODE_REGISTER2(Select::SelectUnionConst<Gecode::Set::SetView, Gecode::Set::SetView>);
    GECODE_REGISTER2(Select::SelectIntersection<Gecode::Set::SetView, Gecode::Set::OffsetSetView<Gecode::Set::SetView> >);
    GECODE_REGISTER2(Select::SelectIntersection<Gecode::Set::SetView, Gecode::Set::SetView>);
    
  }
  
}

// STATISTICS: set-post

