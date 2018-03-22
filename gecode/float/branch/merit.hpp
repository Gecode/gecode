/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *     Christian Schulte, 2012
 *     Vincent Barichard, 2012
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

  // Minimum merit
  forceinline
  MeritMin::MeritMin(Space& home, const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb) {}
  forceinline
  MeritMin::MeritMin(Space& home, MeritMin& m)
    : MeritBase<FloatView,double>(home,m) {}
  forceinline double
  MeritMin::operator ()(const Space&, FloatView x, int) {
    return x.min();
  }

  // Maximum merit
  forceinline
  MeritMax::MeritMax(Space& home, const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb) {}
  forceinline
  MeritMax::MeritMax(Space& home, MeritMax& m)
    : MeritBase<FloatView,double>(home,m) {}
  forceinline double
  MeritMax::operator ()(const Space&, FloatView x, int) {
    return x.max();
  }

  // Size merit
  forceinline
  MeritSize::MeritSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb) {}
  forceinline
  MeritSize::MeritSize(Space& home, MeritSize& m)
    : MeritBase<FloatView,double>(home,m) {}
  forceinline double
  MeritSize::operator ()(const Space&, FloatView x, int) {
    return x.size();
  }

  // Degree over size merit
  forceinline
  MeritDegreeSize::MeritDegreeSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb) {}
  forceinline
  MeritDegreeSize::MeritDegreeSize(Space& home,
                                   MeritDegreeSize& m)
    : MeritBase<FloatView,double>(home,m) {}
  forceinline double
  MeritDegreeSize::operator ()(const Space&, FloatView x, int) {
    return x.size() / static_cast<double>(x.degree());
  }

  // AFC over size merit
  forceinline
  MeritAFCSize::MeritAFCSize(Space& home, const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb), afc(vb.afc()) {}
  forceinline
  MeritAFCSize::MeritAFCSize(Space& home, MeritAFCSize& m)
    : MeritBase<FloatView,double>(home,m), afc(m.afc) {}
  forceinline double
  MeritAFCSize::operator ()(const Space&, FloatView x, int) {
    return x.afc() / static_cast<double>(x.size());
  }
  forceinline bool
  MeritAFCSize::notice(void) const {
    return false;
  }
  forceinline void
  MeritAFCSize::dispose(Space&) {
    // Not needed
    afc.~AFC();
  }

  // Action over size merit
  forceinline
  MeritActionSize::MeritActionSize(Space& home,
                                   const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb), action(vb.action()) {}
  forceinline
  MeritActionSize::MeritActionSize(Space& home,
                                   MeritActionSize& m)
    : MeritBase<FloatView,double>(home,m), action(m.action) {}
  forceinline double
  MeritActionSize::operator ()(const Space&, FloatView x, int i) {
    return action[i] / static_cast<double>(x.size());
  }
  forceinline bool
  MeritActionSize::notice(void) const {
    return true;
  }
  forceinline void
  MeritActionSize::dispose(Space&) {
    action.~Action();
  }


  // CHB Q-score over size merit
  forceinline
  MeritCHBSize::MeritCHBSize(Space& home,
                             const VarBranch<Var>& vb)
    : MeritBase<FloatView,double>(home,vb), chb(vb.chb()) {}
  forceinline
  MeritCHBSize::MeritCHBSize(Space& home,
                             MeritCHBSize& m)
    : MeritBase<FloatView,double>(home,m), chb(m.chb) {}
  forceinline double
  MeritCHBSize::operator ()(const Space&, FloatView x, int i) {
    return chb[i] / static_cast<double>(x.size());
  }
  forceinline bool
  MeritCHBSize::notice(void) const {
    return true;
  }
  forceinline void
  MeritCHBSize::dispose(Space&) {
    chb.~CHB();
  }

}}}

// STATISTICS: float-branch
