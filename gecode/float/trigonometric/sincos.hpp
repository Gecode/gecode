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
   * ASin projection function
   *
   */
  template<class V>
  void aSinProject(const V& aSinIv, FloatInterval& iv) {
    using namespace boost::numeric;
    using namespace boost::numeric::interval_lib;
    using namespace boost::numeric::interval_lib::constants;
#define I0__PI_2I    FloatInterval(0,pi_half_upper<FloatNum>())
#define IPI_2__PII   FloatInterval(pi_half_lower<FloatNum>(),pi_upper<FloatNum>())
#define IPI__3PI_2I  FloatInterval(pi_lower<FloatNum>(),3*pi_half_upper<FloatNum>())
#define I3PI_2__2PII FloatInterval(3*pi_half_lower<FloatNum>(),pi_twice_upper<FloatNum>())
#define POS(X) ((in(X,I0__PI_2I))?0: (in(X,IPI_2__PII))?1: (in(X,IPI__3PI_2I))?2: 3 )
#define CASE(X,Y) case ((X << 2) | Y) :
#define CASE_LABEL(X,Y) case ((X << 2) | Y) : CASE_ ## X ## _ ## Y :
#define SHIFTN_UP(N,X) RND.add_up(RND.mul_up(N,pi_twice_upper<FloatNum>()),X)
#define SHIFTN_DOWN(N,X) RND.add_down(RND.mul_down(N,pi_twice_lower<FloatNum>()),X)
#define GROWING(I) RND.sin_down(iv.lower()) <= RND.sin_up(iv.upper())
#define NOT_GROWING(I) RND.sin_up(iv.lower()) >= RND.sin_down(iv.upper())
#define ASININF_DOWN RND.asin_down(aSinIv.min())
#define ASINSUP_UP RND.asin_up(aSinIv.max())
#define PI_UP pi_upper<FloatNum>()
#define PI_DOWN pi_lower<FloatNum>()
#define PITWICE_UP pi_twice_upper<FloatNum>()
#define PITWICE_DOWN pi_twice_lower<FloatNum>()

    int n = iv.upper() / pi_twice_lower<FloatNum>();
    // 0 <=> in [0;PI/2]
    // 1 <=> in [PI/2;PI]
    // 2 <=> in [PI;3*PI/2]
    // 3 <=> in [3*PI/2;2*PI]
    switch ( (POS(iv.lower()) << 2) | POS(RND.sub_up(iv.upper(),RND.mul_up(n,PITWICE_UP))) )
    {
      CASE_LABEL(0,0)
        if (GROWING(iv)) iv.assign(ASININF_DOWN,SHIFTN_UP(n,ASINSUP_UP));
        else  if (RND.sin_down(iv.lower()) <= aSinIv.max())
                if (RND.sin_up(iv.upper()) >= aSinIv.min()) break; // Nothing changed
                else iv.assign(iv.lower(), SHIFTN_UP(n-1,RND.sub_up(PI_UP, ASININF_DOWN)));
              else
                if (RND.sin_up(iv.upper()) >= aSinIv.min()) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), iv.upper());
                else iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), SHIFTN_UP(n-1,RND.sub_up(PI_UP, ASININF_DOWN)));
        break;
      CASE(0,1)
        if (GROWING(iv)) {
          if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(ASININF_DOWN, iv.upper());
          else goto CASE_0_0;
        } else {
          if (RND.sin_down(iv.lower()) <= aSinIv.max()) iv.assign(iv.lower(), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
          else goto CASE_1_1;
        }
        break;
      CASE_LABEL(0,2)
        if (RND.sin_down(iv.lower()) <= aSinIv.max()) iv.assign(iv.lower(), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
        else goto CASE_1_2;
        break;
      CASE(0,3)
        if (RND.sin_down(iv.lower()) <= aSinIv.max())
          if (RND.sin_up(iv.upper()) >= aSinIv.min()) break; // Nothing changed
          else goto CASE_0_2;
        else goto CASE_1_3;
        break;
      CASE(1,0)
        if (GROWING(iv)) {
          if (RND.sin_up(iv.lower()) >= aSinIv.min()) iv.assign(iv.lower(), SHIFTN_UP(n,ASINSUP_UP));
          else iv.assign(RND.add_down(PITWICE_DOWN,ASININF_DOWN), SHIFTN_UP(n,ASINSUP_UP));
        } else {
          if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), iv.upper());
          else { n--; goto CASE_1_1; }
        }
        break;
      CASE_LABEL(1,1)
        if (NOT_GROWING(iv)) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
        else  if (RND.sin_up(iv.lower()) >= aSinIv.min())
                if (RND.sin_down(iv.upper()) <= aSinIv.max()) break; // Nothing changed
                else iv.assign(iv.lower(), SHIFTN_UP(n,ASINSUP_UP));
              else
                if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), iv.upper());
                else iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), SHIFTN_UP(n,ASINSUP_UP));
        break;
      CASE_LABEL(1,2) goto CASE_2_2;
      CASE_LABEL(1,3)
        if (RND.sin_up(iv.upper()) >= aSinIv.min()) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), iv.upper());
        else goto CASE_1_2;
        break;
      CASE_LABEL(2,0)
        if (RND.sin_up(iv.lower()) >= aSinIv.min()) iv.assign(iv.lower(), SHIFTN_UP(n,ASINSUP_UP));
        else iv.assign(RND.add_down(PITWICE_DOWN,ASININF_DOWN), SHIFTN_UP(n,ASINSUP_UP));
        break;
      CASE(2,1)
        if (RND.sin_up(iv.lower()) >= aSinIv.min())
          if (RND.sin_down(iv.upper()) <= aSinIv.max()) break; // Nothing changed
          else goto CASE_2_0;
        else goto CASE_3_0;
        break;
      CASE_LABEL(2,2)
        if (NOT_GROWING(iv)) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
        else  if (RND.sin_up(iv.lower()) >= aSinIv.min())
                if (RND.sin_down(iv.upper()) <= aSinIv.max()) break; // Nothing changed
                else iv.assign(iv.lower(), SHIFTN_UP(n-1,RND.add_up(PITWICE_UP, ASINSUP_UP)));
              else
                if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), iv.upper());
                else iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), SHIFTN_UP(n-1,RND.add_up(PITWICE_UP, ASINSUP_UP)));
        break;
      CASE(2,3)
        if (GROWING(iv)) {
          if (RND.sin_up(iv.lower()) >= aSinIv.min()) iv.assign(iv.lower(), SHIFTN_UP(n,RND.add_up(PITWICE_UP, ASINSUP_UP)));
          else goto CASE_3_3;
        } else {
          if (RND.sin_up(iv.upper()) >= aSinIv.min()) iv.assign(RND.sub_down(PI_DOWN, ASINSUP_UP), iv.upper());
          else goto CASE_2_2;
        }
        break;
      CASE_LABEL(3,0) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), SHIFTN_UP(n,ASINSUP_UP)); break;
      CASE(3,1)
        if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), iv.upper());
        else goto CASE_3_0;
        break;
      CASE(3,2)
        if (GROWING(iv)) {
          if (RND.sin_down(iv.upper()) <= aSinIv.max()) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN),iv.upper());
          else { n--; goto CASE_3_3; }
        } else {
          if (RND.sin_down(iv.lower()) <= aSinIv.max()) iv.assign(iv.lower(), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
          else iv.assign(RND.add_down(PITWICE_DOWN, RND.sub_down(PI_DOWN, ASINSUP_UP)), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
        }
        break;
      CASE_LABEL(3,3)
        if (GROWING(iv)) iv.assign(RND.add_down(PITWICE_DOWN, ASININF_DOWN), SHIFTN_UP(n,RND.add_up(PITWICE_UP, ASINSUP_UP)));
        else  if (RND.sin_up(iv.lower()) <= aSinIv.max())
                if (RND.sin_down(iv.upper()) >= aSinIv.min()) break; // Nothing changed
                else iv.assign(iv.lower(), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
              else
                if (RND.sin_down(iv.upper()) >= aSinIv.min()) iv.assign(RND.sub_down(RND.add_down(PITWICE_DOWN,PI_DOWN), ASINSUP_UP), iv.upper());
                else iv.assign(RND.sub_down(RND.add_down(PITWICE_DOWN,PI_DOWN), ASINSUP_UP), SHIFTN_UP(n,RND.sub_up(PI_UP, ASININF_DOWN)));
        break;
      default:
        GECODE_NEVER;
        break;
    }
#undef PI_UP
#undef PI_DOWN
#undef PITWICE_UP
#undef PITWICE_DOWN
#undef ASININF_DOWN
#undef ASINSUP_UP
#undef GROWING
#undef SHIFTN_UP
#undef SHIFTN_DOWN
#undef CASE_LABEL
#undef CASE
#undef POS
#undef I0__PI_2I
#undef IPI_2__PII
#undef IPI__3PI_2I
#undef I3PI_2__2PII
  }

  /*
   * Bounds consistent sinus operator
   *
   */

  template<class A, class B>
  forceinline
  Sin<A,B>::Sin(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Sin<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Sin<A,B>(home,x0,x1);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  Sin<A,B>::Sin(Space& home, bool share, Sin<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  Sin<A,B>::copy(Space& home, bool share) {
    return new (home) Sin<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Sin<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,sin(x0.domain())));
    FloatInterval iv = fmod(x0.domain(),pi_twice<FloatInterval>());
    FloatNum offSet(RND.sub_down(x0.min(),iv.lower()));
    aSinProject(x1,iv);
    GECODE_ME_CHECK(x0.eq(home,iv + offSet));
    return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }


  /*
   * Bounds consistent cosinus operator
   *
   */

  template<class A, class B>
  forceinline
  Cos<A,B>::Cos(Home home, A x0, B x1)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,x0,x1) {}

  template<class A, class B>
  ExecStatus
  Cos<A,B>::post(Home home, A x0, B x1) {
    (void) new (home) Cos<A,B>(home,x0,x1);
    return ES_OK;
  }


  template<class A, class B>
  forceinline
  Cos<A,B>::Cos(Space& home, bool share, Cos<A,B>& p)
    : MixBinaryPropagator<A,PC_FLOAT_BND,B,PC_FLOAT_BND>(home,share,p) {}

  template<class A, class B>
  Actor*
  Cos<A,B>::copy(Space& home, bool share) {
    return new (home) Cos<A,B>(home,share,*this);
  }

  template<class A, class B>
  ExecStatus
  Cos<A,B>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ME_CHECK(x1.eq(home,cos(x0.domain())));
    FloatInterval iv = fmod(pi_half<FloatInterval>() + x0.domain(),pi_twice<FloatInterval>());
    FloatNum offSet(RND.sub_down(x0.min(),iv.lower()));
    aSinProject(x1,iv);
    GECODE_ME_CHECK(x0.eq(home,iv + offSet));
    return (x0.assigned() && x1.assigned()) ? home.ES_SUBSUMED(*this) : ES_FIX;
  }

}}}

// STATISTICS: float-prop

