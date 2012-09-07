/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

namespace Gecode { namespace Int { namespace Branch {

  /// Post brancher according to \a v and \a vals
  template<class SelView>
  void
  post(Space& home, ViewArray<IntView>& x, SelView& v,
       const IntValBranch& vals, IntBranchFilter ibf) {
    switch (vals.select()) {
    case IntValBranch::SEL_MIN:
      {
        ValMin<IntView> a(home,vals);
        ViewValBrancher<SelView,ValMin<IntView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_MED:
      {
        ValMed<IntView> a(home,vals);
        ViewValBrancher<SelView,ValMed<IntView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_MAX:
      {
        ValMin<MinusView> a(home,vals);
        ViewValBrancher<SelView,ValMin<MinusView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_RND:
      {
        ValRnd<IntView> a(home,vals);
        ViewValBrancher<SelView,ValRnd<IntView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_SPLIT_MIN:
      {
        ValSplitMin<IntView> a(home,vals);
        ViewValBrancher<SelView,ValSplitMin<IntView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_SPLIT_MAX:
      {
        ValSplitMin<MinusView> a(home,vals);
        ViewValBrancher<SelView,ValSplitMin<MinusView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_RANGE_MIN:
      {
        ValRangeMin<IntView> a(home,vals);
        ViewValBrancher<SelView,ValRangeMin<IntView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_RANGE_MAX:
      {
        ValRangeMin<MinusView> a(home,vals);
        ViewValBrancher<SelView,ValRangeMin<MinusView> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_VAL_COMMIT:
      {
        ValSelValCommit<IntView,2> a(home,vals);
        ViewValBrancher<SelView,ValSelValCommit<IntView,2> >
          ::post(home,x,v,a,ibf);
      }
      break;
    case IntValBranch::SEL_VALUES_MIN:
      ViewValuesBrancher<SelView,IntView>::post(home,x,v,ibf);
      break;
    case IntValBranch::SEL_VALUES_MAX:
      ViewValuesBrancher<SelView,MinusView>::post(home,x,v,ibf);
      break;
    default:
      throw UnknownBranching("Int::branch");
    }
  }

}}}

// STATISTICS: int-branch
