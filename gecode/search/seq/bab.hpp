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
  BAB<Tracer>::BAB(Space* s, const Options& o)
    : tracer(o.tracer), opt(o), path(opt.nogoods_limit), d(0), mark(0), 
      best(nullptr) {
    if (tracer) {
      tracer.engine(SearchTracer::EngineType::BAB, 1U);
      tracer.worker();
    }
    if ((s == nullptr) || (s->status(*this) == SS_FAILED)) {
      fail++;
      cur = nullptr;
      if (!o.clone)
        delete s;
    } else {
      cur = snapshot(s,opt);
    }
  }

  template<class Tracer>
  forceinline Space*
  BAB<Tracer>::next(void) {
    /*
     * The engine maintains the following invariant:
     *  - If the current space (cur) is not nullptr, the path always points
     *    to exactly that space.
     *  - If the current space (cur) is nullptr, the path always points
     *    to the next space (if there is any).
     *
     * This invariant is needed so that no-goods can be extracted properly
     * when the engine is stopped or has found a solution.
     *
     * An additional invariant maintained by the engine is:
     *   For all nodes stored at a depth less than mark, there
     *   is no guarantee of betterness. For those above the mark,
     *   betterness is guaranteed.
     *
     */
    start();
    while (true) {
      if (stop(opt))
        return nullptr;
      // Recompute and add constraint if necessary
      while (cur == nullptr) {
        if (path.empty())
          return nullptr;
        cur = path.recompute(d,opt.a_d,*this,*best,mark,tracer);
        if (cur != nullptr)
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
        cur = nullptr;
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
          delete best;
          best = cur;
          cur = nullptr;
          path.next();
          mark = path.entries();
        }
        return best->clone();
      case SS_BRANCH:
        {
          Space* c;
          if ((d == 0) || (d >= opt.c_d)) {
            c = cur->clone();
            d = 1;
          } else {
            c = nullptr;
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
    return nullptr;
  }

  template<class Tracer>
  forceinline Statistics
  BAB<Tracer>::statistics(void) const {
    return *this;
  }

  template<class Tracer>
  forceinline void
  BAB<Tracer>::constrain(const Space& b) {
    if (best != nullptr) {
      // Check whether b is in fact better than best
      best->constrain(b);
      if (best->status(*this) != SS_FAILED)
        return;
      else
        delete best;
    }
    best = b.clone();
    if (cur != nullptr)
      cur->constrain(b);
    mark = path.entries();
  }

  template<class Tracer>
  forceinline void
  BAB<Tracer>::reset(Space* s) {
    tracer.round();
    delete best;
    best = nullptr;
    path.reset();
    d = 0;
    mark = 0;
    delete cur;
    if ((s == nullptr) || (s->status(*this) == SS_FAILED)) {
      delete s;
      cur = nullptr;
    } else {
      cur = s;
    }
    Worker::reset();
  }

  template<class Tracer>
  forceinline NoGoods&
  BAB<Tracer>::nogoods(void) {
    return path;
  }

  template<class Tracer>
  forceinline
  BAB<Tracer>::~BAB(void) {
    tracer.done();
    path.reset();
    delete best;
    delete cur;
  }

}}}

// STATISTICS: search-seq
