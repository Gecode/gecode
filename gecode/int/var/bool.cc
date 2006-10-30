/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date: 2006-10-25 15:21:37 +0200 (Wed, 25 Oct 2006) $ by $Author: tack $
 *     $Revision: 3791 $
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

  BoolVar::BoolVar(Space* home, int min, int max)
    : var(new (home) Int::BoolVarImp(home,min,max)) {
    if ((min < 0) || (max > 1))
      throw Int::VariableOutOfRangeDomain("BoolVar::BoolVar");
    if (min > max)
      throw Int::VariableEmptyDomain("BoolVar::BoolVar");
  }

  void
  BoolVar::init(Space* home, int min, int max) {
    if ((min < 0) || (max > 1))
      throw Int::VariableOutOfRangeDomain("BoolVar::init");
    if (min > max)
      throw Int::VariableEmptyDomain("BoolVar::init");
    var = new (home) Int::BoolVarImp(home,min,max);
  }

}

// STATISTICS: int-var

