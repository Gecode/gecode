/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"

namespace Gecode {
  
  void 
  dom(Space* home, CpltSetVar x, SetRelType r, const IntSet& s) {
    Set::Limits::check(s, "CpltSet::dom");
    if (home->failed()) return;
    CpltSet::CpltSetView bv(x);

    switch(r) {
    case SRT_EQ:
      {
        if (s.size() == 1) {
          GECODE_ME_FAIL(home, bv.eq(home, s.min(), s.max()));
        } else {
          IntSetRanges ir(s);
          GECODE_ME_FAIL(home,bv.eqI(home, ir));
        }
      }
      break;
    case SRT_DISJ:
      {
        if (s.size() == 1) {
          GECODE_ME_FAIL(home,bv.exclude(home, s.min(), s.max()));
        } else {
          IntSetRanges rd(s);
          GECODE_ME_FAIL(home,bv.excludeI(home, rd));
        }
      }
      break;
    case SRT_NQ:
      {
        if (s.size() == 1) {
          GECODE_ME_FAIL(home, bv.nq(home, s.min(), s.max()));
        } else {
          IntSetRanges ir(s);
          GECODE_ME_FAIL(home, bv.nqI(home, ir));
        }
      }
      break;
    case SRT_SUB:
      {
         if (s.size() == 1) {
           GECODE_ME_FAIL(home,bv.intersect(home, s.min(), s.max()));
         } else {
           IntSetRanges rd(s);
           GECODE_ME_FAIL(home,bv.intersectI(home, rd));
         }
      }
      break;
    case SRT_SUP:
      {
        if (s.size() == 1) {
          GECODE_ME_FAIL(home,bv.include(home, s.min(), s.max()));
        } else {
          IntSetRanges rd(s);
          GECODE_ME_FAIL(home,bv.includeI(home, rd));
        }
      }
      break;
    case SRT_CMPL:
      {
        if (s.size() == 1) {
          GECODE_ME_FAIL(home,bv.exclude(home, s.min(), s.max()));
          GECODE_ME_FAIL(home,
                         bv.include(home,
                                    Set::Limits::min,
                                    s.min()-1) );
          GECODE_ME_FAIL(home,
                         bv.include(home, s.max()+1,
                                    Set::Limits::max) );
        } else {
          IntSetRanges rd1(s);
          Iter::Ranges::Compl<Set::Limits::min, 
            Set::Limits::max, IntSetRanges > rdC1(rd1);
          GECODE_ME_FAIL(home,bv.includeI(home, rdC1));
          IntSetRanges rd2(s);
          Iter::Ranges::Compl<Set::Limits::min, 
            Set::Limits::max, IntSetRanges > rdC2(rd2);
          GECODE_ME_FAIL(home,bv.intersectI(home, rdC2));
        }
      }
      break;
    default:
      {
        // could implement here lex ordering constraints s < 1 or something
        // thats nice
        throw CpltSet::InvalidRelation("rel lex not implemented");
        return;
      }
    }
  }

  void
  dom(Space* home, CpltSetVar x, SetRelType r, int i) {
    IntSet d(i,i);
    dom(home, x, r, d);
  }

  void
  dom(Space* home, CpltSetVar x, SetRelType r, int i, int j) {
    IntSet d(i,j);
    dom(home, x, r, d);
  }


}

// STATISTICS: cpltset-post
