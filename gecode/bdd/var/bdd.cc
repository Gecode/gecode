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

  
  BddVar::BddVar(Space* home, BMI* m) { 
    var = new (home) Bdd::BddVarImp(home, m); 
  }

  BddVar::BddVar(Space* home, BMI* m, int min, int max) {
    assert( m->level() < 4);
    var = new (home) Bdd::BddVarImp(home,m, min,max);
    assert( var->manager()->level() < 4);
  } 

  BddVar::BddVar(Space* home, BMI* m, int glbMin,int glbMax,
		 int lubMin,int lubMax, 
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::BddVarImp(home, m, glbMin, glbMax, 
				    lubMin, lubMax, cardMin, cardMax);
  }

  BddVar::BddVar(Space* home, BMI* m, const IntSet& glbD,
		 int lubMin,int lubMax, 
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::BddVarImp(home, m, glbD, lubMin, lubMax, 
				    cardMin, cardMax);
  }

  BddVar::BddVar(Space* home, BMI* m, int glbMin,int glbMax,
		 const IntSet& lubD,
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::BddVarImp(home, m, glbMin, glbMax, 
				    lubD, cardMin, cardMax);
  }

  BddVar::BddVar(Space* home, BMI* m, const IntSet& glbD, const IntSet& lubD,
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::BddVarImp(home, m,  glbD, lubD, cardMin, cardMax);
  }

}


// STATISTICS: bdd-var
