/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#include "gecode/serialization.hh"

namespace Gecode { namespace Serialization {
  
  void
  Deserializer::var(Reflection::VarSpec& spec) {
	  VarBase* vb;
    if (spec.name() != NULL &&
        (vb = m.variableByName(spec.name())) != NULL) {
      // TODO: assert that spec and original var are compatible,
      // constrain domain of var to spec
      v.push_back(vb);
    } else {
      v.push_back(registry.createVar(home, spec));
      (void) m.put(v[v.size()-1], new Reflection::VarSpec(spec));
    }
  }

  void
  Deserializer::post(Reflection::ActorSpec& spec) {
    registry.post(home, v, spec);
  }
  
    
}}

// STATISTICS: serialization-any
