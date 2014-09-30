/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
 *
 * This file is based on gecode/int/bool/eq.hpp
 * and is under the same license as given below:
 *
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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

namespace Gecode { namespace Int { namespace Bool {

  template<class BVA, class BVB>
  forceinline
  QXor<BVA,BVB>::QXor(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1)
    : BoolBinary<BVA,BVB>(home,b0,b1), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB>
  forceinline
  QXor<BVA,BVB>::QXor(Space& home, bool share, QXor<BVA,BVB>& p)
    : BoolBinary<BVA,BVB>(home,share,p), q0(p.q0), r0(p.r0), q1(p.q1), r1(p.r1) {}

  template<class BVA, class BVB>
  forceinline
  QXor<BVA,BVB>::QXor(Space& home, bool share, Propagator& p,
                    BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1)
    : BoolBinary<BVA,BVB>(home,share,p,b0,b1), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB>
  Actor*
  QXor<BVA,BVB>::copy(Space& home, bool share) {
    return new (home) QXor<BVA,BVB>(home,share,*this);
  }

  template<class BVA, class BVB>
  inline ExecStatus
  QXor<BVA,BVB>::post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1) {
    switch (bool_test(b0,b1)) {
    case BT_SAME: return ES_FAILED;
    case BT_COMP: return ES_OK;
    case BT_NONE:
      if (  ((_q0 == FORALL) && (_q1 == FORALL))
         || ((_r0 < _r1) && (_q1 == FORALL))
         || ((_r0 > _r1) && (_q0 == FORALL)) )
        return ES_FAILED;
      if (b0.zero()) {
        if (_q1 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b1.one(home));
      } else if (b0.one()) {
        if (_q1 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b1.zero(home));
      } else if (b1.zero()) {
        if (_q0 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b0.one(home));
      } else if (b1.one()) {
        if (_q0 == FORALL) return ES_FAILED;
        GECODE_ME_CHECK(b0.zero(home));
      } else {
        (void) new (home) QXor<BVA,BVB>(home,b0,_q0,_r0,b1,_q1,_r1);
      }
      break;
    default: GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class BVA, class BVB>
  inline ExecStatus
  QXor<BVA,BVB>::post(Home home, QBoolVar b0, QBoolVar b1){
    return post(home,b0.x,b0.q,b0.r,b1.x,b1.q,b1.r);
  }

  template<class BVA, class BVB>
  ExecStatus
  QXor<BVA,BVB>::propagate(Space& home, const ModEventDelta&) {
#define GECODE_INT_STATUS(S0,S1) \
  ((BVA::S0<<(1*BVA::BITS))|(BVB::S1<<(0*BVB::BITS)))
    switch ((x0.status() << (1*BVA::BITS)) | (x1.status() << (0*BVB::BITS))) {
    case GECODE_INT_STATUS(NONE,NONE):
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,ZERO):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,NONE):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ZERO,ONE):
      break;
    case GECODE_INT_STATUS(ONE,NONE):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.zero_none(home)); break;
    case GECODE_INT_STATUS(ONE,ZERO):
      break;
    case GECODE_INT_STATUS(ONE,ONE):
      return ES_FAILED;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
#undef GECODE_INT_STATUS
  }

}}}

// STATISTICS: int-prop
