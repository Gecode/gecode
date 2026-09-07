#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/diagnostics.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

namespace O = Gecode::Optimize;
constexpr double inf = std::numeric_limits<double>::infinity();

#ifdef GECODE_DIAGNOSTICS_TEST_FAKE_SOLVER
namespace {
int calls = 0, interrupt_at = 0, corrupt_at = 0, contradict_at = 0, cancel_at = 0;
O::Termination interrupted_reason = O::Termination::TimeLimit;
std::vector<double> limits;
void reset() {
  calls = interrupt_at = corrupt_at = contradict_at = cancel_at = 0;
  interrupted_reason = O::Termination::TimeLimit;
  limits.clear();
}
}
namespace Gecode { namespace Optimize {
SolveResult solve(const ModelSnapshot& model, const SolveOptions& options) {
  ++calls;
  assert(model.objective.terms.empty() && model.objective.offset == 0);
  assert(options.primal_start.empty() && options.relative_gap == 0 && options.absolute_gap == 0);
  limits.push_back(options.time_limit_seconds);
  if (limits.size() > 1) assert(limits.back() <= limits[limits.size()-2]);
  SolveResult result;
  result.model_id = model.model_id; result.revision = model.revision;
  result.backend = "deterministic oracle"; result.backend_version = "test";
  for (const auto& variable : model.variables) result.active_variables.push_back(variable.active);
  result.termination = Termination::Infeasible;
  assert(model.variables.size() == 1);
  for (double value : {-10.0, -3.0, -2.0, -1.0, 0.0, 0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 10.0}) {
    std::vector<double> witness{value};
    if (!validate(model, witness).valid) continue;
    result.values = std::move(witness);
    result.solution_validated = true;
    result.objective = 0;
    result.termination = Termination::Optimal;
    break;
  }
  if (calls == corrupt_at || calls == contradict_at) {
    result.solution_validated = true; result.values = {3}; result.objective = 0;
    result.termination = calls == contradict_at ? Termination::Infeasible : Termination::Optimal;
  }
  if (calls == interrupt_at) result.termination = interrupted_reason;
  if (calls == cancel_at) options.cancellation->cancel();
  return result;
}
}}
#endif

namespace {
// Independent reconstruction from the attribution contract: begin with a free
// continuous system and restore only reported groups, rather than using the
// implementation's deletion operation.
O::ModelSnapshot rebuild(const O::ModelSnapshot& original,
                         const std::vector<O::ConflictGroup>& groups,
                         std::size_t omit = std::numeric_limits<std::size_t>::max()) {
  auto model = original;
  model.objective = {};
  for (auto& variable : model.variables) if (variable.active) {
    variable.type = O::VariableType::Continuous;
    variable.lower = -inf; variable.upper = inf;
  }
  for (auto& row : model.rows) if (row.active) { row.active = false; row.terms.clear(); }
  for (auto& indicator : model.indicators) indicator.active = false;
  for (std::size_t i = 0; i < groups.size(); ++i) if (i != omit) {
    const auto& group = groups[i];
    switch (group.kind) {
    case O::ConflictGroupKind::Row: model.rows[group.row->id] = original.rows[group.row->id]; break;
    case O::ConflictGroupKind::LowerBound:
      model.variables[group.variable->id].lower = original.variables[group.variable->id].lower; break;
    case O::ConflictGroupKind::UpperBound:
      model.variables[group.variable->id].upper = original.variables[group.variable->id].upper; break;
    case O::ConflictGroupKind::Integrality:
      model.variables[group.variable->id].type = O::VariableType::Integer; break;
    case O::ConflictGroupKind::VariableDomain:
      model.variables[group.variable->id] = original.variables[group.variable->id]; break;
    case O::ConflictGroupKind::IndicatorComponent:
      for (auto handle : group.grouped_variables) model.variables[handle.id] = original.variables[handle.id];
      for (auto handle : group.generated_rows) model.rows[handle.id] = original.rows[handle.id];
      for (auto handle : group.indicators) model.indicators[handle.id] = original.indicators[handle.id];
      break;
    }
  }
  return model;
}

O::ConflictOptions evidence_options() {
  O::ConflictOptions options;
  options.retain_deletion_witnesses = true;
  return options;
}

void verify(const O::ModelSnapshot& original, const O::ConflictResult& result) {
  assert(result.irreducible() && result.infeasibility_established);
  assert(result.termination == O::Termination::Infeasible && result.guarantee == O::Guarantee::Numerical);
  assert(result.model_id == original.model_id && result.revision == original.revision);
  const auto conflict = rebuild(original, result.groups);
  O::validate_structure(conflict);
#ifndef GECODE_DIAGNOSTICS_TEST_FAKE_SOLVER
  assert(O::solve(conflict).termination == O::Termination::Infeasible);
#endif
  for (std::size_t i = 0; i < result.groups.size(); ++i) {
    assert(result.groups[i].necessity_verified);
    auto deleted = rebuild(original, result.groups, i);
    O::validate_structure(deleted);
    assert(O::validate(deleted, result.groups[i].deletion_witness).valid);
  }
}

void preflight() {
  O::Model model;
  auto x = model.add_integer(0, 4);
  model.add_row({{x, 1}}, 5, inf);
  auto options = evidence_options();
  options.solve.time_limit_seconds = 0;
  auto result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Unknown && result.termination == O::Termination::TimeLimit);
  assert(!result.infeasibility_established && result.oracle_calls == 0 && result.groups.empty());
  options.solve.time_limit_seconds = inf;
  options.solve.cancellation = std::make_shared<O::CancellationToken>();
  options.solve.cancellation->cancel();
  result = O::analyze_conflict(model, options);
  assert(result.termination == O::Termination::Cancelled && result.oracle_calls == 0);
  options = {}; options.solve.node_limit = 2;
  assert(O::analyze_conflict(model, options).status == O::ConflictStatus::Unsupported);
  options = {}; options.solve.guarantee = O::Guarantee::Exact;
  assert(O::analyze_conflict(model, options).status == O::ConflictStatus::Unsupported);
  options.solve.guarantee = O::Guarantee::Certified;
  assert(O::analyze_conflict(model, options).status == O::ConflictStatus::Unsupported);
  options = {}; options.solve.feasibility_tolerance = -1;
  assert(O::analyze_conflict(model, options).status == O::ConflictStatus::InvalidModel);
  auto malformed = model.snapshot(); malformed.variables[x.id].variable.model_id = 0;
  assert(O::analyze_conflict(malformed).status == O::ConflictStatus::InvalidModel);
}

#ifndef GECODE_DIAGNOSTICS_TEST_FAKE_SOLVER
void actual_oracles() {
  const auto options = evidence_options();
  O::Model rows;
  auto x = rows.add_continuous(-inf, inf, "quantity");
  auto lower = rows.add_row({{x, 1}}, 2, inf, "demand");
  auto upper = rows.add_row({{x, 1}}, -inf, 1, "capacity");
  rows.add_row({{x, 1}}, -inf, 10, "redundant");
  rows.maximize({{x, 7}}, -8);
  const auto original = rows.snapshot();
  auto ignored_start = options; ignored_start.solve.primal_start = {{x, 0}};
  auto result = O::analyze_conflict(rows, ignored_start);
  verify(original, result);
  assert(result.groups.size() == 2 && result.groups[0].row->id == lower.id && result.groups[1].row->id == upper.id);
  assert(rows.revision() == original.revision && rows.row(lower).active && rows.row(upper).active);
  assert(rows.snapshot().objective.offset == -8 && rows.snapshot().objective.sense == O::ObjectiveSense::Maximize);
  auto without_witnesses = O::analyze_conflict(rows);
  assert(without_witnesses.irreducible());
  for (const auto& group : without_witnesses.groups) assert(group.deletion_witness.empty());

  O::Model bound;
  auto y = bound.add_continuous(0, 10, "stock");
  bound.add_row({{y, 1}}, 11, inf, "required");
  result = O::analyze_conflict(bound, options); verify(bound.snapshot(), result);
  assert(result.groups.size() == 2 && result.groups[0].kind == O::ConflictGroupKind::Row &&
         result.groups[1].kind == O::ConflictGroupKind::UpperBound && *result.groups[1].variable == y);

  O::Model integer;
  auto n = integer.add_integer(0.25, 0.75, "count");
  result = O::analyze_conflict(integer, options); verify(integer.snapshot(), result);
  assert(result.groups.size() == 3);
  assert(result.groups[0].kind == O::ConflictGroupKind::LowerBound &&
         result.groups[1].kind == O::ConflictGroupKind::UpperBound &&
         result.groups[2].kind == O::ConflictGroupKind::Integrality);
  assert(integer.variable(n).type == O::VariableType::Integer && integer.variable(n).lower == 0.25);

  O::Model binary;
  auto bit = binary.add_binary(); binary.set_bounds(bit, 0.25, 0.75);
  result = O::analyze_conflict(binary, options); verify(binary.snapshot(), result);
  assert(result.groups.size() == 1 && result.groups[0].kind == O::ConflictGroupKind::VariableDomain);

  for (auto type : {O::VariableType::SemiContinuous, O::VariableType::SemiInteger}) {
    O::Model semi;
    auto s = semi.add_variable(type, 3, 6);
    semi.add_row({{s, 1}}, type == O::VariableType::SemiContinuous ? 1 : 3.25,
                            type == O::VariableType::SemiContinuous ? 2 : 3.75);
    result = O::analyze_conflict(semi, options); verify(semi.snapshot(), result);
    assert(result.groups.size() == 2 && result.groups[1].kind == O::ConflictGroupKind::VariableDomain);
  }

  O::Model constant;
  constant.add_row({}, 1, inf, "constant contradiction");
  result = O::analyze_conflict(constant, options); verify(constant.snapshot(), result);
  assert(result.groups.size() == 1 && result.groups[0].kind == O::ConflictGroupKind::Row);
  O::Model empty;
  auto dead = empty.add_binary(); empty.remove(dead);
  result = O::analyze_conflict(empty, options);
  assert(result.status == O::ConflictStatus::Feasible && !result.infeasibility_established);
  assert(result.feasible_witness.size() == 1 && std::isnan(result.feasible_witness[dead.id]));

  O::Model unbounded;
  auto free = unbounded.add_continuous(-inf, inf);
  unbounded.minimize({{free, -1}});
  result = O::analyze_conflict(unbounded, options);
  assert(result.status == O::ConflictStatus::Feasible && result.oracle_calls == 1);
  assert(O::validate(unbounded.snapshot(), result.feasible_witness).valid);

  O::Model indicators;
  auto b = indicators.add_binary("enabled"); auto c = indicators.add_binary("other");
  auto amount = indicators.add_continuous(0, 5);
  indicators.set_bounds(b, 1, 1); indicators.set_bounds(c, 1, 1);
  auto first = O::add_indicator(indicators, b, true, {{amount, 1}}, 4, inf);
  auto second = O::add_indicator(indicators, c, true, {{amount, 1}}, -inf, 3);
  indicators.add_row({}, -inf, 1, "irrelevant");
  const auto indicator_snapshot = indicators.snapshot();
  result = O::analyze_conflict(indicators, options); verify(indicator_snapshot, result);
  assert(result.groups.size() == 1 && result.groups[0].kind == O::ConflictGroupKind::IndicatorComponent);
  assert(result.groups[0].indicators.size() == 2 && result.groups[0].grouped_variables.size() == 5);
  assert(indicators.revision() == indicator_snapshot.revision && indicators.row(first.rows.front()).active &&
         indicators.row(second.rows.front()).active);
  // Removed indicator tombstones must not impose a hidden nonrelaxable domain.
  O::remove_indicator(indicators, first.indicator); O::remove_indicator(indicators, second.indicator);
  result = O::analyze_conflict(indicators, options);
  assert(result.status == O::ConflictStatus::Feasible);

  O::Model chained;
  auto enable = chained.add_binary(); chained.set_bounds(enable, 0, 0);
  auto first_amount = chained.add_continuous(0, 4);
  auto second_amount = chained.add_continuous(0, 2);
  auto gate_owner = O::add_indicator(chained, enable, true, {{first_amount, 1}}, 2, inf);
  O::add_indicator(chained, *gate_owner.inactive_gate, true, {{second_amount, 1}}, 3, inf);
  // An independent, harmless component must disappear from the conflict.
  auto unrelated = chained.add_binary();
  O::add_indicator(chained, unrelated, true, {}, -inf, inf);
  result = O::analyze_conflict(chained, options); verify(chained.snapshot(), result);
  assert(result.groups.size() == 1 && result.groups[0].indicators.size() == 2);
  assert(result.groups[0].grouped_variables.size() == 5);
}
#else
void coordinator() {
  O::Model model;
  auto x = model.add_continuous(-10, 10);
  model.add_row({{x, 1}}, 2, inf); model.add_row({{x, 1}}, -inf, 1);
  auto options = evidence_options(); options.solve.time_limit_seconds = 100;
  options.solve.primal_start = {{x, 0}};
  reset();
  auto result = O::analyze_conflict(model, options); verify(model.snapshot(), result);
  assert(result.oracle_calls == 5 && result.groups.size() == 2);
  reset(); interrupt_at = 2;
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Incomplete && result.infeasibility_established && !result.irreducible());
  assert(result.termination == O::Termination::TimeLimit && result.groups.size() == 4 && calls == 2);
  reset(); interrupt_at = 5;
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Incomplete && result.groups.size() == 3 && calls == 5);
  assert(result.groups.back().kind == O::ConflictGroupKind::UpperBound);
  assert(result.groups[0].necessity_verified && result.groups[1].necessity_verified);
  reset(); interrupt_at = 1; interrupted_reason = O::Termination::InfeasibleOrUnbounded;
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Unknown && !result.infeasibility_established && result.groups.empty());
  reset(); corrupt_at = 2;
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Incomplete && result.termination == O::Termination::NumericalFailure);
  assert(result.groups.size() == 4);
  reset(); contradict_at = 1;
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Error && result.termination == O::Termination::NumericalFailure);
  assert(!result.infeasibility_established && result.groups.empty());
  reset(); cancel_at = 2;
  options.solve.cancellation = std::make_shared<O::CancellationToken>();
  result = O::analyze_conflict(model, options);
  assert(result.status == O::ConflictStatus::Incomplete && result.termination == O::Termination::Cancelled);
  assert(result.groups.size() == 4 && calls == 2);
}
#endif
}

int main() {
  preflight();
#ifdef GECODE_DIAGNOSTICS_TEST_FAKE_SOLVER
  coordinator();
#else
  if (!O::capabilities().available) {
    O::Model model; model.add_integer(0.25, 0.75);
    auto result = O::analyze_conflict(model);
    assert(result.status == O::ConflictStatus::Unsupported && !result.infeasibility_established);
    std::cout << "Unavailable diagnostic backend contract passed\n";
    return 0;
  }
  actual_oracles();
#endif
  std::cout << "Numerical conflict groups, irreducibility witnesses and budgets passed\n";
}
