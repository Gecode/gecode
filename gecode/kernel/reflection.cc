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
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "gecode/kernel.hh"

#include <map>
#include <vector>

namespace Gecode { namespace Reflection {
  
  /* Variable map */

  /// Implementation of variable maps
  class VarMap::VarMapImp {
  public:
    /// Map variable names to variable implementations
    std::map<std::string,VarBase*> nameToVar;
    /// Map variable implementations to indices in the VarSpec vector
    std::map<VarBase*,int> m;
    /// Vector of variable specifications
    std::vector<VarSpec*>  v;
    /// Destructor, deleting all specifications stored in \a v
    ~VarMapImp(void);
  };
  
  VarMap::VarMapImp::~VarMapImp(void) {
    for (int i=v.size(); i--;)
      delete v[i];
  }
  
  VarMap::VarMap(void) {
    m = new VarMapImp();
  }

  VarMap::~VarMap(void) {
    delete m;
  }

  VarSpec&
  VarMap::get(VarBase* x) {
    std::map<VarBase*,int>::const_iterator i = m->m.find(x);
    if (i==m->m.end())
      throw new ReflectionException("Variable not in VarMap");
    return *m->v[i->second];
  }

  void
  VarMap::name(VarBase* x, const char* n) {
    VarSpec& vs = get(x);
    vs.name(n);
    m->nameToVar[std::string(n)] = x;
  }

  VarSpec&
  VarMap::get(int i) {
    return *m->v[i];
  }

  int
  VarMap::getIndex(VarBase* x) {
    std::map<VarBase*,int>::const_iterator i = m->m.find(x);
    if (i==m->m.end())
      return -1;
    return i->second;
  }

  int
  VarMap::put(VarBase* x, VarSpec* spec) {
    assert(m->m.find(x)==m->m.end());
    m->m[x] = m->v.size();
    m->v.push_back(spec);
    if (spec->name() != NULL)
      m->nameToVar[std::string(spec->name())] = x;
    return m->v.size() - 1;
  }

  VarBase*
  VarMap::variableByName(const char* n) {
    std::map<std::string,VarBase*>::const_iterator i =
      m->nameToVar.find(std::string(n));
    if (i == m->nameToVar.end())
      return NULL;
    return i->second;
  }
  
  /* Variable map iterator */

  VarMapIter::VarMapIter(VarMap& m0) : m(m0), i(0) {}

  bool
  VarMapIter::operator()(void) const {
    return i<m.m->v.size();
  }

  VarSpec&
  VarMapIter::var(void) const {
    return *m.m->v[i];
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

}}

// STATISTICS: kernel-core
