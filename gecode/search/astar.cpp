/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
#include <gecode/search/support.hh>
#include <gecode/search/seq/astar.hh>
#include <gecode/search/par/frontier.hh>

#ifdef GECODE_HAS_EXPERIMENTAL

#ifdef GECODE_HAS_THREADS
namespace Gecode { namespace Search { namespace Par {

  /// Parallel A* search
  class AStar : public Frontier {
  protected:
    /// Open nodes
    Queue queue;
    bool open(void) const override {
      return !queue.empty();
    }
    Entry select(void) override {
      return queue.pop();
    }
    void submit(const Entry& e, bool) override {
      if (!best.improves(e.bound)) {
        stat.fail++;
        delete e.space;
        return;
      }
      Entry f = e;
      f.order = order++;
      queue.push(f);
    }
    bool advance(void) override {
      return false;
    }
    void clear(void) override {
      queue.clear();
    }
  public:
    /// Initialize for space \a s with options \a o
    AStar(Space* s, const Options& o)
      : Frontier(o), queue(Priority{guidance.sense,false}) {
      initialize(s,o.clone);
    }
    /// Destructor
    ~AStar(void) override {
      terminate();
    }
  };

}}}
#endif

namespace Gecode { namespace Search {

  Engine*
  astarengine(Space* s, const Options& o) {
    Options e = o.expand();
    if (e.tracer != nullptr)
      throw UnsupportedTracer("AStar::AStar");
    if ((e.guidance == nullptr) || !e.guidance->bound)
      throw UninitializedGuidance("AStar::AStar");
#ifdef GECODE_HAS_THREADS
    if (e.threads > 1.0)
      return new Par::AStar(s,e);
#endif
    return new WorkerToEngine<Seq::AStar>(s,e);
  }

}}

namespace Gecode { namespace Search { namespace Seq {

  AStar::AStar(Space* s, const Options& o)
    : opt(o), guidance(*o.guidance), queue(Priority{guidance.sense,false}),
      order(0), status(SS_FAILED), depth(0), revision(0), counted(false),
      best(guidance.sense) {
    initialize(s,o.clone);
  }

  void
  AStar::initialize(Space* s, bool clone) {
    if (s == nullptr) {
      fail++;
      return;
    }
    status = s->status(*this);
    if (status == SS_FAILED) {
      fail++;
      if (!clone)
        delete s;
      return;
    }
    expansion.select(std::unique_ptr<Space>(clone ? s->clone() : s));
  }

  void
  AStar::enqueue(std::unique_ptr<Space> s, unsigned long int d) {
    std::int64_t b = guidance.bound(*s);
    if (!best.improves(b)) {
      fail++;
      return;
    }
    std::int64_t r = guidance.rank ? guidance.rank(*s) : 0;
    queue.push({s.release(),b,r,0,d,order++,best.revision});
  }

  Space*
  AStar::publish(std::unique_ptr<Space> s) {
    // Delete exhausted branchers before cloning the solution
    (void) s->choice();
    std::int64_t b = guidance.bound(*s);
    if (!best.improves(b))
      return nullptr;
    Space* copy = s->clone();
    best.update(s.release(),b);
    return copy;
  }

  Space*
  AStar::next(void) {
    start();
    while (true) {
      if (stop_seq(opt))
        return nullptr;
      if (!expansion) {
        if (queue.empty())
          return nullptr;
        Entry e = queue.pop();
        std::unique_ptr<Space> s(e.space);
        if (!best.improves(e.bound)) {
          fail++;
          continue;
        }
        expansion.select(std::move(s));
        depth = e.depth;
        revision = e.revision;
        counted = true;
      }
      if (!counted) {
        // The root
        node++;
        stack_depth(depth);
        counted = true;
        if (status == SS_SOLVED) {
          if (Space* s = publish(expansion.take()))
            return s;
          continue;
        }
        if (!best.improves(guidance.bound(expansion.space()))) {
          fail++;
          expansion.reset();
          continue;
        }
      }
      ExpansionStep step = expansion.next(*this,depth+1,revision,
                                          best.space.get(),best.revision);
      if (step.state == ExpansionStep::EXHAUSTED)
        continue;
      switch (step.status) {
      case SS_FAILED:
        fail++;
        break;
      case SS_SOLVED:
        if (Space* s = publish(std::move(step.space)))
          return s;
        break;
      case SS_BRANCH:
        enqueue(std::move(step.space),depth+1);
        break;
      default:
        GECODE_NEVER;
      }
    }
  }

  Statistics
  AStar::statistics(void) const {
    return *this;
  }

  void
  AStar::constrain(const Space& s) {
    std::int64_t b = guidance.bound(s);
    if (best.improves(b))
      best.update(s.clone(),b);
  }

  void
  AStar::reset(Space* s) {
    std::unique_ptr<Space> replacement(s);
    expansion.reset();
    queue.clear();
    best.reset();
    order = 0;
    revision = 0;
    depth = 0;
    counted = false;
    Worker::reset();
    initialize(s,false);
    // The root is owned by the engine after initialize returns
    (void) replacement.release();
  }

  NoGoods&
  AStar::nogoods(void) {
    return NoGoods::eng;
  }

}}}

#endif

// STATISTICS: search-other
