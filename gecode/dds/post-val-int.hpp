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

namespace Gecode { namespace Decomposition {

  /// Post branching according to \a v and \a vals
  template <class SelView>
  void
  post(Space& home, ViewArray<Gecode::Int::IntView>& x, SelView& v,
       IntValBranch vals, const ValBranchOptions& o_vals) {
    switch (vals) {
    case INT_VAL_MIN:
      {
        Gecode::Int::Branch::ValMin<Gecode::Int::IntView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValMin<Gecode::Int::IntView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_MED:
      {
        Gecode::Int::Branch::ValMed<Gecode::Int::IntView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValMed<Gecode::Int::IntView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_MAX:
      {
        Gecode::Int::Branch::ValMin<Gecode::Int::MinusView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValMin<Gecode::Int::MinusView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_RND:
      {
        Gecode::Int::Branch::ValRnd<Gecode::Int::IntView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValRnd<Gecode::Int::IntView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_SPLIT_MIN:
      {
        Gecode::Int::Branch::ValSplitMin<Gecode::Int::IntView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValSplitMin<Gecode::Int::IntView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_SPLIT_MAX:
      {
        Gecode::Int::Branch::ValSplitMin<Gecode::Int::MinusView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValSplitMin<Gecode::Int::MinusView> >
          (home,x,v,a);
      }
      break;
    default:
      throw Gecode::Int::UnknownBranching("Decomposition::branch");
    }
  }

  template <class SelView, class SelVal>
  void
  registerOneIntView(void) {
    Reflection::registry().add(DecomposingViewValBranching<SelView,SelVal>::ati(),
                               &ValSelToString<SelVal>::toString);
  }

  template <class SelView>
  void
  registerAllIntView(void) {
    // registerOneIntView<SelView,ValMin<Gecode::Int::IntView> >();
    // registerOneIntView<SelView,ValMed<Gecode::Int::IntView> >();
    // registerOneIntView<SelView,ValMin<MinusView> >();
    // registerOneIntView<SelView,ValRnd<Gecode::Int::IntView> >();
    // registerOneIntView<SelView,ValSplitMin<Gecode::Int::IntView> >();
    // registerOneIntView<SelView,ValSplitMin<MinusView> >();
  }

}}

// STATISTICS: int-branch
