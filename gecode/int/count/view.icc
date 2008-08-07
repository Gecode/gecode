/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Int { namespace Count {

  /*
   * General baseclass
   *
   */

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  BaseView<VX,VY,VZ,shr>::BaseView(Space& home,
                                   ViewArray<VX>& x0, VY y0, VZ z0, int c0)
    : Propagator(home), x(x0), y(y0), z(z0), c(c0) {
    x.subscribe(home,*this,PC_INT_DOM);
    y.subscribe(home,*this,PC_INT_DOM);
    z.subscribe(home,*this,PC_INT_BND);
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  BaseView<VX,VY,VZ,shr>::BaseView(Space& home, bool share,
                                   BaseView<VX,VY,VZ,shr>& p)
    : Propagator(home,shr,p), c(p.c) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
    z.update(home,share,p.z);
  }

  template <class VX, class VY, class VZ, bool shr>
  PropCost
  BaseView<VX,VY,VZ,shr>::cost(const Space&, const ModEventDelta&) const {
    return cost_lo(x.size()+1, PC_LINEAR_LO);
  }

  template <class VX, class VY, class VZ, bool shr>
  Reflection::ActorSpec
  BaseView<VX,VY,VZ,shr>::spec(const Space& home, Reflection::VarMap& m,
                               const Support::Symbol& ati) const {
    Reflection::ActorSpec s(ati);
    return s << x.spec(home, m)
             << y.spec(home, m)
             << z.spec(home, m)
             << c;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline size_t
  BaseView<VX,VY,VZ,shr>::dispose(Space& home) {
    assert(!home.failed());
    x.cancel(home,*this,PC_INT_DOM);
    y.cancel(home,*this,PC_INT_DOM);
    z.cancel(home,*this,PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline void
  BaseView<VX,VY,VZ,shr>::count(Space& home) {
    int n = x.size();
    for (int i=n; i--; )
      switch (holds(x[i],y)) {
      case RT_FALSE:
        x[i].cancel(home,*this,PC_INT_DOM); x[i]=x[--n];
        break;
      case RT_TRUE:
        x[i].cancel(home,*this,PC_INT_DOM); x[i]=x[--n];
        c--;
        break;
      case RT_MAYBE:
        break;
      default: 
        GECODE_NEVER;
      }
    x.size(n);
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline int
  BaseView<VX,VY,VZ,shr>::atleast(void) const {
    return -c;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline int
  BaseView<VX,VY,VZ,shr>::atmost(void) const {
    return x.size()-c;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline bool
  BaseView<VX,VY,VZ,shr>::sharing(const ViewArray<VX>& x,
                                  const VY& y, const VZ& z) {
    if (shared(y,z))
      return true;
    for (int i = x.size(); i--; )
      if (shared(x[i],z))
        return true;
    return false;
  }

  /*
   * Equality
   *
   */

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  EqView<VX,VY,VZ,shr>::EqView(Space& home, 
                               ViewArray<VX>& x, VY y, VZ z, int c)
    : BaseView<VX,VY,VZ,shr>(home,x,y,z,c) {}

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  EqView<VX,VY,VZ,shr>::post(Space& home, 
                             ViewArray<VX>& x, VY y, VZ z, int c) {
    if (z.assigned())
      return EqInt<VX,VY>::post(home,x,y,z.val()+c);
    if (sharing(x,y,z))
      (void) new (home) EqView<VX,VY,VZ,true>(home,x,y,z,c);
    else
      (void) new (home) EqView<VX,VY,VZ,false>(home,x,y,z,c);
    return ES_OK;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  EqView<VX,VY,VZ,shr>::EqView(Space& home, bool share, 
                               EqView<VX,VY,VZ,shr>& p)
    : BaseView<VX,VY,VZ,shr>(home,share,p) {}

  template <class VX, class VY, class VZ, bool shr>
  Actor*
  EqView<VX,VY,VZ,shr>::copy(Space& home, bool share) {
    return new (home) EqView<VX,VY,VZ,shr>(home,share,*this);
  }

  template <class VX, class VY, class VZ, bool shr>
  Support::Symbol
  EqView<VX,VY,VZ,shr>::ati(void) {
    return Reflection::mangle<VX,VY,VZ>("Gecode::Int::Count::EqView",shr);
  }

  template <class VX, class VY, class VZ, bool shr>
  Reflection::ActorSpec
  EqView<VX,VY,VZ,shr>::spec(const Space& home, Reflection::VarMap& m) const {
    return BaseView<VX,VY,VZ,shr>::spec(home, m, ati());
  }

  template <class VX, class VY, class VZ, bool shr>
  void
  EqView<VX,VY,VZ,shr>::post(Space& home, Reflection::VarMap& vars,
                             const Reflection::ActorSpec& spec) {
    spec.checkArity(4);
    ViewArray<VX> x(home, vars, spec[0]);
    VY y(home, vars, spec[1]);
    VZ z(home, vars, spec[2]);
    int c = spec[3]->toInt();
    (void) new (home) EqView(home, x, y, z, c);
  }

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  EqView<VX,VY,VZ,shr>::propagate(Space& home, const ModEventDelta&) {
    count(home);

    GECODE_ME_CHECK(z.gq(home,atleast()));
    GECODE_ME_CHECK(z.lq(home,atmost()));

    if (z.assigned()) {
      if (z.val() == atleast())
        return post_false(home,x,y) ? ES_FAILED : ES_SUBSUMED(*this,home);
      if (z.val() == atmost())
        return post_true(home,x,y) ? ES_FAILED : ES_SUBSUMED(*this,home);
      GECODE_REWRITE(*this,(EqInt<VX,VY>::post(home,x,y,z.val()+c)));
    }
    return shr ? ES_NOFIX : ES_FIX;
  }




  /*
   * Disequality
   *
   */

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  NqView<VX,VY,VZ,shr>::NqView(Space& home, 
                               ViewArray<VX>& x, VY y, VZ z, int c)
    : BaseView<VX,VY,VZ,shr>(home,x,y,z,c) {}

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  NqView<VX,VY,VZ,shr>::post(Space& home, 
                             ViewArray<VX>& x, VY y, VZ z, int c) {
    if (z.assigned())
      return NqInt<VX,VY>::post(home,x,y,z.val()+c);
    (void) new (home) NqView<VX,VY,VZ,shr>(home,x,y,z,c);
    return ES_OK;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  NqView<VX,VY,VZ,shr>::NqView(Space& home, bool share, 
                               NqView<VX,VY,VZ,shr>& p)
    : BaseView<VX,VY,VZ,shr>(home,share,p) {}

  template <class VX, class VY, class VZ, bool shr>
  Actor*
  NqView<VX,VY,VZ,shr>::copy(Space& home, bool share) {
    return new (home) NqView<VX,VY,VZ,shr>(home,share,*this);
  }

  template <class VX, class VY, class VZ, bool shr>
  Support::Symbol
  NqView<VX,VY,VZ,shr>::ati(void) {
    return Reflection::mangle<VX,VY,VZ>("Gecode::Int::Count::NqView",shr);
  }

  template <class VX, class VY, class VZ, bool shr>
  Reflection::ActorSpec
  NqView<VX,VY,VZ,shr>::spec(const Space& home, Reflection::VarMap& m) const {
    return BaseView<VX,VY,VZ,shr>::spec(home, m, ati());
  }

  template <class VX, class VY, class VZ, bool shr>
  void
  NqView<VX,VY,VZ,shr>::post(Space& home, Reflection::VarMap& vars,
                             const Reflection::ActorSpec& spec) {
    spec.checkArity(4);
    ViewArray<VX> x(home, vars, spec[0]);
    VY y(home, vars, spec[1]);
    VZ z(home, vars, spec[2]);
    int c = spec[3]->toInt();
    (void) new (home) NqView(home, x, y, z, c);
  }

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  NqView<VX,VY,VZ,shr>::propagate(Space& home, const ModEventDelta&) {
    count(home);
    if (atleast() == atmost()) {
      GECODE_ME_CHECK(z.nq(home,atleast()));
      return ES_SUBSUMED(*this,home);
    }
    if (z.max() < atleast())
      return ES_SUBSUMED(*this,home);
    if (z.min() > atmost())
      return ES_SUBSUMED(*this,home);
    if (z.assigned())
      GECODE_REWRITE(*this,(NqInt<VX,VY>::post(home,x,y,z.val()+c)));
    return ES_FIX;
  }



  /*
   * Less or equal
   *
   */

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  LqView<VX,VY,VZ,shr>::LqView(Space& home, ViewArray<VX>& x, 
                               VY y, VZ z, int c)
    : BaseView<VX,VY,VZ,shr>(home,x,y,z,c) {}

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  LqView<VX,VY,VZ,shr>::post(Space& home, ViewArray<VX>& x, 
                             VY y, VZ z, int c) {
    if (z.assigned())
      return LqInt<VX,VY>::post(home,x,y,z.val()+c);
    if (sharing(x,y,z))
      (void) new (home) LqView<VX,VY,VZ,true>(home,x,y,z,c);
    else
      (void) new (home) LqView<VX,VY,VZ,false>(home,x,y,z,c);
    return ES_OK;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  LqView<VX,VY,VZ,shr>::LqView(Space& home, bool share, 
                               LqView<VX,VY,VZ,shr>& p)
    : BaseView<VX,VY,VZ,shr>(home,share,p) {}

  template <class VX, class VY, class VZ, bool shr>
  Actor*
  LqView<VX,VY,VZ,shr>::copy(Space& home, bool share) {
    return new (home) LqView<VX,VY,VZ,shr>(home,share,*this);
  }

  template <class VX, class VY, class VZ, bool shr>
  Support::Symbol
  LqView<VX,VY,VZ,shr>::ati(void) {
    return Reflection::mangle<VX,VY,VZ>("Gecode::Int::Count::LqView",shr);
  }

  template <class VX, class VY, class VZ, bool shr>
  Reflection::ActorSpec
  LqView<VX,VY,VZ,shr>::spec(const Space& home, Reflection::VarMap& m) const {
    return BaseView<VX,VY,VZ,shr>::spec(home, m, ati());
  }

  template <class VX, class VY, class VZ, bool shr>
  void
  LqView<VX,VY,VZ,shr>::post(Space& home, Reflection::VarMap& vars,
                             const Reflection::ActorSpec& spec) {
    spec.checkArity(4);
    ViewArray<VX> x(home, vars, spec[0]);
    VY y(home, vars, spec[1]);
    VZ z(home, vars, spec[2]);
    int c = spec[3]->toInt();
    (void) new (home) LqView(home, x, y, z, c);
  }

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  LqView<VX,VY,VZ,shr>::propagate(Space& home, const ModEventDelta&) {
    count(home);
    GECODE_ME_CHECK(z.gq(home,atleast()));
    if (z.max() == atleast())
      return post_false(home,x,y) ? ES_FAILED : ES_SUBSUMED(*this,home);
    if (x.size() == 0)
      return ES_SUBSUMED(*this,home);
    if (z.assigned())
      GECODE_REWRITE(*this,(LqInt<VX,VY>::post(home,x,y,z.val()+c)));
    return shr ? ES_NOFIX : ES_FIX;
  }



  /*
   * Greater or equal
   *
   */

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  GqView<VX,VY,VZ,shr>::GqView(Space& home, ViewArray<VX>& x, VY y, VZ z, int c)
    : BaseView<VX,VY,VZ,shr>(home,x,y,z,c) {}

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  GqView<VX,VY,VZ,shr>::post(Space& home, 
                             ViewArray<VX>& x, VY y, VZ z, int c) {
    if (z.assigned())
      return GqInt<VX,VY>::post(home,x,y,z.val()+c);
    if (sharing(x,y,z))
      (void) new (home) GqView<VX,VY,VZ,true>(home,x,y,z,c);
    else
      (void) new (home) GqView<VX,VY,VZ,false>(home,x,y,z,c);
    return ES_OK;
  }

  template <class VX, class VY, class VZ, bool shr>
  forceinline
  GqView<VX,VY,VZ,shr>::GqView(Space& home, bool share, 
                               GqView<VX,VY,VZ,shr>& p)
    : BaseView<VX,VY,VZ,shr>(home,share,p) {}

  template <class VX, class VY, class VZ, bool shr>
  Actor*
  GqView<VX,VY,VZ,shr>::copy(Space& home, bool share) {
    return new (home) GqView<VX,VY,VZ,shr>(home,share,*this);
  }

  template <class VX, class VY, class VZ, bool shr>
  Support::Symbol
  GqView<VX,VY,VZ,shr>::ati(void) {
    return Reflection::mangle<VX,VY,VZ>("Gecode::Int::Count::GqView", shr);
  }

  template <class VX, class VY, class VZ, bool shr>
  Reflection::ActorSpec
  GqView<VX,VY,VZ,shr>::spec(const Space& home, Reflection::VarMap& m) const {
    return BaseView<VX,VY,VZ,shr>::spec(home, m, ati());
  }

  template <class VX, class VY, class VZ, bool shr>
  void
  GqView<VX,VY,VZ,shr>::post(Space& home, Reflection::VarMap& vars,
                             const Reflection::ActorSpec& spec) {
    spec.checkArity(4);
    ViewArray<VX> x(home, vars, spec[0]);
    VY y(home, vars, spec[1]);
    VZ z(home, vars, spec[2]);
    int c = spec[3]->toInt();
    (void) new (home) GqView(home, x, y, z, c);
  }

  template <class VX, class VY, class VZ, bool shr>
  ExecStatus
  GqView<VX,VY,VZ,shr>::propagate(Space& home, const ModEventDelta&) {
    count(home);

    GECODE_ME_CHECK(z.lq(home,atmost()));

    if (z.min() == atmost())
      return post_true(home,x,y) ? ES_FAILED : ES_SUBSUMED(*this,home);
    if (x.size() == 0)
      return ES_SUBSUMED(*this,home);
    if (z.assigned())
      GECODE_REWRITE(*this,(GqInt<VX,VY>::post(home,x,y,z.val()+c)));
    return shr ? ES_NOFIX : ES_FIX;
  }

}}}

// STATISTICS: int-prop

