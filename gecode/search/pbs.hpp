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
  stop(Stop* so);

  /// Create sequential portfolio engine
  GECODE_SEARCH_EXPORT Engine* 
  engine(Engine** slaves, Stop** stops, unsigned int n_slaves, 
         const Statistics& stat, const Search::Options& opt, bool best);

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
  sequential(T* master, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    Region r(*master);

    // In case there are more threads than assets requested
    opt.threads = ceil(opt.threads / static_cast<double>(opt.assets));
    
    unsigned int n_slaves = opt.assets;
    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);
    
    for (unsigned int i=0; i<n_slaves; i++) {
      opt.stop = stops[i] = Sequential::stop(stop);
      Space* slave = (i == n_slaves-1) ? 
        master : master->clone(opt.threads <= 1.0,opt.share);
      (void) slave->slave(i);
      slaves[i] = build<E,T>(slave,opt);
    }
    
    return Sequential::engine(slaves,stops,n_slaves,stat,opt,E<T>::best);
  }

#ifdef GECODE_HAS_THREADS

  template<template<class> class E, class T>
  Engine*
  parallel(T* master, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    Region r(*master);

    // Limit the number of slaves to the number of threads
    unsigned int n_slaves = std::min(static_cast<unsigned int>(opt.threads),
                                     opt.assets);
    // Redistribute additional threads to slaves
    opt.threads = ceil(opt.threads / static_cast<double>(n_slaves));

    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);
        
    for (unsigned int i=0; i<n_slaves; i++) {
      opt.stop = stops[i] = Parallel::stop(stop);
      Space* slave = (i == n_slaves-1) ? 
        master : master->clone(false,opt.share);
      (void) slave->slave(i);
      slaves[i] = build<E,T>(slave,opt);
    }
        
    return Parallel::engine(slaves,stops,n_slaves,stat,E<T>::best);
  }

#endif

}}}

namespace Gecode {

  template<template<class> class E, class T>
  PBS<E,T>::PBS(T* s, const Search::Options& o) {
    Search::Options opt(o.expand());

    if (opt.assets == 0)
      throw Search::NoAssets("PBS::PBS");
    
    Search::Statistics stat;

    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!opt.clone)
        delete s;
      e = new Search::Meta::Dead(stat);
      return;
    }

    // Check whether a clone must be used
    T* master = opt.clone ? 
      dynamic_cast<T*>(s->clone(opt.threads <= 1.0,opt.share)) : s;
    opt.clone = false;

    // Always execute master function
    (void) master->master(0);

    // No need to create a portfolio engine but must run slave function
    if (o.assets == 1) {
      (void) master->slave(0);
      e = Search::build<E,T>(master,opt);
      return;
    }

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::Meta::parallel<E,T>(master,stat,opt);
    else
#endif
      e = Search::Meta::sequential<E,T>(master,stat,opt);
  }

  template<template<class> class E, class T>
  forceinline T*
  pbs(T* s, const Search::Options& o) {
    PBS<E,T> r(s,o);
    return r.next();
  }

}

// STATISTICS: search-meta
