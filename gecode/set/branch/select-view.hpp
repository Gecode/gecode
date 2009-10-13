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
  BySizeMin::BySizeMin(void) : size(0U) {}
  forceinline
  BySizeMin::BySizeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), size(0U) {}
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


  forceinline
  BySizeMax::BySizeMax(void) : size(0U) {}
  forceinline
  BySizeMax::BySizeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), size(0U) {}
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


  forceinline
  ByMinMin::ByMinMin(void) : min(0) {}
  forceinline
  ByMinMin::ByMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), min(0) {}
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


  forceinline
  ByMinMax::ByMinMax(void) : min(0) {}
  forceinline
  ByMinMax::ByMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), min(0) {}
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


  forceinline
  ByMaxMin::ByMaxMin(void) : max(0) {}
  forceinline
  ByMaxMin::ByMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), max(0) {}
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


  forceinline
  ByMaxMax::ByMaxMax(void) : max(0) {}
  forceinline
  ByMaxMax::ByMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), max(0) {}
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


  // Select variable with smallest size/degree
  forceinline
  BySizeDegreeMin::BySizeDegreeMin(void) : sizedegree(0) {}
  forceinline
  BySizeDegreeMin::BySizeDegreeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), sizedegree(0) {}
  forceinline ViewSelStatus
  BySizeDegreeMin::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    sizedegree =
      static_cast<double>(Iter::Ranges::size(u))/
      static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeDegreeMin::select(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    double sd =
      static_cast<double>(Iter::Ranges::size(u))/
      static_cast<double>(x.degree());
    if (sd < sizedegree) {
      sizedegree = sd; return VSS_BETTER;
    } else if (sd > sizedegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with largest size/degree
  forceinline
  BySizeDegreeMax::BySizeDegreeMax(void) : sizedegree(0) {}
  forceinline
  BySizeDegreeMax::BySizeDegreeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), sizedegree(0) {}
  forceinline ViewSelStatus
  BySizeDegreeMax::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    sizedegree =
      static_cast<double>(Iter::Ranges::size(u))/
      static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeDegreeMax::select(Space&, View x) {
    UnknownRanges<SetView> u(x);
    double sd =
      static_cast<double>(Iter::Ranges::size(u))/
      static_cast<double>(x.degree());
    if (sd > sizedegree) {
      sizedegree = sd; return VSS_BETTER;
    } else if (sd < sizedegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with smallest size/afc
  forceinline
  BySizeAfcMin::BySizeAfcMin(void) : sizeafc(0) {}
  forceinline
  BySizeAfcMin::BySizeAfcMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), sizeafc(0) {}
  forceinline ViewSelStatus
  BySizeAfcMin::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    sizeafc = static_cast<double>(Iter::Ranges::size(u))/x.afc();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeAfcMin::select(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    double sa = static_cast<double>(Iter::Ranges::size(u))/x.afc();
    if (sa < sizeafc) {
      sizeafc = sa; return VSS_BETTER;
    } else if (sa > sizeafc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


  // Select variable with largest size/afc
  forceinline
  BySizeAfcMax::BySizeAfcMax(void) : sizeafc(0) {}
  forceinline
  BySizeAfcMax::BySizeAfcMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<SetView>(home,vbo), sizeafc(0) {}
  forceinline ViewSelStatus
  BySizeAfcMax::init(Space&, SetView x) {
    UnknownRanges<SetView> u(x);
    sizeafc = static_cast<double>(Iter::Ranges::size(u))/x.afc();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeAfcMax::select(Space&, View x) {
    UnknownRanges<SetView> u(x);
    double sa = static_cast<double>(Iter::Ranges::size(u))/x.afc();
    if (sa > sizeafc) {
      sizeafc = sa; return VSS_BETTER;
    } else if (sa < sizeafc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

}}}

// STATISTICS: set-branch
