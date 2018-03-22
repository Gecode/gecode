/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
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

#ifndef __GECODE_SEARCH_SEQ_RBS_HH__
#define __GECODE_SEARCH_SEQ_RBS_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Seq {

  class RBS;

  /// %Stop-object for restart engine
  class GECODE_SEARCH_EXPORT RestartStop : public Stop {
    template<class,template<class>class> friend class ::Gecode::RBS;
    friend class ::Gecode::Search::Seq::RBS;
  private:
    /// The failure limit for the engine
    unsigned long int l;
    /// The stop object for the meta engine
    Stop* m_stop;
    /// Whether the engine was stopped
    bool e_stopped;
    /// Accumulated statistics for the meta engine
    Statistics m_stat;
  public:
    /// Stop the meta engine if indicated by the stop object \a s
    RestartStop(Stop* s);
    /// Return true if meta engine must be stopped
    virtual bool stop(const Statistics& s, const Options& o);
    /// Set current limit for the engine to \a l fails
    void limit(const Statistics& s, unsigned long int l);
    /// Update statistics
    void update(const Search::Statistics& s);
    /// Return whether the engine has been stopped
    bool enginestopped(void) const;
    /// Return statistics for the meta engine
    Statistics metastatistics(void) const;
  };

  /// Engine for restart-based search
  class GECODE_SEARCH_EXPORT RBS : public Engine {
  protected:
    /// The actual engine
    Engine* e;
    /// The master space to restart from
    Space* master;
    /// The last solution space (possibly NULL)
    Space* last;
    /// The cutoff object
    Cutoff* co;
    /// The stop control object
    RestartStop* stop;
    /// How many solutions since the last restart
    unsigned long int sslr;
    /// Whether the slave can share info (AFC) with the master
    bool shared_info;
    /// Whether search for the next solution will be complete
    bool complete;
    /// Whether a restart must be performed when next is called
    bool restart;
    /// Whether the engine performs best solution search
    bool best;
  public:
    /// Constructor
    RBS(Space* s, RestartStop* stop0, Engine* e0,
        const Search::Statistics& stat, const Options& o, bool best);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Constrain future solutions to be better than \a b
    virtual void constrain(const Space& b);
    /// Destructor
    virtual ~RBS(void);
  };

}}}

#include <gecode/search/seq/rbs.hpp>

#endif

// STATISTICS: search-seq
