/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Guido Tack, 2012
 *
 *  Last modified:
 *     $Date: 2012-08-29 12:29:14 +0200 (Wed, 29 Aug 2012) $ by $Author: schulte $
 *     $Revision: 13015 $
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


#ifndef __GECODE_SEARCH_SEQUENTIAL_RESTART_HH__
#define __GECODE_SEARCH_SEQUENTIAL_RESTART_HH__

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Sequential {

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
    Restart(Space* s, size_t, Cutoff* co0, MetaStop* stop0,
            Engine* e0, const Options&)
      : e(e0), master(s), co(co0), stop(stop0) {
      stop->limit(Statistics(),(*co)());
    }
    /// Return next solution (NULL, if none exists or search has been stopped)
    virtual Space* next(void);
    /// Return statistics
    virtual Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    virtual bool stopped(void) const;
    /// Reset engine to restart at space \a s and return new root
    virtual Space* reset(Space* s);
    /// Return reference to deepest space on the stack
    const Space& deepest(void) const;
    /// Destructor
    virtual ~Restart(void);
  };

  inline Space*
  Restart::next(void) {
    for (;;) {
      Space* n = e->next();
      if (n != NULL) {
        master->configure(*n);
        if (master->status() == SS_FAILED) {
          std::cerr << "MASTER FAILED AFTER SOL\n";
          delete master;
          master = NULL;
          e->reset(NULL);
        } else {
          e->reset(master->clone());
        }
        return n;
      } else if (e->stopped() && stop->enginestopped()) {
        master->configure(e->deepest());
        long unsigned int nl = (*co)();
        std::cerr << "restart with " << nl << std::endl;
        stop->limit(e->statistics(),nl);
        if (master->status() == SS_FAILED) {
          std::cerr << "MASTER FAILED\n";
          return NULL;
        }
        e->reset(master->clone());
      } else {
        return NULL;
      }
    }
  }
  
  inline Search::Statistics
  Restart::statistics(void) const {
    return stop->metastatistics()+e->statistics();
  }
  
  inline bool
  Restart::stopped(void) const {
    return e->stopped();
  }
  
  inline Space*
  Restart::reset(Space*) { return NULL; }
  
  inline
  Restart::~Restart(void) {
    // Deleting e also deletes stop
    delete e;
    delete master;
    delete co;
  }

  inline const Space&
  Restart::deepest(void) const {
    return e->deepest();
  }

}}}

#endif

// STATISTICS: search-other
