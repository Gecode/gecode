/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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
  template<class View>
  forceinline
  MeritMin<View>::MeritMin(Space& home, const VarBranch& vb)
    : MeritBase<View,int>(home,vb) {}
  template<class View>
  forceinline
  MeritMin<View>::MeritMin(Space& home, bool shared, MeritMin& m)
    : MeritBase<View,int>(home,shared,m) {}
  template<class View>
  forceinline int
  MeritMin<View>::operator ()(const Space&, View x, int) {
    return x.min();
  }

  // Maximum merit
  template<class View>
  forceinline
  MeritMax<View>::MeritMax(Space& home, const VarBranch& vb)
    : MeritBase<View,int>(home,vb) {}
  template<class View>
  forceinline
  MeritMax<View>::MeritMax(Space& home, bool shared, MeritMax& m)
    : MeritBase<View,int>(home,shared,m) {}
  template<class View>
  forceinline int
  MeritMax<View>::operator ()(const Space&, View x, int) {
    return x.max();
  }

  // Size merit
  template<class View>
  forceinline
  MeritSize<View>::MeritSize(Space& home, const VarBranch& vb)
    : MeritBase<View,unsigned int>(home,vb) {}
  template<class View>
  forceinline
  MeritSize<View>::MeritSize(Space& home, bool shared, MeritSize& m)
    : MeritBase<View,unsigned int>(home,shared,m) {}
  template<class View>
  forceinline unsigned int
  MeritSize<View>::operator ()(const Space&, View x, int) {
    return x.size();
  }

  // Size over degree merit
  template<class View>
  forceinline
  MeritSizeDegree<View>::MeritSizeDegree(Space& home, const VarBranch& vb)
    : MeritBase<View,double>(home,vb) {}
  template<class View>
  forceinline
  MeritSizeDegree<View>::MeritSizeDegree(Space& home, bool shared, 
                                   MeritSizeDegree& m)
    : MeritBase<View,double>(home,shared,m) {}
  template<class View>
  forceinline double
  MeritSizeDegree<View>::operator ()(const Space&, View x, int) {
    return static_cast<double>(x.size()) / static_cast<double>(x.degree());
  }

  // Size over AFC merit
  template<class View>
  forceinline
  MeritSizeAfc<View>::MeritSizeAfc(Space& home, const VarBranch& vb)
    : MeritBase<View,double>(home,vb) {}
  template<class View>
  forceinline
  MeritSizeAfc<View>::MeritSizeAfc(Space& home, bool shared, MeritSizeAfc& m)
    : MeritBase<View,double>(home,shared,m) {}
  template<class View>
  forceinline double
  MeritSizeAfc<View>::operator ()(const Space&, View x, int) {
    return static_cast<double>(x.size()) / x.afc();
  }

  // Size over activity merit
  template<class View>
  forceinline
  MeritSizeActivity<View>::MeritSizeActivity(Space& home,
                                       const VarBranch& vb)
    : MeritBase<View,double>(home,vb), activity(vb.activity()) {}
  template<class View>
  forceinline
  MeritSizeActivity<View>::MeritSizeActivity(Space& home, bool shared, 
                                       MeritSizeActivity& m)
    : MeritBase<View,double>(home,shared,m) {
    activity.update(home, shared, m.activity);
  }
  template<class View>
  forceinline double
  MeritSizeActivity<View>::operator ()(const Space&, View x, int i) {
    return static_cast<double>(x.size()) / activity[i];
  }
  template<class View>
  forceinline bool
  MeritSizeActivity<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  MeritSizeActivity<View>::dispose(Space&) {
    activity.~Activity();
  }

  // Minimum regret merit
  template<class View>
  forceinline
  MeritRegretMin<View>::MeritRegretMin(Space& home, const VarBranch& vb)
    : MeritBase<View,unsigned int>(home,vb) {}
  template<class View>
  forceinline
  MeritRegretMin<View>::MeritRegretMin(Space& home, bool shared, MeritRegretMin& m)
    : MeritBase<View,unsigned int>(home,shared,m) {}
  template<class View>
  forceinline unsigned int
  MeritRegretMin<View>::operator ()(const Space&, View x, int) {
    return x.regret_min();
  }

  // Maximum regret merit
  template<class View>
  forceinline
  MeritRegretMax<View>::MeritRegretMax(Space& home, const VarBranch& vb)
    : MeritBase<View,unsigned int>(home,vb) {}
  template<class View>
  forceinline
  MeritRegretMax<View>::MeritRegretMax(Space& home, bool shared, MeritRegretMax& m)
    : MeritBase<View,unsigned int>(home,shared,m) {}
  template<class View>
  forceinline unsigned int
  MeritRegretMax<View>::operator ()(const Space&, View x, int) {
    return x.regret_max();
  }

}}}

// STATISTICS: int-branch
