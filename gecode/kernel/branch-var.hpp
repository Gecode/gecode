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
  typedef double (*BranchTbl)(const Space& home, double w, double b);

  /**
   * \brief Variable branching information
   * \ingroup TaskModelBranch
   */
  class VarBranch {
  protected:
    /// Tie-breaking limit function
    BranchTbl t;
    /// Random number generator
    Rnd r;
    /// Activity information
    Activity a;
    /// Merit function (generic function pointer)
    VoidFunction mf;
  public:
    /// Initialize with tie-break limit function \a t
    VarBranch(BranchTbl t);
    /// Initialize with random number generator \a r
    VarBranch(Rnd r);
    /// Initialize with activity \a a and tie-break limit function \a t
    VarBranch(Activity a, BranchTbl t);
    /// Initialize with merit function \a f and tie-break limit function \a t
    VarBranch(void (*f)(void), BranchTbl t);
    /// Return tie-break limit function
    BranchTbl tbl(void) const;
    /// Return random number generator
    Rnd rnd(void) const;
    /// Return activity
    Activity activity(void) const;
    /// Return merit function
    VoidFunction merit(void) const;
  };

  /// Combine variable selection criteria for tie-breaking
  template<class VarBranch>
  class TieBreak {
  public:
    /// Branching criteria to try in order
    VarBranch a, b, c, d;
    /// Initialize with variable selection criteria
    TieBreak(VarBranch a0 = VarBranch(),
             VarBranch b0 = VarBranch(),
             VarBranch c0 = VarBranch(),
             VarBranch d0 = VarBranch());
  };

  /** 
   * \defgroup TaskModelBranchTieBreak Tie-breaking for variable selection
   * 
   * \ingroup TaskModelBranch
   */
  //@{
  /// Combine variable selection criteria \a a and \a b for tie-breaking
  template<class VarBranch>
  TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b);
  /// Combine variable selection criteria \a a, \a b, and \a c for tie-breaking
  template<class VarBranch>
  TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c);
  /// Combine variable selection criteria \a a, \a b, \a c, and \a d for tie-breaking
  template<class VarBranch>
  TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c, VarBranch d);
  //@}

  // Variable branching
  forceinline 
  VarBranch::VarBranch(BranchTbl t0)
    : t(t0), mf(NULL) {}

  forceinline 
  VarBranch::VarBranch(Activity a0, BranchTbl t0)
    : t(t0), a(a0), mf(NULL) {
    if (!a.initialized())
      throw UninitializedActivity("VarBranch::VarBranch");
  }

  forceinline 
  VarBranch::VarBranch(Rnd r0)
    : t(NULL), r(r0), mf(NULL) {
    if (!r.initialized())
      throw UninitializedRnd("VarBranch::VarBranch");
  }

  forceinline 
  VarBranch::VarBranch(VoidFunction f, BranchTbl t0)
    : t(t0), mf(f) {}

  forceinline BranchTbl
  VarBranch::tbl(void) const {
    return t;
  }

  forceinline Rnd
  VarBranch::rnd(void) const {
    return r;
  }

  forceinline Activity
  VarBranch::activity(void) const {
    return a;
  }

  forceinline VoidFunction
  VarBranch::merit(void) const {
    return mf;
  }


  template<class VarBranch>
  forceinline
  TieBreak<VarBranch>::TieBreak(VarBranch a0,
                                VarBranch b0,
                                VarBranch c0,
                                VarBranch d0)
    : a(a0), b(b0), c(c0), d(d0) {}

  template<class VarBranch>
  forceinline TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b) {
    TieBreak<VarBranch> ab(a,b);
    return ab;
  }

  template<class VarBranch>
  forceinline TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c) {
    TieBreak<VarBranch> abc(a,b,c);
    return abc;
  }

  template<class VarBranch>
  forceinline TieBreak<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c, VarBranch d) {
    TieBreak<VarBranch> abcd(a,b,c,d);
    return abcd;
  }

}

// STATISTICS: kernel-branch
