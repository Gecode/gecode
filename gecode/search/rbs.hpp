/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2014
 *     Guido Tack, 2012
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

#include <gecode/search/support.hh>
#include <gecode/search/seq/dead.hh>

namespace Gecode { namespace Search { namespace Seq {

  /// Create stop object
  GECODE_SEARCH_EXPORT Stop*
  rbsstop(Stop* so);

  /// Create restart engine
  GECODE_SEARCH_EXPORT Engine*
  rbsengine(Space* master, Stop* stop, Engine* slave,
            const Search::Statistics& stat, const Options& opt,
            bool best);

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
    if (m_opt.cutoff == NULL)
      throw Search::UninitializedCutoff("RBS::RBS");
    Search::Options e_opt(m_opt.expand());
    Search::Statistics stat;
    e_opt.clone = false;
    e_opt.stop  = Search::Seq::rbsstop(m_opt.stop);
    Search::WrapTraceRecorder::engine(e_opt.tracer,
                                      SearchTracer::EngineType::RBS, 1U);
    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!m_opt.clone)
        delete s;
      e = Search::Seq::dead(e_opt, stat);
    } else {
      Space* master = m_opt.clone ? s->clone() : s;
      Space* slave  = master->clone();
      MetaInfo mi(0,0,0,NULL,NoGoods::eng);
      slave->slave(mi);
      e = Search::Seq::rbsengine(master,e_opt.stop,
                                 Search::build<T,E>(slave,e_opt),
                                 stat,m_opt,E<T>::best);
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
    if (o.cutoff == NULL)
      throw Search::UninitializedCutoff("rbs");
    return new Search::RbsBuilder<T,E>(o);
  }


}

// STATISTICS: search-seq
