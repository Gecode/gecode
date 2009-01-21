/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *     Guido Tack, 2004
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

namespace Gecode { namespace Search {

  forceinline
  BabEngine::BabEngine(const Options& o,size_t sz)
    : EngineCtrl(sz), opt(o), cur(NULL), d(0),
      mark(0), best(NULL) {}

  forceinline void
  BabEngine::init(Space* s) {
    cur = s;
  }

  forceinline Space*
  BabEngine::explore(void) {
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
        if (stop(opt.stop,stacksize()))
          return NULL;
        node++;
        switch (cur->status(*this)) {
        case SS_FAILED:
          fail++;
          delete cur;
          cur = NULL;
          EngineCtrl::current(NULL);
          break;
        case SS_SOLVED:
          delete best;
          best = cur;
          cur = NULL;
          mark = rcs.entries();
          EngineCtrl::current(NULL);
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
            const BranchingDesc* desc = rcs.push(*this,cur,c);
            EngineCtrl::push(c,desc);
            cur->commit(*desc,0);
            break;
          }
        default:
          GECODE_NEVER;
        }
      }
      // Recompute and add constraint if necessary
      do {
        if (!rcs.next(*this))
          return NULL;
        cur = rcs.recompute(d,opt.a_d,*this,best,mark);
      } while (cur == NULL);
      EngineCtrl::current(cur);
    }
    GECODE_NEVER;
    return NULL;
  }

  forceinline size_t
  BabEngine::stacksize(void) const {
    return rcs.stacksize();
  }

  forceinline
  BabEngine::~BabEngine(void) {
    rcs.reset();
    delete best;
    delete cur;
  }

}}

// STATISTICS: search-any
