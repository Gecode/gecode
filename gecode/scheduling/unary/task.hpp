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
  forceinline void
  ManFixTask::init(const ManFixTask& t) {
    _s=t._s; _p=t._p;
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
  forceinline int
  ManFixTask::pmin(void) const {
    return _p;
  }
  forceinline int
  ManFixTask::pmax(void) const {
    return _p;
  }
  forceinline IntVar
  ManFixTask::st(void) const {
    return _s;
  }

  forceinline bool
  ManFixTask::mandatory(void) const {
    return true;
  }
  forceinline bool
  ManFixTask::excluded(void) const {
    return false;
  }
  forceinline bool
  ManFixTask::optional(void) const {
    return false;
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
  forceinline ModEvent
  ManFixTask::norun(Space& home, int e, int l) {
    if (e <= l) {
      Iter::Ranges::Singleton r(e-_p+1,l);
      return _s.minus_r(home,r,false);
    } else {
      return Int::ME_INT_NONE;
    }
  }


  forceinline ModEvent
  ManFixTask::mandatory(Space&) {
    return Int::ME_INT_NONE;
  }
  forceinline ModEvent
  ManFixTask::excluded(Space&) {
    return Int::ME_INT_FAILED;
  }

  forceinline void
  ManFixTask::update(Space& home, bool share, ManFixTask& t) {
    _s.update(home,share,t._s); _p=t._p;
  }

  forceinline void
  ManFixTask::subscribe(Space& home, Propagator& p, PropCond pc) {
    _s.subscribe(home, p, pc);
  }
  forceinline void
  ManFixTask::cancel(Space& home, Propagator& p, PropCond pc) {
    _s.cancel(home, p, pc);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.pmin() << ':' << t.lct();
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
  forceinline void
  ManFlexTask::init(const ManFlexTask& t) {
    _s=t._s; _p=t._p; _e=t._e;
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
  ManFlexTask::pmin(void) const {
    return _p.min();
  }
  forceinline int
  ManFlexTask::pmax(void) const {
    return _p.max();
  }
  forceinline IntVar
  ManFlexTask::st(void) const {
    return _s;
  }
  forceinline IntVar
  ManFlexTask::p(void) const {
    return _p;
  }
  forceinline IntVar
  ManFlexTask::e(void) const {
    return _e;
  }

  forceinline bool
  ManFlexTask::mandatory(void) const {
    return true;
  }
  forceinline bool
  ManFlexTask::excluded(void) const {
    return false;
  }
  forceinline bool
  ManFlexTask::optional(void) const {
    return false;
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
  forceinline ModEvent
  ManFlexTask::norun(Space& home, int e, int l) {
    if (e <= l) {
      Iter::Ranges::Singleton sr(e-_p.min()+1,l);
      GECODE_ME_CHECK(_s.minus_r(home,sr,false));
      Iter::Ranges::Singleton er(e+1,_p.min()+l);
      return _e.minus_r(home,er,false);
    } else {
      return Int::ME_INT_NONE;
    }
  }


  forceinline ModEvent
  ManFlexTask::mandatory(Space&) {
    return Int::ME_INT_NONE;
  }
  forceinline ModEvent
  ManFlexTask::excluded(Space&) {
    return Int::ME_INT_FAILED;
  }

  forceinline void
  ManFlexTask::update(Space& home, bool share, ManFlexTask& t) {
    _s.update(home,share,t._s);
    _p.update(home,share,t._p);
    _e.update(home,share,t._e);
  }

  forceinline void
  ManFlexTask::subscribe(Space& home, Propagator& p, PropCond pc) {
    _s.subscribe(home, p, pc);
    _p.subscribe(home, p, pc);
    _e.subscribe(home, p, pc);
  }
  forceinline void
  ManFlexTask::cancel(Space& home, Propagator& p, PropCond pc) {
    _s.cancel(home, p, pc);
    _p.cancel(home, p, pc);
    _e.cancel(home, p, pc);
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFlexTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.lst() << ':' << t.pmin() << ':'
      << t.pmax() << ':' << t.ect() << ':' << t.lct();
    return os << s.str();
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
    s << t.est() << ':' << t.pmin() << ':' << t.lct() << ':'
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
    s << t.est() << ':' << t.lst() << ':' << t.pmin() << ':'
      << t.pmax() << ':' << t.ect() << ':' << t.lct() << ':'
      << (t.mandatory() ? '1' : (t.optional() ? '?' : '0'));
    return os << s.str();
  }
    
}}}

// STATISTICS: scheduling-var
