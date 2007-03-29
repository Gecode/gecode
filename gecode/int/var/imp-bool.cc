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
#include "gecode/int/var/imp-bool-body.icc"

namespace Gecode { namespace Int {

  BoolVarImp BoolVarImp::s_one(1);
  BoolVarImp BoolVarImp::s_zero(0);

  /*
   * Advisors
   *
   */
#if GECODE_USE_ADVISORS
  bool
  BoolVarImp::advisors(Space *home) {
    SubscriberType* b = idx[PC_BOOL_ADVISOR];
    SubscriberType* p = idx[PC_BOOL_ADVISOR+1];
    ModEvent me = dom==NONE ? ME_BOOL_NONE : ME_BOOL_VAL;
    int lo = dom==ONE, hi = dom==ZERO;
    while (p-- > b) {
      switch (static_cast<IntAdvisor*>(p->d())
              ->propagate(home,me,lo,hi)) {
      case __ES_SUBSUMED:
        break;
      case ES_FAILED:
        return false;
        break;
      default:
        break;
      }
    }
    return true;
  }
#endif
}}

// STATISTICS: int-var
