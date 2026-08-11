/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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

#include <gecode/word/rel.hh>

namespace Gecode {

  namespace {
    template<class View0, class View1>
    void post_rel(Home home, View0 x, WordRelType wrt, View1 y) {
      switch (wrt) {
      case WRT_EQ:
        GECODE_ES_FAIL((Word::Rel::Eq<View0,View1>::post(home,x,y)));
        break;
      case WRT_NQ:
        GECODE_ES_FAIL((Word::Rel::Nq<View0,View1>::post(home,x,y)));
        break;
      case WRT_ULQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View0,View1,false>::post(home,x,y)));
        break;
      case WRT_ULE:
        GECODE_ES_FAIL((Word::Rel::Le<View0,View1,false>::post(home,x,y)));
        break;
      case WRT_UGQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View1,View0,false>::post(home,y,x)));
        break;
      case WRT_UGR:
        GECODE_ES_FAIL((Word::Rel::Le<View1,View0,false>::post(home,y,x)));
        break;
      case WRT_SLQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View0,View1,true>::post(home,x,y)));
        break;
      case WRT_SLE:
        GECODE_ES_FAIL((Word::Rel::Le<View0,View1,true>::post(home,x,y)));
        break;
      case WRT_SGQ:
        GECODE_ES_FAIL((Word::Rel::Lq<View1,View0,true>::post(home,y,x)));
        break;
      case WRT_SGR:
        GECODE_ES_FAIL((Word::Rel::Le<View1,View0,true>::post(home,y,x)));
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }

    template<class View0, class View1, class CtrlView>
    void post_re_eq(Home home, View0 x, View1 y, CtrlView b,
                    ReifyMode rm) {
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_EQV>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_IMP>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReEq<
          View0,View1,CtrlView,RM_PMI>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    template<class View0, class View1, class CtrlView, bool sign>
    void post_re_lq(Home home, View0 x, View1 y, CtrlView b,
                    ReifyMode rm) {
      switch (rm) {
      case RM_EQV:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_EQV,sign>::post(home,x,y,b)));
        break;
      case RM_IMP:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_IMP,sign>::post(home,x,y,b)));
        break;
      case RM_PMI:
        GECODE_ES_FAIL((Word::Rel::ReLq<
          View0,View1,CtrlView,RM_PMI,sign>::post(home,x,y,b)));
        break;
      default:
        throw Word::UnknownReifyMode("Word::rel");
      }
    }

    template<class View0, class View1, class CtrlView, bool sign>
    void post_re_not_lq(Home home, View0 x, View1 y, CtrlView b,
                        ReifyMode rm) {
      Int::NegBoolView nb(b);
      switch (rm) {
      case RM_EQV: post_re_lq<View0,View1,Int::NegBoolView,sign>(
          home,x,y,nb,RM_EQV); break;
      case RM_IMP: post_re_lq<View0,View1,Int::NegBoolView,sign>(
          home,x,y,nb,RM_PMI); break;
      case RM_PMI: post_re_lq<View0,View1,Int::NegBoolView,sign>(
          home,x,y,nb,RM_IMP); break;
      default: throw Word::UnknownReifyMode("Word::rel");
      }
    }

    template<class View0, class View1>
    void post_rel(Home home, View0 x, WordRelType wrt, View1 y, Reify r) {
      Int::BoolView b(r.var());
      switch (wrt) {
      case WRT_EQ:
        post_re_eq(home,x,y,b,r.mode());
        break;
      case WRT_NQ: {
        Int::NegBoolView nb(b);
        switch (r.mode()) {
        case RM_EQV: post_re_eq(home,x,y,nb,RM_EQV); break;
        case RM_IMP: post_re_eq(home,x,y,nb,RM_PMI); break;
        case RM_PMI: post_re_eq(home,x,y,nb,RM_IMP); break;
        default: throw Word::UnknownReifyMode("Word::rel");
        }
        break;
      }
      case WRT_ULQ:
        post_re_lq<View0,View1,Int::BoolView,false>(
          home,x,y,b,r.mode());
        break;
      case WRT_ULE:
        post_re_not_lq<View1,View0,Int::BoolView,false>(
          home,y,x,b,r.mode());
        break;
      case WRT_UGQ:
        post_re_lq<View1,View0,Int::BoolView,false>(
          home,y,x,b,r.mode());
        break;
      case WRT_UGR:
        post_re_not_lq<View0,View1,Int::BoolView,false>(
          home,x,y,b,r.mode());
        break;
      case WRT_SLQ:
        post_re_lq<View0,View1,Int::BoolView,true>(
          home,x,y,b,r.mode());
        break;
      case WRT_SLE:
        post_re_not_lq<View1,View0,Int::BoolView,true>(
          home,y,x,b,r.mode());
        break;
      case WRT_SGQ:
        post_re_lq<View1,View0,Int::BoolView,true>(
          home,y,x,b,r.mode());
        break;
      case WRT_SGR:
        post_re_not_lq<View0,View1,Int::BoolView,true>(
          home,x,y,b,r.mode());
        break;
      default:
        throw Word::UnknownRelation("Word::rel");
      }
    }
  }

  void
  rel(Home home, WordVar x, WordRelType wrt, WordVar y) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    Word::WordView xv(x), yv(y);
    post_rel(home,xv,wrt,yv);
  }

  void
  rel(Home home, WordVar x, WordRelType wrt, WordVar y, Reify r) {
    if (x.width() != y.width())
      throw Word::WidthMismatch("Word::rel");
    GECODE_POST;
    Word::WordView xv(x), yv(y);
    post_rel(home,xv,wrt,yv,r);
  }

  void
  rel(Home home, WordVar x, WordRelType wrt,
      unsigned int width, WordValue value) {
    if (x.width() != width)
      throw Word::WidthMismatch("Word::rel");
    Word::ConstWordView c(width,value);
    GECODE_POST;
    post_rel(home,Word::WordView(x),wrt,c);
  }

  void
  rel(Home home, WordVar x, WordRelType wrt,
      unsigned int width, WordValue value, Reify r) {
    if (x.width() != width)
      throw Word::WidthMismatch("Word::rel");
    Word::ConstWordView c(width,value);
    GECODE_POST;
    post_rel(home,Word::WordView(x),wrt,c,r);
  }

}

// STATISTICS: word-post
