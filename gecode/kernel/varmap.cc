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

  class VarMap::VarMapObj {
  public:
    /// Map variable names to variable implementations
    Support::SymbolMap<VarBase*> nameToVar;
    /// Map variable implementations to variable names
    Support::PtrMap<VarBase,Support::Symbol> varToName;
    /// Map variable implementations to indices
    Support::PtrMap<VarBase,int> m;
    /// Map shared objects to indices
    Support::PtrMap<void,int>    sharedObjectMap;

    /// Map indices to variable implementations
    Support::DynamicArray<VarBase*>  vars;
    /// Map indices to variable specifications
    Support::DynamicArray<VarSpec*>  specs;
    /// Map indices to shared objects
    Support::DynamicArray<void*>     sharedObjects;
  };

  VarMap::VarMap(void) : vo(new VarMapObj()), n(0), so(0) {}

  VarMap::~VarMap(void) {
    for (int i=n; i--;)
      delete vo->specs[i];
    delete vo;
  }

  int
  VarMap::index(VarBase* x) const {
    int i;
    return vo->m.get(x,i) ? i : -1;
  }

  int
  VarMap::index(const Support::Symbol& n) const {
    VarBase* v;      
    return vo->nameToVar.get(n, v) ? index(v) : -1;
  }

  bool
  VarMap::nameIsKnown(const Support::Symbol& n) const {
    VarBase* v;
    return vo->nameToVar.get(n,v);
  }

  bool
  VarMap::hasName(VarBase* x) const {
    Support::Symbol s;
    return vo->varToName.get(x,s);
  }

  bool
  VarMap::hasName(int i) const {
    Support::Symbol s;
    return vo->varToName.get(vo->vars[i], s);
  }

  Support::Symbol
  VarMap::name(VarBase* x) const {
    Support::Symbol s;
    vo->varToName.get(x,s);
    return s;
  }

  Support::Symbol
  VarMap::name(int i) const {
    Support::Symbol s;
    vo->varToName.get(vo->vars[i],s);
    return s;
  }

  VarBase*
  VarMap::var(const Support::Symbol& n) const {
    VarBase* v;
    return vo->nameToVar.get(n,v) ? v : NULL;
  }

  VarBase*
  VarMap::var(int i) const {
    return vo->vars[i];
  }

  VarSpec&
  VarMap::spec(VarBase* x) const {
    int i;
    if (!vo->m.get(x,i))
      throw new ReflectionException("Variable not in VarMap");
    return *vo->specs[i];
  }

  VarSpec&
  VarMap::spec(int i) const {
    return *vo->specs[i];
  }

  VarSpec&
  VarMap::spec(const Support::Symbol& n) const {
    return spec(var(n));
  }

  void
  VarMap::name(VarBase* x, const Support::Symbol& n) {
    vo->nameToVar.put(n, x);
    vo->varToName.put(x, n);
  }

  int
  VarMap::put(VarBase* x, VarSpec* spec) {
    int newIndex = n++;
    vo->m.put(x, newIndex);
    vo->specs[newIndex] = spec;
    vo->vars[newIndex] = x;
    if (hasName(x)) {
      spec->name(name(x));
    } else if (spec->hasName()) {
      vo->nameToVar.put(spec->name(), x);
      vo->varToName.put(x, spec->name());
    }
    return newIndex;
  }
  
  void
  VarMap::putMasterObject(void* obj) {
    vo->sharedObjectMap.put(obj, so);
    vo->sharedObjects[so++] = obj;
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
    assert(i < so);
    return vo->sharedObjects[i];
  }

  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(&m0), i(0) {}

  bool
  VarMapIter::operator()(void) const { return i<m->n; }

  VarSpec&
  VarMapIter::var(void) const { return *m->vo->specs[i]; }

  void
  VarMapIter::operator++(void) { i++; }  

}}

// STATISTICS: kernel-other
