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
#include <algorithm>
#include <limits>

#if defined(GECODE_HAS_EXPERIMENTAL) && defined(GECODE_HAS_THREADS)
namespace Gecode { namespace Search { namespace Par {

  bool
  Frontier::Earlier::operator()(const Entry& a, const Entry& b) const {
    if ((beam ? ranked : ordering == RANK_FIRST) && a.rank != b.rank)
      return a.rank < b.rank;
    if ((!beam || !ranked) && a.bound != b.bound)
      return sense == HeuristicGuidance::MINIMIZE ?
        a.bound < b.bound : a.bound > b.bound;
    if (!beam && a.rank != b.rank) return a.rank < b.rank;
    return a.order < b.order;
  }

  Frontier::Earlier
  Frontier::earlier(void) const {
    return {guidance.sense,ordering,beam,static_cast<bool>(guidance.rank)};
  }

  Frontier::Runner::Runner(Frontier& e, unsigned int i)
    : Support::Runnable(false), engine(e), id(i) {}

  void
  Frontier::Runner::run(void) { engine.work(id); }

  Support::Terminator*
  Frontier::Runner::terminator(void) const {
    return const_cast<Runner*>(this);
  }

  void
  Frontier::Runner::terminated(void) { done.signal(); }

  Frontier::Frontier(Space* s, const HeuristicGuidance& g, AStarOrder ordering0,
                     const Options& o, bool beam0, const CABSOptions& limits0)
    : Search::Engine(o,static_cast<unsigned int>(o.threads)), opt(o),
      guidance(g), ordering(ordering0), beam(beam0), limits(limits0) {
    initialize(s,o.clone);
  }

  void
  Frontier::initialize(Space* s, bool clone) {
    if (!s) { stat.fail++; return; }
    const SpaceStatus status=s->status(stat);
    if (status == SS_FAILED) {
      stat.fail++;
      if (!clone) delete s;
      return;
    }
    auto first=std::make_unique<Pending>();
    std::unique_ptr<Space> restart_root;
    if (beam) restart_root.reset(s->clone());
    std::unique_ptr<Space> initial(clone ? s->clone() : nullptr);
    // Allocation and cloning must finish before taking the caller's root.
    pending.push_back(std::move(first));
    pending.back()->expansion.select(
      std::unique_ptr<Space>(clone ? initial.release() : s));
    root=std::move(restart_root);
  }

  void
  Frontier::launch(void) {
    if (!runners.empty()) return;
    const auto capacity=static_cast<unsigned int>(opt.threads);
    std::vector<std::unique_ptr<Runner>> allocated;
    allocated.reserve(capacity);
    for (unsigned int i=0; i<capacity; i++)
      allocated.push_back(std::make_unique<Runner>(*this,i));
    runners=std::move(allocated);
    unsigned int started=0;
    try {
      for (; started<capacity; started++) Support::Thread::run(runners[started].get());
    } catch (...) {
      { Support::Lock lock(mutex); shutdown=true; }
      wake();
      for (unsigned int i=0; i<started; i++) runners[i]->done.wait();
      runners.clear();
      throw;
    }
  }

  void
  Frontier::wake(void) {
    if (worker_control.engaged()) WorkerControlAccess::signal_all(worker_control);
    else for (const auto& r : runners) r->wake.signal();
  }

  Frontier::~Frontier(void) {
    { Support::Lock lock(mutex); shutdown=true; running=false; }
    wake();
    for (const auto& r : runners) r->done.wait();
  }

  bool
  Frontier::improves(std::int64_t bound) const {
    return !best || (guidance.sense == HeuristicGuidance::MINIMIZE ?
                    bound < best_bound : bound > best_bound);
  }

  bool
  Frontier::advance(void) {
    assert(active == 0 && pending.empty() && frontier.empty());
    if (!beam) return false;
    if (!candidates.empty()) {
      candidate_index.clear();
      frontier.swap(candidates);
      const Earlier cmp=earlier();
      std::sort(frontier.begin(),frontier.end(),
                [&](const Entry& a, const Entry& b) { return cmp(b,a); });
      return true;
    }
    if (!discarded_bound || !improves(*discarded_bound)) return false;
    if (width > std::numeric_limits<unsigned long int>::max()/2 ||
        (limits.max_width && width*2 > limits.max_width)) {
      limited=stopped_=true;
      return false;
    }
    width*=2;
    stat.restart++;
    discarded_bound.reset();
    auto first=std::make_unique<Pending>();
    first->expansion.select(std::unique_ptr<Space>(root->clone()));
    pending.push_back(std::move(first));
    return true;
  }

  void
  Frontier::unindex(const Entry& entry) {
    if (!limits.dominates || !limits.dominance_key) return;
    const auto range=candidate_index.equal_range(entry.key);
    for (auto i=range.first; i!=range.second; ++i)
      if (i->second == entry.space.get()) {
        candidate_index.erase(i);
        return;
      }
    GECODE_NEVER;
  }

  void
  Frontier::admit(Entry entry, bool parent) {
    if (!improves(entry.bound)) { stat.fail++; return; }
    entry.order=order++;
    const Earlier cmp=earlier();
    if (!beam) {
      frontier.push_back(std::move(entry));
      std::push_heap(frontier.begin(),frontier.end(),
                     [&](const Entry& a, const Entry& b) { return cmp(b,a); });
      return;
    }
    if (limits.dominates) {
      std::vector<std::size_t> victims;
      if (limits.dominance_key) {
        const auto range=candidate_index.equal_range(entry.key);
        for (auto i=range.first; i!=range.second; ++i)
          if (limits.dominates(*i->second,*entry.space)) return;
        std::vector<const Space*> dominated;
        for (auto i=range.first; i!=range.second; ++i)
          if (limits.dominates(*entry.space,*i->second)) dominated.push_back(i->second);
        // Heap entries move; recover victim slots only after finding dominance.
        if (!dominated.empty())
          for (std::size_t i=0; i<candidates.size(); i++)
            if (std::find(dominated.begin(),dominated.end(),candidates[i].space.get())
                != dominated.end()) victims.push_back(i);
      } else {
        for (const auto& candidate : candidates)
          if (limits.dominates(*candidate.space,*entry.space)) return;
        for (std::size_t i=0; i<candidates.size(); i++)
          if (limits.dominates(*entry.space,*candidates[i].space)) victims.push_back(i);
      }
      if (!victims.empty()) {
        if (limits.dominance_key)
          candidate_index.emplace(entry.key,entry.space.get());
        for (auto i : victims) unindex(candidates[i]);
        candidates[victims.front()]=std::move(entry);
        for (std::size_t i=victims.size(); i>1; i--)
          candidates.erase(candidates.begin()+victims[i-1]);
        std::make_heap(candidates.begin(),candidates.end(),cmp);
        return;
      }
    }
    if (candidates.size() == width) {
      const bool replace=cmp(entry,candidates.front());
      const std::int64_t discarded=replace ? candidates.front().bound : entry.bound;
      if (!discarded_bound || (guidance.sense == HeuristicGuidance::MINIMIZE ?
                              discarded < *discarded_bound :
                              discarded > *discarded_bound)) discarded_bound=discarded;
      if (!replace) return;
      if (limits.dominates && limits.dominance_key)
        candidate_index.emplace(entry.key,entry.space.get());
      unindex(candidates.front());
      std::pop_heap(candidates.begin(),candidates.end(),cmp);
      candidates.back()=std::move(entry);
      std::push_heap(candidates.begin(),candidates.end(),cmp);
      return;
    }
    // Other leases reserve a parent and an unsubmitted successor.
    assert(active > 0);
    const auto retained=frontier.size()+candidates.size()+pending.size()+
      2*(active-1)+parent;
    if (limits.frontier_limit && retained >= limits.frontier_limit) {
      limited=stopped_=true;
      running=false;
      return;
    }
    candidates.push_back(std::move(entry));
    try {
      if (limits.dominates && limits.dominance_key)
        candidate_index.emplace(candidates.back().key,candidates.back().space.get());
    } catch (...) {
      candidates.pop_back();
      throw;
    }
    std::push_heap(candidates.begin(),candidates.end(),cmp);
  }

  void
  Frontier::publish(std::unique_ptr<Space> s, std::int64_t bound) {
    if (!improves(bound)) return;
    std::unique_ptr<Space> copy(s->clone());
    best=std::move(s);
    best_bound=bound;
    revision++;
    solution=std::move(copy);
    running=false;
  }

  void
  Frontier::work(unsigned int id) {
    std::unique_ptr<Pending> task;
    std::unique_ptr<Space> incumbent;
    unsigned long long int current_revision=0;
    Worker local;
    bool leased=false;
    Seq::ExpansionStep step{Seq::ExpansionStep::EXHAUSTED,nullptr,SS_FAILED};
    std::int64_t bound=0, rank=0;
    std::size_t key=0;
    std::optional<std::int64_t> cutoff;
    while (true) {
      try {
        bool notify=false;
        {
          Support::Lock lock(mutex);
          const bool empty=pending.empty() && frontier.empty();
          if (leased) {
            stat+=local;
            local.reset();
            if (step.state != Seq::ExpansionStep::EXHAUSTED) {
              switch (step.status) {
              case SS_FAILED: stat.fail++; break;
              case SS_SOLVED: publish(std::move(step.space),bound); break;
              case SS_BRANCH:
                admit({std::move(step.space),bound,rank,key,
                       task->depth+1,0,current_revision},
                      static_cast<bool>(task->expansion));
                break;
              default: GECODE_NEVER;
              }
            }
            step.space.reset();
            step.state=Seq::ExpansionStep::EXHAUSTED;
          }
          if (running && opt.stop && opt.stop->stop(stat,opt)) {
            stopped_=true;
            running=false;
          }
          const bool enabled=!worker_control.engaged() ||
            id < WorkerControlAccess::requested(worker_control);
          if (task && (!task->expansion || !running || !enabled || shutdown)) {
            if (task->expansion) pending.push_back(std::move(task));
            task.reset();
            active--;
            leased=false;
          }
          if (shutdown) {
            incumbent.reset();
            return;
          }
          if (running && enabled && !task) {
            while (running && !task) {
              if (pending.empty() && frontier.empty() && active == 0 && !advance())
                running=false;
              if (!running || (pending.empty() && frontier.empty())) break;
              if (!pending.empty()) {
                task=std::move(pending.front());
                pending.pop_front();
              } else {
                if (!beam) {
                  const Earlier cmp=earlier();
                  std::pop_heap(frontier.begin(),frontier.end(),
                    [&](const Entry& a, const Entry& b) { return cmp(b,a); });
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
                      id >= WorkerControlAccess::requested(worker_control)) break;
                  continue;
                }
                task=std::make_unique<Pending>();
                task->expansion.select(std::move(entry.space));
                task->depth=entry.depth;
                task->revision=entry.revision;
                task->counted=true;
              }
              active++;
              leased=true;
              break;
            }
          }
          if (task) {
            if (current_revision != revision) incumbent.reset();
            if (best && task->revision != revision && !incumbent)
              incumbent.reset(best->clone());
            current_revision=revision;
            cutoff=best ? std::optional<std::int64_t>(best_bound) : std::nullopt;
          } else {
            // Retire cached model state before the caller observes quiescence.
            incumbent.reset();
            current_revision=0;
          }
          notify=empty && (!pending.empty() || !frontier.empty());
          if (!running && active == 0) changed.signal();
        }
        if (notify) wake();
        if (!task) {
          if (worker_control.engaged()) WorkerControlAccess::wait(worker_control,id);
          else runners[id]->wake.wait();
          continue;
        }
        if (!task->counted) {
          local.node++;
          local.stack_depth(task->depth);
          task->counted=true;
        }
        // Failed alternatives need no shared admission. Keep stop and worker
        // control checks bounded even for choices with many failing children.
        for (unsigned int i=0; i<32 && task->expansion; i++) {
          step=task->expansion.next(local,task->depth+1,
            task->revision,incumbent.get(),current_revision);
          bound=rank=0;
          key=0;
          if (step.space && step.status != SS_FAILED) {
            if (step.status == SS_SOLVED) (void) step.space->choice();
            bound=guidance.bound(*step.space);
            if (step.status == SS_BRANCH && cutoff &&
                (guidance.sense == HeuristicGuidance::MINIMIZE ?
                 bound >= *cutoff : bound <= *cutoff)) {
              step.status=SS_FAILED;
            } else {
              if (step.status == SS_BRANCH && guidance.rank) rank=guidance.rank(*step.space);
              if (step.status == SS_BRANCH && beam && limits.dominates && limits.dominance_key)
                key=limits.dominance_key(*step.space);
              break;
            }
          }
          if (step.state != Seq::ExpansionStep::EXHAUSTED) local.fail++;
          step.space.reset();
          step.state=Seq::ExpansionStep::EXHAUSTED;
          if (worker_control.engaged() &&
              id >= WorkerControlAccess::requested(worker_control)) break;
        }
      } catch (...) {
        {
          Support::Lock lock(mutex);
          step.space.reset();
          step.state=Seq::ExpansionStep::EXHAUSTED;
          task.reset();
          incumbent.reset();
          current_revision=0;
          if (leased) { active--; stat+=local; local.reset(); leased=false; }
          if (!error) error=std::current_exception();
          running=false;
          if (active == 0) changed.signal();
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
      if (error) std::rethrow_exception(error);
      if (limited) { stopped_=true; return nullptr; }
      stopped_=false;
      running=true;
    }
    wake();
    while (true) {
      changed.wait();
      Support::Lock lock(mutex);
      if (running || active) continue;
      if (error) std::rethrow_exception(error);
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
    if (!improves(bound)) return;
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
    candidate_index.clear();
    pending.clear(); frontier.clear(); candidates.clear();
    root.reset(); best.reset(); solution.reset();
    width=1; order=revision=0;
    discarded_bound.reset();
    stopped_=limited=false;
    stat.reset();
    initialize(s,false);
    replacement.release();
  }

}}}
#endif

// STATISTICS: search-par
