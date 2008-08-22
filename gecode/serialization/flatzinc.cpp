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
#ifdef GECODE_HAS_INT_VARS
#include "gecode/int.hh"
#endif
#ifdef GECODE_HAS_SET_VARS
#include "gecode/set.hh"
#endif

namespace Gecode {
  
  namespace {
#ifdef GECODE_HAS_INT_VARS
    void emitIntVar(std::ostream& os, int varNo, Reflection::VarSpec& vs) {
      using namespace std;
      os << "var ";
      Reflection::Arg* dom = vs.dom();
      if (!dom->isIntArray())
        throw Exception("Serialization",
        "Internal error: invalid domain specification for IntVar.");
      Reflection::IntArrayArg* a = dom->toIntArray();
      if (a->size() == 2) {
        os << (*a)[0] << ".." << (*a)[1];
      } else {
        os << "{";
        for (int i=0; i<a->size(); i+=2) {
          for (int j=(*a)[i]; j<=(*a)[i+1]; j++)
            os << j << (i==a->size()-2 && j==(*a)[i+1] ? "" : ",");
        }
        os << "}";
      }
      if (!vs.name().empty()) {
        os << ": " << vs.name() << ";" << endl;
      }
      else
        os << ": _v" << varNo << ";" << endl;      
    }
    void emitBoolVar(std::ostream& os, int varNo, Reflection::VarSpec& vs) {
      using namespace std;
      os << "var ";
      Reflection::Arg* dom = vs.dom();
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
      if (!vs.name().empty()) {
        os << ": " << vs.name() << ";" << endl;
      }
      else
        os << ": _v" << varNo << ";" << endl;
    }
#endif
#ifdef GECODE_HAS_SET_VARS
    void emitSetVar(std::ostream& os, int varNo, Reflection::VarSpec& vs) {
      using namespace std;
      os << "var set of ";
      Reflection::Arg* dom = vs.dom();
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
      Reflection::IntArrayArg* a = dom->second()->first()->toIntArray();
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
      if (!vs.name().empty()) {
        os << ": " << vs.name() << ";" << endl;
      }
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
        if (!vs.name().empty()) {
          os << "," << vs.name() << ");" << endl;
        }
        else
          os << ", _v" << varNo << ");" << endl;
      }

      if (lbCard != dom->first()->second()->toInt() ||
          ubCard != dom->second()->second()->toInt()) {
        os << "constraint cardinality(";
        if (!vs.name().empty())
          os << vs.name() << ", ";
        else
          os << "_v" << varNo;
        os << ", " << lbCard << ", " << ubCard << ");" << endl;
        
      }

    }
#endif
    
    void emitVar(std::ostream& os, int v, Reflection::VarMap& vm) {
      using namespace std;
      Reflection::VarSpec& vs = vm.spec(v);
      if (vs.name().empty())
        os << "_v" << v;
      else
        os << vs.name();
    }
    
    void emitVarArray(std::ostream& os, Reflection::ArrayArg* a, 
                      Reflection::VarMap& vm) {
      using namespace std;
      for (int i=0; i<a->size(); i++)
        emitVar(os, (*a)[i]->toVar(), vm);
    }
    
    void emitArray(std::ostream& os, Reflection::ArrayArg* a,
                   Reflection::VarMap& vm) {
      using namespace std;
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
        Reflection::ArrayArg* aa = Reflection::Arg::newArray(a->size());
        Reflection::ArrayArg* ab = Reflection::Arg::newArray(a->size());
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
    
    void emitArg(std::ostream& os, Reflection::Arg* arg, 
                 Reflection::VarMap& vm) {
      using namespace std;
      if (arg->isInt()) {
        os << arg->toInt();
        return;
      }
      if (arg->isString()) {
        os << "\"" << arg->toString() << "\"";
        return;
      }
      if (arg->isVar()) {
        Reflection::VarSpec& s = vm.spec(arg->toVar());
        if (s.name().empty())
          os << "_v" << arg->toVar();
        else
          os << s.name();
        return;
      }
      if (arg->isIntArray()) {
        Reflection::IntArrayArg* a = arg->toIntArray();
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
        Reflection::ArrayArg* a = arg->toArray();
        if (a->size() == 0) {
          os << "[]";
          return;
        }
        emitArray(os, a, vm);
        return;
      }
      if (arg->isSharedReference()) {
        os << "_array" << arg->toSharedReference();
        return;
      }
      assert(!arg->isSharedObject());
      throw Exception("Serialization", "Specification not understood");
    }

    void emitSharedObject(std::ostream& os, int soCount,
                          Reflection::VarMap& vm,
                          Reflection::Arg* arg0) {
      using namespace std;
      Reflection::Arg* arg = arg0->toSharedObject();
      if (arg->isIntArray()) {
        Reflection::IntArrayArg* a = arg->toIntArray();
        os << "array[0.."<<a->size()-1<<"] of int: _array" << soCount << " = ";
        os << "[";
        for (int i=0; i<a->size(); i++) {
          os << (*a)[i];
          if (i<a->size()-1)
            os << ", ";
        }
        os << "];" << std::endl;
        return;
      }
      if (arg->isArray()) {
        Reflection::ArrayArg* a = arg->toArray();
        os << "array[0.."<<a->size()-1<<"] of int: _array" << soCount << " = ";
        if (a->size() == 0) {
          os << "[];" << std::endl;
          return;
        }
        emitArray(os, a, vm);
        os << ";" << std::endl;
        return;
      }
      return;    
    }

    void emitVarMap(std::ostream& os, int& varCount,
                    Reflection::VarMapIter& vmi) {
      for (; vmi(); ++vmi, ++varCount) {
        Reflection::VarSpec& vs = vmi.spec();
        if (false) { }
#ifdef GECODE_HAS_INT_VARS
        else if (vs.vti() == Int::IntVarImp::vti)
          emitIntVar(os, varCount, vs);
        else if (vs.vti() == Int::BoolVarImp::vti)
          emitBoolVar(os, varCount, vs);
#endif
#ifdef GECODE_HAS_SET_VARS
        else if (vs.vti() == Set::SetVarImp::vti)
          emitSetVar(os, varCount, vs);
#endif        
      }
    }
  }
  
  
  void emitFlatzinc(Space& home, std::ostream& os) {
    using namespace std;
    Reflection::VarMap vm;
    home.getVars(vm, false);
    Reflection::VarMapIter vmi(vm);
    int varCount = 0;
    int soCount = 0;
    emitVarMap(os,varCount,vmi);
    for (Reflection::ActorSpecIter si(home, vm); si(); ++si) {
      Reflection::ActorSpec s = si.actor();

      emitVarMap(os,varCount,vmi);

      int soBase = soCount;
      for (int i=0; i<s.noOfArgs(); i++) {
        if (s[i] && s[i]->isSharedObject())
          emitSharedObject(os, soBase++, vm, s[i]);
      }

      os << "constraint " << s.ati() << "(";

      soBase = soCount;
      for (int i=0; i<s.noOfArgs(); i++) {
        if (s[i] == NULL)
          os << "[]";
        else if (s[i]->isSharedObject())
          os << "_array" << soBase++;
        else
          emitArg(os, s[i], vm);
        if (i<s.noOfArgs()-1)
          os << ", ";
      }
      os << ");" << endl;
      soCount = soBase;
    }
  }
  
}

// STATISTICS: serialization-any
