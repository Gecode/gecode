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
#include "gecode/support/dynamic-array.hh"

namespace Gecode { namespace Reflection {
  
  /* Variable map */

  /// Implementation of variable maps
  class VarMap::VarMapImp {
  public:
    /// Map variable names to variable implementations
    Support::StringMap<VarBase*> nameToVar;
    /// Map variable implementations to variable names
    Support::PtrMap<VarBase,Support::String> varToName;
    /// Map variable implementations to indices
    Support::PtrMap<VarBase,int> m;
    /// Number of indices in use
    int n;
    /// Map indices to variable implementations
    Support::DynamicArray<VarBase*>  var;
    /// Map indices to variable specifications
    Support::DynamicArray<VarSpec*>  spec;
    
    /// Default constructor
    VarMapImp(void) : n(0) {}
    /// Destructor, deleting all specifications stored in \a v
    ~VarMapImp(void);
  };
  
  VarMap::VarMapImp::~VarMapImp(void) {
    for (int i=n; i--;)
      delete spec[i];
  }
  
  VarMap::VarMap(void) {
    m = new VarMapImp();
  }

  VarMap::~VarMap(void) {
    delete m;
  }

  int
  VarMap::index(VarBase* x) const {
    int i;
    if (!m->m.get(x,i))
      return -1;
    return i;
  }

  int
  VarMap::index(const Support::String& n) const {
    VarBase* v;      
    if (!m->nameToVar.get(n, v))
      return -1;
    return index(v);
  }

  bool
  VarMap::nameIsKnown(const Support::String& n) const {
    VarBase* v;
    return m->nameToVar.get(n,v);
  }

  bool
  VarMap::hasName(VarBase* x) const {
    Support::String s;
    return m->varToName.get(x,s);
  }

  bool
  VarMap::hasName(int i) const {
    Support::String s;
    return m->varToName.get(m->var[i], s);
  }

  Support::String
  VarMap::name(VarBase* x) const {
    Support::String s;
    m->varToName.get(x,s);
    return s;
  }

  Support::String
  VarMap::name(int i) const {
    Support::String s;
    m->varToName.get(m->var[i],s);
    return s;
  }

  VarBase*
  VarMap::var(const Support::String& n) const {
    VarBase* v;
    if (!m->nameToVar.get(n,v))
      return NULL;
    return v;
  }

  VarBase*
  VarMap::var(int i) const {
    return m->var[i];
  }

  VarSpec&
  VarMap::spec(VarBase* x) const {
    int i;
    if (!m->m.get(x,i))
      throw new ReflectionException("Variable not in VarMap");
    return *m->spec[i];
  }

  VarSpec&
  VarMap::spec(int i) const {
    return *m->spec[i];
  }

  VarSpec&
  VarMap::spec(const Support::String& n) const {
    return spec(var(n));
  }

  void
  VarMap::name(VarBase* x, const Support::String& n) {
    m->nameToVar.put(n, x);
    m->varToName.put(x, n);
  }

  int
  VarMap::put(VarBase* x, VarSpec* spec) {
    int newIndex = m->n++;
    m->m.put(x, newIndex);
    m->spec[newIndex] = spec;
    m->var[newIndex] = x;
    if (hasName(x)) {
      spec->name(name(x));
    } else if (spec->hasName()) {
      m->nameToVar.put(spec->name(), x);
      m->varToName.put(x, spec->name());
    }
    return newIndex;
  }

  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(&m0), i(0) {}

  bool
  VarMapIter::operator()(void) const {
    return i<m->m->n;
  }

  VarSpec&
  VarMapIter::var(void) const {
    return *m->m->spec[i];
  }

  void
  VarMapIter::operator++(void) {
    i++;
  }  

  bool
  Type::equal(Type* t0) {
    if (n != t0->n)
      return false;
    if (n == 0)
      return _name == t0->_name;
    for (int i=n; i--;)
      if (!t[i]->equal(t0->t[i]))
        return false;
    return true;
  }

  void Type::print(std::ostream& os) {
    if (n == 0) {
      os << _name;
    } else {
      os << "<";
      for (int i=0; i<n; i++) {
        if (t[i])
          t[i]->print(os);
        else
          os << "NULL";
        if (i<n-1)
          os << ",";
      }
      os << ">";
    }
  }

  Support::String
  Type::toString(void) {
    if (n == 0)
      return _name;
    Support::String ret = "<";
    for (int i=0; i<n; i++) {
      if (t[i])
        ret = ret + t[i]->toString();
      else
        ret = ret +"NULL";
      if (i<n-1)
        ret = ret + ",";
    }
    return ret+">";
  }

  // Registry
  
  Registry registry;
  
  VarBase*
  Registry::createVar(Space* home, VarSpec& spec) {
    varCreator vc;
    if (!varCreators.get(spec.vti(),vc)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return vc(home, spec);
  }

  void
  Registry::post(Space* home, const VarMap& vm, const ActorSpec& spec) {
    poster p;
      
    if (!posters.get(spec.name(),p)) {
      throw Reflection::ReflectionException("Constraint not found");
    }
    p(home, vm, spec);
  }

  void
  Registry::add(int vti, varCreator vc) {
    varCreators.put(vti, vc);
  }

  void
  Registry::add(const Support::String& id, poster p) {
    posters.put(id, p);
  }

  void
  Registry::print(std::ostream& out) {
    out << "Posters: " << std::endl;
    for (int i=0; i<posters.size(); i++) {
      out << posters.key(i) << std::endl;
    }
    out << "Varcreators: " << std::endl;
    for (int i=0; i<varCreators.size(); i++) {
      out << varCreators.key(i) << std::endl;
    }
    
  }

  Arg::~Arg(void) {}

  ArrayArg::~ArrayArg(void) {
    for (int i=n; i--;)
      delete a[i];
    Memory::free(a);
  }

  IntArrayArg::~IntArrayArg(void) {
    Memory::free(a);
  }

  PairArg::~PairArg(void) {
    delete a;
    delete b;
  }

  TypedArg::~TypedArg(void) {
    delete t;
    delete a;
  }

}}

// STATISTICS: kernel-core
