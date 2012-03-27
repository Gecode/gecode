/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *
 *  Copyright:
 *
 *  Last modified:
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

namespace Gecode { namespace Float { namespace Branch {

  // Select variable with smallest min
  forceinline
  ByMinMin::ByMinMin(void) : min(0) {}
  forceinline
  ByMinMin::ByMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), min(0) {}
  forceinline ViewSelStatus
  ByMinMin::init(Space&, View x, int) {
    min = x.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMin::select(Space&, View x, int) {
    if (x.min() < min) {
      min = x.min(); return VSS_BETTER;
    } else if (x.min() > min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with largest min
  forceinline
  ByMinMax::ByMinMax(void) : min(0) {}
  forceinline
  ByMinMax::ByMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), min(0) {}
  forceinline ViewSelStatus
  ByMinMax::init(Space&, View x, int) {
    min = x.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMax::select(Space&, View x, int) {
    if (x.min() > min) {
      min = x.min(); return VSS_BETTER;
    } else if (x.min() < min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with smallest max
  forceinline
  ByMaxMin::ByMaxMin(void) : max(0) {}
  forceinline
  ByMaxMin::ByMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), max(0) {}
  forceinline ViewSelStatus
  ByMaxMin::init(Space&, View x, int) {
    max = x.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMin::select(Space&, View x, int) {
    if (x.max() < max) {
      max = x.max(); return VSS_BETTER;
    } else if (x.max() > max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with largest max
  forceinline
  ByMaxMax::ByMaxMax(void) : max(0) {}
  forceinline
  ByMaxMax::ByMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), max(0) {}
  forceinline ViewSelStatus
  ByMaxMax::init(Space&, View x, int) {
    max = x.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMax::select(Space&, View x, int) {
    if (x.max() > max) {
      max = x.max(); return VSS_BETTER;
    } else if (x.max() < max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with smallest width
  forceinline
  ByWidthMin::ByWidthMin(void) : width(0) {}
  forceinline
  ByWidthMin::ByWidthMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), width(0) {}
  forceinline ViewSelStatus
  ByWidthMin::init(Space&, View x, int) {
    width = x.width();
    return (width == 2) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthMin::select(Space&, View x, int) {
    if (x.width() < width) {
      width = x.width();
      return (width == 2) ? VSS_BEST : VSS_BETTER;
    } else if (x.width() > width) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with largest width
  forceinline
  ByWidthMax::ByWidthMax(void) : width(0) {}
  forceinline
  ByWidthMax::ByWidthMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), width(0) {}
  forceinline ViewSelStatus
  ByWidthMax::init(Space&, View x, int) {
    width = x.width();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthMax::select(Space&, View x, int) {
    if (x.width() > width) {
      width = x.width();
      return VSS_BETTER;
    } else if (x.width() < width) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with smallest width/degree
  forceinline
  ByWidthDegreeMin::ByWidthDegreeMin(void) : widthdegree(0) {}
  forceinline
  ByWidthDegreeMin::ByWidthDegreeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), widthdegree(0) {}
  forceinline ViewSelStatus
  ByWidthDegreeMin::init(Space&, View x, int) {
    widthdegree =
      static_cast<double>(x.width())/static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthDegreeMin::select(Space&, View x, int) {
    double sd =
      static_cast<double>(x.width())/static_cast<double>(x.degree());
    if (sd < widthdegree) {
      widthdegree = sd; return VSS_BETTER;
    } else if (sd > widthdegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with largest width/degree
  forceinline
  ByWidthDegreeMax::ByWidthDegreeMax(void) : widthdegree(0) {}
  forceinline
  ByWidthDegreeMax::ByWidthDegreeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), widthdegree(0) {}
  forceinline ViewSelStatus
  ByWidthDegreeMax::init(Space&, View x, int) {
    widthdegree =
      static_cast<double>(x.width())/static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthDegreeMax::select(Space&, View x, int) {
    double sd =
      static_cast<double>(x.width())/static_cast<double>(x.degree());
    if (sd > widthdegree) {
      widthdegree = sd; return VSS_BETTER;
    } else if (sd < widthdegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with smallest width/afc
  forceinline
  ByWidthAfcMin::ByWidthAfcMin(void) : widthafc(0) {}
  forceinline
  ByWidthAfcMin::ByWidthAfcMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), widthafc(0) {}
  forceinline ViewSelStatus
  ByWidthAfcMin::init(Space&, View x, int) {
    widthafc = static_cast<double>(x.width())/x.afc();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthAfcMin::select(Space&, View x, int) {
    double sa = static_cast<double>(x.width())/x.afc();
    if (sa < widthafc) {
      widthafc = sa; return VSS_BETTER;
    } else if (sa > widthafc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }

  // Select variable with largest width/afc
  forceinline
  ByWidthAfcMax::ByWidthAfcMax(void) : widthafc(0) {}
  forceinline
  ByWidthAfcMax::ByWidthAfcMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<FloatView>(home,vbo), widthafc(0) {}
  forceinline ViewSelStatus
  ByWidthAfcMax::init(Space&, View x, int) {
    widthafc = static_cast<double>(x.width())/x.afc();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByWidthAfcMax::select(Space&, View x, int) {
    double sa = static_cast<double>(x.width())/x.afc();
    if (sa > widthafc) {
      widthafc = sa; return VSS_BETTER;
    } else if (sa < widthafc) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }


}}}

// STATISTICS: float-branch
