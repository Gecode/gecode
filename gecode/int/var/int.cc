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

  IntVar::IntVar(Space* home, int min, int max)
    : var(new (home) Int::IntVarImp(home,min,max)) {
    if ((min < Limits::Int::int_min) || (max > Limits::Int::int_max))
      throw Int::VariableOutOfRangeDomain("IntVar");
    if (min > max)
      throw Int::VariableEmptyDomain("IntVar");
  }

  IntVar::IntVar(Space* home, const IntSet& ds)
    : var(new (home) Int::IntVarImp(home,ds)) {
    if ((ds.min() < Limits::Int::int_min) || (ds.max() > Limits::Int::int_max))
      throw Int::VariableOutOfRangeDomain("IntVar");
    if (ds.size() == 0)
      throw Int::VariableEmptyDomain("IntVar");
  }

}

// STATISTICS: int-var

