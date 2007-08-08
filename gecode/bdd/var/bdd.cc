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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/bdd.hh"

namespace Gecode {

  
  CpltSetVar::CpltSetVar(Space* home, BMI* m) { 
    var = new (home) Bdd::CpltSetVarImp(home, m); 
  }

  CpltSetVar::CpltSetVar(Space* home, BMI* m, int min, int max) {
    assert( m->level() < 4);
    var = new (home) Bdd::CpltSetVarImp(home,m, min,max);
    assert( var->manager()->level() < 4);
  } 

  CpltSetVar::CpltSetVar(Space* home, BMI* m, int glbMin,int glbMax,
		 int lubMin,int lubMax, 
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::CpltSetVarImp(home, m, glbMin, glbMax, 
				    lubMin, lubMax, cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, BMI* m, const IntSet& glbD,
		 int lubMin,int lubMax, 
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::CpltSetVarImp(home, m, glbD, lubMin, lubMax, 
				    cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, BMI* m, int glbMin,int glbMax,
		 const IntSet& lubD,
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::CpltSetVarImp(home, m, glbMin, glbMax, 
				    lubD, cardMin, cardMax);
  }

  CpltSetVar::CpltSetVar(Space* home, BMI* m, const IntSet& glbD, const IntSet& lubD,
		 unsigned int cardMin, unsigned int cardMax) {
    var = new (home) Bdd::CpltSetVarImp(home, m,  glbD, lubD, cardMin, cardMax);
  }

}


// STATISTICS: bdd-var
