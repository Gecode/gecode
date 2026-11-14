/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2015
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
#include <memory>
#include <gecode/search/worker-control.hh>

namespace Gecode { namespace Search {

  /// Dispose partially constructed portfolio assets
  forceinline void
  pbscleanup(Engine** assets, Stop** stops, unsigned int n_assets,
             Space* hub) {
    for (unsigned int i=0U; i<n_assets; i++) {
      delete assets[i];
      delete stops[i];
    }
    delete hub;
  }

  /// Dispose builders consumed by an explicit portfolio
  forceinline void
  pbscleanup(SEBs& sebs) {
    for (int i=0; i<sebs.size(); i++)
      delete sebs[i];
  }

  /// Validate worker-control placement for a homogeneous portfolio
  forceinline void
  pbscontrol(const Options& opt) {
    if ((opt.assets > 1U) &&
        opt.worker_control.engaged())
      throw WorkerControlInUse("PBS::PBS");
  }

  /// Validate worker-control placement for an explicit portfolio
  forceinline void
  pbscontrol(const SEBs& sebs, const Options& opt) {
    if (opt.worker_control.engaged())
      throw WorkerControlInUse("PBS::PBS");
#ifdef GECODE_HAS_THREADS
    const bool parallel = opt.threads > 1.0;
#else
    const bool parallel = false;
#endif
    for (int i=0; i<sebs.size(); i++) {
      const WorkerControl& x = sebs[i]->options().worker_control;
      if (!x.engaged())
        continue;
      // Sequential PBS cannot move past an asset blocked by a pause request.
      if (!parallel)
        throw WorkerControlInUse("PBS::PBS");
      for (int j=0; j<i; j++) {
        const WorkerControl& y = sebs[j]->options().worker_control;
        if (y.engaged() &&
            WorkerControlAccess::same_identity(x,y))
          throw WorkerControlInUse("PBS::PBS");
      }
    }
  }

  /**
   * \brief Whether engine \a E takes ownership of the cutoff in its options
   * \internal
   */
  template<class E>
  struct PbsOwnsCutoff {
    static constexpr bool value = false;
  };

  /**
   * \brief Restart-based search takes ownership of the cutoff
   * \internal
   */
  template<class T, template<class> class E>
  struct PbsOwnsCutoff<RBS<T,E> > {
    static constexpr bool value = true;
  };

  /**
   * \brief A portfolio takes ownership of the cutoff if its assets do
   * \internal
   */
  template<class T, template<class> class E>
  struct PbsOwnsCutoff<PBS<T,E> > {
    static constexpr bool value = PbsOwnsCutoff<E<T> >::value;
  };

  /**
   * \brief Return the cutoff for the next asset of a portfolio
   *
   * When the assets of engine \a E take ownership of the cutoff \a c,
   * each asset gets its own copy of \a c, and the portfolio deletes \a c
   * once it has been built. Otherwise the assets share \a c and the caller
   * keeps it.
   *
   * \internal
   */
  template<class E>
  forceinline std::unique_ptr<Cutoff>
  pbs_cutoff(const Cutoff* c) {
    if (!PbsOwnsCutoff<E>::value || (c == nullptr))
      return nullptr;
    return std::unique_ptr<Cutoff>(c->copy());
  }

  /**
   * \brief Delete the cutoff \a c of a built portfolio if its assets of
   * engine \a E own copies
   *
   * \internal
   */
  template<class E>
  forceinline void
  pbs_release_cutoff(Cutoff* c) {
    if (PbsOwnsCutoff<E>::value)
      delete c;
  }

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

#include <gecode/search/seq/dead.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop*
  pbsstop(Stop* so);

  /// Create stop object for builder stop \a so and portfolio stop \a po
  GECODE_SEARCH_EXPORT Stop*
  pbsstop(Stop* so, Stop* po);

  /// Create sequential portfolio engine
  GECODE_SEARCH_EXPORT Engine*
  pbsengine(Engine** assets, Stop** stops, unsigned int n_assets,
            const Statistics& stat, const Search::Options& opt, bool best);

}}}

namespace Gecode { namespace Search { namespace Par {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop*
  pbsstop(Stop* so, const WorkerControl& control);

  /// Create stop object for builder stop \a so and portfolio stop \a po
  GECODE_SEARCH_EXPORT Stop*
  pbsstop(Stop* so, Stop* po, const WorkerControl& control);

  /// Create parallel portfolio engine
  GECODE_SEARCH_EXPORT Engine*
  pbsengine(Engine** assets, Stop** stops, unsigned int n_assets,
            const Statistics& stat, bool best);

}}}

namespace Gecode { namespace Search {

  template<class T, template<class> class E>
  Engine*
  pbsseq(T* hub, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    // Retain each seed: a throwing nested builder may consume its own clone.
    opt.clone = true;
    Region r;

    // In case there are more threads than assets requested
    opt.threads = std::max(floor(opt.threads /
                                 static_cast<double>(opt.assets)),1.0);

    unsigned int n_assets = opt.assets;
    Engine** assets = r.alloc<Engine*>(n_assets);
    Stop** stops = r.alloc<Stop*>(n_assets);
    for (unsigned int i=0U; i<n_assets; i++) {
      assets[i] = nullptr;
      stops[i] = nullptr;
    }

    WrapTraceRecorder::engine(opt.tracer,
                              SearchTracer::EngineType::PBS, n_assets);

    Cutoff* cutoff = opt.cutoff;
    Space* unowned_hub = hub;
    try {
      for (unsigned int i=0U; i<n_assets; i++) {
        opt.stop = stops[i] = Seq::pbsstop(nullptr,stop);
        std::unique_ptr<Space> spoke((i == n_assets-1) ?
                                     hub : hub->clone());
        if (spoke.get() == hub)
          unowned_hub = nullptr;
        (void) spoke->spoke(i);
        std::unique_ptr<Cutoff> c(pbs_cutoff<E<T> >(cutoff));
        if (c != nullptr)
          opt.cutoff = c.get();
        assets[i] = build<T,E>(spoke.get(),opt);
        // The asset owns its cutoff
        (void) c.release();
      }
      Engine* e = Seq::pbsengine(
        assets,stops,n_assets,stat,opt,E<T>::best);
      pbs_release_cutoff<E<T> >(cutoff);
      return e;
    } catch (...) {
      pbscleanup(assets,stops,n_assets,unowned_hub);
      throw;
    }
  }

  template<class T, template<class> class E>
  Engine*
  pbsseq(T* hub, SEBs& sebs,
             const Search::Statistics& stat, Options& opt, bool best) {
    Region r;

    int n_assets = sebs.size();
    Engine** assets = r.alloc<Engine*>(n_assets);
    Stop** stops = r.alloc<Stop*>(n_assets);
    for (int i=0; i<n_assets; i++) {
      assets[i] = nullptr;
      stops[i] = nullptr;
    }

    WrapTraceRecorder::engine(opt.tracer,
                              SearchTracer::EngineType::PBS,
                              static_cast<unsigned int>(n_assets));

    Space* unowned_hub = hub;
    bool builders_owned = true;
    try {
      for (int i=0; i<n_assets; i++) {
        // Re-configure asset options, the portfolio stop applies as well
        stops[i] = Seq::pbsstop(sebs[i]->options().stop,opt.stop);
        sebs[i]->options().stop  = stops[i];
        sebs[i]->options().clone = true;
        std::unique_ptr<Space> spoke((i == n_assets-1) ?
                                     hub : hub->clone());
        if (spoke.get() == hub)
          unowned_hub = nullptr;
        (void) spoke->spoke(static_cast<unsigned int>(i));
        assets[i] = (*sebs[i])(spoke.get());
      }
      pbscleanup(sebs);
      builders_owned = false;
      return Seq::pbsengine(
        assets,stops,static_cast<unsigned int>(n_assets),stat,opt,best);
    } catch (...) {
      if (builders_owned)
        pbscleanup(sebs);
      pbscleanup(assets,stops,static_cast<unsigned int>(n_assets),
                 unowned_hub);
      throw;
    }
  }

#ifdef GECODE_HAS_THREADS

  template<class T, template<class> class E>
  Engine*
  pbspar(T* hub, const Search::Statistics& stat, Options& opt) {
    Stop* stop = opt.stop;
    // Retain each seed: a throwing nested builder may consume its own clone.
    opt.clone = true;
    Region r;

    // Run every asset in its own thread, even with fewer threads than assets
    unsigned int n_assets = opt.assets;
    // Redistribute additional threads to assets
    opt.threads = std::max(floor(opt.threads / static_cast<double>(n_assets)), 1.0);

    WrapTraceRecorder::engine(opt.tracer,
                              SearchTracer::EngineType::PBS, n_assets);

    Engine** assets = r.alloc<Engine*>(n_assets);
    Stop** stops = r.alloc<Stop*>(n_assets);
    for (unsigned int i=0U; i<n_assets; i++) {
      assets[i] = nullptr;
      stops[i] = nullptr;
    }

    Cutoff* cutoff = opt.cutoff;
    Space* unowned_hub = hub;
    try {
      for (unsigned int i=0U; i<n_assets; i++) {
        opt.stop = stops[i] = Par::pbsstop(nullptr,stop,opt.worker_control);
        std::unique_ptr<Space> spoke((i == n_assets-1) ?
                                     hub : hub->clone());
        if (spoke.get() == hub)
          unowned_hub = nullptr;
        (void) spoke->spoke(static_cast<unsigned int>(i));
        std::unique_ptr<Cutoff> c(pbs_cutoff<E<T> >(cutoff));
        if (c != nullptr)
          opt.cutoff = c.get();
        assets[i] = build<T,E>(spoke.get(),opt);
        // The asset owns its cutoff
        (void) c.release();
      }
      Engine* e = Par::pbsengine(assets,stops,n_assets,stat,E<T>::best);
      pbs_release_cutoff<E<T> >(cutoff);
      return e;
    } catch (...) {
      pbscleanup(assets,stops,n_assets,unowned_hub);
      throw;
    }
  }

  template<class T, template<class> class E>
  Engine*
  pbspar(T* hub, SEBs& sebs,
         const Search::Statistics& stat, Options& opt, bool best) {
    Region r;

    // Run every asset in its own thread, even with fewer threads than assets
    int n_assets = sebs.size();

    WrapTraceRecorder::engine(opt.tracer,
                              SearchTracer::EngineType::PBS,
                              static_cast<unsigned int>(n_assets));

    Engine** assets = r.alloc<Engine*>(n_assets);
    Stop** stops = r.alloc<Stop*>(n_assets);
    for (int i=0; i<n_assets; i++) {
      assets[i] = nullptr;
      stops[i] = nullptr;
    }

    Space* unowned_hub = hub;
    bool builders_owned = true;
    try {
      for (int i=0; i<n_assets; i++) {
        // Re-configure asset options, the portfolio stop applies as well
        stops[i] = Par::pbsstop(sebs[i]->options().stop,opt.stop,
                                sebs[i]->options().worker_control);
        sebs[i]->options().stop  = stops[i];
        sebs[i]->options().clone = true;
        std::unique_ptr<Space> spoke((i == n_assets-1) ?
                                     hub : hub->clone());
        if (spoke.get() == hub)
          unowned_hub = nullptr;
        (void) spoke->spoke(static_cast<unsigned int>(i));
        assets[i] = (*sebs[i])(spoke.get());
      }
      pbscleanup(sebs);
      builders_owned = false;
      return Par::pbsengine(
        assets,stops,static_cast<unsigned int>(n_assets),stat,best);
    } catch (...) {
      if (builders_owned)
        pbscleanup(sebs);
      pbscleanup(assets,stops,static_cast<unsigned int>(n_assets),
                 unowned_hub);
      throw;
    }
  }

#endif

}}

namespace Gecode {

  template<class T, template<class> class E>
  PBS<T,E>::PBS(T* s, const Search::Options& o) {
    Search::Options opt(o.expand());

    if (opt.assets == 0)
      throw Search::NoAssets("PBS::PBS");
    Search::pbscontrol(opt);

    Search::Statistics stat;

    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!opt.clone)
        delete s;
      e = Search::Seq::dead(opt,stat);
      /*
       * The assets would own the cutoff, but a dead engine does not take
       * it. This includes the cutoff of an outer portfolio when this is
       * one of its assets.
       */
      if (Search::PbsOwnsCutoff<E<T> >::value)
        delete opt.cutoff;
      return;
    }

    // Check whether a clone must be used
    T* hub = opt.clone ?
      dynamic_cast<T*>(s->clone()) : s;
    opt.clone = false;

    // Always execute hub function
    (void) hub->hub(0);

    // No need to create a portfolio engine but must run spoke function
    if (o.assets == 1) {
      std::unique_ptr<T> seed(hub);
      (void) hub->spoke(0);
      // Retain the seed if a nested builder rejects its worker control.
      opt.clone = true;
      e = Search::build<T,E>(seed.get(),opt);
      return;
    }

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::pbspar<T,E>(hub,stat,opt);
    else
#endif
      e = Search::pbsseq<T,E>(hub,stat,opt);
  }

  template<class T, template<class> class E>
  void
  PBS<T,E>::build(T* s, SEBs& sebs, const Search::Options& o) {
    Search::Options opt(o.expand());
    Search::pbscontrol(sebs,opt);

    // Check whether all sebs do either best solution search or not
    bool best;
    {
      int b = 0;
      for (int i=0; i<sebs.size(); i++)
        b += sebs[i]->best() ? 1 : 0;
      if ((b > 0) && (b < sebs.size()))
        throw Search::MixedBest("PBS::PBS");
      best = (b == sebs.size());
    }

    Search::Statistics stat;

    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!opt.clone)
        delete s;
      Search::pbscleanup(sebs);
      e = Search::Seq::dead(opt,stat);
      return;
    }

    // Check whether a clone must be used
    T* hub = opt.clone ?
      dynamic_cast<T*>(s->clone()) : s;
    opt.clone = false;

    // Always execute hub function
    (void) hub->hub(0);

#ifdef GECODE_HAS_THREADS
    if (opt.threads > 1.0)
      e = Search::pbspar<T,E>(hub,sebs,stat,opt,best);
    else
#endif
      e = Search::pbsseq<T,E>(hub,sebs,stat,opt,best);
  }

  template<class T, template<class> class E>
  inline
  PBS<T,E>::PBS(T* s, SEBs& sebs, const Search::Options& o) {
    build(s,sebs,o);
  }

  template<class T, template<class> class E>
  inline void
  PBS<T,E>::constrain(const T& b) {
    e->constrain(b);
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

// STATISTICS: search-other
