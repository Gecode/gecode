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
    for (int i = size(); i--; )
      x[i].init(home, m);
    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n, int a, int b)
    : VarArray<BddVar>(home,n) {
    // std::cout << "initialize array\n";
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
    if ((lbMin < Limits::Set::int_min) || 
	(lbMax > Limits::Set::int_max) ||
	(ubMin < Limits::Set::int_min) || 
	(ubMax > Limits::Set::int_max))
      throw Bdd::VariableOutOfRangeDomain("BddVarArray");
    if (maxCard > Limits::Set::card_max)
      throw Bdd::VariableOutOfRangeCardinality("BddVarArray");
    if (minCard > maxCard)
      throw Bdd::VariableFailedDomain("BddVarArray");
    for (int i = 0; i < size(); i++)
      x[i].init(home,m, lbMin,lbMax,ubMin,ubMax,minCard,maxCard);    
    
    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   const IntSet& glb,int ubMin,int ubMax,
			   unsigned int minCard,unsigned int maxCard)
    : VarArray<BddVar>(home,n) {
    if ( ((glb.size() > 0) &&
	  ((glb.min() < Limits::Set::int_min) ||
	   (glb.max() > Limits::Set::int_max))) ||
	 (ubMin < Limits::Set::int_min) || 
	 (ubMax > Limits::Set::int_max))
      throw Bdd::VariableOutOfRangeDomain("BddVar");
    if (maxCard > Limits::Set::card_max)
      throw Bdd::VariableOutOfRangeCardinality("BddVarArray");
    if (minCard > maxCard)
      throw Bdd::VariableFailedDomain("BddVarArray");
    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,ubMin,ubMax,minCard,maxCard);

    hls_order(home, *this);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   int lbMin,int lbMax,const IntSet& lub,
			   unsigned int minCard,unsigned int maxCard)
    : VarArray<BddVar>(home,n) {
    if ( ((lub.size() > 0) &&
	  ((lub.min() < Limits::Set::int_min) ||
	   (lub.max() > Limits::Set::int_max))) ||
	 (lbMin < Limits::Set::int_min) || 
	 (lbMax > Limits::Set::int_max))
      throw Bdd::VariableOutOfRangeDomain("BddVarArray");
    if (maxCard > Limits::Set::card_max)
      throw Bdd::VariableOutOfRangeCardinality("BddVarArray");
    if (minCard > maxCard)
      throw Bdd::VariableFailedDomain("BddVarArray");
    for (int i = 0; i < size(); i++)
      x[i].init(home, m, lbMin,lbMax,lub,minCard,maxCard);
  }

  BddVarArray::BddVarArray(Space* home, BMI* m, int n,
			   const IntSet& glb, const IntSet& lub,
			   unsigned int minCard, unsigned int maxCard)
    : VarArray<BddVar>(home,n) {
    // std::cerr << "BddVarArray glb="<<glb<<" lub="<<lub<<"\n";
    assert(m != NULL);
    // m->debug(std::cerr);
//     if (((glb.size() > 0) &&
// 	 ((glb.min() < Limits::Set::int_min) ||
// 	  (glb.max() > Limits::Set::int_max)))  ||
// 	((lub.size() > 0) &&
// 	 ((lub.min() < Limits::Set::int_min) ||
// 	  (lub.max() > Limits::Set::int_max))))
//       throw Bdd::VariableOutOfRangeDomain("BddVar");
//     if (maxCard > Limits::Set::card_max)
//       throw Bdd::VariableOutOfRangeCardinality("BddVar");
    for (int i = 0; i < size(); i++)
      x[i].init(home, m, glb,lub,minCard,maxCard);
//     if (minCard > maxCard)
//       throw Bdd::VariableFailedDomain("BddVar");
    hls_order(home, *this);
  }

}

// STATISTICS: set-other

