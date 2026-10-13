/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#ifndef GECODE_SEARCH_COMPLETION_HH
#define GECODE_SEARCH_COMPLETION_HH

#include <gecode/search.hh>
#include <memory>

namespace Gecode { namespace Search {

  /// Brancher-owned existential DFS, advanced at the native choice boundary.
  class Completion : public Brancher {
    struct Pending : Stop {
      std::unique_ptr<DFS<Space>> engine;
      Statistics published;
      Statistics* caller;
      const Options* options;

      Pending(void) : caller(nullptr), options(nullptr) {}

      Statistics delta(const Statistics& s) const {
        Statistics d=s;
        d.node-=published.node; d.fail-=published.fail;
        d.propagate-=published.propagate;
        d.restart-=published.restart; d.nogood-=published.nogood;
        // Depth is the maximum constituent depth, as in operator+=.
        return d;
      }
      bool stop(const Statistics& s, const Options&) override {
        if (!options || !options->stop) return false;
        Statistics total=*caller;
        total+=delta(s);
        return options->stop->stop(total,*options);
      }
      void publish(void) {
        if (engine && caller) {
          Statistics s=engine->statistics();
          *caller+=delta(s);
          published=s;
        }
      }
    };
    Pending* pending;
    enum Result { UNSET, FOUND, EXHAUSTED } result;
    unsigned int revision;
  protected:
    bool done;
    bool suppressed;

    Completion(Home home)
      : Brancher(home), pending(nullptr), result(UNSET), revision(0),
        done(false), suppressed(false) {
      static_cast<Space&>(home).retain_branchers();
    }
    Completion(Space& home, Completion& b)
      : Brancher(home,b), pending(nullptr), result(b.result),
        revision(b.revision), done(b.done), suppressed(b.suppressed) {}

    /// Clone the trial root and post its ordinary completion branching.
    virtual Space* trial(Space& home)=0;

    class CompletionChoice : public Choice {
    public:
      bool fail;
      unsigned int revision;
      CompletionChoice(const Brancher& b, bool f, unsigned int r)
        : Choice(b,1), fail(f), revision(r) {}
      size_t size(void) const { return sizeof(*this); }
      void archive(Archive& e) const override {
        Choice::archive(e); e.put(fail); e.put(revision);
      }
    };

  public:
    bool retained(void) const override { return !suppressed; }
    void invalidate(Space&) override {
      delete pending; pending=nullptr;
      result=UNSET; done=false; revision++;
    }
    bool prepare(Space& home, Statistics& stat,
                 const Options& options) override {
      if (result != UNSET) return true;
      if (options.stop && options.stop->stop(stat,options)) return false;
      if (!pending) pending=new Pending;
      pending->caller=&stat;
      pending->options=&options;
      try {
        if (!pending->engine) {
          Options o=options;
          o.clone=false; o.threads=1; o.stop=pending; o.tracer=nullptr;
          // The trial runs inside the owning worker's engine. Its control is
          // already bound there and cannot be attached to a second engine.
          o.worker_control=WorkerControl();
          // Suppression belongs only to this trial clone, never to its source.
          suppressed=true;
          std::unique_ptr<Space> root;
          try { root.reset(trial(home)); }
          catch (...) { suppressed=false; throw; }
          suppressed=false;
          pending->engine.reset(new DFS<Space>(root.get(),o));
          root.release();
        }
        std::unique_ptr<Space> solution(pending->engine->next());
        pending->publish();
        if (solution) result=FOUND;
        else if (!pending->engine->stopped()) result=EXHAUSTED;
        pending->caller=nullptr; pending->options=nullptr;
      } catch (...) {
        pending->publish();
        pending->caller=nullptr; pending->options=nullptr;
        throw;
      }
      if (result == UNSET) return false;
      delete pending; pending=nullptr;
      return true;
    }
    const Choice* choice(Space& home) override {
      Statistics stat;
      Options options;
      (void) prepare(home,stat,options);
      return new CompletionChoice(*this,result==EXHAUSTED,revision);
    }
    const Choice* choice(const Space&, Archive& e) override {
      bool fail; unsigned int r;
      e >> fail >> r;
      return new CompletionChoice(*this,fail,r);
    }
    ExecStatus commit(Space&, const Choice& c, unsigned int) override {
      const CompletionChoice& ch=static_cast<const CompletionChoice&>(c);
      if (ch.fail) return ES_FAILED;
      // A strengthened replay must return to preparation before being solved.
      if (revision == ch.revision) { done=true; result=FOUND; }
      return ES_OK;
    }
    size_t dispose(Space& home) override {
      delete pending; pending=nullptr;
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }
  };
}}

#endif
