/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Scheduling { namespace Unary {

  /*
   * Mandatory fixed task
   */

  forceinline
  ManFixTask::ManFixTask(void) {}
  forceinline
  ManFixTask::ManFixTask(IntVar s, int p) : _s(s), _p(p) {}
  forceinline void
  ManFixTask::init(IntVar s, int p) {
    _s=s; _p=p;
  }

  forceinline int 
  ManFixTask::est(void) const {
    return _s.min();
  }
  forceinline int
  ManFixTask::ect(void) const {
    return _s.min()+_p;
  }
  forceinline int
  ManFixTask::lst(void) const {
    return _s.max();
  }
  forceinline int
  ManFixTask::lct(void) const {
    return _s.max()+_p;
  }
  forceinline IntVar
  ManFixTask::st(void) const {
    return _s;
  }
  forceinline int
  ManFixTask::p(void) const {
    return _p;
  }

  forceinline bool
  ManFixTask::assigned(void) const {
    return _s.assigned();
  }

  forceinline ModEvent 
  ManFixTask::est(Space& home, int n) {
    return _s.gq(home,n);
  }
  forceinline ModEvent
  ManFixTask::ect(Space& home, int n) {
    return _s.gq(home,n-_p);
  }
  forceinline ModEvent
  ManFixTask::lst(Space& home, int n) {
    return _s.lq(home,n);
  }
  forceinline ModEvent
  ManFixTask::lct(Space& home, int n) {
    return _s.lq(home,n-_p);
  }

  forceinline void
  ManFixTask::update(Space& home, bool share, ManFixTask& t) {
    _s.update(home,share,t._s); _p=t._p;
  }

  forceinline void
  ManFixTask::subscribe(Space& home, Propagator& p) {
    _s.subscribe(home, p, Int::PC_INT_BND);
  }
  forceinline void
  ManFixTask::cancel(Space& home, Propagator& p) {
    _s.cancel(home, p, Int::PC_INT_BND);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.p() << ':' << t.lct();
    return os << s.str();
  }
    


  /*
   * Mandatory flexible task
   */

  forceinline
  ManFlexTask::ManFlexTask(void) {}
  forceinline
  ManFlexTask::ManFlexTask(IntVar s, IntVar p, IntVar e) 
    : _s(s), _p(p), _e(e) {}
  forceinline void
  ManFlexTask::init(IntVar s, IntVar p, IntVar e) {
    _s=s; _p=p; _e=e;
  }

  forceinline int 
  ManFlexTask::est(void) const {
    return _s.min();
  }
  forceinline int
  ManFlexTask::ect(void) const {
    return _e.min();
  }
  forceinline int
  ManFlexTask::lst(void) const {
    return _s.max();
  }
  forceinline int
  ManFlexTask::lct(void) const {
    return _e.max();
  }
  forceinline int
  ManFlexTask::p(void) const {
    return _p.min();
  }
  forceinline IntVar
  ManFlexTask::st(void) const {
    return _s;
  }
  forceinline IntVar
  ManFlexTask::pt(void) const {
    return _p;
  }
  forceinline IntVar
  ManFlexTask::et(void) const {
    return _e;
  }

  forceinline bool
  ManFlexTask::assigned(void) const {
    return _s.assigned() && _p.assigned() && _e.assigned();
  }

  forceinline ModEvent 
  ManFlexTask::est(Space& home, int n) {
    return _s.gq(home,n);
  }
  forceinline ModEvent
  ManFlexTask::ect(Space& home, int n) {
    return _e.gq(home,n);
  }
  forceinline ModEvent
  ManFlexTask::lst(Space& home, int n) {
    return _s.lq(home,n);
  }
  forceinline ModEvent
  ManFlexTask::lct(Space& home, int n) {
    return _e.lq(home,n);
  }

  forceinline void
  ManFlexTask::update(Space& home, bool share, ManFlexTask& t) {
    _s.update(home,share,t._s);
    _p.update(home,share,t._p);
    _e.update(home,share,t._e);
  }

  forceinline void
  ManFlexTask::subscribe(Space& home, Propagator& p) {
    _s.subscribe(home, p, Int::PC_INT_BND);
    _p.subscribe(home, p, Int::PC_INT_BND);
    _e.subscribe(home, p, Int::PC_INT_BND);
  }
  forceinline void
  ManFlexTask::cancel(Space& home, Propagator& p) {
    _s.cancel(home, p, Int::PC_INT_BND);
    _p.cancel(home, p, Int::PC_INT_BND);
    _e.cancel(home, p, Int::PC_INT_BND);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFlexTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.pt() << ':' << t.lct();
    return os << s.str();
  }
    


  /*
   * Turn mandatory into optional task
   */
  template<class ManTask>
  forceinline
  ManToOptTask<ManTask>::ManToOptTask(void) {}

  template<class ManTask>
  forceinline bool
  ManToOptTask<ManTask>::mandatory(void) const {
    return _m.one();
  }
  template<class ManTask>
  forceinline bool
  ManToOptTask<ManTask>::excluded(void) const {
    return _m.zero();
  }
  template<class ManTask>
  forceinline bool
  ManToOptTask<ManTask>::optional(void) const {
    return _m.none();
  }

  template<class ManTask>
  forceinline bool
  ManToOptTask<ManTask>::assigned(void) const {
    return ManTask::assigned() && _m.assigned();
  }

  template<class ManTask>
  forceinline ModEvent 
  ManToOptTask<ManTask>::mandatory(Space& home) {
    return _m.one(home);
  }
  template<class ManTask>
  forceinline ModEvent 
  ManToOptTask<ManTask>::excluded(Space& home) {
    return _m.zero(home);
  }

  template<class ManTask>
  forceinline void
  ManToOptTask<ManTask>::update(Space& home, bool share, 
                                ManToOptTask<ManTask>& t) {
    ManTask::update(home, share, t);
    _m.update(home,share,t._m);
  }

  template<class ManTask>
  forceinline void
  ManToOptTask<ManTask>::subscribe(Space& home, Propagator& p) {
    ManTask::subscribe(home, p);
    _m.subscribe(home, p, Int::PC_BOOL_VAL);
  }
  template<class ManTask>
  forceinline void
  ManToOptTask<ManTask>::cancel(Space& home, Propagator& p) {
    _m.cancel(home, p, Int::PC_BOOL_VAL);
    ManTask::cancel(home, p);
  }


  /*
   * Optional fixed task
   */

  forceinline
  OptFixTask::OptFixTask(void) {}
  forceinline
  OptFixTask::OptFixTask(IntVar s, int p, BoolVar m) {
    ManFixTask::init(s,p); _m=m;
  }
  forceinline void
  OptFixTask::init(IntVar s, int p, BoolVar m) {
    ManFixTask::init(s,p); _m=m;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFixTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.p() << ':' << t.lct() << ':'
      << (t.mandatory() ? '1' : (t.optional() ? '?' : '0'));
    return os << s.str();
  }
    

  /*
   * Optional flexible task
   */

  forceinline
  OptFlexTask::OptFlexTask(void) {}
  forceinline
  OptFlexTask::OptFlexTask(IntVar s, IntVar p, IntVar e, BoolVar m) {
    ManFlexTask::init(s,p,e); _m=m;
  }
  forceinline void
  OptFlexTask::init(IntVar s, IntVar p, IntVar e, BoolVar m) {
    ManFlexTask::init(s,p,e); _m=m;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFlexTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.pt() << ':' << t.lct() << ':'
      << (t.mandatory() ? '1' : (t.optional() ? '?' : '0'));
    return os << s.str();
  }
    
}}}

// STATISTICS: scheduling-var
