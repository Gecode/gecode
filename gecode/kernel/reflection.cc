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

  // Registry
  
  Registry registry;
  
  VarBase*
  Registry::createVar(Space* home, VarSpec& spec) {
    varCreator vc = NULL;
    if (!varCreators.get(spec.vti(),vc)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return vc(home, spec);
  }

  void
  Registry::constrainVar(Space* home, VarBase* v, VarSpec& spec) {
    varConstrainer vc = NULL;
    if (!varConstrainers.get(spec.vti(),vc)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    vc(home, v, spec);
  }

  void
  Registry::post(Space* home, const VarMap& vm, const ActorSpec& spec) {
    poster p = NULL;
      
    if (!posters.get(spec.name(),p)) {
      throw Reflection::ReflectionException("Constraint not found");
    }
    p(home, vm, spec);
  }

  void
  Registry::add(Support::Symbol vti, varCreator vc) {
    varCreators.put(vti, vc);
  }

  void
  Registry::add(Support::Symbol vti, varConstrainer vc) {
    varConstrainers.put(vti, vc);
  }

  void
  Registry::add(const Support::Symbol& id, poster p) {
    posters.put(id, p);
  }

  void
  Registry::print(std::ostream&) {
    // out << "Posters: " << std::endl;
    // for (int i=0; i<posters.size(); i++) {
    //   out << posters.key(i) << std::endl;
    // }
    // out << "Varcreators: " << std::endl;
    // for (int i=0; i<varCreators.size(); i++) {
    //   out << varCreators.key(i) << std::endl;
    // }
    
  }

  /*
   * Arguments
   *
   */
  
  bool
  Arg::isInt(void) {
    return (dynamic_cast<IntArg*>(this) != NULL);
  }
  int
  Arg::toInt(void) {
    IntArg* ia = dynamic_cast<IntArg*>(this);
    if (ia == NULL)
      throw ReflectionException("not an IntArg");
    return ia->i;
  }

  bool
  Arg::isVar(void) {
    return (dynamic_cast<VarArg*>(this) != NULL);
  }
  int
  Arg::toVar(void) {
    VarArg* va = dynamic_cast<VarArg*>(this);
    if (va == NULL)
      throw ReflectionException("not a VarArg");
    return va->i;
  }

  bool
  Arg::isArray(void) {
    return (dynamic_cast<ArrayArg*>(this) != NULL);
  }
  ArrayArg*
  Arg::toArray(void) {
    ArrayArg* aa = dynamic_cast<ArrayArg*>(this);
    if (aa == NULL)
      throw ReflectionException("not an ArrayArg");
    return aa;
  }
  bool
  Arg::isIntArray(void) {
    return (dynamic_cast<IntArrayArg*>(this) != NULL);
  }
  IntArrayArg*
  Arg::toIntArray(void) {
    IntArrayArg* aa = dynamic_cast<IntArrayArg*>(this);
    if (aa == NULL)
      throw ReflectionException("not an IntArrayArg");
    return aa;
  }

  bool
  Arg::isString(void) {
    return (dynamic_cast<StringArg*>(this) != NULL);
  }
  Support::Symbol
  Arg::toString(void) {
    StringArg* sa = dynamic_cast<StringArg*>(this);
    if (sa == NULL)
      throw ReflectionException("not a StringArg");
    return sa->s;
  }

  bool
  Arg::isPair(void) {
    return (dynamic_cast<PairArg*>(this) != NULL);
  }
  Arg*
  Arg::first(void) {
    PairArg* pa = dynamic_cast<PairArg*>(this);
    if (pa == NULL)
      throw ReflectionException("not a PairArg");
    return pa->a;
  }
  Arg*
  Arg::second(void) {
    PairArg* pa = dynamic_cast<PairArg*>(this);
    if (pa == NULL)
      throw ReflectionException("not a PairArg");
    return pa->b;
  }

  Arg::~Arg(void) {}

  IntArg::IntArg(int i0) : i(i0) {}

  const Arg*
  ArrayArg::operator[](int i) const {
    if (i >= n)
      throw ReflectionException("Array index out of range");
    return a[i];
  }
  Arg*&
  ArrayArg::operator[](int i) {
    if (i >= n)
      throw ReflectionException("Array index out of range");
    return a[i];
  }
  int
  ArrayArg::size(void) const {
    return n;
  }

  ArrayArg::ArrayArg(int n0) : n(n0) {
    a = static_cast<Arg**>(Memory::malloc(sizeof(Arg*)*n));
  }

  ArrayArg::~ArrayArg(void) {
    for (int i=n; i--;)
      delete a[i];
    Memory::free(a);
  }

  const int&
  IntArrayArg::operator[](int i) const {
    if (i >= n)
      throw ReflectionException("Array index out of range");
    return a[i];
  }
  int&
  IntArrayArg::operator[](int i) {
    if (i >= n)
      throw ReflectionException("Array index out of range");
    return a[i];
  }
  int
  IntArrayArg::size(void) const {
    return n;
  }

  IntArrayArg::IntArrayArg(int n0) : n(n0) {
    a = static_cast<int*>(Memory::malloc(sizeof(int)*n));
  }

  IntArrayArg::~IntArrayArg(void) {
    Memory::free(a);
  }

  IntArrayArgRanges::IntArrayArgRanges(Reflection::IntArrayArg* a0) 
    : a(a0), n(0) {}

  bool
  IntArrayArgRanges::operator()(void) { return n < a->size(); }

  void
  IntArrayArgRanges::operator++(void) { n += 2; }

  int
  IntArrayArgRanges::min(void) const { return (*a)[n]; }

  int
  IntArrayArgRanges::max(void) const { return (*a)[n+1]; }

  unsigned int
  IntArrayArgRanges::width(void) const { 
    return static_cast<unsigned int>(max() - min()) + 1; 
  }

  StringArg::StringArg(const Support::Symbol& s0) : s(s0) {}

  PairArg::PairArg(Arg* a0, Arg* b0) : a(a0), b(b0) {}

  PairArg::~PairArg(void) {
    delete a;
    delete b;
  }

  VarArg::VarArg(int i0) : i(i0) {}

  //
  // VarSpec
  //

  class VarSpec::Domain {
  public:
    /// The variable type identifier for this variable
    Support::Symbol _vti;
    /// The name of this variable
    Support::Symbol _n;
    /// A representation of the domain of this variable
    Arg* _dom;
    /// Reference count
    int r;
    /// Constructor
    Domain(Support::Symbol vti, Arg* domain);
    /// Destructor
    ~Domain(void);
  };

  /*
   * Variable specifications
   *
   */

  inline
  VarSpec::Domain::Domain(Support::Symbol vti, Arg* domain)
  : _vti(vti), _dom(domain), r(1) {}

  inline
  VarSpec::Domain::~Domain(void) {
    delete _dom;    
  }

  VarSpec::VarSpec(Support::Symbol vti, Arg* dom) 
  : _dom(new Domain(vti,dom)) {}

  VarSpec::VarSpec(const VarSpec& s) : _dom(s._dom) {
    _dom->r++;
  }
  
  const VarSpec&
  VarSpec::operator=(const VarSpec& s) {
    if (this != &s) {
      if (--_dom->r == 0)
        delete _dom;
      _dom = s._dom;
      _dom->r++;
    }
    return *this;
  }
  
  VarSpec::~VarSpec(void) {
    if (--_dom->r == 0)
      delete _dom;
  }

  void
  VarSpec::name(const Support::Symbol& n) {
    _dom->_n = n;
  }
  
  Support::Symbol
  VarSpec::name(void) const {
    return _dom->_n;
  }

  bool
  VarSpec::hasName(void) const {
    return !_dom->_n.empty();
  }

  Support::Symbol
  VarSpec::vti(void) const {
    return _dom->_vti;
  }

  Arg*
  VarSpec::dom(void) const {
    return _dom->_dom;
  }

  //
  // ActorSpec
  //

  class ActorSpec::Arguments {
  public:
    /// The name of this actor
    Support::Symbol _name;
    /// The size of the argument array
    int   size;
    /// The number of arguments of this actor
    int   n;
    /// The arguments of this actor
    Arg** a;
    /// Reference counter
    int r;
    /// Construct empty arguments
    Arguments(const Support::Symbol&);
    /// Destructor
    ~Arguments(void);
  };

  /*
   * Actor specifications
   *
   */

  inline
  ActorSpec::Arguments::Arguments(const Support::Symbol& name)
   :  _name(name), size(4), n(0), r(1) {
     a = static_cast<Arg**>(Memory::malloc(sizeof(Arg*)*size));
  }

  inline
  ActorSpec::Arguments::~Arguments(void) {
    for (int i=n; i--;)
      delete a[i];
    Memory::free(a);
  }

  void
  ActorSpec::resize(void) {
    _args->size = _args->size * 3 / 2;
    Arg** newargs =
      static_cast<Arg**>(Memory::malloc(sizeof(Arg*)*_args->size));
    for (int i=_args->n; i--;)
      newargs[i] = _args->a[i];
    Memory::free(_args->a);
    _args->a = newargs;
  }

  ActorSpec::ActorSpec(const Support::Symbol& name) {
    _args = new Arguments(name);
  }

  ActorSpec::ActorSpec(const ActorSpec& s) : _args(s._args) {
    _args->r++;
  }
  
  const ActorSpec&
  ActorSpec::operator=(const ActorSpec& s) {
    if (this != &s) {
      if (--_args->r == 0)
        delete _args;
      _args = s._args;
      _args->r++;
    }
    return *this;
  }

  Arg*
  ActorSpec::operator[](int i) const {
    if (i >= _args->n)
      throw ReflectionException("Array index out of range");
    return _args->a[i];
  }

  int
  ActorSpec::noOfArgs(void) const {
    return _args->n;
  }

  Support::Symbol
  ActorSpec::name(void) const {
    return _args->_name;
  }

  bool
  ActorSpec::hasName(void) const {
    return !_args->_name.empty();
  }

  ActorSpec::~ActorSpec(void) {
    if (--_args->r == 0)
      delete _args;
  }

  void
  ActorSpec::add(Arg* arg) {
    if (_args->n == _args->size)
      resize();
    _args->a[_args->n] = arg;
    _args->n++;
  }

  ActorSpec&
  ActorSpec::operator<<(Arg* arg) {
    add(arg);
    return (*this);
  }
  
  ActorSpec&
  ActorSpec::operator<<(int i) {
    return (*this) << new IntArg(i);
  }

  ActorSpec&
  ActorSpec::operator<<(unsigned int i) {
    return (*this) << new IntArg(static_cast<int>(i));
  }

  ActorSpec&
  ActorSpec::operator<<(double i) {
    return (*this) << new IntArg(static_cast<int>(i));
  }

  /*
   * Specification iterator
   *
   */
   
  bool
  SpecIter::operator()(void) const {
    return cur != &s->a_actors;
  }

  SpecIter::SpecIter(Space* s0, VarMap& m0) : m(&m0), s(s0),
    curSpec(NULL) {
    cur = s->a_actors.next();
    if ((*this)())
      curSpec = &static_cast<Actor*>(cur)->spec(s,*m);
  }

  void
  SpecIter::operator++(void) {
    delete curSpec;
    cur = cur->next();
    if ((*this)())
      curSpec = &static_cast<Actor*>(cur)->spec(s,*m);
  }

  ActorSpec&
  SpecIter::actor(void) const {
    return *curSpec;
  }

}}

// STATISTICS: kernel-core
