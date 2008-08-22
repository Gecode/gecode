/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2004
 *     Christian Schulte, 2004
 *     Gabor Szokoli, 2004
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

namespace Gecode { namespace Set { namespace Branch {

  forceinline
  BySizeMin::BySizeMin(void) {}
  forceinline
  BySizeMin::BySizeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMin::init(Space&, SetView x) { 
    size = x.unknownSize();
    return (size == 1) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMin::select(Space&, SetView x) {
    unsigned int us = x.unknownSize();
    if (us < size) {
      size = us;
      return (size == 1) ? VSS_BEST : VSS_BETTER;
    } else if (us > size) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeMin::type(void) {
    return Support::Symbol("Gecode::Set::Branch::BySizeMin");
  }

  forceinline
  BySizeMax::BySizeMax(void) {}
  forceinline
  BySizeMax::BySizeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMax::init(Space&, SetView x) { 
    size = x.unknownSize();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMax::select(Space&, SetView x) {
    unsigned int us = x.unknownSize();
    if (us > size) {
      size = us;
      return VSS_BETTER;
    } else if (us < size) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeMax::type(void) {
    return Support::Symbol("Gecode::Set::Branch::BySizeMax");
  }

  forceinline
  ByMinMin::ByMinMin(void) {}
  forceinline
  ByMinMin::ByMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMin::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    min = u.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMin::select(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    if (u.min() < min) {
      min = u.min(); return VSS_BETTER;
    } else if (u.min() > min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMin::type(void) {
    return Support::Symbol("Gecode::Set::Branch::ByMinMin");
  }

  forceinline
  ByMinMax::ByMinMax(void) {}
  forceinline
  ByMinMax::ByMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMax::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    min = u.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMax::select(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    if (u.min() > min) {
      min = u.min(); return VSS_BETTER;
    } else if (u.min() < min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMax::type(void) {
    return Support::Symbol("Gecode::Set::Branch::ByMinMax");
  }

  forceinline
  ByMaxMin::ByMaxMin(void) {}
  forceinline
  ByMaxMin::ByMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMin::init(Space&, SetView x) {
    for (UnknownRanges<SetView> u(x); u(); ++u)
      max = u.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMin::select(Space&, SetView x) {
    int um = 0;
    for (UnknownRanges<SetView> u(x); u(); ++u)
      um = u.max();
    if (um < max) {
      max = um; return VSS_BETTER;
    } else if (um > max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMaxMin::type(void) {
    return Support::Symbol("Gecode::Set::Branch::ByMaxMin");
  }

  forceinline
  ByMaxMax::ByMaxMax(void) {}
  forceinline
  ByMaxMax::ByMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMax::init(Space&, SetView x) {
    for (UnknownRanges<SetView> u(x); u(); ++u)
      max = u.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMax::select(Space&, SetView x) {
    int um = 0;
    for (UnknownRanges<SetView> u(x); u(); ++u)
      um = u.max();
    if (um > max) {
      max = um; return VSS_BETTER;
    } else if (um < max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMaxMax::type(void) {
    return Support::Symbol("Gecode::Set::Branch::ByMaxMax");
  }

}}}

// STATISTICS: set-branch

