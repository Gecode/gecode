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

#include <iostream>
#include <algorithm>

#include "iter.hh"

namespace Gecode { namespace Int { namespace Linear {

  class SupportSet {
  private:
    /// Number of bits per unsigned integer
    static const unsigned int bpui = sizeof(unsigned int) * 8;
    /// Array of bits
    unsigned int* bits;
  public:
    /// Initialize support set with cardinality \arg n
    void init(unsigned int n) {
      bits = Memory::bmalloc<unsigned int>((n / bpui) + 1);
      for (int i = (n / bpui) + 1; i--; )
	bits[i] = 0;
    }
    /// Record that there is support at position \arg i
    void support(unsigned int i) {
      unsigned int p = i / bpui;
      bits[p] |= 1 << (p-i);
    }
    /// Check whether position \arg i has support
    bool supported(unsigned int i) const {
      unsigned int p = i / bpui;
      return (bits[p] & (1 << (p-i))) != 0;
    }

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
      ResultIter(void) {}
      /// Initialize iterator
      void init(SupportSet* s0, IntView& x) {
	s = s0; 
	ViewRanges<IntView> r(x);
	i.init(r);
	p = 0;
	while (i() && !s->supported(p)) {
	  ++i; ++p;
	} 
      }
      /// Test whether more values are left
      bool operator()(void) const {
	return i();
      }
      /// Increment to next supported value
      void operator++(void) {
	do {
	  ++i; ++p;
	} while (i() && !s->supported(p));
      }
      /// Return current value
      int val(void) const {
	return i.val();
      }
    };
    
  public:
    /// Perform tell according to recorded support information on \arg x
    ModEvent tell(Space* home, IntView x) {
      /*
      ExecStatus es = ES_OK;
      if (size == x.size())
	return ES_OK;
      if (size == 0)
	return ES_FAILED;
      */
      ResultIter i;
      i.init(this,x);
      Iter::Values::ToRanges<ResultIter> r;
      r.init(i);
      return x.narrow(home,r);
    }

    void dispose(void) {
      Memory::free(bits);
    }

  };





  /// Base-class for support-based iterator
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
    void init(int a0, IntView& x0, double l0, double u0) {
      a=a0; x=x0; l=l0; u=u0;
      s.init(x.size());
    }
    /// Record value at current position as supported
    void support(void) {
      s.support(p);
    }
    /// Tell back new variable domain according to support found
    ModEvent tell(Space* home) {
      return s.tell(home,x);
    }
    /// Dispose memory for support set
    void dispose(void) {
      s.dispose();
    }
    void print(void) {
      std::cout << a << "*" << c << ", ";
    }

  };


  /// Support-based iterator for positive view
  class PosSupportIter : public SupportIter {
  private:
    /// Iterate ranges of integer view in increasing order
    IntVarImpFwd i;
  public:
    /// Reset iterator to beginning and adjust \arg d accordingly
    bool reset(double& d) {
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
    /// Adjust \arg d and return true if next value still works
    bool adjust(double& d) {
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
  };

  /// Support-based iterator for negative view
  class NegSupportIter : public SupportIter {
  private:
    /// Iterate ranges of integer view in decreasing order
    IntVarImpBwd i;
  public:
    /// Reset iterator to beginning and adjust \arg d accordingly
    bool reset(double& d) {
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

    /// Adjust \arg d and return true if next value still works
    bool adjust(double& d) {
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
  };

  class SupportChecker {
  private:
    int a;
    IntView x;
    SupportSet s;
  public:
    SupportChecker(int a0, IntView& x0) 
      : a(a0), x(x0) {
      s.init(x.size());
    }
    bool supported(double d) {
      //      /* FIXME
      DoubleScaleView v(a,x);
      std::cout << "Check " << d << " for a=" << a 
		<< " x=" << x << " (" << v.in(d) << "): ";
      //      */
      /*
      DoubleScaleView v(a,x);
      std::cout << "Check " << d << " for a=" << a 
		<< " x=" << x << ": ";
      */
      int da = static_cast<int>(ceil(d/a));
      // d must be a multiple of a
      if (a*static_cast<double>(da) != d) {
	//	std::cout << "NO (rounding)" << std::endl;
	return false;
      }
      if ((da < x.min()) || (da > x.max())) {
	//	std::cout << "NO (bounds)" << std::endl;
	return false;
      }
      ViewRanges<IntView> i(x);
      unsigned int p = 0;
      while (da > i.max()) {
	p += i.width(); ++i; 
      }
      if (da < i.min()) {
	//	std::cout << "NO (hole)" << std::endl;
	return false;
      }
      p += da-i.min();
      //      std::cout << "YES p=" << p << std::endl;
      s.support(p);
      return true;
    }
    void dispose(void) {
      s.dispose();
    }
    ModEvent tell(Space* home) {
      return s.tell(home,x);
    }
  };


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
    /*
      for (int i=0; i<x.size(); i++)
      std::cout << "+" << x[i].scale() << "*" << x[i].base();
      for (int i=0; i<y.size(); i++)
	std::cout << "-" << y[i].scale() << "*" << y[i].base();
      std::cout << " = " << c << std::endl;
      */
      
      // Normalize
      /*
      assert(x.size() + y.size() > 1);
      if (x.size() == 0) {
	// At least one negative view is needed
	std::swap(x,y); c = -c;
	assert(x.size() > 0);
      }
      */

      double d = -c;

      //      int n = x.size()-1;
      int n = x.size();
      int m = y.size();

      // Create support-base iterators for all but one guy
      GECODE_AUTOARRAY(PosSupportIter, xp, n);
      GECODE_AUTOARRAY(NegSupportIter, yp, m);
      // That guy becomes a support checker
      //      SupportChecker sc(x[n].scale(),x[n].base());

      // Initialize views for assignments
      {
	// double l = -x[n].min();
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
      {
	// Force reset of all iterators in first round
	int i = 0;
	int j = 0;

      next_i:
	// Reset all iterators for positive views and update d
	while (i<n) {
	  assert(j == 0);
	  if (!xp[i].reset(d))
	    goto prev_i;
	  i++;
	}
      next_j:
	// Reset all iterators for negative views and update d
	while (j<m) {
	  if (!yp[j].reset(d))
	    goto prev_j;
	  j++;
	}
	iter++;
	/*
	for (int is=0; is<n; is++)
	  xp[is].print();
	for (int js=0; js<m; js++)
	  yp[js].print();
	std::cout << std::endl;
	*/

	// Check whether current assignment is solution
	// if (sc.supported(-d)) {
	if (d == 0) {
	  //	  std::cout << "Solution:" << std::endl;
	  for (int is=0; is<n; is++)
	    xp[is].support();
	  for (int js=0; js<m; js++)
	    yp[js].support();
	}
      prev_j:
	// Try iterating to next assignment: negative views
	while (j>0) {
	  if (yp[j-1].adjust(d))
	    goto next_j;
	  j--;
	}
      prev_i:
	assert((j == 0) && (i == n));
	// Try iterating to next assignment: positive views
	while (i>0) {
	  if (xp[i-1].adjust(d))
	    goto next_i;
	  i--;
	}
      }

      //      std::cout << "Iterations: " << iter << std::endl;
      // Tell back new variable domains
      bool failed = false;
      /*
      if (me_failed(sc.tell(home))) {
	failed = true; goto dispose;
      }
      */
      for (int i=n; i--; )
	if (me_failed(xp[i].tell(home))) {
	  failed = true; goto dispose;
	}
      for (int j=m; j--; )
	if (me_failed(yp[j].tell(home))) {
	  failed = true; goto dispose;
	}
    dispose:
      /*
      bool assigned = x[n].assigned();
      sc.dispose();
      */
      bool assigned = true;
      for (int i=n; i--; ) {
	xp[i].dispose();
	if (!x[i].assigned())
	  assigned = false;
      }
      for (int j=m; j--; ) {
	yp[j].dispose();
	if (!y[j].assigned())
	  assigned = false;
      }
      if (failed)
	return ES_FAILED;
      //      if (assigned)
      //	return ES_SUBSUMED;
      return ES_FIX;
    }
    
}}}
