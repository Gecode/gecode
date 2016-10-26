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
 * This file is based on gecode/int/bool/eqv.hpp
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

  /*
   * Quantified Boolean equivalence propagator
   *
   */

  template<class BVA, class BVB, class BVC>
  forceinline
  QXorv<BVA,BVB,BVC>::QXorv(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2)
    : BoolTernary<BVA,BVB,BVC>(home,b0,b1,b2), q0(_q0), r0(_r0), q1(_q1), r1(_r1) {}

  template<class BVA, class BVB, class BVC>
  forceinline
  QXorv<BVA,BVB,BVC>::QXorv(Space& home, bool share, QXorv<BVA,BVB,BVC>& p)
    : BoolTernary<BVA,BVB,BVC>(home,share,p), q0(p.q0), r0(p.r0), q1(p.q1), r1(p.r1) {}

  template<class BVA, class BVB, class BVC>
  inline ExecStatus
  QXorv<BVA,BVB,BVC>::post(Home home, BVA b0, TQuantifier _q0, int _r0, BVB b1, TQuantifier _q1, int _r1, BVC b2) {
    switch (bool_test(b0,b1)) {
    case BT_SAME:
      GECODE_ME_CHECK(b2.zero(home)); break;
    case BT_COMP:
      GECODE_ME_CHECK(b2.one(home)); break;
    case BT_NONE:
      if (  ((_q0 == FORALL) && (_q1 == FORALL))
         || ((_r0 < _r1) && (_q1 == FORALL))
         || ((_r0 > _r1) && (_q0 == FORALL)) )
      {
        GECODE_ME_CHECK(b2.zero(home)); return ES_OK;
      }
      if (b2.one())
        return QXor<BVA,BVB>::post(home,b0,_q0,_r0,b1,_q1,_r1);
      if (b2.zero())
        return QEq<BVA,BVB>::post(home,b0,_q0,_r0,b1,_q1,_r1);
      if (b0.one()) {
        if (b1.zero()) {
          GECODE_ME_CHECK(b2.one(home)); return ES_OK;
        } else if (b1.one()) {
          GECODE_ME_CHECK(b2.zero(home)); return ES_OK;
        }
      }
      if (b0.zero()) {
        if (b1.one()) {
          GECODE_ME_CHECK(b2.one(home)); return ES_OK;
        } else if (b1.zero()) {
          GECODE_ME_CHECK(b2.zero(home)); return ES_OK;
        }
      }
      (void) new (home) QXorv(home,b0,_q0,_r0,b1,_q1,_r1,b2);
      break;
    default:
      GECODE_NEVER;
    }
    return ES_OK;
  }

  template<class BVA, class BVB, class BVC>
  inline ExecStatus
  QXorv<BVA,BVB,BVC>::post(Home home, QBoolVar b0, QBoolVar b1, BVC b2) {
    return post(home,b0.x,b0.q,b0.r,b1.x,b1.q,b1.r,b2);
  }

  template<class BVA, class BVB, class BVC>
  Actor*
  QXorv<BVA,BVB,BVC>::copy(Space& home, bool share) {
    return new (home) QXorv<BVA,BVB,BVC>(home,share,*this);
  }

  template<class BVA, class BVB, class BVC>
  ExecStatus
  QXorv<BVA,BVB,BVC>::propagate(Space& home, const ModEventDelta&) {
#define GECODE_INT_STATUS(S0,S1,S2) \
  ((BVA::S0<<(2*BVA::BITS))|(BVB::S1<<(1*BVB::BITS))|(BVC::S2<<(0*BVC::BITS)))
    switch ((x0.status() << (2*BVA::BITS)) | (x1.status() << (1*BVB::BITS)) |
            (x2.status() << (0*BVC::BITS))) {
    case GECODE_INT_STATUS(NONE,NONE,NONE):
      GECODE_NEVER;
    case GECODE_INT_STATUS(NONE,NONE,ZERO):
    case GECODE_INT_STATUS(NONE,NONE,ONE):
      return ES_FIX;
    case GECODE_INT_STATUS(NONE,ZERO,NONE):
      if (q0 == FORALL)
      {
        GECODE_ME_CHECK(x2.zero_none(home)); break;
      }
      return ES_FIX;
    case GECODE_INT_STATUS(NONE,ZERO,ZERO):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.zero_none(home)); break;
    case GECODE_INT_STATUS(NONE,ZERO,ONE):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE,NONE):
      if (q0 == FORALL)
      {
        GECODE_ME_CHECK(x2.zero_none(home)); break;
      }
      return ES_FIX;
    case GECODE_INT_STATUS(NONE,ONE,ZERO):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.one_none(home)); break;
    case GECODE_INT_STATUS(NONE,ONE,ONE):
      if (q0 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x0.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,NONE,NONE):
      if (q1 == FORALL)
      {
        GECODE_ME_CHECK(x2.zero_none(home)); break;
      }
      return ES_FIX;
    case GECODE_INT_STATUS(ZERO,NONE,ZERO):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,NONE,ONE):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO,NONE):
      GECODE_ME_CHECK(x2.zero_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ZERO,ZERO):
      break;
    case GECODE_INT_STATUS(ZERO,ZERO,ONE):
      return ES_FAILED;
    case GECODE_INT_STATUS(ZERO,ONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ZERO,ONE,ZERO):
      break;
    case GECODE_INT_STATUS(ZERO,ONE,ONE):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,NONE,NONE):
      if (q1 == FORALL)
      {
        GECODE_ME_CHECK(x2.zero_none(home)); break;
      }
      return ES_FIX;
    case GECODE_INT_STATUS(ONE,NONE,ZERO):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,NONE,ONE):
      if (q1 == FORALL) return ES_FAILED;
      GECODE_ME_CHECK(x1.zero_none(home)); break;
    case GECODE_INT_STATUS(ONE,ZERO,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,ZERO,ZERO):
      return ES_FAILED;
    case GECODE_INT_STATUS(ONE,ZERO,ONE):
      break;
    case GECODE_INT_STATUS(ONE,ONE,NONE):
      GECODE_ME_CHECK(x2.one_none(home)); break;
    case GECODE_INT_STATUS(ONE,ONE,ZERO):
      break;
    case GECODE_INT_STATUS(ONE,ONE,ONE):
      return ES_FAILED;
    default:
      GECODE_NEVER;
    }
    return home.ES_SUBSUMED(*this);
#undef GECODE_INT_STATUS
  }


}}}

// STATISTICS: int-prop
