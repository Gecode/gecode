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

#include "gecode/int.hh"
#ifdef GECODE_HAVE_SET_VARS
#include "gecode/set.hh"
#endif

#include "gecode/int/element.hh"

namespace Gecode { namespace Serialization {  

  namespace {
    
    VarImpBase* createIntVar(Space* home, Reflection::VarSpec& spec) {
      Reflection::IntArrayArgRanges ai(spec.dom()->toIntArray());
      VarImpBase* ret = Int::IntView(IntVar(home, IntSet(ai))).var();
      return ret;
    }
    void constrainIntVar(Space* home, VarImpBase* v, Reflection::VarSpec& spec) {
      Reflection::IntArrayArgRanges ai(spec.dom()->toIntArray());
      dom(home, IntVar(Int::IntView(static_cast<Int::IntVarImp*>(v))), 
          IntSet(ai));
    }
    VarImpBase* createBoolVar(Space* home, Reflection::VarSpec& spec) {
      int dom = spec.dom()->toInt();
      int min = 0;
      int max = 1;
      if (dom == Int::BoolVarImp::ZERO)
        max = 0;
      else if (dom == Int::BoolVarImp::ONE)
        min = 1;
      VarImpBase* ret = Int::BoolView(BoolVar(home, min, max)).var();
      return ret;
    }
    void constrainBoolVar(Space* home, VarImpBase* v,
                          Reflection::VarSpec& spec) {
      int d = spec.dom()->toInt();
      if (d == Int::BoolVarImp::ZERO)
        rel(home, BoolVar(Int::BoolView(static_cast<Int::BoolVarImp*>(v))),
            IRT_EQ, 0);
      else if (d == Int::BoolVarImp::ONE)
        rel(home, BoolVar(Int::BoolView(static_cast<Int::BoolVarImp*>(v))),
            IRT_EQ, 1);
    }
#ifdef GECODE_HAVE_SET_VARS
    VarImpBase* createSetVar(Space* home, Reflection::VarSpec& spec) {
      int cardMin = spec.dom()->first()->second()->toInt();
      int cardMax = spec.dom()->second()->second()->toInt();
      Reflection::IntArrayArgRanges 
        glb(spec.dom()->first()->first()->toIntArray());
      Reflection::IntArrayArgRanges 
        lub(spec.dom()->second()->first()->toIntArray());
      VarImpBase* ret = Set::SetView(SetVar(home, IntSet(glb), IntSet(lub), 
                                  cardMin, cardMax)).var();
      return ret;
    }
    void constrainSetVar(Space* home, VarImpBase* v,
                         Reflection::VarSpec& spec) {
      int cardMin = spec.dom()->first()->second()->toInt();
      int cardMax = spec.dom()->second()->second()->toInt();
      Reflection::IntArrayArgRanges 
        glb(spec.dom()->first()->first()->toIntArray());
      Reflection::IntArrayArgRanges 
        lub(spec.dom()->second()->first()->toIntArray());
      SetVar sv(Set::SetView(static_cast<Set::SetVarImp*>(v)));
      dom(home, sv, SRT_SUP, IntSet(glb));
      dom(home, sv, SRT_SUB, IntSet(lub));
      cardinality(home, sv, cardMin, cardMax);
    }
#endif

    class VariableCreators {
    public:
        VariableCreators() {
        Reflection::registry().add("VTI_INT", createIntVar);
        Reflection::registry().add("VTI_INT", constrainIntVar);
        Reflection::registry().add("VTI_BOOL", createBoolVar);
        Reflection::registry().add("VTI_BOOL", constrainBoolVar);
#ifdef GECODE_HAVE_SET_VARS
        Reflection::registry().add("VTI_SET", createSetVar);
        Reflection::registry().add("VTI_SET", constrainSetVar);
#endif
      }
    };
    VariableCreators __variableCreators;
  }
    
}}

// STATISTICS: serialization-any
