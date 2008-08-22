/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

namespace Gecode {

  forceinline
  Projector::Projector(void) : i(0), glb(), lub() {}
  
  forceinline
  Projector::Projector(SetExpr::var_idx x,
                       const SetExpr& sglb, const SetExpr& slub)
    : i(x), glb(sglb.encode()), lub(slub.encode()),
      _arity(std::max(sglb.arity(), slub.arity())) {}

  template <bool negated>
  inline ExecStatus
  Projector::propagate(Space& home, ViewArray<Set::SetView>& x) {
    if (negated) {
      switch (ExecStatus es=check(home, x)) {
      case ES_FAILED: 
        return __ES_SUBSUMED;
      case __ES_SUBSUMED:
        return ES_FAILED;
      default: 
        return es;
      }
    } else {
      // Prune greatest lower and least upper bound according to given ranges
      {
        SetExprRanges glbranges(x,glb,false);
        GECODE_ME_CHECK(x[i].includeI(home, glbranges));
      }
      {
        SetExprRanges lubranges(x,lub,true);
        GECODE_ME_CHECK(x[i].intersectI(home, lubranges));
      }
      return ES_NOFIX;
    }
  }

  forceinline unsigned int
  Projector::glbSize(ViewArray<Set::SetView>& x) {
    SetExprRanges glbranges(x, glb, false);
    return Iter::Ranges::size(glbranges);
  }

  forceinline unsigned int
  Projector::lubSize(ViewArray<Set::SetView>& x) {
    SetExprRanges lubranges(x, lub, true);
    return Iter::Ranges::size(lubranges);
  }

  forceinline const SetExprCode&
  Projector::getGlb(void) const { return glb; }

  forceinline const SetExprCode&
  Projector::getLub(void) const { return lub; }

  forceinline int
  Projector::getIdx(void) const { return i; }

}

// STATISTICS: set-prop
