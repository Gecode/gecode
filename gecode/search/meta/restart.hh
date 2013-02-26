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


#ifndef __GECODE_SEARCH_META_RESTART_HH__
#define __GECODE_SEARCH_META_RESTART_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Meta {

  class Restart : public Engine {
  private:
    /// The actual engine
    Engine* e;
    /// The master space to restart from
    Space* master;
    /// The cutoff object
    Cutoff* co;
    /// The stop control object
    MetaStop* stop;
  public:
    /// Constructor
    Restart(Space*, size_t, Cutoff* co0, MetaStop* stop0,
            Engine* e0, const Options& o);
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Reset engine to restart at space \a s and return new root
    virtual Space* reset(Space* s);
    /// Destructor
    virtual ~Restart(void);
  };

  inline
  Restart::Restart(Space* s, size_t, Cutoff* co0, MetaStop* stop0,
                   Engine* e0, const Options&)
    : e(e0), master(s), co(co0), stop(stop0) {
    stop->limit(Statistics(),(*co)());
  }

  inline Space*
  Restart::next(void) {
    while (true) {
      Space* n = e->next();
      unsigned long int i = stop->m_stat.restart;
      if (n != NULL) {
        master->constrain(*n);
        master->master(i,n);
        if (master->status(stop->m_stat) == SS_FAILED) {
          delete master;
          master = NULL;
          (void) e->reset(NULL);
        } else {
          Space* slave = master->clone();
          slave->slave(i);
          (void) e->reset(slave);
        }
        return n;
      } else if (e->stopped() && stop->enginestopped()) {
        master->master(i,NULL);
        long unsigned int nl = (*co)();
        stop->limit(e->statistics(),nl);
        if (master->status(stop->m_stat) == SS_FAILED)
          return NULL;
        Space* slave = master->clone();
        slave->slave(i);
        (void) e->reset(slave);
      } else {
        return NULL;
      }
    }
    GECODE_NEVER;
    return NULL;
  }
  
  inline Search::Statistics
  Restart::statistics(void) const {
    return stop->metastatistics()+e->statistics();
  }
  
  inline bool
  Restart::stopped(void) const {
    return e->stopped() && !stop->enginestopped();
  }
  
  inline Space*
  Restart::reset(Space*) { 
    return NULL; 
  }
  
  inline
  Restart::~Restart(void) {
    // Deleting e also deletes stop
    delete e;
    delete master;
    delete co;
  }

}}}

#endif

// STATISTICS: search-other
