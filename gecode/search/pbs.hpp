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

namespace Gecode { namespace Search {

  /// A PBS engine builder
  template<class T, template<class> class E>
  class PbsBuilder : public Builder {
    using Builder::opt;
  public:
    /// The constructor
    PbsBuilder(const Options& opt);
    /// The actual build function
    virtual Engine* operator() (Space* s) const;
  };

  template<class T, template<class> class E>
  inline
  PbsBuilder<T,E>::PbsBuilder(const Options& opt)
    : Builder(opt,E<T>::best) {}

  template<class T, template<class> class E>
  Engine*
  PbsBuilder<T,E>::operator() (Space* s) const {
    return build<T,PBS<T,E> >(s,opt);
  }

}}

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

  template<class T, template<class> class E>
  Engine*
  sequential(T* master, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    Region r(*master);

    // In case there are more threads than assets requested
    opt.threads = std::max(floor(opt.threads /
                                 static_cast<double>(opt.assets)),1.0);

    unsigned int n_slaves = opt.assets;
    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);

    for (unsigned int i=0; i<n_slaves; i++) {
      opt.stop = stops[i] = Sequential::stop(stop);
      Space* slave = (i == n_slaves-1) ?
        master : master->clone(opt.threads <= 1.0,opt.share_pbs);
      (void) slave->slave(i);
      slaves[i] = build<T,E>(slave,opt);
    }

    return Sequential::engine(slaves,stops,n_slaves,stat,opt,E<T>::best);
  }

  template<class T, template<class> class E>
  Engine*
  sequential(T* master, SEBs& sebs,
             const Search::Statistics& stat, Options& opt, bool best) {
    Region r(*master);

    int n_slaves = sebs.size();
    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);

    for (int i=0; i<n_slaves; i++) {
      // Re-configure slave options
      stops[i] = Sequential::stop(sebs[i]->options().stop);
      sebs[i]->options().stop  = stops[i];
      sebs[i]->options().clone = false;
      Space* slave = (i == n_slaves-1) ?
        master : master->clone(sebs[i]->options().threads <= 1.0,
                               sebs[i]->options().share_pbs);
      (void) slave->slave(i);
      slaves[i] = (*sebs[i])(slave);
      delete sebs[i];
    }

    return Sequential::engine(slaves,stops,n_slaves,stat,opt,best);
  }

#ifdef GECODE_HAS_THREADS

  template<class T, template<class> class E>
  Engine*
  parallel(T* master, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    Region r(*master);

    // Limit the number of slaves to the number of threads
    unsigned int n_slaves = std::min(static_cast<unsigned int>(opt.threads),
                                     opt.assets);
    // Redistribute additional threads to slaves
    opt.threads = floor(opt.threads / static_cast<double>(n_slaves));

    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);

    for (unsigned int i=0; i<n_slaves; i++) {
      opt.stop = stops[i] = Parallel::stop(stop);
      Space* slave = (i == n_slaves-1) ?
        master : master->clone(false,opt.share_pbs);
      (void) slave->slave(i);
      slaves[i] = build<T,E>(slave,opt);
    }

    return Parallel::engine(slaves,stops,n_slaves,stat,E<T>::best);
  }

  template<class T, template<class> class E>
  Engine*
  parallel(T* master, SEBs& sebs,
           const Search::Statistics& stat, Options& opt, bool best) {
    Region r(*master);

    // Limit the number of slaves to the number of threads
    int n_slaves = std::min(static_cast<int>(opt.threads),
                            sebs.size());
    Engine** slaves = r.alloc<Engine*>(n_slaves);
    Stop** stops = r.alloc<Stop*>(n_slaves);

    for (int i=0; i<n_slaves; i++) {
      // Re-configure slave options
      stops[i] = Parallel::stop(sebs[i]->options().stop);
      sebs[i]->options().stop  = stops[i];
      sebs[i]->options().clone = false;
      Space* slave = (i == n_slaves-1) ?
        master : master->clone(false,sebs[i]->options().share_pbs);
      (void) slave->slave(i);
      slaves[i] = (*sebs[i])(slave);
      delete sebs[i];
    }
    // Delete excess builders
    for (int i=n_slaves; i<sebs.size(); i++)
      delete sebs[i];

    return Parallel::engine(slaves,stops,n_slaves,stat,best);
  }

#endif

}}}

namespace Gecode {

  template<class T, template<class> class E>
  PBS<T,E>::PBS(T* s, const Search::Options& o) {
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
      dynamic_cast<T*>(s->clone(opt.threads <= 1.0,opt.share_pbs)) : s;
    opt.clone = false;

    // Always execute master function
    (void) master->master(0);

    // No need to create a portfolio engine but must run slave function
    if (o.assets == 1) {
      (void) master->slave(0);
      e = Search::build<T,E>(master,opt);
      return;
    }

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::Meta::parallel<T,E>(master,stat,opt);
    else
#endif
      e = Search::Meta::sequential<T,E>(master,stat,opt);
  }

  template<class T, template<class> class E>
  void
  PBS<T,E>::build(T* s, SEBs& sebs, const Search::Options& o) {
    // Check whether all sebs do either best solution search or not
    bool best;
    {
      int b = 0;
      for (int i=sebs.size(); i--; )
        b += sebs[i]->best() ? 1 : 0;
      if ((b > 0) && (b < sebs.size()))
        throw Search::MixedBest("PBS::PBS");
      best = (b == sebs.size());
    }

    Search::Options opt(o.expand());
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
      dynamic_cast<T*>(s->clone(opt.threads <= 1.0,opt.share_pbs)) : s;
    opt.clone = false;

    // Always execute master function
    (void) master->master(0);

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::Meta::parallel<T,E>(master,sebs,stat,opt,best);
    else
#endif
      e = Search::Meta::sequential<T,E>(master,sebs,stat,opt,best);
  }

  template<class T, template<class> class E>
  inline
  PBS<T,E>::PBS(T* s, SEBs& sebs, const Search::Options& o) {
    build(s,sebs,o);
  }
  template<class T, template<class> class E>
  inline
  PBS<T,E>::PBS(T* s, SEB seb0, SEB seb1,
                const Search::Options& o) {
    SEBs sebs(2, seb0, seb1);
    build(s,sebs,o);
  }
  template<class T, template<class> class E>
  inline
  PBS<T,E>::PBS(T* s, SEB seb0, SEB seb1, SEB seb2,
                const Search::Options& o) {
    SEBs sebs(3, seb0, seb1, seb2);
    build(s,sebs,o);
  }
  template<class T, template<class> class E>
  inline
  PBS<T,E>::PBS(T* s, SEB seb0, SEB seb1, SEB seb2, SEB seb3,
                const Search::Options& o) {
    SEBs sebs(4, seb0, seb1, seb2, seb3);
    build(s,sebs,o);
  }

  template<class T, template<class> class E>
  inline T*
  pbs(T* s, const Search::Options& o) {
    PBS<T,E> r(s,o);
    return r.next();
  }

  template<class T, template<class> class E>
  inline SEB
  pbs(const Search::Options& o) {
    return new Search::PbsBuilder<T,E>(o);
  }

}

// STATISTICS: search-meta
