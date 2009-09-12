/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Patrick Pekczynski <pekczynski@ps.uni-sb.de>
 *
 *  Copyright:
 *     Patrick Pekczynski, 2004/2005
 *
 *  Last modified:
 *     $Date: 2009-09-08 19:02:37 +0200 (Di, 08 Sep 2009) $ by $Author: tack $
 *     $Revision: 9690 $
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

#include <gecode/int/linear.hh>
#include <gecode/int/distinct.hh>

namespace Gecode { namespace Int { namespace GCC {


  /**
   * \brief Post side constraints for the GCC
   *
   */
  template<class Card>
  inline ExecStatus
  postSideConstraints(Space& home, ViewArray<IntView>& x, ViewArray<Card>& k) {
    int n = x.size();
    int smin = 0;
    int smax = 0;
    for (int i = k.size(); i--; ) {
      GECODE_ME_CHECK((k[i].gq(home, 0)));
      GECODE_ME_CHECK((k[i].lq(home, n)));
      smax += k[i].max();
      smin += k[i].min();
    }

    if (n < smin || smax < n) {
      // not enough variables to satisfy min req
      return ES_FAILED;
    }

    // Remove all values from the x that are not in v
    IntArgs v(k.size());
    for (int i=k.size(); i--;)
      v[i] = k[i].card();
    IntSet valid(&v[0], v.size());
    for (int i = n; i--; ) {
      IntSetRanges ir(valid);
      GECODE_ME_CHECK(x[i].inter_r(home, ir));
    }

    // Remove all values with 0 max occurrence
    // and remove corresponding occurrence variables from k
    
    int noOfZeroes = 0;
    for (int i=k.size(); i--;)
      if (k[i].max() == 0)
        noOfZeroes++;

    if (noOfZeroes > 0) {
      IntArgs zeroIdx(noOfZeroes);
      noOfZeroes = 0;
      int j = 0;
      for (int i=0; i<k.size(); i++) {
        if (k[i].max() == 0) {
          zeroIdx[noOfZeroes++] = k[i].card();            
        } else {
          k[j++] = k[i];
        }
      }
      k.size(j);
      for (int i=x.size(); i--;) {
        IntSet zeroesI(&zeroIdx[0], noOfZeroes);
        IntSetRanges zeroesR(zeroesI);
        GECODE_ME_CHECK((x[i].minus_r(home, zeroesR)));
      }
    }

    if (Card::propagate) {
      Region re(home);
      Linear::Term<IntView>* t = re.alloc<Linear::Term<IntView> >(k.size());
      for (int i = k.size(); i--; ) {
        t[i].a=1; t[i].x=k[i].base();
      }
      Linear::post(home,t,k.size(),IRT_EQ,x.size(),ICL_BND);
    }
    return ES_OK;
  }


  /**
   * \brief Check if GCC is equivalent to distinct
   *
   */
  template<class Card>
  inline bool
  isDistinct(Space& home, ViewArray<IntView>& x, ViewArray<Card>& k) {
    if (Card::propagate) {
      Region r(home);
      ViewRanges<IntView>* xrange = r.alloc<ViewRanges<IntView> >(x.size());
      for (int i = x.size(); i--; ){
        ViewRanges<IntView> iter(x[i]);
        xrange[i] = iter;
      }
      Iter::Ranges::NaryUnion<ViewRanges<IntView> > drl(&xrange[0], x.size());
      if (static_cast<unsigned int>(k.size()) == Iter::Ranges::size(drl)) {
        for (int i=k.size(); i--;)
          if (k[i].min() != 1 || k[i].max() != 1)
            return false;
        return true;
      } else {
        return false;
      }
    } else {
      for (int i=k.size(); i--;)
        if (k[i].min() != 0 || k[i].max() != 1)
          return false;
      return true;
    }
  }

}}}

// STATISTICS: int-prop
