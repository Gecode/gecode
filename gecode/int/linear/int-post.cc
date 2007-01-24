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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/int/rel.hh"
#include "gecode/int/linear.hh"

#include "gecode/support/sort.hh"

#include <climits>
#include <algorithm>

namespace Gecode { namespace Int { namespace Linear {

  /// Decide the required precision and check for overflow
  inline bool
  precision(Term<IntView> t[], int n, int c) {
    double sn = 0.0; double sp = 0.0;

    for (int i = n; i--; ) {
      const double l = t[i].a * static_cast<double>(t[i].x.min());
      if (l < 0.0) sn += l; else sp += l;
      const double u = t[i].a * static_cast<double>(t[i].x.max());
      if (u < 0.0) sn += u; else sp += u;
    }
    double cp = (c<0) ? -c : c;
    if ((sn-cp < Limits::Int::double_min) ||
        (sp+cp > Limits::Int::double_max))
      throw NumericalOverflow("Int::linear");

    return ((sn >= Limits::Int::int_min) && (sn <= Limits::Int::int_max) &&
            (sp >= Limits::Int::int_min) && (sp <= Limits::Int::int_max) &&
            (sn-c >= Limits::Int::int_min) && (sn-c <= Limits::Int::int_max) &&
            (sp-c >= Limits::Int::int_min) && (sp-c <= Limits::Int::int_max));
  }

  /*
   * Posting plain propagators
   *
   */

  template <class Val, class View>
  forceinline void
  post_nary(Space* home,
            ViewArray<View>& x, ViewArray<View>& y, IntRelType r, Val c) {
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(Eq<Val,View,View >::post(home,x,y,c)));
      break;
    case IRT_NQ:
      GECODE_ES_FAIL(home,(Nq<Val,View,View >::post(home,x,y,c)));
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(Lq<Val,View,View >::post(home,x,y,c)));
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post(Space* home, Term<IntView> t[], int n, IntRelType r, int c,
       IntConLevel icl) {
    if ((c < Limits::Int::int_min) || (c > Limits::Int::int_max))
      throw NumericalOverflow("Int::linear");

    normalize<IntView>(t,n,r,c);

    Term<IntView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = separate<IntView>(t,n,t_p,n_p,t_n,n_n);

    if (n == 0) {
      switch (r) {
      case IRT_EQ: if (c != 0) home->fail(); break;
      case IRT_NQ: if (c == 0) home->fail(); break;
      case IRT_LQ: if (c < 0)  home->fail(); break;
      default: GECODE_NEVER;
      }
      return;
    }
    if (n == 1) {
      if (n_p == 1) {
        DoubleScaleView y(t_p[0].a,t_p[0].x);
        switch (r) {
        case IRT_EQ: GECODE_ME_FAIL(home,y.eq(home,c)); break;
        case IRT_NQ: GECODE_ME_FAIL(home,y.nq(home,c)); break;
        case IRT_LQ: GECODE_ME_FAIL(home,y.lq(home,c)); break;
        default: GECODE_NEVER;
        }
      } else {
        DoubleScaleView y(t_n[0].a,t_n[0].x);
        switch (r) {
        case IRT_EQ: GECODE_ME_FAIL(home,y.eq(home,-c)); break;
        case IRT_NQ: GECODE_ME_FAIL(home,y.nq(home,-c)); break;
        case IRT_LQ: GECODE_ME_FAIL(home,y.gq(home,-c)); break;
        default: GECODE_NEVER;
        }
      }
      return;
    }

    bool is_ip = precision(t,n,c);

    if (is_unit && is_ip && (icl != ICL_DOM)) {
      if (n == 2) {
        switch (r) {
        case IRT_EQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL(home,(EqBin<int,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(EqBin<int,IntView,MinusView>::post
                                 (home,t_p[0].x,t_n[0].x,c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(EqBin<int,IntView,IntView>::post
                                 (home,t_n[0].x,t_n[1].x,-c)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_NQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL(home,(NqBin<int,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(NqBin<int,IntView,MinusView>::post
                                 (home,t_p[0].x,t_n[0].x,c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(NqBin<int,IntView,IntView>::post
                                 (home,t_n[0].x,t_n[1].x,-c)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_LQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL(home,(LqBin<int,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(LqBin<int,IntView,MinusView>::post
                                 (home,t_p[0].x,t_n[0].x,c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(LqBin<int,MinusView,MinusView>::post
                                 (home,t_n[0].x,t_n[1].x,c)));
            break;
          default: GECODE_NEVER;
          }
          break;
        default: GECODE_NEVER;
        }
      } else if (n == 3) {
        switch (r) {                                                
        case IRT_EQ:
          switch (n_p) {
          case 3:
            GECODE_ES_FAIL(home,(EqTer<int,IntView,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,t_p[2].x,c)));
            break;
          case 2:
            GECODE_ES_FAIL(home,(EqTer<int,IntView,IntView,MinusView>::post
                                 (home,t_p[0].x,t_p[1].x,t_n[0].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(EqTer<int,IntView,IntView,MinusView>::post
                                 (home,t_n[0].x,t_n[1].x,t_p[0].x,-c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(EqTer<int,IntView,IntView,IntView>::post
                                 (home,t_n[0].x,t_n[1].x,t_n[2].x,-c)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_NQ:
          switch (n_p) {
          case 3:
            GECODE_ES_FAIL(home,(NqTer<int,IntView,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,t_p[2].x,c)));
            break;
          case 2:
            GECODE_ES_FAIL(home,(NqTer<int,IntView,IntView,MinusView>::post
                                 (home,t_p[0].x,t_p[1].x,t_n[0].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(NqTer<int,IntView,IntView,MinusView>::post
                                 (home,t_n[0].x,t_n[1].x,t_p[0].x,-c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(NqTer<int,IntView,IntView,IntView>::post
                                 (home,t_n[0].x,t_n[1].x,t_n[2].x,-c)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_LQ:
          switch (n_p) {
          case 3:
            GECODE_ES_FAIL(home,(LqTer<int,IntView,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,t_p[2].x,c)));
            break;
          case 2:
            GECODE_ES_FAIL(home,(LqTer<int,IntView,IntView,MinusView>::post
                                 (home,t_p[0].x,t_p[1].x,t_n[0].x,c)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(LqTer<int,IntView,MinusView,MinusView>::post
                                 (home,t_p[0].x,t_n[0].x,t_n[1].x,c)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(LqTer<int,MinusView,MinusView,MinusView>::post
                                 (home,t_n[0].x,t_n[1].x,t_n[2].x,c)));
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
        post_nary<int,IntView>(home,x,y,r,c);
      }
    } else if (is_ip) {
      // Arbitrary coefficients with integer precision
      ViewArray<IntScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i].init(t_p[i].a,t_p[i].x);
      ViewArray<IntScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i].init(t_n[i].a,t_n[i].x);
      if ((icl == ICL_DOM) && (r == IRT_EQ)) {
        GECODE_ES_FAIL(home,(DomEq<int,IntScaleView>::post(home,x,y,c)));
      } else {
        post_nary<int,IntScaleView>(home,x,y,r,c);
      }
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<DoubleScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i].init(t_p[i].a,t_p[i].x);
      ViewArray<DoubleScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i].init(t_n[i].a,t_n[i].x);
      if ((icl == ICL_DOM) && (r == IRT_EQ)) {
        GECODE_ES_FAIL(home,(DomEq<double,DoubleScaleView>::post(home,x,y,c)));
      } else {
        post_nary<double,DoubleScaleView>(home,x,y,r,c);
      }
    }
  }



  /*
   * Posting reified propagators
   *
   */

  template <class Val, class View>
  forceinline void
  post_nary(Space* home,
            ViewArray<View>& x, ViewArray<View>& y, 
            IntRelType r, Val c, BoolView b) {
    switch (r) {
    case IRT_EQ:
      GECODE_ES_FAIL(home,(ReEq<Val,View,View,BoolView>::post(home,x,y,c,b)));
      break;
    case IRT_NQ:
      {
        NegBoolView n(b);
        GECODE_ES_FAIL(home,(ReEq<Val,View,View,NegBoolView>::post
                             (home,x,y,c,n)));
      }
      break;
    case IRT_LQ:
      GECODE_ES_FAIL(home,(ReLq<Val,View,View>::post(home,x,y,c,b)));
      break;
    default: GECODE_NEVER;
    }
  }

  void
  post(Space* home,
       Term<IntView> t[], int n, IntRelType r, int c, BoolView b) {
    if ((c < Limits::Int::int_min) || (c > Limits::Int::int_max))
      throw NumericalOverflow("Int::linear");

    normalize<IntView>(t,n,r,c);

    Term<IntView> *t_p, *t_n;
    int n_p, n_n;
    bool is_unit = separate<IntView>(t,n,t_p,n_p,t_n,n_n);

    if (n == 0) {
      bool fail = false;
      switch (r) {
      case IRT_EQ: fail = (c != 0); break;
      case IRT_NQ: fail = (c == 0); break;
      case IRT_LQ: fail = (0 > c);  break;
      default: GECODE_NEVER;
      }
      if ((fail ? b.zero(home) : b.one(home)) == ME_INT_FAILED)
        home->fail();
      return;
    }

    bool is_ip = precision(t,n,c);

    if (is_unit && is_ip) {
      if (n == 1) {
        switch (r) {
        case IRT_EQ:
          if (n_p == 1) {
            GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,BoolView>::post
                                 (home,t_p[0].x,c,b)));
          } else {
            GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,BoolView>::post
                                 (home,t_n[0].x,-c,b)));
          }
          break;
        case IRT_NQ:
          {
            NegBoolView n(b);
            if (n_p == 1) {
              GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,NegBoolView>::post
                                   (home,t_p[0].x,c,n)));
            } else {
              GECODE_ES_FAIL(home,(Rel::ReEqBndInt<IntView,NegBoolView>::post
                                   (home,t_n[0].x,-c,n)));
            }
          }
          break;
        case IRT_LQ:
          if (n_p == 1) {
            GECODE_ES_FAIL(home,(Rel::ReLqInt<IntView,BoolView>::post
                                 (home,t_p[0].x,c,b)));
          } else {
            NegBoolView n(b);
            GECODE_ES_FAIL(home,(Rel::ReLqInt<IntView,NegBoolView>::post
                                 (home,t_n[0].x,-c-1,n)));
          }
          break;
        default: GECODE_NEVER;
        }
      } else if (n == 2) {
        switch (r) {
        case IRT_EQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL(home,(ReEqBin<int,IntView,IntView,BoolView>::post
                                 (home,t_p[0].x,t_p[1].x,c,b)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(ReEqBin<int,IntView,MinusView,BoolView>::post
                                 (home,t_p[0].x,t_n[0].x,c,b)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(ReEqBin<int,IntView,IntView,BoolView>::post
                                 (home,t_n[0].x,t_n[1].x,-c,b)));
            break;
          default: GECODE_NEVER;
          }
          break;
        case IRT_NQ:
          {
            NegBoolView n(b);
            switch (n_p) {
            case 2:
              GECODE_ES_FAIL(home,
                             (ReEqBin<int,IntView,IntView,NegBoolView>::post
                              (home,t_p[0].x,t_p[1].x,c,n)));
              break;
            case 1:
              GECODE_ES_FAIL(home,
                             (ReEqBin<int,IntView,MinusView,NegBoolView>::post
                              (home,t_p[0].x,t_n[0].x,c,b)));
              break;
            case 0:
              GECODE_ES_FAIL(home,
                             (ReEqBin<int,IntView,IntView,NegBoolView>::post
                              (home,t_p[0].x,t_p[1].x,-c,b)));
              break;
            default: GECODE_NEVER;
            }
          }
          break;
        case IRT_LQ:
          switch (n_p) {
          case 2:
            GECODE_ES_FAIL(home,(ReLqBin<int,IntView,IntView>::post
                                 (home,t_p[0].x,t_p[1].x,c,b)));
            break;
          case 1:
            GECODE_ES_FAIL(home,(ReLqBin<int,IntView,MinusView>::post
                                 (home,t_p[0].x,t_n[0].x,c,b)));
            break;
          case 0:
            GECODE_ES_FAIL(home,(ReLqBin<int,MinusView,MinusView>::post
                                 (home,t_n[0].x,t_n[1].x,c,b)));
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
      ViewArray<IntScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i].init(t_p[i].a,t_p[i].x);
      ViewArray<IntScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i].init(t_n[i].a,t_n[i].x);
      post_nary<int,IntScaleView>(home,x,y,r,c,b);
    } else {
      // Arbitrary coefficients with double precision
      ViewArray<DoubleScaleView> x(home,n_p);
      for (int i = n_p; i--; )
        x[i].init(t_p[i].a,t_p[i].x);
      ViewArray<DoubleScaleView> y(home,n_n);
      for (int i = n_n; i--; )
        y[i].init(t_n[i].a,t_n[i].x);
      post_nary<double,DoubleScaleView>(home,x,y,r,c,b);
    }
  }

}}}

// STATISTICS: int-post

