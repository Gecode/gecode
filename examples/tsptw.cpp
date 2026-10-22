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

#include "tsptw-data.hh"
#include "tsptw-transition.hh"
#include <gecode/search.hh>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>

using namespace Gecode;

// Construct every successor and visit-time variable before search. A circuit
// visits each customer once; timing constraints link departure to depot return.
// Times need no branching: if any schedule fits a fixed route, its earliest
// schedule also fits, because arriving early permits waiting.
class TSPTWClosed : public Space {
public:
  std::shared_ptr<const TSPTW::Instance> data;
  IntVarArray successor, arc_cost, visit;
  IntVar departure, returning, total;

  explicit TSPTWClosed(std::shared_ptr<const TSPTW::Instance> instance)
    : data(std::move(instance)),
      successor(*this,data->n,0,data->n-1),
      arc_cost(*this,data->n,0,data->cost_limit),
      visit(*this,data->n-1,0,data->time_limit),
      departure(*this,0,0), returning(*this,0,data->due[0]),
      total(*this,0,data->cost_limit) {
    const int n=data->n;
    // Indirect travel can be cheaper than a direct arc. Shortest paths give
    // lower bounds on arrival from the depot and on travel back to it, even
    // when those paths use customers that the tour has already visited.
    const auto shortest=TSPTW::shortest_paths(*data);
    std::vector<int> lower(n,0), upper(n,data->due[0]);
    for (int i=1; i<n; i++) {
      const long long lo=std::max<long long>(data->ready[i],shortest[i]);
      const long long hi=std::min<long long>(data->due[i],
                                            data->due[0]-shortest[i*n]);
      if (lo>hi) { fail(); return; }
      lower[i]=static_cast<int>(lo);
      upper[i]=static_cast<int>(hi);
      dom(*this,visit[i-1],lower[i],upper[i]);
    }
    circuit(*this,IntArgs(n*n,data->cost.data()),successor,arc_cost,total,
            IPL_DOM);
    for (int i=0; i<n; i++) {
      rel(*this,successor[i],IRT_NQ,i);
      for (int j=0; j<n; j++) {
        if (i==j) continue;
        const int cost=data->arc(i,j);
        if (static_cast<long long>(lower[i])+cost>upper[j]) {
          rel(*this,successor[i],IRT_NQ,j);
          continue;
        }
        BoolVar edge(*this,0,1);
        rel(*this,successor[i],IRT_EQ,j,Reify(edge,RM_EQV));
        const IntVar from=i==0 ? departure : visit[i-1];
        const IntVar to=j==0 ? returning : visit[j-1];
        linear(*this,IntArgs({1,-1}),IntVarArgs({from,to}),IRT_LQ,-cost,
               Reify(edge,RM_IMP),IPL_BND);
      }
    }
    branch(*this,arc_cost,INT_VAR_REGRET_MAX_MAX(),INT_VAL_MIN());
    branch(*this,successor,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
  }

  TSPTWClosed(TSPTWClosed& s) : Space(s), data(s.data) {
    successor.update(*this,s.successor);
    arc_cost.update(*this,s.arc_cost);
    visit.update(*this,s.visit);
    departure.update(*this,s.departure);
    returning.update(*this,s.returning);
    total.update(*this,s.total);
  }
  Space* copy(void) override { return new TSPTWClosed(*this); }
  int cost(void) const { return total.val(); }
  static Search::HeuristicGuidance guidance(void) {
    return {Search::HeuristicGuidance::MINIMIZE,
      [](const Space& s)->std::int64_t {
        return static_cast<const TSPTWClosed&>(s).total.min();
      }};
  }
  void constrain(const Space& s0) override {
    const auto& s=static_cast<const TSPTWClosed&>(s0);
    rel(*this,total,IRT_LE,s.total.val());
  }
  std::vector<int> tour(void) const {
    std::vector<int> result;
    int node=0;
    for (int i=1; i<data->n; i++) {
      node=successor[node].val();
      if (node==0) throw std::runtime_error("incomplete circuit solution");
      result.push_back(node);
    }
    if (successor[node].val()!=0)
      throw std::runtime_error("circuit solution does not return to depot");
    return result;
  }
};

struct TourValue {
  int cost, return_time;
};

// Reconstruct the earliest schedule and travel cost from customer IDs alone.
// This validates both formulations independently of their variable assignments.
static TourValue evaluate_tour(const TSPTW::Instance& data,
                               const std::vector<int>& tour) {
  if (tour.size()!=static_cast<std::size_t>(data.n-1))
    throw std::runtime_error("tour omits customers or has extra visits");
  std::vector<bool> seen(data.n,false);
  for (int node : tour) {
    if (node<=0 || node>=data.n || seen[node])
      throw std::runtime_error("tour has a duplicate or invalid customer");
    seen[node]=true;
  }
  long long time=0, cost=0;
  int previous=0;
  for (std::size_t i=0; i<=tour.size(); i++) {
    const int node=i==tour.size() ? 0 : tour[i];
    const int arc=data.arc(previous,node);
    time=std::max<long long>(time+arc,data.ready[node]);
    cost+=arc;
    if (time>data.due[node])
      throw std::runtime_error(node==0 ? "late return" : "late customer");
    previous=node;
  }
  // The input reader checks upper bounds on tour cost and time, so both
  // totals fit the model's integer type.
  return {static_cast<int>(cost),static_cast<int>(time)};
}

static std::string exact_text(int value, int scale) {
  std::string text=std::to_string(value/scale);
  if (value%scale) {
    std::string fraction=std::to_string(scale+value%scale).substr(1);
    while (fraction.back()=='0') fraction.pop_back();
    text+='.'+fraction;
  }
  return text;
}

static void write_tour(const std::vector<int>& tour, TourValue value, int scale) {
  std::cout << "\"tour\":[";
  for (std::size_t i=0; i<tour.size(); i++) {
    if (i) std::cout << ',';
    std::cout << tour[i];
  }
  std::cout << "],\"scaled_cost\":" << value.cost << ",\"scale\":" << scale
            << ",\"cost\":\"" << exact_text(value.cost,scale)
            << "\",\"return_time\":\"" << exact_text(value.return_time,scale) << '"';
}

struct RunOptions {
  std::string instance, engine="bab", formulation="closed";
  std::string bound="prefix", residual_propagation="off";
  std::optional<unsigned long long> time_ms, node_limit;
  Search::CABSOptions cabs;
  unsigned int threads=1;
  bool dominance=true, dominance_set=false;
  bool width_set=false, frontier_set=false;
};

static unsigned long long read_unsigned(const std::string& text) {
  if (text.empty() || text.find_first_not_of("0123456789")!=std::string::npos)
    throw std::runtime_error("expected a nonnegative integer");
  return std::stoull(text);
}

static RunOptions read_options(int argc, char* argv[]) {
  if (argc<2)
    throw std::runtime_error("usage: tsptw INSTANCE [--engine bab|astar|cabs] "
      "[--formulation closed|transition] "
      "[--bound prefix|min-arcs] [--residual-propagation off|cheap|unary] "
      "[--threads N] [--time-ms N] [--node-limit N] "
      "[--max-width N] [--frontier-limit N] [--dominance off|on]");
  RunOptions options;
  options.instance=argv[1];
  for (int i=2; i<argc; i+=2) {
    const std::string option=argv[i];
    if (i+1==argc) throw std::runtime_error("missing value for "+option);
    const std::string value=argv[i+1];
    if (option=="--engine") options.engine=value;
    else if (option=="--formulation") options.formulation=value;
    else if (option=="--bound") options.bound=value;
    else if (option=="--residual-propagation") options.residual_propagation=value;
    else if (option=="--dominance") {
      if (value!="off" && value!="on")
        throw std::runtime_error("dominance must be off or on");
      options.dominance=value=="on"; options.dominance_set=true;
    }
    else if (option=="--time-ms") options.time_ms=read_unsigned(value);
    else if (option=="--node-limit") options.node_limit=read_unsigned(value);
    else if (option=="--threads") {
      const auto n=read_unsigned(value);
      if (n==0 || n>std::numeric_limits<unsigned int>::max())
        throw std::runtime_error("threads must be a positive integer");
      options.threads=static_cast<unsigned int>(n);
    }
    else if (option=="--max-width" || option=="--frontier-limit") {
      const auto limit=read_unsigned(value);
      if (limit>std::numeric_limits<unsigned long>::max())
        throw std::runtime_error("search limit is too large");
      if (option=="--max-width") {
        options.cabs.max_width=limit; options.width_set=true;
      } else {
        options.cabs.frontier_limit=limit; options.frontier_set=true;
      }
    } else throw std::runtime_error("unknown option: "+option);
  }
  if (options.engine!="bab" && options.engine!="astar" &&
      options.engine!="cabs")
    throw std::runtime_error("engine must be bab, astar, or cabs");
  if (options.formulation!="closed" && options.formulation!="transition")
    throw std::runtime_error("formulation must be closed or transition");
  if (options.bound!="prefix" && options.bound!="min-arcs")
    throw std::runtime_error("bound must be prefix or min-arcs");
  if (options.residual_propagation!="off" && options.residual_propagation!="cheap" &&
      options.residual_propagation!="unary")
    throw std::runtime_error("residual propagation must be off, cheap or unary");
  if (options.formulation=="closed" &&
      (options.bound!="prefix" || options.residual_propagation!="off"))
    throw std::runtime_error("bound and residual propagation switches require transition");
  if ((options.width_set || options.frontier_set) && options.engine!="cabs")
    throw std::runtime_error("width and frontier limits require cabs");
  if (options.dominance_set &&
      (options.engine!="cabs" || options.formulation=="closed"))
    throw std::runtime_error("dominance requires transition cabs");
  return options;
}

class LimitsStop : public Search::Stop {
  std::unique_ptr<Search::Stop> time, nodes;
public:
  explicit LimitsStop(const RunOptions& options) {
    if (options.time_ms) time.reset(Search::Stop::time(*options.time_ms));
    if (options.node_limit) nodes.reset(Search::Stop::node(*options.node_limit));
  }
  bool stop(const Search::Statistics& s, const Search::Options& o) override {
    return (time && time->stop(s,o)) || (nodes && nodes->stop(s,o));
  }
};

template<class Model>
static int run_search(const RunOptions& options, Model& root,
                      const Search::Options& search) {
  const auto& data=root.data;
  std::optional<TourValue> incumbent;
  std::vector<int> best_tour;
  const auto guidance=[&]() {
    if constexpr (std::is_same<Model,TSPTW::Transition>::value)
      return Model::guidance(options.bound=="prefix" ? Model::PREFIX : Model::MIN_ARCS);
    else return Model::guidance();
  }();
  std::unique_ptr<Search::Engine> engine;
  if (options.engine=="bab") engine.reset(Search::babengine(&root,search));
  else if (options.engine=="astar")
    engine.reset(Search::astarengine(&root,guidance,search));
  else {
    auto cabs=options.cabs;
    if constexpr (std::is_same<Model,TSPTW::Transition>::value)
      if (options.dominance) {
        cabs.dominates=Model::dominates;
        cabs.dominance_key=Model::dominance_key;
      }
    engine.reset(Search::cabsengine(&root,guidance,search,cabs));
  }
  // Search clones root (search.clone is true), so the caller retains it.
  // The caller owns the stop object and keeps it alive until engine destruction.
  while (std::unique_ptr<Model> solution{
           static_cast<Model*>(engine->next())}) {
    auto tour=solution->tour();
    const TourValue value=evaluate_tour(*data,tour);
    if (value.cost!=solution->cost() || (incumbent && value.cost>=incumbent->cost))
      throw std::runtime_error("invalid or nonimproving model solution");
    incumbent=value;
    best_tour=std::move(tour);
    std::cout << "{\"type\":\"solution\",";
    write_tour(best_tour,value,data->scale);
    std::cout << '}' << std::endl;
  }
  const auto statistics=engine->statistics();
  const char* termination=engine->stopped() ? "stopped" :
                          incumbent ? "optimal" : "infeasible";
  std::cout << "{\"type\":\"summary\",\"engine\":\"" << options.engine
            << "\",\"formulation\":\"" << options.formulation
            << "\",\"bound\":\"" << (options.formulation=="closed" ? "cp-total" : options.bound)
            << "\",\"residual_propagation\":\"" << options.residual_propagation
            << "\",\"termination\":\"" << termination << '"';
  std::cout << ",\"threads\":" << options.threads;
  if (options.engine=="cabs" && options.formulation!="closed")
    std::cout << ",\"dominance\":\"" << (options.dominance ? "on" : "off") << '"';
  std::cout << ",\"incumbent\":";
  if (incumbent) {
    std::cout << '{';
    write_tour(best_tour,*incumbent,data->scale);
    std::cout << '}';
  } else std::cout << "null";
  std::cout << ",\"statistics\":{\"node\":"
            << statistics.node << ",\"fail\":" << statistics.fail
            << ",\"depth\":" << statistics.depth << ",\"propagate\":"
            << statistics.propagate << ",\"restart\":" << statistics.restart
            << ",\"nogood\":" << statistics.nogood << "}}\n";
  return 0;
}

static int run(const RunOptions& options) {
  LimitsStop stop(options);
  auto data=std::make_shared<const TSPTW::Instance>(TSPTW::read(options.instance));
  Search::Options search;
  search.clone=true;
  search.threads=options.threads;
  search.stop=&stop;
  if (options.formulation=="closed") {
    TSPTWClosed root(data);
    return run_search(options,root,search);
  }
  TSPTW::Transition root(data,options.residual_propagation=="off" ?
                         TSPTW::Transition::OFF : options.residual_propagation=="cheap" ?
                         TSPTW::Transition::CHEAP : TSPTW::Transition::UNARY);
  return run_search(options,root,search);
}

int main(int argc, char* argv[]) {
  try {
    return run(read_options(argc,argv));
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
