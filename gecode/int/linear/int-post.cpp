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

#include <cfloat>
#include <algorithm>

#include <gecode/int/rel.hh>
#include <gecode/int/linear.hh>

namespace Gecode { namespace Int { namespace Linear {

  /// Eliminate assigned views
  inline void
  eliminate(Term<IntView>* t, int &n, double& d) {
    for (int i=n; i--; )
      if (t[i].x.assigned()) {
        d -= t[i].a * static_cast<double>(t[i].x.val());
        t[i]=t[--n];
      }
    if ((d < Limits::double_min) || (d > Limits::double_max))
      throw OutOfLimits("Int::linear");
  }

  /// Rewrite all inequations in terms of IRT_LQ
  inline void
  rewrite(IntRelType &r, double &d,
          Term<IntView>* &t_p, int &n_p,
          Term<IntView>* &t_n, int &n_n) {
    switch (r) {
    case IRT_EQ: case IRT_NQ: case IRT_LQ:
      break;
    case IRT_LE:
      d--; r = IRT_LQ; break;
    case IRT_GR:
      d++; /* fall through */
    case IRT_GQ:
      r = IRT_LQ;
      std::swap(n_p,n_n); std::swap(t_p,t_n); d = -d;
      break;
    default:
      throw UnknownRelation("Int::linear");
    }
  }

  /// Decide the required precision and check for overflow
  inline bool
  precision(Term<IntView>* t_p, int n_p,
            Term<IntView>* t_n, int n_n,
            double d) {
    double sl = 0.0;
    double su = 0.0;

    for (int i = n_p; i--; ) {
      sl += t_p[i].a * static_cast<double>(t_p[i].x.min());
      su += t_p[i].a * static_cast<double>(t_p[i].x.max());
      if ((sl < Limits::double_min) || (su > Limits::double_max))
        throw OutOfLimits("Int::linear");
    }
    for (int i = n_n; i--; ) {
      sl -= t_n[i].a * static_cast<double>(t_n[i].x.max());
      su -= t_n[i].a * static_cast<double>(t_n[i].x.min());
      if ((sl < Limits::double_min) || (su > Limits::double_max))
        throw OutOfLimits("Int::linear");
    }

    bool is_ip = (sl >= Limits::min) && (su <= Limits::max);

    sl -= d;
    su -= d;
    if ((sl < Limits::double_min) || (su > Limits::double_max))
      throw OutOfLimits("Int::linear");

    is_ip = is_ip && (sl >= Limits::min) && (su <= Limits::max);

    for (int i = n_p; i--; ) {
      if (sl - t_p[i].a * static_cast<double>(t_p[i].x.min()) 
          < Limits::double_min)
        throw OutOfLimits("Int::linear");
      if (sl - t_p[i].a * static_cast<double>(t_p[i].x.min()) < Limits::min)
        is_ip = false;
      if (su - t_p[i].a * static_cast<double>(t_p[i].x.max()) 
          > Limits::double_max)
        throw OutOfLimits("Int::linear");
      if (su - t_p[i].a * static_cast<double>(t_p[i].x.max()) > Limits::max)
        is_ip = false;
    }
    for (int i = n_n; i--; ) {
      if (sl + t_n[i].a * static_cast<double>(t_n[i].x.min()) 
          < Limits::double_min)
        throw OutOfLimits("Int::linear");
      if (sl + t_n[i].a * static_cast<double>(t_n[i].x.min()) < Limits::min)
        is_ip = false;
      if (su + t_n[i].a * static_cast<double>(t_n[i].x.max()) 
          > Limits::double_max)
        throw OutOfLimits("Int::linear");
      if (su + t_n[i].a * static_cast<double>(t_n[i].x.max()) > Limits::max)
        is_ip = false;
    }
    return is_ip;
  }

  /**
   * \brief Posting n-ary propagators
   *
   */
  template<class Val, class View>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y, IntRelType r, Val c) {
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL((Eq<Val,View,View >::post(home,x,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL((Nq<Val,View,View >::post(home,x,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL((Lq<Val,View,View >::post(home,x,y,c)));
      break;
    default: GECODE_NEVER;
    }
  }


/// Macro for posting binary special cases for linear constraints
#define GECODE_INT_PL_BIN(CLASS)                                             \
  switch (n_p) {                                                             \
  case 2:                                                                    \
    GECODE_ES_FAIL((CLASS<int,IntView,IntView>::post                    \
                         (home,t_p[0].x,t_p[1].x,c)));                       \
    break;                                                                   \
  case 1:                                                                    \
    GECODE_ES_FAIL((CLASS<int,IntView,MinusView>::post                  \
                         (home,t_p[0].x,MinusView(t_n[0].x),c)));            \
    break;                                                                   \
  case 0:                                                                    \
    GECODE_ES_FAIL((CLASS<int,MinusView,MinusView>::post                \
                         (home,MinusView(t_n[0].x),MinusView(t_n[1].x),c))); \
    break;                                                                   \
  default: GECODE_NEVER;                                                     \
  }

/// Macro for posting ternary special cases for linear constraints
#define GECODE_INT_PL_TER(CLASS)                                        \
  switch (n_p) {                                                        \
  case 3:                                                               \
    GECODE_ES_FAIL((CLASS<int,IntView,IntView,IntView>::post       \
                         (home,t_p[0].x,t_p[1].x,t_p[2].x,c)));         \
    break;                                                              \
  case 2:                                                               \
    GECODE_ES_FAIL((CLASS<int,IntView,IntView,MinusView>::post     \
                         (home,t_p[0].x,t_p[1].x,                       \
                          MinusView(t_n[0].x),c)));                     \
    break;                                                              \
  case 1:                                                               \
    GECODE_ES_FAIL((CLASS<int,IntView,MinusView,MinusView>::post   \
                         (home,t_p[0].x,                                \
                          MinusView(t_n[0].x),MinusView(t_n[1].x),c))); \
    break;                                                              \
  case 0:                                                               \
    GECODE_ES_FAIL((CLASS<int,MinusView,MinusView,MinusView>::post \
                         (home,MinusView(t_n[0].x),                     \
                          MinusView(t_n[1].x),MinusView(t_n[2].x),c))); \
    break;                                                              \
  default: GECODE_NEVER;                                                \
  }

  void
  post(Home home, Term<IntView>* t, int n, IntRelType r, int c,
       IntConLevel icl) {

    Limits::check(c,"Int::linear");

    double d = c;

    eliminate(t,n,d);

    Term<IntView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize<IntView>(t,n,t_p,n_p,t_n,n_n);

    rewrite(r,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (r) {
      case IRT_EQ: if (d != 0.0) home.fail(); break;
      case IRT_NQ: if (d == 0.0) home.fail(); break;
      case IRT_LQ: if (d < 0.0)  home.fail(); break;
      default: GECODE_NEVER;
      }
      return;
    }

    if (n == 1) {
      if (n_p == 1) {
        DoubleScaleView y(t_p[0].a,t_p[0].x);
        switch (r) {
        case IRT_EQ: GECODE_ME_FAIL(y.eq(home,d)); break;
        case IRT_NQ: GECODE_ME_FAIL(y.nq(home,d)); break;
        case IRT_LQ: GECODE_ME_FAIL(y.lq(home,d)); break;
        default: GECODE_NEVER;
        }
      } else {
        DoubleScaleView y(t_n[0].a,t_n[0].x);
        switch (r) {
        case IRT_EQ: GECODE_ME_FAIL(y.eq(home,-d)); break;
        case IRT_NQ: GECODE_ME_FAIL(y.nq(home,-d)); break;
        case IRT_LQ: GECODE_ME_FAIL(y.gq(home,-d)); break;
        default: GECODE_NEVER;
        }
      }
      return;
    }

    bool is_ip = precision(t_p,n_p,t_n,n_n,d);

    if (is_unit && is_ip && (icl != ICL_DOM)) {
      // Unit coefficients with integer precision
      c = static_cast<int>(d);
      if (n == 2) {
        switch (r) {
        case IRT_EQ: GECODE_INT_PL_BIN(EqBin); break;
        case IRT_NQ: GECODE_INT_PL_BIN(NqBin); break;
        case IRT_LQ: GECODE_INT_PL_BIN(LqBin); break;
        default: GECODE_NEVER;
        }
      } else if (n == 3) {
        switch (r) {
        case IRT_EQ: GECODE_INT_PL_TER(EqTer); break;
        case IRT_NQ: GECODE_INT_PL_TER(NqTer); break;
        case IRT_LQ: GECODE_INT_PL_TER(LqTer); break;
        default: GECODE_NEVER;
        }
      } else {
        ViewArray<IntView> x(home,n_p);
        for (int i = n_p; i--; )
          x[i] = t_p[i].x;
        ViewArray<IntView> y(home,n_n);
        for (int i = n_n; i--; )
          y[i] = t_n[i].x;
        post_nary<int,IntView>(home,x,y,r,c);
      }
    } else if (is_ip) {
      if (n==2 && is_unit && icl == ICL_DOM && r == IRT_EQ) {
        // Binary domain-consistent equality
        c = static_cast<int>(d);
        if (c==0) {
          switch (n_p) {
          case 2: {
            IntView x(t_p[0].x);
            MinusView y(t_p[1].x);
            GECODE_ES_FAIL(
              (Rel::EqDom<IntView,MinusView>::post(home,x,y)));
            break;
          }
          case 1: {
            IntView x(t_p[0].x);
            IntView y(t_n[0].x);
            GECODE_ES_FAIL(
              (Rel::EqDom<IntView,IntView>::post(home,x,y)));
            break;
          }
          case 0: {
            IntView x(t_n[0].x);
            MinusView y(t_n[1].x);
            GECODE_ES_FAIL(
              (Rel::EqDom<IntView,MinusView>::post(home,x,y)));
            break;
          }
          default:
            GECODE_NEVER;
          }
        } else {
          switch (n_p) {
          case 2: {
            MinusView x(t_p[0].x);
            OffsetView y(t_p[1].x, -c);
            GECODE_ES_FAIL(
              (Rel::EqDom<MinusView,OffsetView>::post(home,x,y)));
            break;
          }
          case 1: {
            IntView x(t_p[0].x);
            OffsetView y(t_n[0].x, c);
            GECODE_ES_FAIL(
              (Rel::EqDom<IntView,OffsetView>::post(home,x,y)));
            break;
          }
          case 0: {
            MinusView x(t_n[0].x);
            OffsetView y(t_n[1].x, c);
            GECODE_ES_FAIL(
              (Rel::EqDom<MinusView,OffsetView>::post(home,x,y)));
            break;
          }
          default:
            GECODE_NEVER;
          }          
        }
      } else {
        // Arbitrary coefficients with integer precision
        c = static_cast<int>(d);
        ViewArray<IntScaleView> x(home,n_p);
        for (int i = n_p; i--; )
          x[i] = IntScaleView(t_p[i].a,t_p[i].x);
        ViewArray<IntScaleView> y(home,n_n);
        for (int i = n_n; i--; )
          y[i] = IntScaleView(t_n[i].a,t_n[i].x);
        if ((icl == ICL_DOM) && (r == IRT_EQ)) {
          GECODE_ES_FAIL((DomEq<int,IntScaleView>::post(home,x,y,c)));
        } else {
          post_nary<int,IntScaleView>(home,x,y,r,c);
        }
      }
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<DoubleScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = DoubleScaleView(t_p[i].a,t_p[i].x);
      ViewArray<DoubleScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = DoubleScaleView(t_n[i].a,t_n[i].x);
      if ((icl == ICL_DOM) && (r == IRT_EQ)) {
        GECODE_ES_FAIL((DomEq<double,DoubleScaleView>::post(home,x,y,d)));
      } else {
        post_nary<double,DoubleScaleView>(home,x,y,r,d);
      }
    }
  }

#undef GECODE_INT_PL_BIN
#undef GECODE_INT_PL_TER


  /**
   * \brief Posting reified n-ary propagators
   *
   */
  template<class Val, class View>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y,
            IntRelType r, Val c, BoolView b) {
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL((ReEq<Val,View,View,BoolView>::post(home,x,y,c,b)));
      break;
    case IRT_NQ:
      {
        NegBoolView n(b);
        GECODE_ES_FAIL((ReEq<Val,View,View,NegBoolView>::post
                             (home,x,y,c,n)));
      }
      break;
    case IRT_LQ:
      GECODE_ES_FAIL((ReLq<Val,View,View>::post(home,x,y,c,b)));
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post(Home home,
       Term<IntView>* t, int n, IntRelType r, int c, BoolView b,
       IntConLevel) {

    Limits::check(c,"Int::linear");

    double d = c;

    eliminate(t,n,d);

    Term<IntView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize<IntView>(t,n,t_p,n_p,t_n,n_n);

    rewrite(r,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      bool fail = false;
      switch (r) {
      case IRT_EQ: fail = (d != 0.0); break;
      case IRT_NQ: fail = (d == 0.0); break;
      case IRT_LQ: fail = (0.0 > d); break;
      default: GECODE_NEVER;
      }
      if ((fail ? b.zero(home) : b.one(home)) == ME_INT_FAILED)
        home.fail();
      return;
    }

    bool is_ip = precision(t_p,n_p,t_n,n_n,d);

    if (is_unit && is_ip) {
      c = static_cast<int>(d);
      if (n == 1) {
        switch (r) {
        case IRT_EQ:
          if (n_p == 1) {
            GECODE_ES_FAIL((Rel::ReEqBndInt<IntView,BoolView>::post
                                 (home,t_p[0].x,c,b)));
          } else {
            GECODE_ES_FAIL((Rel::ReEqBndInt<IntView,BoolView>::post
                                 (home,t_n[0].x,-c,b)));
          }
          break;
        case IRT_NQ:
          {
            NegBoolView nb(b);
            if (n_p == 1) {
              GECODE_ES_FAIL((Rel::ReEqBndInt<IntView,NegBoolView>::post
                                   (home,t_p[0].x,c,nb)));
            } else {
              GECODE_ES_FAIL((Rel::ReEqBndInt<IntView,NegBoolView>::post
                                   (home,t_n[0].x,-c,nb)));
            }
          }
          break;
        case IRT_LQ:
          if (n_p == 1) {
            GECODE_ES_FAIL((Rel::ReLqInt<IntView,BoolView>::post
                                 (home,t_p[0].x,c,b)));
          } else {
            NegBoolView nb(b);
            GECODE_ES_FAIL((Rel::ReLqInt<IntView,NegBoolView>::post
                                 (home,t_n[0].x,-c-1,nb)));
          }
          break;
        default: GECODE_NEVER;
        }
      } else if (n == 2) {
        switch (r) {
        case IRT_EQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL((ReEqBin<int,IntView,IntView,BoolView>::post
                                 (home,t_p[0].x,t_p[1].x,c,b)));
            break;
          case 1:
            GECODE_ES_FAIL((ReEqBin<int,IntView,MinusView,BoolView>::post
                                 (home,t_p[0].x,MinusView(t_n[0].x),c,b)));
            break;
          case 0:
            GECODE_ES_FAIL((ReEqBin<int,IntView,IntView,BoolView>::post
                                 (home,t_n[0].x,t_n[1].x,-c,b)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_NQ:
          {
            NegBoolView nb(b);
            switch (n_p) {
            case 2:
              GECODE_ES_FAIL(
                             (ReEqBin<int,IntView,IntView,NegBoolView>::post
                              (home,t_p[0].x,t_p[1].x,c,nb)));
              break;
            case 1:
              GECODE_ES_FAIL(
                             (ReEqBin<int,IntView,MinusView,NegBoolView>::post
                              (home,t_p[0].x,MinusView(t_n[0].x),c,
                               NegBoolView(b))));
              break;
            case 0:
              GECODE_ES_FAIL(
                             (ReEqBin<int,IntView,IntView,NegBoolView>::post
                              (home,t_p[0].x,t_p[1].x,-c,NegBoolView(b))));
              break;
            default: GECODE_NEVER;
            }
          }
          break;
        case IRT_LQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL((ReLqBin<int,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,c,b)));
            break;
          case 1:
            GECODE_ES_FAIL((ReLqBin<int,IntView,MinusView>::post
                                 (home,t_p[0].x,MinusView(t_n[0].x),c,b)));
            break;
          case 0:
            GECODE_ES_FAIL((ReLqBin<int,MinusView,MinusView>::post
                                 (home,MinusView(t_n[0].x),
                                  MinusView(t_n[1].x),c,b)));
            break;
          default: GECODE_NEVER;
          }
          break;
        default: GECODE_NEVER;
        }
      } else {
        ViewArray<IntView> x(home,n_p);
        for (int i = n_p; i--; )
          x[i] = t_p[i].x;
        ViewArray<IntView> y(home,n_n);
        for (int i = n_n; i--; )
          y[i] = t_n[i].x;
        post_nary<int,IntView>(home,x,y,r,c,b);
      }
    } else if (is_ip) {
      // Arbitrary coefficients with integer precision
      c = static_cast<int>(d);
      ViewArray<IntScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = IntScaleView(t_p[i].a,t_p[i].x);
      ViewArray<IntScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = IntScaleView(t_n[i].a,t_n[i].x);
      post_nary<int,IntScaleView>(home,x,y,r,c,b);
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<DoubleScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = DoubleScaleView(t_p[i].a,t_p[i].x);
      ViewArray<DoubleScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = DoubleScaleView(t_n[i].a,t_n[i].x);
      post_nary<double,DoubleScaleView>(home,x,y,r,d,b);
    }
  }

}}}

// STATISTICS: int-post
