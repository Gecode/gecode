/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *
 *  Last modified:
 *     $Date: 2007-01-17 11:28:00 +0100 (Wed, 17 Jan 2007) $ by $Author: tack $
 *     $Revision: 4060 $
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

#include "gecode/int.hh"

namespace Gecode { namespace Serialization {
    
  Registry registry;
  
  VarBase*
  Registry::createVar(Space* home, Reflection::VarSpec& spec) {
    std::map<int, varCreator>::iterator i = varCreators.find(spec.vti());
    if (i == varCreators.end()) {
      /// TODO: throw exception
      std::cout << "VTI " << spec.vti() << " not found. Aborting." << std::endl;
      std::exit(2);
    }
    return i->second(home, spec);
  }

  void
  Registry::post(Space* home, const std::vector<VarBase*>& v,
                 Reflection::ActorSpec& spec) {
    std::map<std::string, poster>::iterator i = posters.find(std::string(spec.name()));
    if (i == posters.end()) {
      std::cout << "Constraint " << spec.name() << " not found. Aborting." << std::endl;
      /// TODO: throw exception
      std::exit(2);
    }
    i->second(home, v, spec);
  }

  void
  Registry::add(int vti, varCreator vc) {
    varCreators[vti] = vc;
  }

  void
  Registry::add(const char* id, poster p) {
    posters[std::string(id)] = p;
  }
    
  namespace {

    class ArrayArgIter {
    private:
      Reflection::ArrayArg<int>& a;
      int n;
    public:
      ArrayArgIter(Reflection::ArrayArg<int>* a0) : a(*a0), n(0) {}
      bool operator()(void) { return n < a.size(); }
      void operator++(void) { n += 2; }
      int min(void) const { return a[n]; }
      int max(void) const { return a[n+1]; }
      unsigned int width(void) const { return max() - min() + 1; }
    };
    
    VarBase* createIntVar(Space* home, Reflection::VarSpec& spec) {
      ArrayArgIter ai(spec.dom()->toArray<int>());
      IntSet dom(ai);
      IntVar v(home, dom);
      Int::IntView iv(v);
      return iv.variable();
    }
    VarBase* createBoolVar(Space* home, Reflection::VarSpec& spec) {
      return NULL;
    }
#ifdef GECODE_HAVE_SET_VARS
    VarBase* createSetVar(Space* home, Reflection::VarSpec& spec) {
      return NULL;
    }
#endif

    void distinct_val(Space* home, const std::vector<VarBase*>& v,
                      Reflection::ActorSpec& spec) {
      Reflection::Type t_intview("int.IntView");
      if (spec[0]->type()->equal(&t_intview)) {
        Reflection::ArrayArg<Reflection::Arg*>* a =
          spec[0]->typedArg()->toArray<Reflection::Arg*>();
        IntVarArgs iva(a->size());
        for (int i=a->size(); i--;) {
          iva[i] = IntVar(Int::IntView(static_cast<Int::IntVarImp*>(v[(*a)[i]->toVar()])));
        }
        distinct(home, iva, ICL_VAL);
        return;
      }

      Reflection::Type t_offsetview("int.OffsetView");
      if (spec[0]->type()->equal(&t_offsetview)) {
        Reflection::ArrayArg<Reflection::Arg*>* a = spec[0]->typedArg()->toArray<Reflection::Arg*>();
        IntVarArgs iva(a->size());
        IntArgs ia(a->size());
        for (int i=a->size(); i--;) {
          ia[i] = (*a)[i]->first()->toInt();
          iva[i] = IntVar(Int::IntView(static_cast<Int::IntVarImp*>(v[(*a)[i]->second()->toVar()])));
        }
        distinct(home, ia, iva, ICL_VAL);
        return;
      }

    }
    
    class SomeIntConstraints {
    public:
        SomeIntConstraints() {
        registry.add(VTI_INT, createIntVar);
        registry.add(VTI_BOOL, createBoolVar);
#ifdef GECODE_HAVE_SET_VARS
        registry.add(VTI_SET, createSetVar);
#endif
        registry.add("int.distinct.Val", distinct_val);
      }
    };
    SomeIntConstraints __someIntConstraints;
  }
    
}}

// STATISTICS: serialization-any
