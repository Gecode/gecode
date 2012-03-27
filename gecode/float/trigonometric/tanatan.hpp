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

namespace Gecode { namespace Float { namespace Trigonometric {

  /*
   * Bounds consistent tangent operator
   *
   */

  template<class A, class B>
  forceinline
  Tan<A,B>::Tan(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Tan<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Tan<A,B>(home,x0,x1);
    return ES_OK;
  }

  template<class A, class B>
  forceinline
  Tan<A,B>::Tan(Space& home, bool share, Tan<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  Tan<A,B>::copy(Space& home, bool share) {
    return new (home) Tan<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Tan<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,tan(x0.domain())));
    FloatInterval iv = fmod(x0.domain(),boost::numeric::interval_lib::pi<FloatInterval>());
    FloatNum offSet(RND.sub_down(x0.min(),iv.lower()));

    using namespace boost::numeric;
    using namespace boost::numeric::interval_lib;
    using namespace boost::numeric::interval_lib::constants;
#define I0__PI_2I    FloatInterval(0,pi_half_upper<FloatNum>())
#define IPI_2__PII   FloatInterval(pi_half_lower<FloatNum>(),pi_upper<FloatNum>())
#define POS(X) ((in(X,I0__PI_2I))?0:1)
#define CASE(X,Y) case ((X << 2) | Y) :
#define SHIFTN_UP(N,X) RND.add_up(RND.mul_up(N,pi_upper<FloatNum>()),X)
#define GROWING(I) RND.tan_down(iv.lower()) <= RND.tan_up(iv.upper())
#define ATANINF_DOWN RND.atan_down(x1.min())
#define ATANSUP_UP RND.atan_up(x1.max())
#define PI_UP pi_upper<FloatNum>()
#define PI_DOWN pi_lower<FloatNum>()
#define PI_TWICE_DOWN pi_twice_lower<FloatNum>()

    int n = iv.upper() / pi_lower<FloatNum>();
    // 0 <=> in [0;PI/2]
    // 1 <=> in [PI/2;PI]
    switch ( (POS(iv.lower()) << 2) | POS(RND.sub_up(iv.upper(),RND.mul_up(n,PI_UP))) )
    {
      CASE(0,0)
        if (GROWING(iv)) iv.assign(ATANINF_DOWN,SHIFTN_UP(n,ATANSUP_UP));
        else  if (RND.tan_down(iv.lower()) <= x1.max())
                if (RND.tan_up(iv.upper()) >= x1.min()) break; // Nothing changed
                else iv.assign(iv.lower(),SHIFTN_UP(n-1, ATANSUP_UP));
              else
                if (RND.tan_up(iv.upper()) >= x1.min()) iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), iv.upper());
                else { if (n <= 1) return ES_FAILED; else iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), SHIFTN_UP(n-1, ATANSUP_UP)); }
        break;
      CASE(0,1)
        if (RND.tan_down(iv.lower()) <= x1.max())
          if (RND.tan_up(iv.upper()) >= x1.min()) break; // Nothing changed
          else iv.assign(iv.lower(), SHIFTN_UP(n,ATANSUP_UP));
        else
          if (RND.tan_up(iv.upper()) >= x1.min()) iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), iv.upper());
          else { if (n <= 1) return ES_FAILED; else iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), SHIFTN_UP(n,ATANSUP_UP)); }
        break;
      CASE(1,0)
        iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), SHIFTN_UP(n,ATANSUP_UP));
        break;
      CASE(1,1)
        if (GROWING(iv)) iv.assign(RND.add_down(PI_DOWN, ATANINF_DOWN), SHIFTN_UP(n+1,ATANSUP_UP));
        else  if (RND.tan_down(iv.lower()) <= x1.max())
                if (RND.tan_up(iv.upper()) >= x1.min()) break; // Nothing changed
                else iv.assign(iv.lower(), SHIFTN_UP(n,ATANSUP_UP));
              else
                if (RND.tan_up(iv.upper()) >= x1.min()) iv.assign(RND.add_down(PI_TWICE_DOWN, ATANINF_DOWN), iv.upper());
                else { if (n <= 1) return ES_FAILED; iv.assign(RND.add_down(PI_TWICE_DOWN, ATANINF_DOWN), SHIFTN_UP(n,ATANSUP_UP)); }
        break;
      default:
        GECODE_NEVER;
        break;
    }
#undef PI_TWICE_DOWN
#undef PI_UP
#undef PI_DOWN
#undef ATANINF_DOWN
#undef ATANSUP_UP
#undef GROWING
#undef SHIFTN_UP
#undef CASE
#undef POS
#undef I0__PI_2I
#undef IPI_2__PII
    GECODE_ME_CHECK(x0.eq(home,iv + offSet));
    return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

  /*
   * Bounds consistent arc tangent operator
   *
   */

  template<class A, class B>
  forceinline
  ATan<A,B>::ATan(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  ATan<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) ATan<A,B>(home,x0,x1);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  ATan<A,B>::ATan(Space& home, bool share, ATan<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  ATan<A,B>::copy(Space& home, bool share) {
    return new (home) ATan<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  ATan<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,atan(x0.domain())));
    GECODE_ME_CHECK(x0.eq(home,tan(x1.domain())));
    return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: float-prop

