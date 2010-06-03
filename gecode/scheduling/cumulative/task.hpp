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

namespace Gecode { namespace Scheduling { namespace Cumulative {

  /*
   * Mandatory fixed task
   */

  forceinline
  ManFixTask::ManFixTask(void) {}
  forceinline
  ManFixTask::ManFixTask(IntVar s, int p, int c) 
    : Unary::ManFixTask(s,p), _c(c) {}
  forceinline void
  ManFixTask::init(IntVar s, int p, int c) {
    Unary::ManFixTask::init(s,p); _c=c;
  }
  forceinline void
  ManFixTask::init(const ManFixTask& t) {
    Unary::ManFixTask::init(t); _c=t._c;
  }

  forceinline int
  ManFixTask::c(void) const {
    return _c;
  }
  forceinline double
  ManFixTask::e(void) const {
    return static_cast<double>(pmin())*c();
  }

  forceinline void
  ManFixTask::update(Space& home, bool share, ManFixTask& t) {
    Unary::ManFixTask::update(home,share,t); _c=t._c;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFixTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ":[" << t.pmin() << ',' << t.c() << "]:" << t.lct();
    return os << s.str();
  }
    
  /*
   * Mandatory flexible task
   */

  forceinline
  ManFlexTask::ManFlexTask(void) {}
  forceinline
  ManFlexTask::ManFlexTask(IntVar s, IntVar p, IntVar e, int c)
    : Unary::ManFlexTask(s,p,e), _c(c) {}
  forceinline void
  ManFlexTask::init(IntVar s, IntVar p, IntVar e, int c) {
    Unary::ManFlexTask::init(s,p,e); _c=c;
  }
  forceinline void
  ManFlexTask::init(const ManFlexTask& t) {
    Unary::ManFlexTask::init(t); _c=t._c;
  }

  forceinline int
  ManFlexTask::c(void) const {
    return _c;
  }
  forceinline double
  ManFlexTask::e(void) const {
    return static_cast<double>(pmin())*c();
  }

  forceinline void
  ManFlexTask::update(Space& home, bool share, ManFlexTask& t) {
    Unary::ManFlexTask::update(home,share,t); _c=t._c;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ManFlexTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.lst() << ':' << t.pmin() << ':'
      << t.pmax() << ':' << t.c() << ':' << t.ect() << ':' << t.lct();
    return os << s.str();
  }

  /*
   * Optional fixed task
   */

  forceinline
  OptFixTask::OptFixTask(void) {}
  forceinline
  OptFixTask::OptFixTask(IntVar s, int p, int c, BoolVar m) {
    ManFixTask::init(s,p,c); _m=m;
  }
  forceinline void
  OptFixTask::init(IntVar s, int p, int c, BoolVar m) {
    ManFixTask::init(s,p,c); _m=m;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFixTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ":[" << t.pmin() << ',' << t.c() << "]:" << t.lct() << ':'
      << (t.mandatory() ? '1' : (t.optional() ? '?' : '0'));
    return os << s.str();
  }

  /*
   * Optional flexible task
   */

  forceinline
  OptFlexTask::OptFlexTask(void) {}
  forceinline
  OptFlexTask::OptFlexTask(IntVar s, IntVar p, IntVar e, int c, BoolVar m) {
    ManFlexTask::init(s,p,e,c); _m=m;
  }
  forceinline void
  OptFlexTask::init(IntVar s, IntVar p, IntVar e, int c, BoolVar m) {
    ManFlexTask::init(s,p,e,c); _m=m;
  }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const OptFlexTask& t) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    s << t.est() << ':' << t.lst() << ':' << t.pmin() << ':'
      << t.pmax() << ':' << t.c() << ':' << t.ect() << ':' << t.lct()
      << (t.mandatory() ? '1' : (t.optional() ? '?' : '0'));
    return os << s.str();
  }
    
}}}

// STATISTICS: scheduling-var
