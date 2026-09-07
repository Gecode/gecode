/* Sparse numerical model implementation. */
#include <gecode/optimize/model.hpp>
#include <gecode/optimize/globals.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {

ModelId allocate_model_id() {
  static std::atomic<ModelId> next{1};
  ModelId candidate = next.load(std::memory_order_relaxed);
  for (;;) {
    if (candidate == std::numeric_limits<ModelId>::max())
      throw ModelError("Model identity space exhausted");
    if (next.compare_exchange_weak(candidate, candidate + 1,
                                   std::memory_order_relaxed))
      return candidate;
  }
}

void check_bounds(double lower, double upper) {
  if (std::isnan(lower) || std::isnan(upper) ||
      lower == std::numeric_limits<double>::infinity() ||
      upper == -std::numeric_limits<double>::infinity() || lower > upper)
    throw ModelError("Invalid lower/upper bounds");
}

void check_variable_bounds(VariableType type, double lower, double upper) {
  check_bounds(lower, upper);
  switch (type) {
  case VariableType::Continuous:
  case VariableType::Integer:
    break;
  case VariableType::Binary:
    if (lower < 0.0 || upper > 1.0)
      throw ModelError("Binary bounds must be within [0,1]");
    break;
  case VariableType::SemiContinuous:
  case VariableType::SemiInteger:
    if (!std::isfinite(lower) || lower <= 0.0)
      throw ModelError("Semi-variable nonzero interval must have a positive finite lower bound");
    break;
  default:
    throw ModelError("Unknown variable type");
  }
}

void check_finite(double value, const char* message) {
  if (!std::isfinite(value))
    throw ModelError(message);
}

void check_sense(ObjectiveSense sense) {
  if (sense != ObjectiveSense::Minimize && sense != ObjectiveSense::Maximize)
    throw ModelError("Unknown objective sense");
}

bool references(const std::vector<Term>& terms, Variable variable) {
  return std::any_of(terms.begin(), terms.end(), [variable](const Term& term) {
    return term.variable == variable;
  });
}

std::vector<Term> replaced(const std::vector<Term>& terms, Variable variable,
                           double coefficient) {
  std::vector<Term> result;
  result.reserve(terms.size() + 1);
  for (const Term& term : terms)
    if (term.variable != variable)
      result.push_back(term);
  if (coefficient != 0.0)
    result.push_back({variable, coefficient});
  return result;
}

} // namespace

Model::Model() : model_id_(allocate_model_id()) {}

Model::Model(Model&& other) noexcept
    : model_id_(std::exchange(other.model_id_, 0)),
      revision_(std::exchange(other.revision_, 0)),
      variables_(std::move(other.variables_)), rows_(std::move(other.rows_)),
      objective_(std::move(other.objective_)), indicators_(std::move(other.indicators_)),
      globals_(std::move(other.globals_)) {}

Model& Model::operator=(Model&& other) noexcept {
  if (this != &other) {
    model_id_ = std::exchange(other.model_id_, 0);
    revision_ = std::exchange(other.revision_, 0);
    variables_ = std::move(other.variables_);
    rows_ = std::move(other.rows_);
    objective_ = std::move(other.objective_);
    indicators_ = std::move(other.indicators_);
    globals_ = std::move(other.globals_);
  }
  return *this;
}

void Model::require_live() const {
  if (model_id_ == 0)
    throw ModelError("Cannot use a moved-from model");
}

void Model::require_revision_capacity() const {
  require_live();
  if (revision_ == std::numeric_limits<Revision>::max())
    throw ModelError("Model revision space exhausted");
}

const VariableData& Model::variable(Variable handle) const {
  require_live();
  if (handle.model_id != model_id_ || handle.id >= variables_.size() ||
      !variables_[static_cast<std::size_t>(handle.id)].active)
    throw ModelError("Variable handle is foreign, invalid, or deleted");
  return variables_[static_cast<std::size_t>(handle.id)];
}

const RowData& Model::row(Constraint handle) const {
  require_live();
  if (handle.model_id != model_id_ || handle.id >= rows_.size() ||
      !rows_[static_cast<std::size_t>(handle.id)].active)
    throw ModelError("Constraint handle is foreign, invalid, or deleted");
  return rows_[static_cast<std::size_t>(handle.id)];
}

std::vector<Term> Model::normalize(const std::vector<Term>& terms,
                                  const std::vector<VariableData>* additions) const {
  require_live();
  for (const Term& term : terms) {
    if (term.variable.id < variables_.size() || !additions) {
      (void) variable(term.variable);
    } else if (term.variable.model_id != model_id_ ||
               term.variable.id - variables_.size() >= additions->size() ||
               !(*additions)[static_cast<std::size_t>(term.variable.id - variables_.size())].active) {
      throw ModelError("Variable handle is foreign, invalid, or deleted");
    }
    check_finite(term.coefficient, "Linear coefficients must be finite");
  }
  std::vector<Term> ordered = terms;
  // Sorting duplicate coefficients as well as IDs gives a fixed summation order
  // independent of insertion order. This is numerical, not exact arithmetic.
  std::sort(ordered.begin(), ordered.end(), [](const Term& a, const Term& b) {
    return a.variable.id != b.variable.id ? a.variable.id < b.variable.id :
      a.coefficient < b.coefficient;
  });
  std::vector<Term> result;
  result.reserve(ordered.size());
  for (std::size_t i = 0; i < ordered.size();) {
    const Variable handle = ordered[i].variable;
    long double sum = 0.0L, correction = 0.0L;
    do {
      const long double value = static_cast<long double>(ordered[i].coefficient);
      const long double next = sum + value;
      if (!std::isfinite(next))
        throw ModelError("Linear coefficient coalescing overflow");
      // Neumaier compensation also helps targets where long double is double.
      correction += std::abs(sum) >= std::abs(value) ?
        (sum - next) + value : (value - next) + sum;
      sum = next;
      if (!std::isfinite(correction))
        throw ModelError("Linear coefficient coalescing overflow");
      ++i;
    } while (i < ordered.size() && ordered[i].variable == handle);
    sum += correction;
    if (!std::isfinite(sum))
      throw ModelError("Linear coefficient coalescing overflow");
    if (sum < -static_cast<long double>(std::numeric_limits<double>::max()) ||
        sum > static_cast<long double>(std::numeric_limits<double>::max()))
      throw ModelError("Coalesced coefficient exceeds the numerical range");
    const double coefficient = static_cast<double>(sum);
    if (coefficient != 0.0)
      result.push_back({handle, coefficient});
  }
  return result;
}

ModelSnapshot Model::snapshot() const {
  require_live();
  return {model_id_, revision_, variables_, rows_, objective_, indicators_, globals_};
}

Variable Model::add_variable(VariableType type, double lower, double upper,
                              std::string name) {
  require_revision_capacity();
  check_variable_bounds(type, lower, upper);
  if (variables_.size() >= std::numeric_limits<std::uint64_t>::max())
    throw ModelError("Variable slot space exhausted");
  Variable handle{model_id_, static_cast<std::uint64_t>(variables_.size())};
  variables_.push_back({handle, type, lower, upper, std::move(name), true, std::nullopt});
  ++revision_;
  return handle;
}

Variable Model::add_continuous(double lower, double upper, std::string name) {
  return add_variable(VariableType::Continuous, lower, upper, std::move(name));
}

Variable Model::add_integer(double lower, double upper, std::string name) {
  return add_variable(VariableType::Integer, lower, upper, std::move(name));
}

Variable Model::add_binary(std::string name) {
  return add_variable(VariableType::Binary, 0.0, 1.0, std::move(name));
}

Constraint Model::add_row(const std::vector<Term>& terms, double lower,
                           double upper, std::string name) {
  require_revision_capacity();
  check_bounds(lower, upper);
  auto normalized = normalize(terms);
  if (rows_.size() >= std::numeric_limits<std::uint64_t>::max())
    throw ModelError("Constraint slot space exhausted");
  Constraint handle{model_id_, static_cast<std::uint64_t>(rows_.size())};
  rows_.push_back({handle, std::move(normalized), lower, upper, std::move(name), true, std::nullopt});
  ++revision_;
  return handle;
}

namespace {
template<class Data>
void check_bulk_capacity(const std::vector<Data>& target, std::size_t additions) {
  if (additions > target.max_size() - target.size() ||
      additions > std::numeric_limits<std::uint64_t>::max() - target.size())
    throw ModelError("Bulk addition exceeds the model slot capacity");
}
template<class Data>
void append_prepared(std::vector<Data>& target, std::vector<Data>& prepared) {
  static_assert(std::is_nothrow_move_constructible<Data>::value,
                "Atomic bulk commit requires noexcept entity moves");
  // A failed reserve preserves both the old values and their addresses. All
  // subsequent moves are nonthrowing and fit within the reserved allocation.
  const auto required = target.size() + prepared.size();
  if (required > target.capacity()) {
    const auto growth = std::min(target.capacity(), target.max_size() - target.capacity());
    target.reserve(std::max(required, target.capacity() + growth));
  }
  for (auto& item : prepared) target.push_back(std::move(item));
}
}

std::vector<Variable> Model::add_variables(const std::vector<VariableSpec>& input) {
  require_live();
  if (input.empty()) return {};
  require_revision_capacity();
  check_bulk_capacity(variables_, input.size());
  std::vector<VariableData> prepared;
  std::vector<Variable> handles;
  prepared.reserve(input.size()); handles.reserve(input.size());
  for (std::size_t i = 0; i < input.size(); ++i) {
    const auto& item = input[i];
    check_variable_bounds(item.type, item.lower, item.upper);
    const Variable handle{model_id_, static_cast<std::uint64_t>(variables_.size() + i)};
    prepared.push_back({handle, item.type, item.lower, item.upper, item.name, true, std::nullopt});
    handles.push_back(handle);
  }
  append_prepared(variables_, prepared);
  ++revision_;
  return handles;
}

std::vector<Constraint> Model::add_rows(const std::vector<RowSpec>& input) {
  require_live();
  if (input.empty()) return {};
  require_revision_capacity();
  check_bulk_capacity(rows_, input.size());
  std::vector<RowData> prepared;
  std::vector<Constraint> handles;
  prepared.reserve(input.size()); handles.reserve(input.size());
  for (std::size_t i = 0; i < input.size(); ++i) {
    const auto& item = input[i];
    check_bounds(item.lower, item.upper);
    const Constraint handle{model_id_, static_cast<std::uint64_t>(rows_.size() + i)};
    prepared.push_back({handle, normalize(item.terms), item.lower, item.upper, item.name, true, std::nullopt});
    handles.push_back(handle);
  }
  append_prepared(rows_, prepared);
  ++revision_;
  return handles;
}

std::vector<Constraint> Model::add_rows_sparse(const SparseRowBatch& input) {
  require_live();
  const auto count = input.lower.size(), nonzeros = input.coefficient.size();
  if (input.row_start.empty() || input.row_start.size() - 1 != count ||
      input.upper.size() != count || input.column.size() != nonzeros ||
      (!input.names.empty() && input.names.size() != count) ||
      input.row_start.front() != 0 || input.row_start.back() != nonzeros)
    throw ModelError("Invalid sparse row-batch dimensions or endpoints");
  for (std::size_t i = 0; i < count; ++i)
    if (input.row_start[i] > input.row_start[i + 1] || input.row_start[i + 1] > nonzeros)
      throw ModelError("Sparse row offsets must be monotone and within nonzeros");
  // Reject stale/foreign/duplicate column handles even when a column is unused.
  std::vector<std::uint64_t> seen;
  seen.reserve(input.columns.size());
  for (const auto handle : input.columns) { (void) variable(handle); seen.push_back(handle.id); }
  std::sort(seen.begin(), seen.end());
  if (std::adjacent_find(seen.begin(), seen.end()) != seen.end())
    throw ModelError("Sparse row-batch column mapping contains duplicate variables");
  for (const auto column : input.column)
    if (column >= input.columns.size()) throw ModelError("Sparse row-batch column index is out of range");
  if (!count) return {};
  require_revision_capacity();
  check_bulk_capacity(rows_, count);
  std::vector<RowData> prepared;
  std::vector<Constraint> handles;
  prepared.reserve(count); handles.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    check_bounds(input.lower[i], input.upper[i]);
    std::vector<Term> terms;
    terms.reserve(input.row_start[i + 1] - input.row_start[i]);
    for (auto k = input.row_start[i]; k < input.row_start[i + 1]; ++k)
      terms.push_back({input.columns[input.column[k]], input.coefficient[k]});
    const Constraint handle{model_id_, static_cast<std::uint64_t>(rows_.size() + i)};
    prepared.push_back({handle, normalize(terms), input.lower[i], input.upper[i],
                       input.names.empty() ? std::string{} : input.names[i], true, std::nullopt});
    handles.push_back(handle);
  }
  append_prepared(rows_, prepared);
  ++revision_;
  return handles;
}

void Model::set_objective(const std::vector<Term>& terms, ObjectiveSense sense,
                           double offset) {
  require_revision_capacity();
  check_sense(sense);
  check_finite(offset, "Objective offset must be finite");
  auto normalized = normalize(terms);
  objective_.terms.swap(normalized);
  objective_.offset = offset;
  objective_.sense = sense;
  ++revision_;
}

void Model::minimize(const std::vector<Term>& terms, double offset) {
  set_objective(terms, ObjectiveSense::Minimize, offset);
}

void Model::maximize(const std::vector<Term>& terms, double offset) {
  set_objective(terms, ObjectiveSense::Maximize, offset);
}

void Model::set_bounds(Variable handle, double lower, double upper) {
  const VariableType type = variable(handle).type;
  require_revision_capacity();
  check_variable_bounds(type, lower, upper);
  for (const auto& indicator : indicators_) {
    if (!indicator.active) continue;
    if (indicator.inactive_gate && *indicator.inactive_gate == handle &&
        (lower != 0.0 || upper != 1.0))
      throw ModelError("Cannot change an active indicator gate's bounds; remove_indicator first");
    for (const auto& domain : indicator.domains) {
      if (domain.variable != handle) continue;
      const double effective_lower = type == VariableType::SemiContinuous ||
        type == VariableType::SemiInteger ? 0.0 : lower;
      if (effective_lower < domain.lower || upper > domain.upper)
        throw ModelError("Bound widening invalidates an indicator formulation; remove and rebuild it first");
    }
  }
  auto& data = variables_[static_cast<std::size_t>(handle.id)];
  data.lower = lower;
  data.upper = upper;
  ++revision_;
}

void Model::set_bounds(Constraint handle, double lower, double upper) {
  (void) row(handle);
  require_unprotected(handle);
  require_revision_capacity();
  check_bounds(lower, upper);
  auto& data = rows_[static_cast<std::size_t>(handle.id)];
  data.lower = lower;
  data.upper = upper;
  ++revision_;
}

void Model::set_coefficient(Constraint constraint, Variable handle,
                             double coefficient) {
  (void) variable(handle);
  const auto& data = row(constraint);
  require_unprotected(constraint);
  require_revision_capacity();
  check_finite(coefficient, "Linear coefficients must be finite");
  auto normalized = normalize(replaced(data.terms, handle, coefficient));
  rows_[static_cast<std::size_t>(constraint.id)].terms.swap(normalized);
  ++revision_;
}

void Model::set_objective_coefficient(Variable handle, double coefficient) {
  (void) variable(handle);
  require_revision_capacity();
  check_finite(coefficient, "Objective coefficients must be finite");
  auto normalized = normalize(replaced(objective_.terms, handle, coefficient));
  objective_.terms.swap(normalized);
  ++revision_;
}

void Model::set_objective_offset(double offset) {
  require_revision_capacity();
  check_finite(offset, "Objective offset must be finite");
  objective_.offset = offset;
  ++revision_;
}

void Model::set_name(Variable handle, std::string name) {
  (void) variable(handle);
  require_revision_capacity();
  variables_[static_cast<std::size_t>(handle.id)].name.swap(name);
  ++revision_;
}

void Model::set_name(Constraint handle, std::string name) {
  (void) row(handle);
  require_revision_capacity();
  rows_[static_cast<std::size_t>(handle.id)].name.swap(name);
  ++revision_;
}

void Model::remove(Variable handle) {
  (void) variable(handle);
  require_revision_capacity();
  for (const auto& record : globals_) if (record.active)
    for (const auto variable : Detail::global_variables(record.payload))
      if (variable == handle) throw ModelError("Cannot remove a variable referenced by an active global constraint");
  for (const auto& indicator : indicators_) {
    if (!indicator.active) continue;
    if (indicator.activator == handle ||
        (indicator.inactive_gate && *indicator.inactive_gate == handle) ||
        references(indicator.terms, handle))
      throw ModelError("Cannot remove an active indicator variable; remove_indicator first");
  }
  if (references(objective_.terms, handle))
    throw ModelError("Cannot remove a variable referenced by the objective");
  for (const RowData& data : rows_)
    if (data.active && references(data.terms, handle))
      throw ModelError("Cannot remove a variable referenced by an active constraint");
  variables_[static_cast<std::size_t>(handle.id)].active = false;
  ++revision_;
}

void Model::remove(Constraint handle) {
  (void) row(handle);
  require_unprotected(handle);
  require_revision_capacity();
  auto& data = rows_[static_cast<std::size_t>(handle.id)];
  data.active = false;
  data.terms.clear();
  ++revision_;
}

void Model::require_unprotected(Constraint handle) const {
  for (const auto& indicator : indicators_) {
    if (!indicator.active) continue;
    for (const auto& generated : indicator.generated_rows)
      if (generated.model_id == handle.model_id && generated.id == handle.id)
        throw ModelError("Cannot edit an indicator's generated row; remove_indicator first");
  }
}

GlobalConstraint Model::add_global(GlobalPayload payload, std::string name) {
  require_revision_capacity();
  Detail::validate_global_payload(payload, model_id_, variables_, true);
  if (globals_.size() >= std::numeric_limits<std::uint64_t>::max())
    throw ModelError("Global constraint slot space exhausted");
  GlobalConstraint handle{model_id_, static_cast<std::uint64_t>(globals_.size())};
  globals_.push_back({handle, std::move(payload), std::move(name), true});
  ++revision_;
  return handle;
}
const GlobalData& Model::global(GlobalConstraint handle) const {
  require_live();
  if (handle.model_id != model_id_ || handle.id >= globals_.size() || !globals_[handle.id].active)
    throw ModelError("Global constraint handle is foreign, invalid, or deleted");
  return globals_[handle.id];
}
void Model::remove(GlobalConstraint handle) {
  (void) global(handle); require_revision_capacity();
  globals_[handle.id].active = false;
  ++revision_;
}
void Model::set_name(GlobalConstraint handle, std::string name) {
  (void) global(handle); require_revision_capacity();
  globals_[handle.id].name.swap(name);
  ++revision_;
}

}}
