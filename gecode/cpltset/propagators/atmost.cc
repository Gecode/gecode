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

#include "gecode/cpltset/propagators.hh"

using namespace Gecode::CpltSet;

namespace Gecode {



  void
  exactly(Space* home, CpltSetVar x, IntSet& is, int c) {
    if (home->failed()) return;

    ViewArray<CpltSetView> bv(home, 1);
    bv[0] = x;
    // std::cerr << "exactly on " << x << "\n";
    // SUBSUMPTION CHECK
    CpltSetVarGlbRanges glb(x);
    if (glb()) {
      IntSetRanges ir(is);
      Gecode::Iter::Ranges::Inter<CpltSetVarGlbRanges, IntSetRanges> inter(glb, ir);
      if (inter()) {
        // int v = inter.min();
        int s = inter.width();
        ++inter;
        if (!inter() && s == 1) {
          // std::cerr << "EXACTLY SUBSUMED!\n";
          // std::cerr << "IntSet = " << is << "\n";
          // std::cerr << "val = " << v << " and width = " << s << "\n";        
          return;
        } else {
          home->fail();
          return;
        }
      }
      // std::cerr << "DONE\n";
    }

    CpltSetVarUnknownRanges delta(x);
    IntSetRanges irange(is);
    Gecode::Iter::Ranges::Inter<CpltSetVarUnknownRanges, IntSetRanges> 
      interdel(delta, irange);
    if (!interdel()) {
      // std::cerr << "EMPTY INTERSECT\n";
      home->fail();
      return;
    } 

    int mi = interdel.min();
    int ma = interdel.max();
    int s = interdel.width();
    ++interdel;
    if (!interdel() && s == 1) {
      // std::cerr << "EXACTLY INCLUDE!\n";
      // std::cerr << "EXACTLY SUBSUMED!\n";
      // std::cerr << "IntSet = " << is << "\n";
      // std::cerr << "val = " << mi << " and width = " << s << "\n";        
      GECODE_ME_FAIL(home, bv[0].include(home, mi));
      // std::cerr << "OKDONE\n";
      return;
    }
    // std::cerr << "DONE\n";

    Iter::Ranges::SingletonAppend<
      Gecode::Iter::Ranges::Inter<CpltSetVarUnknownRanges, IntSetRanges>
      > si(mi,ma,interdel);

    BMI* mgr = bv[0].manager();
    unsigned int xtab = bv[0].table_width();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].mgr_min();
    
    GecodeBdd d = cardConst(mgr, xtab, xoff, xmin, c, c, si);
    // std::cerr << "exactly: tell...";
    // GECODE_ME_FAIL(home, bv[0].tell_formula(home, d));
    GECODE_ES_FAIL(home, UnaryBddProp<CpltSetView>::post(home, bv[0], d));
    // std::cerr << "TELL-OK\n";

  }

  void
  atmost(Space* home, CpltSetVar x, IntSet& is, int c) {
    if (home->failed()) return;

    // SUBSUMPTION CHECK
//     CpltSetVarLubRanges lub(x);
//     IntSetRanges ir(is);
//     if (!Gecode::Iter::Ranges::subset(ir, lub)) {
//       std::cout << "SUBSUMED\n";
//     }

    ViewArray<CpltSetView> bv(home, 1);
    bv[0] = x;

    BMI* mgr = bv[0].manager();
    unsigned int xtab = bv[0].table_width();
    unsigned int xoff = bv[0].offset();
    int xmin = bv[0].mgr_min();
    IntSetRanges ir(is);
    GecodeBdd d = cardConst(mgr, xtab, xoff, xmin, 0, c, ir);

    //GECODE_ME_FAIL(home, bv[0].tell_formula(home, d));

    GECODE_ES_FAIL(home, UnaryBddProp<CpltSetView>::post(home, bv[0], d));

  }

  void 
  atmost(Space* home, CpltSetVar x, CpltSetVar y, int c) {
    atmost_con(home, x, y, c, SRT_EQ, -1);
  }

  void 
  atmostLex(Space* home, CpltSetVar x, CpltSetVar y, int c, 
            CpltSetRelType lex) {
    atmost_con(home, x, y, c, lex, -1);
  }

  void 
  atmostLexCard(Space* home, CpltSetVar x, CpltSetVar y, int c, 
                CpltSetRelType lex, int d) {
    atmost_con(home, x, y, c, lex, d);
  }

  void 
  atmostCard(Space* home, CpltSetVar x, CpltSetVar y, int c, int d) {
    atmost_con(home, x, y, c, SRT_EQ, d);
  }

  void 
  atmost(Space* home, CpltSetVar x, CpltSetVar y, CpltSetVar z, int c) {
    atmost_con(home, x, y, z, c, SRT_EQ, -1);
  }

  void 
  atmostOne(Space* home, const CpltSetVarArgs& x, int c) {
    atmostOne_con(home, x, c);
  }


}

// STATISTICS: cpltset-post
