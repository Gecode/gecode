#include <gecode/optimize/constraints.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <type_traits>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
constexpr double infinity = std::numeric_limits<double>::infinity();

void bounds(double lower, double upper) {
  if (std::isnan(lower) || std::isnan(upper) || lower == infinity ||
      upper == -infinity || lower > upper)
    throw ModelError("Invalid indicator lower/upper bounds");
}

double effective_lower(const VariableData& variable) {
  return variable.type == VariableType::SemiContinuous ||
    variable.type == VariableType::SemiInteger ? 0.0 : variable.lower;
}

double outward(double value, bool up) {
  if (!std::isfinite(value))
    throw ModelError("Indicator activity/M overflow; tighten domains or rescale the model");
  const double result = std::nextafter(value, up ? infinity : -infinity);
  if (!std::isfinite(result))
    throw ModelError("Indicator activity/M cannot be enclosed finitely; rescale the model");
  return result;
}

double add_bound(double a, double b, bool up) {
  if (a == 0.0) return b;
  if (b == 0.0) return a;
  if (a == -b) return 0.0;
  return outward(a + b, up);
}

double product_bound(double coefficient, double endpoint, bool up) {
  if (!std::isfinite(endpoint))
    throw ModelError("Indicator needs a finite inactive-domain activity bound; tighten the relevant variable bounds");
  if (endpoint == 0.0) return 0.0;
  if (endpoint == 1.0) return coefficient;
  if (endpoint == -1.0) return -coefficient;
  if (coefficient == 1.0) return endpoint;
  if (coefficient == -1.0) return -endpoint;
  return outward(coefficient * endpoint, up);
}

struct Derived {
  std::optional<double> lower_m;
  std::optional<double> upper_m;
};

Derived derive(const IndicatorData& data) {
  Derived result;
  for (bool upper : {false, true}) {
    const double bound = upper ? data.upper : data.lower;
    if (!std::isfinite(bound)) continue;
    double activity = 0.0;
    std::size_t domain = 0;
    for (const auto& term : data.terms) {
      double lower, higher;
      if (term.variable == data.activator) {
        lower = higher = data.active_value ? 0.0 : 1.0;
      } else {
        if (domain >= data.domains.size() || data.domains[domain].variable != term.variable)
          throw ModelError("Indicator domain dependencies do not match original terms");
        lower = data.domains[domain].lower;
        higher = data.domains[domain].upper;
        ++domain;
      }
      const bool use_upper = (term.coefficient > 0.0) == upper;
      const double contribution = product_bound(term.coefficient,
                                                use_upper ? higher : lower, upper);
      activity = add_bound(activity, contribution, upper);
    }
    double m = 0.0;
    if (upper ? activity > bound : bound > activity)
      m = upper ? add_bound(activity, -bound, true) : add_bound(bound, -activity, true);
    if (upper) result.upper_m = m;
    else result.lower_m = m;
  }
  return result;
}

bool needs_gate(const IndicatorData& data) {
  return data.lower_m.value_or(0.0) > 0.0 || data.upper_m.value_or(0.0) > 0.0;
}

struct LoweredRow {
  std::vector<Term> terms;
  double lower;
  double upper;
  const char* suffix;
};

std::vector<LoweredRow> lower_rows(const IndicatorData& data) {
  std::vector<LoweredRow> result;
  if (data.inactive_gate) {
    std::vector<Term> terms{{data.activator, data.active_value ? 1.0 : -1.0},
                             {*data.inactive_gate, 1.0}};
    std::sort(terms.begin(), terms.end(), [](const Term& a, const Term& b) {
      return a.variable.id < b.variable.id;
    });
    const double rhs = data.active_value ? 1.0 : 0.0;
    result.push_back({std::move(terms), rhs, rhs, "gate"});
  }
  for (bool upper : {false, true}) {
    const auto& m = upper ? data.upper_m : data.lower_m;
    if (!m) continue;
    auto terms = data.terms;
    if (*m > 0.0) {
      if (!data.inactive_gate) throw ModelError("Indicator M requires an inactive gate");
      terms.push_back({*data.inactive_gate, upper ? -*m : *m});
      std::sort(terms.begin(), terms.end(), [](const Term& a, const Term& b) {
        return a.variable.id < b.variable.id;
      });
    }
    result.push_back({std::move(terms), upper ? -infinity : data.lower,
                       upper ? data.upper : infinity, upper ? "upper" : "lower"});
  }
  return result;
}

bool same_terms(const std::vector<Term>& a, const std::vector<Term>& b) {
  if (a.size() != b.size()) return false;
  for (std::size_t i = 0; i < a.size(); ++i)
    if (a[i].variable != b[i].variable || a[i].coefficient != b[i].coefficient)
      return false;
  return true;
}

const VariableData& checked_variable(const ModelSnapshot& model, Variable handle,
                                     bool active = true) {
  if (handle.model_id != model.model_id || handle.id >= model.variables.size() ||
      (active && !model.variables[static_cast<std::size_t>(handle.id)].active))
    throw ModelError("Indicator references a foreign, absent, or deleted variable");
  return model.variables[static_cast<std::size_t>(handle.id)];
}

void require_binary(const VariableData& variable) {
  if (variable.type != VariableType::Binary)
    throw ModelError("Logical helpers require Binary variables; use add_binary");
}
} // namespace

namespace Detail {

/** Private two-phase append: allocations complete before any observable change. */
class ConstraintBatch {
  Model& model_;
  std::vector<VariableData> variables_;
  std::vector<RowData> rows_;
  std::vector<IndicatorData> indicators_;

  template<class T>
  struct Append {
    std::vector<T>& target;
    std::vector<T>& additions;
    std::vector<T> replacement;
    bool replace = false;
    Append(std::vector<T>& target, std::vector<T>& additions)
        : target(target), additions(additions) {
      static_assert(std::is_nothrow_move_constructible<T>::value, "Commit must not throw");
      if (additions.size() > target.max_size() - target.size())
        throw ModelError("Logical formulation exceeds container limits");
      const auto desired = target.size() + additions.size();
      if (desired <= target.capacity()) return;
      replace = true;
      const auto extra = std::min(target.size(), target.max_size() - target.size());
      replacement.reserve(std::max(desired, target.size() + extra));
      replacement.insert(replacement.end(), target.begin(), target.end());
      for (auto& value : additions) replacement.push_back(std::move(value));
    }
    void commit() noexcept {
      if (replace) target.swap(replacement);
      else for (auto& value : additions) target.push_back(std::move(value));
    }
  };

public:
  explicit ConstraintBatch(Model& model) : model_(model) {
    model_.require_revision_capacity();
  }
  std::vector<Term> normalize(const std::vector<Term>& terms) const {
    return model_.normalize(terms, &variables_);
  }
  Variable binary(std::string name, Indicator origin) {
    if (variables_.size() >= std::numeric_limits<std::uint64_t>::max() - model_.variables_.size())
      throw ModelError("Variable slot space exhausted");
    Variable handle{model_.id(), static_cast<std::uint64_t>(model_.variables_.size() + variables_.size())};
    variables_.push_back({handle, VariableType::Binary, 0.0, 1.0, std::move(name), true, origin});
    return handle;
  }
  Indicator indicator_handle() const {
    if (indicators_.size() >= std::numeric_limits<std::uint64_t>::max() - model_.indicators_.size())
      throw ModelError("Indicator slot space exhausted");
    return {model_.id(), static_cast<std::uint64_t>(model_.indicators_.size() + indicators_.size())};
  }
  Constraint row(const std::vector<Term>& terms, double lower, double upper,
                 std::string name, std::optional<Indicator> origin = {}) {
    bounds(lower, upper);
    if (rows_.size() >= std::numeric_limits<std::uint64_t>::max() - model_.rows_.size())
      throw ModelError("Constraint slot space exhausted");
    auto normalized = normalize(terms);
    Constraint handle{model_.id(), static_cast<std::uint64_t>(model_.rows_.size() + rows_.size())};
    rows_.push_back({handle, std::move(normalized), lower, upper, std::move(name), true, origin});
    return handle;
  }
  void indicator(IndicatorData data) { indicators_.push_back(std::move(data)); }
  void commit() {
    if (variables_.empty() && rows_.empty() && indicators_.empty()) return;
    model_.require_revision_capacity();
    Append<VariableData> variables(model_.variables_, variables_);
    Append<RowData> rows(model_.rows_, rows_);
    Append<IndicatorData> indicators(model_.indicators_, indicators_);
    variables.commit();
    rows.commit();
    indicators.commit();
    ++model_.revision_;
  }
  static void remove(Model& model, Indicator handle) {
    model.require_revision_capacity();
    if (handle.model_id != model.id() || handle.id >= model.indicators_.size() ||
        !model.indicators_[static_cast<std::size_t>(handle.id)].active)
      throw ModelError("Indicator handle is foreign, invalid, or removed");
    auto& data = model.indicators_[static_cast<std::size_t>(handle.id)];
    for (auto row : data.generated_rows) {
      auto& generated = model.rows_[static_cast<std::size_t>(row.id)];
      generated.active = false;
      generated.terms.clear();
    }
    data.active = false;
    ++model.revision_;
  }
};

void validate_indicators(const ModelSnapshot& model) {
  std::set<std::uint64_t> owned_rows;
  std::set<std::uint64_t> owned_gates;
  for (std::size_t i = 0; i < model.indicators.size(); ++i) {
    const auto& data = model.indicators[i];
    if (data.indicator.model_id != model.model_id || data.indicator.id != i)
      throw ModelError("Indicator identity does not match its model/slot");
    if (data.inactive_gate) {
      const auto& gate = checked_variable(model, *data.inactive_gate, data.active);
      if (!owned_gates.insert(data.inactive_gate->id).second || !gate.indicator_origin ||
          gate.indicator_origin->model_id != model.model_id || gate.indicator_origin->id != i)
        throw ModelError("Indicator gate identity is invalid, shared, or missing its origin");
    }
    for (auto handle : data.generated_rows) {
      if (handle.model_id != model.model_id || handle.id >= model.rows.size() ||
          !owned_rows.insert(handle.id).second)
        throw ModelError("Indicator generated row identity is invalid or duplicated");
      const auto& row = model.rows[static_cast<std::size_t>(handle.id)];
      if (!row.indicator_origin || row.indicator_origin->model_id != model.model_id ||
          row.indicator_origin->id != i || row.active != data.active)
        throw ModelError("Indicator/generated row lifecycle mismatch");
    }
    if (!data.active) continue;
    require_binary(checked_variable(model, data.activator));
    bounds(data.lower, data.upper);
    std::optional<std::uint64_t> previous;
    std::size_t domain = 0;
    for (const auto& term : data.terms) {
      const auto& variable = checked_variable(model, term.variable);
      if (!std::isfinite(term.coefficient) || term.coefficient == 0.0 ||
          (previous && term.variable.id <= *previous))
        throw ModelError("Indicator original terms are not canonical");
      previous = term.variable.id;
      if (term.variable == data.activator) continue;
      if (domain >= data.domains.size() || data.domains[domain].variable != term.variable)
        throw ModelError("Indicator domain dependencies do not match original terms");
      const auto& captured = data.domains[domain++];
      bounds(captured.lower, captured.upper);
      if (effective_lower(variable) < captured.lower || variable.upper > captured.upper)
        throw ModelError("Model bounds exceed the domains used by an indicator; rebuild the formulation");
    }
    if (domain != data.domains.size())
      throw ModelError("Indicator has unrelated domain dependencies");
    const auto required = derive(data);
    for (bool upper : {false, true}) {
      const auto& supplied = upper ? data.upper_m : data.lower_m;
      const auto& minimum = upper ? required.upper_m : required.lower_m;
      if (supplied.has_value() != minimum.has_value() ||
          (supplied && (!std::isfinite(*supplied) || *supplied < *minimum)))
        throw ModelError("Indicator M does not safely relax the captured inactive domain");
    }
    if (data.inactive_gate.has_value() != needs_gate(data))
      throw ModelError("Indicator inactive gate does not match its relaxation");
    if (data.inactive_gate) {
      const auto& gate = checked_variable(model, *data.inactive_gate);
      require_binary(gate);
      if (gate.lower != 0.0 || gate.upper != 1.0 ||
          *data.inactive_gate == data.activator ||
          std::any_of(data.terms.begin(), data.terms.end(), [&](const Term& term) {
            return term.variable == *data.inactive_gate;
          }))
        throw ModelError("Indicator gate is aliased or its domain has changed");
    }
    const auto expected = lower_rows(data);
    if (expected.size() != data.generated_rows.size())
      throw ModelError("Indicator is missing generated rows");
    for (std::size_t r = 0; r < expected.size(); ++r) {
      const auto& row = model.rows[static_cast<std::size_t>(data.generated_rows[r].id)];
      if (row.lower != expected[r].lower || row.upper != expected[r].upper ||
          !same_terms(row.terms, expected[r].terms))
        throw ModelError("Indicator generated row no longer matches its original meaning");
    }
  }
  for (const auto& row : model.rows)
    if (row.indicator_origin && owned_rows.count(row.constraint.id) == 0)
      throw ModelError("Generated indicator row is missing its original metadata");
  for (const auto& variable : model.variables)
    if (variable.indicator_origin && owned_gates.count(variable.variable.id) == 0)
      throw ModelError("Generated indicator gate is missing its original metadata");
}
} // namespace Detail

IndicatorFormulation add_indicator(Model& model, Variable activator,
                                  bool active_value, const std::vector<Term>& terms,
                                  double lower, double upper, std::string name) {
  Detail::ConstraintBatch batch(model);
  require_binary(model.variable(activator));
  bounds(lower, upper);
  IndicatorData data;
  data.indicator = batch.indicator_handle();
  data.activator = activator;
  data.active_value = active_value;
  data.terms = batch.normalize(terms);
  data.lower = lower;
  data.upper = upper;
  if (!std::isfinite(lower) && !std::isfinite(upper)) data.terms.clear();
  for (const auto& term : data.terms)
    if (term.variable != activator) {
      const auto& variable = model.variable(term.variable);
      data.domains.push_back({term.variable, effective_lower(variable), variable.upper});
    }
  const auto derived = derive(data);
  data.lower_m = derived.lower_m;
  data.upper_m = derived.upper_m;
  const std::string prefix = name.empty() ? "indicator_" + std::to_string(data.indicator.id) : name;
  if (needs_gate(data)) data.inactive_gate = batch.binary(prefix + "_inactive", data.indicator);
  for (const auto& row : lower_rows(data))
    data.generated_rows.push_back(batch.row(row.terms, row.lower, row.upper,
                                            prefix + "_" + row.suffix, data.indicator));
  IndicatorFormulation result{data.indicator, data.inactive_gate, data.generated_rows,
                               data.lower_m, data.upper_m};
  batch.indicator(std::move(data));
  batch.commit();
  return result;
}

void remove_indicator(Model& model, Indicator indicator) {
  Detail::ConstraintBatch::remove(model, indicator);
}

namespace {
std::vector<Constraint> boolean(Model& model, Variable result,
                                std::vector<Variable> inputs, bool conjunction,
                                std::string name) {
  Detail::ConstraintBatch batch(model);
  require_binary(model.variable(result));
  for (auto input : inputs) require_binary(model.variable(input));
  std::sort(inputs.begin(), inputs.end(), [](Variable a, Variable b) { return a.id < b.id; });
  inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
  if (inputs.size() > 9007199254740991ULL)
    throw ModelError("Boolean cardinality exceeds exact double integer range");
  const std::string prefix = name.empty() ? (conjunction ? "and" : "or") : name;
  std::vector<Constraint> rows;
  if (inputs.empty()) {
    const double value = conjunction ? 1.0 : 0.0;
    rows.push_back(batch.row({{result, 1.0}}, value, value, prefix + "_empty"));
  } else if (inputs.size() == 1) {
    rows.push_back(batch.row({{result, 1.0}, {inputs.front(), -1.0}}, 0, 0, prefix + "_equal"));
  } else {
    for (std::size_t i = 0; i < inputs.size(); ++i)
      rows.push_back(batch.row({{result, 1.0}, {inputs[i], -1.0}},
                               conjunction ? -infinity : 0.0,
                               conjunction ? 0.0 : infinity,
                               prefix + "_input_" + std::to_string(i)));
    std::vector<Term> aggregate{{result, 1.0}};
    for (auto input : inputs) aggregate.push_back({input, -1.0});
    rows.push_back(batch.row(aggregate,
                             conjunction ? 1.0 - static_cast<double>(inputs.size()) : -infinity,
                             conjunction ? infinity : 0.0, prefix + "_aggregate"));
  }
  batch.commit();
  return rows;
}
} // namespace

std::vector<Constraint> add_boolean_and(Model& model, Variable result,
                                      const std::vector<Variable>& inputs,
                                      std::string name) {
  return boolean(model, result, inputs, true, std::move(name));
}
std::vector<Constraint> add_boolean_or(Model& model, Variable result,
                                     const std::vector<Variable>& inputs,
                                     std::string name) {
  return boolean(model, result, inputs, false, std::move(name));
}

}}
