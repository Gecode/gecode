/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
 *
 *  Last modified:
 *     $Date: 2007-05-11 15:34:01 +0000 (Fr, 11 Mai 2007) $ by $Author: pekczynski $
 *     $Revision: 4588 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/bdd.hh"

namespace Gecode {

  BddVarArray::BddVarArray(Space* home, BMI* m, int n) 
    : VarArray<BddVar>(home,n) {
    for (int i = size(); i--; ) {
      x[i].init(home, m);
    }
    m->debug(std::cerr);
    // at this place we MUST NOT use the ordering function
    // since we did not allocate a variable so far
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n, int a, int b)
    : VarArray<BddVar>(home,n) {
    // dangerous use correct offsets
    for (int i = 0; i < size();i++) {
      x[i].init(home, m, a, b);
    }
    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   int lbMin,int lbMax,int ubMin,int ubMax,
			   unsigned int minCard,
			   unsigned int maxCard)
    : VarArray<BddVar>(home,n) {
    
    IntSet glb(lbMin, lbMax);
    IntSet lub(ubMin, ubMax);      
    testConsistency(glb, lub, minCard, maxCard, "BddVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home,m, lbMin,lbMax,ubMin,ubMax,minCard,maxCard);    
    
    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   const IntSet& glb,int ubMin,int ubMax,
			   unsigned int minCard,unsigned int maxCard)
    : VarArray<BddVar>(home,n) {

    IntSet lub(ubMin, ubMax);      
    testConsistency(glb, lub, minCard, maxCard, "BddVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,ubMin,ubMax,minCard,maxCard);

    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   int lbMin,int lbMax,const IntSet& lub,
			   unsigned int minCard,unsigned int maxCard)
    : VarArray<BddVar>(home,n) {

    IntSet glb(lbMin, lbMax);      
    testConsistency(glb, lub, minCard, maxCard, "BddVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, lbMin, lbMax, lub, minCard, maxCard);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   const IntSet& glb, const IntSet& lub,
			   unsigned int minCard, unsigned int maxCard)
    : VarArray<BddVar>(home,n) {

    testConsistency(glb, lub, minCard, maxCard, "BddVarArray");

    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,lub,minCard,maxCard);
    hls_order(home, *this);
  }

}

// STATISTICS: set-other

