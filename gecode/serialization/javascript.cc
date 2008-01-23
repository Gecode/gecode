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

#include "gecode/kernel.hh"
#include "gecode/serialization.hh"

#ifdef GECODE_HAVE_QT
#include "gecode/serialization/javascript.hh"
#include "gecode/serialization.hh"
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
    home->getVars(vm, false);
    int rootSize = vm.size();
    Reflection::VarMapIter vmi(vm);
    int varCount = 0;
    int soCount = 0;
    for (Reflection::ActorSpecIter si(home, vm); si(); ++si) {
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
        if (!vs.name().empty()) {
          os << "\"" << vs.name() << "\", ";
        }
        emitArg(os, vs.dom(), vm);
        os << ");" << endl;
      }

      int soBase = soCount;
      for (int i=0; i<s.noOfArgs(); i++) {
        if (s[i] && s[i]->isSharedObject())
          emitSharedObject(os, soBase++, vm, s[i]);
      }

      os << "constraint(\"" << s.ati() << "\", ";

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

    os << "[";    
    bool first = true;
    for (int i=0; i<rootSize; i++) {
      if (first)
        first = false;
      else
        os << ", ";
      Support::Symbol name = vm.spec(i).name();
      if (name.empty())
        os << "_v" << i;
      else
        os << name;
    }
    os << "];" << endl;
  }

#ifdef GECODE_HAVE_QT

  namespace Serialization {
    Gecode::Reflection::Arg*
    GJSSpace::scriptValToArg(QScriptValue v) {
      if (v.isArray()) {
        bool isIntArray = true;
        int size = 0;
        QScriptValueIterator vi(v);
        while (vi.hasNext()) {
          vi.next();
          if (!vi.value().isNumber())
            isIntArray = false;
          size++;
        }
        if (isIntArray) {
          Gecode::Reflection::IntArrayArg* a =
            Gecode::Reflection::Arg::newIntArray(size);
          int count = 0;
          QScriptValueIterator vi(v);
          while (vi.hasNext()) {
            vi.next();
            (*a)[count++] = vi.value().toNumber();
          }
          return a;
        } else {
          Gecode::Reflection::ArrayArg* a =
            Gecode::Reflection::Arg::newArray(size);
          int count = 0;
          QScriptValueIterator vi(v);
          while (vi.hasNext()) {
            vi.next();
            Gecode::Reflection::Arg* ai = scriptValToArg(vi.value());
            (*a)[count++] = ai;
          }
          return a;      
        }
      } else if (v.isNumber()) {
        return Gecode::Reflection::Arg::newInt(v.toNumber());
      } else if (v.isBoolean()) {
        return Gecode::Reflection::Arg::newInt(v.toBoolean());    
      } else if (v.isObject() && v.prototype().strictlyEquals(varProto)) {
        return Gecode::Reflection::Arg::newVar(v.property("no").toNumber());    
      } else if (v.isObject() && v.prototype().strictlyEquals(pairProto)) {
        Gecode::Reflection::Arg* a = scriptValToArg(v.property("a"));
        Gecode::Reflection::Arg* b = scriptValToArg(v.property("b"));
        return Gecode::Reflection::Arg::newPair(a,b);
      } else {
        return NULL;
      }
    }

    GJSSpace::GJSSpace(QScriptEngine* engine, Gecode::Space* s0)
    : varProto(engine->newObject()), pairProto(engine->newObject()),
      s(s0), d(s, vm) {
      s->getVars(vm, true);
    }

    QScriptValue
    GJSSpace::variable(const QString& vti, QScriptValue args) {
      assert(args.isArray());
      Support::Symbol vtiSymbol(vti.toStdString().c_str(), true);
      int size = args.property("length").toNumber();
      if (size < 1 || size > 2) {
        throw Exception("Serialization", "Argument mismatch");
      }
      int newVar = vm.size();
      Gecode::Reflection::Arg* dom =
        scriptValToArg(args.property(size == 1 ? "0" : "1"));
      Reflection::VarSpec vs(vtiSymbol, dom);
      if (size == 2) {
        Support::Symbol nameSymbol(
          args.property("0").toString().toStdString().c_str(), true);
          vs.name(nameSymbol);
      }
      d.var(vs);
      QScriptValue object = engine()->newObject();
      object.setPrototype(varProto);
      object.setProperty("no", QScriptValue(engine(), newVar));
      return object;  
    }

    void
    GJSSpace::constraint(const QString& name, QScriptValue args) {
      Gecode::Support::Symbol nameSymbol(name.toStdString().c_str(), true);
      Gecode::Reflection::ActorSpec as(nameSymbol);
      assert(args.isArray());
      QScriptValueIterator argsI(args);
      while (argsI.hasNext()) {
        argsI.next();
        as << scriptValToArg(argsI.value());
      }
      d.post(as);
    }

    QScriptValue
    GJSSpace::pair(QScriptValue a, QScriptValue b) {
      QScriptValue object = engine()->newObject();
      object.setPrototype(pairProto);
      object.setProperty("a", a);
      object.setProperty("b", b);
      return object;
    }
   
  }
  
  void fromJavaScript(Space* space, const std::string& model) {
    QScriptEngine engine;
    Serialization::GJSSpace gjsspace(&engine, space);
    QScriptValue spaceValue = engine.newQObject(&gjsspace);
    engine.globalObject().setProperty("Space", spaceValue);

    QString prelude = 
    "function constraint() {"
    "  var name = arguments[0];"
    "  var args = new Array;"
    "  for (var i=1; i<arguments.length; i++) {"
    "    args[i-1] = arguments[i];"
    "  }"
    "  Space.constraint(name, args);"
    "}"
    "function variable() {" 
    "  var vti = arguments[0];"
    "  var args = new Array;"
    "  for (var i=1; i<arguments.length; i++) {"
    "    args[i-1] = arguments[i];"
    "  }"
    "  return Space.variable(vti, args);"
    "}"
    "function pair(a, b) { return Space.pair(a,b); }\n";

    QString program = prelude + QString(model.c_str());
    QScriptValue ret = engine.evaluate(program,"",0);
    if (engine.hasUncaughtException()) {
      throw Exception("Serialization", "Error in JavaScript execution");
      // std::cerr << "Error in script execution: "
      //           << ret.toString().toStdString() << " at line "
      //           << engine.uncaughtExceptionLineNumber() << std::endl;
    }
  }

#endif  
}

// STATISTICS: serialization-any
