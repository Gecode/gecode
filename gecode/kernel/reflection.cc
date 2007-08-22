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
