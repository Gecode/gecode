/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

namespace Gecode { namespace Search { namespace Seq {

  template<class Tracer>
  forceinline
  DFS<Tracer>::DFS(Space* s, const Options& o)
    : tracer(o.tracer), opt(o), path(opt.nogoods_limit), d(0) {
    if (tracer) {
      tracer.engine(SearchTracer::EngineType::DFS, 1U);
      tracer.worker();
    }
    if ((s == NULL) || (s->status(*this) == SS_FAILED)) {
      fail++;
      cur = NULL;
      if (!opt.clone)
        delete s;
    } else {
      cur = snapshot(s,opt);
    }
  }

  template<class Tracer>
  forceinline void
  DFS<Tracer>::reset(Space* s) {
    tracer.round();
    delete cur;
    path.reset();
    d = 0;
    if ((s == NULL) || (s->status(*this) == SS_FAILED)) {
      delete s;
      cur = NULL;
    } else {
      cur = s;
    }
    Worker::reset();
  }

  template<class Tracer>
  forceinline NoGoods&
  DFS<Tracer>::nogoods(void) {
    return path;
  }

  template<class Tracer>
  forceinline Space*
  DFS<Tracer>::next(void) {
    /*
     * The engine maintains the following invariant:
     *  - If the current space (cur) is not NULL, the path always points
     *    to exactly that space.
     *  - If the current space (cur) is NULL, the path always points
     *    to the next space (if there is any).
     *
     * This invariant is needed so that no-goods can be extracted properly
     * when the engine is stopped or has found a solution.
     *
     */
    start();
    while (true) {
      if (stop(opt))
        return NULL;
      while (cur == NULL) {
        if (path.empty())
          return NULL;
        cur = path.recompute(d,opt.a_d,*this,tracer);
        if (cur != NULL)
          break;
        path.next();
      }
      node++;
      SearchTracer::EdgeInfo ei;
      if (tracer && (path.entries() > 0)) {
        typename Path<Tracer>::Edge& top = path.top();
        ei.init(tracer.wid(), top.nid(), top.truealt(), *cur, *top.choice());
      }
      unsigned int nid = tracer.nid();
      switch (cur->status(*this)) {
      case SS_FAILED:
        if (tracer) {
          SearchTracer::NodeInfo ni(SearchTracer::NodeType::FAILED,
                                    tracer.wid(), nid, *cur);
          tracer.node(ei,ni);
        }
        fail++;
        delete cur;
        cur = NULL;
        path.next();
        break;
      case SS_SOLVED:
        {
          if (tracer) {
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::SOLVED,
                                      tracer.wid(), nid, *cur);
            tracer.node(ei,ni);
          }
          // Deletes all pending branchers
          (void) cur->choice();
          Space* s = cur;
          cur = NULL;
          path.next();
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
          const Choice* ch = path.push(*this,cur,c,nid);
          if (tracer) {
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::BRANCH,
                                      tracer.wid(), nid, *cur, ch);
            tracer.node(ei,ni);
          }
          cur->commit(*ch,0);
          break;
        }
      default:
        GECODE_NEVER;
      }
    }
    GECODE_NEVER;
    return NULL;
  }

  template<class Tracer>
  forceinline Statistics
  DFS<Tracer>::statistics(void) const {
    return *this;
  }

  template<class Tracer>
  forceinline void
  DFS<Tracer>::constrain(const Space& b) {
    (void) b;
    assert(false);
  }

  template<class Tracer>
  forceinline
  DFS<Tracer>::~DFS(void) {
    delete cur;
    tracer.done();
    path.reset();
  }

}}}

// STATISTICS: search-seq
