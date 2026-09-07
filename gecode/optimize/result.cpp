#include "result.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>

namespace Gecode { namespace Optimize {

const char* to_string(Termination termination) noexcept {
  switch (termination) {
  case Termination::Unknown: return "unknown";
  case Termination::Optimal: return "optimal";
  case Termination::Infeasible: return "infeasible";
  case Termination::Unbounded: return "unbounded";
  case Termination::InfeasibleOrUnbounded: return "infeasible_or_unbounded";
  case Termination::TimeLimit: return "time_limit";
  case Termination::NodeLimit: return "node_limit";
  case Termination::MemoryLimit: return "memory_limit";
  case Termination::IterationLimit: return "iteration_limit";
  case Termination::SolutionLimit: return "solution_limit";
  case Termination::ObjectiveLimit: return "objective_limit";
  case Termination::Cancelled: return "cancelled";
  case Termination::NumericalFailure: return "numerical_failure";
  case Termination::Unsupported: return "unsupported";
  case Termination::InvalidModel: return "invalid_model";
  case Termination::BackendError: return "backend_error";
  }
  return "unknown";
}

void CancellationToken::cancel() noexcept {
  cancelled_.store(true, std::memory_order_release);
}

bool CancellationToken::cancelled() const noexcept {
  return cancelled_.load(std::memory_order_acquire);
}

namespace {
void require_nonnegative_finite(double value, const char* name) {
  if (!std::isfinite(value) || value < 0.0)
    throw ModelError(std::string(name) + " must be nonnegative and finite");
}

double nonnegative_double(long double value) noexcept {
  if (value > static_cast<long double>(std::numeric_limits<double>::max()))
    return std::numeric_limits<double>::infinity();
  return static_cast<double>(value);
}
}

void SolveOptions::validate() const {
  switch (backend) {
  case Backend::Auto:
  case Backend::Highs:
  case Backend::Native:
    break;
  default:
    throw ModelError("invalid backend option");
  }
  switch (guarantee) {
  case Guarantee::Numerical:
  case Guarantee::Exact:
  case Guarantee::Certified:
    break;
  default:
    throw ModelError("invalid guarantee option");
  }
  if (std::isnan(time_limit_seconds) || time_limit_seconds < 0.0)
    throw ModelError("time limit must be nonnegative or positive infinity");
  if (threads < 1)
    throw ModelError("threads must be positive");
  if (random_seed < 0)
    throw ModelError("random seed must be nonnegative");
  require_nonnegative_finite(relative_gap, "relative gap");
  require_nonnegative_finite(absolute_gap, "absolute gap");
  if (!std::isfinite(feasibility_tolerance) || feasibility_tolerance <= 0.0)
    throw ModelError("feasibility tolerance must be positive and finite");
  if (!std::isfinite(integrality_tolerance) || integrality_tolerance <= 0.0 ||
      integrality_tolerance >= 0.5)
    throw ModelError("integrality tolerance must be finite and between 0 and 0.5");
  for (const auto& entry : primal_start)
    if (!std::isfinite(entry.value))
      throw ModelError("primal start values must be finite");
}

struct SolveBudget::State {
  using Clock = std::chrono::steady_clock;
  const Clock::time_point start;
  const double time_limit;
  const std::optional<std::uint64_t> node_limit;
  const std::shared_ptr<CancellationToken> cancellation;
  std::atomic<std::uint64_t> nodes{0};

  State(const SolveOptions& options, Clock::time_point started)
    : start(started), time_limit(options.time_limit_seconds), node_limit(options.node_limit),
      cancellation(options.cancellation ? options.cancellation
                                        : std::make_shared<CancellationToken>()) {}
};

SolveBudget::SolveBudget(const SolveOptions& options) {
  const auto started = State::Clock::now();
  options.validate();
  state_ = std::make_shared<State>(options, started);
}

SolveBudget SolveBudget::slice(double seconds, std::uint64_t node_allowance) const {
  if (std::isnan(seconds) || seconds < 0.0)
    throw ModelError("budget slice time must be nonnegative or positive infinity");
  auto result=*this;
  result.local_deadline_=std::min(local_deadline_,elapsed_seconds()+seconds);
  const auto used=nodes(), maximum=std::numeric_limits<std::uint64_t>::max();
  const auto limit=node_allowance>maximum-used ? maximum:used+node_allowance;
  result.local_node_limit_=local_node_limit_ ? std::min(*local_node_limit_,limit):limit;
  return result;
}

double SolveBudget::elapsed_seconds() const noexcept {
  return std::max(0.0, std::chrono::duration<double>(
    State::Clock::now() - state_->start).count());
}

double SolveBudget::remaining_seconds() const noexcept {
  return std::max(0.0, std::min(state_->time_limit,local_deadline_) - elapsed_seconds());
}

bool SolveBudget::cancelled() const noexcept {
  return state_->cancellation->cancelled();
}

bool SolveBudget::time_limit_reached() const noexcept {
  return elapsed_seconds() >= std::min(state_->time_limit,local_deadline_);
}

bool SolveBudget::node_limit_reached() const noexcept {
  const auto used=nodes();
  return (state_->node_limit && used >= *state_->node_limit) ||
    (local_node_limit_ && used >= *local_node_limit_);
}

bool SolveBudget::expired() const noexcept {
  return stop_reason().has_value();
}

std::optional<Termination> SolveBudget::stop_reason() const noexcept {
  if (cancelled()) return Termination::Cancelled;
  if (time_limit_reached()) return Termination::TimeLimit;
  if (node_limit_reached()) return Termination::NodeLimit;
  return std::nullopt;
}

void SolveBudget::add_nodes(std::uint64_t count) noexcept {
  auto old = state_->nodes.load(std::memory_order_relaxed);
  const auto maximum = std::numeric_limits<std::uint64_t>::max();
  for (;;) {
    const auto next = count > maximum - old ? maximum : old + count;
    if (state_->nodes.compare_exchange_weak(old, next, std::memory_order_relaxed))
      return;
  }
}

std::uint64_t SolveBudget::nodes() const noexcept {
  return state_->nodes.load(std::memory_order_relaxed);
}

std::shared_ptr<CancellationToken> SolveBudget::cancellation() const noexcept {
  return state_->cancellation;
}

bool SolveResult::has_solution() const noexcept {
  if (!solution_validated || !objective || !std::isfinite(*objective) ||
      values.size() != active_variables.size())
    return false;
  for (std::size_t i = 0; i < values.size(); ++i)
    if (active_variables[i] && !std::isfinite(values[i]))
      return false;
  return true;
}

double SolveResult::value(Variable variable) const {
  if (variable.model_id != model_id)
    throw ModelError("variable belongs to a different model than this result");
  if (variable.id >= active_variables.size() || !active_variables[variable.id])
    throw ModelError("variable is absent or deleted in this result snapshot");
  if (!has_solution())
    throw ModelError("result has no validated solution");
  return values[variable.id];
}

void SolveResult::update_gaps(ObjectiveSense sense) {
  absolute_gap.reset();
  relative_gap.reset();
  if (sense != ObjectiveSense::Minimize && sense != ObjectiveSense::Maximize)
    throw ModelError("invalid objective sense for gap computation");
  if (objective && !std::isfinite(*objective))
    throw ModelError("objective must be finite for gap computation");
  if (best_bound && std::isnan(*best_bound))
    throw ModelError("best bound must not be NaN");
  if (!objective || !best_bound)
    return;

  if ((sense == ObjectiveSense::Minimize && *best_bound > *objective) ||
      (sense == ObjectiveSense::Maximize && *best_bound < *objective))
    throw ModelError("global best bound is inconsistent with the objective");
  if (!std::isfinite(*best_bound))
    return;

  const long double primal = *objective;
  const long double dual = *best_bound;
  const long double gap = sense == ObjectiveSense::Minimize
    ? primal - dual : dual - primal;
  absolute_gap = nonnegative_double(gap);
  const long double scale = std::max({1.0L, std::fabs(primal), std::fabs(dual)});
  // Some platforms implement long double as double. Dividing before the
  // subtraction avoids an overflowing absolute gap hiding a finite ratio.
  relative_gap = nonnegative_double(std::isfinite(gap)
    ? gap / scale : std::fabs(primal / scale - dual / scale));
}

}}
