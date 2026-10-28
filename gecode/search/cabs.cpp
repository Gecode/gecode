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
#include <gecode/search/seq/cabs.hh>
#include <gecode/search/par/frontier.hh>

#ifdef GECODE_HAS_EXPERIMENTAL

#ifdef GECODE_HAS_THREADS
namespace Gecode { namespace Search { namespace Par {

  /// Parallel complete anytime beam search
  class CABS : public Frontier {
  protected:
    /// Order of nodes in a layer
    Priority priority;
    /// Root for restarting with a wider beam
    std::unique_ptr<Space> root;
    /// Nodes of the current layer, preferred node on top
    Entries layer;
    /// Candidates for the next layer
    Beam candidates;
    bool open(void) const override {
      return !layer.empty();
    }
    Entry select(void) override {
      return layer.pop();
    }
    void submit(const Entry& e, bool leased) override {
      if (!best.improves(e.bound)) {
        stat.fail++;
        delete e.space;
        return;
      }
      Entry f = e;
      f.order = order++;
      // Every other worker holds a parent and at most one child
      unsigned long int stored = layer.entries() + candidates.size() +
        n_pending + 2*(active-1) + (leased ? 1 : 0);
      bool room = (opt.frontier_limit == 0UL) || (stored < opt.frontier_limit);
      if (candidates.offer(f,guidance,room) == Beam::FULL) {
        delete f.space;
        limited = _stopped = true;
        running = false;
      }
    }
    bool advance(void) override {
      if (!candidates.empty()) {
        candidates.release(layer);
        layer.sort(priority);
        return true;
      }
      if (!candidates.improving(best))
        return false;
      if (!candidates.widen(opt.max_width)) {
        limited = _stopped = true;
        return false;
      }
      stat.restart++;
      std::unique_ptr<Task> t(new Task);
      t->expansion.select(std::unique_ptr<Space>(root->clone()));
      push(t.release());
      return true;
    }
    void clear(void) override {
      layer.clear();
      candidates.clear();
      root.reset();
    }
    void started(const Space& r) override {
      root.reset(r.clone());
    }
  public:
    /// Initialize for space \a s with options \a o
    CABS(Space* s, const Options& o)
      : Frontier(o), priority{guidance.sense,static_cast<bool>(guidance.rank)},
        candidates(priority,guidance.dominates && guidance.dominance_key) {
      batch = (o.frontier_limit == 0UL) ? max_batch : 1;
      keyed = guidance.dominates && guidance.dominance_key;
      initialize(s,o.clone);
    }
    /// Destructor
    ~CABS(void) override {
      terminate();
    }
  };

}}}
#endif

namespace Gecode { namespace Search {

  Engine*
  cabsengine(Space* s, const Options& o) {
    Options e = o.expand();
    if (e.tracer != nullptr)
      throw UnsupportedTracer("CABS::CABS");
    if ((e.guidance == nullptr) || !e.guidance->bound)
      throw UninitializedGuidance("CABS::CABS");
#ifdef GECODE_HAS_THREADS
    if (e.threads > 1.0)
      return new Par::CABS(s,e);
#endif
    return new WorkerToEngine<Seq::CABS>(s,e);
  }

}}

namespace Gecode { namespace Search { namespace Seq {

  CABS::CABS(Space* s, const Options& o)
    : opt(o), guidance(*o.guidance),
      priority{guidance.sense,static_cast<bool>(guidance.rank)},
      candidates(priority,guidance.dominates && guidance.dominance_key),
      order(0), status(SS_FAILED), depth(0), revision(0), counted(false),
      best(guidance.sense), limited(false) {
    initialize(s,o.clone);
  }

  void
  CABS::initialize(Space* s, bool clone) {
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
    // The restart root and the first node are clones of the stable root
    std::unique_ptr<Space> r(clone ? s->clone() : nullptr);
    expansion.select(std::unique_ptr<Space>((clone ? r.get() : s)->clone()));
    root.reset(clone ? r.release() : s);
  }

  bool
  CABS::admit(std::unique_ptr<Space> s, unsigned long int d) {
    std::int64_t b = guidance.bound(*s);
    if (!best.improves(b)) {
      fail++;
      return true;
    }
    std::int64_t r = guidance.rank ? guidance.rank(*s) : 0;
    std::size_t k = (guidance.dominates && guidance.dominance_key) ?
      guidance.dominance_key(*s) : 0;
    unsigned long int stored = layer.entries() + candidates.size() +
      (expansion ? 1 : 0);
    bool room = (opt.frontier_limit == 0UL) || (stored < opt.frontier_limit);
    Entry e{s.get(),b,r,k,d,order++,best.revision};
    if (candidates.offer(e,guidance,room) == Beam::FULL) {
      limited = _stopped = true;
      return false;
    }
    (void) s.release();
    return true;
  }

  Space*
  CABS::publish(std::unique_ptr<Space> s) {
    // Delete exhausted branchers before cloning the solution
    (void) s->choice();
    std::int64_t b = guidance.bound(*s);
    if (!best.improves(b))
      return nullptr;
    Space* copy = s->clone();
    best.update(s.release(),b);
    return copy;
  }

  bool
  CABS::advance(void) {
    if (!candidates.empty()) {
      candidates.release(layer);
      layer.sort(priority);
      return true;
    }
    if (!candidates.improving(best))
      return false;
    if (!candidates.widen(opt.max_width)) {
      limited = _stopped = true;
      return false;
    }
    restart++;
    expansion.select(std::unique_ptr<Space>(root->clone()));
    status = SS_BRANCH;
    depth = 0;
    revision = 0;
    counted = false;
    return true;
  }

  Space*
  CABS::next(void) {
    if (limited) {
      _stopped = true;
      return nullptr;
    }
    start();
    while (true) {
      if (stop_seq(opt))
        return nullptr;
      if (!expansion) {
        if (layer.empty() && !advance())
          return nullptr;
        if (!expansion) {
          Entry e = layer.pop();
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
      }
      if (!counted) {
        // The root of a pass
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
        if (!admit(std::move(step.space),depth+1))
          return nullptr;
        break;
      default:
        GECODE_NEVER;
      }
    }
  }

  Statistics
  CABS::statistics(void) const {
    return *this;
  }

  void
  CABS::constrain(const Space& s) {
    std::int64_t b = guidance.bound(s);
    if (best.improves(b))
      best.update(s.clone(),b);
  }

  void
  CABS::reset(Space* s) {
    std::unique_ptr<Space> replacement(s);
    expansion.reset();
    layer.clear();
    candidates.clear();
    root.reset();
    best.reset();
    order = 0;
    revision = 0;
    depth = 0;
    counted = limited = false;
    Worker::reset();
    initialize(s,false);
    // The root is owned by the engine after initialize returns
    (void) replacement.release();
  }

  NoGoods&
  CABS::nogoods(void) {
    return NoGoods::eng;
  }

}}}

#endif

// STATISTICS: search-other
