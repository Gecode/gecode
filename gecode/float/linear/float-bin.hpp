/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2012
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

namespace Gecode { namespace Float { namespace Linear {

  /*
   * Binary linear propagators
   *
   */
  template<class A, class B, PropCond pc>
  forceinline
  LinBin<A,B,pc>::LinBin(Home home, A y0, B y1, FloatInterval c0)
    : Propagator(home), x0(y0), x1(y1), c(c0) {
    x0.subscribe(home,*this,pc);
    x1.subscribe(home,*this,pc);
  }

  template<class A, class B, PropCond pc>
  forceinline
  LinBin<A,B,pc>::LinBin(Space& home, bool share, LinBin<A,B,pc>& p)
    : Propagator(home,share,p), c(p.c) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
  }

  template<class A, class B, PropCond pc>
  forceinline
  LinBin<A,B,pc>::LinBin(Space& home, bool share, Propagator& p,
                             A y0, B y1, FloatInterval c0)
    : Propagator(home,share,p), c(c0) {
    x0.update(home,share,y0);
    x1.update(home,share,y1);
  }

  template<class A, class B, PropCond pc>
  PropCost
  LinBin<A,B,pc>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class A, class B, PropCond pc>
  forceinline size_t
  LinBin<A,B,pc>::dispose(Space& home) {
    x0.cancel(home,*this,pc);
    x1.cancel(home,*this,pc);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  /*
   * Binary reified linear propagators
   *
   */
  template<class A, class B, PropCond pc, class Ctrl>
  forceinline
  ReLinBin<A,B,pc,Ctrl>::ReLinBin(Home home, A y0, B y1, FloatVal c0, Ctrl b0)
    : Propagator(home), x0(y0), x1(y1), c(c0), b(b0) {
    x0.subscribe(home,*this,pc);
    x1.subscribe(home,*this,pc);
    b.subscribe(home,*this,Int::PC_BOOL_VAL);
  }

  template<class A, class B, PropCond pc, class Ctrl>
  forceinline
  ReLinBin<A,B,pc,Ctrl>::ReLinBin(Space& home, bool share,
                                      ReLinBin<A,B,pc,Ctrl>& p)
    : Propagator(home,share,p), c(p.c) {
    x0.update(home,share,p.x0);
    x1.update(home,share,p.x1);
    b.update(home,share,p.b);
  }

  template<class A, class B, PropCond pc, class Ctrl>
  PropCost
  ReLinBin<A,B,pc,Ctrl>::cost(const Space&, const ModEventDelta&) const {
    return PropCost::binary(PropCost::LO);
  }

  template<class A, class B, PropCond pc, class Ctrl>
  forceinline size_t
  ReLinBin<A,B,pc,Ctrl>::dispose(Space& home) {
    x0.cancel(home,*this,pc);
    x1.cancel(home,*this,pc);
    b.cancel(home,*this,Int::PC_BOOL_VAL);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }


  /*
   * Binary bounds consistent linear equality
   *
   */

  template<class A, class B>
  forceinline
  EqBin<A,B>::EqBin(Home home, A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  EqBin<A,B>::post(Home home, A x0, B x1, FloatInterval c) {
    (void) new (home) EqBin<A,B>(home,x0,x1,c);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  EqBin<A,B>::EqBin(Space& home, bool share, EqBin<A,B>& p)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  forceinline
  EqBin<A,B>::EqBin(Space& home, bool share, Propagator& p,
                        A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p,x0,x1,c) {}

  template<class A, class B>
  Actor*
  EqBin<A,B>::copy(Space& home, bool share) {
    return new (home) EqBin<A,B>(home,share,*this);
  }

  /// Describe which view has been modified how
  enum BinMod {
    BM_X0_MIN = 1<<0,
    BM_X0_MAX = 1<<1,
    BM_X1_MIN = 1<<2,
    BM_X1_MAX = 1<<3,
    BM_ALL    = BM_X0_MIN|BM_X0_MAX|BM_X1_MIN|BM_X1_MAX
  };

#define GECODE_FLOAT_PV(CASE,TELL,UPDATE)       \
  if (bm & (CASE)) {                            \
    bm -= (CASE); ModEvent me = (TELL);         \
    if (me_failed(me))   return ES_FAILED;      \
    if (me_modified(me)) bm |= (UPDATE);        \
  }

  template<class A, class B>
  ExecStatus
  EqBin<A,B>::propagate(Space& home, const ModEventDelta&) {
    int bm = BM_ALL;
    do {
      GECODE_FLOAT_PV(BM_X0_MIN, x0.gq(home,c - x1.max()), BM_X1_MAX);
      GECODE_FLOAT_PV(BM_X1_MIN, x1.gq(home,c - x0.max()), BM_X0_MAX);
      GECODE_FLOAT_PV(BM_X0_MAX, x0.lq(home,c - x1.min()), BM_X1_MIN);
      GECODE_FLOAT_PV(BM_X1_MAX, x1.lq(home,c - x0.min()), BM_X0_MIN);
    } while (bm);
    return x0.assigned() ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

#undef GECODE_FLOAT_PV


  /*
   * Reified binary bounds consistent linear equality
   *
   */

  template<class A, class B, class Ctrl, ReifyMode rm>
  forceinline
  ReEqBin<A,B,Ctrl,rm>::ReEqBin(Home home, A x0, B x1, FloatVal c, Ctrl b)
    : ReLinBin<A,B,PC_FLOAT_BND,Ctrl>(home,x0,x1,c,b) {}

  template<class A, class B, class Ctrl, ReifyMode rm>
  ExecStatus
  ReEqBin<A,B,Ctrl,rm>::post(Home home, A x0, B x1, FloatVal c, Ctrl b) {
    (void) new (home) ReEqBin<A,B,Ctrl,rm>(home,x0,x1,c,b);
    return ES_OK;
  }


  template<class A, class B, class Ctrl, ReifyMode rm>
  forceinline
  ReEqBin<A,B,Ctrl,rm>::ReEqBin(Space& home, bool share,
                                ReEqBin<A,B,Ctrl,rm>& p)
    : ReLinBin<A,B,PC_FLOAT_BND,Ctrl>(home,share,p) {}

  template<class A, class B, class Ctrl, ReifyMode rm>
  Actor*
  ReEqBin<A,B,Ctrl,rm>::copy(Space& home, bool share) {
    return new (home) ReEqBin<A,B,Ctrl,rm>(home,share,*this);
  }

  template<class A, class B, class Ctrl, ReifyMode rm>
  ExecStatus
  ReEqBin<A,B,Ctrl,rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.zero())
      GECODE_REWRITE(*this,(NqBin<A,B>::post(home(*this),x0,x1,c)));
    if (b.one())
      GECODE_REWRITE(*this,(EqBin<A,B>::post(home(*this),x0,x1,c)));
    if ((c < x0.min() + x1.min()) || (c > x0.max() + x1.max())) {
      GECODE_ME_CHECK(b.zero_none(home)); return home.ES_SUBSUMED(*this);
    }
    if (x0.assigned() && x1.assigned()) {
      assert(x0.val() + x1.val() == c);
      GECODE_ME_CHECK(b.one_none(home)); return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }


  /*
   * Binary domain consistent linear disequality
   *
   */
  template<class A, class B>
  forceinline
  NqBin<A,B>::NqBin(Home home, A x0, B x1, FloatVal c)
    : LinBin<A,B,PC_FLOAT_VAL>(home,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  NqBin<A,B>::post(Home home, A x0, B x1, FloatVal c) {
    (void) new (home) NqBin<A,B>(home,x0,x1,c);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  NqBin<A,B>::NqBin(Space& home, bool share, NqBin<A,B>& p)
    : LinBin<A,B,PC_FLOAT_VAL>(home,share,p) {}

  template<class A, class B>
  Actor*
  NqBin<A,B>::copy(Space& home, bool share) {
    return new (home) NqBin<A,B>(home,share,*this);
  }

  template<class A, class B>
  forceinline
  NqBin<A,B>::NqBin(Space& home, bool share, Propagator& p,
                    A x0, B x1, FloatVal c)
    : LinBin<A,B,PC_FLOAT_VAL>(home,share,p,x0,x1,c) {}


  template<class A, class B>
  ExecStatus
  NqBin<A,B>::propagate(Space& home, const ModEventDelta&) {
    if (x0.assigned()) {
      GECODE_ME_CHECK(x1.nq(home,c-x0.val()));
    } else {
      assert(x1.assigned());
      GECODE_ME_CHECK(x0.nq(home,c-x1.val()));
    }
    return ES_FIX;
  }



  /*
   * Binary domain consistent less equal
   *
   */

  template<class A, class B>
  forceinline
  LqBin<A,B>::LqBin(Home home, A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  LqBin<A,B>::post(Home home, A x0, B x1, FloatInterval c) {
    (void) new (home) LqBin<A,B>(home,x0,x1,c);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  LqBin<A,B>::LqBin(Space& home, bool share, LqBin<A,B>& p)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  LqBin<A,B>::copy(Space& home, bool share) {
    return new (home) LqBin<A,B>(home,share,*this);
  }

  template<class A, class B>
  forceinline
  LqBin<A,B>::LqBin(Space& home, bool share, Propagator& p,
                        A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  LqBin<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x0.lq(home,c - x1.min()));
    GECODE_ME_CHECK(x1.lq(home,c - x0.min()));
    return (c >= RND.add_up(x0.max(),x1.max())) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }




  /*
   * Binary domain consistent greater equal
   *
   */

  template<class A, class B>
  forceinline
  GqBin<A,B>::GqBin(Home home, A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  GqBin<A,B>::post(Home home, A x0, B x1, FloatInterval c) {
    (void) new (home) GqBin<A,B>(home,x0,x1,c);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  GqBin<A,B>::GqBin(Space& home, bool share, GqBin<A,B>& p)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  GqBin<A,B>::copy(Space& home, bool share) {
    return new (home) GqBin<A,B>(home,share,*this);
  }

  template<class A, class B>
  forceinline
  GqBin<A,B>::GqBin(Space& home, bool share, Propagator& p,
                        A x0, B x1, FloatInterval c)
    : LinBin<A,B,PC_FLOAT_BND>(home,share,p,x0,x1,c) {}

  template<class A, class B>
  ExecStatus
  GqBin<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x0.gq(home,c - x1.max()));
    GECODE_ME_CHECK(x1.gq(home,c - x0.max()));
    return (c <= RND.add_down(x0.min(),x1.min())) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Reified binary domain consistent less equal
   *
   */

  template<class A, class B, class Ctrl, ReifyMode rm>
  forceinline
  ReLqBin<A,B,Ctrl,rm>::ReLqBin(Home home, A x0, B x1, FloatVal c, Ctrl b)
    : ReLinBin<A,B,PC_FLOAT_BND,Ctrl>(home,x0,x1,c,b) {}

  template<class A, class B, class Ctrl, ReifyMode rm>
  ExecStatus
  ReLqBin<A,B,Ctrl,rm>::post(Home home, A x0, B x1, FloatVal c, Ctrl b) {
    (void) new (home) ReLqBin<A,B,Ctrl,rm>(home,x0,x1,c,b);
    return ES_OK;
  }


  template<class A, class B, class Ctrl, ReifyMode rm>
  forceinline
  ReLqBin<A,B,Ctrl,rm>::ReLqBin(Space& home, bool share, ReLqBin<A,B,Ctrl,rm>& p)
    : ReLinBin<A,B,PC_FLOAT_BND,Ctrl>(home,share,p) {}

  template<class A, class B, class Ctrl, ReifyMode rm>
  Actor*
  ReLqBin<A,B,Ctrl,rm>::copy(Space& home, bool share) {
    return new (home) ReLqBin<A,B,Ctrl,rm>(home,share,*this);
  }

  template<class A, class B, class Ctrl, ReifyMode rm>
  ExecStatus
  ReLqBin<A,B,Ctrl,rm>::propagate(Space& home, const ModEventDelta&) {
    if (b.one())
      GECODE_REWRITE(*this,(LqBin<A,B>::post(home(*this),x0,x1,c)));
    if (b.zero())
    {
      GECODE_ES_CHECK((NqBin<A,B>::post(home(*this),x0,x1,c)));
      GECODE_REWRITE(*this,(GqBin<A,B>::post(home(*this),x0,x1,c)));
    }
    if (c >= x0.max() + x1.max()) {
      GECODE_ME_CHECK(b.one_none(home)); return home.ES_SUBSUMED(*this);
    }
    if (c < x0.min() + x1.min()) {
      GECODE_ME_CHECK(b.zero_none(home)); return home.ES_SUBSUMED(*this);
    }
    return ES_FIX;
  }

}}}

// STATISTICS: float-prop

