/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#ifndef GECODE_EXAMPLES_TSPTW_TRANSITION_HH
#define GECODE_EXAMPLES_TSPTW_TRANSITION_HH

#include "tsptw-data.hh"
#include <gecode/search.hh>
#include <chrono>
#include <memory>
#include <numeric>

namespace TSPTW {

  /// One run owns these aggregates; clones and callbacks count repeated work.
  struct Measurements {
    const bool parallel;
    Gecode::Support::Mutex mutex;
    explicit Measurements(bool p=false) : parallel(p) {}
    std::uint64_t bound_calls=0, inference_calls=0;
    void add(double& total, double elapsed, std::uint64_t* calls=nullptr) {
      if (parallel) mutex.acquire();
      total+=elapsed;
      if (calls) (*calls)++;
      if (parallel) mutex.release();
    }
    double bound_ms=0, inference_ms=0, propagation_ms=0;
  };

  /// One concrete visit per search edge, with an explicit depot return.
  class Transition : public Gecode::Space {
  public:
    enum Representation { LAZY, EAGER };
    enum BoundMode { PREFIX, MIN_ARCS };
    enum ResidualPropagation { OFF, CHEAP, UNARY };
    std::shared_ptr<const Instance> data;
    std::vector<int> unvisited;
    int location=0, time=0, prefix_cost=0, depth=0;
    bool returned=false;

    explicit Transition(std::shared_ptr<const Instance> instance,
                        Representation representation=LAZY,
                        ResidualPropagation propagation=OFF,
                        std::shared_ptr<Measurements> measurements=nullptr);
    Transition(Transition& s);
    Gecode::Space* copy(void) override { return new Transition(*this); }
    void constrain(const Gecode::Space& s) override;
    std::vector<int> tour(void) const;
    int cost(void) const { return prefix_cost; }
    int allocated_positions(void) const {
      return representation==LAZY ? lazy.known_length() : eager.size();
    }
    Gecode::IntVar position(int p) const {
      return representation==LAZY ? lazy[p] : eager[p];
    }
    std::vector<int> actions(void) const {
      std::vector<int> result;
      if (returned) return result;
      if (unvisited.empty()) {
        if (applicable(0)) result.push_back(0);
      } else {
        for (int node : unvisited)
          if (applicable(node)) result.push_back(node);
      }
      return result;
    }
    std::int64_t bound(BoundMode mode) const;
    /// Equal residual tours share a key; the predicate checks resource dominance.
    static std::size_t dominance_key(const Gecode::Space& s0) {
      const auto& s=static_cast<const Transition&>(s0);
      std::size_t key=static_cast<std::size_t>(s.location);
      for (int node : s.unvisited) Gecode::cmb_hash(key,node);
      return key;
    }
    /// Same residual tour, no later arrival, and no greater travel cost.
    static bool dominates(const Gecode::Space& a0, const Gecode::Space& b0) {
      const auto& a=static_cast<const Transition&>(a0);
      const auto& b=static_cast<const Transition&>(b0);
      return a.data==b.data && a.location==b.location &&
        a.unvisited==b.unvisited && a.returned==b.returned &&
        a.time<=b.time && a.prefix_cost<=b.prefix_cost;
    }
#ifdef GECODE_HAS_EXPERIMENTAL
    static Gecode::Search::HeuristicGuidance
    guidance(BoundMode mode=PREFIX,
             std::shared_ptr<Measurements> measurements=nullptr) {
      return {Gecode::Search::HeuristicGuidance::MINIMIZE,
        [mode,measurements](const Gecode::Space& s)->std::int64_t {
          const auto start=std::chrono::steady_clock::now();
          const auto result=static_cast<const Transition&>(s).bound(mode);
          if (measurements) {
            measurements->add(measurements->bound_ms,
              std::chrono::duration<double,std::milli>(
                std::chrono::steady_clock::now()-start).count(),
              &measurements->bound_calls);
          }
          return result;
        }};
    }

#endif

  private:
    class VisitBrancher;
    Representation representation;
    ResidualPropagation propagation;
    std::shared_ptr<const std::vector<std::int64_t>> distance;
    std::shared_ptr<Measurements> measurements;
    std::vector<int> residual_actions;
    Gecode::OpenIntVarList lazy;
    Gecode::IntVarArray eager;
    int ceiling;

    bool applicable(int node) const {
      return time+data->arc(location,node)<=data->due[node] &&
        prefix_cost+data->arc(location,node)<ceiling &&
        (propagation!=UNARY ||
         std::find(residual_actions.begin(),residual_actions.end(),node)!=
           residual_actions.end());
    }
    bool refresh_inference(void);
    Gecode::IntVar materialize(int p) {
      return representation==LAZY ? lazy.get(*this,p) : eager[p];
    }
  };

  class Transition::VisitBrancher : public Gecode::Brancher {
    class VisitChoice : public Gecode::Choice {
    public:
      int position;
      std::vector<int> actions;
      VisitChoice(const Gecode::Brancher& b, int p, std::vector<int> a)
        : Choice(b,static_cast<unsigned int>(a.size())),
          position(p), actions(std::move(a)) {}
      void archive(Gecode::Archive& e) const override {
        Choice::archive(e);
        e << position << static_cast<unsigned int>(actions.size());
        for (int node : actions) e << node;
      }
    };
  public:
    VisitBrancher(Gecode::Home home) : Brancher(home) {}
    VisitBrancher(Gecode::Space& home, VisitBrancher& b) : Brancher(home,b) {}
    bool status(const Gecode::Space& home) const override {
      return !static_cast<const Transition&>(home).returned;
    }
    void prepare(Gecode::Space& home, Gecode::StatusStatistics& stat) override {
      auto& s = static_cast<Transition&>(home);
      (void) s.materialize(s.depth);
      (void) s.status(stat);
    }
    const Gecode::Choice* choice(Gecode::Space& home) override {
      const auto& s=static_cast<const Transition&>(home);
      return new VisitChoice(*this,s.depth,s.actions());
    }
    const Gecode::Choice* choice(const Gecode::Space&,
                                 Gecode::Archive& e) override {
      int position;
      unsigned int count;
      e >> position >> count;
      std::vector<int> actions(count);
      for (int& node : actions) e >> node;
      return new VisitChoice(*this,position,std::move(actions));
    }
    Gecode::ExecStatus commit(Gecode::Space& home, const Gecode::Choice& c,
                              unsigned int alternative) override {
      auto& s=static_cast<Transition&>(home);
      const auto& choice=static_cast<const VisitChoice&>(c);
      const int node=choice.actions[alternative];
      const auto customer=std::find(s.unvisited.begin(),s.unvisited.end(),node);
      if (s.returned || choice.position!=s.depth || !s.applicable(node) ||
          (node==0 ? !s.unvisited.empty() : customer==s.unvisited.end()))
        return Gecode::ES_FAILED;
      // BAB replay can skip prepare for historical choices. Allocating the
      // recorded position has no visit effects, just as in prepare.
      Gecode::IntVar next=s.materialize(choice.position);
      if (home.failed() || Gecode::me_failed(Gecode::Int::IntView(next).eq(home,node)))
        return Gecode::ES_FAILED;
      const int arc=s.data->arc(s.location,node);
      s.time=std::max(s.time+arc,s.data->ready[node]);
      s.prefix_cost+=arc;
      s.location=node;
      s.depth++;
      if (node==0) {
        s.returned=true;
        if (s.representation==LAZY) s.lazy.close(home);
      } else {
        s.unvisited.erase(customer);
      }
      // Rebuild from the new residual state, including historical BAB replay.
      if (!s.refresh_inference() || (!s.returned && s.actions().empty()))
        return Gecode::ES_FAILED;
      return home.failed() ? Gecode::ES_FAILED : Gecode::ES_OK;
    }
    void print(const Gecode::Space&, const Gecode::Choice& c,
               unsigned int alternative, std::ostream& out) const override {
      const auto& choice=static_cast<const VisitChoice&>(c);
      out << "visit[" << choice.position << "] = " << choice.actions[alternative];
    }
    Gecode::Actor* copy(Gecode::Space& home) override {
      return new (home) VisitBrancher(home,*this);
    }
    size_t dispose(Gecode::Space& home) override {
      (void) Brancher::dispose(home);
      return sizeof(*this);
    }
  };

  inline Transition::Transition(std::shared_ptr<const Instance> instance,
                                Representation representation0,
                                ResidualPropagation propagation0,
                                std::shared_ptr<Measurements> measurements0)
    : data(std::move(instance)), unvisited(data->n-1),
      representation(representation0), propagation(propagation0),
      measurements(std::move(measurements0)), ceiling(data->cost_limit+1) {
    std::iota(unvisited.begin(),unvisited.end(),1);
    if (propagation!=OFF)
      distance=std::make_shared<const std::vector<std::int64_t>>(
        shortest_paths(*data));
    if (representation==LAZY)
      lazy=Gecode::OpenIntVarList(*this,
        [](Gecode::Space& home, int) {
          const auto& s=static_cast<const Transition&>(home);
          return Gecode::IntVar(home,0,s.data->n-1);
        },data->n);
    else
      eager=Gecode::IntVarArray(*this,data->n,0,data->n-1);
    (void) new (*this) VisitBrancher(*this);
    if (!refresh_inference() || actions().empty()) fail();
  }

  inline Transition::Transition(Transition& s)
    : Space(s), data(s.data), unvisited(s.unvisited), location(s.location),
      time(s.time), prefix_cost(s.prefix_cost), depth(s.depth),
      returned(s.returned), representation(s.representation),
      propagation(s.propagation), distance(s.distance), measurements(s.measurements),
      residual_actions(s.residual_actions), ceiling(s.ceiling) {
    if (representation==LAZY) lazy.update(*this,s.lazy);
    else eager.update(*this,s.eager);
  }

  inline std::int64_t Transition::bound(BoundMode mode) const {
    if (returned || mode==PREFIX) return prefix_cost;
    if (unvisited.empty()) return add_cost(prefix_cost,data->arc(location,0));
    std::int64_t incoming=0, outgoing=0;
    int enter=std::numeric_limits<int>::max();
    int returning=std::numeric_limits<int>::max();
    for (int u : unvisited) {
      enter=std::min(enter,data->arc(location,u));
      returning=std::min(returning,data->arc(u,0));
      int leave=data->arc(u,0), arrive=data->arc(location,u);
      for (int v : unvisited) if (u!=v) {
        leave=std::min(leave,data->arc(u,v));
        arrive=std::min(arrive,data->arc(v,u));
      }
      outgoing=add_cost(outgoing,leave);
      incoming=add_cost(incoming,arrive);
    }
    outgoing=add_cost(outgoing,enter);
    incoming=add_cost(incoming,returning);
    return add_cost(prefix_cost,std::max(incoming,outgoing));
  }

  /// Every residual tour induces nonoverlapping minimum-outgoing intervals.
  class ResidualSchedule : public Gecode::Space {
  public:
    Gecode::IntVarArray starts;
    ResidualSchedule(const Instance& data, const std::vector<int>& unvisited,
                     int location, int time,
                     const std::vector<std::int64_t>& distance)
      : starts(*this,static_cast<int>(unvisited.size()),0,data.time_limit) {
      Gecode::IntArgs durations(starts.size());
      for (int k=0; k<starts.size(); k++) {
        const int u=unvisited[k];
        int duration=data.arc(u,0);
        for (int v : unvisited) if (u!=v)
          duration=std::min(duration,data.arc(u,v));
        durations[k]=duration;
        const auto lower=std::max<std::int64_t>(data.ready[u],
          add_cost(time,distance[location*data.n+u]));
        const auto upper=std::min<std::int64_t>(data.due[u],
          std::min<std::int64_t>(data.due[0]-distance[u*data.n],
                                data.due[0]-duration));
        if (lower>upper) { fail(); return; }
        Gecode::dom(*this,starts[k],static_cast<int>(lower),static_cast<int>(upper));
      }
      Gecode::unary(*this,starts,durations,Gecode::IPL_DEF);
    }
    ResidualSchedule(ResidualSchedule& s) : Space(s) {
      starts.update(*this,s.starts);
    }
    Gecode::Space* copy(void) override { return new ResidualSchedule(*this); }
  };

  inline bool Transition::refresh_inference(void) {
    if (propagation==OFF) return true;
    const auto start=std::chrono::steady_clock::now();
    residual_actions.clear();
    bool feasible=true;
    if (!returned && unvisited.empty()) {
      if (time+data->arc(location,0)<=data->due[0]) residual_actions.push_back(0);
      else feasible=false;
    } else if (!returned && propagation==CHEAP) {
      // Shortest paths may use unavailable vertices: this weakens the check
      // but keeps it necessary for asymmetric and nonmetric travel matrices.
      for (int u : unvisited) {
        const auto earliest=std::max<std::int64_t>(data->ready[u],
          add_cost(time,(*distance)[location*data->n+u]));
        if (earliest>data->due[u] ||
            add_cost(earliest,(*distance)[u*data->n])>data->due[0]) {
          feasible=false;
          break;
        }
      }
    } else if (!returned) {
      ResidualSchedule schedule(*data,unvisited,location,time,*distance);
      const auto propagation_start=std::chrono::steady_clock::now();
      feasible=schedule.status()!=Gecode::SS_FAILED;
      if (measurements)
        measurements->add(measurements->propagation_ms,
          std::chrono::duration<double,std::milli>(
            std::chrono::steady_clock::now()-propagation_start).count());
      if (feasible)
        for (std::size_t k=0; k<unvisited.size(); k++) {
          const int u=unvisited[k];
          const int earliest=std::max(time+data->arc(location,u),data->ready[u]);
          if (schedule.starts[static_cast<int>(k)].in(earliest))
            residual_actions.push_back(u);
        }
    }
    if (measurements) {
      measurements->add(measurements->inference_ms,
        std::chrono::duration<double,std::milli>(
          std::chrono::steady_clock::now()-start).count(),
        &measurements->inference_calls);
    }
    return feasible;
  }


  inline void Transition::constrain(const Gecode::Space& s0) {
    ceiling=std::min(ceiling,static_cast<const Transition&>(s0).cost());
    if (cost()>=ceiling || (!returned && actions().empty())) fail();
  }

  inline std::vector<int> Transition::tour(void) const {
    if (!returned || depth!=data->n)
      throw std::runtime_error("incomplete transition solution");
    std::vector<int> result;
    for (int p=0; p<depth-1; p++) result.push_back(position(p).val());
    return result;
  }

}
#endif
