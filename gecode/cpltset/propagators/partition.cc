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
#include "gecode/cpltset/propagators.hh"

using namespace Gecode::CpltSet;

namespace Gecode {

  void partition(Space* home, const CpltSetVarArgs& x) {
    partition_con(home, x, false, SRT_EQ, false, -1);
  }

  void partition(Space* home, const CpltSetVarArgs& x, const CpltSetVar& y) {
    partition_con(home, x, y, false, SRT_EQ, false, -1);
  }

  void partition(Space* home, const IntVarArgs& x, const CpltSetVar& y) {
    partition_con(home, x, y, false, SRT_EQ, false, -1);
  }

  void partitionLex(Space* home, const CpltSetVarArgs& x, CpltSetRelType lex) {
    partition_con(home, x, true, lex, false, -1);
  }

  void partitionLexCard(Space* home, const CpltSetVarArgs& x,
                        CpltSetRelType lex, int c) {
    partition_con(home, x, true, lex, true, c);
  }

  void partitionCard(Space* home, const CpltSetVarArgs& x, int c) {
    partition_con(home, x, false, SRT_EQ, true, c);
  }
}

// STATISTICS: cpltset-post
