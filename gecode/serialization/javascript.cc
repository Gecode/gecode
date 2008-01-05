/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2007-12-11 14:41:35 +0100 (Di, 11 Dez 2007) $ by $Author: tack $
 *     $Revision: 5668 $
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

namespace Gecode {
  
  namespace {
    
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
        os << "[";
        for (int i=0; i<a->size(); i++) {
          emitArg(os, (*a)[i], vm);
          if (i<a->size()-1)
            os << ", ";
        }
        os << "]";
        return;
      }
      if (arg->isSharedReference()) {
        os << "_array" << arg->toSharedReference();
        return;
      }
      if (arg->isPair()) {
        os << "pair(";
        emitArg(os, arg->first(), vm);
        os << ", ";
        emitArg(os, arg->second(), vm);
        os << ")";
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
      os << "var _array" << soCount << " = ";
      emitArg(os, arg, vm);
      os << ";" << endl;
    }
    
  }
  
  void emitJavaScript(Space* home, std::ostream& os) {
    using namespace std;
    Reflection::VarMap vm;
    home->getVars(vm);
    Reflection::VarMapIter vmi(vm);
    int varCount = 0;
    int soCount = 0;
    for (Reflection::SpecIter si(home, vm); si(); ++si) {
      Reflection::ActorSpec& s = si.actor();
      for (; vmi(); ++vmi, ++varCount) {
        Reflection::VarSpec& vs = vmi.spec();
        os << "var ";
        if (!vs.name().empty()) {
          os << vs.name();
        } else {
          os << "_v" << varCount;
        }
        os << " = variable(\"" << vs.vti() << "\", ";
        emitArg(os, vs.dom(), vm);
        os << ");" << endl;
      }

      int soBase = soCount;
      for (int i=0; i<s.noOfArgs(); i++) {
        if (s[i]->isSharedObject())
          emitSharedObject(os, soBase++, vm, s[i]);
      }

      os << "constraint(\"" << s.name() << "\", ";

      soBase = soCount;
      for (int i=0; i<s.noOfArgs(); i++) {
        if (s[i]->isSharedObject())
          os << "_array" << soBase++;
        else
          emitArg(os, s[i], vm);
        if (i<s.noOfArgs()-1)
          os << ", ";
      }
      os << ");" << endl;
      soCount = soBase;
    }

    Reflection::VarMap vars;
    home->getVars(vars);
    os << "[";    
    varCount = 0;
    bool first = true;
    for (Reflection::VarMapIter varsI(vars); varsI(); ++varsI, varCount++) {
      if (first)
        first = false;
      else
        os << ", ";
      Support::Symbol name = varsI.spec().name();
      if (name.empty())
        os << "_v" << varCount;
      else
        os << name;
    }
    os << "];" << endl;
  }
  
}

// STATISTICS: serialization-any
