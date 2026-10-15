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
#include <gecode/search/seq/column.hh>
#include <algorithm>
#include <limits>

namespace Gecode { namespace Search {

  Engine*
  columnengine(Space* s, const HeuristicGuidance& g, const Options& o,
              const ColumnOptions& limits, const StatePruning& p) {
    Options expanded = o.expand();
    if (expanded.threads != 1.0)
      throw UnsupportedColumnThreads("Column");
    if (expanded.tracer != nullptr)
      throw UnsupportedColumnTracer("Column");
    if (!g.bound)
      throw UninitializedColumnBound("Column");
    if (!limits.initial_quota)
      throw InvalidColumnQuota("Column");
    if (static_cast<bool>(p.extract) != static_cast<bool>(p.compare))
      throw UninitializedStatePruning("Column");
    return new WorkerToEngine<Seq::Column>(s,expanded,
                                         Seq::Column::Configuration{g,limits,p});
  }

}}

namespace Gecode { namespace Search { namespace Seq {

  bool
  Column::Later::operator()(const Entry& a, const Entry& b) const {
    if (ranked) {
      if (a.rank != b.rank) return a.rank > b.rank;
    } else if (a.bound != b.bound) {
      return sense == HeuristicGuidance::MINIMIZE ?
        a.bound > b.bound : a.bound < b.bound;
    }
    return a.order > b.order;
  }

  Column::Column(Space* s, const Options& o, const Configuration& c)
    : opt(o), guidance(c.guidance), limits(c.limits), pruning(c.pruning),
      registry(pruning.extract ?
               new StateRegistry(pruning.compare,pruning.descriptor_limit) :
               nullptr),
      queued(0), cycle_depth(0),
      quota(limits.initial_quota), expanded(0), limited(false), order(0),
      incumbent_revision(0),
      current_status(SS_FAILED), current_depth(0), current_revision(0),
      counted(false), best_bound(0) {
    initialize(s,o.clone);
  }

  void
  Column::initialize(Space* s, bool clone) {
    // Evaluate guidance only after construction has transferred ownership.
    if (s == nullptr) {
      fail++;
    } else {
      current_status = s->status(*this);
      if (current_status == SS_FAILED) {
        fail++;
        if (!clone) delete s;
      } else {
        expansion.select(std::unique_ptr<Space>(clone ? s->clone() : s));
      }
    }
  }

  bool
  Column::improves(std::int64_t bound) const {
    return !best || (guidance.sense == HeuristicGuidance::MINIMIZE ?
                    bound < best_bound : bound > best_bound);
  }

  bool
  Column::admit(std::unique_ptr<Space> s, unsigned long int depth,
                unsigned long long int revision) {
    const std::int64_t bound = guidance.bound(*s);
    if (!improves(bound)) {
      fail++;
      return true;
    }
    std::optional<StatePruning::Descriptor> descriptor;
    if (registry) {
      descriptor = pruning.extract(*s);
      if (descriptor && registry->subsumes(*descriptor)) {
        state_pruned++;
        return true;
      }
    }
    if (limits.frontier_limit &&
        queued + (expansion ? 1U : 0U) >= limits.frontier_limit) {
      limited = _stopped = true;
      return false;
    }
    const std::int64_t rank = guidance.rank ? guidance.rank(*s) : 0;
    if (columns.size() <= depth) columns.resize(depth+1);
    auto& column = columns[depth];
    column.push_back({std::move(s),bound,rank,depth,order++,revision});
    std::push_heap(column.begin(),column.end(),
                   Later{guidance.sense,static_cast<bool>(guidance.rank)});
    queued++;
    if (!descriptor) return true;
    // Admit fresh work before replacing labels, even after earlier expansion.
    // Pending Spaces keep their original Choice cursors and coverage.
    registry->store(std::move(*descriptor));
    state_descriptors = std::max(state_descriptors,registry->size());
    state_bytes = std::max(state_bytes,registry->storage());
    return true;
  }

  Space*
  Column::publish(std::unique_ptr<Space> s) {
    // Delete exhausted branchers; completion branchers must survive restrictions.
    (void) s->choice();
    const std::int64_t bound = guidance.bound(*s);
    if (!improves(bound)) return nullptr;
    std::unique_ptr<Space> result(s->clone());
    best = std::move(s);
    best_bound = bound;
    incumbent_revision++;
    return result.release();
  }

  bool
  Column::select(void) {
    while (queued) {
      if (stop_seq(opt)) return false;
      if (cycle_depth >= columns.size()) {
        cycle_depth = 0;
        expanded = 0;
        const auto maximum = std::numeric_limits<unsigned long int>::max();
        quota += std::min(limits.quota_growth,maximum-quota);
      }
      auto& column = columns[cycle_depth];
      if (column.empty() || expanded == quota) {
        cycle_depth++;
        expanded = 0;
        continue;
      }
      std::pop_heap(column.begin(),column.end(),
                    Later{guidance.sense,static_cast<bool>(guidance.rank)});
      Entry entry = std::move(column.back());
      column.pop_back();
      queued--;
      if (!improves(entry.bound)) {
        fail++;
        continue;
      }
      expansion.select(std::move(entry.space));
      current_depth = entry.depth;
      current_revision = entry.incumbent_revision;
      current_status = SS_BRANCH;
      counted = true;
      expanded++;
      return true;
    }
    return false;
  }

  Space*
  Column::next(void) {
    if (limited) {
      _stopped = true;
      return nullptr;
    }
    start();
    while (true) {
      if (stop_seq(opt)) return nullptr;
      if (!expansion && !select()) return nullptr;
      if (!counted) {
        node++;
        stack_depth(current_depth);
        counted = true;
        if (current_status == SS_SOLVED) {
          if (Space* solution = publish(expansion.take())) return solution;
          continue;
        }
        if (!improves(guidance.bound(expansion.space()))) {
          fail++;
          expansion.reset();
          continue;
        }
        expanded++;
      }
      const auto child_depth = current_depth+1;
      ExpansionStep step = expansion.next(*this,opt,child_depth,current_revision,
                                           best.get(),incumbent_revision);
      if (step.state == ExpansionStep::SUSPENDED) return nullptr;
      if (step.state == ExpansionStep::EXHAUSTED) continue;
      switch (step.status) {
      case SS_FAILED:
        fail++;
        break;
      case SS_SOLVED:
        if (Space* solution = publish(std::move(step.space))) return solution;
        break;
      case SS_BRANCH:
        if (!admit(std::move(step.space),child_depth,incumbent_revision))
          return nullptr;
        break;
      default:
        GECODE_NEVER;
      }
    }
  }

  Statistics
  Column::statistics(void) const {
    return *this;
  }

  void
  Column::constrain(const Space& b) {
    const std::int64_t bound = guidance.bound(b);
    if (!improves(bound)) return;
    std::unique_ptr<Space> incumbent(b.clone());
    best = std::move(incumbent);
    best_bound = bound;
    incumbent_revision++;
  }

  void
  Column::reset(Space* s) {
    expansion.reset();
    columns.clear();
    queued = 0;
    cycle_depth = expanded = 0;
    quota = limits.initial_quota;
    limited = false;
    if (registry) registry->clear();
    best.reset();
    order = 0;
    incumbent_revision = current_revision = 0;
    current_depth = 0;
    counted = false;
    Worker::reset();
    initialize(s,false);
  }

  NoGoods&
  Column::nogoods(void) {
    return NoGoods::eng;
  }

}}}

// STATISTICS: search-other
