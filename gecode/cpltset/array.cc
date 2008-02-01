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

  namespace CpltSet {
    void 
    setVariableOrderFromArray(Space* home, const CpltSetVarArray& x) {
      int n = x.size();
      ViewArray<CpltSetView> view(home, n);
      for (int i = n; i--; ) {
        view[i] = x[i];
      }
      variableorder(view);
    }
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, int n,
                                   int lbMin,int lbMax,int ubMin,int ubMax,
                                   unsigned int minCard,
                                   unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {    
    Set::Limits::check(lbMin,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(lbMax,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(ubMin,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(ubMax,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(maxCard,"CpltSetVarArray::CpltSetVarArray");
    if (minCard > maxCard)
      throw Set::VariableEmptyDomain("CpltSetVarArray::CpltSetVarArray");

    IntSet glb(lbMin, lbMax);
    IntSet lub(ubMin, ubMax);      
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home,lbMin,lbMax,ubMin,ubMax,minCard,maxCard);    
    
    CpltSet::setVariableOrderFromArray(home, *this);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, int n,
                                   const IntSet& glb,int ubMin,int ubMax,
                                   unsigned int minCard,unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {
    Set::Limits::check(glb,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(ubMin,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(ubMax,"CpltSetVarArray::CpltSetVarArray");
    Set::Limits::check(maxCard,"CpltSetVarArray::CpltSetVarArray");
    if (minCard > maxCard)
      throw Set::VariableEmptyDomain("CpltSetVarArray::CpltSetVarArray");

    IntSet lub(ubMin, ubMax);
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, glb,ubMin,ubMax,minCard,maxCard);

    CpltSet::setVariableOrderFromArray(home, *this);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, int n,
                                   int lbMin,int lbMax,const IntSet& lub,
                                   unsigned int minCard,unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {
    IntSet glb(lbMin, lbMax);      
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, lbMin, lbMax, lub, minCard, maxCard);
  }

  CpltSetVarArray::CpltSetVarArray(Space* home, int n,
                                   const IntSet& glb, const IntSet& lub,
                                   unsigned int minCard, unsigned int maxCard)
    : VarArray<CpltSetVar>(home,n) {
    CpltSet::testConsistency(glb, lub, minCard, maxCard, "CpltSetVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, glb,lub,minCard,maxCard);
    CpltSet::setVariableOrderFromArray(home, *this);
  }

}

// STATISTICS: cpltset-other
