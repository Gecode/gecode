/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

#include "gecode/int.hh"

namespace Gecode {

  IntVarArray::IntVarArray(Space* home, int n, int min, int max)
    : VarArray<IntVar>(home,n) {
    if ((min < Limits::Int::int_min) || (max > Limits::Int::int_max))
      throw Int::VariableOutOfRangeDomain("IntVarArray");
    if (min > max)
      throw Int::VariableEmptyDomain("IntVarArray");
    for (int i = size(); i--; )
      x[i].init(home,min,max);
  }

  IntVarArray::IntVarArray(Space* home, int n, const IntSet& s)
    : VarArray<IntVar>(home,n) {
    if ((s.min() < Limits::Int::int_min) || (s.max() > Limits::Int::int_max))
      throw Int::VariableOutOfRangeDomain("IntVarArray");
    if (s.size() == 0)
      throw Int::VariableEmptyDomain("IntVarArray");
    for (int i = size(); i--; )
      x[i].init(home,s);
  }

  BoolVarArray::BoolVarArray(Space* home, int n, int min, int max)
    : VarArray<BoolVar>(home, n) {
    if ((min < 0) || (max > 1))
      throw Int::VariableOutOfRangeDomain("BoolVarArray");
    if (min > max)
      throw Int::VariableEmptyDomain("BoolVarArray");
    for (int i = size(); i--; )
      x[i].init(home,min,max);
  }


}


// STATISTICS: int-post

