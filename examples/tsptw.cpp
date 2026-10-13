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
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>

using namespace Gecode;

// A costed circuit with a departure-to-return timing chain. Time slack
// remains unbranched: a fixed route has an earliest feasible schedule.
class TSPTWClosed : public Space {
public:
  std::shared_ptr<const TSPTW::Instance> data;
  IntVarArray successor, arc_cost, visit;
  IntVar departure, returning, total;

  explicit TSPTWClosed(std::shared_ptr<const TSPTW::Instance> instance,
                       IntPropLevel circuit_propagation, bool size_branching)
    : data(std::move(instance)),
      successor(*this,data->n,0,data->n-1),
      arc_cost(*this,data->n,0,data->cost_limit),
      visit(*this,data->n-1,0,data->time_limit),
      departure(*this,0,0), returning(*this,0,data->due[0]),
      total(*this,0,data->cost_limit) {
    const int n=data->n;
    // Direct arcs need not bound paths in nonmetric data. Shortest paths
    // relax the tour and give sound elapsed-time bounds in both directions.
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
            circuit_propagation);
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
    branch(*this,arc_cost,size_branching ? INT_VAR_SIZE_MIN() :
           INT_VAR_REGRET_MAX_MAX(),INT_VAL_MIN());
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

// Check concrete routes independently of the CP timing and cost variables.
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
  // The reader bounds all n-arc tours and waiting-plus-travel arithmetic.
  return {static_cast<int>(cost),static_cast<int>(time)};
}

static std::vector<int> read_tour(const std::string& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("cannot open seed tour: "+path);
  std::vector<int> tour;
  std::string token;
  while (input>>token) {
    if (token.find_first_not_of("0123456789")!=std::string::npos)
      throw std::runtime_error("invalid seed tour IDs");
    tour.push_back(std::stoi(token));
  }
  if (input.bad()) throw std::runtime_error("cannot read seed tour");
  return tour;
}

static std::unique_ptr<TSPTWClosed>
make_seed(TSPTWClosed& root, const std::vector<int>& tour, TourValue value) {
  if (root.status()==SS_FAILED)
    throw std::runtime_error("checked seed rejected by CP root");
  std::unique_ptr<TSPTWClosed> seed(static_cast<TSPTWClosed*>(root.clone()));
  int previous=0;
  for (int node : tour) {
    rel(*seed,seed->successor[previous],IRT_EQ,node);
    previous=node;
  }
  rel(*seed,seed->successor[previous],IRT_EQ,0);
  if (seed->status()!=SS_SOLVED || seed->total.val()!=value.cost)
    throw std::runtime_error("checked seed rejected by CP model");
  return seed;
}

static std::unique_ptr<TSPTW::Transition>
make_seed(TSPTW::Transition& root, const std::vector<int>& tour, TourValue value) {
  if (root.status() == SS_FAILED)
    throw std::runtime_error("checked seed rejected by transition root");
  std::unique_ptr<TSPTW::Transition> seed(
    static_cast<TSPTW::Transition*>(root.clone()));
  Search::Statistics stat;
  Search::Options options;
  // Replay checked customer IDs and then the explicit return using the same
  // decisions as search, without directly installing residual state.
  for (std::size_t p=0; p<=tour.size(); p++) {
    const int node = p == tour.size() ? 0 : tour[p];
    if (seed->status() != SS_BRANCH || !Search::prepare(*seed,stat,options) ||
        seed->status() != SS_BRANCH)
      throw std::runtime_error("checked seed rejected by transition model");
    const auto actions = seed->actions();
    const auto action = std::find(actions.begin(),actions.end(),node);
    if (action == actions.end())
      throw std::runtime_error("checked seed has no applicable transition");
    std::unique_ptr<const Choice> choice(seed->choice());
    seed->commit(*choice,static_cast<unsigned int>(action-actions.begin()));
  }
  if (seed->status() != SS_SOLVED || seed->cost() != value.cost)
    throw std::runtime_error("checked seed rejected by transition model");
  return seed;
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
  std::string instance, engine="bab", formulation="closed", seed, state_pruning="off";
  std::string bound="prefix", residual_propagation="off";
  std::string circuit_propagation="dom", cost_branching="regret";
  std::optional<unsigned long long> time_ms, node_limit;
  Search::CABSOptions cabs;
  Search::ColumnOptions column;
  bool quota_set=false, width_set=false, frontier_set=false;
  bool closed_settings_set=false;
};

static unsigned long long read_limit(const std::string& text) {
  if (text.empty() || text.find_first_not_of("0123456789")!=std::string::npos)
    throw std::runtime_error("limits must be nonnegative integers");
  return std::stoull(text);
}

static RunOptions read_options(int argc, char* argv[]) {
  if (argc<2)
    throw std::runtime_error("usage: tsptw INSTANCE [--engine bab|rbs|astar|cabs|column] "
      "[--formulation closed|transition|eager] [--state-pruning off|pareto] "
      "[--bound prefix|min-arcs] [--residual-propagation off|unary] "
      "[--circuit-propagation dom|val] [--cost-branching regret|size] "
      "[--seed TOUR] [--time-ms N] [--node-limit N] "
      "[--max-width N] [--frontier-limit N] [--initial-quota N] [--quota-growth N]");
  RunOptions options;
  options.instance=argv[1];
  for (int i=2; i<argc; i+=2) {
    const std::string option=argv[i];
    if (i+1==argc) throw std::runtime_error("missing value for "+option);
    const std::string value=argv[i+1];
    if (option=="--engine") options.engine=value;
    else if (option=="--formulation") options.formulation=value;
    else if (option=="--seed") options.seed=value;
    else if (option=="--state-pruning") options.state_pruning=value;
    else if (option=="--bound") options.bound=value;
    else if (option=="--residual-propagation") options.residual_propagation=value;
    else if (option=="--circuit-propagation" || option=="--cost-branching") {
      if (option=="--circuit-propagation") options.circuit_propagation=value;
      else options.cost_branching=value;
      options.closed_settings_set=true;
    } else if (option=="--time-ms") options.time_ms=read_limit(value);
    else if (option=="--node-limit") options.node_limit=read_limit(value);
    else if (option=="--max-width" || option=="--frontier-limit" ||
             option=="--initial-quota" || option=="--quota-growth") {
      const auto limit=read_limit(value);
      if (limit>std::numeric_limits<unsigned long>::max())
        throw std::runtime_error("search limit is too large");
      if (option=="--max-width") {
        options.cabs.max_width=limit; options.width_set=true;
      } else if (option=="--frontier-limit") {
        options.cabs.frontier_limit=options.column.frontier_limit=limit;
        options.frontier_set=true;
      } else {
        if (option=="--initial-quota") options.column.initial_quota=limit;
        else options.column.quota_growth=limit;
        options.quota_set=true;
      }
    } else throw std::runtime_error("unknown option: "+option);
  }
  if (options.engine!="bab" && options.engine!="astar" &&
      options.engine!="cabs" && options.engine!="column" && options.engine!="rbs")
    throw std::runtime_error("engine must be bab, rbs, astar, cabs, or column");
  if (options.formulation!="closed" && options.formulation!="transition" &&
      options.formulation!="eager")
    throw std::runtime_error("formulation must be closed, transition, or eager");
  if (options.circuit_propagation!="dom" && options.circuit_propagation!="val")
    throw std::runtime_error("circuit propagation must be dom or val");
  if (options.cost_branching!="regret" && options.cost_branching!="size")
    throw std::runtime_error("cost branching must be regret or size");
  if (options.formulation!="closed" &&
      (options.closed_settings_set || options.engine=="rbs"))
    throw std::runtime_error("circuit/branching settings and rbs require closed");
  if (options.state_pruning!="off" && options.state_pruning!="pareto")
    throw std::runtime_error("state pruning must be off or pareto");
  if (options.bound!="prefix" && options.bound!="min-arcs")
    throw std::runtime_error("bound must be prefix or min-arcs");
  if (options.residual_propagation!="off" && options.residual_propagation!="cheap" &&
      options.residual_propagation!="unary")
    throw std::runtime_error("residual propagation must be off, cheap or unary");
  if (options.formulation=="closed" &&
      (options.bound!="prefix" || options.residual_propagation!="off"))
    throw std::runtime_error("bound and residual propagation switches require transition/eager");
  if (options.state_pruning=="pareto" &&
      (options.formulation=="closed" || options.engine=="bab"))
    throw std::runtime_error("Pareto pruning requires transition/eager A*, CABS, or column");
  if (options.width_set && options.engine!="cabs")
    throw std::runtime_error("width limits require cabs");
  if (options.frontier_set && options.engine!="cabs" && options.engine!="column")
    throw std::runtime_error("frontier limits require cabs or column");
  if (options.quota_set && options.engine!="column")
    throw std::runtime_error("quota settings require column");
  if (!options.column.initial_quota)
    throw std::runtime_error("initial quota must be positive");
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
                      const Search::Options& search,
                      std::chrono::steady_clock::time_point start,
                      std::shared_ptr<TSPTW::Measurements> measurements=nullptr) {
  auto elapsed=[start]() {
    return std::chrono::duration<double,std::milli>(
      std::chrono::steady_clock::now()-start).count();
  };
  const auto& data=root.data;
  std::optional<TourValue> incumbent;
  std::vector<int> best_tour;
  if (!options.seed.empty()) {
    best_tour=read_tour(options.seed);
    incumbent=evaluate_tour(*data,best_tour);
  }
  std::unique_ptr<Model> seed;
  if (incumbent) seed=make_seed(root,best_tour,*incumbent);
  int root_positions = 0, prepared_positions = 0, solution_positions = 0;
  if constexpr (std::is_same<Model,TSPTW::Transition>::value) {
    if (seed)
      solution_positions = seed->allocated_positions();
    root_positions = root.allocated_positions();
    if (root.status() == SS_BRANCH) {
      Search::Statistics stat;
      // Record the first prepared prefix without committing a visit.
      (void) Search::prepare(root,stat,search);
    }
    prepared_positions = root.allocated_positions();
  }
  const auto guidance=[&]() {
    if constexpr (std::is_same<Model,TSPTW::Transition>::value)
      return Model::guidance(options.bound=="prefix" ? Model::PREFIX : Model::MIN_ARCS,
                             measurements);
    else return Model::guidance();
  }();
  Search::StatePruning pruning;
  if constexpr (std::is_same<Model,TSPTW::Transition>::value)
    if (options.state_pruning=="pareto") pruning=Model::pareto_pruning();
  const double preprocessing_ms=elapsed();
  const auto search_start=std::chrono::steady_clock::now();
  std::unique_ptr<Search::Engine> engine;
  if (options.engine=="bab") engine.reset(Search::babengine(&root,search));
  else if (options.engine=="rbs") {
    std::unique_ptr<Search::Builder> builder(rbs<Model,BAB>(search));
    engine.reset((*builder)(&root));
  } else if (options.engine=="astar")
    engine.reset(Search::astarengine(&root,guidance,search,pruning));
  else if (options.engine=="column")
    engine.reset(Search::columnengine(&root,guidance,search,options.column,pruning));
  else engine.reset(Search::cabsengine(&root,guidance,search,options.cabs,pruning));
  // Default clone ownership keeps root and seed independently owned here.
  // The stop object was declared first and outlives every engine.
  if (seed) engine->constrain(*seed);
  std::cout << std::fixed << std::setprecision(3);
  if (incumbent) {
    std::cout << "{\"type\":\"solution\",";
    write_tour(best_tour,*incumbent,data->scale);
    std::cout << ",\"elapsed_ms\":" << elapsed() << '}' << std::endl;
  }
  while (std::unique_ptr<Model> solution{
           static_cast<Model*>(engine->next())}) {
    auto tour=solution->tour();
    const TourValue value=evaluate_tour(*data,tour);
    if (value.cost!=solution->cost() || (incumbent && value.cost>=incumbent->cost))
      throw std::runtime_error("invalid or nonimproving model solution");
    incumbent=value;
    best_tour=std::move(tour);
    if constexpr (std::is_same<Model,TSPTW::Transition>::value)
      solution_positions=solution->allocated_positions();
    std::cout << "{\"type\":\"solution\",";
    write_tour(best_tour,value,data->scale);
    std::cout << ",\"elapsed_ms\":" << elapsed() << '}' << std::endl;
  }
  const double search_ms=std::chrono::duration<double,std::milli>(
    std::chrono::steady_clock::now()-search_start).count();
  const auto statistics=engine->statistics();
  const char* termination=engine->stopped() ? "stopped" :
                          incumbent ? "optimal" : "infeasible";
  std::cout << "{\"type\":\"summary\",\"engine\":\"" << options.engine
            << "\",\"formulation\":\"" << options.formulation
            << "\",\"state_pruning\":\"" << options.state_pruning
            << "\",\"bound\":\"" << (options.formulation=="closed" ? "cp-total" : options.bound)
            << "\",\"residual_propagation\":\"" << options.residual_propagation
            << "\",\"termination\":\"" << termination << '"';
  if (options.formulation=="closed")
    std::cout << ",\"circuit_propagation\":\"" << options.circuit_propagation
              << "\",\"cost_branching\":\"" << options.cost_branching << '"';
  if (options.engine=="rbs")
    std::cout << ",\"restart_cutoff\":\"luby\",\"restart_scale\":1000"
              << ",\"nogoods_limit\":128";
  if (options.engine=="column") {
    std::cout << ",\"initial_quota\":" << options.column.initial_quota
              << ",\"quota_growth\":" << options.column.quota_growth
              << ",\"frontier_limit\":" << options.column.frontier_limit;
  }
  std::cout << ",\"incumbent\":";
  if (incumbent) {
    std::cout << '{';
    write_tour(best_tour,*incumbent,data->scale);
    std::cout << '}';
  } else std::cout << "null";
  if constexpr (std::is_same<Model,TSPTW::Transition>::value) {
    std::cout << ",\"allocation\":{\"space_bytes\":" << sizeof(Model)
              << ",\"intvar_handle_bytes\":" << sizeof(IntVar)
              << ",\"root_positions\":" << root_positions
              << ",\"prepared_root_positions\":" << prepared_positions
              << ",\"solution_positions\":" << solution_positions << '}';
    std::cout << ",\"measurements\":{\"bound_calls\":" << measurements->bound_calls
              << ",\"bound_ms\":" << measurements->bound_ms
              << ",\"inference_calls\":" << measurements->inference_calls
              << ",\"inference_ms\":" << measurements->inference_ms
              << ",\"propagation_ms\":" << measurements->propagation_ms << '}';
  }
  std::cout << ",\"preprocessing_ms\":" << preprocessing_ms
            << ",\"search_ms\":" << search_ms
            << ",\"elapsed_ms\":" << elapsed() << ",\"statistics\":{\"node\":"
            << statistics.node << ",\"fail\":" << statistics.fail
            << ",\"depth\":" << statistics.depth << ",\"propagate\":"
            << statistics.propagate << ",\"restart\":" << statistics.restart
            << ",\"nogood\":" << statistics.nogood
            << ",\"state_pruned\":" << statistics.state_pruned
            << ",\"state_descriptors\":" << statistics.state_descriptors
            << ",\"state_bytes\":" << statistics.state_bytes << "}}\n";
  return 0;
}

static int run(const RunOptions& options) {
  const auto start=std::chrono::steady_clock::now();
  LimitsStop stop(options);
  auto data=std::make_shared<const TSPTW::Instance>(TSPTW::read(options.instance));
  Search::Options search;
  search.clone=true;
  search.threads=1;
  search.stop=&stop;
  if (options.engine=="rbs") {
    search.cutoff=Search::Cutoff::luby(1000);
    search.nogoods_limit=128;
  }
  if (options.formulation=="closed") {
    TSPTWClosed root(data,options.circuit_propagation=="dom" ? IPL_DOM : IPL_VAL,
                     options.cost_branching=="size");
    return run_search(options,root,search,start);
  }
  auto measurements=std::make_shared<TSPTW::Measurements>();
  TSPTW::Transition root(data,options.formulation=="transition" ?
                         TSPTW::Transition::LAZY : TSPTW::Transition::EAGER,
                         options.residual_propagation=="off" ?
                         TSPTW::Transition::OFF : options.residual_propagation=="cheap" ?
                         TSPTW::Transition::CHEAP : TSPTW::Transition::UNARY,
                         measurements);
  return run_search(options,root,search,start,measurements);
}

int main(int argc, char* argv[]) {
  try {
    return run(read_options(argc,argv));
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
