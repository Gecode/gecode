/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Int { namespace Unary {

  template<class ManTask, class PL>
  forceinline
  ManProp<ManTask,PL>::ManProp(Home home, TaskArray<ManTask>& t)
    : TaskProp<ManTask,PL>(home,t) {}

  template<class ManTask, class PL>
  forceinline
  ManProp<ManTask,PL>::ManProp(Space& home, ManProp<ManTask,PL>& p)
    : TaskProp<ManTask,PL>(home,p) {}

  template<class ManTask, class PL>
  forceinline ExecStatus
  ManProp<ManTask,PL>::post(Home home, TaskArray<ManTask>& t) {
    if (t.size() > 1)
      (void) new (home) ManProp<ManTask,PL>(home,t);
    return ES_OK;
  }

  template<class ManTask, class PL>
  Actor*
  ManProp<ManTask,PL>::copy(Space& home) {
    return new (home) ManProp<ManTask,PL>(home,*this);
  }

  template<class ManTask, class PL>
  ExecStatus
  ManProp<ManTask,PL>::propagate(Space& home, const ModEventDelta&) {
    GECODE_ES_CHECK(overload(t));

    if (PL::basic)
      GECODE_ES_CHECK(timetabling(home,*this,t));

    if (PL::advanced) {
      GECODE_ES_CHECK(detectable(home,t));
      GECODE_ES_CHECK(notfirstnotlast(home,t));
      GECODE_ES_CHECK(edgefinding(home,t));
    }

    if (!PL::basic)
      GECODE_ES_CHECK(subsumed(home,*this,t));

    return ES_NOFIX;
  }

}}}

// STATISTICS: int-prop
