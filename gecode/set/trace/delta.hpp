/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2016
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
  SetTraceDelta::Glb::Glb(RangeList* o0, Set::SetView n0)
    : o(o0), n(n0) {
    init(n,o);
  }
  forceinline
  SetTraceDelta::Lub::Lub(RangeList* o0, Set::SetView n0)
    : o(o0), n(n0) {
    init(o,n);
  }

  forceinline
  SetTraceDelta::SetTraceDelta(Set::SetTraceView o, Set::SetView n,
                               const Delta&)
    : _glb(o.glb(),n), _lub(o.lub(),n) {}
  forceinline SetTraceDelta::Glb&
  SetTraceDelta::glb(void) {
    return _glb;
  }
  forceinline SetTraceDelta::Lub&
  SetTraceDelta::lub(void) {
    return _lub;
  }

}

// STATISTICS: set-trace
