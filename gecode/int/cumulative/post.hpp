/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

namespace Gecode { namespace Int { namespace Cumulative {

  template<class ManTask, class Cap>
  forceinline ExecStatus
  manpost(Home home, Cap c, TaskArray<ManTask>& t, IntPropLevel ipl) {
    switch (ba(ipl)) {
    case IPL_BASIC: default:
      return ManProp<ManTask,Cap,PLB>::post(home,c,t);
    case IPL_ADVANCED:
      return ManProp<ManTask,Cap,PLA>::post(home,c,t);
    case IPL_BASIC_ADVANCED:
      return ManProp<ManTask,Cap,PLBA>::post(home,c,t);
    }
    GECODE_NEVER;
    return ES_OK;
  }

  template<class OptTask, class Cap>
  forceinline ExecStatus
  optpost(Home home, Cap c, TaskArray<OptTask>& t, IntPropLevel ipl) {
    switch (ba(ipl)) {
    case IPL_BASIC: default:
      return OptProp<OptTask,Cap,PLB>::post(home,c,t);
    case IPL_ADVANCED:
      return OptProp<OptTask,Cap,PLA>::post(home,c,t);
    case IPL_BASIC_ADVANCED:
      return OptProp<OptTask,Cap,PLBA>::post(home,c,t);
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}

// STATISTICS: int-post
