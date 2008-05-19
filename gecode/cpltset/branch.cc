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

#include "gecode/cpltset/branch.hh"

namespace Gecode {

  using namespace CpltSet;

  void
  branch(Space* home, const CpltSetVarArgs& xa,
         CpltSetVarBranch vars, CpltSetValBranch vals) {
    if (home->failed()) 
      return;
    ViewArray<CpltSetView> x(home,xa);
    switch (vars) {
    case CPLTSET_VAR_NONE:
      Branch::create<Branch::ByNone>(home,x,vals); break;
    case CPLTSET_VAR_SIZE_MIN:
      Branch::create<Branch::ByMinCard>(home,x,vals); break;
    case CPLTSET_VAR_SIZE_MAX:
      Branch::create<Branch::ByMaxCard>(home,x,vals); break;
    case CPLTSET_VAR_MIN_MIN:
      Branch::create<Branch::ByMinUnknown>(home,x,vals); break;
    case CPLTSET_VAR_MAX_MAX:
      Branch::create<Branch::ByMaxUnknown>(home,x,vals); break;
    default:
      throw CpltSet::UnknownBranching("CpltSet::branch");
    }
  }

}

// STATISTICS: cpltset-post
