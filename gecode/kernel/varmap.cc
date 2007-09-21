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

  VarMap::VarMap(void) : n(0) {}

  VarMap::~VarMap(void) {
    for (int i=n; i--;)
      delete specs[i];
  }

  int
  VarMap::index(VarBase* x) const {
    int i;
    return m.get(x,i) ? i : -1;
  }

  int
  VarMap::index(const Support::Symbol& n) const {
    VarBase* v;      
    return nameToVar.get(n, v) ? index(v) : -1;
  }

  bool
  VarMap::nameIsKnown(const Support::Symbol& n) const {
    VarBase* v;
    return nameToVar.get(n,v);
  }

  bool
  VarMap::hasName(VarBase* x) const {
    Support::Symbol s;
    return varToName.get(x,s);
  }

  bool
  VarMap::hasName(int i) const {
    Support::Symbol s;
    return varToName.get(vars[i], s);
  }

  Support::Symbol
  VarMap::name(VarBase* x) const {
    Support::Symbol s;
    varToName.get(x,s);
    return s;
  }

  Support::Symbol
  VarMap::name(int i) const {
    Support::Symbol s;
    varToName.get(vars[i],s);
    return s;
  }

  VarBase*
  VarMap::var(const Support::Symbol& n) const {
    VarBase* v;
    return nameToVar.get(n,v) ? v : NULL;
  }

  VarBase*
  VarMap::var(int i) const {
    return vars[i];
  }

  VarSpec&
  VarMap::spec(VarBase* x) const {
    int i;
    if (!m.get(x,i))
      throw new ReflectionException("Variable not in VarMap");
    return *specs[i];
  }

  VarSpec&
  VarMap::spec(int i) const {
    return *specs[i];
  }

  VarSpec&
  VarMap::spec(const Support::Symbol& n) const {
    return spec(var(n));
  }

  void
  VarMap::name(VarBase* x, const Support::Symbol& n) {
    nameToVar.put(n, x);
    varToName.put(x, n);
  }

  int
  VarMap::put(VarBase* x, VarSpec* spec) {
    int newIndex = n++;
    m.put(x, newIndex);
    specs[newIndex] = spec;
    vars[newIndex] = x;
    if (hasName(x)) {
      spec->name(name(x));
    } else if (spec->hasName()) {
      nameToVar.put(spec->name(), x);
      varToName.put(x, spec->name());
    }
    return newIndex;
  }
  
  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(&m0), i(0) {}

  bool
  VarMapIter::operator()(void) const { return i<m->n; }

  VarSpec&
  VarMapIter::var(void) const { return *m->specs[i]; }

  void
  VarMapIter::operator++(void) { i++; }  
  
}}

// STATISTICS: kernel-core
