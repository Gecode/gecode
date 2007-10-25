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

  Arg::Arg(argtype t0) : t(t0) {}
  
  bool
  Arg::isInt(void) const {
    return t == INT_ARG;
  }
  int
  Arg::toInt(void) const {
    if (!isInt())
      throw ReflectionException("not an IntArg");
    return arg1.i;
  }
  Arg*
  Arg::newInt(int i) {
    Arg* ret = new Arg(INT_ARG);
    ret->arg1.i = i;
    return ret;
  }

  bool
  Arg::isVar(void) const {
    return (t == VAR_ARG);
  }
  int
  Arg::toVar(void) const {
    if (!isVar())
      throw ReflectionException("not a VarArg");
    return arg1.i;
  }
  Arg*
  Arg::newVar(int i) {
    Arg* ret = new Arg(VAR_ARG);
    ret->arg1.i = i;
    return ret;
  }

  bool
  Arg::isArray(void) const {
    return (t == ARRAY_ARG);
  }
  ArrayArg*
  Arg::toArray(void) {
    if (!isArray())
      throw ReflectionException("not an ArrayArg");
    return static_cast<ArrayArg*>(this);
  }
  ArrayArg*
  Arg::newArray(int n) {
    Arg* ret = new Arg(ARRAY_ARG);
    ret->arg1.i = n;
    ret->arg2.aa = static_cast<Arg**>(Memory::malloc(sizeof(Arg*)*n));
    return static_cast<ArrayArg*>(ret);
  }

  bool
  Arg::isIntArray(void) const {
    return (t == INT_ARRAY_ARG);
  }
  IntArrayArg*
  Arg::toIntArray(void) {
    if (!isIntArray())
      throw ReflectionException("not an IntArrayArg");
    return static_cast<IntArrayArg*>(this);
  }
  IntArrayArg*
  Arg::newIntArray(int n) {
    Arg* ret = new Arg(INT_ARRAY_ARG);
    ret->arg1.i = n;
    ret->arg2.ia = static_cast<int*>(Memory::malloc(sizeof(int)*n));
    return static_cast<IntArrayArg*>(ret);
  }

  bool
  Arg::isString(void) const {
    return (t == STRING_ARG);
  }
  const char*
  Arg::toString(void) const {
    if (!isString())
      throw ReflectionException("not a StringArg");
    return arg1.s;
  }

#if defined(_MSC_VER)
#define strdup _strdup
#endif

  Arg*
  Arg::newString(const char* s) {
    Arg* ret = new Arg(STRING_ARG);
    ret->arg1.s = strdup(s);
    return ret;    
  }

#if defined(_MSC_VER)
#undef strdup
#endif

  bool
  Arg::isPair(void) const {
    return (t == PAIR_ARG);
  }
  Arg*
  Arg::first(void) const {
    if (!isPair())
      throw ReflectionException("not a PairArg");
    return arg1.first;
  }
  Arg*
  Arg::second(void) const {
    if (!isPair())
      throw ReflectionException("not a PairArg");
    return arg2.second;
  }
  Arg*
  Arg::newPair(Arg* a, Arg* b) {
    Arg* ret = new Arg(PAIR_ARG);
    ret->arg1.first = a;
    ret->arg2.second = b;
    return ret;
  }

  Arg::~Arg(void) {
    switch (t) {
    case ARRAY_ARG:
      for (int i=arg1.i; i--;)
        delete arg2.aa[i];
      Memory::free(arg2.aa);
      break;
    case INT_ARRAY_ARG:
      Memory::free(arg2.aa);
      break;
    case PAIR_ARG:
      delete arg1.first;
      delete arg2.second;
      break;
    case STRING_ARG:
      ::free(arg1.s);
      break;
    default:
      break;
    }
  }

  void
  Arg::dispose(Arg* a) {
    delete a;
  }

  const Arg*
  ArrayArg::operator[](int i) const {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.aa[i];
  }
  Arg*&
  ArrayArg::operator[](int i) {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.aa[i];
  }
  int
  ArrayArg::size(void) const {
    return arg1.i;
  }

  const int&
  IntArrayArg::operator[](int i) const {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.ia[i];
  }
  int&
  IntArrayArg::operator[](int i) {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.ia[i];
  }
  int
  IntArrayArg::size(void) const {
    return arg1.i;
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
    Arg::dispose(_dom);
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
    /// The queue where the actor is schedules
    int queue;
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
      Arg::dispose(a[i]);
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

  int
  ActorSpec::queue(void) const {
    return _args->queue;
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
    return (*this) << Arg::newInt(i);
  }

  ActorSpec&
  ActorSpec::operator<<(unsigned int i) {
    return (*this) << Arg::newInt(static_cast<int>(i));
  }

  ActorSpec&
  ActorSpec::operator<<(double i) {
    return (*this) << Arg::newInt(static_cast<int>(i));
  }

  void
  ActorSpec::queue(int q) {
    _args->queue = q;
  }

  /*
   * Specification iterator
   *
   */
   
  bool
  SpecIter::operator()(void) const {
    return cur != &s->a_actors;
  }

  void
  SpecIter::operator++(void) {
    delete curSpec;
    cur = cur->next();
    while (queue > 0 && cur == &s->pool[queue]) {
      queue--;
      cur = &s->pool[queue];
      cur = cur->next();
    }
    if (queue == 0 && cur == &s->pool[0]) {
      queue--;
      cur = s->a_actors.next();
    }
    if ((*this)()) {
      curSpec = &static_cast<Actor*>(cur)->spec(s,*m);
      curSpec->queue(queue);
    }
  }

  SpecIter::SpecIter(Space* s0, VarMap& m0)
  : m(&m0), s(s0), queue(s0->pool_next), curSpec(NULL) {
    if (queue >= 0)
      cur = &s->pool[queue];
    else
      cur = &s->a_actors;
    ++(*this);
  }

  ActorSpec&
  SpecIter::actor(void) const {
    return *curSpec;
  }

}}

// STATISTICS: kernel-core
