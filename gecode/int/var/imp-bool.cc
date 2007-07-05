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

  Reflection::Arg*
  BoolVarImp::spec(Space* home, Reflection::VarMap& m) {
    int specIndex = m.getIndex(this);
    if (specIndex != -1)
      return new Reflection::VarArg(specIndex);
    Reflection::VarSpec* spec =
      new Reflection::VarSpec(VTI_BOOL, new Reflection::IntArg(dom));
    return (new Reflection::VarArg(m.put(this, spec)));
  }

}}

// STATISTICS: int-var
