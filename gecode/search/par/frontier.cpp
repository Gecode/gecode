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

#include <gecode/search/par/frontier.hh>
#include <gecode/search/support.hh>
#include <algorithm>
#include <limits>

#if defined(GECODE_HAS_EXPERIMENTAL) && defined(GECODE_HAS_THREADS)
namespace Gecode { namespace Search { namespace Par {

  bool
  Frontier::Earlier::operator()(const Entry& a, const Entry& b) const {
    if ((beam ? ranked : ordering == AStarOrder::RANK_FIRST) && a.rank != b.rank) {
      return a.rank < b.rank;
    }
    if ((!beam || !ranked) && a.bound != b.bound) {
      return sense == HeuristicGuidance::Sense::MINIMIZE ?
        a.bound < b.bound : a.bound > b.bound;
    }
    if (!beam && a.rank != b.rank) {
      return a.rank < b.rank;
    }
    return a.order < b.order;
  }

  Frontier::Earlier
  Frontier::earlier(void) const {
    return {guidance.sense,ordering,beam,static_cast<bool>(guidance.rank)};
  }

  Frontier::Runner::Runner(Frontier& e, unsigned int i)
    : Support::Runnable(false), engine(e), id(i) {}

  void
  Frontier::Runner::run(void) {
    engine.work(id);
  }

  Support::Terminator*
  Frontier::Runner::terminator(void) const {
    return const_cast<Runner*>(this);
  }

  void
  Frontier::Runner::terminated(void) {
    done.signal();
  }

  Frontier::Frontier(Space* s, const HeuristicGuidance& g, AStarOrder ordering0,
                     const Options& o, bool beam0, const CABSOptions& limits0)
    : Search::Engine(o,static_cast<unsigned int>(o.threads)), opt(o),
      guidance(g), ordering(ordering0), beam(beam0), limits(limits0) {
    initialize(s,o.clone);
  }

  void
  Frontier::initialize(Space* s, bool clone) {
    if (!s) {
      stat.fail++;
      return;
    }
    const SpaceStatus status=s->status(stat);
    if (status == SS_FAILED) {
      stat.fail++;
      if (!clone) {
        delete s;
      }
      return;
    }
    auto first=std::make_unique<ExpansionTask>();
    std::unique_ptr<Space> restart_root;
    if (beam) {
      restart_root.reset(s->clone());
    }
    std::unique_ptr<Space> initial(clone ? s->clone() : nullptr);
    // Allocation and cloning must finish before taking the caller's root.
    pending.push_back(std::move(first));
    pending.back()->expansion.select(
      std::unique_ptr<Space>(clone ? initial.release() : s));
    root=std::move(restart_root);
  }

  void
  Frontier::launch(void) {
    if (!runners.empty()) {
      return;
    }
    const auto capacity=static_cast<unsigned int>(opt.threads);
    std::vector<std::unique_ptr<Runner>> allocated;
    allocated.reserve(capacity);
    for (unsigned int i=0; i<capacity; i++) {
      allocated.push_back(std::make_unique<Runner>(*this,i));
    }
    runners=std::move(allocated);
    unsigned int started=0;
    try {
      for (; started<capacity; started++) {
        Support::Thread::run(runners[started].get());
      }
    } catch (...) {
      {
        Support::Lock lock(mutex);
        shutdown=true;
      }
      wake();
      for (unsigned int i=0; i<started; i++) {
        runners[i]->done.wait();
      }
      runners.clear();
      throw;
    }
  }

  void
  Frontier::wake(void) {
    if (worker_control.engaged()) {
      WorkerControlAccess::signal_all(worker_control);
    } else {
      for (const auto& runner : runners) {
        runner->wake.signal();
      }
    }
  }

  Frontier::~Frontier(void) {
    {
      Support::Lock lock(mutex);
      shutdown=true;
      running=false;
    }
    wake();
    for (const auto& runner : runners) {
      runner->done.wait();
    }
  }

  bool
  Frontier::improves(std::int64_t bound) const {
    return !best || (guidance.sense == HeuristicGuidance::Sense::MINIMIZE ?
                    bound < best_bound : bound > best_bound);
  }

  bool
  Frontier::advance(void) {
    assert(active == 0 && pending.empty() && frontier.empty());
    if (!beam) {
      return false;
    }
    if (!candidates.empty()) {
      candidates.release(frontier);
      const Earlier compare=earlier();
      std::sort(frontier.begin(),frontier.end(),
                [&](const Entry& a, const Entry& b) {
                  return compare(b,a);
                });
      return true;
    }
    if (!discarded_bound || !improves(*discarded_bound)) {
      return false;
    }
    if (width > std::numeric_limits<unsigned long int>::max()/2 ||
        (limits.max_width && width*2 > limits.max_width)) {
      limited=true;
      stopped_=true;
      return false;
    }
    width*=2;
    stat.restart++;
    discarded_bound.reset();
    auto first=std::make_unique<ExpansionTask>();
    first->expansion.select(std::unique_ptr<Space>(root->clone()));
    pending.push_back(std::move(first));
    return true;
  }

  std::unique_ptr<Space>
  Frontier::admit(Entry entry, bool parent) {
    if (!improves(entry.bound)) {
      stat.fail++;
      return std::move(entry.space);
    }
    entry.order=order++;
    const Earlier compare=earlier();
    if (!beam) {
      frontier.push_back(std::move(entry));
      std::push_heap(frontier.begin(),frontier.end(),
                     [&](const Entry& a, const Entry& b) {
                       return compare(b,a);
                     });
      return nullptr;
    }
    if (limits.dominates) {
      if (limits.dominance_key) {
        const auto range=candidates.equal_range(entry.key);
        for (auto i=range.first; i!=range.second; ++i) {
          if (limits.dominates(*candidates[i->second].space,*entry.space)) {
            return std::move(entry.space);
          }
        }
        std::vector<const std::size_t*> victims;
        for (auto i=range.first; i!=range.second; ++i) {
          if (limits.dominates(*entry.space,*candidates[i->second].space)) {
            victims.push_back(&i->second);
          }
        }
        if (!victims.empty()) {
          // Slot addresses survive heap moves and index rehashes. Erase each
          // victim through its current slot, without scanning the beam.
          Entry removed=candidates.replace(*victims.front(),std::move(entry),true,compare);
          for (std::size_t i=1; i<victims.size(); i++) {
            candidates.erase(*victims[i],compare);
          }
          return std::move(removed.space);
        }
      } else {
        for (const auto& candidate : candidates.all()) {
          if (limits.dominates(*candidate.space,*entry.space)) {
            return std::move(entry.space);
          }
        }
        std::vector<std::size_t> victims;
        for (std::size_t i=0; i<candidates.size(); i++) {
          if (limits.dominates(*entry.space,*candidates[i].space)) {
            victims.push_back(i);
          }
        }
        if (!victims.empty()) {
          Entry removed=candidates.replace(victims,std::move(entry),compare);
          return std::move(removed.space);
        }
      }
    }
    if (candidates.size() == width) {
      const bool replace=compare(entry,candidates.front());
      const std::int64_t discarded=replace ? candidates.front().bound : entry.bound;
      if (!discarded_bound || (guidance.sense == HeuristicGuidance::Sense::MINIMIZE ?
                              discarded < *discarded_bound :
                              discarded > *discarded_bound)) {
        discarded_bound=discarded;
      }
      if (!replace) {
        return std::move(entry.space);
      }
      Entry removed=candidates.replace(0,std::move(entry),
        limits.dominates && limits.dominance_key,compare);
      return std::move(removed.space);
    }
    // Reserve room for each other active worker's parent and unsubmitted child.
    assert(active > 0);
    const auto retained=frontier.size()+candidates.size()+pending.size()+
      2*(active-1)+parent;
    if (limits.frontier_limit && retained >= limits.frontier_limit) {
      limited=true;
      stopped_=true;
      running=false;
      return std::move(entry.space);
    }
    candidates.push(std::move(entry),limits.dominates && limits.dominance_key,compare);
    return nullptr;
  }

  void
  Frontier::publish(std::unique_ptr<Space> s, std::int64_t bound) {
    if (!improves(bound)) {
      return;
    }
    std::unique_ptr<Space> copy(s->clone());
    best=std::move(s);
    best_bound=bound;
    revision++;
    solution=std::move(copy);
    running=false;
  }

  std::unique_ptr<Frontier::ExpansionTask>
  Frontier::select_task(unsigned int id) {
    while (running) {
      if (pending.empty() && frontier.empty() && active == 0 && !advance()) {
        running=false;
      }
      if (!running || (pending.empty() && frontier.empty())) {
        return nullptr;
      }
      if (!pending.empty()) {
        auto task=std::move(pending.front());
        pending.pop_front();
        return task;
      }
      if (!beam) {
        const Earlier compare=earlier();
        std::pop_heap(frontier.begin(),frontier.end(),
          [&](const Entry& a, const Entry& b) {
            return compare(b,a);
          });
      }
      Entry entry=std::move(frontier.back());
      frontier.pop_back();
      if (!improves(entry.bound)) {
        stat.fail++;
        if (opt.stop && opt.stop->stop(stat,opt)) {
          stopped_=true;
          running=false;
        }
        if (worker_control.engaged() &&
            id >= WorkerControlAccess::requested(worker_control)) {
          return nullptr;
        }
        continue;
      }
      auto task=std::make_unique<ExpansionTask>();
      task->expansion.select(std::move(entry.space));
      task->depth=entry.depth;
      task->revision=entry.revision;
      task->counted=true;
      return task;
    }
    return nullptr;
  }

  void
  Frontier::work(unsigned int id) {
    std::unique_ptr<ExpansionTask> task;
    std::unique_ptr<Space> incumbent;
    unsigned long long int current_revision=0;
    Worker local;
    bool counted_active=false;
    std::unique_ptr<Space> found_solution;
    std::int64_t solution_bound=0;
    Entry batch[8];
    unsigned int batch_size=0;
    const unsigned int batch_capacity=beam && !limits.frontier_limit ? 8 : 1;
    std::optional<std::int64_t> cutoff;
    while (true) {
      try {
        bool notify=false;
        bool retiring=false;
        {
          Support::Lock lock(mutex);
          const bool empty=pending.empty() && frontier.empty();
          if (counted_active) {
            stat+=local;
            local.reset();
            for (unsigned int i=0; i<batch_size; i++) {
              batch[i].space=admit(std::move(batch[i]),
                                   static_cast<bool>(task->expansion));
            }
            if (found_solution) {
              publish(std::move(found_solution),solution_bound);
            }
          }
          if (running && opt.stop && opt.stop->stop(stat,opt)) {
            stopped_=true;
            running=false;
          }
          const bool enabled=!worker_control.engaged() ||
            id < WorkerControlAccess::requested(worker_control);
          if (task && (!task->expansion || !running || !enabled || shutdown)) {
            // Count this worker as active until it has disposed of all rejected
            // or replaced spaces, so next() cannot return during model cleanup.
            for (unsigned int i=0; i<batch_size; i++) {
              retiring=retiring || static_cast<bool>(batch[i].space);
            }
            if (!retiring) {
              if (task->expansion) {
                pending.push_back(std::move(task));
              }
              task.reset();
              active--;
              counted_active=false;
            }
          }
          if (shutdown && !retiring) {
            incumbent.reset();
            return;
          }
          if (running && enabled && !task) {
            task=select_task(id);
            if (task) {
              active++;
              counted_active=true;
            }
          }
          if (task) {
            if (current_revision != revision) {
              incumbent.reset();
            }
            if (best && task->revision != revision && !incumbent) {
              incumbent.reset(best->clone());
            }
            current_revision=revision;
            cutoff=best ? std::optional<std::int64_t>(best_bound) : std::nullopt;
          } else {
            // Dispose of the cached incumbent before next() can return.
            incumbent.reset();
            current_revision=0;
          }
          notify=empty && (!pending.empty() || !frontier.empty());
          if (!running && active == 0) {
            changed.signal();
          }
        }
        if (notify) {
          wake();
        }
        for (unsigned int i=0; i<batch_size; i++) {
          batch[i].space.reset();
        }
        batch_size=0;
        if (retiring) {
          continue;
        }
        if (!task) {
          if (worker_control.engaged()) {
            WorkerControlAccess::wait(worker_control,id);
          } else {
            runners[id]->wake.wait();
          }
          continue;
        }
        if (!task->counted) {
          local.node++;
          local.stack_depth(task->depth);
          task->counted=true;
        }
        // Bound both sibling batches and the work between stop checks, even
        // for choices with many failing children.
        for (unsigned int i=0; i<32 && task->expansion; i++) {
          Seq::ExpansionStep step=task->expansion.next(local,task->depth+1,
            task->revision,incumbent.get(),current_revision);
          if (step.state == Seq::ExpansionStep::State::EXHAUSTED) {
            break;
          }
          if (step.status == SS_FAILED) {
            local.fail++;
          } else {
            if (step.status == SS_SOLVED) {
              (void) step.space->choice();
            }
            const std::int64_t bound=guidance.bound(*step.space);
            if (step.status == SS_SOLVED) {
              found_solution=std::move(step.space);
              solution_bound=bound;
              break;
            }
            if (cutoff &&
                (guidance.sense == HeuristicGuidance::Sense::MINIMIZE ?
                 bound >= *cutoff : bound <= *cutoff)) {
              local.fail++;
            } else {
              const std::int64_t rank=guidance.rank ? guidance.rank(*step.space) : 0;
              const std::size_t key=beam && limits.dominates && limits.dominance_key ?
                limits.dominance_key(*step.space) : 0;
              batch[batch_size++]={std::move(step.space),bound,rank,key,
                                   task->depth+1,0,current_revision};
              if (batch_size == batch_capacity) {
                break;
              }
            }
          }
          step.space.reset();
          if (worker_control.engaged() &&
              id >= WorkerControlAccess::requested(worker_control)) {
            break;
          }
        }
      } catch (...) {
        {
          Support::Lock lock(mutex);
          found_solution.reset();
          for (unsigned int i=0; i<batch_size; i++) {
            batch[i].space.reset();
          }
          batch_size=0;
          task.reset();
          incumbent.reset();
          current_revision=0;
          if (counted_active) {
            active--;
            stat+=local;
            local.reset();
            counted_active=false;
          }
          if (!error) {
            error=std::current_exception();
          }
          running=false;
          if (active == 0) {
            changed.signal();
          }
        }
        wake();
      }
    }
  }

  Space*
  Frontier::next(void) {
    launch();
    {
      Support::Lock lock(mutex);
      if (error) {
        std::rethrow_exception(error);
      }
      if (limited) {
        return nullptr;
      }
      stopped_=false;
      running=true;
    }
    wake();
    while (true) {
      changed.wait();
      Support::Lock lock(mutex);
      if (running || active) {
        continue;
      }
      if (error) {
        std::rethrow_exception(error);
      }
      return solution.release();
    }
  }

  Search::Statistics
  Frontier::statistics(void) const {
    Support::Lock lock(mutex);
    return stat;
  }

  bool
  Frontier::stopped(void) const {
    Support::Lock lock(mutex);
    return stopped_;
  }

  void
  Frontier::constrain(const Space& s) {
    const std::int64_t bound=guidance.bound(s);
    Support::Lock lock(mutex);
    if (!improves(bound)) {
      return;
    }
    std::unique_ptr<Space> copy(s.clone());
    best=std::move(copy);
    best_bound=bound;
    revision++;
    solution.reset();
  }

  void
  Frontier::reset(Space* s) {
    std::unique_ptr<Space> replacement(s);
    Support::Lock lock(mutex);
    assert(!running && active == 0);
    pending.clear();
    frontier.clear();
    candidates.clear();
    root.reset();
    best.reset();
    solution.reset();
    width=1;
    order=0;
    revision=0;
    discarded_bound.reset();
    stopped_=false;
    limited=false;
    stat.reset();
    initialize(s,false);
    replacement.release();
  }

}}}
#endif

// STATISTICS: search-par
