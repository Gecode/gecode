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

  CpltSetVar::CpltSetVar(Space* home, int glbMin,int glbMax,
                         int lubMin,int lubMax, 
                         unsigned int cardMin, unsigned int cardMax) {
    varimp = new (home) CpltSet::CpltSetVarImp(home, glbMin, glbMax, 
                                               lubMin, lubMax, cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, const IntSet& glbD,
                         int lubMin,int lubMax, 
                         unsigned int cardMin, unsigned int cardMax) {
    varimp = new (home) CpltSet::CpltSetVarImp(home,
                                               glbD, lubMin, lubMax, 
                                               cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, int glbMin,int glbMax,
                         const IntSet& lubD,
                         unsigned int cardMin, unsigned int cardMax) {
    varimp = new (home) CpltSet::CpltSetVarImp(home, glbMin, glbMax, 
                                               lubD, cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, const IntSet& glbD,
                         const IntSet& lubD,
                         unsigned int cardMin, unsigned int cardMax) {
    varimp = new (home) CpltSet::CpltSetVarImp(home, 
                                               glbD, lubD, cardMin, cardMax);
  }

}


// STATISTICS: cpltset-var
