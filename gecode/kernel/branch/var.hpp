/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <functional>

namespace Gecode {

  /**
   * \brief Tie-break limit function
   *
   * Here the value \a w is the worst and \b is the best merit value
   * found. The function must return the merit value that is considered
   * the limit for breaking ties.
   *
   * \ingroup TaskModelBranch
   */
  typedef std::function<double(const Space& home, double w, double b)>
    BranchTbl;

  /**
   * \brief Variable branching information
   * \ingroup TaskModelBranch
   */
  template<class Var>
  class VarBranch {
  public:
    /// Corresponding merit function
    typedef typename BranchTraits<Var>::Merit MeritFunction;
  protected:
    /// Tie-breaking limit function
    BranchTbl _tbl;
    /// Random number generator
    Rnd _rnd;
    /// Decay information for AFC and action
    double _decay;
    /// AFC information
    AFC _afc;
    /// Action information
    Action _act;
    /// CHB information
    CHB _chb;
    /// Merit function
    MeritFunction _mf;
  public:
    /// Initialize
    VarBranch(void);
    /// Initialize with tie-break limit function \a t
    VarBranch(BranchTbl t);
    /// Initialize with random number generator \a r
    VarBranch(Rnd r);
    /// Initialize with decay factor \a d and tie-break limit function \a t
    VarBranch(double d, BranchTbl t);
    /// Initialize with AFC \a a and tie-break limit function \a t
    VarBranch(AFC a, BranchTbl t);
    /// Initialize with action \a a and tie-break limit function \a t
    VarBranch(Action a, BranchTbl t);
    /// Initialize with CHB \a c and tie-break limit function \a t
    VarBranch(CHB c, BranchTbl t);
    /// Initialize with merit function \a f and tie-break limit function \a t
    VarBranch(MeritFunction f, BranchTbl t);
    /// Return tie-break limit function
    BranchTbl tbl(void) const;
    /// Return random number generator
    Rnd rnd(void) const;
    /// Return decay factor
    double decay(void) const;
    /// Return AFC
    AFC afc(void) const;
    /// Set AFC to \a a
    void afc(AFC a);
    /// Return action
    Action action(void) const;
    /// Set action to \a a
    void action(Action a);
    /// Return CHB
    CHB chb(void) const;
    /// Set CHB to \a chb
    void chb(CHB chb);
    /// Return merit function
    MeritFunction merit(void) const;
  };

  // Variable branching
  template<class Var>
  inline
  VarBranch<Var>::VarBranch(void)
    : _tbl(nullptr), _decay(1.0) {}

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(BranchTbl t)
    : _tbl(t), _decay(1.0) {}

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(double d, BranchTbl t)
    : _tbl(t), _decay(d) {}

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(AFC a, BranchTbl t)
    : _tbl(t), _decay(1.0), _afc(a) {
    if (!_afc)
      throw UninitializedAFC("VarBranch<Var>::VarBranch");
  }

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(Action a, BranchTbl t)
    : _tbl(t), _decay(1.0), _act(a) {
    if (!_act)
      throw UninitializedAction("VarBranch<Var>::VarBranch");
  }

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(CHB c, BranchTbl t)
    : _tbl(t), _decay(1.0), _chb(c) {
    if (!_chb)
      throw UninitializedCHB("VarBranch<Var>::VarBranch");
  }

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(Rnd r)
    : _tbl(nullptr), _rnd(r), _decay(1.0) {
    if (!_rnd)
      throw UninitializedRnd("VarBranch<Var>::VarBranch");
  }

  template<class Var>
  inline
  VarBranch<Var>::VarBranch(MeritFunction f, BranchTbl t)
    : _tbl(t), _decay(1.0), _mf(f) {}

  template<class Var>
  inline BranchTbl
  VarBranch<Var>::tbl(void) const {
    return _tbl;
  }

  template<class Var>
  inline Rnd
  VarBranch<Var>::rnd(void) const {
    return _rnd;
  }

  template<class Var>
  inline double
  VarBranch<Var>::decay(void) const {
    return _decay;
  }

  template<class Var>
  inline AFC
  VarBranch<Var>::afc(void) const {
    return _afc;
  }

  template<class Var>
  inline void
  VarBranch<Var>::afc(AFC a) {
    _afc=a;
  }

  template<class Var>
  inline Action
  VarBranch<Var>::action(void) const {
    return _act;
  }

  template<class Var>
  inline void
  VarBranch<Var>::action(Action a) {
    _act=a;
  }

  template<class Var>
  inline CHB
  VarBranch<Var>::chb(void) const {
    return _chb;
  }

  template<class Var>
  inline void
  VarBranch<Var>::chb(CHB chb) {
    _chb=chb;
  }

  template<class Var>
  inline typename VarBranch<Var>::MeritFunction
  VarBranch<Var>::merit(void) const {
    return _mf;
  }

}

// STATISTICS: kernel-branch
