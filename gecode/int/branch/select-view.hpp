/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

namespace Gecode { namespace Int { namespace Branch {

  // Select variable with smallest min
  forceinline
  ByMinMin::ByMinMin(void) {}
  forceinline
  ByMinMin::ByMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMin::init(Space&, View x) {
    min = x.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMin::select(Space&, View x) {
    if (x.min() < min) {
      min = x.min(); return VSS_BETTER;
    } else if (x.min() > min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByMinMin");
  }

  // Select variable with largest min
  forceinline
  ByMinMax::ByMinMax(void) {}
  forceinline
  ByMinMax::ByMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMinMax::init(Space&, View x) {
    min = x.min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMinMax::select(Space&, View x) {
    if (x.min() > min) {
      min = x.min(); return VSS_BETTER;
    } else if (x.min() < min) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMinMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByMinMax");
  }

  // Select variable with smallest max
  forceinline
  ByMaxMin::ByMaxMin(void) {}
  forceinline
  ByMaxMin::ByMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMin::init(Space&, View x) {
    max = x.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMin::select(Space&, View x) {
    if (x.max() < max) {
      max = x.max(); return VSS_BETTER;
    } else if (x.max() > max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMaxMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByMaxMin");
  }

  // Select variable with largest max
  forceinline
  ByMaxMax::ByMaxMax(void) {}
  forceinline
  ByMaxMax::ByMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByMaxMax::init(Space&, View x) {
    max = x.max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByMaxMax::select(Space&, View x) {
    if (x.max() > max) {
      max = x.max(); return VSS_BETTER;
    } else if (x.max() < max) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByMaxMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByMaxMax");
  }

  // Select variable with smallest size
  forceinline
  BySizeMin::BySizeMin(void) {}
  forceinline
  BySizeMin::BySizeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMin::init(Space&, View x) {
    size = x.size();
    return (size == 2) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMin::select(Space&, View x) {
    if (x.size() < size) {
      size = x.size();
      return (size == 2) ? VSS_BEST : VSS_BETTER;
    } else if (x.size() > size) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::BySizeMin");
  }

  // Select variable with largest size
  forceinline
  BySizeMax::BySizeMax(void) {}
  forceinline
  BySizeMax::BySizeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeMax::init(Space&, View x) {
    size = x.size();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeMax::select(Space&, View x) {
    if (x.size() > size) {
      size = x.size();
      return VSS_BETTER;
    } else if (x.size() < size) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::BySizeMax");
  }

  // Select variable with smallest size/degree
  forceinline
  BySizeDegreeMin::BySizeDegreeMin(void) {}
  forceinline
  BySizeDegreeMin::BySizeDegreeMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeDegreeMin::init(Space&, View x) {
    sizedegree =
      static_cast<double>(x.size())/static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeDegreeMin::select(Space&, View x) {
    double sd =
      static_cast<double>(x.size())/static_cast<double>(x.degree());
    if (sd < sizedegree) {
      sizedegree = sd; return VSS_BETTER;
    } else if (sd > sizedegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeDegreeMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::BySizeDegreeMin");
  }

  // Select variable with largest size/degree
  forceinline
  BySizeDegreeMax::BySizeDegreeMax(void) {}
  forceinline
  BySizeDegreeMax::BySizeDegreeMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  BySizeDegreeMax::init(Space&, View x) {
    sizedegree =
      static_cast<double>(x.size())/static_cast<double>(x.degree());
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  BySizeDegreeMax::select(Space&, View x) {
    double sd =
      static_cast<double>(x.size())/static_cast<double>(x.degree());
    if (sd > sizedegree) {
      sizedegree = sd; return VSS_BETTER;
    } else if (sd < sizedegree) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  BySizeDegreeMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::BySizeDegreeMax");
  }

  // Select variable with smallest min-regret
  forceinline
  ByRegretMinMin::ByRegretMinMin(void) {}
  forceinline
  ByRegretMinMin::ByRegretMinMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByRegretMinMin::init(Space&, View x) {
    regret = x.regret_min();
    return (regret == 1) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByRegretMinMin::select(Space&, View x) {
    if (x.regret_min() < regret) {
      regret = x.regret_min();
      return (regret == 1) ? VSS_BEST : VSS_BETTER;
    } else if (x.regret_min() > regret) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByRegretMinMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByRegretMinMin");
  }

  // Select variable with largest min-regret
  forceinline
  ByRegretMinMax::ByRegretMinMax(void) {}
  forceinline
  ByRegretMinMax::ByRegretMinMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByRegretMinMax::init(Space&, View x) {
    regret = x.regret_min();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByRegretMinMax::select(Space&, View x) {
    if (x.regret_min() > regret) {
      regret = x.regret_min();
      return VSS_BETTER;
    } else if (x.regret_min() < regret) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByRegretMinMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByRegretMinMax");
  }

  // Select variable with smallest max-regret
  forceinline
  ByRegretMaxMin::ByRegretMaxMin(void) {}
  forceinline
  ByRegretMaxMin::ByRegretMaxMin(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByRegretMaxMin::init(Space&, View x) {
    regret = x.regret_max();
    return (regret == 1) ? VSS_BEST : VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByRegretMaxMin::select(Space&, View x) {
    if (x.regret_max() < regret) {
      regret = x.regret_max();
      return (regret == 1) ? VSS_BEST : VSS_BETTER;
    } else if (x.regret_max() > regret) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByRegretMaxMin::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByRegretMaxMin");
  }

  // Select variable with largest max-regret
  forceinline
  ByRegretMaxMax::ByRegretMaxMax(void) {}
  forceinline
  ByRegretMaxMax::ByRegretMaxMax(Space& home, const VarBranchOptions& vbo)
    : ViewSelBase<IntView>(home,vbo) {}
  forceinline ViewSelStatus
  ByRegretMaxMax::init(Space&, View x) {
    regret = x.regret_max();
    return VSS_BETTER;
  }
  forceinline ViewSelStatus
  ByRegretMaxMax::select(Space&, View x) {
    if (x.regret_max() > regret) {
      regret = x.regret_max();
      return VSS_BETTER;
    } else if (x.regret_max() < regret) {
      return VSS_WORSE;
    } else {
      return VSS_TIE;
    }
  }
  inline Support::Symbol
  ByRegretMaxMax::type(void) {
    return Support::Symbol("Gecode::Int::Branch::ByRegretMaxMax");
  }

}}}

// STATISTICS: int-branch
