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

#include <map>
#include <vector>
#include <sstream>

namespace Gecode { namespace Reflection {
  
  /* Variable map */

  /// Implementation of variable maps
  class VarMap::VarMapImp {
  public:
    /// Map variable names to variable implementations
    std::map<std::string,VarBase*> nameToVar;
    /// Map variable implementations to variable names
    std::map<VarBase*,std::string> varToName;
    /// Map variable implementations to indices
    std::map<VarBase*,int> m;
    /// Map indices to variable implementations
    std::vector<VarBase*>  var;
    /// Map indices to variable specifications
    std::vector<VarSpec*>  spec;
        
    /// Destructor, deleting all specifications stored in \a v
    ~VarMapImp(void);
  };
  
  VarMap::VarMapImp::~VarMapImp(void) {
    for (int i=spec.size(); i--;)
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
    std::map<VarBase*,int>::const_iterator i = m->m.find(x);
    if (i==m->m.end())
      return -1;
    return i->second;
  }

  int
  VarMap::index(const std::string& n) const {
    std::map<std::string,VarBase*>::const_iterator i =
      m->nameToVar.find(n);
    if (i == m->nameToVar.end())
      return -1;
    return index(i->second);
  }

  bool
  VarMap::nameIsKnown(const std::string& n) const {
    std::map<std::string,VarBase*>::const_iterator i =
      m->nameToVar.find(n);
    return i != m->nameToVar.end();
  }

  bool
  VarMap::hasName(VarBase* x) const {
    std::map<VarBase*,std::string>::const_iterator i =
      m->varToName.find(x);
    return i != m->varToName.end();    
  }

  bool
  VarMap::hasName(int i) const {
    std::map<VarBase*,std::string>::const_iterator it =
      m->varToName.find(m->var[i]);
    return it != m->varToName.end();
  }

  std::string
  VarMap::name(VarBase* x) const {
    std::map<VarBase*,std::string>::const_iterator i =
      m->varToName.find(x);
    return i->second;
  }

  std::string
  VarMap::name(int i) const {
    std::map<VarBase*,std::string>::const_iterator it =
      m->varToName.find(m->var[i]);
    return it->second;
  }

  VarBase*
  VarMap::var(const std::string& n) const {
    std::map<std::string,VarBase*>::const_iterator i =
      m->nameToVar.find(n);
    if (i == m->nameToVar.end())
      return NULL;
    return i->second;
  }

  VarBase*
  VarMap::var(int i) const {
    return m->var[i];
  }

  VarSpec&
  VarMap::spec(VarBase* x) const {
    std::map<VarBase*,int>::const_iterator i = m->m.find(x);
    if (i==m->m.end())
      throw new ReflectionException("Variable not in VarMap");
    return *m->spec[i->second];
  }

  VarSpec&
  VarMap::spec(int i) const {
    return *m->spec[i];
  }

  VarSpec&
  VarMap::spec(const std::string& n) const {
    return spec(var(n));
  }

  void
  VarMap::name(VarBase* x, const std::string& n) {
    m->nameToVar[n] = x;
    m->varToName[x] = n;
  }

  int
  VarMap::put(VarBase* x, VarSpec* spec) {
    assert(m->m.find(x)==m->m.end());
    m->m[x] = m->spec.size();
    m->spec.push_back(spec);
    m->var.push_back(x);
    if (hasName(x)) {
      spec->name(name(x).c_str());
    } else if (spec->name() != NULL) {
      m->nameToVar[std::string(spec->name())] = x;
      m->varToName[x] = std::string(spec->name());
    }
    return m->spec.size() - 1;
  }

  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(m0), i(0) {}

  bool
  VarMapIter::operator()(void) const {
    return i<m.m->spec.size();
  }

  VarSpec&
  VarMapIter::var(void) const {
    return *m.m->spec[i];
  }

  void
  VarMapIter::operator++(void) {
    i++;
  }  

  bool
  Type::equal(Type* t) {
    if (n != t->n)
      return false;
    if (n == 0)
      return !strcmp(u.n, t->u.n);
    for (int i=n; i--;)
      if (!u.t[i]->equal(t->u.t[i]))
        return false;
    return true;
  }

  void Type::print(std::ostream& os) {
    if (n == 0) {
      os << u.n;
    } else {
      os << "<";
      for (int i=0; i<n; i++) {
        if (u.t[i])
          u.t[i]->print(os);
        else
          os << "NULL";
        if (i<n-1)
          os << ",";
      }
      os << ">";
    }
  }

  std::string
  Type::toString(void) {
    std::ostringstream s;
    print(s);
    return s.str();
  }

  // Registry
  
  Registry registry;
  
  VarBase*
  Registry::createVar(Space* home, VarSpec& spec) {
    std::map<int, varCreator>::iterator i = varCreators.find(spec.vti());
    if (i == varCreators.end()) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return i->second(home, spec);
  }

  void
  Registry::post(Space* home, const VarMap& vm, const ActorSpec& spec) {
    std::map<std::string, poster>::iterator i = 
      posters.find(std::string(spec.name()));
    if (i == posters.end()) {
      throw Reflection::ReflectionException("Constraint not found");
    }
    i->second(home, vm, spec);
  }

  void
  Registry::add(int vti, varCreator vc) {
    varCreators[vti] = vc;
  }

  void
  Registry::add(const std::string& id, poster p) {
    posters[id] = p;
  }

  void
  Registry::print(std::ostream& out) {
    out << "Posters: " << std::endl;
    std::map<std::string, poster>::iterator i = posters.begin();
    for (; i != posters.end(); ++i) {
      out << i->first << std::endl;
    }
  }

}}

// STATISTICS: kernel-core
