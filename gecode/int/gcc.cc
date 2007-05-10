/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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

#include "gecode/int/gcc.hh"

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
  check_alldiff(int n, ViewArray<Card>& k){
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
   * \brief Compute the cardinality of the union of all variable domains in \a x.
   *
   */
  template <class View>
  forceinline int
  x_card(ViewArray<View>& x, IntConLevel icl) {

    int n = x.size();

    GECODE_AUTOARRAY(ViewRanges<View>, xrange, n);
    for (int i = n; i--; ){
      ViewRanges<View> iter(x[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<View> > drl(&xrange[0], x.size());
    int r = 0;
    if (icl == ICL_BND) {
      int fstv = drl.min();
      int lstv = 0;
      for ( ; drl(); ++drl){
        lstv = drl.max();
      }
      r = lstv - fstv + 1;

    } else {

      for ( ; drl(); ++drl){
        for (int v = drl.min(); v <=drl.max(); v++) {
          r ++;
        }
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
  initcard(Space* home, ViewArray<View>& x, ViewArray<Card>& k,
           int lb, int ub,
           IntConLevel icl) {
    GECODE_AUTOARRAY(ViewRanges<View>, xrange, x.size());
    for (int i = x.size(); i--; ){
      ViewRanges<View> iter(x[i]);
      xrange[i] = iter;
    }

    Iter::Ranges::NaryUnion<ViewRanges<View> > drl(&xrange[0], x.size());
    if (icl == ICL_BND) {
      int fstv = drl.min();
      int lstv = 0;
      for ( ; drl(); ++drl){
        lstv = drl.max();
      }
      for (int i = fstv; i <= lstv; i++) {
        k[i - fstv].init(home, lb, ub, i);
      }
    } else {
      int idx = 0;
      for ( ; drl(); ++drl){
        for (int v = drl.min(); v <= drl.max(); v++){
          k[idx].init(home, lb, ub, v);
          idx++;
        }
      }
    }
  }


  /**
   * \brief Reset already existing cardinalities to zero.
   *
   */

  template <class Card, class View, bool isView>
  forceinline void
  setcard(Space* home, ViewArray<View>& x, ViewArray<Card>& k,
          int xmin, int xmax) {

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
  card_cons(Space* home, ViewArray<Card>& k, int n, bool all) {
    // this should be the required min and allowed max
    int smin = 0;
    int smax = 0;
    int m    = k.size();
    for (int i = m; i--; ) {
      int ci = k[i].counter();
      if (ci > k[i].max() ) {
//         std::cout << "more occurrences of "<<k[i].card()
//                   << " than allowed\n";
        return ES_FAILED;
      } else {
        smax += (k[i].max() - ci);
        if (ci < k[i].min()) {
          smin += (k[i].min() - ci);
        }
      }
      if (k[i].min() > n) {
//         std::cout << "cannot satisfy requiremnts for " << k[i].card() << "!\n";
        return ES_FAILED;
      }
      if (!k[i].assigned()) {
        ModEvent me = k[i].lq(home, n);
        if (me_failed(me)) {
//           std::cout << "invalid\n";
          return ES_FAILED;
        }
      }
    }

//     std::cout << "mi:" << smin << " x: "<< n << " ma:" << smax<<"\n";
    if (n < smin) {
//       std::cout << "not enough variables to satisfy min req\n";
      return ES_FAILED;
    }


//     for (int i = 0; i < k.size(); i++) {
//       std::cout << k[i] << " ";
//     }


    if (all && smax < n) {
//       std::cout << "the maximal occurrence for the cardinalities cannot be satisfied\n";
      return ES_FAILED;
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
  post_template(Space* home, ViewArray<View>& x, ViewArray<Card>& k,
                IntConLevel& icl, bool& all){

    int  n        = x_card(x, icl);
    bool rewrite  = false;
    if (!isView) {
     rewrite = check_alldiff(n, k);
    }

    GECODE_ES_FAIL(home, (card_cons<Card, isView>(home, k, x.size(), all)));
    if (!isView && rewrite) {
      IntVarArgs xv(x.size());
      for (int i = 0; i < x.size(); i++) {
        IntVar iv(x[i]);
        xv[i] = iv;
      }
      distinct(home, xv, icl);
    } else {
      switch (icl) {
      case ICL_BND:
      GECODE_ES_FAIL(home, (GCC::Bnd<View, Card, isView>::post(home, x, k, all)));
      break;
      case ICL_DOM:
        GECODE_ES_FAIL(home, (GCC::Dom<View, Card, isView>::post(home, x, k, all)));
        break;
      default:
        GECODE_ES_FAIL(home, (GCC::Val<View, Card, isView>::post(home, x, k, all)));
      }
    }
  }

}}

  using namespace Int;
  using namespace Int::GCC;
  using namespace Support;

  template <class View>
  void initCV(Space* home,
              const IntArgs& ia, const ViewArray<View>& x,
              int l, ViewArray<OccurBndsView>& a,
              int iasize, int val, int nov,
              int min, int max,
              int unspec_low, int unspec_up) {

    int n = x.size();

    GECODE_AUTOARRAY(ViewRanges<View>, xrange, n);
    for (int i = n; i--; ){
      ViewRanges<View> iter(x[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<View> >
      drl(&xrange[0], x.size());
    Gecode::Iter::Ranges::Cache<
      Gecode::Iter::Ranges::
      NaryUnion<ViewRanges<View> > > crl(drl);

    int c = 0;
    int r = 0;

    GECODE_AUTOARRAY(bool, indom, (max - (min - 1)));
    for (int i = max - (min - 1); i--; ) {
      indom[i] = false;
    }
    for ( ; crl(); ++crl) {
      for (int v = crl.min(); v <= crl.max(); v++) {
        indom[v - min] = true;
      }
    }

    int xmin = min;
    int xmax = max;
    // mark those values that are specified
    min = std::min(xmin, ia[0]);
    max = std::max(xmax, ia[(val - 1) * 3]);

    for (int v = min; v <= max; v++) {
      if (c > l - 1) {
        break;
      }
      // value is in a variable domain
      if (v >= xmin && indom[v - xmin]) {
        if (r < iasize) {
          if (v == ia[r]) {
            // value is specified with cardinalities
            // checking should be outsourced to gcc.cc
            if (ia[r + 1] > ia[r + 2]) {
              throw ArgumentSizeMismatch("Int::gcc");
            }
        
            a[c].card(v);
            a[c].counter(0);
            a[c].min(ia[r + 1]);
            a[c].max(ia[r + 2]);
            c++;
            r += 3;
          } else {
            // value is not specified with cardinalities
            // the value is unspecified
            a[c].card(v);
            a[c].counter(0);
            a[c].min(unspec_low);
            a[c].max(unspec_up);
            c++;
          }
        } else {
          // there are more values in the variable domains
          // than specified
            a[c].card(v);
            a[c].counter(0);
            a[c].min(unspec_low);
            a[c].max(unspec_up);
            c++;
        }
      } else {
        // the value is not in a variable domain of the current assignment
        if (r < iasize) {
          // but it is specified
          if (v == ia[r]) {
            // checking should be outsourced to gcc.cc
            if (ia[r + 1] > ia[r + 2]) {
              throw ArgumentSizeMismatch("Int::gcc");
            }
        
            a[c].card(v);
            a[c].counter(0);
            a[c].min(ia[r + 1]);
            a[c].max(ia[r + 2]);
            c++;
            r += 3;
          } else {
            // the value is unspecified
            a[c].card(v);
            a[c].counter(0);
            a[c].min(unspec_low);
            a[c].max(unspec_up);
            c++;
          }
        } else {
          // there are more values in the variable domains
          // than specified
            a[c].card(v);
            a[c].counter(0);
            a[c].min(unspec_low);
            a[c].max(unspec_up);
            c++;
        }
      }
    }

    if (c < l) {
      for ( ; r < iasize; r+=3) {
        assert(0 <= c && c < l);
        a[c].card(ia[r]);
        a[c].counter(0);
        a[c].min(unspec_low);
        a[c].max(unspec_up);
        c++;
        r+=3;
      }
    }
  }

  // Interfacing gcc with fixed cardinalities
  void gcc(Space* home, const IntVarArgs& x, const IntArgs& c,
           int m, int unspec_low, int unspec_up, int min, int max,
           IntConLevel icl, PropVar) {
    if (home->failed()) {
      return;
    }

    ViewArray<IntView> x0(home, x);
    if (x0.shared()) {
      throw ArgumentSame("Int::GCC");
    }


    ViewArray<IntView> xv(home, x);

    int iasize = m;
    int val = m / 3;
    int nov = max - (min - 1);

    ViewArray<OccurBndsView> cv(home, std::max(nov, val));

    initCV(home, c, xv,
           std::max(nov, val), cv,
           iasize, val, nov,
           min, max,
           unspec_low, unspec_up);

    // compute number of zero entries
    int z = 0;
    for (int j = cv.size(); j--; ){
      if (cv[j].max() == 0){
        z++;
      }
    }

    bool all = (m == (max) - (min - 1));
    // if there are zero entries
    if (z > 0) {

      // reduce the occurences
      ViewArray<OccurBndsView> red(home, cv.size() - z);
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
      int n = xv.size();
      for (int i = n; i--; ) {
        IntSetRanges remzero(zero);
        GECODE_ME_FAIL(home, xv[i].minus(home, remzero));
      }
      GCC::post_template<IntView,OccurBndsView,false>(home, xv, red, icl, all);
    } else {
      GCC::post_template<IntView,OccurBndsView,false>(home, xv, cv, icl, all);
    }
  }

  void gcc(Space* home, const IntVarArgs& x, const IntArgs& c,
           int m, int unspec, int min, int max,
           IntConLevel icl, PropVar) {
    gcc(home, x, c, m, 0, unspec, min, max, icl);
  }

  void gcc(Space* home, const IntVarArgs& x, int lb, int ub,
           IntConLevel icl, PropVar) {
    if (home->failed()) {
      return;
    }

    ViewArray<IntView> x0(home, x);
    if (x0.shared()) {
      throw ArgumentSame("Int::GCC");
    }

    ViewArray<IntView> xv(home,x);

    int values = x_card(xv, icl);

    ViewArray<OccurBndsView> c(home, values);
    GCC::initcard(home, xv, c, lb, ub, icl);
    bool all = true;
    GCC::post_template<IntView, OccurBndsView, false>(home, xv, c, icl, all);
  }


  void gcc(Space* home, const IntVarArgs& x, int ub, 
           IntConLevel icl, PropVar) {
    gcc(home, x, ub, ub, icl);
  }

  // Interfacing gcc with cardinality variables

  void gcc(Space* home, const IntVarArgs& x, const IntVarArgs& c,
           int min, int max, IntConLevel icl, PropVar) {

    if (home->failed()) {
      return;
    }

    ViewArray<IntView> xv(home, x);

    linear(home, c, IRT_EQ, xv.size());

    ViewArray<CardView> cv(home, c);

    int interval = max - min + 1;

    GECODE_AUTOARRAY(bool, done, interval);
    for (int i = 0; i < interval; i++) {
      done[i] = false;
    }

    GECODE_AUTOARRAY(ViewRanges<IntView>, xrange, xv.size());
    for (int i = xv.size(); i--; ){
      ViewRanges<IntView> iter(xv[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<IntView> >
      drl(&xrange[0], xv.size());
    Gecode::Iter::Ranges::Cache<
      Gecode::Iter::Ranges::
      NaryUnion<ViewRanges<IntView> > > crl(drl);
    for ( ; crl(); ++crl) {
      for (int v = crl.min(); v <= crl.max(); v++) {
        done[v - min] = true;
      }
    }

    for (int i = 0; i < interval; i++) {
      if (!done[i]) {
        if (icl == ICL_DOM) {
          GECODE_ME_FAIL(home, cv[i].eq(home, 0));
        }
      }
    }

    GCC::setcard<CardView, IntView, true>(home, xv, cv, min, max);

    int sum_min = 0;
    int sum_max = 0;
    IntArgs dx(cv.size());
    for (int i = 0; i < cv.size(); i++) {
      dx[i] = cv[i].card();
      sum_min += cv[i].card() * cv[i].min();
      sum_max += cv[i].card() * cv[i].max();
    }
    bool all = true;
    GCC::post_template<IntView, CardView, true>(home, xv, cv, icl, all);
  }

  void gcc(Space* home,
           const IntVarArgs& x, const IntArgs& v, const IntVarArgs& c,
           int m, int unspec, bool all, int min, int max,
           IntConLevel icl, PropVar) {
    gcc(home, x, v, c, m, 0, unspec, all, min, max, icl);
  }

  void gcc(Space* home,
           const IntVarArgs& x, const IntArgs& v, const IntVarArgs& c,
           int m, int unspec_low, int unspec_up, bool all, int min, int max,
           IntConLevel icl, PropVar) {

    if (m != c.size()) {
      throw ArgumentSizeMismatch("Int::gcc");
    }
    if (home->failed()) {
      return;
    }

    ViewArray<IntView> xv(home, x);

    int interval = max - (min - 1);

    GECODE_AUTOARRAY(bool, done, interval);
    for (int i = 0; i < interval; i++) {
      done[i] = false;
    }

    GECODE_AUTOARRAY(ViewRanges<IntView>, xrange, xv.size());
    for (int i = xv.size(); i--; ){
      ViewRanges<IntView> iter(xv[i]);
      xrange[i] = iter;
    }

    Gecode::Iter::Ranges::NaryUnion<ViewRanges<IntView> >
      drl(&xrange[0], xv.size());
    Gecode::Iter::Ranges::Cache<
      Gecode::Iter::Ranges::
      NaryUnion<ViewRanges<IntView> > > crl(drl);
    for ( ; crl(); ++crl) {
      for (int v = crl.min(); v <= crl.max(); v++) {
        done[v - min] = true;
      }
    }

    if (all) {
      linear(home, c, IRT_EQ, xv.size());
      for (int i = 0; i < interval; i++) {
        done[i] = true;
      }
    } else {
      linear(home, c, IRT_LQ, xv.size());
    }

    // iterating over cardinality variables
    int ci  = 0;
    // iterating over the new cardvars
    int cvi = 0;
    IntVarArgs cv(interval);
    for (int i = min; i <= max; i++) {
      // value in var domain
      if (done[i - min]) {
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


    ViewArray<CardView> cardv(home, cv);

    GCC::setcard<CardView, IntView, true>(home, xv, cardv, min, max);
    GCC::post_template<IntView, CardView, true>(home, xv, cardv, icl, all);
  }
}


// STATISTICS: int-post
