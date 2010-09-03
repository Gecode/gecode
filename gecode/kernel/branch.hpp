/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <ctime>

namespace Gecode {

  /**
   * \defgroup TaskModelBranch Generic branching support
   *
   * Support for options to branchings, tie-breaking, filter functions, 
   * and branchings that are independent of a particular variable domain.
   *
   * \ingroup TaskModel
   */

  /** \brief Branch filter function type
   * \ingroup TaskModelBranch
   */
  typedef bool (*BranchFilter)(const Space& home, int i, const Var& x);

  /** \brief Variable branch options
   * \ingroup TaskModelBranch
   */
  class VarBranchOptions {
  public:
    /// Branch filter function
    BranchFilter bf;
    /// Seed for random variable selection
    unsigned int seed;
    /// Default options
    GECODE_KERNEL_EXPORT static const VarBranchOptions def;
    /// Initialize with default values
    VarBranchOptions(BranchFilter bf0=NULL);
    /// Return object with time-based seed value
    static VarBranchOptions time(BranchFilter bf=NULL);
  };

  /** \brief Value branch options
   * \ingroup TaskModelBranch
   */
  class ValBranchOptions {
  public:
    /// Seed for random value selection
    unsigned int seed;
    /// Default options
    GECODE_KERNEL_EXPORT static const ValBranchOptions def;
    /// Initialize with default values
    ValBranchOptions(void);
    /// Return object with time-based seed value
    static ValBranchOptions time(void);
  };


  /// Combine variable selection criteria for tie-breaking
  template<class VarBranch>
  class TieBreakVarBranch {
  public:
    /// Branching criteria to try in order
    VarBranch a, b, c, d;
    /// Initialize with variable selection criteria
    TieBreakVarBranch(VarBranch a0 = static_cast<VarBranch>(0),
                      VarBranch b0 = static_cast<VarBranch>(0),
                      VarBranch c0 = static_cast<VarBranch>(0),
                      VarBranch d0 = static_cast<VarBranch>(0));
  };

  /// Combine variable branch options for tie-breaking
  class TieBreakVarBranchOptions {
  public:
    /// Options for branching criteria to try in order
    VarBranchOptions a, b, c, d;
    /// Default options
    GECODE_KERNEL_EXPORT static const TieBreakVarBranchOptions def;
    /// Initialize with variable selection criteria
    TieBreakVarBranchOptions(const VarBranchOptions& a0
                             = VarBranchOptions::def,
                             const VarBranchOptions& b0
                             = VarBranchOptions::def,
                             const VarBranchOptions& c0
                             = VarBranchOptions::def,
                             const VarBranchOptions& d0
                             = VarBranchOptions::def);
  };

  /** 
   * \defgroup TaskModelBranchTieBreak Tie-breaking for variable selection
   * 
   * \ingroup TaskModelBranch
   */
  //@{
  /// Combine variable selection criteria \a a and \a b for tie-breaking
  template<class VarBranch>
  TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b);
  /// Combine variable selection criteria \a a, \a b, and \a c for tie-breaking
  template<class VarBranch>
  TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c);
  /// Combine variable selection criteria \a a, \a b, \a c, and \a d for tie-breaking
  template<class VarBranch>
  TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c, VarBranch d);
  /// Combine variable branch options \a a and \a b for tie-breaking
  TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b);
  /// Combine variable branch options \a a, \a b, and \a c for tie-breaking
  TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b, VarBranchOptions c);
  /// Combine variable branch options \a a, \a b, \a c, and \a d for tie-breaking
  TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b, VarBranchOptions c,
           VarBranchOptions d);
  //@}

  /** 
   * \defgroup TaskModelBranchExec Branch with a function
   * 
   * This does not really branch (it just offers a single alternative) but
   * executes a single function during branching. A typical
   * application is to post more constraints after another brancher 
   * has finished.
   *
   * \ingroup TaskModelBranch
   */
  //@{
  /// Call the function \a f (with the current space as argument) for branching
  GECODE_KERNEL_EXPORT void
  branch(Home home, void (*f)(Space& home));
  //@}


  // Variable branch options
  forceinline
  VarBranchOptions::VarBranchOptions(BranchFilter bf0) 
    : bf(bf0), seed(0) {}

  forceinline VarBranchOptions
  VarBranchOptions::time(BranchFilter bf) {
    VarBranchOptions o(bf); 
    o.seed=static_cast<unsigned int>(::time(NULL));
    return o;
  }

  // Value branch options
  forceinline
  ValBranchOptions::ValBranchOptions(void) : seed(0) {}

  forceinline ValBranchOptions
  ValBranchOptions::time(void) {
    ValBranchOptions o; o.seed=static_cast<unsigned int>(::time(NULL));
    return o;
  }


  /*
   * Combine variable selection criteria for tie-breaking
   */
  template<class VarBranch>
  forceinline
  TieBreakVarBranch<VarBranch>::TieBreakVarBranch(VarBranch a0,
                                                  VarBranch b0,
                                                  VarBranch c0,
                                                  VarBranch d0)
    : a(a0), b(b0), c(c0), d(d0) {}

  template<class VarBranch>
  forceinline TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b) {
    TieBreakVarBranch<VarBranch> ab(a,b);
    return ab;
  }

  template<class VarBranch>
  forceinline TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c) {
    TieBreakVarBranch<VarBranch> abc(a,b,c);
    return abc;
  }

  template<class VarBranch>
  forceinline TieBreakVarBranch<VarBranch>
  tiebreak(VarBranch a, VarBranch b, VarBranch c, VarBranch d) {
    TieBreakVarBranch<VarBranch> abcd(a,b,c,d);
    return abcd;
  }

  /*
   * Combine branch options for tie-breaking
   */
  forceinline
  TieBreakVarBranchOptions::
  TieBreakVarBranchOptions(const VarBranchOptions& a0,
                           const VarBranchOptions& b0,
                           const VarBranchOptions& c0,
                           const VarBranchOptions& d0)
    : a(a0), b(b0), c(c0), d(d0) {}

  forceinline TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b) {
    TieBreakVarBranchOptions ab(a,b);
    return ab;
  }

  forceinline TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b, VarBranchOptions c) {
    TieBreakVarBranchOptions abc(a,b,c);
    return abc;
  }

  forceinline TieBreakVarBranchOptions
  tiebreak(VarBranchOptions a, VarBranchOptions b, VarBranchOptions c,
           VarBranchOptions d) {
    TieBreakVarBranchOptions abcd(a,b,c,d);
    return abcd;
  }

}

// STATISTICS: kernel-branch
