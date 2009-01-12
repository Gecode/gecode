/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode { namespace Set { namespace Branch {

  /// Post branching according to \a v and \a vals
  template <class SelView>
  void
  post(Space& home, ViewArray<SetView>& x, SelView& v,
       SetValBranch vals, const ValBranchOptions& o_vals) {
    switch (vals) {
    case SET_VAL_MIN_INC: 
      {
        ValMin<true> a(home,o_vals);
        (void) new (home) ViewValBranching<SelView,ValMin<true> >(home,x,v,a);
      }
      break;
    case SET_VAL_MIN_EXC: 
      {
        ValMin<false> a(home,o_vals);
        (void) new (home) ViewValBranching<SelView,ValMin<false> >(home,x,v,a);
      }
      break;
    case SET_VAL_MAX_INC:
      {
        ValMax<true> a(home,o_vals);
        (void) new (home) ViewValBranching<SelView,ValMax<true> >(home,x,v,a);
      }
      break;
    case SET_VAL_MAX_EXC:
      {
        ValMax<false> a(home,o_vals);
        (void) new (home) ViewValBranching<SelView,ValMax<false> >(home,x,v,a);
      }
      break;
    default:
      throw UnknownBranching("Set::branch");
    }
  }

  template <class SelView, class SelVal>
  void
  registerOne(void) {
    Reflection::registry().add(ViewValBranching<SelView,SelVal>::ati(), 
                               &ValSelToString<SelVal>::toString);
  }
  
  template <class SelView>
  void
  registerAll(void) {
    registerOne<SelView,ValMin<false> >();
    registerOne<SelView,ValMin<true> >();
    registerOne<SelView,ValMax<false> >();
    registerOne<SelView,ValMax<true> >();
  }

}}}

// STATISTICS: set-branch
