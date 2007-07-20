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

    class VariableCreators {
    public:
        VariableCreators() {
        registry.add(VTI_INT, createIntVar);
        registry.add(VTI_BOOL, createBoolVar);
#ifdef GECODE_HAVE_SET_VARS
        registry.add(VTI_SET, createSetVar);
#endif
      }
    };
    VariableCreators __variableCreators;
  }
    
}}

namespace Gecode { namespace Serialization {
  // Additional constraint posting functions
  namespace {
    void
    my_count(Space* home, const IntVarArgs& x, IntVar y,
          IntRelType r, IntVar z, int c) {
      if (c == 0) {
        count(home, x, y, r, z);
      } else if (c == 1) {
        assert(r == IRT_GQ);
        count(home, x, y, IRT_GR, z);
      } else if (c == -1) {
        assert(r == IRT_LQ);
        count(home, x, y, IRT_LE, z);        
      }
    }
    void
    my_count(Space* home, const IntVarArgs& x, int y,
          IntRelType r, IntVar z, int c) {
      if (c == 0) {
        count(home, x, y, r, z);
      } else if (c == 1) {
        assert(r == IRT_GQ);
        count(home, x, y, IRT_GR, z);
      } else if (c == -1) {
        assert(r == IRT_LQ);
        count(home, x, y, IRT_LE, z);        
      }
    }
    
    void
    my_count(Space* home, const IntVarArgs& x, const IntArgs& y,
          IntRelType r, IntVar z, int c) {
      if (c == 0) {
        count(home, x, y, r, z);
      } else if (c == 1) {
        assert(r == IRT_GQ);
        count(home, x, y, IRT_GR, z);
      } else if (c == -1) {
        assert(r == IRT_LQ);
        count(home, x, y, IRT_LE, z);        
      }
    }
    
    void my_distinct(Space* home, int o0, int o1, int o2,
                     IntVar x0, IntVar x1, IntVar x2, IntConLevel icl) {
       IntArgs ia(3); 
       IntVarArgs iva(3);
       ia[0] = o0; ia[1] = o1; ia[2] = o2;
       iva[0] = x0; iva[1] = x1; iva[2] = x2;
       distinct(home, ia, iva, icl);
    }
    void my_distinct(Space* home, int o0, int o1,
                     IntVar x0, IntVar x1, IntConLevel icl) {
       IntArgs ia(2); 
       IntVarArgs iva(2);
       ia[0] = o0; ia[1] = o1;
       iva[0] = x0; iva[1] = x1;
       distinct(home, ia, iva, icl);
    }
    
    class IntArrayIter {
    private:
      Reflection::ArrayArg<int>* a;
      int n;
    public:
      IntArrayIter(Reflection::ArrayArg<int>* a0) : a(a0), n(0) {}
      bool operator()(void) { return n+1 < a->size(); }
      void operator++(void) { n+=2; }
      int min(void) const { return (*a)[n]; }
      int max(void) const { return (*a)[n+1]; }
      unsigned int width(void) const { return max() - min() + 1; }
    };
    
  }
}}

#include "gecode/serialization/registry_generated.icc"

// STATISTICS: serialization-any
