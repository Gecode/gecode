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

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m, int n) 
    : VarArray<CpltSetVar>(home,n) {
    for (int i = size(); i--; ) {
      x[i].init(home, m);
    }
    m.debug(std::cerr);
    // at this place we MUST NOT use the ordering function
    // since we did not allocate a variable so far
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m,
                                   int n, int a, int b)
    : VarArray<CpltSetVar>(home,n) {
    // dangerous! use correct offsets
    for (int i = 0; i < size();i++) {
      x[i].init(home, m, a, b);
    }
    CpltSet::hls_order(home, *this);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m, int n,
                                   int lbMin,int lbMax,int ubMin,int ubMax,
                                   unsigned int minCard,
                                   unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {
    
    IntSet glb(lbMin, lbMax);
    IntSet lub(ubMin, ubMax);      
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home,m, lbMin,lbMax,ubMin,ubMax,minCard,maxCard);    
    
    CpltSet::hls_order(home, *this);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m, int n,
                                   const IntSet& glb,int ubMin,int ubMax,
                                   unsigned int minCard,unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {

    IntSet lub(ubMin, ubMax);      
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,ubMin,ubMax,minCard,maxCard);

    CpltSet::hls_order(home, *this);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m, int n,
                                   int lbMin,int lbMax,const IntSet& lub,
                                   unsigned int minCard,unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {

    IntSet glb(lbMin, lbMax);      
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, lbMin, lbMax, lub, minCard, maxCard);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, BddMgr& m, int n,
                                   const IntSet& glb, const IntSet& lub,
                                   unsigned int minCard, unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {

    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,lub,minCard,maxCard);
    CpltSet::hls_order(home, *this);
  }

}

// STATISTICS: cpltset-other
