/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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


#include "gecode/set.hh"

namespace Gecode {

  SetVar::SetVar(Space* home)
    : x(new (home) Set::SetVarImp(home)) {}

  SetVar::SetVar(Space* home,int lbMin,int lbMax,int ubMin,int ubMax,
                 unsigned int minCard, unsigned int maxCard)
    : x(new (home) Set::SetVarImp(home,lbMin,lbMax,ubMin,ubMax,
                                  minCard,maxCard)) {
    if ((lbMin < Limits::Set::int_min) || 
        (lbMax > Limits::Set::int_max) ||
        (ubMin < Limits::Set::int_min) || 
        (ubMax > Limits::Set::int_max))
      throw Set::VariableOutOfRangeDomain("SetVar");
    if (maxCard > Limits::Set::card_max)
      throw Set::VariableOutOfRangeCardinality("SetVar");
    if (minCard > maxCard)
      throw Set::VariableFailedDomain("SetVar");
  }

  SetVar::SetVar(Space* home, const IntSet& glb,int ubMin,int ubMax,
                 unsigned int minCard, unsigned int maxCard)
    : x(new (home) Set::SetVarImp(home,glb,ubMin,ubMax,minCard,maxCard)) {
    if ( ((glb.size() > 0) &&
          ((glb.min() < Limits::Set::int_min) ||
           (glb.max() > Limits::Set::int_max))) ||
         (ubMin < Limits::Set::int_min) || 
         (ubMax > Limits::Set::int_max))
      throw Set::VariableOutOfRangeDomain("SetVar");
    if (maxCard > Limits::Set::card_max)
      throw Set::VariableOutOfRangeCardinality("SetVar");
    if (minCard > maxCard)
      throw Set::VariableFailedDomain("SetVar");
  }

  SetVar::SetVar(Space* home,int lbMin,int lbMax,const IntSet& lub,
                 unsigned int minCard, unsigned int maxCard)
    : x(new (home) Set::SetVarImp(home,lbMin,lbMax,lub,minCard,maxCard)) {
    if ( ((lub.size() > 0) &&
          ((lub.min() < Limits::Set::int_min) ||
           (lub.max() > Limits::Set::int_max))) ||
         (lbMin < Limits::Set::int_min) || 
         (lbMax > Limits::Set::int_max))
      throw Set::VariableOutOfRangeDomain("SetVarArray");
    if (maxCard > Limits::Set::card_max)
      throw Set::VariableOutOfRangeCardinality("SetVar");
    if (minCard > maxCard)
      throw Set::VariableFailedDomain("SetVar");
  }

  SetVar::SetVar(Space* home,
                 const IntSet& glb, const IntSet& lub,
                 unsigned int minCard, unsigned int maxCard)
    : x(new (home) Set::SetVarImp(home,glb,lub,minCard,maxCard)) {
    if (((glb.size() > 0) &&
         ((glb.min() < Limits::Set::int_min) ||
          (glb.max() > Limits::Set::int_max)))  ||
        ((glb.size() > 0) &&
         ((lub.min() < Limits::Set::int_min) ||
          (lub.max() > Limits::Set::int_max))))
      throw Set::VariableOutOfRangeDomain("SetVar");
    if (maxCard > Limits::Set::card_max)
      throw Set::VariableOutOfRangeCardinality("SetVar");
    if (minCard > maxCard)
      throw Set::VariableFailedDomain("SetVar");
  }

}

// STATISTICS: set-var

