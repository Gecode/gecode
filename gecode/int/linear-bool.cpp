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

#include <gecode/int/linear.hh>

namespace Gecode {

  using namespace Int;

  void
  linear(Space& home, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel icl) {
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,icl);
  }

  void
  linear(Space& home, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel icl) {
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y,0,icl);
  }

  void
  linear(Space& home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home.failed()) 
      return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,icl);
  }

  void
  linear(Space& home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home.failed()) 
      return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t =
      re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }
    int min, max;
    estimate(t,n,0,min,max);
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
    if (home.failed()) return;
    Linear::post(home,t,n,r,y,0,icl);
  }

  void
  linear(Space& home, const BoolVarArgs& x, IntRelType r, int c,
         BoolVar b, IntConLevel icl) {
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,b,icl);
  }

  void
  linear(Space& home, const BoolVarArgs& x, IntRelType r, IntVar y,
         BoolVar b, IntConLevel icl) {
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=1; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y,b,icl);
  }

  void
  linear(Space& home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, int c,
         BoolVar b, IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,c,b,icl);
  }

  void
  linear(Space& home,
         const IntArgs& a, const BoolVarArgs& x, IntRelType r, IntVar y,
         BoolVar b, IntConLevel icl) {
    if (a.size() != x.size())
      throw ArgumentSizeMismatch("Int::linear");
    if (home.failed()) return;

    int n=x.size();
    Region re(home);
    Linear::Term<BoolView>* t = re.alloc<Linear::Term<BoolView> >(n);
    for (int i=n; i--; ) {
      t[i].a=a[i]; t[i].x=x[i];
    }

    Linear::post(home,t,n,r,y,b,icl);
  }


  namespace {
    using namespace Int;
    GECODE_REGISTER1(Linear::EqBoolInt<BoolView>::Memory);
    GECODE_REGISTER1(Linear::EqBoolInt<BoolView>::Speed);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::EqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER2(Linear::EqBoolView<BoolView,IntView>);
    GECODE_REGISTER2(Linear::EqBoolView<BoolView,MinusView>);
    GECODE_REGISTER1(Linear::GqBoolInt<BoolView>::Memory);
    GECODE_REGISTER1(Linear::GqBoolInt<BoolView>::Speed);
    GECODE_REGISTER1(Linear::GqBoolInt<NegBoolView>::Memory);
    GECODE_REGISTER1(Linear::GqBoolInt<NegBoolView>::Speed);
    GECODE_REGISTER2(Linear::GqBoolView<BoolView,IntView>);
    GECODE_REGISTER2(Linear::GqBoolView<BoolView,MinusView>);
    GECODE_REGISTER2(Linear::GqBoolView<NegBoolView,IntView>);
    GECODE_REGISTER2(Linear::GqBoolView<NegBoolView,MinusView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,MinusView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,MinusView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,MinusView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,MinusView>);
    GECODE_REGISTER3(Linear::LqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER1(Linear::NqBoolInt<BoolView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::EmptyScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::EmptyScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::EmptyScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,IntView>);
    GECODE_REGISTER3(Linear::NqBoolScale<Linear::ScaleBoolArray,Linear::ScaleBoolArray,ZeroIntView>);
    GECODE_REGISTER2(Linear::NqBoolView<BoolView,IntView>);
    GECODE_REGISTER2(Linear::NqBoolView<BoolView,MinusView>);
  }

}

// STATISTICS: int-post
