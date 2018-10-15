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

namespace Gecode { namespace Int { namespace Unary {

  template<class ManTask>
  forceinline ExecStatus
  manpost(Home home, TaskArray<ManTask>& t, IntPropLevel ipl) {
    switch (ba(ipl)) {
    case IPL_BASIC:
      return ManProp<ManTask,PLB>::post(home,t);
    case IPL_ADVANCED:
      return ManProp<ManTask,PLA>::post(home,t);
    case IPL_BASIC_ADVANCED: default:
      return ManProp<ManTask,PLBA>::post(home,t);
    }
    GECODE_NEVER;
    return ES_OK;
  }

  template<class OptTask>
  forceinline ExecStatus
  optpost(Home home, TaskArray<OptTask>& t, IntPropLevel ipl) {
    switch (ba(ipl)) {
    case IPL_BASIC:
      return OptProp<OptTask,PLB>::post(home,t);
    case IPL_ADVANCED:
      return OptProp<OptTask,PLA>::post(home,t);
    case IPL_BASIC_ADVANCED: default:
      return OptProp<OptTask,PLBA>::post(home,t);
    }
    GECODE_NEVER;
    return ES_OK;
  }

}}}

// STATISTICS: int-post
