#include <gecode/optimize/result.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <string>
#include <thread>
#include <vector>

using namespace Gecode::Optimize;

namespace {
template<class F> void rejects(F action) {
  bool rejected = false;
  try { action(); } catch (const ModelError&) { rejected = true; }
  assert(rejected);
}

void option_tests() {
  SolveOptions options;
  options.validate();
  assert(options.threads == 1 && options.random_seed == 0);
  assert(options.guarantee == Guarantee::Numerical);
  const double inf = std::numeric_limits<double>::infinity();
  const double nan = std::numeric_limits<double>::quiet_NaN();

  for (double invalid : {-1.0, -inf, nan}) {
    auto changed = options;
    changed.time_limit_seconds = invalid;
    rejects([&] { changed.validate(); });
    rejects([&] { SolveBudget budget(changed); });
  }
  for (double valid : {0.0, -0.0, 1.0, std::numeric_limits<double>::max(), inf}) {
    auto changed = options;
    changed.time_limit_seconds = valid;
    changed.validate();
  }
  for (int invalid : {0, -1}) {
    auto changed = options;
    changed.threads = invalid;
    rejects([&] { changed.validate(); });
  }
  auto changed = options;
  changed.random_seed = -1;
  rejects([&] { changed.validate(); });
  changed = options;
  changed.backend = static_cast<Backend>(999);
  rejects([&] { changed.validate(); });
  changed = options;
  changed.guarantee = static_cast<Guarantee>(999);
  rejects([&] { changed.validate(); });
  for (Guarantee policy : {Guarantee::Exact, Guarantee::Certified}) {
    changed = options;
    changed.guarantee = policy;
    changed.validate(); // unsupported policy is a backend result, not bad input
  }
  for (double invalid : {-1.0, inf, nan}) {
    changed = options;
    changed.relative_gap = invalid;
    rejects([&] { changed.validate(); });
    changed = options;
    changed.absolute_gap = invalid;
    rejects([&] { changed.validate(); });
  }
  for (double invalid : {0.0, -1.0, inf, nan}) {
    changed = options;
    changed.feasibility_tolerance = invalid;
    rejects([&] { changed.validate(); });
    changed = options;
    changed.integrality_tolerance = invalid;
    rejects([&] { changed.validate(); });
  }
  changed = options;
  changed.integrality_tolerance = 0.5;
  rejects([&] { changed.validate(); });
  changed = options;
  changed.relative_gap = 0.0;
  changed.absolute_gap = 0.0;
  changed.node_limit = 0;
  changed.validate();
}

void budget_tests() {
  SolveOptions options;
  SolveBudget unlimited(options);
  assert(!unlimited.expired() && !unlimited.stop_reason());
  assert(std::isinf(unlimited.remaining_seconds()));
  assert(!unlimited.node_limit_reached());
  unlimited.add_nodes(std::numeric_limits<std::uint64_t>::max());
  unlimited.add_nodes();
  assert(unlimited.nodes() == std::numeric_limits<std::uint64_t>::max());
  assert(!unlimited.node_limit_reached()); // no implicit limit at UINT64_MAX

  SolveBudget parent(options);
  auto child=parent.slice(3600,2);
  auto nested=child.slice(7200,100);
  assert(nested.remaining_seconds()<=3600);
  nested.add_nodes(2);
  assert(child.node_limit_reached() && nested.node_limit_reached());
  assert(parent.nodes()==2 && !parent.expired());
  auto next=parent.slice(3600,1);
  next.add_nodes();
  assert(next.node_limit_reached() && parent.nodes()==3 && !parent.expired());
  auto zero=parent.slice(0,10);
  assert(zero.stop_reason()==Termination::TimeLimit && !parent.expired());
  rejects([&]{parent.slice(-1,1);});
  rejects([&]{parent.slice(std::numeric_limits<double>::quiet_NaN(),1);});
  parent.cancellation()->cancel();
  assert(next.cancelled() && child.stop_reason()==Termination::Cancelled);

  options.time_limit_seconds = 0.0;
  SolveBudget immediate(options);
  assert(immediate.remaining_seconds() == 0.0);
  assert(immediate.stop_reason() == Termination::TimeLimit);
  options.time_limit_seconds = std::numeric_limits<double>::infinity();
  options.node_limit = 0;
  SolveBudget no_nodes(options);
  assert(no_nodes.stop_reason() == Termination::NodeLimit);

  options.node_limit = 2;
  SolveBudget nodes(options);
  auto shared = nodes;
  nodes.add_nodes();
  assert(!shared.expired());
  shared.add_nodes();
  assert(nodes.nodes() == 2 && nodes.stop_reason() == Termination::NodeLimit);
  shared.cancellation()->cancel();
  assert(nodes.cancelled());
  assert(nodes.stop_reason() == Termination::Cancelled);

  options.node_limit.reset();
  options.time_limit_seconds = 3600.0;
  options.cancellation = std::make_shared<CancellationToken>();
  SolveBudget clock(options);
  auto same_clock = clock;
  const auto elapsed = clock.elapsed_seconds();
  const auto remaining = clock.remaining_seconds();
  assert(elapsed >= 0.0 && remaining >= 0.0 && remaining <= 3600.0);
  assert(same_clock.elapsed_seconds() >= elapsed);
  assert(same_clock.remaining_seconds() <= remaining);
  options.time_limit_seconds = 0.0; // options are copied, not retained by reference
  assert(!clock.time_limit_reached());
  options.cancellation->cancel();
  assert(clock.stop_reason() == Termination::Cancelled);
  SolveBudget pre_cancelled(options);
  assert(pre_cancelled.stop_reason() == Termination::Cancelled);

  SolveOptions parallel_options;
  SolveBudget parallel(parallel_options);
  std::vector<std::thread> workers;
  for (unsigned i = 0; i < 4; ++i)
    workers.emplace_back([parallel]() mutable {
      for (unsigned j = 0; j < 1000; ++j) parallel.add_nodes();
      parallel.cancellation()->cancel();
    });
  for (auto& worker : workers) worker.join();
  assert(parallel.nodes() == 4000 && parallel.cancelled());
}

void result_tests() {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  SolveResult result;
  assert(!result.has_solution());
  assert(result.guarantee == Guarantee::Numerical);
  result.model_id = 42;
  result.revision = 7;
  result.objective = 3.0;
  result.values = {1.0, nan, 2.0};
  result.active_variables = {true, false, true};
  assert(!result.has_solution());
  result.solution_validated = true;
  result.termination = Termination::TimeLimit;
  assert(result.has_solution());
  assert(result.value({42, 0}) == 1.0 && result.value({42, 2}) == 2.0);
  rejects([&] { result.value({43, 0}); });
  rejects([&] { result.value({42, 1}); });
  rejects([&] { result.value({42, 3}); });
  rejects([&] { result.value({42, std::numeric_limits<std::uint64_t>::max()}); });

  const auto historical = result;
  result.revision = 8;
  result.values[0] = 9.0;
  result.active_variables[0] = false;
  assert(historical.revision == 7 && historical.value({42, 0}) == 1.0);
  result.active_variables[1] = true;
  assert(!result.has_solution());
  result = historical;
  result.values.pop_back();
  assert(!result.has_solution());
  result = historical;
  result.objective = nan;
  assert(!result.has_solution());
  result = historical;
  result.solution_validated = false;
  rejects([&] { result.value({42, 0}); });

  SolveResult empty;
  empty.model_id = 9;
  empty.objective = 0.0;
  empty.solution_validated = true;
  assert(empty.has_solution()); // a zero-variable feasible solution is present
  rejects([&] { empty.value({9, 0}); });
  assert(std::string(to_string(Termination::Unsupported)) == "unsupported");
  assert(std::string(to_string(Termination::MemoryLimit)) == "memory_limit");
  assert(std::string(to_string(Termination::InfeasibleOrUnbounded)) ==
         "infeasible_or_unbounded");
}

void gap_tests() {
  SolveResult result;
  result.objective = -10.0;
  result.best_bound = -12.0;
  result.native_backend_gap = 0.125;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(result.absolute_gap == 2.0);
  assert(std::fabs(*result.relative_gap - 1.0/6.0) < 1e-15);
  assert(result.native_backend_gap == 0.125);
  result.objective = -12.0;
  result.best_bound = -10.0;
  result.update_gaps(ObjectiveSense::Maximize);
  assert(result.absolute_gap == 2.0);
  assert(std::fabs(*result.relative_gap - 1.0/6.0) < 1e-15);
  result.objective = 1.0;
  result.best_bound = -1.0;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(result.absolute_gap == 2.0 && result.relative_gap == 2.0);

  result.objective = 0.0;
  result.best_bound = -1.0;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(result.absolute_gap == 1.0 && result.relative_gap == 1.0);
  result.best_bound = 1.0;
  result.update_gaps(ObjectiveSense::Maximize);
  assert(result.absolute_gap == 1.0 && result.relative_gap == 1.0);
  result.best_bound = -0.0;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(result.absolute_gap == 0.0 && result.relative_gap == 0.0);
  result.objective = 0.25;
  result.best_bound = -0.25;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(result.absolute_gap == 0.5 && result.relative_gap == 0.5);
  result.objective.reset();
  result.update_gaps(ObjectiveSense::Minimize);
  assert(!result.absolute_gap && !result.relative_gap);
  result.objective = 2.0;
  result.best_bound.reset();
  result.update_gaps(ObjectiveSense::Minimize);
  assert(!result.absolute_gap && !result.relative_gap);

  result.best_bound = 3.0;
  rejects([&] { result.update_gaps(ObjectiveSense::Minimize); });
  assert(!result.absolute_gap && !result.relative_gap);
  result.best_bound = std::nextafter(2.0, 3.0);
  rejects([&] { result.update_gaps(ObjectiveSense::Minimize); });
  result.best_bound = 1.0;
  rejects([&] { result.update_gaps(ObjectiveSense::Maximize); });
  rejects([&] { result.update_gaps(static_cast<ObjectiveSense>(999)); });

  const auto inf = std::numeric_limits<double>::infinity();
  result.best_bound = -inf;
  result.update_gaps(ObjectiveSense::Minimize);
  assert(!result.absolute_gap && !result.relative_gap);
  rejects([&] { result.update_gaps(ObjectiveSense::Maximize); });
  result.best_bound = inf;
  result.update_gaps(ObjectiveSense::Maximize);
  assert(!result.absolute_gap && !result.relative_gap);
  rejects([&] { result.update_gaps(ObjectiveSense::Minimize); });
  result.best_bound = std::numeric_limits<double>::quiet_NaN();
  rejects([&] { result.update_gaps(ObjectiveSense::Minimize); });
  result.best_bound = 0.0;
  result.objective = inf;
  rejects([&] { result.update_gaps(ObjectiveSense::Minimize); });

  result.objective = std::numeric_limits<double>::max();
  result.best_bound = -std::numeric_limits<double>::max();
  result.update_gaps(ObjectiveSense::Minimize);
  assert(std::isinf(*result.absolute_gap));
  assert(result.relative_gap == 2.0);
}
}

int main() {
  option_tests();
  budget_tests();
  result_tests();
  gap_tests();
}
