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

#include <cmath>
#include <algorithm>

#include <gecode/search/meta/dead.hh>

namespace Gecode { namespace Search { namespace Meta { namespace Sequential {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop* 
  stop(const Options& opt);

  /// Create sequential portfolio engine
  GECODE_SEARCH_EXPORT Engine* 
  engine(Engine** slaves, unsigned int n_slaves, 
         const Statistics& stat, Stop* stop, bool best);

}}}}

namespace Gecode { namespace Search { namespace Meta { namespace Parallel {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop* 
  stop(Stop* so);

  /// Create parallel portfolio engine
  GECODE_SEARCH_EXPORT Engine* 
  engine(Engine** slaves, Stop** stops, unsigned int n_slaves, 
         const Statistics& stat, bool best);

}}}}

namespace Gecode { namespace Search { namespace Meta {

  template<template<class> class E, class T>
  Engine*
  sequential(T* s, const Search::Statistics& stat, Options& opt) {
    // Whether to clone
    bool clone = opt.clone;
    
    // Configure for slave creation
    opt.clone   = false;
    opt.stop    = Sequential::stop(opt);
    // In case there are more threads than assets requested
    opt.threads = ceil(opt.threads / static_cast<double>(opt.assets));
    
    unsigned int n_slaves = opt.assets;
    Engine** slaves = heap.alloc<Engine*>(n_slaves);
    
    for (unsigned int i=1; i<n_slaves; i++) {
      Space* slave = s->clone(opt.threads <= 1.0);
      (void) slave->slave(i);
      slaves[i] = build<E,T>(slave,opt);
    }
    {
      Space* slave = clone ? s->clone(opt.threads <= 1.0) : s;
      (void) slave->slave(0);
      slaves[0] = build<E,T>(slave,opt);
    }
    
    return Sequential::engine(slaves,n_slaves,stat,opt.stop,E<T>::best);
  }

#ifdef GECODE_HAS_THREADS

  template<template<class> class E, class T>
  Engine*
  parallel(T* s, const Search::Statistics& stat, Options& opt) {
    bool clone = opt.clone;
    Stop* stop = opt.stop;

    Region r(*s);

    // Is there at least one thread per asset?
    if (opt.threads >= static_cast<double>(opt.assets)) {
      opt.threads = std::max(opt.threads - static_cast<double>(opt.assets),
                             1.0);    
      opt.clone = false;
      unsigned int n_slaves = opt.assets;
      Engine** slaves = r.alloc<Engine*>(n_slaves);
      Stop** stops = r.alloc<Stop*>(n_slaves);
        
      for (unsigned int i=1; i<n_slaves; i++) {
        opt.stop = stops[i] = Parallel::stop(stop);
        Space* slave = s->clone(false);
        (void) slave->slave(i);
        slaves[i] = build<E,T>(slave,opt);
      }
      {
        opt.stop = stops[0] = Parallel::stop(stop);
        Space* slave = clone ? s->clone(false) : s;
        (void) slave->slave(0);
        slaves[0] = build<E,T>(slave,opt);
      }
        
      return Parallel::engine(slaves,stops,n_slaves,stat,E<T>::best);
    } else {
      // Okay, some assets must be run by a single thread
      unsigned int n_threads = static_cast<unsigned int>(opt.threads);
      unsigned int n_assets = opt.assets;
      unsigned int apt = n_assets / n_threads;
      unsigned int apr = n_assets % n_threads;
      opt.threads = 1.0;
      opt.clone   = false;
        
      unsigned int n_pslaves = n_threads;
      Engine** pslaves = r.alloc<Engine*>(n_pslaves);
      Stop** pstops = r.alloc<Stop*>(n_pslaves);
      
      // The asset number
      unsigned int a = 0;        

      for (unsigned int i=0; i<n_pslaves; i++) {
        // How many sequential assets for iteration i needed?
        unsigned int api = apt + ((i < apr) ? 1 : 0);
          
        if (api > 1) {
          Search::Statistics sstat;
          opt.stop = stop;
          Stop* sstop = Sequential::stop(opt);
          Engine** sslaves = heap.alloc<Engine*>(api);
          Space* sroot = (!clone && (a == n_assets-api)) ? s : s->clone(false);
          opt.stop = sstop;
          for (int j=1; j<api; j++) {
            Space* sslave = sroot->clone(true);
            (void) sslave->slave(a++);
            sslaves[j] = build<E,T>(sslave,opt);
          }
          {
            Space* sslave = sroot;
            (void) sslave->slave(a++);
            sslaves[0] = build<E,T>(sslave,opt);
          }
          pslaves[i] = Sequential::engine(sslaves,api,sstat,sstop,
                                          E<T>::best);
          pstops[i] = Parallel::stop(sstop);
        } else {
          opt.stop = pstops[i] = Parallel::stop(stop);
          Space* pslave = (!clone && (a == n_assets-1)) ? s : s->clone(false);
          (void) pslave->slave(a++);
          pslaves[i] = build<E,T>(pslave,opt);
        }
      }
      assert(a == n_assets);
      return Parallel::engine(pslaves,pstops,n_pslaves,stat,E<T>::best);
    }
  }

#endif

}}}

namespace Gecode {

  template<template<class> class E, class T>
  PBS<E,T>::PBS(T* s, const Search::Options& o) {
    Search::Options opt(o.expand());

    if (opt.assets == 0)
      throw Search::NoAssets("PBS::PBS");
    
    // Always execute master function
    {
      MetaInfo mi(0);
      (void) s->master(mi);
    }
    // No need to create a portfolio engine but must run slave function
    if (o.assets == 1) {
      MetaInfo mi(0);
      (void) s->slave(mi);
      e = Search::build<E,T>(s,opt);
      return;
    }

    Search::Statistics stat;

    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      e = new Search::Meta::Dead(stat);
      return;
    }

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::Meta::parallel<E,T>(s,stat,opt);
    else
#endif
      e = Search::Meta::sequential<E,T>(s,stat,opt);
  }

  template<template<class> class E, class T>
  forceinline T*
  pbs(T* s, const Search::Options& o) {
    PBS<E,T> r(s,o);
    return r.next();
  }

}

// STATISTICS: search-meta
