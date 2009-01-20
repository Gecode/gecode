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

#include <sstream>
#include <gecode/support/buddy/kernel.h>

namespace Gecode { namespace CpltSet {

  /// Print bound of complete set variable
  template<class Char, class Traits, class I>
  void
  printBound(std::basic_ostream<Char,Traits>& s, I& r) {
    s << '{';
    while (r()) {
      if (r.min() == r.max()) {
        s << r.min();
      } else if (r.min()+1 == r.max()) {
        s << r.min() << "," << r.max();
      } else {
        s << r.min() << ".." << r.max();
      }
      ++r;
      if (!r()) break;
      s << ',';
    }
    s << '}';
  }

  /// Print complete set variable domain represented by \a r to \a os
  template<class Char, class Traits>
  bool
  printDom(std::basic_ostream<Char,Traits>& s,
           int off, int min, int width, bool first, char* profile, bdd& r) {
    if (r == bdd_true()) {
      if (!first)
        s << ",";
      bool assigned = true;
      int last = -1;
      int lastUnknown = -1;
      for (int i = 0; i<width; i++) {
        if (profile[i] < 0) {
          assigned = false; lastUnknown = i;
        }
        if (profile[i] == 1) {
          last = i; lastUnknown = i;
        }
      }
      if (assigned) {
        s << "{";
        for (int i = 0; i<=last; i++) {
          if (profile[i] == 1)
            s << min+i << (i<last ? "," : "");
        }
        s << "}";
      } else {
        s << "{";
        for (int i = 0; i<=last; i++) {
          if (profile[i] == 1)
            s << min+i << (i<last ? "," : "");
        }
        s << "}..{";
        for (int i = 0; i<=lastUnknown; i++) {
          if (profile[i] != 0)
            s << min+i << (i<lastUnknown ? "," : "");
        }
        s << "}";
      }
      return false;
    }

    if (r == bdd_false())
      return first;

    if (bdd_low(r) != bdd_false()) {
      profile[bddlevel2var[r.getlevel()]-off] = 0;
      for (int v=bdd_low(r).getlevel()-1 ; v>r.getlevel() ; --v)
        profile[bddlevel2var[v]-off] = -1;
      bdd rlow = bdd_low(r);
      first = printDom(s, off, min, width, first, profile, rlow);
    }

    if (bdd_high(r) != bdd_false()) {
      profile[bddlevel2var[r.getlevel()]-off] = 1;
      for (int v=bdd_high(r).getlevel()-1 ; v>r.getlevel() ; --v)
        profile[bddlevel2var[v]-off] = -1;
      bdd rhigh = bdd_high(r);
      first = printDom(s, off, min, width, first, profile, rhigh);
    }
    return first;
  }



  template<class Char, class Traits>
  inline std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const CpltSetView& x) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    if (x.assigned()) {
      GlbValues<CpltSetView> glb(x);
      Iter::Values::ToRanges<GlbValues<CpltSetView> > glbr(glb);
      printBound(s, glbr);
    } else {
      bdd dom = x.dom();
      s << "{";
      char* profile = heap.alloc<char>(x.tableWidth());
      for (int i=x.tableWidth(); i--; )
        profile[i] = -1;
      printDom(s, x.offset(), x.initialLubMin(), x.tableWidth(), true,
               profile, dom);
      heap.rfree(profile);
      s << "}";
    }
    return os << s.str();
  }

}}

// STATISTICS: cpltset-var
