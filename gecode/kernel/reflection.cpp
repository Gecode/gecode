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

#include <gecode/kernel.hh>

namespace Gecode { namespace Reflection {

  // Registry

  /// The actual registry, holding SymbolMap objects
  class Registry::RegistryObject {
  public:
    /// The registry of constraint posting functions
    Support::SymbolMap<poster> posters;
    /// The registry of variable creation functions
    Support::SymbolMap<varCreator> varCreators;
    /// The registry of variable constraining functions
    Support::SymbolMap<varConstrainer> varConstrainers;
    /// The registry of variable update functions
    Support::SymbolMap<varUpdater> varUpdaters;
    /// The registry of variable print functions
    Support::SymbolMap<varPrinter> varPrinters;
    /// The registry of variable reflection functions
    Support::SymbolMap<varSpec> varSpecs;
  };
  
  Registry::Registry(void) : ro(new RegistryObject()) {}

  Registry::~Registry(void) { delete ro; }

  Registry& registry(void) {
    static Registry r;
    return r;
  }
  
  VarImpBase*
  Registry::createVar(Space& home, VarSpec& spec) const {
    varCreator vc = NULL;
    if (!ro->varCreators.get(spec.vti(),vc)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return vc(home, spec);
  }

  void
  Registry::constrainVar(Space& home, VarImpBase* v, VarSpec& spec) const {
    varConstrainer vc = NULL;
    if (!ro->varConstrainers.get(spec.vti(),vc)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    vc(home, v, spec);
  }

  VarImpBase*
  Registry::updateVariable(Space& home, bool share, VarImpBase* v,
                           const Support::Symbol& vti) const {
    varUpdater vu = NULL;
    if (!ro->varUpdaters.get(vti,vu)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return vu(home, share, v);
  }

  std::ostream&
  Registry::printVariable(std::ostream& os, VarImpBase* v,
                          const Support::Symbol& vti) const {
    varPrinter vp = NULL;
    if (!ro->varPrinters.get(vti,vp)) {
      throw Reflection::ReflectionException("VTI not found");
    }
    return vp(os, v);
  }

  Arg*
  Registry::spec(const Space& home, VarMap& vm,
                 VarImpBase* v, const Support::Symbol& vti) const {
    varSpec vs = NULL;
    if (!ro->varSpecs.get(vti,vs)) {
      throw Reflection::ReflectionException("VTI not found");      
    }
    return vs(home, vm, v);
  }

  void
  Registry::post(Space& home, VarMap& vm, const ActorSpec& spec) const {
    poster p = NULL;
    if (!ro->posters.get(spec.ati(),p)) {
      throw Reflection::ReflectionException("Constraint not found");
    }
    p(home, vm, spec);
  }

  void
  Registry::add(Support::Symbol vti, varCreator vc) {
    ro->varCreators.put(vti, vc);
  }

  void
  Registry::add(Support::Symbol vti, varConstrainer vc) {
    ro->varConstrainers.put(vti, vc);
  }

  void
  Registry::add(Support::Symbol vti, varUpdater vu) {
    ro->varUpdaters.put(vti, vu);
  }

  void
  Registry::add(Support::Symbol vti, varPrinter vp) {
    ro->varPrinters.put(vti, vp);
  }

  void
  Registry::add(Support::Symbol vti, varSpec vs) {
    ro->varSpecs.put(vti, vs);
  }

  void
  Registry::add(const Support::Symbol& id, poster p) {
    ro->posters.put(id, p);
  }

  void
  Registry::print(std::ostream&) {
  }

  /*
   * Arguments
   *
   */

  Arg::Arg(argtype t0) : t(t0) {}

#define ARGACCESSORS(N,T,F,A) \
  bool \
  Arg::is##N(void) const { \
    return t == A; \
  } \
  T \
  Arg::to##N(void) const { \
    if (!is##N()) \
      throw ReflectionException("not an A arg"); \
    return static_cast<T>(arg1.F); \
  } \
  Arg* \
  Arg::new##N(T i) { \
    Arg* ret = new Arg(A); \
    ret->arg1.F = i; \
    return ret; \
  } \
  void \
  Arg::init##N(T i) { \
    t = A; \
    arg1.F = i; \
  }

ARGACCESSORS(Bool,bool,i,BOOL_ARG)
ARGACCESSORS(Char,char,i,CHAR_ARG)
ARGACCESSORS(SignedChar,signed char,i,SCHAR_ARG)
ARGACCESSORS(UnsignedChar,unsigned char,u_i,UCHAR_ARG)
ARGACCESSORS(Int,int,i,INT_ARG)
ARGACCESSORS(UnsignedInt,unsigned int,u_i,UINT_ARG)
ARGACCESSORS(Short,short,i,SHORT_ARG)
ARGACCESSORS(UnsignedShort,unsigned short,u_i,USHORT_ARG)
ARGACCESSORS(Long,long,i,LONG_ARG)
ARGACCESSORS(UnsignedLong,unsigned long,u_i,LONG_ARG)
ARGACCESSORS(Float,float,d,FLOAT_ARG)
ARGACCESSORS(Double,double,d,DOUBLE_ARG)

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
  void
  Arg::initVar(int i) {
    t = VAR_ARG;
    arg1.i = i;
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
  const ArrayArg*
  Arg::toArray(void) const {
    if (!isArray())
      throw ReflectionException("not an ArrayArg");
    return static_cast<const ArrayArg*>(this);
  }
  ArrayArg*
  Arg::newArray(int n) {
    Arg* ret = new Arg(ARRAY_ARG);
    ret->arg1.i = n;
    if (n > 0)
      ret->arg2.aa = heap.alloc<Arg*>(n);
    else
      ret->arg2.aa = NULL;
    return static_cast<ArrayArg*>(ret);
  }
  void
  Arg::initArray(int n) {
    t = ARRAY_ARG;
    arg1.i = n;
    arg2.aa = heap.alloc<Arg*>(n);
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
  const IntArrayArg*
  Arg::toIntArray(void) const {
    if (!isIntArray())
      throw ReflectionException("not an IntArrayArg");
    return static_cast<const IntArrayArg*>(this);
  }
  IntArrayArg*
  Arg::newIntArray(int n) {
    Arg* ret = new Arg(INT_ARRAY_ARG);
    ret->arg1.i = n;
    if (n > 0)
      ret->arg2.ia = heap.alloc<int>(n);
    else
      ret->arg2.ia = NULL;
    return static_cast<IntArrayArg*>(ret);
  }
  void
  Arg::initIntArray(int n) {
    t = INT_ARRAY_ARG;
    arg1.i = n;
    arg2.ia = heap.alloc<int>(n);
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
  Arg*
  Arg::newString(const Support::Symbol& s) {
    Arg* ret = new Arg(STRING_ARG);
    ret->arg1.s = strdup(s.toString().c_str());
    return ret;    
  }
  void
  Arg::initString(const char* s) {
    t = STRING_ARG;
    arg1.s = strdup(s);
  }

#if defined(_MSC_VER)
#undef strdup
#endif

  bool
  Arg::isPair(void) const {
    return (t == PAIR_ARG);
  }
  Arg*
  Arg::first(void) {
    if (!isPair())
      throw ReflectionException("not a PairArg");
    return arg1.first;
  }
  const Arg*
  Arg::first(void) const {
    if (!isPair())
      throw ReflectionException("not a PairArg");
    return arg1.first;
  }
  Arg*
  Arg::second(void) {
    if (!isPair())
      throw ReflectionException("not a PairArg");
    return arg2.second;
  }
  const Arg*
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
  void
  Arg::initPair(Arg* a, Arg* b) {
    t = PAIR_ARG;
    arg1.first = a;
    arg2.second = b;
  }

  bool
  Arg::isSharedObject(void) const {
    return (t == SHARED_OBJECT_ARG);
  }
  Arg*
  Arg::toSharedObject(void) {
    if (!isSharedObject())
      throw ReflectionException("not a SharedObjectArg");
    return arg1.first;
  }
  const Arg*
  Arg::toSharedObject(void) const {
    if (!isSharedObject())
      throw ReflectionException("not a SharedObjectArg");
    return arg1.first;
  }
  Arg*
  Arg::newSharedObject(Arg* a) {
    Arg* ret = new Arg(SHARED_OBJECT_ARG);
    ret->arg1.first = a;
    return ret;
  }
  void
  Arg::initSharedObject(Arg* a) {
    t = SHARED_OBJECT_ARG;
    arg1.first = a;
  }
  
  bool
  Arg::isSharedReference(void) const {
    return (t == SHARED_REF_ARG);
  }
  int
  Arg::toSharedReference(void) const {
    if (!isSharedReference())
      throw ReflectionException("not a SharedReferenceArg");
    return arg1.i;
  }
  Arg*
  Arg::newSharedReference(int ref) {
    Arg* ret = new Arg(SHARED_REF_ARG);
    ret->arg1.i = ref;
    return ret;
  }
  void
  Arg::initSharedReference(int ref) {
    t = SHARED_REF_ARG;
    arg1.i = ref;
  }

  Arg::~Arg(void) {
    switch (t) {
    case ARRAY_ARG:
      for (int i=arg1.i; i--;)
        delete arg2.aa[i];
      if (arg2.aa != NULL)
        heap.rfree(arg2.aa);
      break;
    case INT_ARRAY_ARG:
      if (arg2.ia != NULL)
        heap.rfree(arg2.ia);
      break;
    case PAIR_ARG:
      delete arg1.first;
      delete arg2.second;
      break;
    case STRING_ARG:
      ::free(arg1.s);
      break;
    case SHARED_OBJECT_ARG:
      delete arg1.first;
      break;
    default:
      break;
    }
  }

  const Arg*
  ArrayArg::operator [](int i) const {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.aa[i];
  }
  Arg*&
  ArrayArg::operator [](int i) {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.aa[i];
  }
  int
  ArrayArg::size(void) const {
    return arg1.i;
  }

  const int&
  IntArrayArg::operator [](int i) const {
    if (i >= arg1.i)
      throw ReflectionException("Array index out of range");
    return arg2.ia[i];
  }
  int&
  IntArrayArg::operator [](int i) {
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
  IntArrayArgRanges::operator ()(void) { return n < a->size(); }

  void
  IntArrayArgRanges::operator ++(void) { n += 2; }

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

  /// Implementation of a VarSpec, holding all information about a variable
  class VarSpec::Domain {
  private:
    /// A representation of the domain of this variable
    Arg* _dom;
  public:
    /// The variable type identifier for this variable
    Support::Symbol _vti;
    /// The name of this variable
    Support::Symbol _n;
    /// Reference count
    int r;
    /// Return representation of the domain
    Arg* dom(void) const;
    /// Return whether domain is assigned
    bool assigned(void) const;
    /// Constructor
    Domain(Support::Symbol vti, Arg* domain, bool assigned);
    /// Destructor
    ~Domain(void);
  };

  /*
   * Variable specifications
   *
   */

  inline
  VarSpec::Domain::Domain(Support::Symbol vti, Arg* domain, bool assigned)
  : _dom(assigned ? static_cast<Arg*>(Support::mark(domain)) : domain),
    _vti(vti),
    r(1) {}

  inline
  VarSpec::Domain::~Domain(void) {
    if (Support::marked(_dom))
      delete static_cast<Arg*>(Support::unmark(_dom));
    else
      delete _dom;
  }

  inline Arg*
  VarSpec::Domain::dom(void) const {
    if (Support::marked(_dom))
      return static_cast<Arg*>(Support::unmark(_dom));
    else
      return _dom;    
  }

  inline bool
  VarSpec::Domain::assigned(void) const {
    return Support::marked(_dom);
  }

  VarSpec::VarSpec(void) : _dom(NULL) {}
  
  VarSpec::VarSpec(Support::Symbol vti, Arg* dom, bool assigned) 
  : _dom(new Domain(vti,dom,assigned)) {}

  VarSpec::VarSpec(const VarSpec& s) : _dom(s._dom) {
    if (_dom)
      _dom->r++;
  }
  
  const VarSpec&
  VarSpec::operator =(const VarSpec& s) {
    if (this != &s) {
      if (_dom && --_dom->r == 0)
        delete _dom;
      _dom = s._dom;
      if (_dom)
        _dom->r++;
    }
    return *this;
  }
  
  VarSpec::~VarSpec(void) {
    if (_dom && --_dom->r == 0)
      delete _dom;
  }

  void
  VarSpec::name(const Support::Symbol& n) {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    _dom->_n = n;
  }
  
  Support::Symbol
  VarSpec::name(void) const {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    return _dom->_n;
  }

  bool
  VarSpec::hasName(void) const {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    return !_dom->_n.empty();
  }

  Support::Symbol
  VarSpec::vti(void) const {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    return _dom->_vti;
  }

  Arg*
  VarSpec::dom(void) const {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    return _dom->dom();
  }

  bool
  VarSpec::assigned(void) const {
    if (_dom == NULL)
      throw ReflectionException("Empty VarSpec");
    return _dom->assigned();    
  }

  //
  // ActorSpec
  //

  /// Implementation of an ActorSpec, holding all information about an actor
  class ActorSpec::Arguments {
  public:
    /// The actor type identifier of this actor
    Support::Symbol _ati;
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
  ActorSpec::Arguments::Arguments(const Support::Symbol& ati)
   :  _ati(ati), size(4), n(0), r(1) {
     a = heap.alloc<Arg*>(size);
  }

  inline
  ActorSpec::Arguments::~Arguments(void) {
    for (int i=n; i--;)
      delete a[i];
    heap.free<Arg*>(a,n);
  }

  void
  ActorSpec::resize(void) {
    assert(_args != NULL);
    int ns = _args->size * 3 / 2;
    _args->a = heap.realloc<Arg*>(_args->a, _args->size, ns);
    _args->size = ns;
  }

  ActorSpec::ActorSpec(void) : _args(NULL) {}

  ActorSpec::ActorSpec(const Support::Symbol& ati) {
    _args = new Arguments(ati);
  }

  ActorSpec::ActorSpec(const ActorSpec& s) : _args(s._args) {
    if (_args != NULL)
      _args->r++;
  }
  
  const ActorSpec&
  ActorSpec::operator =(const ActorSpec& s) {
    if (this != &s) {
      if (_args && --_args->r == 0)
        delete _args;
      _args = s._args;
      if (_args)
        _args->r++;
    }
    return *this;
  }

  Arg*
  ActorSpec::operator [](int i) const {
    if (_args == NULL || i < 0 || i >= _args->n)
      throw ReflectionException("Array index out of range");
    return _args->a[i];
  }

  int
  ActorSpec::noOfArgs(void) const {
    return _args == NULL ? 0 : _args->n;
  }

  void
  ActorSpec::checkArity(int n) const {
    if (_args == NULL || _args->n != n) {
      throw ReflectionException("Illegal arity in ActorSpec");
    }
  }

  Support::Symbol
  ActorSpec::ati(void) const {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    return _args->_ati;
  }

  bool
  ActorSpec::isBranching(void) const {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    return _args->queue < 0;
  }

  int
  ActorSpec::queue(void) const {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    return _args->queue-1;
  }

  unsigned int
  ActorSpec::branchingId(void) const {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    assert(isBranching());
    return static_cast<unsigned int>(-_args->queue-1);
  }

  ActorSpec::~ActorSpec(void) {
    if (_args && --_args->r == 0)
      delete _args;
  }

  void
  ActorSpec::add(Arg* arg) {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    if (_args->n == _args->size)
      resize();
    _args->a[_args->n] = arg;
    _args->n++;
  }

  void
  ActorSpec::queue(int q) {
    if (_args == NULL)
      throw ReflectionException("Empty ActorSpec");
    _args->queue = q;
  }

  /*
   * Branch specification
   *
   */
   
  /// Implementation of a BranchingSpec
  class BranchingSpec::Arguments {
  public:
    /// The actor type identifier of this actor
    Support::Symbol _ati;
    /// The number of alternatives of this branch
    unsigned int   n;
    /// The arguments of this branch
    Arg** a;
    /// Reference counter
    int r;
    /// Construct arguments for \a id with \a a alternatives
    Arguments(const Support::Symbol& ati, unsigned int n);
    /// Destructor
    ~Arguments(void);
  };

  inline
  BranchingSpec::Arguments::Arguments(const Support::Symbol& ati0,
                                      unsigned int n0)
   : _ati(ati0), n(n0), r(1) {
     a = heap.alloc<Arg*>(n);
     for (unsigned int i=n; i--;)
       a[i] = NULL;
  }

  inline
  BranchingSpec::Arguments::~Arguments(void) {
    for (unsigned int i=n; i--;)
      delete a[i];
    heap.free<Arg*>(a,n);
  }
  
  BranchingSpec::BranchingSpec(void) : _args(NULL) {}
  
  BranchingSpec::BranchingSpec(const Support::Symbol& ati, unsigned int n)
    : _args(new Arguments(ati,n)) {}

  BranchingSpec::BranchingSpec(const BranchingSpec& s)
    : _args(s._args) {
    if (_args)
      _args->r++;
  }
  
  const BranchingSpec&
  BranchingSpec::operator =(const BranchingSpec& s) {
    if (this != &s) {
      if (_args && --_args->r == 0)
        delete _args;
      _args = s._args;
      if (_args)
        _args->r++;
    }
    return *this;
  }

  Support::Symbol
  BranchingSpec::ati(void) const {
    return _args->_ati;
  }

  Arg*
  BranchingSpec::operator [](int i) const {
    if (_args == NULL || i < 0 || static_cast<unsigned int>(i) >= _args->n)
      throw ReflectionException("Array index out of range");
    return _args->a[i];
  }

  Arg*&
  BranchingSpec::operator [](int i) {
    if (_args == NULL || i < 0 || static_cast<unsigned int>(i) >= _args->n)
      throw ReflectionException("Array index out of range");
    return _args->a[i];
  }

  BranchingSpec::~BranchingSpec(void) {
    if (_args && --_args->r == 0)
      delete _args;
  }
  
  unsigned int
  BranchingSpec::size(void) const {
    return _args == NULL ? 0 : _args->n;
  }
  

  /*
   * Specification iterator
   *
   */
   
  bool
  ActorSpecIter::operator ()(void) const {
    return cur != &s.a_actors;
  }

  void
  ActorSpecIter::operator ++(void) {
    cur = cur->next();
    while (active > &s.pc.p.queue[0] && (cur == active)) {
      active--;
      cur = active;
      cur = cur->next();
    }
    if (active == &s.pc.p.queue[0] && cur == active) {
      active--;
      cur = s.a_actors.next();
    }
    if (cur == s.b_commit)
      isBranching = true;
  }

  ActorSpecIter::ActorSpecIter(const Space& s0, VarMap& m0)
  : m(&m0), s(s0),
    active(s0.pc.p.active == NULL ?
           &s.pc.p.queue[PC_MAX] : s0.pc.p.active),
    cur(active),
    isBranching(false) {
    if (s.stable() && !s.failed())
      cur = &s.a_actors;
    ++(*this);
  }

  ActorSpec
  ActorSpecIter::actor(void) const {
    ActorSpec spec = static_cast<const Actor*>(cur)->spec(s,*m);
    if (isBranching)
      spec.queue(-1-static_cast<const Branching*>(cur)->id);
    else
      spec.queue( (active - &s.pc.p.queue[0]) + 1);
    return spec;
  }

  Unreflector::Unreflector(Space& home0, Reflection::VarMap& m0)
    : home(home0), m(m0) {}

  Unreflector::~Unreflector(void) {}
    
  Reflection::VarMap&
  Unreflector::varMap(void) const {
    return m;
  }

  void
  Unreflector::var(Reflection::VarSpec& spec) {
    VarImpBase* vb = NULL;
    if (!spec.name().empty() &&
        (vb = m.varImpBase(spec.name())) != NULL) {
      // TODO: assert that spec and original var are compatible,
      // constrain domain of var to spec
      Reflection::registry().constrainVar(home, vb, spec);
    } else {
      vb = Reflection::registry().createVar(home, spec);
    }
    (void) m.put(vb, new Reflection::VarSpec(spec));
  }

  void
  Unreflector::post(Reflection::ActorSpec& spec) {
    Reflection::registry().post(home, m, spec);
  }

  /* Generic variable */
  
  void
  Var::update(Space& home, bool share, Var& v) {
    new (&_vti) Support::Symbol(v._vti);
    _var = registry().updateVariable(home, share, v._var, v._vti);
  }
  
  Arg*
  Var::spec(const Space& home, VarMap& vm) const {
    return registry().spec(home, vm, _var, _vti);
  }

  std::ostream&
  operator <<(std::ostream& os, const Var& v) {
    std::ostringstream s;
    registry().printVariable(s, v._var, v._vti);
    return os << s.str();
  }

}}

// STATISTICS: kernel-other
