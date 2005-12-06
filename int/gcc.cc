/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004
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

#include "int/gcc.hh"
#include "int/linear.hh"
#include "int/distinct.hh"

namespace Gecode { namespace Int { namespace GCC {

  /**
   * \brief Check whether gcc can be rewritten to distinct
   * 
   * If the number of available values equals \f$ |x| \f$, 
   * we can rewrite gcc to distinct if every value occurs exactly once.
   * If the number of available values is greater than \f$ |x| \f$, 
   * we can rewrite gcc to distinct if every value occurs at least zero times
   * and atmost once, otherwise, there is no rewriting possible. 
   */

  template<class Card>
  forceinline bool 
  check_alldiff(int n, Card& k){
    int left     = 0;
    int right    = k.size() - 1;
    bool alldiff = true;

    if (k.size() == 1) {
      return (n == 1 && k[0].min() == 1 && k[0].max() == 1);
    }
    if (n == k.size()) {
      while (left < right) {
	alldiff &= (k[left].max()  == 1 && 
		    k[left].min()  == 1 &&
		    k[right].max() == 1 &&
		    k[left].max()  == 1);
	if (!alldiff) {
	  break;
	}

	left++;
	right--;
      }
    } else {
      if (n < k.size()) {
	while (left < right) {
	  alldiff &= (k[left].max()  == 1 && 
		      k[left].min()  == 0 &&
		      k[right].max() == 1 &&
		      k[left].max()  == 0);
	  if (!alldiff) {
	    break;
	  }
	  left++;
	  right--;
	}
      } else {
	return false;
      }
    }
    return alldiff;
  }

  /**
   * \brief %Set the index of every variable to its initial position in \a x. 
   */ 
  template <class View>
  forceinline void
  x_setidx(ViewArray<View>& x) {
    for (int i = x.size(); i--; ) {
      x[i].index(i);
      x[i].oldindex(i);
    }
  }

  /**
   * \brief Compute the cardinality of the union of all variable domains in \a x.
   *
   */
  template <class View>
  forceinline int
  x_card(ViewArray<View>& x) {

    int n = x.size();

    GECODE_AUTOARRAY(ViewRanges<View>, xrange, n);
    for (int i = n; i--; ){
      ViewRanges<View> iter(x[i]);
      xrange[i] = iter;
    }
    
    Gecode::Iter::Ranges::NaryUnion<ViewRanges<View> > drl(&xrange[0], x.size());
    int r = 0;
    for ( ; drl(); ++drl){
      for (int v = drl.min(); v <=drl.max(); v++) {
	r ++;
      }
    }
    return r;
  }
  
  /**
   * \brief Initialize the cardinalities for the values in \a k.
   *
   */
  
  template <class Card, class View>
  forceinline void
  initcard(Space* home, ViewArray<View>& x, Card& k, int lb, int ub) {
    GECODE_AUTOARRAY(ViewRanges<View>, xrange, x.size());
    for (int i = x.size(); i--; ){
      ViewRanges<View> iter(x[i]);
      xrange[i] = iter;
    }
    
    Iter::Ranges::NaryUnion<ViewRanges<View> > drl(&xrange[0], x.size());
    int idx = 0;
    for ( ; drl(); ++drl){
      for (int v = drl.min(); v <= drl.max(); v++){
	k[idx].init(home, lb, ub, v);
	idx++;
      }
    }
  }

  /**
   * \brief Reset already existing cardinalities to zero. 
   *
   */  

  template <class Card, class View, bool isView>
  forceinline void
  setcard(Space* home, ViewArray<View>& x, Card& k, int xmin, int xmax) {

    int idx = 0;
    for (int v = xmin; v <= xmax; v++) {
      k[idx].card(v);
      k[idx].counter(0);
      idx++;
    }
    
    bool assigned = true;
    for (int i = x.size(); i--; ) {
      assigned &= x[i].assigned();
    }

    if (assigned) {
      // check validity
      int size = xmax - (xmin - 1);
      GECODE_AUTOARRAY(int, count, size);
      for (int i = size; i--; ) {
	count[i] = 0;
      }
      for (int i = x.size(); i--; ) {
	count[x[i].val() - xmin]++;
      }
      for (int i = k.size(); i--; ) {
	if (k[i].min() > count[i]) {
	  home->fail();
	}
      }
    }
  }


  /** 
   * \brief Check whether the cardinalities are consistent
   *
   * -# \f$\forall i\in\{0, \dots, |k| - 1\}: max(k_i) \leq |x|\f$
   * -# \f$\neg\exists i\in\{0, \dots, |k| - 1\}: min(k_i) > |x|\f$
   */

  template<class Card, bool isView>
  forceinline ExecStatus
  card_cons(Space* home, Card& k, int n) {
    for (int i = k.size(); i--; ) {
      if (k[i].min() > n) {
	return ES_FAILED;
      }
      ModEvent me = k[i].lq(home, n);
      if (me_failed(me)) {
	return ES_FAILED;
      }
    }
    return ES_OK;
  }

  /**
   * \brief Template to post the global cardinality constraint
   *  for the different interfaces.
   *
   */
  template<class View, class Card, bool isView>
  forceinline void
  post_template(Space* home, ViewArray<View>& x, Card& k, IntConLevel& icl){ 

    int  n        = x_card(x);
    bool rewrite  = false; 
    if (!isView) {
     rewrite = check_alldiff(n, k);
    }

    GECODE_ES_FAIL(home, (card_cons<Card, isView>(home, k, x.size())));

    if (!isView && rewrite) {
      switch (icl) {
      case ICL_BND:
	GECODE_ES_FAIL(home,Distinct::Bnd<View>::post(home, x));
	break;
      case ICL_DOM:
	GECODE_ES_FAIL(home,Distinct::Dom<View>::post(home, x));
	break;
      default:
	GECODE_ES_FAIL(home,Distinct::Val<View>::post(home, x));
      }
    } else {
      switch (icl) {
      case ICL_BND:
      GECODE_ES_FAIL(home, (GCC::Bnd<View, Card, isView>::post(home, x, k)));
      break;
      case ICL_DOM:
	GECODE_ES_FAIL(home, (GCC::Dom<View, Card, isView>::post(home, x, k)));
	break;
      default:
	GECODE_ES_FAIL(home, (GCC::Val<View, Card, isView>::post(home, x, k)));
      }
    }
  }
  
}}

  using namespace Int;
  using namespace Support;


  // Interfacing gcc with fixed cardinalities
  void gcc(Space* home, const IntVarArgs& x, const IntArgs& c, 
	   int m, int unspec_low, int unspec_up, int min, int max,
	   IntConLevel icl) {

    if (home->failed()) {
      return;
    }
    
    GccIdxView xv(home, x);

    x_setidx(xv);

    FixCard cv(c, xv, m, (m / 3), (max - (min - 1)), 
	       min, max, unspec_low, unspec_up);

    // compute number of zero entries
    int z = 0;
    for (int j = cv.size(); j--; ){
      if (cv[j].max() == 0){
	z++;
      }
    }

    // if there are zero entries
    if (z > 0) {

      // reduce the occurences 
      FixCard red(cv.size() - z);
      IntArgs rem(z);
      z = 0;
      int c = 0;
      int r = red.size() - 1;
      for (int j = cv.size(); j--;) {
	if (cv[j].max() == 0){
	  rem[z] = cv[j].card();
	  z++;
	} else {
	  red[r]= cv[j];
	  r--;
	}
	c++;
      }
      
      IntSet zero(&rem[0], z);
      IntSetRanges remzero(zero);
      int n = xv.size();
      for (int i = n; i--; ) {
	GECODE_ME_FAIL(home, xv[i].minus(home, remzero));
      }
      GCC::post_template<GCC::IdxView, FixCard, false>(home, xv, red, icl);
    } else { 
      GCC::post_template<GCC::IdxView, FixCard, false>(home, xv, cv, icl);
    }
  }

  void gcc(Space* home, const IntVarArgs& x, const IntArgs& c, 
	   int m, int unspec, int min, int max, 
	   IntConLevel icl) {
    gcc(home, x, c, m, 0, unspec, min, max, icl);
  }

  void gcc(Space* home, const IntVarArgs& x, int lb, int ub,
	   IntConLevel icl) {
    if (home->failed()) {
      return;
    }
   
    GccIdxView xv(home,x);
    x_setidx(xv);

    int values = x_card(xv);
    FixCard c(home, values);
    GCC::initcard(home, xv, c, lb, ub);
    GCC::post_template<GCC::IdxView, FixCard, false>(home, xv, c, icl);
  }

  // Interfacing gcc with cardinality variables

  void gcc(Space* home, const IntVarArgs& x, int ub, IntConLevel icl) {
    gcc(home, x, ub, ub, icl);
  }

  void gcc(Space* home, const IntVarArgs& x, const IntVarArgs& c, 
	   int min, int max, IntConLevel icl) {
    
    if (home->failed()) {
      return;
    }

    GccIdxView xv(home, x);
    x_setidx(xv);

    linear(home, c, IRT_EQ, xv.size());

    VarCard cv(home, c);

    int interval = max - (min - 1);

    GECODE_AUTOARRAY(bool, done, interval);
    for (int i = 0; i < interval; i++) {
      done[i] = false;
    }

    GECODE_AUTOARRAY(ViewRanges<Gecode::Int::GCC::IdxView>, xrange, xv.size());
    for (int i = xv.size(); i--; ){
      ViewRanges<Gecode::Int::GCC::IdxView> iter(xv[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<Gecode::Int::GCC::IdxView> >     
      drl(&xrange[0], xv.size());
    Gecode::Iter::Ranges::Cache<
      Gecode::Iter::Ranges::
      NaryUnion<ViewRanges<Gecode::Int::GCC::IdxView> > > crl(drl);
    for ( ; crl(); ++crl) {
      for (int v = crl.min(); v <= crl.max(); v++) {
	done[v - min] = true;
      }
    }

    for (int i = 0; i < interval; i++) {
      if (!done[i]) {
	GECODE_ME_FAIL(home, cv[i].eq(home, 0));
      }
    }

    GCC::setcard<VarCard, Gecode::Int::GCC::IdxView, true>
      (home, xv, cv, min, max);

    GCC::post_template<GCC::IdxView, VarCard, true>(home, xv, cv, icl);
  }

  void gcc(Space* home, const IntVarArgs& x, const IntArgs& v, 
	   const IntVarArgs& c,int m, int unspec, bool all, 
	   int xmin, int xmax, IntConLevel icl) {
    gcc(home, x, v, c, m, 0, unspec, all, xmin, xmax, icl);
  }

  void gcc(Space* home, const IntVarArgs& x, const IntArgs& v, 
	   const IntVarArgs& c,int m, 
	   int unspec_low, int unspec_up, bool all, 
	   int xmin, int xmax, IntConLevel icl) {
    
    if (m != c.size()) {
      throw ArgumentSizeMismatch("Int::gcc");
    }
    if (home->failed()) {
      return;
    }

    GccIdxView xv(home, x);
    x_setidx(xv);

    int interval = xmax - (xmin - 1);

    GECODE_AUTOARRAY(bool, done, interval);
    for (int i = 0; i < interval; i++) {
      done[i] = false;
    }

    GECODE_AUTOARRAY(ViewRanges<Gecode::Int::GCC::IdxView>, xrange, xv.size());
    for (int i = xv.size(); i--; ){
      ViewRanges<Gecode::Int::GCC::IdxView> iter(xv[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<Gecode::Int::GCC::IdxView> >     
      drl(&xrange[0], xv.size());
    Gecode::Iter::Ranges::Cache<
      Gecode::Iter::Ranges::
      NaryUnion<ViewRanges<Gecode::Int::GCC::IdxView> > > crl(drl);
    for ( ; crl(); ++crl) {
      for (int v = crl.min(); v <= crl.max(); v++) {
	done[v - xmin] = true;
      }
    }

    if (all) {
      for (int i = 0; i < interval; i++) {
	done[i] = true;
      }
    }

    // iterating over cardinality variables
    int ci  = 0;  
    // iterating over the new cardvars
    int cvi = 0;  
    IntVarArgs cv(interval);
    for (int i = xmin; i <= xmax; i++) {
      // value in var domain
      if (done[i - xmin]) {
	if (ci < m) {
	  // specified wih cardinalities
	  if (i == v[ci]) {
	    cv[cvi] = c[ci];
	    ci++;
	    cvi++;
	  } else {
	    // value in domain but unspecified
	    IntVar iv(home, unspec_low, unspec_up);
	    cv[cvi] = iv; 
	    cvi++;
	  }
	} else {
	  // in domain but after the specification
	  IntVar iv(home, unspec_low, unspec_up);
	  cv[cvi] = iv; 
	  cvi++;
	}
      } else {
	// not in the variable domain of the current assignment
	if (ci < m) {
	  // but is specified
	  if (i == v[ci]) {
	    cv[cvi] = c[ci];
	    ci++;
	    cvi++;
	  } else {
	    // unspecified and not in x
	    IntVar iv(home, unspec_low, unspec_up);
	    cv[cvi] = iv; 
	    cvi++;
	  }
	} else {
	  // more values than specified
	  IntVar iv(home, unspec_low, unspec_up);
	  cv[cvi] = iv; 
	  cvi++;
	}
      }
    }

    if (ci < m) {
      // still values left
      for (; ci < m; ci++) {
	cv[cvi] = c[ci];
	ci++;
	cvi++;
      }
    }

    linear(home, c, IRT_LQ, xv.size());
    VarCard cardv(home, cv);

    GCC::setcard<VarCard, Gecode::Int::GCC::IdxView, true>
      (home, xv, cardv, xmin, xmax);
    GCC::post_template<GCC::IdxView, VarCard, true>(home, xv, cardv, icl);
  }
}


// STATISTICS: int-post

