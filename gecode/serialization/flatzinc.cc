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

using namespace std;

namespace Gecode {
  using namespace Reflection;
  
  namespace {
    
    void emitIntVar(ostream& os, int varNo, VarSpec& vs) {
      os << "var ";
      Arg* dom = vs.dom();
      if (!dom->isIntArray())
        throw Exception("Serialization",
        "Internal error: invalid domain specification for IntVar.");
      IntArrayArg* a = dom->toIntArray();
      if (a->size() == 2) {
        os << (*a)[0] << ".." << (*a)[1];
      } else {
        os << "{";
        for (int i=0; i<a->size(); i+=2) {
          for (int j=(*a)[i]; j<(*a)[i+1]; j++)
            os << j << (i==a->size()-2 && j==(*a)[i+1]-1 ? "" : ",");
        }
        os << "}";
      }
      if (vs.name() != NULL)
        os << ": " << vs.name() << ";" << endl;
      else
        os << ": _v" << varNo << ";" << endl;      
    }
    void emitBoolVar(ostream& os, int varNo, VarSpec& vs) {
      os << "var ";
      Arg* dom = vs.dom();
      if (!dom->isInt())
        throw Exception("Serialization",
        "Internal error: invalid domain specification for BoolVar.");
      switch (dom->toInt()) {
        case 0: os << "false..true"; break;
        case 1: os << "false"; break;
        case 2: os << "true"; break;
        default:
          throw Exception("Serialization",
            "Internal error: invalid domain specification for BoolVar.");
      }
      if (vs.name() != NULL)
        os << ": " << vs.name() << ";" << endl;
      else
        os << ": _v" << varNo << ";" << endl;
    }
#ifdef GECODE_HAVE_SET_VARS
    void emitSetVar(ostream& os, int varNo, VarSpec& vs) {
      os << "var set of ";
      Arg* dom = vs.dom();
      if (! (dom->isPair() && 
             dom->first()->isPair() && dom->second()->isPair() &&
             dom->first()->first()->isIntArray() &&
             dom->second()->first()->isIntArray() &&
             dom->first()->second()->isInt() &&
             dom->second()->second()->isInt()))
        throw Exception("Serialization",
        "Internal error: invalid domain specification for SetVar.");

      int lbCard = 0;
      int ubCard = 0;
      
      // Output upper bound
      IntArrayArg* a = dom->second()->first()->toIntArray();
      if (a->size() == 2) {
        os << (*a)[0] << ".." << (*a)[1];
        ubCard = (*a)[1] - (*a)[0];
      } else {
        os << "{";
        for (int i=0; i<a->size(); i+=2) {
          for (int j=(*a)[i]; j<(*a)[i+1]; j++) {
            os << j << (i==a->size()-2 && j==(*a)[i+1]-1 ? "" : ",");
            ubCard += (*a)[i+1] - (*a)[i];
          }
        }
        os << "}";
      }
      if (vs.name() != NULL)
        os << ": " << vs.name() << ";" << endl;
      else
        os << ": _v" << varNo << ";" << endl;

      // Output additional constraints if lower bound is given and/or
      // cardinality is tighter than bounds

      if (dom->first()->first()->toIntArray()->size() != 0) {
        os << "constraint subset(";
        a = dom->first()->first()->toIntArray();
        if (a->size() == 2) {
          os << (*a)[0] << ".." << (*a)[1];
        } else {
          os << "{";
          for (int i=0; i<a->size(); i+=2) {
            for (int j=(*a)[i]; j<(*a)[i+1]; j++)
              os << j << (i==a->size()-2 && j==(*a)[i+1]-1 ? "" : ",");
          }
          os << "}";
        }
        if (vs.name() != NULL)
          os << "," << vs.name() << ");" << endl;
        else
          os << ", _v" << varNo << ");" << endl;
      }

      if (lbCard != dom->first()->second()->toInt() ||
          ubCard != dom->second()->second()->toInt()) {
        os << "constraint cardinality(";
        if (vs.name() != NULL)
          os << vs.name() << ", ";
        else
          os << "_v" << varNo;
        os << ", " << lbCard << ", " << ubCard << ");" << endl;
        
      }

    }
#endif
    
    void emitVar(ostream& os, int v, VarMap& vm) {
      VarSpec& vs = vm.spec(v);
      if (vs.name() == NULL)
        os << "_v" << v;
      else
        os << vs.name();
    }
    
    void emitVarArray(ostream& os, ArrayArg* a, VarMap& vm) {
      for (int i=0; i<a->size(); i++)
        emitVar(os, (*a)[i]->toVar(), vm);
    }
    
    void emitArray(ostream& os, ArrayArg* a, VarMap& vm) {
      if ((*a)[0]->isInt()) {
        os << "[";
        for (int i=0; i<a->size(); i++) {
          os << (*a)[i]->toInt();
          if (i<a->size()-1)
            os << ", ";
        }
        os << "]";
        return;
      }

      if ((*a)[0]->isVar()) {
        os << "[";
        for (int i=0; i<a->size(); i++) {
          emitVar(os, (*a)[i]->toVar(), vm);
          if (i<a->size()-1)
            os << ",";
        }
        os << "]";
        return;
      }
      
      if ((*a)[0]->isPair()) {
        ArrayArg* aa = new ArrayArg(a->size());
        ArrayArg* ab = new ArrayArg(a->size());
        for (int i=0; i<a->size(); i++) {
          (*aa)[i] = (*a)[i]->first();
          (*ab)[i] = (*a)[i]->second();
        }
        emitArray(os, aa, vm);
        os << ", ";
        emitArray(os, ab, vm);
        // Clear array, so that only the array is deleted
        for (int i=0; i<a->size(); i++) {
          (*aa)[i] = NULL;
          (*ab)[i] = NULL;
        }
        delete aa;
        delete ab;
        return;
      }
      
      throw Exception("Serialization", "Specification not understood");
    }
    
    void emitArg(ostream& os, Arg* arg, VarMap& vm) {
      if (arg->isInt()) {
        os << arg->toInt();
        return;
      }
      if (arg->isString()) {
        os << "\"" << arg->toString() << "\"";
        return;
      }
      if (arg->isTypedArg()) {
        emitArg(os, arg->typedArg(), vm);
        return;
      }
      if (arg->isVar()) {
        VarSpec& s = vm.spec(arg->toVar());
        if (s.name() == NULL)
          os << "_v" << arg->toVar();
        else
          os << s.name();
        return;
      }
      if (arg->isIntArray()) {
        IntArrayArg* a = arg->toIntArray();
        os << "[";
        for (int i=0; i<a->size(); i++) {
          os << (*a)[i];
          if (i<a->size()-1)
            os << ", ";
        }
        os << "]";
        return;
      }
      if (arg->isArray()) {
        ArrayArg* a = arg->toArray();
        if (a->size() == 0) {
          os << "[]";
          return;
        }
        emitArray(os, a, vm);
        return;
      }
    }
    
  }
  
  void emitFlatzinc(Space* home, VarMap& vm, ostream& os) {
    if (home->status() == SS_FAILED)
      throw Exception("Serialization",
        "Attempt to serialize failed space");

    VarMapIter vmi(vm);
    int varCount = 0;
    for (SpecIter si = home->actorSpecs(vm); si(); ++si) {
      ActorSpec& s = si.actor();
      for (; vmi(); ++vmi, ++varCount) {
        VarSpec& vs = vmi.var();
        switch (vs.vti()) {
        case VTI_INT: emitIntVar(os, varCount, vs); break;
        case VTI_BOOL: emitBoolVar(os, varCount, vs); break;
#ifdef GECODE_HAVE_SET_VARS
        case VTI_SET: emitSetVar(os, varCount, vs); break;
#endif
        }
      }

      os << "constraint " << s.name();
      os << "(";
      for (int i=0; i<s.noOfArgs(); i++) {
        emitArg(os, s[i], vm);
        if (i<s.noOfArgs()-1)
          os << ", ";
      }
      os << ");" << endl;
    }
    
    
    
  }
  
}

// STATISTICS: serialization-any
