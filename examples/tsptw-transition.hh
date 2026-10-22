/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#ifndef GECODE_EXAMPLES_TSPTW_TRANSITION_HH
#define GECODE_EXAMPLES_TSPTW_TRANSITION_HH

#include "tsptw-data.hh"
#include <gecode/search.hh>
#include <limits>
#include <memory>
#include <numeric>

namespace TSPTW {

  /// A search edge visits one customer, or returns to the depot after all visits.
  /// Position variables are created as the route grows; remaining customers
  /// are stored as IDs.
  class Transition : public Gecode::Space {
  public:
    enum BoundMode { PREFIX, MIN_ARCS };
    enum ResidualPropagation { OFF, CHEAP, UNARY };
    std::shared_ptr<const Instance> data;
    std::vector<int> unvisited;
    // time includes waiting; prefix_cost counts travel only.
    int location=0, time=0, prefix_cost=0, depth=0;
    bool returned=false;

    explicit Transition(std::shared_ptr<const Instance> instance,
                        ResidualPropagation propagation=OFF);
    Transition(Transition& s);
    Gecode::Space* copy(void) override { return new Transition(*this); }
    void constrain(const Gecode::Space& s) override;
    std::vector<int> tour(void) const;
    int cost(void) const { return prefix_cost; }
    int allocated_positions(void) const {
      return positions.known_length();
    }
    Gecode::IntVar position(int p) const {
      return positions[p];
    }
    std::vector<int> actions(void) const {
      std::vector<int> result;
      if (returned) return result;
      if (unvisited.empty()) {
        if (can_visit(0)) result.push_back(0);
      } else {
        for (int node : unvisited)
          if (can_visit(node)) result.push_back(node);
      }
      return result;
    }
    std::int64_t bound(BoundMode mode) const;
    /// Group prefixes with the same location and remaining customers.
    /// The dominance predicate checks equality and resource bounds after hashing.
    static std::size_t dominance_key(const Gecode::Space& s0) {
      const auto& s=static_cast<const Transition&>(s0);
      std::size_t key=static_cast<std::size_t>(s.location);
      for (int node : s.unvisited) Gecode::cmb_hash(key,node);
      return key;
    }
    /// A prefix dominates another if it can follow every remaining route
    /// with no later visit times and no greater total travel cost.
    static bool dominates(const Gecode::Space& a0, const Gecode::Space& b0) {
      const auto& a=static_cast<const Transition&>(a0);
      const auto& b=static_cast<const Transition&>(b0);
      return a.data==b.data && a.location==b.location &&
        a.unvisited==b.unvisited && a.returned==b.returned &&
        a.time<=b.time && a.prefix_cost<=b.prefix_cost;
    }
#ifdef GECODE_HAS_EXPERIMENTAL
    static Gecode::Search::HeuristicGuidance
    guidance(BoundMode mode=PREFIX) {
      return {Gecode::Search::HeuristicGuidance::MINIMIZE,
        [mode](const Gecode::Space& s)->std::int64_t {
          return static_cast<const Transition&>(s).bound(mode);
        }};
    }

#endif

  private:
    class VisitBrancher;
    ResidualPropagation propagation;
    std::shared_ptr<const std::vector<std::int64_t>> distance;
    std::vector<int> residual_actions;
    Gecode::OpenIntVarList positions;
    int ceiling;

    // Waiting is allowed, so arrival before ready time remains feasible.
    // The ceiling is an exclusive upper bound on travel cost.
    bool can_visit(int node) const {
      return time+data->arc(location,node)<=data->due[node] &&
        prefix_cost+data->arc(location,node)<ceiling &&
        (propagation!=UNARY ||
         std::find(residual_actions.begin(),residual_actions.end(),node)!=
           residual_actions.end());
    }
    bool filter_remaining_visits(void);
    Gecode::IntVar materialize(int p) {
      return positions.get(*this,p);
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
      if (s.returned || choice.position!=s.depth || !s.can_visit(node) ||
          (node==0 ? !s.unvisited.empty() : customer==s.unvisited.end()))
        return Gecode::ES_FAILED;
      // Branch-and-bound recomputation can replay a choice without prepare.
      // Materialize its recorded variable here too; only commit advances the tour.
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
        s.positions.close(home);
      } else {
        s.unvisited.erase(customer);
      }
      // Recompute deadline checks and allowed next visits from the updated
      // prefix, whether this choice is fresh or replayed during recomputation.
      if (!s.filter_remaining_visits() || (!s.returned && s.actions().empty()))
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
                                ResidualPropagation propagation0)
    : data(std::move(instance)), unvisited(data->n-1),
      propagation(propagation0),
      ceiling(data->cost_limit+1) {
    std::iota(unvisited.begin(),unvisited.end(),1);
    if (propagation!=OFF)
      distance=std::make_shared<const std::vector<std::int64_t>>(
        shortest_paths(*data));
    positions=Gecode::OpenIntVarList(*this,
      [](Gecode::Space& home, int) {
        const auto& s=static_cast<const Transition&>(home);
        return Gecode::IntVar(home,0,s.data->n-1);
      },data->n);
    (void) new (*this) VisitBrancher(*this);
    if (!filter_remaining_visits() || actions().empty()) fail();
  }

  inline Transition::Transition(Transition& s)
    : Space(s), data(s.data), unvisited(s.unvisited), location(s.location),
      time(s.time), prefix_cost(s.prefix_cost), depth(s.depth),
      returned(s.returned),
      propagation(s.propagation), distance(s.distance),
      residual_actions(s.residual_actions), ceiling(s.ceiling) {
    positions.update(*this,s.positions);
  }

  inline std::int64_t Transition::bound(BoundMode mode) const {
    if (returned || mode==PREFIX) return prefix_cost;
    if (unvisited.empty()) return prefix_cost+data->arc(location,0);
    // Every completion enters each remaining customer and the depot once,
    // and leaves the current location and each remaining customer once.
    // Independent cheapest incoming/outgoing arcs can underestimate these
    // costs; the larger of their totals is still a lower bound.
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
      outgoing+=leave;
      incoming+=arrive;
    }
    outgoing+=enter;
    incoming+=returning;
    return prefix_cost+std::max(incoming,outgoing);
  }

  /// Relax remaining visits to tasks that cannot overlap.
  /// Each task starts at a customer visit and lasts as long as the cheapest
  /// outgoing arc from that customer. A feasible tour gives a feasible schedule,
  /// so schedule failure rules out the prefix.
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
        // Shortest travel times bound arrival from the current location and
        // leave enough time to return to the depot by its deadline.
        const auto lower=std::max<std::int64_t>(data.ready[u],
          time+distance[location*data.n+u]);
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

  inline bool Transition::filter_remaining_visits(void) {
    if (propagation==OFF) return true;
    residual_actions.clear();
    if (returned) return true;
    if (unvisited.empty()) {
      if (time+data->arc(location,0)>data->due[0]) return false;
      residual_actions.push_back(0);
      return true;
    }
    if (propagation==CHEAP) {
      // Allowing paths through already visited customers can only shorten
      // travel. A failed deadline check therefore rules out every completion,
      // even when arcs are asymmetric or indirect travel is cheaper.
      for (int u : unvisited) {
        const auto earliest=std::max<std::int64_t>(data->ready[u],
          time+(*distance)[location*data->n+u]);
        if (earliest>data->due[u] ||
            earliest+(*distance)[u*data->n]>data->due[0])
          return false;
      }
      return true;
    }
    ResidualSchedule schedule(*data,unvisited,location,time,*distance);
    if (schedule.status()==Gecode::SS_FAILED) return false;
    for (std::size_t k=0; k<unvisited.size(); k++) {
      const int u=unvisited[k];
      const int earliest=std::max(time+data->arc(location,u),data->ready[u]);
      // The next customer is visited at its earliest possible time.
      // Reject it if the scheduling relaxation has removed that time.
      if (schedule.starts[static_cast<int>(k)].in(earliest))
        residual_actions.push_back(u);
    }
    return true;
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
