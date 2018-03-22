/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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
   * \brief Value branching information
   * \ingroup TaskModelBranch
   */
  template<class Var>
  class ValBranch {
  public:
    /// The corresponding branching value function
    typedef typename BranchTraits<Var>::Val BranchVal;
    /// The corresponding branching commit function
    typedef typename BranchTraits<Var>::Commit BranchCommit;
  protected:
    /// Random number generator
    Rnd r;
    /// Value function
    BranchVal vf;
    /// Commit function
    BranchCommit cf;
  public:
    /// Initialize
    ValBranch(void);
    /// Initialize with random number generator \a r
    ValBranch(Rnd r);
    /// Initialize with value function \a v and commit function \a c
    ValBranch(BranchVal v, BranchCommit c);
    /// Return random number generator
    Rnd rnd(void) const;
    /// Return value function
    BranchVal val(void) const;
    /// Return commit function
    BranchCommit commit(void) const;
  };


  template<class Var>
  inline
  ValBranch<Var>::ValBranch(void)
    : vf(nullptr), cf(nullptr) {}

  template<class Var>
  inline
  ValBranch<Var>::ValBranch(Rnd r0)
    : r(r0), vf(nullptr), cf(nullptr) {
    if (!r)
      throw UninitializedRnd("ValBranch::ValBranch");
  }

  template<class Var>
  inline
  ValBranch<Var>::ValBranch(BranchVal v, BranchCommit c)
    : vf(v), cf(c) {}

  template<class Var>
  inline Rnd
  ValBranch<Var>::rnd(void) const {
    return r;
  }

  template<class Var>
  inline typename ValBranch<Var>::BranchVal
  ValBranch<Var>::val(void) const {
    return vf;
  }

  template<class Var>
  inline typename ValBranch<Var>::BranchCommit
  ValBranch<Var>::commit(void) const {
    return cf;
  }

}

// STATISTICS: kernel-branch
