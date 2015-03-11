/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2012
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

#ifndef __GECODE_SEARCH_META_RBS_HH__
#define __GECODE_SEARCH_META_RBS_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Meta {

  class RBS;

  /// %Stop-object for restart engine
  class GECODE_SEARCH_EXPORT RestartStop : public Stop {
    template<template<class>class,class> friend class ::Gecode::RBS;
    friend class ::Gecode::Search::Meta::RBS;
  private:
    /// The failure stop object for the engine
    FailStop* e_stop;
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
    void limit(const Search::Statistics& s, unsigned long int l);
    /// Update statistics
    void update(const Search::Statistics& s);
    /// Return the stop object to control the engine
    Stop* enginestop(void) const;
    /// Return whether the engine has been stopped
    bool enginestopped(void) const;
    /// Return statistics for the meta engine
    Statistics metastatistics(void) const;
    /// Delete object
    ~RestartStop(void);
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
    /// Whether the slave can be shared with the master
    bool shared;
    /// Whether search for the next solution will be complete
    bool complete;
    /// Whether a restart must be performed when next is called
    bool restart;
  public:
    /// Constructor
    RBS(Space* s, RestartStop* stop0, Engine* e0, const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Destructor
    virtual ~RBS(void);
  };

  /*
   * Stopping for meta search engines
   *
   */

  forceinline
  RestartStop::RestartStop(Stop* s) 
    : e_stop(new FailStop(0)), m_stop(s), e_stopped(false) {}

  forceinline void
  RestartStop::limit(const Search::Statistics& s, unsigned long int l) {
    m_stat += s;
    e_stopped = false;
    e_stop->limit(l);
  }

  forceinline void
  RestartStop::update(const Search::Statistics& s) {
    m_stat += s;
  }

  forceinline Stop*
  RestartStop::enginestop(void) const { 
    return e_stop; 
  }

  forceinline bool
  RestartStop::enginestopped(void) const { 
    return e_stopped; 
  }

  forceinline Statistics 
  RestartStop::metastatistics(void) const { 
    return m_stat; 
  }

  forceinline
  RestartStop::~RestartStop(void) {
    delete e_stop;
    delete m_stop;
  }



  forceinline
  RBS::RBS(Space* s, RestartStop* stop0,
           Engine* e0, const Options& opt)
    : e(e0), master(s), last(NULL), co(opt.cutoff), stop(stop0),
      sslr(0),
      shared(opt.threads == 1), complete(true), restart(false) {
    stop->limit(Statistics(),(*co)());
  }

}}}

#endif

// STATISTICS: search-other
