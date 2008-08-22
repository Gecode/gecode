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

namespace Gecode {

  namespace Search {

    /*
     * DFS engine
     *
     */
    forceinline
    DfsEngine::DfsEngine(unsigned int c_d0, unsigned int a_d0,
                         Stop* st, size_t sz)
      : EngineCtrl(st,sz), rcs(a_d0), cur(NULL), c_d(c_d0), d(0) {}


    forceinline void
    DfsEngine::init(Space* s) {
      cur = s;
    }

    forceinline void
    DfsEngine::reset(Space* s) {
      delete cur;
      rcs.reset();
      cur = s;
      d   = 0;
      EngineCtrl::reset(s);
    }

    forceinline void
    DfsEngine::reset(void) {
      delete cur;
      rcs.reset();
      cur = NULL;
      d   = 0;
      EngineCtrl::reset();
    }

    forceinline size_t
    DfsEngine::stacksize(void) const {
      return rcs.stacksize();
    }

    forceinline Space*
    DfsEngine::explore(void) {
      start();
      while (true) {
        while (cur) {
          if (stop(stacksize()))
            return NULL;
          switch (cur->status(propagate)) {
          case SS_FAILED:
            fail++;
            delete cur;
            cur = NULL;
            EngineCtrl::current(NULL);
            break;
          case SS_SOLVED:
            {
              Space* s = cur;
              cur = NULL;
              EngineCtrl::current(NULL);
              return s;
            }
          case SS_BRANCH:
            {
              Space* c;
              if ((d == 0) || (d >= c_d)) {
                clone++;
                c = cur->clone();
                d = 1;
              } else {
                c = NULL;
                d++;
              }
              const BranchingDesc* desc = rcs.push(cur,c);
              EngineCtrl::push(c,desc);
              commit++;
              cur->commit(*desc,0);
              break;
            }
          default: GECODE_NEVER;
          }
        }
        if (!rcs.next(*this))
          return NULL;
        cur = rcs.recompute<false>(d,*this);
        EngineCtrl::current(cur);
      }
      return NULL;
    }

    forceinline
    DfsEngine::~DfsEngine(void) {
      delete cur;
      rcs.reset();
    }

  }

  /*
   * Control for DFS search engine
   *
   */

  template <class T>
  forceinline
  DFS<T>::DFS(T* s, const Search::Options& o)
    : Search::DFS(s,o.c_d,o.a_d,o.stop,sizeof(T)) {}

  template <class T>
  forceinline T*
  DFS<T>::next(void) {
    Space* s = Search::DFS::next();
    if (s == NULL)
      return NULL;
    T* t = dynamic_cast<T*>(s);
    if (t == NULL)
      throw DynamicCastFailed("DFS");
    return t;
  }



  /*
   * DFS convenience
   *
   */

  template <class T>
  forceinline T*
  dfs(T* s, const Search::Options& o) {
    DFS<T> d(s,o);
    return d.next();
  }

}

// STATISTICS: search-any
