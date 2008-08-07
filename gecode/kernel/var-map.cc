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

namespace Gecode { namespace Reflection {

  /*
   * Variable maps
   *
   */

  /// Implementation of a VarMap
  class VarMap::VarMapObj {
  public:
    VarMapObj(void);
    /// Map variable names to variable implementations
    Support::SymbolMap<VarImpBase*> nameToVar;
    /// Map variable implementations to variable names
    Support::PtrMap<VarImpBase,Support::Symbol> varToName;
    /// Map variable implementations to indices
    Support::PtrMap<VarImpBase,int> m;
    /// Map shared objects to indices
    Support::PtrMap<void,int>    sharedObjectMap;

    /// Map indices to variable implementations
    Support::DynamicArray<VarImpBase*,Heap>  vars;
    /// Map indices to variable specifications
    Support::DynamicArray<VarSpec*,Heap>  specs;
    /// Map indices to shared objects
    Support::DynamicArray<void*,Heap>     sharedObjects;

    /// Number of variable indices in use
    int n;
    /// Number of shared object indices in use
    int so;

    /// Reference count
    int r;
  };

  VarMap::VarMapObj::VarMapObj(void) 
    : vars(heap), specs(heap), sharedObjects(heap),
      n(0), so(0), r(1) {}

  VarMap::VarMap(void) : vo(new VarMapObj()) {}

  VarMap::VarMap(const VarMap& v) : vo(v.vo) { vo->r++; }

  VarMap&
  VarMap::operator=(const VarMap& v) {
    if (this != &v) {
      if (--vo->r == 0) {
        for (int i=vo->n; i--;)
          delete vo->specs[i];
        delete vo;        
      }
      vo = v.vo;
      vo->r++;
    }
    return *this;
  }

  VarMap::~VarMap(void) {
    if (--vo->r == 0) {
      for (int i=vo->n; i--;)
        delete vo->specs[i];
      delete vo;
    }
  }

  int
  VarMap::size(void) const {
    return vo->n;
  }
  
  int
  VarMap::index(const VarImpBase* cx) const {
    int i;
    VarImpBase* x = const_cast<VarImpBase*>(cx);
    return vo->m.get(x,i) ? i : -1;
  }

  int
  VarMap::index(const Support::Symbol& n) const {
    VarImpBase* v;      
    return vo->nameToVar.get(n, v) ? index(v) : -1;
  }

  bool
  VarMap::nameIsKnown(const Support::Symbol& n) const {
    VarImpBase* v;
    return vo->nameToVar.get(n,v);
  }

  bool
  VarMap::hasName(const VarImpBase* cx) const {
    Support::Symbol s;
    VarImpBase* x = const_cast<VarImpBase*>(cx);
    return vo->varToName.get(x,s);
  }

  bool
  VarMap::hasName(int i) const {
    Support::Symbol s;
    return vo->varToName.get(vo->vars[i], s);
  }

  Support::Symbol
  VarMap::name(const VarImpBase* cx) const {
    Support::Symbol s;
    VarImpBase* x = const_cast<VarImpBase*>(cx);
    vo->varToName.get(x,s);
    return s;
  }

  Support::Symbol
  VarMap::name(int i) const {
    Support::Symbol s;
    vo->varToName.get(vo->vars[i],s);
    return s;
  }

  VarImpBase*
  VarMap::varImpBase(const Support::Symbol& n) const {
    VarImpBase* v;
    return vo->nameToVar.get(n,v) ? v : NULL;
  }

  VarImpBase*
  VarMap::varImpBase(int i) const {
    if (i<0 || i>=vo->n)
      throw ReflectionException("Variable not in VarMap");
    return vo->vars[i];
  }

  VarSpec&
  VarMap::spec(const VarImpBase* cx) const {
    int i;
    VarImpBase* x = const_cast<VarImpBase*>(cx);
    if (!vo->m.get(x,i))
      throw ReflectionException("Variable not in VarMap");
    return *vo->specs[i];
  }

  VarSpec&
  VarMap::spec(int i) const {
    if (i<0 || i>=vo->n)
      throw ReflectionException("Variable not in VarMap");
    return *vo->specs[i];
  }

  VarSpec&
  VarMap::spec(const Support::Symbol& n) const {
    return spec(varImpBase(n));
  }

  void
  VarMap::name(VarImpBase* x, const Support::Symbol& n) {
    VarImpBase* y;
    if (vo->nameToVar.get(n, y) && x != y)
      throw
        ReflectionException("Variable with the same name already in VarMap");
    vo->nameToVar.put(n, x);
    vo->varToName.put(x, n);
  }

  int
  VarMap::put(const VarImpBase* cx, VarSpec* spec) {
    VarImpBase* x = const_cast<VarImpBase*>(cx);
    int newIndex = vo->n++;
    vo->m.put(x, newIndex);
    vo->specs[newIndex] = spec;
    vo->vars[newIndex] = x;
    if (hasName(x)) {
      spec->name(name(x));
    } else if (spec != NULL && spec->hasName()) {
      vo->nameToVar.put(spec->name(), x);
      vo->varToName.put(x, spec->name());
    }
    return newIndex;
  }
  
  void
  VarMap::putMasterObject(void* obj) {
    vo->sharedObjectMap.put(obj, vo->so);
    vo->sharedObjects[vo->so++] = obj;
  }

  int
  VarMap::getSharedIndex(void* obj) const {
    int idx;
    if (vo->sharedObjectMap.get(obj, idx))
      return idx;
    return -1;
  }
  
  void*
  VarMap::getSharedObject(int i) const {
    assert(i < vo->so);
    return vo->sharedObjects[i];
  }

  Var
  VarMap::var(const Support::Symbol& n) const {
    return Var(varImpBase(n), spec(n).vti());
  }

  Var
  VarMap::var(int i) const {
    return Var(varImpBase(i), spec(i).vti());
  }

  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(&m0), i(0) {}

  bool
  VarMapIter::operator()(void) const { return i<m->vo->n; }

  VarSpec&
  VarMapIter::spec(void) const { return *m->vo->specs[i]; }

  VarImpBase*
  VarMapIter::varImpBase(void) const { return m->vo->vars[i]; }

  void
  VarMapIter::operator++(void) { i++; }  

  Var
  VarMapIter::var(void) const {
    return Var(varImpBase(), spec().vti());
  }

}}

// STATISTICS: kernel-other
