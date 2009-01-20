/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *
 *  Last modified:
 *     $Date: 2009-01-13 14:17:21 +0100 (Di, 13 Jan 2009) $ by $Author: tack $
 *     $Revision: 8063 $
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
  post(Space& home, ViewArray<Gecode::Int::BoolView>& x, SelView& v, 
       IntValBranch vals, const ValBranchOptions& o_vals) {
    switch (vals) {
    case INT_VAL_MIN:
    case INT_VAL_MED:
    case INT_VAL_SPLIT_MIN:
    case INT_VALUES_MIN:
      {
        Gecode::Int::Branch::ValZeroOne<Gecode::Int::BoolView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValZeroOne<Gecode::Int::BoolView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_MAX:
    case INT_VAL_SPLIT_MAX:
    case INT_VALUES_MAX:
      {
        Gecode::Int::Branch::ValZeroOne<Gecode::Int::NegBoolView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValZeroOne<Gecode::Int::NegBoolView> >
          (home,x,v,a);
      }
      break;
    case INT_VAL_RND:
      {
        Gecode::Int::Branch::ValRnd<Gecode::Int::BoolView> a(home,o_vals);
        (void) new (home) DecomposingViewValBranching<SelView,Gecode::Int::Branch::ValRnd<Gecode::Int::BoolView> >
          (home,x,v,a);
      }
      break;
    default:
      throw Gecode::Int::UnknownBranching("Decomposition::branch");
    }
  }

  template <class SelView, class SelVal>
  void
  registerOneBoolView(void) {
    Reflection::registry().add(DecomposingViewValBranching<SelView,SelVal>::ati(), 
                               &ValSelToString<SelVal>::toString);
  }
  
  template <class SelView>
  void
  registerAllBoolView(void) {
    // registerOneBoolView<SelView,ValZeroOne<BoolView> >();
    // registerOneBoolView<SelView,ValZeroOne<NegBoolView> >();
    // registerOneBoolView<SelView,ValRnd<BoolView> >();
  }


      
}}

// STATISTICS: int-branch
