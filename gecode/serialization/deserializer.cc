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
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "gecode/serialization.hh"

namespace Gecode { namespace Serialization {
  
  Deserializer::Deserializer(Space* home0, Reflection::VarMap& m0)
    : home(home0), m(m0) {}

  Deserializer::~Deserializer(void) {}
    
  Reflection::VarMap&
  Deserializer::varMap(void) const {
    return m;
  }

  void
  Deserializer::var(Reflection::VarSpec& spec) {
    VarImpBase* vb = NULL;
    if (!spec.name().empty() &&
        (vb = m.var(spec.name())) != NULL) {
      // TODO: assert that spec and original var are compatible,
      // constrain domain of var to spec
      Reflection::registry.constrainVar(home, vb, spec);
    } else {
      vb = Reflection::registry.createVar(home, spec);
    }
    (void) m.put(vb, new Reflection::VarSpec(spec));
  }

  void
  Deserializer::post(Reflection::ActorSpec& spec) {
    Reflection::registry.post(home, m, spec);
  }
  
    
}}

// STATISTICS: serialization-any
