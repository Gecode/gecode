/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

namespace Gecode { namespace CpltSet { namespace Branch {

  forceinline
  BySizeMin::BySizeMin(void) {}
  forceinline
  BySizeMin::BySizeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMin::init(Space&, CpltSetView x) { 
    size = x.unknownSize();
    return (size == 1) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMin::select(Space&, CpltSetView x) {
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
    return Support::Symbol("Gecode::CpltSet::Branch::BySizeMin");
  }

  forceinline
  BySizeMax::BySizeMax(void) {}
  forceinline
  BySizeMax::BySizeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMax::init(Space&, CpltSetView x) { 
    size = x.unknownSize();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMax::select(Space&, CpltSetView x) {
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
    return Support::Symbol("Gecode::CpltSet::Branch::BySizeMax");
  }



  forceinline
  ByMinMin::ByMinMin(void) {}
  forceinline
  ByMinMin::ByMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMin::init(Space&, CpltSetView x) {
    Set::UnknownRanges<CpltSetView> u(x);
    min = u.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMin::select(Space&, CpltSetView x) {
    Set::UnknownRanges<CpltSetView> u(x);
    int um = u.min();
    if (um < min) {
      min = um; return VSS_BETTER;
    } else if (um > min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMin::type(void) {
    return Support::Symbol("Gecode::CpltSet::Branch::ByMinMin");
  }


  forceinline
  ByMinMax::ByMinMax(void) {}
  forceinline
  ByMinMax::ByMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMax::init(Space&, CpltSetView x) {
    Set::UnknownRanges<CpltSetView> u(x);
    min = u.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMax::select(Space&, CpltSetView x) {
    Set::UnknownRanges<CpltSetView> u(x);
    int um = u.min();
    if (um > min) {
      min = um; return VSS_BETTER;
    } else if (um < min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMax::type(void) {
    return Support::Symbol("Gecode::CpltSet::Branch::ByMinMax");
  }


  forceinline
  ByMaxMin::ByMaxMin(void) {}
  forceinline
  ByMaxMin::ByMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMin::init (Space&, CpltSetView x) {
    for (Set::UnknownRanges<CpltSetView> u(x); u(); ++u)
      max = u.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMin::select(Space&, CpltSetView x) {
    int um = 0;
    for (Set::UnknownRanges<CpltSetView> u(x); u(); ++u)
      max = u.max();
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
    return Support::Symbol("Gecode::CpltSet::Branch::ByMaxMin");
  }

  forceinline
  ByMaxMax::ByMaxMax(void) {}
  forceinline
  ByMaxMax::ByMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<CpltSetView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMax::init (Space&, CpltSetView x) {
    for (Set::UnknownRanges<CpltSetView> u(x); u(); ++u)
      max = u.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMax::select(Space&, CpltSetView x) {
    int um = 0;
    for (Set::UnknownRanges<CpltSetView> u(x); u(); ++u)
      max = u.max();
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
    return Support::Symbol("Gecode::CpltSet::Branch::ByMaxMax");
  }

}}}

// STATISTICS: cpltset-branch
