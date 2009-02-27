/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2003
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

namespace Gecode { namespace Search {

  DFS::DFS(Space* s, size_t sz, const Options& o)
    : Engine(sz), opt(o), d(0) {
    cur = (s->status(*this) == SS_FAILED) ? NULL : snapshot(s,opt);
    current(s);
    current(NULL);
    current(cur);
    if (cur == NULL)
      fail++;
  }

  void
  DFS::reset(Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if (s->status(*this) == SS_FAILED) {
      cur = NULL;
      Engine::reset();
    } else {
      cur = s->clone();
      Engine::reset(cur);
    }
  }

  Space*
  DFS::next(void) {
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
          Engine::current(NULL);
          break;
        case SS_SOLVED:
          {
            // Deletes all pending branchings
            (void) cur->description();
            Space* s = cur;
            cur = NULL;
            Engine::current(NULL);
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
            const BranchingDesc* desc = path.push(*this,cur,c);
            Engine::push(c,desc);
            cur->commit(*desc,0);
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
      Engine::current(cur);
    }
    GECODE_NEVER;
    return NULL;
  }

  Statistics
  DFS::statistics(void) const {
    Statistics s = *this;
    s.memory += path.size();
    return s;
  }

  DFS::~DFS(void) {
    delete cur;
    path.reset();
  }

}}

// STATISTICS: search-any
