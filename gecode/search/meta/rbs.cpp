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


#include <gecode/search/meta/rbs.hh>

namespace Gecode { namespace Search { namespace Meta {

  Space*
  RBS::next(void) {
    while (true) {
      Space* n = e->next();
      unsigned long int i = stop->m_stat.restart;
      if (n != NULL) {
        master->constrain(*n);
        master->master(i,n);
        if (master->status(stop->m_stat) == SS_FAILED) {
          delete master;
          master = NULL;
          e->reset(NULL);
        } else {
          Space* slave = master;
          master = master->clone(shared);
          slave->slave(i,n);
          e->reset(slave);
        }
        return n;
      } else if (e->stopped() && stop->enginestopped()) {
        master->master(i,NULL);
        long unsigned int nl = (*co)();
        stop->limit(e->statistics(),nl);
        if (master->status(stop->m_stat) == SS_FAILED)
          return NULL;
        Space* slave = master;
        master = master->clone(shared);
        slave->slave(i,n);
        e->reset(slave);
      } else {
        return NULL;
      }
    }
    GECODE_NEVER;
    return NULL;
  }
  
  Search::Statistics
  RBS::statistics(void) const {
    return stop->metastatistics()+e->statistics();
  }
  
  bool
  RBS::stopped(void) const {
    return e->stopped() && !stop->enginestopped();
  }
  
  void
  RBS::reset(Space*) { 
  }
  
  RBS::~RBS(void) {
    // Deleting e also deletes stop
    delete e;
    delete master;
    delete co;
  }

}}}

// STATISTICS: search-other
