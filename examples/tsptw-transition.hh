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
    std::uint64_t bound_calls=0, inference_calls=0;
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
    Gecode::SpaceComparison compare(const Gecode::Space& s) const override;
    void constrain(const Gecode::Space& s) override;
    std::vector<int> tour(void) const;
    int cost(void) const { return prefix_cost; }
    int allocated_positions(void) const {
      return representation==LAZY ? lazy.prefix_size() : eager.size();
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
#ifdef GECODE_HAS_EXPERIMENTAL
    static Gecode::Search::HeuristicGuidance
    guidance(BoundMode mode=PREFIX,
             std::shared_ptr<Measurements> measurements=nullptr) {
      return {Gecode::Search::HeuristicGuidance::MINIMIZE,
        [mode,measurements](const Gecode::Space& s)->std::int64_t {
          const auto start=std::chrono::steady_clock::now();
          const auto result=static_cast<const Transition&>(s).bound(mode);
          if (measurements) {
            measurements->bound_calls++;
            measurements->bound_ms+=std::chrono::duration<double,std::milli>(
              std::chrono::steady_clock::now()-start).count();
          }
          return result;
        }};
    }

    /// Cost-aware Pareto labels for this model's history-independent rules.
    static Gecode::Search::StatePruning
    pareto_pruning(unsigned long int descriptor_limit=0) {
      using Pruning=Gecode::Search::StatePruning;
      return {[](const Gecode::Space& s)->std::optional<Pruning::Descriptor> {
        const auto& state=static_cast<const Transition&>(s);
        Pruning::Descriptor descriptor{0,{state.location},
                                       {state.time,state.prefix_cost}};
        descriptor.key.insert(descriptor.key.end(),
                              state.unvisited.begin(),state.unvisited.end());
        for (std::int64_t value : descriptor.key)
          descriptor.hash=descriptor.hash*31+std::hash<std::int64_t>{}(value);
        return descriptor;
      },[](const Pruning::Descriptor& a, const Pruning::Descriptor& b) {
        if (a.label==b.label) return Pruning::EQUIVALENT;
        if (a.label[0]<=b.label[0] && a.label[1]<=b.label[1])
          return Pruning::DOMINATES;
        if (a.label[0]>=b.label[0] && a.label[1]>=b.label[1])
          return Pruning::DOMINATED;
        return Pruning::INCOMPARABLE;
      },descriptor_limit};
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
    bool prepare(Gecode::Space& home, Gecode::Search::Statistics& stat,
                 const Gecode::Search::Options&) override {
      auto& s=static_cast<Transition&>(home);
      (void) s.materialize(s.depth);
      (void) s.status(stat);
      return true;
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
        measurements->propagation_ms+=std::chrono::duration<double,std::milli>(
          std::chrono::steady_clock::now()-propagation_start).count();
      if (feasible)
        for (std::size_t k=0; k<unvisited.size(); k++) {
          const int u=unvisited[k];
          const int earliest=std::max(time+data->arc(location,u),data->ready[u]);
          if (schedule.starts[static_cast<int>(k)].in(earliest))
            residual_actions.push_back(u);
        }
    }
    if (measurements) {
      measurements->inference_calls++;
      measurements->inference_ms+=std::chrono::duration<double,std::milli>(
        std::chrono::steady_clock::now()-start).count();
    }
    return feasible;
  }

  inline Gecode::SpaceComparison
  Transition::compare(const Gecode::Space& s0) const {
    const auto& s=static_cast<const Transition&>(s0);
    if (cost()==s.cost()) return Gecode::SC_EQUIVALENT;
    return cost()<s.cost() ? Gecode::SC_BETTER : Gecode::SC_WORSE;
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
