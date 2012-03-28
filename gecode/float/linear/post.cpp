/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include <algorithm>
#include <climits>

#include <gecode/float/linear.hh>
#include <gecode/float/rel.hh>

namespace Gecode { namespace Float { namespace Linear {

  /// Eliminate assigned views
  inline void
  eliminate(Term<FloatView>* t, int &n, FloatVal& d) {
    for (int i=n; i--; )
      if (t[i].x.assigned()) {
        d -= t[i].a * t[i].x.val();
        t[i]=t[--n];
      }
    if ((d < Limits::min) || (d > Limits::max))
      throw OutOfLimits("Float::linear");
  }

  /// Rewrite all inequations in terms of FRT_LQ
  inline void
  rewrite(FloatRelType &frt, FloatVal &d,
          Term<FloatView>* &t_p, int &n_p,
          Term<FloatView>* &t_n, int &n_n) {
    switch (frt) {
    case FRT_EQ: case FRT_LQ:
      break;
    case FRT_GQ:
      frt = FRT_LQ;
      std::swap(n_p,n_n); std::swap(t_p,t_n); d = -d;
      break;
    default:
      throw UnknownRelation("Float::linear");
    }
  }


  /**
   * \brief Posting n-ary propagators
   *
   */
  template<class View>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y, FloatRelType frt, FloatVal c) {
    switch (frt) {
    case FRT_EQ:
      GECODE_ES_FAIL((Eq<View,View >::post(home,x,y,c)));
      break;
    case FRT_LQ:
      GECODE_ES_FAIL((Lq<View,View >::post(home,x,y,c)));
      break;
    default: GECODE_NEVER;
    }
  }


/// Macro for posting binary special cases for linear constraints
#define GECODE_FLOAT_PL_BIN(CLASS)                                             \
  switch (n_p) {                                                             \
  case 2:                                                                    \
    GECODE_ES_FAIL((CLASS<FloatView,FloatView>::post                         \
                         (home,t_p[0].x,t_p[1].x,c)));                       \
    break;                                                                   \
  case 1:                                                                    \
    GECODE_ES_FAIL((CLASS<FloatView,MinusView>::post                       \
                         (home,t_p[0].x,MinusView(t_n[0].x),c)));            \
    break;                                                                   \
  case 0:                                                                    \
    GECODE_ES_FAIL((CLASS<MinusView,MinusView>::post                     \
                         (home,MinusView(t_n[0].x),MinusView(t_n[1].x),c))); \
    break;                                                                   \
  default: GECODE_NEVER;                                                     \
  }

/// Macro for posting ternary special cases for linear constraints
#define GECODE_FLOAT_PL_TER(CLASS)                                        \
  switch (n_p) {                                                        \
  case 3:                                                               \
    GECODE_ES_FAIL((CLASS<FloatView,FloatView,FloatView>::post            \
                         (home,t_p[0].x,t_p[1].x,t_p[2].x,c)));         \
    break;                                                              \
  case 2:                                                               \
    GECODE_ES_FAIL((CLASS<FloatView,FloatView,MinusView>::post          \
                         (home,t_p[0].x,t_p[1].x,                       \
                          MinusView(t_n[0].x),c)));                     \
    break;                                                              \
  case 1:                                                               \
    GECODE_ES_FAIL((CLASS<FloatView,MinusView,MinusView>::post        \
                         (home,t_p[0].x,                                \
                          MinusView(t_n[0].x),MinusView(t_n[1].x),c))); \
    break;                                                              \
  case 0:                                                               \
    GECODE_ES_FAIL((CLASS<MinusView,MinusView,MinusView>::post      \
                         (home,MinusView(t_n[0].x),                     \
                          MinusView(t_n[1].x),MinusView(t_n[2].x),c))); \
    break;                                                              \
  default: GECODE_NEVER;                                                \
  }

  void
  post(Home home, Term<FloatView>* t, int n, FloatRelType frt, FloatVal c) {

    Limits::check(c,"Float::linear");

    FloatVal d = c;

    eliminate(t,n,d);

    Term<FloatView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize<FloatView>(t,n,t_p,n_p,t_n,n_n);

    rewrite(frt,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (frt) {
      case FRT_EQ: if (d != 0.0) home.fail(); break;
      case FRT_LQ: if (d < 0.0)  home.fail(); break;
      default: GECODE_NEVER;
      }
      return;
    }

    if (n == 1) {
      if (n_p == 1) {
        ScaleView y(t_p[0].a,t_p[0].x);
        switch (frt) {
        case FRT_EQ: GECODE_ME_FAIL(y.eq(home,d)); break;
        case FRT_LQ: GECODE_ME_FAIL(y.lq(home,d)); break;
        default: GECODE_NEVER;
        }
      } else {
        ScaleView y(t_n[0].a,t_n[0].x);
        switch (frt) {
        case FRT_EQ: GECODE_ME_FAIL(y.eq(home,-d)); break;
        case FRT_LQ: GECODE_ME_FAIL(y.gq(home,-d)); break;
        default: GECODE_NEVER;
        }
      }
      return;
    }

    if (is_unit) {
      // Unit coefficients
      if (n == 2) {
        switch (frt) {
        case FRT_EQ: GECODE_FLOAT_PL_BIN(EqBin); break;
        case FRT_LQ: GECODE_FLOAT_PL_BIN(LqBin); break;
        default: GECODE_NEVER;
        }
      } else if (n == 3) {
        switch (frt) {
        case FRT_EQ: GECODE_FLOAT_PL_TER(EqTer); break;
        case FRT_LQ: GECODE_FLOAT_PL_TER(LqTer); break;
        default: GECODE_NEVER;
        }
      } else {
        ViewArray<FloatView> x(home,n_p);
        for (int i = n_p; i--; )
          x[i] = t_p[i].x;
        ViewArray<FloatView> y(home,n_n);
        for (int i = n_n; i--; )
          y[i] = t_n[i].x;
        post_nary<FloatView>(home,x,y,frt,d);
      }
    } else {
      // Arbitrary coefficients 
      ViewArray<ScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = ScaleView(t_p[i].a,t_p[i].x);
      ViewArray<ScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = ScaleView(t_n[i].a,t_n[i].x);
      post_nary<ScaleView>(home,x,y,frt,d);
    }
  }

#undef GECODE_FLOAT_PL_BIN
#undef GECODE_FLOAT_PL_TER

  /**
   * \brief Posting reified n-ary propagators
   *
   */
  template<class View, class ReifyView>
  forceinline void
  post_nary(Home home,
            ViewArray<View>& x, ViewArray<View>& y,
            FloatRelType frt, FloatVal c, Reify r) {
    switch (frt) {
    case FRT_EQ:
      switch (r.mode()) {
      case RM_EQV:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_EQV>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_IMP>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((ReEq<View,View,ReifyView,RM_PMI>::
                        post(home,x,y,c,ReifyView(r.var()))));
        break;
      default: GECODE_NEVER;
      }
      break;
    case FRT_LQ:
        switch (r.mode()) {
        case RM_EQV:
          GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_EQV>::
                          post(home,x,y,c,ReifyView(r.var()))));
          break;
        case RM_IMP:
          GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_IMP>::
                          post(home,x,y,c,ReifyView(r.var()))));
          break;
        case RM_PMI:
          GECODE_ES_FAIL((ReLq<View,View,ReifyView,RM_PMI>::
                          post(home,x,y,c,ReifyView(r.var()))));
          break;
        default: GECODE_NEVER;
        }
      break;
    default: GECODE_NEVER;
    }
  }

#define WRITE_REIFIED_UNA_PROP(name,view0,mode,a,c,bvar) \
        if (b) GECODE_ES_FAIL((name<view0,Int::BoolView,mode>:: \
                               post(home,a,c,bvar))); \
        else   GECODE_ES_FAIL((name<view0,Int::NegBoolView,mode>:: \
                               post(home,a,c,Int::NegBoolView(bvar))));

#define WRITE_REIFIED_BIN_PROP(name,view0,view1,mode,a0,a1,c,bvar) \
        if (b) GECODE_ES_FAIL((name<view0,view1,Int::BoolView,mode>:: \
                               post(home,a0,a1,c,bvar))); \
        else   GECODE_ES_FAIL((name<view0,view1,Int::NegBoolView,mode>:: \
                               post(home,a0,a1,c,Int::NegBoolView(bvar))));

  void
  post(Home home,
       Term<FloatView>* t, int n, FloatRelType frt, FloatVal c, Reify r, bool b) {
    Limits::check(c,"Float::linear");

    FloatVal d = c;

    eliminate(t,n,d);

    Term<FloatView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = normalize<FloatView>(t,n,t_p,n_p,t_n,n_n);

    rewrite(frt,d,t_p,n_p,t_n,n_n);

    if (n == 0) {
      // FIXME!
      bool fail = false;
      switch (frt) {
      case FRT_EQ: fail = (d != 0.0); break;
      case FRT_LQ: fail = (d < 0.0); break;
      default: GECODE_NEVER;
      }
      if (((fail && b)? 
           Int::BoolView(r.var()).zero(home) : 
           Int::BoolView(r.var()).one(home)) == Int::ME_INT_FAILED)
        home.fail();
      return;
    }

    if (is_unit) {
      c = d;
      if (n == 1) {
        switch (frt) {
        case FRT_EQ:
          if (n_p == 1) {
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_EQV,t_p[0].x,c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_IMP,t_p[0].x,c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_PMI,t_p[0].x,c,r.var());
              break;
            default: GECODE_NEVER;
            }
          } else {
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_EQV,t_n[0].x,-c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_IMP,t_n[0].x,-c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_UNA_PROP(Rel::ReEqFloat,FloatView,RM_PMI,t_n[0].x,-c,r.var());
              break;
            default: GECODE_NEVER;
            }
          }
          break;
        case FRT_LQ:
          if (n_p == 1) {
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_EQV,t_p[0].x,c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_IMP,t_p[0].x,c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,FloatView,RM_PMI,t_p[0].x,c,r.var());
              break;
            default: GECODE_NEVER;
            }
          } else {
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,MinusView,RM_EQV,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,MinusView,RM_IMP,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_UNA_PROP(Rel::ReLqFloat,MinusView,RM_PMI,MinusView(t_n[0].x),c,r.var());
              break;
            default: GECODE_NEVER;
            }
          }
          break;
        default: GECODE_NEVER;
        }
      } else if (n == 2) {
        switch (frt) {
        case FRT_EQ:
          switch (n_p) {
          case 2:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_EQV,t_p[0].x,t_p[1].x,c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_IMP,t_p[0].x,t_p[1].x,c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_PMI,t_p[0].x,t_p[1].x,c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          case 1:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,MinusView,RM_EQV,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,MinusView,RM_IMP,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,MinusView,RM_PMI,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          case 0:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_EQV,t_n[0].x,t_n[1].x,-c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_IMP,t_n[0].x,t_n[1].x,-c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReEqBin,FloatView,FloatView,RM_PMI,t_n[0].x,t_n[1].x,-c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          default: GECODE_NEVER;
          }
          break;
        case FRT_LQ:
          switch (n_p) {
          case 2:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,FloatView,RM_EQV,t_p[0].x,t_p[1].x,c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,FloatView,RM_IMP,t_p[0].x,t_p[1].x,c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,FloatView,RM_PMI,t_p[0].x,t_p[1].x,c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          case 1:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,MinusView,RM_EQV,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,MinusView,RM_IMP,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReLqBin,FloatView,MinusView,RM_PMI,t_p[0].x,MinusView(t_n[0].x),c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          case 0:
            switch (r.mode()) {
            case RM_EQV:
              WRITE_REIFIED_BIN_PROP(ReLqBin,MinusView,MinusView,RM_EQV,MinusView(t_n[0].x),MinusView(t_n[1].x),c,r.var());
              break;
            case RM_IMP:
              WRITE_REIFIED_BIN_PROP(ReLqBin,MinusView,MinusView,RM_IMP,MinusView(t_n[0].x),MinusView(t_n[1].x),c,r.var());
              break;
            case RM_PMI:
              WRITE_REIFIED_BIN_PROP(ReLqBin,MinusView,MinusView,RM_PMI,MinusView(t_n[0].x),MinusView(t_n[1].x),c,r.var());
              break;
            default: GECODE_NEVER;
            }
            break;
          default: GECODE_NEVER;
          }
          break;
        default: GECODE_NEVER;
        }
      } else {
        ViewArray<FloatView> x(home,n_p);
        for (int i = n_p; i--; )
          x[i] = t_p[i].x;
        ViewArray<FloatView> y(home,n_n);
        for (int i = n_n; i--; )
          y[i] = t_n[i].x;
        if (b) post_nary<FloatView,Int::BoolView>(home,x,y,frt,c,r);
        else   post_nary<FloatView,Int::NegBoolView>(home,x,y,frt,c,r);
      }
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<ScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i] = ScaleView(t_p[i].a,t_p[i].x);
      ViewArray<ScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i] = ScaleView(t_n[i].a,t_n[i].x);
      if (b) post_nary<ScaleView,Int::BoolView>(home,x,y,frt,d,r);
      else   post_nary<ScaleView,Int::NegBoolView>(home,x,y,frt,d,r);
    }
  }
#undef WRITE_REIFIED_UNA_PROP
#undef WRITE_REIFIED_BIN_PROP

}}}

// STATISTICS: float-post

