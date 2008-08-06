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

#include "gecode/int/linear.hh"

namespace Gecode {

  using namespace Int;

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, 
         IntConLevel icl, PropKind) {
    if (home->failed()) return;
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size());
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    Linear::post(home,t,x.size(),r,c,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, int c, BoolVar b, 
         IntConLevel, PropKind) {
    if (home->failed()) return;
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size());
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
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size());
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
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size());
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
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=1; t[i].x=x[i];
    }
    int min, max;
    estimate(t,x.size(),0,min,max);
    IntView v(y);
    switch (r) {
    case IRT_EQ:
      GECODE_ME_FAIL(home,v.gq(home,min)); GECODE_ME_FAIL(home,v.lq(home,max));
      break;
    case IRT_GQ:
      GECODE_ME_FAIL(home,v.lq(home,max));
      break;
    case IRT_LQ:
      GECODE_ME_FAIL(home,v.gq(home,min));
      break;
    default: ;
    }
    if (home->failed()) return;
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,icl);
  }

  void
  linear(Space* home,
         const IntVarArgs& x, IntRelType r, IntVar y, BoolVar b,
         IntConLevel, PropKind) {
    if (home->failed()) return;
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size()+1);
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
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    int min, max;
    estimate(t,x.size(),0,min,max);
    IntView v(y);
    switch (r) {
    case IRT_EQ:
      GECODE_ME_FAIL(home,v.gq(home,min)); GECODE_ME_FAIL(home,v.lq(home,max));
      break;
    case IRT_GQ:
      GECODE_ME_FAIL(home,v.lq(home,max));
      break;
    case IRT_LQ:
      GECODE_ME_FAIL(home,v.gq(home,min));
      break;
    default: ;
    }
    if (home->failed()) return;
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
    Scratch s(home);
    Linear::Term<IntView>* t = s.alloc<Linear::Term<IntView> >(x.size()+1);
    for (int i = x.size(); i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    t[x.size()].a=-1; t[x.size()].x=y;
    Linear::post(home,t,x.size()+1,r,0,b);
  }

    namespace {
      using namespace Int;
      GECODE_REGISTER3(Linear::DomEq<double,ScaleView<double,double> >);
      GECODE_REGISTER3(Linear::DomEq<int,ScaleView<int,unsigned int> >);
      GECODE_REGISTER3(Linear::Eq<double,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Eq<double,ScaleView<double,double>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Eq<double,ScaleView<double,double>,ScaleView<double,double> >);
      GECODE_REGISTER3(Linear::Eq<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::Eq<int,IntView,Linear::NoView>);
      GECODE_REGISTER3(Linear::Eq<int,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Eq<int,ScaleView<int,unsigned int>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Eq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int> >);
      GECODE_REGISTER3(Linear::EqBin<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::EqBin<int,IntView,MinusView>);
      GECODE_REGISTER3(Linear::EqBin<int,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::EqTer<int,IntView,IntView,IntView>);
      GECODE_REGISTER4(Linear::EqTer<int,IntView,IntView,MinusView>);
      GECODE_REGISTER4(Linear::EqTer<int,IntView,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::EqTer<int,MinusView,MinusView,MinusView>);
      GECODE_REGISTER3(Linear::GqBin<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::GqBin<int,IntView,MinusView>);
      GECODE_REGISTER3(Linear::GqBin<int,MinusView,MinusView>);
      GECODE_REGISTER3(Linear::Lq<double,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Lq<double,Linear::NoView,ScaleView<double,double> >);
      GECODE_REGISTER4(Linear::Lq<double,ScaleView<double,double>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Lq<double,ScaleView<double,double>,ScaleView<double,double> >);
      GECODE_REGISTER3(Linear::Lq<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::Lq<int,IntView,Linear::NoView>);
      GECODE_REGISTER3(Linear::Lq<int,Linear::NoView,IntView>);
      GECODE_REGISTER3(Linear::Lq<int,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Lq<int,Linear::NoView,ScaleView<int,unsigned int> >);
      GECODE_REGISTER4(Linear::Lq<int,ScaleView<int,unsigned int>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Lq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int> >);
      GECODE_REGISTER3(Linear::LqBin<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::LqBin<int,IntView,MinusView>);
      GECODE_REGISTER3(Linear::LqBin<int,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::LqTer<int,IntView,IntView,IntView>);
      GECODE_REGISTER4(Linear::LqTer<int,IntView,IntView,MinusView>);
      GECODE_REGISTER4(Linear::LqTer<int,IntView,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::LqTer<int,MinusView,MinusView,MinusView>);
      GECODE_REGISTER3(Linear::Nq<double,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Nq<double,ScaleView<double,double>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Nq<double,ScaleView<double,double>,ScaleView<double,double> >);
      GECODE_REGISTER3(Linear::Nq<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::Nq<int,IntView,Linear::NoView>);
      GECODE_REGISTER3(Linear::Nq<int,Linear::NoView,Linear::NoView>);
      GECODE_REGISTER4(Linear::Nq<int,ScaleView<int,unsigned int>,Linear::NoView>);
      GECODE_REGISTER5(Linear::Nq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int> >);
      GECODE_REGISTER3(Linear::NqBin<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::NqBin<int,IntView,MinusView>);
      GECODE_REGISTER3(Linear::NqBin<int,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::NqTer<int,IntView,IntView,IntView>);
      GECODE_REGISTER4(Linear::NqTer<int,IntView,IntView,MinusView>);
      GECODE_REGISTER4(Linear::NqTer<int,IntView,MinusView,MinusView>);
      GECODE_REGISTER4(Linear::NqTer<int,MinusView,MinusView,MinusView>);
      GECODE_REGISTER5(Linear::ReEq<double,ScaleView<double,double>,Linear::NoView,BoolView>);
      GECODE_REGISTER5(Linear::ReEq<double,ScaleView<double,double>,Linear::NoView,NegBoolView>);
      GECODE_REGISTER6(Linear::ReEq<double,ScaleView<double,double>,ScaleView<double,double>,BoolView>);
      GECODE_REGISTER6(Linear::ReEq<double,ScaleView<double,double>,ScaleView<double,double>,NegBoolView>);
      GECODE_REGISTER4(Linear::ReEq<int,IntView,IntView,BoolView>);
      GECODE_REGISTER4(Linear::ReEq<int,IntView,IntView,NegBoolView>);
      GECODE_REGISTER4(Linear::ReEq<int,IntView,Linear::NoView,BoolView>);
      GECODE_REGISTER4(Linear::ReEq<int,IntView,Linear::NoView,NegBoolView>);
      GECODE_REGISTER5(Linear::ReEq<int,ScaleView<int,unsigned int>,Linear::NoView,BoolView>);
      GECODE_REGISTER5(Linear::ReEq<int,ScaleView<int,unsigned int>,Linear::NoView,NegBoolView>);
      GECODE_REGISTER6(Linear::ReEq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int>,BoolView>);
      GECODE_REGISTER6(Linear::ReEq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int>,NegBoolView>);
      GECODE_REGISTER4(Linear::ReEqBin<int,IntView,IntView,BoolView>);
      GECODE_REGISTER4(Linear::ReEqBin<int,IntView,IntView,NegBoolView>);
      GECODE_REGISTER4(Linear::ReEqBin<int,IntView,MinusView,BoolView>);
      GECODE_REGISTER4(Linear::ReEqBin<int,IntView,MinusView,NegBoolView>);
      GECODE_REGISTER4(Linear::ReLq<double,Linear::NoView,ScaleView<double,double> >);
      GECODE_REGISTER4(Linear::ReLq<double,ScaleView<double,double>,Linear::NoView>);
      GECODE_REGISTER5(Linear::ReLq<double,ScaleView<double,double>,ScaleView<double,double> >);
      GECODE_REGISTER3(Linear::ReLq<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::ReLq<int,IntView,Linear::NoView>);
      GECODE_REGISTER3(Linear::ReLq<int,Linear::NoView,IntView>);
      GECODE_REGISTER4(Linear::ReLq<int,Linear::NoView,ScaleView<int,unsigned int> >);
      GECODE_REGISTER4(Linear::ReLq<int,ScaleView<int,unsigned int>,Linear::NoView>);
      GECODE_REGISTER5(Linear::ReLq<int,ScaleView<int,unsigned int>,ScaleView<int,unsigned int> >);
      GECODE_REGISTER3(Linear::ReLqBin<int,IntView,IntView>);
      GECODE_REGISTER3(Linear::ReLqBin<int,IntView,MinusView>);
      GECODE_REGISTER3(Linear::ReLqBin<int,MinusView,MinusView>);
    }

}

// STATISTICS: int-post

