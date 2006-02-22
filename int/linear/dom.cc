/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2006
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "int/linear.hh"
#include "iter.hh"
#include "support/sort.hh"

namespace Gecode { namespace Int { namespace Linear {

  /**
   * \brief Set for support information
   *
   * Records supported positions of values such that with iteration
   * the supported values can be reconstructed.
   *
   */
  class SupportSet {
  private:
    /// Number of bits per unsigned integer
    static const unsigned int bpui = sizeof(unsigned int) * 8;
    /// Array of bits
    unsigned int* bits;
  public:
    /// Initialize support set with cardinality \arg n
    void init(unsigned int n);
    /// Record that there is support at position \arg i
    void support(unsigned int i);
    /// Check whether position \arg i has support
    bool supported(unsigned int i) const;

  private:
    /// Support-based iterator
    class ResultIter {
    protected:
      /// The support set used
      SupportSet* s;
      /// The values of the integer view
      Iter::Ranges::ToValues<ViewRanges<IntView> > i;
      /// The current position of the value
      unsigned int p;
    public:
      /// Default constructor
      ResultIter(void);
      /// Initialize iterator
      void init(SupportSet* s0, IntView& x);
      /// Test whether more values are left
      bool operator()(void) const;
      /// Increment to next supported value
      void operator++(void);
      /// Return current value
      int val(void) const;
    };
    
  public:
    /// Perform tell according to recorded support information on \arg x
    ModEvent tell(Space* home, IntView x);
    /// Release memory
    void dispose(void);
  };

  /**
   * \name Base-class for support-based iterator
   *
   */
  class SupportIter {
  protected:
    /// Integer coefficient for view
    int           a;
    /// Integer view
    IntView       x;
    /// Set of support for values in x
    SupportSet    s;
    /// Current value
    int           c;
    /// Position of current value
    unsigned int  p;
    /// Lower bound information for value
    double        l;
    /// Upper bound information for value
    double        u;
  public:
    /// Initialize view
    void init(int a0, IntView& x0, double l0, double u0);
    /// Record value at current position as supported
    void support(void);
    /// Tell back new variable domain according to support found
    ModEvent tell(Space* home);
    /// Dispose memory for support set
    void dispose(void);
  };


  /**
   * \brief Support-based iterator for positive view
   *
   */
  class PosSupportIter : public SupportIter {
  private:
    /// Iterate ranges of integer view in increasing order
    IntVarImpFwd i;
  public:
    /// Reset iterator to beginning and adjust \arg d accordingly
    bool reset(double& d);
    /// Adjust \arg d and return true if next value still works
    bool adjust(double& d);
  };


  /**
   * \brief Support-based iterator for negative view
   *
   */
  class NegSupportIter : public SupportIter {
  private:
    /// Iterate ranges of integer view in decreasing order
    IntVarImpBwd i;
  public:
    /// Reset iterator to beginning and adjust \arg d accordingly
    bool reset(double& d);
    /// Adjust \arg d and return true if next value still works
    bool adjust(double& d);
  };


  /*
   * Support set
   *
   */
  forceinline void 
  SupportSet::init(unsigned int n) {
    bits = Memory::bmalloc<unsigned int>((n / bpui) + 1);
    for (unsigned int i = (n / bpui) + 1; i--; )
      bits[i] = 0;
  }
  forceinline void 
  SupportSet::support(unsigned int i) {
    unsigned int p = i / bpui;
    bits[p] |= 1 << (p-i);
  }
  forceinline bool 
  SupportSet::supported(unsigned int i) const {
    unsigned int p = i / bpui;
    return (bits[p] & (1 << (p-i))) != 0;
  }

  forceinline
  SupportSet::ResultIter::ResultIter(void) {}
  forceinline void 
  SupportSet::ResultIter::init(SupportSet* s0, IntView& x) {
    s = s0; 
    ViewRanges<IntView> r(x);
    i.init(r); p = 0;
    while (i() && !s->supported(p)) {
      ++i; ++p;
    } 
  }
  forceinline bool 
  SupportSet::ResultIter::operator()(void) const {
    return i();
  }
  forceinline void 
  SupportSet::ResultIter::operator++(void) {
    do {
      ++i; ++p;
    } while (i() && !s->supported(p));
  }
  forceinline int 
  SupportSet::ResultIter::val(void) const {
    return i.val();
  }

  ModEvent 
  SupportSet::tell(Space* home, IntView x) {
    const unsigned int none = 0;
    const unsigned int full = ~static_cast<unsigned int>(0);
    unsigned int n = x.size() / bpui;
    if (bits[0] == none) {
      // Maybe all bits are zero
      for (unsigned int i=n+1; i-- > 1; )
	if (bits[i] != none)
	  goto tell;
      return ME_INT_FAILED;
    }
    if (bits[0] == full) {
      // Maybe all bits are one
      for (unsigned int i=n; i-- > 1; )
	if (bits[i] != full)
	  goto tell;
      for (unsigned int i=n*bpui; i<x.size(); i++)
	if (!supported(i))
	  goto tell;
      return ME_INT_NONE;
    }
  tell:
    ResultIter i;
    i.init(this,x);
    Iter::Values::ToRanges<ResultIter> r;
    r.init(i);
    return x.narrow(home,r);
  }

  forceinline void 
  SupportSet::dispose(void) {
    Memory::free(bits);
  }


  /*
   * Base-class for support-based iterator
   *
   */
  forceinline void 
  SupportIter::init(int a0, IntView& x0, double l0, double u0) {
    a=a0; x=x0; l=l0; u=u0;
    s.init(x.size());
  }
  forceinline void 
  SupportIter::support(void) {
    s.support(p);
  }
  forceinline ModEvent
  SupportIter::tell(Space* home) {
    return s.tell(home,x);
  }
  forceinline void 
  SupportIter::dispose(void) {
    s.dispose();
  }


  /*
   * Support-based iterator for positive view
   *
   */
  forceinline bool 
  PosSupportIter::reset(double& d) {
    // Way too small, no value can make it big enough
    if (d + static_cast<double>(a)*x.max() < u)
      return false;
    // Restart iterator and position of values
    i.init(x.variable()); p = 0;
    // Skip all ranges which are too small
    while (d + static_cast<double>(a)*i.max() < u) {
      p += i.width(); ++i;
    }
    // There is at least one range left (check of max)
    assert(i());
    // Initialize current range and adjust value
    c = i.min();
    // Skip all values which are too small
    while (d + static_cast<double>(a)*c < u) {
      p++; c++;
    }      
    // Adjust to new value
    d += static_cast<double>(a) * c;
    return true;
  }
  forceinline bool 
  PosSupportIter::adjust(double& d) {
    // Current value
    double v = static_cast<double>(a) * c;
    // Subtract current value from d
    d -= v;
    // Move to next position (number of value)
    p++;
    // Still in the same range
    if (c < i.max()) {
      // Decrement current values
      c += 1; v += a;
    } else {
      // Go to next range
      ++i;
      if (!i())
	return false;
      c = i.min(); v = static_cast<double>(a) * c;
    }
    // Is d with the current value too large?
    if (d + v > l)
      return false;
    // Update d
    d += v; 
    return true;
  }


  /*
   * Support-based iterator for negative view
   *
   */
  forceinline bool 
  NegSupportIter::reset(double& d) {
    // Way too small, no value can make it big enough
    if (d + static_cast<double>(a)*x.min() < u)
      return false;
    // Restart iterator and position of values
    i.init(x.variable()); p = x.size()-1;
    // Skip all ranges which are too small
    while (d + static_cast<double>(a)*i.min() < u) {
      p -= i.width(); ++i;
    }
    // There is at least one range left (check of max)
    assert(i());
    // Initialize current range
    c = i.max();
    // Skip all values which are too small
    while (d + static_cast<double>(a)*c < u) {
      p--; c--;
    }      
    // Adjust to new value
    d += static_cast<double>(a) * c;
    return true;
  }
  forceinline bool 
  NegSupportIter::adjust(double& d) {
    // Current value
    double v = static_cast<double>(a) * c;
    // Subtract current value from d
    d -= v;
    // Move to next position (number of value)
    p--;
    // Still in the same range
    if (c > i.min()) {
      // Decrement current values
      c -= 1; v -= a;
    } else {
      // Go to next range
      ++i;
      if (!i())
	return false;
      c = i.max(); v = static_cast<double>(a) * c;
    }
    // Is d with the current value too large?
    if (d + v > l)
      return false;
    // Update d
    d += v; 
    return true;
  }



  /*
   * The domain-consisten equality propagator
   *
   */
  Actor* 
  DomEq::copy(Space* home, bool share) {
    return new (home) DomEq(home,share,*this);
  }
  PropCost 
  DomEq::cost(void) const {
    return PC_CRAZY_HI;
  }
  
  ExecStatus 
  DomEq::post(Space* home,
	      ViewArray<DoubleScaleView>& x, 
	      ViewArray<DoubleScaleView>& y, 
	      double c) {
    (void) new (home) DomEq(home,x,y,c);
    return ES_OK;
  }
   
  ExecStatus 
  DomEq::propagate(Space* home) {
    // Value of equation for partial assignment
    double d = -c;

    int n = x.size();
    int m = y.size();

    // Create support-base iterators
    GECODE_AUTOARRAY(PosSupportIter, xp, n);
    GECODE_AUTOARRAY(NegSupportIter, yp, m);

    // Initialize views for assignments
    {
      double l = 0;
      double u = 0;
      for (int j=m; j--; ) {
	yp[j].init(-y[j].scale(),y[j].base(),l,u); 
	l += y[j].max(); u += y[j].min();
      }
      for (int i=n; i--; ) {
	xp[i].init(x[i].scale(),x[i].base(),l,u); 
	l -= x[i].min(); u -= x[i].max();
      }
    }
    
    unsigned int iter = 0;
    // Collect support information by iterating assignments
    {
      // Force reset of all iterators in first round
      int i = 0;
      int j = 0;
      
    next_i:
      // Reset all iterators for positive views and update d
      while (i<n) {
	if (!xp[i].reset(d)) goto prev_i;
	i++;
      }
    next_j:
      // Reset all iterators for negative views and update d
      while (j<m) {
	if (!yp[j].reset(d)) goto prev_j;
	j++;
      }
      iter++;
      // Check whether current assignment is solution
      if (d == 0) {
	// Record support
	for (int is=n; is--; ) xp[is].support();
	for (int js=m; js--; ) yp[js].support();
      }
    prev_j:
      // Try iterating to next assignment: negative views
      while (j>0) {
	if (yp[j-1].adjust(d)) goto next_j;
	j--;
      }
    prev_i:
      // Try iterating to next assignment: positive views
      while (i>0) {
	if (xp[i-1].adjust(d)) goto next_i;
	i--;
      }
    }
    
    //    std::cout << iter << ", ";
    // Tell back new variable domains
    ExecStatus es = ES_SUBSUMED;
    for (int i=n; i--; ) {
      ModEvent me = xp[i].tell(home);
      if (me_failed(me)) {
	es = ES_FAILED; goto dispose;
      }
      //      if (me != ME_INT_VAL)
      if (!x[i].assigned())
	es = ES_FIX;
    }
    for (int j=m; j--; ) {
      ModEvent me = yp[j].tell(home);
      if (me_failed(me)) {
	es = ES_FAILED; goto dispose;
      }
      if (!y[j].assigned())
	//      if (me != ME_INT_VAL)
	es = ES_FIX;
    }
    
    // Release memory
  dispose:
    for (int i=n; i--; )
      xp[i].dispose();
    for (int j=m; j--; )
      yp[j].dispose();
    return es;
  }
    
}}}
