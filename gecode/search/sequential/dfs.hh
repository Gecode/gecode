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

#ifndef __GECODE_SEARCH_SEQUENTIAL_DFS_HH__
#define __GECODE_SEARCH_SEQUENTIAL_DFS_HH__

#include <gecode/search.hh>
#include <gecode/search/support.hh>
#include <gecode/search/worker.hh>
#include <gecode/search/sequential/path.hh>

namespace Gecode { namespace Search { namespace Sequential {

  /// Depth-first search engine implementation
  class DFS : public Worker {
  private:
    /// Search options
    Options opt;
    /// Current path ins search tree
    Path path;
    /// Current space being explored
    Space* cur;
    /// Distance until next clone
    unsigned int d;
  protected:
    /// Reset engine to restart at space \a s and return new root
    Space* reset(Space* s);
  public:
    /// Initialize for space \a s (of size \a sz) with options \a o
    DFS(Space* s, size_t sz, const Options& o);
    /// %Search for next solution
    Space* next(void);
    /// Return statistics
    Statistics statistics(void) const;
    /// Destructor
    ~DFS(void);
  };

  forceinline 
  DFS::DFS(Space* s, size_t sz, const Options& o)
    : Worker(sz), opt(o), d(0) {
    current(s);
    if (s->status(*this) == SS_FAILED) {
      fail++;
      cur = NULL;
      if (!o.clone)
        delete s;
    } else {
      cur = snapshot(s,opt);
    }
    current(NULL);
    current(cur);
  }

  forceinline Space*
  DFS::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if (s->status(*this) == SS_FAILED) {
      cur = NULL;
      Worker::reset();
      return NULL;
    } else {
      cur = s;
      Worker::reset(cur);
      return cur->clone();
    }
  }

  forceinline Space*
  DFS::next(void) {
    start();
    while (true) {
      while (cur) {
        if (stop(opt,path.size()))
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
          {
            // Deletes all pending branchers
            (void) cur->choice();
            Space* s = cur;
            cur = NULL;
            Worker::current(NULL);
            return s;
          }
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
            const Choice* ch = path.push(*this,cur,c);
            Worker::push(c,ch);
            cur->commit(*ch,0);
            break;
          }
        default:
          GECODE_NEVER;
        }
      }
      do {
        if (!path.next(*this))
          return NULL;
        cur = path.recompute(d,opt.a_d,*this);
      } while (cur == NULL);
      Worker::current(cur);
    }
    GECODE_NEVER;
    return NULL;
  }

  forceinline Statistics
  DFS::statistics(void) const {
    Statistics s = *this;
    s.memory += path.size();
    return s;
  }

  forceinline 
  DFS::~DFS(void) {
    delete cur;
    path.reset();
  }

}}}

#endif

// STATISTICS: search-sequential
