/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Gabor Szokoli <szokoli@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
 *     Gabor Szokoli, 2004
 *     Guido Tack, 2004
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

  // Minimum merit
  forceinline
  MeritMin::MeritMin(Space& home, const VarBranch& vb)
    : MeritBase<SetView,int>(home,vb) {}
  forceinline
  MeritMin::MeritMin(Space& home, bool shared, MeritMin& m)
    : MeritBase<SetView,int>(home,shared,m) {}
  forceinline int
  MeritMin::operator ()(const Space&, SetView x, int) {
    UnknownRanges<SetView> u(x);
    return u.min();
  }

  // Maximum merit
  forceinline
  MeritMax::MeritMax(Space& home, const VarBranch& vb)
    : MeritBase<SetView,int>(home,vb) {}
  forceinline
  MeritMax::MeritMax(Space& home, bool shared, MeritMax& m)
    : MeritBase<SetView,int>(home,shared,m) {}
  forceinline int
  MeritMax::operator ()(const Space&, SetView x, int) {
    int max = Limits::max;
    for (UnknownRanges<SetView> u(x); u(); ++u)
      max = u.max();
    return max;
  }

  // Size merit
  forceinline
  MeritSize::MeritSize(Space& home, const VarBranch& vb)
    : MeritBase<SetView,unsigned int>(home,vb) {}
  forceinline
  MeritSize::MeritSize(Space& home, bool shared, MeritSize& m)
    : MeritBase<SetView,unsigned int>(home,shared,m) {}
  forceinline unsigned int
  MeritSize::operator ()(const Space&, SetView x, int) {
    return x.unknownSize();
  }

  // Size over degree merit
  forceinline
  MeritSizeDegree::MeritSizeDegree(Space& home, const VarBranch& vb)
    : MeritBase<SetView,double>(home,vb) {}
  forceinline
  MeritSizeDegree::MeritSizeDegree(Space& home, bool shared, 
                                   MeritSizeDegree& m)
    : MeritBase<SetView,double>(home,shared,m) {}
  forceinline double
  MeritSizeDegree::operator ()(const Space&, SetView x, int) {
    return static_cast<double>(x.unknownSize()) / 
      static_cast<double>(x.degree());
  }

  // Size over AFC merit
  forceinline
  MeritSizeAfc::MeritSizeAfc(Space& home, const VarBranch& vb)
    : MeritBase<SetView,double>(home,vb) {}
  forceinline
  MeritSizeAfc::MeritSizeAfc(Space& home, bool shared, MeritSizeAfc& m)
    : MeritBase<SetView,double>(home,shared,m) {}
  forceinline double
  MeritSizeAfc::operator ()(const Space&, SetView x, int) {
    return static_cast<double>(x.unknownSize()) / x.afc();
  }

  // Size over activity merit
  forceinline
  MeritSizeActivity::MeritSizeActivity(Space& home,
                                       const VarBranch& vb)
    : MeritBase<SetView,double>(home,vb), activity(vb.activity()) {}
  forceinline
  MeritSizeActivity::MeritSizeActivity(Space& home, bool shared, 
                                       MeritSizeActivity& m)
    : MeritBase<SetView,double>(home,shared,m) {
    activity.update(home, shared, m.activity);
  }
  forceinline double
  MeritSizeActivity::operator ()(const Space&, SetView x, int i) {
    return static_cast<double>(x.unknownSize()) / activity[i];
  }
  forceinline bool
  MeritSizeActivity::notice(void) const {
    return true;
  }
  forceinline void
  MeritSizeActivity::dispose(Space&) {
    activity.~Activity();
  }

}}}

// STATISTICS: set-branch
