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

  // Minimum merit
  forceinline
  MeritMin::MeritMin(void) {}
  forceinline
  MeritMin::MeritMin(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritMin::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.min());
  }

  // Maximum merit
  forceinline
  MeritMax::MeritMax(void) {}
  forceinline
  MeritMax::MeritMax(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritMax::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.max());
  }

  // Size merit
  forceinline
  MeritSize::MeritSize(void) {}
  forceinline
  MeritSize::MeritSize(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritSize::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.size());
  }

  // Size over degree merit
  forceinline
  MeritSizeDegree::MeritSizeDegree(void) {}
  forceinline
  MeritSizeDegree::MeritSizeDegree(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritSizeDegree::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.size()) / static_cast<double>(x.degree());
  }

  // Size over AFC merit
  forceinline
  MeritSizeAfc::MeritSizeAfc(void) {}
  forceinline
  MeritSizeAfc::MeritSizeAfc(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritSizeAfc::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.size()) / x.afc();
  }

  // Size over activity merit
  forceinline
  MeritSizeActivity::MeritSizeActivity(void) {}
  forceinline
  MeritSizeActivity::MeritSizeActivity(Space& home,
                                       const VarBranch& vb)
    : MeritBase<IntView>(home,vb), activity(vb.activity()) {}
  forceinline double
  MeritSizeActivity::operator ()(Space&, IntView x, int i) {
    return static_cast<double>(x.size()) / activity[i];
  }
  forceinline void
  MeritSizeActivity::update(Space& home, bool share, 
                            MeritSizeActivity& msa) {
    activity.update(home, share, msa.activity);
  }
  forceinline bool
  MeritSizeActivity::notice(void) const {
    return true;
  }
  forceinline void
  MeritSizeActivity::dispose(Space&) {
    activity.~Activity();
  }

  // Minimum regret merit
  forceinline
  MeritRegretMin::MeritRegretMin(void) {}
  forceinline
  MeritRegretMin::MeritRegretMin(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritRegretMin::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.regret_min());
  }

  // Maximum regret merit
  forceinline
  MeritRegretMax::MeritRegretMax(void) {}
  forceinline
  MeritRegretMax::MeritRegretMax(Space& home, const VarBranch& vb)
    : MeritBase<IntView>(home,vb) {}
  forceinline double
  MeritRegretMax::operator ()(Space&, IntView x, int) {
    return static_cast<double>(x.regret_max());
  }

}}}

// STATISTICS: int-branch
