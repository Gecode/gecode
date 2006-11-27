/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
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

  BoolVar::BoolVar(Space* home, int min, int max) {
    if ((min < 0) || (max > 1))
      throw Int::VariableOutOfRangeDomain("BoolVar::BoolVar");
    if (min > max)
      throw Int::VariableEmptyDomain("BoolVar::BoolVar");
    if (min > 0)
      var = &Int::BoolVarImp::s_one;
    else if (max == 0)
      var = &Int::BoolVarImp::s_zero;
    else
      var = new (home) Int::BoolVarImp(home,0,1);
  }

  void
  BoolVar::init(Space* home, int min, int max) {
    if ((min < 0) || (max > 1))
      throw Int::VariableOutOfRangeDomain("BoolVar::init");
    if (min > max)
      throw Int::VariableEmptyDomain("BoolVar::init");
    if (min > 0)
      var = &Int::BoolVarImp::s_one;
    else if (max == 0)
      var = &Int::BoolVarImp::s_zero;
    else
      var = new (home) Int::BoolVarImp(home,0,1);
  }

}

// STATISTICS: int-var

