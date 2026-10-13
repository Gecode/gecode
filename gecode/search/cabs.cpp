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
#include <algorithm>
#include <limits>

namespace Gecode { namespace Search {

  Engine*
  cabsengine(Space* s, const HeuristicGuidance& g, const Options& o,
             const CABSOptions& limits, const StatePruning& p) {
    Options expanded = o.expand();
    if (expanded.threads != 1.0)
      throw UnsupportedCABSThreads("CABS");
    if (expanded.tracer != nullptr)
      throw UnsupportedCABSTracer("CABS");
    if (!g.bound)
      throw UninitializedCABSBound("CABS");
    if (static_cast<bool>(p.extract) != static_cast<bool>(p.compare))
      throw UninitializedStatePruning("CABS");
    return new WorkerToEngine<Seq::CABS>(s,expanded,
                                       Seq::CABS::Configuration{g,limits,p});
  }

}}

namespace Gecode { namespace Search { namespace Seq {

  bool
  CABS::Earlier::operator()(const Entry& a, const Entry& b) const {
    if (ranked) {
      if (a.rank != b.rank) return a.rank < b.rank;
    } else if (a.bound != b.bound) {
      return sense == HeuristicGuidance::MINIMIZE ?
        a.bound < b.bound : a.bound > b.bound;
    }
    return a.order < b.order;
  }

  CABS::CABS(Space* s, const Options& o, const Configuration& c)
    : opt(o), guidance(c.guidance), limits(c.limits), pruning(c.pruning),
      candidate_descriptors(0), candidate_bytes(0), width(1), order(0),
      incumbent_revision(0), current_status(SS_FAILED), current_depth(0),
      current_revision(0), counted(false), best_bound(0),
      limited(false) {
    initialize(s,o.clone);
  }

  void
  CABS::initialize(Space* s, bool clone) {
    if (s == nullptr) {
      fail++;
      return;
    }
    current_status = s->status(*this);
    if (current_status == SS_FAILED) {
      fail++;
      if (!clone) delete s;
      return;
    }
    // Finish all construction-time clones before taking the caller's root.
    std::unique_ptr<Space> restart_root(clone ? s->clone() : nullptr);
    expansion.select(std::unique_ptr<Space>(
      (clone ? restart_root.get() : s)->clone()));
    root.reset(clone ? restart_root.release() : s);
  }

  bool
  CABS::improves(std::int64_t bound) const {
    return !best || (guidance.sense == HeuristicGuidance::MINIMIZE ?
                    bound < best_bound : bound > best_bound);
  }

  bool
  CABS::admit(std::unique_ptr<Space> s, unsigned long int depth,
              unsigned long long int revision) {
    const std::int64_t bound = guidance.bound(*s);
    if (!improves(bound)) {
      fail++;
      return true;
    }
    std::optional<StatePruning::Descriptor> descriptor;
    std::vector<std::size_t> victims;
    if (pruning.extract) {
      descriptor = pruning.extract(*s);
      if (descriptor)
        for (std::size_t i=0; i<candidates.size(); i++) {
          const auto& existing = candidates[i].descriptor;
          if (!existing || descriptor->hash != existing->hash ||
              descriptor->key != existing->key) continue;
          const auto relation = pruning.compare(*descriptor,*existing);
          if (relation == StatePruning::EQUIVALENT ||
              relation == StatePruning::DOMINATED) {
            state_pruned++;
            return true;
          }
          if (relation == StatePruning::DOMINATES) victims.push_back(i);
        }
    }
    const std::int64_t rank = guidance.rank ? guidance.rank(*s) : 0;
    Entry entry{std::move(s),bound,rank,depth,order++,revision,
                std::move(descriptor)};
    Earlier earlier{guidance.sense,static_cast<bool>(guidance.rank)};
    if (!victims.empty()) {
      // A dominator takes a victim's existing slot regardless of rank. Only
      // after it survives may the other dominated live candidates disappear.
      for (std::size_t victim : victims)
        remove_descriptor(candidates[victim]);
      candidates[victims.front()] = std::move(entry);
      add_descriptor(candidates[victims.front()]);
      std::size_t write = victims.front()+1;
      std::size_t victim = 1;
      for (std::size_t read=write; read<candidates.size(); read++) {
        if (victim<victims.size() && read==victims[victim]) {
          victim++;
        } else {
          if (write!=read) candidates[write] = std::move(candidates[read]);
          write++;
        }
      }
      candidates.erase(candidates.begin()+write,candidates.end());
      std::make_heap(candidates.begin(),candidates.end(),earlier);
      state_pruned += victims.size();
      record_descriptors();
      return true;
    }
    if (candidates.size() == width) {
      const bool replace = earlier(entry,candidates.front());
      const std::int64_t discarded = replace ? candidates.front().bound : entry.bound;
      if (!discarded_bound || (guidance.sense == HeuristicGuidance::MINIMIZE ?
                              discarded < *discarded_bound :
                              discarded > *discarded_bound))
        discarded_bound = discarded;
      if (!replace) return true;
      // Replacement uses the discarded entry's existing slot.
      std::pop_heap(candidates.begin(),candidates.end(),earlier);
      remove_descriptor(candidates.back());
      candidates.back() = std::move(entry);
      add_descriptor(candidates.back());
      std::push_heap(candidates.begin(),candidates.end(),earlier);
      record_descriptors();
      return true;
    }
    const auto pending = layer.size() + candidates.size() + (expansion ? 1U : 0U);
    if (limits.frontier_limit && pending >= limits.frontier_limit) {
      limited = _stopped = true;
      return false;
    }
    candidates.push_back(std::move(entry));
    add_descriptor(candidates.back());
    std::push_heap(candidates.begin(),candidates.end(),earlier);
    record_descriptors();
    return true;
  }

  void
  CABS::add_descriptor(const Entry& entry) {
    if (entry.descriptor) {
      candidate_descriptors++;
      candidate_bytes += entry.descriptor->storage();
    }
  }

  void
  CABS::remove_descriptor(const Entry& entry) {
    if (entry.descriptor) {
      candidate_descriptors--;
      candidate_bytes -= entry.descriptor->storage();
    }
  }

  void
  CABS::record_descriptors(void) {
    if (pruning.descriptor_limit &&
        candidate_descriptors > pruning.descriptor_limit) {
      // Forget metadata only; every admitted candidate still gets expanded.
      for (auto& candidate : candidates) candidate.descriptor.reset();
      candidate_descriptors = 0;
      candidate_bytes = 0;
      return;
    }
    state_descriptors = std::max(state_descriptors,candidate_descriptors);
    state_bytes = std::max(state_bytes,candidate_bytes);
  }

  Space*
  CABS::publish(std::unique_ptr<Space> s) {
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
  CABS::advance(void) {
    if (!candidates.empty()) {
      layer.swap(candidates);
      // Only live next-layer candidates can justify state pruning.
      if (pruning.extract)
        for (auto& entry : layer) entry.descriptor.reset();
      candidate_descriptors = 0;
      candidate_bytes = 0;
      Earlier earlier{guidance.sense,static_cast<bool>(guidance.rank)};
      // Keep the preferred node at the back for constant-time selection.
      std::sort(layer.begin(),layer.end(),
                [&](const Entry& a, const Entry& b) { return earlier(b,a); });
      return true;
    }
    if (!discarded_bound || !improves(*discarded_bound)) return false;
    if (width > std::numeric_limits<unsigned long int>::max()/2 ||
        (limits.max_width && width*2 > limits.max_width)) {
      limited = _stopped = true;
      return false;
    }
    width *= 2;
    restart++;
    discarded_bound.reset();
    expansion.select(std::unique_ptr<Space>(root->clone()));
    current_status = SS_BRANCH;
    current_depth = 0;
    current_revision = 0;
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
      if (stop_seq(opt)) return nullptr;
      if (!expansion) {
        if (layer.empty() && !advance()) return nullptr;
        if (!expansion) {
          Entry entry = std::move(layer.back());
          layer.pop_back();
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
      }
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
  CABS::statistics(void) const {
    return *this;
  }

  void
  CABS::constrain(const Space& b) {
    const std::int64_t bound = guidance.bound(b);
    if (!improves(bound)) return;
    std::unique_ptr<Space> incumbent(b.clone());
    best = std::move(incumbent);
    best_bound = bound;
    incumbent_revision++;
  }

  void
  CABS::reset(Space* s) {
    expansion.reset();
    layer.clear();
    candidates.clear();
    candidate_descriptors = 0;
    candidate_bytes = 0;
    root.reset();
    best.reset();
    width = 1;
    order = 0;
    incumbent_revision = current_revision = 0;
    current_depth = 0;
    discarded_bound.reset();
    counted = limited = false;
    Worker::reset();
    start();
    initialize(s,false);
  }

  NoGoods&
  CABS::nogoods(void) {
    return NoGoods::eng;
  }

}}}

// STATISTICS: search-other
