/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *     Mikael Lagerkvist, 2006
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

namespace Gecode { namespace Int { namespace Channel {

  /**
   * \brief Combine view with information for value propagation
   *
   */
  template<class View>
  class ValInfo {
  public:
    /// The view
    View view;
    /// Whether it has been propagated that view is assigned
    bool a;
    /// Initialize
    void init(View x, int n);
    /// Update during cloning
    void update(Space& home, bool share, ValInfo<View>& vi);
    /// Check whether propagation for assignment is to be done
    bool doval(void) const;
    /// Check whether propagation for domain is to be done
    bool dodom(void) const;
    /// Record that view got assigned
    void assigned(void);
    /// Record that one value got removed
    void removed(int i);
    /// Update the cardinality and bounds information after pruning
    void done(void);
  };

  template<class View>
  forceinline void
  ValInfo<View>::init(View x, int) {
    view = x; a = false;
  }

  template<class View>
  forceinline void
  ValInfo<View>::update(Space& home, bool share, ValInfo<View>& vi) {
    view.update(home,share,vi.view); a = vi.a;
  }

  template<class View>
  forceinline bool
  ValInfo<View>::doval(void) const {
    return !a && view.assigned();
  }

  template<class View>
  forceinline bool
  ValInfo<View>::dodom(void) const {
    return false;
  }

  template<class View>
  forceinline void
  ValInfo<View>::assigned(void) {
    a = true;
  }

  template<class View>
  forceinline void
  ValInfo<View>::removed(int) {}

  template<class View>
  forceinline void
  ValInfo<View>::done(void) {}


  // Propagate assigned views for x
  template<class View, class Info>
  ExecStatus
  doprop_val(Space& home, int n, Info* x, Info* y,
             int& n_na, ProcessStack& xa, ProcessStack& ya) {
    do {
      int i = xa.pop();
      int j = x[i].view.val();
      // Assign the y variable to i (or test if already assigned!)
      {
        ModEvent me = y[j].view.eq(home,i);
        if (me_failed(me))
          return ES_FAILED;
        // Record that y[j] has been assigned and must be propagated
        if (me_modified(me))
          ya.push(j);
      }
      // Prune the value j from all x variables
      for (int k=i; k--; ) {
        ModEvent me = x[k].view.nq(home,j);
        if (me_failed(me))
          return ES_FAILED;
        if (me_modified(me)) {
          if (me == ME_INT_VAL) {
            // Record that x[k] has been assigned and must be propagated
            xa.push(k);
          } else {
            // One value has been removed and this removal does not need
            // to be propagated again: after all y[j] = i, so it holds
            // that y[j] != k.
            x[k].removed(j);
          }
        }
      }
      // The same for the other views
      for (int k=i+1; k<n; k++) {
        ModEvent me = x[k].view.nq(home,j);
        if (me_failed(me))
          return ES_FAILED;
        if (me_modified(me)) {
          if (me == ME_INT_VAL) {
            // Record that x[k] has been assigned and must be propagated
            xa.push(k);
          } else {
            // One value has been removed and this removal does not need
            // to be propagated again: after all y[j] = i, so it holds
            // that y[j] != k.
            x[k].removed(j);
          }
        }
      }
      x[i].assigned(); n_na--;
    } while (!xa.empty());
    return ES_OK;
  }

  // Just do a test whether a call to the routine is necessary
  template<class View, class Info>
  forceinline ExecStatus
  prop_val(Space& home, int n, Info* x, Info* y,
           int& n_na, ProcessStack& xa, ProcessStack& ya) {
    if (xa.empty())
      return ES_OK;
    return doprop_val<View,Info>(home,n,x,y,n_na,xa,ya);
  }

  /*
   * The actual propagator
   *
   */
  template<class View, bool shared>
  forceinline
  Val<View,shared>::Val(Space& home, int n, ValInfo<View>* xy)
    : Base<ValInfo<View>,PC_INT_VAL>(home,n,xy) {}

  template<class View, bool shared>
  forceinline
  Val<View,shared>::Val(Space& home, bool share, Val<View,shared>& p)
    : Base<ValInfo<View>,PC_INT_VAL>(home,share,p) {}

  template<class View, bool shared>
  Actor*
  Val<View,shared>::copy(Space& home, bool share) {
    return new (home) Val<View,shared>(home,share,*this);
  }

  template<class View, bool shared>
  ExecStatus
  Val<View,shared>::propagate(Space& home, const ModEventDelta&) {
    Region r(home);
    ProcessStack xa(r,n);
    ProcessStack ya(r,n);

    ValInfo<View>* x = xy;
    ValInfo<View>* y = xy+n;

    // Scan x and y for assigned but not yet propagated views
    for (int i = n; i--; ) {
      if (x[i].doval()) xa.push(i);
      if (y[i].doval()) ya.push(i);
    }

    do {
      // Propagate assigned views for x
      GECODE_ES_CHECK((prop_val<View,ValInfo<View> >(home,n,x,y,n_na,xa,ya)));
      // Propagate assigned views for y
      GECODE_ES_CHECK((prop_val<View,ValInfo<View> >(home,n,y,x,n_na,ya,xa)));
      assert(ya.empty());
    } while (!xa.empty());

    if (n_na == 0)
      return ES_SUBSUMED(*this,home);
    return shared ? ES_NOFIX : ES_FIX;
  }

  template<class View, bool shared>
  ExecStatus
  Val<View,shared>::post(Space& home, int n, ValInfo<View>* xy) {
    assert(n > 0);
    if (n == 1) {
      GECODE_ME_CHECK(xy[0].view.eq(home,0));
      GECODE_ME_CHECK(xy[1].view.eq(home,0));
      return ES_OK;
    }
    for (int i=2*n; i--; ) {
      GECODE_ME_CHECK(xy[i].view.gq(home,0));
      GECODE_ME_CHECK(xy[i].view.le(home,n));
    }
    (void) new (home) Val<View,shared>(home,n,xy);
    return ES_OK;
  }

}}}

// STATISTICS: int-prop

