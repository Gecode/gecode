/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2006
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

namespace Gecode {

  forceinline
  ProjectorSet::ProjectorSet(void) {}

  forceinline
  ProjectorSet::ProjectorSet(int n) : _ps(n), _count(0), _arity(0) {}

  forceinline void
  ProjectorSet::update(Space& home, bool share, ProjectorSet& p) {
    _ps.update(home, share, p._ps);
    _count = p._count;
    _arity = p._arity;
  }

  forceinline int
  ProjectorSet::arity(void) const { return _arity; }

  template <bool negated>
  inline ExecStatus
  ProjectorSet::propagate(Space& home, ViewArray<Set::SetView>& x) {
    int failedCount = 0;
    for (int i=_count; i--; ) {
      ExecStatus es = _ps[i].propagate<negated>(home, x);
      switch (es) {
      case ES_FAILED:
        if (!negated)
          return ES_FAILED;
        failedCount++;
        break;
      case __ES_SUBSUMED:
        return __ES_SUBSUMED;
      default:
        break;
      }
    }
    return failedCount==_count ? ES_FAILED : ES_NOFIX;
  }

  forceinline int
  ProjectorSet::size(void) const { return _count; }

  forceinline const Projector&
  ProjectorSet::operator [](int i) const { return _ps[i]; }

  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const ProjectorSet& ps) {
    std::basic_ostringstream<Char,Traits> s;
    s.copyfmt(os); s.width(0);
    for (int i=0; i<ps.size(); i++) {
      s << ps[i] << std::endl;
    }
    return os << s.str();
  }

}

// STATISTICS: set-prop
