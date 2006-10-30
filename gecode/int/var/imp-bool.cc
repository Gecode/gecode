/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
 *
 *  Last modified:
 *     $Date: 2006-10-30 18:30:04 +0100 (Mon, 30 Oct 2006) $ by $Author: schulte $
 *     $Revision: 3825 $
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

  /*
   * Copying variables
   *
   */

  forceinline
  BoolVarImp::BoolVarImp(Space* home, bool share, BoolVarImp& x)
    : BoolVarImpBase(home,share,x), dom(x.dom) {}

  BoolVarImp*
  BoolVarImp::perform_copy(Space* home, bool share) {
    return new (home) BoolVarImp(home,share,*this);
  }

}}

// STATISTICS: int-var
