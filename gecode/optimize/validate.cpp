/* Independent original-model checks; no numerical backend is consulted. */
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/globals.hpp>
#include <gecode/optimize/constraints.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Gecode { namespace Optimize {
namespace {

using Wide = long double;

std::string slot(const char* kind, std::size_t index) {
  return std::string(kind) + " slot " + std::to_string(index);
}

void check_range(double lower, double upper, const std::string& location) {
  if (std::isnan(lower) || std::isnan(upper))
    throw ModelError(location + ": NaN bound");
  if (lower == std::numeric_limits<double>::infinity() ||
      upper == -std::numeric_limits<double>::infinity())
    throw ModelError(location + ": invalid infinity in bound");
  if (lower > upper)
    throw ModelError(location + ": lower bound exceeds upper bound");
}

void check_type(const VariableData& variable, const std::string& location) {
  switch (variable.type) {
  case VariableType::Continuous:
  case VariableType::Integer:
    break;
  case VariableType::Binary:
    if (variable.lower < 0.0 || variable.upper > 1.0)
      throw ModelError(location + ": binary bounds must lie in [0,1]");
    break;
  case VariableType::SemiContinuous:
  case VariableType::SemiInteger:
    if (!std::isfinite(variable.lower) || variable.lower <= 0.0)
      throw ModelError(location + ": semi-variable lower bound must be finite and positive");
    break;
  default:
    throw ModelError(location + ": unknown variable type");
  }
}

void check_terms(const ModelSnapshot& model, const std::vector<Term>& terms,
                 const std::string& location, bool active) {
  std::uint64_t previous = 0;
  bool first = true;
  for (const auto& term : terms) {
    if (term.variable.model_id != model.model_id)
      throw ModelError(location + ": variable belongs to another model");
    if (term.variable.id >= model.variables.size())
      throw ModelError(location + ": dangling variable reference");
    if (active && !model.variables[static_cast<std::size_t>(term.variable.id)].active)
      throw ModelError(location + ": deleted variable reference");
    if (!std::isfinite(term.coefficient))
      throw ModelError(location + ": non-finite coefficient");
    if (term.coefficient == 0.0)
      throw ModelError(location + ": zero coefficient is not canonical");
    if (!first && term.variable.id <= previous)
      throw ModelError(location + ": terms must have ascending unique variable IDs");
    previous = term.variable.id;
    first = false;
  }
}

// Compensated summation also helps on platforms where long double has the
// same precision as double. Overflow still invalidates the numerical check.
class WideSum {
  Wide sum_ = 0.0L;
  Wide correction_ = 0.0L;
public:
  bool add(Wide value) {
    if (!std::isfinite(value))
      return false;
    const Wide next = sum_ + value;
    if (!std::isfinite(next))
      return false;
    const Wide correction = std::fabs(sum_) >= std::fabs(value)
      ? (sum_ - next) + value : (value - next) + sum_;
    const Wide next_correction = correction_ + correction;
    if (!std::isfinite(next_correction))
      return false;
    sum_ = next;
    correction_ = next_correction;
    return true;
  }
  bool value(Wide& result) const {
    result = sum_ + correction_;
    return std::isfinite(result);
  }
};

bool evaluate(const std::vector<Term>& terms, double offset,
              const std::vector<double>& values, Wide& result) {
  WideSum sum;
  if (!sum.add(static_cast<Wide>(offset)))
    return false;
  for (const auto& term : terms) {
    const Wide product = static_cast<Wide>(term.coefficient) *
      static_cast<Wide>(values[static_cast<std::size_t>(term.variable.id)]);
    if (!sum.add(product))
      return false;
  }
  return sum.value(result);
}

Wide range_violation(Wide value, double lower, double upper) {
  Wide violation = 0.0L;
  if (std::isfinite(lower))
    violation = std::max(violation, static_cast<Wide>(lower) - value);
  if (std::isfinite(upper))
    violation = std::max(violation, value - static_cast<Wide>(upper));
  return violation;
}

// Include each finite bound in the compensated sum. Collapsing a large row
// activity first can erase a small residual before subtracting its bound (for
// example 1 + 1e16 <= 1e16 on platforms with double-width long double).
bool linear_violation(const std::vector<Term>& terms, double lower, double upper,
                      const std::vector<double>& values, Wide& violation) {
  Wide activity = 0.0L;
  if (!evaluate(terms, 0.0, values, activity))
    return false;
  violation = 0.0L;
  Wide residual = 0.0L;
  if (std::isfinite(lower)) {
    if (!evaluate(terms, -lower, values, residual))
      return false;
    violation = std::max(violation, -residual);
  }
  if (std::isfinite(upper)) {
    if (!evaluate(terms, -upper, values, residual))
      return false;
    violation = std::max(violation, residual);
  }
  return true;
}

void record_violation(Wide violation, double& maximum) {
  const double reported = violation > static_cast<Wide>(std::numeric_limits<double>::max())
    ? std::numeric_limits<double>::infinity() : static_cast<double>(violation);
  maximum = std::max(maximum, reported);
}

void record_failure(ValidationReport& report, const std::string& message) {
  if (report.message.empty())
    report.message = message;
}

} // namespace

void validate_structure(const ModelSnapshot& model) {
  if (model.model_id == 0)
    throw ModelError("model ID must be nonzero");
  for (std::size_t i = 0; i < model.variables.size(); ++i) {
    const auto& variable = model.variables[i];
    const auto location = slot("variable", i);
    if (variable.variable.model_id != model.model_id)
      throw ModelError(location + ": owner does not match model");
    if (variable.variable.id != i)
      throw ModelError(location + ": ID does not match stable slot");
    check_range(variable.lower, variable.upper, location);
    check_type(variable, location);
  }
  for (std::size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    const auto location = slot("row", i);
    if (row.constraint.model_id != model.model_id)
      throw ModelError(location + ": owner does not match model");
    if (row.constraint.id != i)
      throw ModelError(location + ": ID does not match stable slot");
    check_range(row.lower, row.upper, location);
    // Inactive rows can legitimately retain terms referencing tombstones.
    check_terms(model, row.terms, location, row.active);
  }
  if (!std::isfinite(model.objective.offset))
    throw ModelError("objective: non-finite offset");
  switch (model.objective.sense) {
  case ObjectiveSense::Minimize:
  case ObjectiveSense::Maximize:
    break;
  default:
    throw ModelError("objective: unknown sense");
  }
  check_terms(model, model.objective.terms, "objective", true);
  Detail::validate_indicators(model);
  Detail::validate_globals(model);
}

ValidationReport validate(const ModelSnapshot& model,
                          const std::vector<double>& slot_values,
                          double feasibility_tolerance,
                          double integrality_tolerance) {
  ValidationReport report;
  try {
    validate_structure(model);
    report.model_valid = true;
  } catch (const ModelError& error) {
    report.message = std::string("invalid model: ") + error.what();
    return report;
  }
  if (!std::isfinite(feasibility_tolerance) || feasibility_tolerance < 0.0 ||
      !std::isfinite(integrality_tolerance) || integrality_tolerance < 0.0) {
    report.message = "validation tolerances must be finite and nonnegative";
    return report;
  }
  if (slot_values.size() != model.variables.size()) {
    report.message = "assignment size does not match original variable slots";
    return report;
  }
  const Wide feasibility = static_cast<Wide>(feasibility_tolerance);
  const Wide integrality = static_cast<Wide>(integrality_tolerance);
  for (std::size_t i = 0; i < model.variables.size(); ++i) {
    const auto& variable = model.variables[i];
    if (!variable.active)
      continue;
    if (!std::isfinite(slot_values[i])) {
      report.message = slot("variable", i) + ": non-finite assignment";
      return report;
    }
    const Wide value = static_cast<Wide>(slot_values[i]);
    Wide bound_violation = range_violation(value, variable.lower, variable.upper);
    if (variable.type == VariableType::SemiContinuous ||
        variable.type == VariableType::SemiInteger)
      bound_violation = std::min(bound_violation, std::fabs(value));
    record_violation(bound_violation, report.max_bound_violation);
    if (!std::isfinite(bound_violation) || bound_violation > feasibility)
      record_failure(report, slot("variable", i) + ": bound violation");
    if (variable.type == VariableType::Integer ||
        variable.type == VariableType::Binary ||
        variable.type == VariableType::SemiInteger) {
      const Wide violation = std::fabs(value - std::round(value));
      record_violation(violation, report.max_integrality_violation);
      if (!std::isfinite(violation) || violation > integrality)
        record_failure(report, slot("variable", i) + ": integrality violation");
    }
  }
  for (std::size_t i = 0; i < model.rows.size(); ++i) {
    const auto& row = model.rows[i];
    if (!row.active)
      continue;
    Wide violation = 0.0L;
    if (!linear_violation(row.terms, row.lower, row.upper, slot_values, violation)) {
      report.message = slot("row", i) + ": non-finite accumulated activity";
      return report;
    }
    record_violation(violation, report.max_row_violation);
    if (!std::isfinite(violation) || violation > feasibility)
      record_failure(report, slot("row", i) + ": row violation");
  }
  for (std::size_t i = 0; i < model.indicators.size(); ++i) {
    const auto& indicator = model.indicators[i];
    if (!indicator.active ||
        std::round(slot_values[static_cast<std::size_t>(indicator.activator.id)]) !=
          (indicator.active_value ? 1.0 : 0.0))
      continue;
    Wide violation = 0.0L;
    if (!linear_violation(indicator.terms, indicator.lower, indicator.upper, slot_values, violation)) {
      report.message = slot("indicator", i) + ": non-finite original activity";
      return report;
    }
    record_violation(violation, report.max_indicator_violation);
    if (!std::isfinite(violation) || violation > feasibility)
      record_failure(report, slot("indicator", i) + ": original logical constraint violated");
  }
  for (std::size_t i=0;i<model.globals.size();++i) {
    const auto& record=model.globals[i];
    if (!record.active) continue;
    std::string reason;
    if (!Detail::global_satisfied(record.payload,slot_values,integrality_tolerance,reason)) {
      ++report.violated_globals;
      record_failure(report,slot("global",i)+": "+reason);
    }
  }
  Wide objective = 0.0L;
  if (!evaluate(model.objective.terms, model.objective.offset, slot_values, objective) ||
      std::fabs(objective) > static_cast<Wide>(std::numeric_limits<double>::max())) {
    report.message = "objective: non-finite or unrepresentable accumulated value";
    return report;
  }
  report.objective = static_cast<double>(objective);
  report.valid = report.message.empty();
  if (report.valid)
    report.message = "original model satisfied within numerical tolerances";
  return report;
}

}}
