/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2015
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

#include <climits>

namespace Gecode { namespace Search { namespace Meta { namespace Sequential {


  forceinline
  PortfolioStop::PortfolioStop(const Options& opt)
    : so(opt.stop), done(false), slice(opt.slice), l(opt.slice) {}
  forceinline void 
  PortfolioStop::inc(void) {
    done = false;
    l += slice;
  }
  forceinline bool 
  PortfolioStop::exhausted(void) const {
    return done;
  }
  forceinline void
  PortfolioStop::disable(void) {
    l = ULONG_MAX;
  }


  template<bool best>
  forceinline
  PBS<best>::PBS(Engine** s, unsigned int n, 
           const Statistics& stat0, PortfolioStop* so)
    : stat(stat0), slaves(s), n_slaves(n), cur(0),
      slave_stop(false), stop(so) {
  }

  template<bool best>
  Space* 
  PBS<best>::next(void) {
    slave_stop = false;
    unsigned int n_exhausted = 0;
    while (n_slaves > 0) {
      if (Space* s = slaves[cur]->next()) {
        // Constrain other slaves
        if (best) {
          for (unsigned int i=0; i<cur; i++)
            slaves[i]->constrain(*s);
          for (unsigned int i=cur+1; i<n_slaves; i++)
            slaves[i]->constrain(*s);
        }          
        return s;
      }
      if (slaves[cur]->stopped()) {
        if (stop->exhausted()) {
          cur++; n_exhausted++;
        } else {
          slave_stop = true;
          return NULL;
        }
      } else {
        // This slave is done, kill it after saving the statistics
        stat += slaves[cur]->statistics();
        delete slaves[cur];
        slaves[cur] = slaves[--n_slaves];
        if (n_slaves == 1)
          // Disable stoping
          stop->disable();
      }
      if (n_exhausted == n_slaves) {
        n_exhausted = 0;
        // Increment by one slice
        stop->inc();
      }
      if (cur == n_slaves)
        cur = 0;
    }
    return NULL;
  }

  template<bool best>
  bool 
  PBS<best>::stopped(void) const {
    return slave_stop;
  }

  template<bool best>
  Statistics 
  PBS<best>::statistics(void) const {
    Statistics s(stat);
    for (unsigned int i=n_slaves; i--; )
      s += slaves[i]->statistics();
    return s;
  }

  template<bool best>
  PBS<best>::~PBS(void) {
    for (unsigned int i=n_slaves; i--; )
      delete slaves[i];
    // Note that n_slaves might be different now!
    heap.rfree(slaves);
    delete stop;
  }

}}}}

// STATISTICS: search-meta
