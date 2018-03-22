/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

  /// Function type for branch filter functions
  template<class Var>
  using BranchFilter = std::function<bool(const Space& home, 
                                          Var x, int i)>;
 
  /// Class storing a branch filter function
  template<class View>
  class BrancherFilter {
  public:
    /// The corresponding variable type
    typedef typename View::VarType Var;
  protected:
    SharedData<BranchFilter<Var>> f;
  public:
    /// Initialize
    BrancherFilter(BranchFilter<Var> bf);
    /// Initialize during cloning
    BrancherFilter(BrancherFilter& bf);
    /// Whether filtering is enabled
    operator bool(void) const;
    /// Invoke filter function
    bool operator ()(const Space& home, View x, int i) const;
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete object
    void dispose(Space& home);
  };

  /// Class withot a branch filter function
  template<class View>
  class BrancherNoFilter {
  public:
    /// The corresponding variable type
    typedef typename View::VarType Var;
  public:
    /// Initialize
    BrancherNoFilter(BranchFilter<Var> bf);
    /// Initialize during cloning
    BrancherNoFilter(BrancherNoFilter& bf);
    /// Whether filtering is enabled
    operator bool(void) const;
    /// Invoke filter function
    bool operator ()(const Space& home, View x, int i) const;
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete object
    void dispose(Space& home);
  };


  template<class View>
  forceinline
  BrancherFilter<View>::BrancherFilter(BranchFilter<Var> bf) : f(bf) {
    if (!bf)
      throw Gecode::InvalidFunction("BrancherFilter::BrancherFilter");
  }
  
  template<class View>
  forceinline
  BrancherFilter<View>::BrancherFilter(BrancherFilter<View>& bf)
    : f(bf.f) {
  }

  template<class View>
  forceinline
  BrancherFilter<View>::operator bool(void) const {
    return true;
  }

  template<class View>
  forceinline bool
  BrancherFilter<View>::operator ()(const Space& home, View x, int i) const {
    GECODE_VALID_FUNCTION(f());
    Var xv(x.varimp());
    return f()(home,xv,i);
  }

  template<class View>
  forceinline bool
  BrancherFilter<View>::notice(void) const {
    return true;
  }

  template<class View>
  forceinline void
  BrancherFilter<View>::dispose(Space&) {
    f.~SharedData<BranchFilter<Var>>();
  }


  template<class View>
  forceinline
  BrancherNoFilter<View>::BrancherNoFilter(BranchFilter<Var> bf) {
    assert(!bf);
    (void) bf;
  }
  
  template<class View>
  forceinline
  BrancherNoFilter<View>::BrancherNoFilter(BrancherNoFilter<View>&) {}

  template<class View>
  forceinline
  BrancherNoFilter<View>::operator bool(void) const {
    return false;
  }

  template<class View>
  forceinline bool
  BrancherNoFilter<View>::operator ()(const Space&, View, int) const {
    return true;
  }
  template<class View>
  forceinline bool
  BrancherNoFilter<View>::notice(void) const {
    return false;
  }

  template<class View>
  forceinline void
  BrancherNoFilter<View>::dispose(Space&) {
  }

}

// STATISTICS: kernel-branch
