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

#if defined(GECODE_HAS_EXPERIMENTAL) && defined(GECODE_HAS_THREADS)
namespace Gecode { namespace Search { namespace Par {

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

  Frontier::Frontier(const Options& o)
    : Search::Engine(o,static_cast<unsigned int>(o.threads)), opt(o),
      guidance(*o.guidance), best(guidance.sense), order(0),
      pending(heap), n_pending(0), runners(nullptr), active(0),
      running(false), shutdown(false), _stopped(false), _limited(false),
      batch(1), keyed(false) {}

  void
  Frontier::started(const Space&) {}

  void
  Frontier::push(Task* t) {
    pending.push(t); n_pending++;
  }

  void
  Frontier::initialize(Space* s, bool clone) {
    if (s == nullptr) {
      stat.fail++;
      return;
    }
    if (s->status(stat) == SS_FAILED) {
      stat.fail++;
      if (!clone)
        delete s;
      return;
    }
    started(*s);
    std::unique_ptr<Task> t(new Task);
    t->expansion.select(std::unique_ptr<Space>(clone ? s->clone() : s));
    push(t.release());
  }

  void
  Frontier::launch(void) {
    if (runners != nullptr)
      return;
    unsigned int n = static_cast<unsigned int>(opt.threads);
    runners = heap.alloc<Runner*>(n);
    for (unsigned int i=0; i<n; i++)
      runners[i] = new Runner(*this,i);
    unsigned int started = 0;
    try {
      for (; started<n; started++)
        Support::Thread::run(runners[started]);
    } catch (...) {
      { Support::Lock lock(mutex); shutdown = true; }
      wake();
      for (unsigned int i=0; i<started; i++)
        runners[i]->done.wait();
      for (unsigned int i=0; i<n; i++)
        delete runners[i];
      heap.free<Runner*>(runners,n);
      runners = nullptr;
      // All started runners have terminated, so there are no concurrent readers.
      shutdown = false;
      throw;
    }
  }

  void
  Frontier::wake(void) {
    if (worker_control.engaged()) {
      WorkerControlAccess::signal_all(worker_control);
    } else if (runners != nullptr) {
      for (unsigned int i=0; i<static_cast<unsigned int>(opt.threads); i++)
        runners[i]->wake.signal();
    }
  }

  bool
  Frontier::enabled(unsigned int id) {
    return !worker_control.engaged() ||
      (id < WorkerControlAccess::requested(worker_control));
  }

  void
  Frontier::terminate(void) {
    if (runners == nullptr)
      return;
    { Support::Lock lock(mutex); shutdown = true; running = false; }
    wake();
    unsigned int n = static_cast<unsigned int>(opt.threads);
    for (unsigned int i=0; i<n; i++)
      runners[i]->done.wait();
    for (unsigned int i=0; i<n; i++)
      delete runners[i];
    heap.free<Runner*>(runners,n);
    runners = nullptr;
  }

  Frontier::~Frontier(void) {
    terminate();
    while (n_pending > 0) {
      delete pending.pop(); n_pending--;
    }
  }

  void
  Frontier::publish(Space* s, std::int64_t b) {
    if (!best.improves(b)) {
      delete s;
      return;
    }
    std::unique_ptr<Space> incumbent(s);
    solution.reset(s->clone());
    best.update(incumbent.release(),b);
    running = false;
  }

  void
  Frontier::work(unsigned int id) {
    // The leased task and the incumbent used for constraining its children
    Task* task = nullptr;
    std::unique_ptr<Space> incumbent;
    unsigned long long int revision = 0;
    std::optional<std::int64_t> cutoff;
    Worker local;
    // Children and solution found since the last submission
    Entry children[max_batch];
    unsigned int n_children = 0, submitted = 0;
    std::unique_ptr<Space> solved;
    std::int64_t solved_bound = 0;
    while (true) {
      try {
        bool notify = false;
        {
          Support::Lock lock(mutex);
          bool idle = (n_pending == 0) && !open();
          bool enabled = this->enabled(id);
          if (task != nullptr) {
            stat += local;
            local.reset();
            for (submitted=0; submitted<n_children; submitted++)
              submit(children[submitted],static_cast<bool>(task->expansion));
            n_children = submitted = 0;
            if (solved)
              publish(solved.release(),solved_bound);
          }
          if (running && (opt.stop != nullptr) && opt.stop->stop(stat,opt)) {
            _stopped = true; running = false;
          }
          if (task != nullptr) {
            if (!task->expansion || !running || !enabled || shutdown) {
              if (task->expansion)
                push(task);
              else
                delete task;
              task = nullptr;
              active--;
              // An enabled worker must check for termination
              notify = (active == 0) && running && !enabled;
            }
          }
          if (shutdown) {
            incumbent.reset();
            return;
          }
          while (running && enabled && (task == nullptr)) {
            if ((n_pending == 0) && !open() && (active == 0) && !advance())
              running = false;
            if (!running || ((n_pending == 0) && !open()))
              break;
            if (n_pending > 0) {
              task = pending.pop(); n_pending--;
            } else {
              Entry e = select();
              std::unique_ptr<Space> s(e.space);
              if (!best.improves(e.bound)) {
                stat.fail++;
                if ((opt.stop != nullptr) && opt.stop->stop(stat,opt)) {
                  _stopped = true; running = false;
                }
                continue;
              }
              task = new Task;
              task->expansion.select(std::move(s));
              task->depth = e.depth;
              task->revision = e.revision;
              task->counted = true;
            }
            active++;
          }
          if (task != nullptr) {
            if (revision != best.revision)
              incumbent.reset();
            if (best.space && (task->revision != best.revision) && !incumbent)
              incumbent.reset(best.space->clone());
            revision = best.revision;
            cutoff.reset();
            if (best.space)
              cutoff = best.bound;
          } else {
            incumbent.reset();
            revision = 0;
          }
          notify = notify || (idle && ((n_pending > 0) || open()));
          if (!running && (active == 0))
            changed.signal();
        }
        if (notify)
          wake();
        if (task == nullptr) {
          if (worker_control.engaged())
            WorkerControlAccess::wait(worker_control,id);
          else
            runners[id]->wake.wait();
          continue;
        }
        if (!task->counted) {
          local.node++;
          local.stack_depth(task->depth);
          task->counted = true;
        }
        // Limit the work between stop checks
        for (unsigned int i=0; (i<32) && task->expansion; i++) {
          Seq::ExpansionStep step =
            task->expansion.next(local,task->depth+1,task->revision,
                                 incumbent.get(),revision);
          if (step.state == Seq::ExpansionStep::EXHAUSTED)
            break;
          if (step.status != SS_FAILED) {
            if (step.status == SS_SOLVED)
              (void) step.space->choice();
            std::int64_t b = guidance.bound(*step.space);
            if (step.status == SS_SOLVED) {
              if (!cutoff || better(guidance.sense,b,*cutoff)) {
                solved = std::move(step.space);
                solved_bound = b;
                break;
              }
              continue;
            }
            if ((step.status == SS_BRANCH) &&
                (!cutoff || better(guidance.sense,b,*cutoff))) {
              std::int64_t r = guidance.rank ? guidance.rank(*step.space) : 0;
              std::size_t k = keyed ? guidance.dominance_key(*step.space) : 0;
              children[n_children++] =
                {step.space.release(),b,r,k,task->depth+1,0,revision};
              if (n_children == batch)
                break;
              continue;
            }
          }
          local.fail++;
          if (!enabled(id))
            break;
        }
      } catch (...) {
        Support::Lock lock(mutex);
        for (unsigned int i=submitted; i<n_children; i++)
          delete children[i].space;
        n_children = 0; submitted = 0;
        solved.reset();
        if (task != nullptr) {
          delete task;
          task = nullptr;
          active--;
          stat += local;
        }
        local.reset();
        incumbent.reset();
        revision = 0;
        if (!error)
          error = std::current_exception();
        running = false;
        if (active == 0)
          changed.signal();
      }
    }
  }

  Space*
  Frontier::next(void) {
    launch();
    {
      Support::Lock lock(mutex);
      if (error)
        std::rethrow_exception(error);
      if (_limited) {
        _stopped = true;
        return nullptr;
      }
      _stopped = false;
      running = true;
    }
    wake();
    while (true) {
      changed.wait();
      Support::Lock lock(mutex);
      if (running || (active > 0))
        continue;
      if (error)
        std::rethrow_exception(error);
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
    return _stopped;
  }

  bool
  Frontier::limited(void) const {
    Support::Lock lock(mutex);
    return _limited;
  }

  void
  Frontier::constrain(const Space& s) {
    std::int64_t b = guidance.bound(s);
    Support::Lock lock(mutex);
    if (best.improves(b)) {
      best.update(s.clone(),b);
      solution.reset();
    }
  }

  void
  Frontier::reset(Space* s) {
    std::unique_ptr<Space> replacement(s);
    Support::Lock lock(mutex);
    while (n_pending > 0) {
      delete pending.pop(); n_pending--;
    }
    clear();
    best.reset();
    solution.reset();
    order = 0;
    _stopped = _limited = false;
    error = nullptr;
    stat.reset();
    initialize(s,false);
    // The root is owned by the engine after initialize returns
    (void) replacement.release();
  }

}}}
#endif

// STATISTICS: search-par
