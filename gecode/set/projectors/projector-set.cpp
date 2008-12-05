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

#include <gecode/set/projectors.hh>

namespace Gecode {

  void
  ProjectorSet::add(const Projector& p) {
    new (&_ps[_count]) Projector(p);
    _count++;

    _arity = std::max(_arity, p.arity());
  }

  void
  ProjectorSet::scope(Support::DynamicArray<int,Heap>& s,
                      unsigned int size) const {
    // Clear out s
    for (int i=std::max((unsigned)_arity+1, size); i--;)
      s[i]=Set::PC_SET_ANY + 1;

    // Collect scope from individual projectors
    for (int i=_count; i--; ) {
      _ps[i].scope(s);
    }
  }

  ExecStatus
  ProjectorSet::check(Space& home, ViewArray<Set::SetView>& x) {
    ExecStatus es = __ES_SUBSUMED;
    for (int i=0; i<_count; i++) {
      ExecStatus es_new = _ps[i].check(home, x);
      switch (es_new) {
      case ES_FAILED:
        return ES_FAILED;
      case __ES_SUBSUMED:
        break;
      default:
        es = es_new;
        break;
      }
    }
    return es;
  }

  std::ostream&
  operator <<(std::ostream& os, const ProjectorSet& ps) {
    for (int i=0; i<ps.size(); i++) {
      os << ps[i] << std::endl;
    }
    return os;
  }

}

// STATISTICS: set-prop
