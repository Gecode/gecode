/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2006
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

#include "gecode/cpltset.hh"
#include "gecode/cpltset/propagators.hh"

using namespace Gecode::CpltSet;

namespace Gecode {

  namespace CpltSet { namespace RangeRoots {
    
    /**
     * \namespace Gecode::CpltSet::RangeRoots
     * \brief Propagators for range/roots constraints
     */
    
    /*
     * Posting functions
     *
     */

    /// Constructs \a d0 as the bdd representing the Range constraint for the given variables
    template <class View0, class View1>
    forceinline void
    buildRange(Space* home, ViewArray<View0>& seq, View1 selview,
               View1 unionview, bdd& d0) {

      if (home->failed()) return;
      int n = seq.size();

      unsigned int xrange = seq[0].tableWidth();
      int xmax            = seq[0].initialLubMax();
      int xmin            = seq[0].initialLubMin();
      // compute maximum value
      for (int i = n; i--; ) {
        if (seq[i].initialLubMax() > xmax) {
          xmax = seq[i].initialLubMax();
        }
        if (seq[i].initialLubMin() < xmin) {
          xmin = seq[i].initialLubMin();
        }
        if (seq[i].tableWidth() > xrange) {
          xrange = seq[i].tableWidth();
        }
      }

      GECODE_ME_FAIL(home, unionview.intersect(home, xmin, xmax));

      int unionmin = unionview.initialLubMin();

      // restrict selector variable s to be \f$ s\subseteq \{0, n - 1\}\f$
      Iter::Ranges::Singleton idx(0, n - 1);
      // shift selection view to the right index
      int shift = 0 - selview.initialLubMin();
      GECODE_ME_FAIL(home, selview.intersectI(home, idx));

      // check for different ranges

      for (int k = 0; k < (int) xrange; k++) {
        bdd inter = bdd_false();
        for (int j = 0; j < n; j++) {
          LubValues<View0> lub(seq[j]);
          int seqmin = seq[j].initialLubMin();
          int seqmax = seq[j].initialLubMax();
          int cur    = xmin + k;
          if (seqmin <= cur && cur <= seqmax) {
            while (lub() && cur != lub.val()) {
              ++lub;
            }
            if (lub() && cur == lub.val()) {
              inter |= (selview.element(j + shift) & 
                        seq[j].element(k - (seqmin - xmin)));
              ++lub;
            }
          }
        }

        /// TODO: shouldnt we allow inter to be bdd_false() ?
        d0 &= (unionview.element(k - (unionmin - xmin)) % inter);
      }

      for (int i = 0; i < n; i++) {
        if (seq[i].assigned()) {
          d0 &= seq[i].dom();
        }
      }
      if (selview.assigned()) {
        d0 &= selview.dom();
      }
      if (unionview.assigned()) {
        d0 &= unionview.dom();
      }

    }

    template <class View0, class View1>
    forceinline void 
    range_post(Space* home, ViewArray<View0>& seq, View1 selview,
               View1 unionview) {
      if (home->failed())  return;

      bdd d0 = bdd_true();
      buildRange(home, seq, selview, unionview, d0);
      if (home->failed())  return;
      GECODE_ES_FAIL(home,
        (NaryTwoCpltSetPropagator<View0, View1>::post(home, seq, selview, 
                                                      unionview, d0)));
    }

    forceinline void 
    range_con(Space* home, const IntVarArgs& x, const CpltSetVar& s,
              const CpltSetVar& t) {
      int n = x.size();
      CpltSetView selview(s);
      CpltSetView unionview(t);

      ViewArray<Gecode::Int::IntView> iv(home, n);
      for (int i = 0; i < n; i++) {
        iv[i] = x[i];
      }
      ViewArray<SingletonCpltSetView> sbv(home, n);
      for (int i = 0; i < n; i++) {
        int rmin = std::min(unionview.initialLubMin(), iv[i].min());
        int rmax = std::max(unionview.initialLubMax(), iv[i].max());
        sbv[i].init(rmin, rmax, iv[i]);
      }

      range_post(home, sbv, selview, unionview);
    }

    /// Constructs \a d0 as the bdd representing the Roots constraint for the given variables
    template <class View0, class View1>
    forceinline void
    buildRoots(Space* home, ViewArray<View0>& seq, View1 selview,
               View1 unionview, bdd& d0) {
      if (home->failed()) return;
      int n = seq.size();

      unsigned int xrange = seq[0].tableWidth();
      int xmax            = seq[0].initialLubMax();
      int xmin            = seq[0].initialLubMin();
      // compute maximum value
      for (int i = n; i--; ) {
        if (seq[i].tableWidth() > xrange) {
          xrange = seq[i].tableWidth();
        }
        if (seq[i].initialLubMax() > xmax) {
          xmax = seq[i].initialLubMax();
        }
        if (seq[i].initialLubMin() < xmin) {
          xmin = seq[i].initialLubMin();
        }
      }

      int unionmin = unionview.initialLubMin();
      int unionmax = unionview.initialLubMax();
      if (unionview.assigned()) {
        xmin = unionview.glbMin();
        xmax = unionview.glbMax();
        xrange = xmax - xmin + 1;
      } else {
        if (unionmin < xmin) { xmin = unionmin; }
        if (unionmax < xmax) { xmax = unionmax; }
        if (unionview.tableWidth() > xrange) {
          xrange = unionview.tableWidth();
        }
      }

      // restrict selection variable s to be \f$ s\subseteq \{0, n - 1\}\f$
      Iter::Ranges::Singleton idx(0, n - 1);
      GECODE_ME_FAIL(home, selview.intersectI(home, idx));
      // in case the selection variable ranges over negative values
      int shift = 0 - selview.initialLubMin();

      for (int j = 0; j < n; j++) {    
        bdd subset = bdd_true();
        LubValues<SingletonCpltSetView> lub(seq[j]);
        for (unsigned int k = 0; k < xrange; k++) {
          int seqmin = seq[j].initialLubMin();
          int seqmax = seq[j].initialLubMax();
          int cur    = xmin + k;
          if (seqmin <= cur && cur <= seqmax) {
            while (lub() && cur != lub.val()) {
              ++lub;
            }
            if (lub() && cur == lub.val()) {
              if (unionmin <= cur && cur <= unionmax) {
                subset &= (seq[j].element(k - (seqmin - xmin)) >>= 
                           unionview.element(k - (unionmin - xmin)));
              }
              ++lub;
            }
          }
        }
        if (!manager.ctrue(subset)) {
          d0 &= (selview.element(j + shift) % (subset));
        }
        if (seq[j].assigned()) {
          d0 &= seq[j].dom();
        }
      }

      if (unionview.assigned()) {
        d0 &= unionview.dom();
      }   
      if (selview.assigned()) {
        d0 &= selview.dom();
      }
    }

    template <class View0, class View1>
    forceinline void 
    roots_post(Space* home, ViewArray<View0>& seq, View1 selview,
               View1 unionview) {
      if (home->failed()) return;

      bdd d0 = bdd_true();
      buildRoots(home, seq, selview, unionview, d0);
      if (home->failed()) return;
      GECODE_ES_FAIL(home,
        (NaryTwoCpltSetPropagator<View0, View1>::post(home, seq, selview,   
                                                      unionview, d0)));
    }

    forceinline void 
    roots_con(Space* home, const IntVarArgs& x, const CpltSetVar& s,
              const CpltSetVar& t, const CpltSetVarArgs& allvars) {
      int n = x.size();

      CpltSetView selview(s);
      CpltSetView unionview(t);

      ViewArray<Gecode::Int::IntView> iv(home, n);
      for (int i = 0; i < n; i++) {
        iv[i] = x[i];
      }
      ViewArray<SingletonCpltSetView> sbv(home, n);
      for (int i = 0; i < n; i++) {
        sbv[i].init(iv[i].min(), iv[i].max(), iv[i]);
      }

      // do ordering
      ViewArray<CpltSetView> vars(home, allvars.size());
      for (int i = allvars.size(); i--; ) {
        vars[i] = allvars[i];
      }

      variableorder(vars, sbv);
      roots_post(home, sbv, selview, unionview);
    }

    template <class View0, class View1>
    forceinline void 
    nvalue_post(Space* home, ViewArray<View0>& seq, View1 selview,
                View1 unionview, int usedvalues) {
      std::cout << "nvalue_post\n";
      if (home->failed())  return;
      /// TODO: WE HAVE TO ORDER ALL BDD VARS AVAILABLE SO FAR
      variableorder(seq);

      bdd d0 = bdd_true();
      int n = seq.size();
      Iter::Ranges::Singleton idx(0, n - 1);
      // select all variables in the sequence
      GECODE_ME_FAIL(home, selview.eqI(home, idx));
      // n values must be used (alldiff uses all |seq| values)
      GECODE_ME_FAIL(home,
        unionview.cardinality(home, usedvalues, usedvalues));
      // build the bdd for the range constraint
      buildRange(home, seq, selview, unionview, d0);
      if (home->failed())  return;
      GECODE_ES_FAIL(home,
        (NaryTwoCpltSetPropagator<View0, View1>::post(home, seq, selview, 
                                                      unionview, d0)));
    }


    forceinline void 
    nvalue_con(Space* home, const IntVarArgs& x, const CpltSetVar& s,
               const CpltSetVar& t, 
               int usedvalues, const CpltSetVarArgs& allvars) {
      int n = x.size();
      CpltSetView selview(s);
      CpltSetView unionview(t);

      ViewArray<Gecode::Int::IntView> iv(home, n);
      for (int i = 0; i < n; i++) {
        iv[i] = x[i];
      }
      ViewArray<SingletonCpltSetView> sbv(home, n);
      for (int i = 0; i < n; i++) {
        int rmin = std::min(unionview.initialLubMin(), iv[i].min());
        int rmax = std::max(unionview.initialLubMax(), iv[i].max());
        sbv[i].init(rmin, rmax, iv[i]);
      }

      // do ordering
      ViewArray<CpltSetView> vars(home, allvars.size());
      for (int i = allvars.size(); i--; ) {
        vars[i] = allvars[i];
      }

      variableorder(vars, sbv);
      nvalue_post(home, sbv, selview, unionview, usedvalues);
    }

    template <class View0, class View1>
    forceinline void 
    uses_post(Space* home, ViewArray<View0>& seq, View1 selview,
              View1 unionview, 
              ViewArray<View0>& seqprime, View1 selviewprime,
              View1 unionviewprime) {
      if (home->failed())  return;

      bdd d0 = bdd_true();
      int n = seq.size();
      Iter::Ranges::Singleton idx(0, n - 1);
      // select all variables in the sequence
      GECODE_ME_FAIL(home, selview.eqI(home, idx));

      // build the bdd for the range constraint
      buildRange(home, seq, selview, unionview, d0);
      if (home->failed())  return;

      int m = seqprime.size();
      Iter::Ranges::Singleton idx2(0, m - 1);
      // select all variables in the sequence
      GECODE_ME_FAIL(home, selviewprime.eqI(home, idx2));

      // build the bdd for the range constraint

      bdd e0 = bdd_true();
      buildRange(home, seqprime, selviewprime, unionviewprime, e0);
      if (home->failed())  return;

      // unionviewprime is a subset of unionview
      bdd r0 = bdd_true();
      int tab = std::max(unionview.tableWidth(), 
                         unionviewprime.tableWidth());
      for (int i = 0; i < (int) tab; i++) {
        r0 &= (unionviewprime.element(i)) >>= (unionview.element(i));
      }

     GECODE_ES_FAIL(home,
       (NaryTwoCpltSetPropagator<View0, View1>::post(home, seq, selview, 
                                                     unionview, d0)));
     GECODE_ES_FAIL(home,
       (NaryTwoCpltSetPropagator<View0, View1>::post(home, seqprime, 
                                                     selviewprime,  
                                                     unionviewprime, e0)));
     GECODE_ES_FAIL(home,
       (BinaryCpltSetPropagator<View1,View1>::post(home, unionview, 
                                                   unionviewprime, r0)));
    }


    forceinline void 
    uses_con(Space* home, const IntVarArgs& x, const CpltSetVar& s,
             const CpltSetVar& t, 
             const IntVarArgs& y, const CpltSetVar& u, const CpltSetVar& v) {
      int n = x.size();
      CpltSetView selview(s);
      CpltSetView unionview(t);

      ViewArray<Gecode::Int::IntView> iv(home, n);
      for (int i = 0; i < n; i++) {
        iv[i] = x[i];
      }
      ViewArray<SingletonCpltSetView> sbv(home, n);
      for (int i = 0; i < n; i++) {
        int rmin = std::min(unionview.initialLubMin(), iv[i].min());
        int rmax = std::max(unionview.initialLubMax(), iv[i].max());
        sbv[i].init(rmin, rmax, iv[i]);
      }

      CpltSetView selviewprime(u);
      CpltSetView unionviewprime(v);
      int m = y.size();
      ViewArray<Gecode::Int::IntView> ivprime(home, m);
      for (int i = 0; i < m; i++) {
        ivprime[i] = y[i];
      }
      ViewArray<SingletonCpltSetView> sbvprime(home, m);
      for (int i = 0; i < m; i++) {
        int rmin = std::min(unionviewprime.initialLubMin(), ivprime[i].min());
        int rmax = std::max(unionviewprime.initialLubMax(), ivprime[i].max());
        sbvprime[i].init(rmin, rmax, ivprime[i]);
      }

      uses_post(home, sbv, selview, unionview, 
                sbvprime, selviewprime, unionviewprime);
    }
    
    template <class View>
    forceinline void 
    selectUnion_post(Space* home, ViewArray<View>& x) {
      if (home->failed()) return;   

      bdd d0 = bdd_true();
      int n = x.size() - 2;
      ViewArray<View> seq(home, n);
      for (int i = 0; i < n; i++) {
        seq[i] = x[i];
      }
      buildRange(home, seq, x[n], x[n + 1], d0);
      if (home->failed()) return;    
      GECODE_ES_FAIL(home, NaryCpltSetPropagator<View>::post(home, x, d0));
    }

    forceinline void 
    selectUnion_con(Space* home, const CpltSetVarArgs& x, const CpltSetVar& s, 
                    const CpltSetVar& t) {
      int n = x.size();
      int m = n + 2;
      ViewArray<CpltSetView> bv(home, m);
      for (int i = 0; i < n; i++) {
        bv[i] = x[i];
      }
      bv[n] = s;
      bv[n + 1] = t;
      selectUnion_post(home, bv);
    }

  }} // end namespace CpltSet::RangeRoots;
  
  using namespace CpltSet::RangeRoots;

  void range(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t) {
    range_con(home, x, s, t);
  }

  void roots(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
             const CpltSetVarArgs& allvars) {
    roots_con(home, x, s, t, allvars);
  }

  // constraints using the range constraint
  void alldifferent(Space* home, const IntVarArgs& x, CpltSetVar s, 
                    CpltSetVar t, const CpltSetVarArgs& allvars) {
    nvalue_con(home, x, s, t, x.size(), allvars);
  }

  void nvalue(Space* home, const IntVarArgs& x, CpltSetVar s, 
                    CpltSetVar t, int n, const CpltSetVarArgs& allvars) {
    nvalue_con(home, x, s, t, n, allvars);
  }

  void uses(Space* home, const IntVarArgs& x, CpltSetVar s, CpltSetVar t, 
            const IntVarArgs& y, CpltSetVar u, CpltSetVar v) {
    uses_con(home, x, s, t, y, u, v);
  }

  void selectUnion(Space* home, const CpltSetVarArgs& x, CpltSetVar s, 
                   CpltSetVar t) {
    selectUnion_con(home, x, s, t);
  }


}

// STATISTICS: cpltset-post
