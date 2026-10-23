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
#include <algorithm>

namespace Gecode { namespace Search {

  Engine*
  astarengine(Space* s, const HeuristicGuidance& g, const Options& o) {
    return bestfirstengine(s,g,AStarOrder::BOUND_FIRST,o);
  }

  Engine*
  bestfirstengine(Space* s, const HeuristicGuidance& g, AStarOrder ordering,
                  const Options& o) {
    Options expanded = o.expand();
    if (expanded.tracer != nullptr)
      throw UnsupportedAStarTracer("AStar");
    if (!g.bound)
      throw UninitializedAStarBound("AStar");
#ifdef GECODE_HAS_THREADS
    if (expanded.threads != 1.0)
      return new Par::Frontier(s,g,ordering,expanded,false);
#endif
    return new WorkerToEngine<Seq::AStar>(s,expanded,
                                        Seq::AStar::Configuration{g,ordering});
  }

}}

namespace Gecode { namespace Search { namespace Seq {

  bool
  AStar::Later::operator()(const Entry& a, const Entry& b) const {
    if (ordering == AStarOrder::RANK_FIRST && a.rank != b.rank)
      return a.rank > b.rank;
    if (a.bound != b.bound)
      return sense == HeuristicGuidance::Sense::MINIMIZE ?
        a.bound > b.bound : a.bound < b.bound;
    if (a.rank != b.rank)
      return a.rank > b.rank;
    return a.order > b.order;
  }

  AStar::AStar(Space* s, const Options& o, const Configuration& c)
    : opt(o), guidance(c.guidance), ordering(c.ordering),
      order(0), incumbent_revision(0),
      current_status(SS_FAILED), current_depth(0), current_revision(0),
      counted(false), best_bound(0) {
    initialize(s,o.clone);
  }

  void
  AStar::initialize(Space* s, bool clone) {
    // Evaluate guidance only after construction has transferred ownership.
    if (s == nullptr) {
      fail++;
    } else {
      current_status = s->status(*this);
      if (current_status == SS_FAILED) {
        fail++;
        if (!clone)
          delete s;
      } else {
        expansion.select(std::unique_ptr<Space>(clone ? s->clone() : s));
      }
    }
  }

  bool
  AStar::improves(std::int64_t bound) const {
    return !best || (guidance.sense == HeuristicGuidance::Sense::MINIMIZE ?
                    bound < best_bound : bound > best_bound);
  }

  void
  AStar::enqueue(std::unique_ptr<Space> s, unsigned long int depth,
                 unsigned long long int revision) {
    const std::int64_t bound = guidance.bound(*s);
    if (!improves(bound)) {
      fail++;
      return;
    }
    const std::int64_t rank = guidance.rank ? guidance.rank(*s) : 0;
    frontier.push_back({std::move(s),bound,rank,depth,order++,revision});
    std::push_heap(frontier.begin(),frontier.end(),Later{guidance.sense,ordering});
  }

  Space*
  AStar::publish(std::unique_ptr<Space> s) {
    // Delete exhausted branchers before cloning the solution.
    (void) s->choice();
    const std::int64_t bound = guidance.bound(*s);
    if (!improves(bound))
      return nullptr;
    std::unique_ptr<Space> result(s->clone());
    best = std::move(s);
    best_bound = bound;
    incumbent_revision++;
    return result.release();
  }

  Space*
  AStar::next(void) {
    start();
    while (true) {
      if (stop_seq(opt))
        return nullptr;
      if (!expansion) {
        if (frontier.empty())
          return nullptr;
        std::pop_heap(frontier.begin(),frontier.end(),Later{guidance.sense,ordering});
        Entry entry = std::move(frontier.back());
        frontier.pop_back();
        if (!improves(entry.bound)) {
          fail++;
          continue;
        }
        expansion.select(std::move(entry.space));
        current_depth = entry.depth;
        current_revision = entry.incumbent_revision;
        current_status = SS_BRANCH;
        counted = true;
      }
      if (!counted) {
        node++;
        stack_depth(current_depth);
        counted = true;
        if (current_status == SS_SOLVED) {
          if (Space* solution = publish(expansion.take()))
            return solution;
          continue;
        }
        if (!improves(guidance.bound(expansion.space()))) {
          fail++;
          expansion.reset();
          continue;
        }
      }
      const auto child_depth = current_depth+1;
      ExpansionStep step = expansion.next(*this,child_depth,current_revision,
                                          best.get(),incumbent_revision);
      if (step.state == ExpansionStep::State::EXHAUSTED)
        continue;
      switch (step.status) {
      case SS_FAILED:
        fail++;
        break;
      case SS_SOLVED:
        if (Space* solution = publish(std::move(step.space)))
          return solution;
        break;
      case SS_BRANCH:
        enqueue(std::move(step.space),child_depth,incumbent_revision);
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
  AStar::constrain(const Space& b) {
    const std::int64_t bound = guidance.bound(b);
    if (!improves(bound))
      return;
    std::unique_ptr<Space> incumbent(b.clone());
    best = std::move(incumbent);
    best_bound = bound;
    incumbent_revision++;
  }

  void
  AStar::reset(Space* s) {
    std::unique_ptr<Space> replacement(s);
    expansion.reset();
    frontier.clear();
    best.reset();
    order = 0;
    incumbent_revision = 0;
    current_revision = 0;
    current_depth = 0;
    counted = false;
    Worker::reset();
    initialize(s,false);
    // initialize consumes the root on normal return, including failure.
    replacement.release();
  }

  NoGoods&
  AStar::nogoods(void) {
    return NoGoods::eng;
  }

}}}

// STATISTICS: search-other
