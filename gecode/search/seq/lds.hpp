/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004, 2016
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

  /*
   * Nodes for the probing engine (just remember next alternative
   * to try)
   *
   */

  template<class Tracer>
  forceinline
  Probe<Tracer>::Node::Node(void) {}

  template<class Tracer>
  forceinline
  Probe<Tracer>::Node::Node(Space* s, const Choice* c, unsigned int a,
                            unsigned int nid)
    : _space(s), _choice(c), _alt(a), _nid(nid) {}

  template<class Tracer>
  forceinline Space*
  Probe<Tracer>::Node::space(void) const {
    return _space;
  }

  template<class Tracer>
  forceinline const Choice*
  Probe<Tracer>::Node::choice(void) const {
    return _choice;
  }

  template<class Tracer>
  forceinline unsigned int
  Probe<Tracer>::Node::alt(void) const {
    return _alt;
  }

  template<class Tracer>
  forceinline unsigned int
  Probe<Tracer>::Node::nid(void) const {
    return _nid;
  }

  template<class Tracer>
  forceinline void
  Probe<Tracer>::Node::next(void) {
    _alt--;
  }


  /*
   * The probing engine: computes all solutions with
   * exact number of discrepancies (solutions with
   * fewer discrepancies are discarded)
   *
   */

  template<class Tracer>
  forceinline
  Probe<Tracer>::Probe(const Options& opt)
    : tracer(opt.tracer), ds(heap) {
    tracer.engine(SearchTracer::EngineType::LDS, 1U);
    tracer.worker();
  }

  template<class Tracer>
  forceinline void
  Probe<Tracer>::init(Space* s) {
    cur = s; d = 0U; exhausted = true;
    if (tracer)
      tracer.ei()->invalidate();
  }

  template<class Tracer>
  forceinline void
  Probe<Tracer>::reset(Space* s, unsigned int d0) {
    tracer.round();
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
    cur = s; d = d0; exhausted = true;
    if (tracer)
      tracer.ei()->invalidate();
    Worker::reset(0);
  }

  template<class Tracer>
  forceinline Statistics
  Probe<Tracer>::statistics(void) const {
    return *this;
  }

  template<class Tracer>
  forceinline bool
  Probe<Tracer>::done(void) const {
    return exhausted;
  }

  template<class Tracer>
  forceinline
  Probe<Tracer>::~Probe(void) {
    tracer.done();
    delete cur;
    while (!ds.empty())
      delete ds.pop().space();
  }

  template<class Tracer>
  forceinline Space*
  Probe<Tracer>::next(const Options& opt) {
    start();
    while (true) {
      if (cur == NULL) {
      backtrack:
        if (ds.empty())
          return NULL;
        if (stop(opt))
          return NULL;
        unsigned int a = ds.top().alt();
        const Choice* ch = ds.top().choice();
        unsigned int nid = ds.top().nid();
        if (a == 0) {
          cur = ds.pop().space();
          if (tracer)
            tracer.ei()->init(tracer.wid(), nid, 0, *cur, *ch);
          cur->commit(*ch,0);
          delete ch;
        } else {
          ds.top().next();
          cur = ds.top().space()->clone();
          if (tracer)
            tracer.ei()->init(tracer.wid(), nid, a, *cur, *ch);
          cur->commit(*ch,a);
        }
        node++;
        d++;
      }
    check_discrepancy:
      if (d == 0) {
        Space* s = cur;
        while (s->status(*this) == SS_BRANCH) {
          if (stop(opt)) {
            cur = s;
            return NULL;
          }
          const Choice* ch = s->choice();
          if (ch->alternatives() > 1)
            exhausted = false;
          if (tracer) {
            unsigned int nid = tracer.nid();
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::BRANCH,
                                      tracer.wid(), nid, *s, ch);
            tracer.node(*tracer.ei(),ni);
            if (tracer)
              tracer.ei()->init(tracer.wid(), nid, 0, *cur, *ch);
          }
          s->commit(*ch,0);
          node++;
          delete ch;
        }
        cur = NULL;
        if (s->failed()) {
          if (tracer) {
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::FAILED,
                                      tracer.wid(), tracer.nid(), *s);
            tracer.node(*tracer.ei(),ni);
          }
          fail++;
          delete s;
          goto backtrack;
        } else {
          if (tracer) {
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::SOLVED,
                                      tracer.wid(), tracer.nid(), *s);
            tracer.node(*tracer.ei(),ni);
          }
          // Deletes all pending branchings
          (void) s->choice();
          return s;
        }
      } else {
        node++;
        switch (cur->status(*this)) {
        case SS_FAILED:
          if (tracer) {
            SearchTracer::NodeInfo ni(SearchTracer::NodeType::FAILED,
                                      tracer.wid(), tracer.nid(), *cur);
            tracer.node(*tracer.ei(),ni);
          }
          fail++;
          delete cur;
          cur = NULL;
          goto backtrack;
        case SS_SOLVED:
          if (tracer) {
            tracer.skip(*tracer.ei());
          }
          delete cur;
          cur = NULL;
          goto backtrack;
        case SS_BRANCH:
          {
            const Choice* ch = cur->choice();
            unsigned int alt = ch->alternatives();
            unsigned int nid = tracer.nid();
            if (tracer) {
              SearchTracer::NodeInfo ni(SearchTracer::NodeType::BRANCH,
                                        tracer.wid(), nid, *cur, ch);
              tracer.node(*tracer.ei(),ni);
            }
            if (alt > 1) {
              if (d < alt-1)
                exhausted = false;
              unsigned int d_a = (d >= alt-1) ? alt-1 : d;
              Space* cc = cur->clone();
              Node sn(cc,ch,d_a-1,nid);
              ds.push(sn);
              stack_depth(static_cast<unsigned long int>(ds.entries()));
              if (tracer)
                tracer.ei()->init(tracer.wid(), nid, d_a, *cur, *ch);
              cur->commit(*ch,d_a);
              d -= d_a;
            } else {
              if (tracer)
                tracer.ei()->init(tracer.wid(), nid, 0, *cur, *ch);
              cur->commit(*ch,0);
              node++;
              delete ch;
            }
            goto check_discrepancy;
          }
        default: GECODE_NEVER;
        }
      }
    }
  }

  template<class Tracer>
  forceinline
  LDS<Tracer>::LDS(Space* s, const Options& o)
    : opt(o), e(opt), root(NULL), d(0) {
    e.node = 1;
    if (s->status(e) == SS_FAILED) {
      e.fail++;
      e.init(NULL);
    } else {
      Space* c = snapshot(s,opt);
      if (opt.d_l > 0) {
        root = c->clone();
      }
      e.init(c);
    }
  }

  template<class Tracer>
  Space*
  LDS<Tracer>::next(void) {
    while (true) {
      Space* s = e.next(opt);
      if (s != NULL)
        return s;
      if (((s == NULL) && e.stopped()) || (++d > opt.d_l) || e.done())
        break;
      if (d == opt.d_l) {
        if (root != NULL)
          e.reset(root,d);
        root = NULL;
      } else if (root != NULL) {
        e.reset(root->clone(),d);
      }
    }
    return NULL;
  }

  template<class Tracer>
  bool
  LDS<Tracer>::stopped(void) const {
    return e.stopped();
  }

  template<class Tracer>
  Statistics
  LDS<Tracer>::statistics(void) const {
    return e.statistics();
  }


  template<class Tracer>
  forceinline void
  LDS<Tracer>::reset(Space* s) {
    delete root; root=NULL; d=0;
    e.node = 1;
    if ((s == NULL) || (s->status(e) == SS_FAILED)) {
      delete s;
      e.fail++;
      e.reset(NULL,0);
    } else {
      if (opt.d_l > 0) {
        root = s->clone();
      }
      e.reset(s,0);
    }
  }

  template<class Tracer>
  forceinline void
  LDS<Tracer>::constrain(const Space& b) {
    (void) b;
    assert(false);
  }

  template<class Tracer>
  LDS<Tracer>::~LDS(void) {
    delete root;
  }

}}}

// STATISTICS: search-seq
