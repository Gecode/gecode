/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2014
 *     Guido Tack, 2012
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

#include <gecode/search/support.hh>
#include <gecode/search/seq/dead.hh>
#include <memory>

namespace Gecode { namespace Search { namespace Seq {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop*
  rbsstop(Stop* so);

  /// Create restart engine whose first spoke has SpokeCoverage::Full
  GECODE_SEARCH_EXPORT Engine*
  rbsengine(Space* hub, Stop* stop, Engine* engine,
            const Search::Statistics& stat, const Options& opt,
            bool best);

  /**
   * \brief Create restart engine
   *
   * The value \a coverage is the coverage that the spoke hook declared for
   * the space explored by the first \a engine.
   */
  GECODE_SEARCH_EXPORT Engine*
  rbsengine(Space* hub, Stop* stop, Engine* engine,
            const Search::Statistics& stat, const Options& opt,
            bool best, SpokeCoverage coverage);

}}}

namespace Gecode { namespace Search {

  /// A RBS engine builder
  template<class T, template<class> class E>
  class RbsBuilder : public Builder {
    using Builder::opt;
  public:
    /// The constructor
    RbsBuilder(const Options& opt);
    /// The actual build function
    virtual Engine* operator() (Space* s) const;
  };

  template<class T, template<class> class E>
  inline
  RbsBuilder<T,E>::RbsBuilder(const Options& opt)
    : Builder(opt,E<T>::best) {}

  template<class T, template<class> class E>
  Engine*
  RbsBuilder<T,E>::operator() (Space* s) const {
    return build<T,RBS<T,E> >(s,opt);
  }

}}

namespace Gecode {

  template<class T, template<class> class E>
  inline
  RBS<T,E>::RBS(T* s, const Search::Options& m_opt) {
    if (m_opt.cutoff == nullptr)
      throw Search::UninitializedCutoff("RBS::RBS");
    Search::Options e_opt(m_opt.expand());
    Search::Statistics stat;
    std::unique_ptr<Search::Stop> stop(Search::Seq::rbsstop(m_opt.stop));
    e_opt.stop = stop.get();
    // Retain the seed while a potentially nested builder owns its clone.
    e_opt.clone = true;
    Search::WrapTraceRecorder::engine(e_opt.tracer,
                                      SearchTracer::EngineType::RBS, 1U);
    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!m_opt.clone)
        delete s;
      e = Search::Seq::dead(e_opt,stat,Search::EndReason::Exhausted);
      // The engine owns the cutoff, but a dead engine does not take it
      delete m_opt.cutoff;
    } else {
      std::unique_ptr<Space> hub(m_opt.clone ? s->clone() : s);
      Search::Statistics start;
      HubContext hc = Search::MetaContextAccess::hub(
        MetaEngine::Restart,MetaEvent::Start,MetaDecision::Restart,0UL,0U,1U,
        nullptr,start,NoGoods::eng);
      hub->hub(hc);
      if ((hub->status(stat) == SS_FAILED) ||
          (hc.decision() == MetaDecision::Finish)) {
        // A failed hub is a proof, finishing is not
        Search::EndReason r = hub->failed() ?
          Search::EndReason::Exhausted : Search::EndReason::Incomplete;
        if (r == Search::EndReason::Exhausted)
          stat.fail++;
        e = Search::Seq::dead(e_opt,stat,r);
        delete m_opt.cutoff;
        return;
      }
      std::unique_ptr<Space> spoke(hub->clone());
      SpokeContext sc = Search::MetaContextAccess::spoke(
        MetaEngine::Restart,MetaEvent::Start,0UL,0U,1U,nullptr,start,
        NoGoods::eng);
      spoke->spoke(sc);
      spoke->random_split(sc);
      hub->reserve_brancher_ids(*spoke);
      std::unique_ptr<Search::Engine> leaf(
        Search::build<T,E>(spoke.get(),e_opt));
      e = Search::Seq::rbsengine(hub.get(),stop.get(),leaf.get(),
                                stat,m_opt,E<T>::best,sc.coverage());
      hub.release();
      stop.release();
      leaf.release();
    }
  }


  template<class T, template<class> class E>
  inline T*
  rbs(T* s, const Search::Options& o) {
    RBS<T,E> r(s,o);
    return r.next();
  }

  template<class T, template<class> class E>
  SEB
  rbs(const Search::Options& o) {
    if (o.cutoff == nullptr)
      throw Search::UninitializedCutoff("rbs");
    return new Search::RbsBuilder<T,E>(o);
  }


}

// STATISTICS: search-seq
