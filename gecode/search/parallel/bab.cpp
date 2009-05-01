/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#include <gecode/search.hh>

namespace Gecode { namespace Search { namespace Parallel {

  /// Implementation of depth-first branch-and-bound search engine
  class BAB : public Worker {
  private:
    /// Search options
    Options opt;
    /// Current path in search tree
    Path path;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
    /// Number of entries not yet constrained to be better
    int mark;
    /// Best solution found so far
    Space* best;
  public:
    /// Initialize with space \a s (of size \a sz) and search options \a o
    BAB(Space* s, size_t sz, const Options& o);
    /// %Search for next better solution
    Space* next(void);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~BAB(void);
  };

  BAB::BAB(Space* s, size_t sz, const Options& o)
    : Worker(sz), opt(o), d(0), mark(0), best(NULL) {
    cur = (s->status(*this) == SS_FAILED) ? NULL : snapshot(s,opt);
    current(s);
    current(NULL);
    current(cur);
    if (cur == NULL)
      fail++;
  }

  Space*
  BAB::next(void) {
    /*
     * The invariant maintained by the engine is:
     *   For all nodes stored at a depth less than mark, there
     *   is no guarantee of betterness. For those above the mark,
     *   betterness is guaranteed.
     *
     * The engine maintains the path on the stack for the current
     * node to be explored.
     *
     */
    start();
    while (true) {
      while (cur) {
        if (stop(opt.stop,path.size()))
          return NULL;
        node++;
        switch (cur->status(*this)) {
        case SS_FAILED:
          fail++;
          delete cur;
          cur = NULL;
          Worker::current(NULL);
          break;
        case SS_SOLVED:
          // Deletes all pending branchings
          (void) cur->description();
          delete best;
          best = cur;
          cur = NULL;
          mark = path.entries();
          Worker::current(NULL);
          return best->clone();
        case SS_BRANCH:
          {
            Space* c;
            if ((d == 0) || (d >= opt.c_d)) {
              c = cur->clone();
              d = 1;
            } else {
              c = NULL;
              d++;
            }
            const BranchingDesc* desc = path.push(*this,cur,c);
            Worker::push(c,desc);
            cur->commit(*desc,0);
            break;
          }
        default:
          GECODE_NEVER;
        }
      }
      // Recompute and add constraint if necessary
      do {
        if (!path.next(*this))
          return NULL;
        cur = path.recompute(d,opt.a_d,*this,best,mark);
      } while (cur == NULL);
      Worker::current(cur);
    }
    GECODE_NEVER;
    return NULL;
  }

  Statistics
  BAB::statistics(void) const {
    Statistics s = *this;
    s.memory += path.size();
    return s;
  }

  BAB::~BAB(void) {
    path.reset();
    delete best;
    delete cur;
  }


  // Create branch and bound engine
  Engine* bab(Space* s, size_t sz, const Options& o) {
    return new WorkerToEngine<BAB>(s,sz,o);
  }


}}}

// STATISTICS: search-any
