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

#include "gecode/set/projectors.hh"

namespace Gecode {

  int
  Projector::arity(void) const {
    return _arity;
  }

  void codeScope(Support::DynamicArray<int>& s, const SetExprCode& c,
                 bool monotone) {
    int tmp = 0;
    for (int i=0; i<c.size(); i++) {
      switch (c[i]) {
      case SetExprCode::COMPLEMENT:
      case SetExprCode::INTER:
      case SetExprCode::UNION:
      case SetExprCode::EMPTY:
      case SetExprCode::UNIVERSE:
        break;
      case SetExprCode::GLB:
        if (s[tmp] == Set::PC_SET_ANY+1)
          s[tmp] = monotone ? Set::PC_SET_CGLB : Set::PC_SET_CLUB;
        else if (monotone && s[tmp] != Set::PC_SET_CGLB)
          s[tmp] = Set::PC_SET_ANY;
        else if (!monotone && s[tmp] != Set::PC_SET_CLUB)
          s[tmp] = Set::PC_SET_ANY;
        break;
      case SetExprCode::LUB:
        if (s[tmp] == Set::PC_SET_ANY+1)
          s[tmp] = monotone ? Set::PC_SET_CLUB : Set::PC_SET_CGLB;
        else if (monotone && s[tmp] != Set::PC_SET_CLUB)
          s[tmp] = Set::PC_SET_ANY;
        else if (!monotone && s[tmp] != Set::PC_SET_CGLB)
          s[tmp] = Set::PC_SET_ANY;
        break;
      default:
        tmp = c[i]-SetExprCode::LAST;
        break;
      }
    }
  }

  void
  Projector::scope(Support::DynamicArray<int>& s) const {
    codeScope(s, glb, false);
    codeScope(s, lub, true);
  }

  ExecStatus
  Projector::check(Space*, ViewArray<Set::SetView>& x) {
    {
      // Check if glb violates current upper bound of x[i]
      SetExprRanges glbranges(x,glb,false);
      Iter::Ranges::Size<SetExprRanges> g(glbranges);
      Set::LubRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(g, xir))
        return ES_FAILED;
      while (g()) ++g;
      if (g.size() > x[i].cardMax()) {
        return ES_FAILED;
      }        
    }
    {
      // Check if lub violates current lower bound of x[i]
      SetExprRanges lubranges(x,lub,true);
      Iter::Ranges::Size<SetExprRanges> l(lubranges);
      Set::GlbRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(xir, l))
        return ES_FAILED;
      while (l()) ++l;
      if (l.size() < x[i].cardMin()) {
        return ES_FAILED;
      }
    }
    {
      // Check if monotone interpretation of lower bound is
      // contained in current lower bound of x[i]. If not, the constraint
      // is not subsumed.
      SetExprRanges glbranges(x,glb,true);
      Set::GlbRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(glbranges, xir)) {
        return ES_FIX;
      }
    }
    {
      // Check if current upper bound of x[i] if contained in
      // anti-monotone interpretation of upper bound. If not, the constraint
      // is not subsumed.
      SetExprRanges lubranges(x,lub,false);
      Set::LubRanges<Set::SetView> xir(x[i]);
      if (!Iter::Ranges::subset(xir, lubranges)) {
        return ES_FIX;
      }
    }
    // Both bounds, interpreted monotonically (glb) and anti-monotonically
    // (lub) are contained in the respective bounds of x[i]. This means
    // that the bounds are entailed.
    return __ES_SUBSUMED;
  }

}

std::ostream&
operator<<(std::ostream& os, const Gecode::Projector& p) {
  return os << p.getGlb() << " <= x[" << p.getIdx() << "] <= " << p.getLub()
            << std::endl;
}

// STATISTICS: set-prop
