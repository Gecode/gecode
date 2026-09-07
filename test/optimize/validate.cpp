#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/constraints.hpp>

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace O = Gecode::Optimize;

namespace {

constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double nan = std::numeric_limits<double>::quiet_NaN();

void require(bool condition, const std::string& message) {
  if (!condition)
    throw std::runtime_error(message);
}

void near(double actual, double expected, const std::string& message) {
  require(std::fabs(actual - expected) <= 1e-12, message);
}

O::ModelSnapshot empty_model() {
  O::ModelSnapshot model;
  model.model_id = 42;
  return model;
}

O::Variable add_variable(O::ModelSnapshot& model, O::VariableType type,
                         double lower, double upper, bool active = true) {
  O::VariableData data;
  data.variable = {model.model_id, static_cast<std::uint64_t>(model.variables.size())};
  data.type = type;
  data.lower = lower;
  data.upper = upper;
  data.active = active;
  model.variables.push_back(data);
  return data.variable;
}

void add_row(O::ModelSnapshot& model, std::vector<O::Term> terms,
             double lower, double upper, bool active = true) {
  O::RowData data;
  data.constraint = {model.model_id, static_cast<std::uint64_t>(model.rows.size())};
  data.terms = std::move(terms);
  data.lower = lower;
  data.upper = upper;
  data.active = active;
  model.rows.push_back(std::move(data));
}

O::ModelSnapshot production() {
  auto model = empty_model();
  const auto open = add_variable(model, O::VariableType::Binary, 0.0, 1.0);
  const auto quantity = add_variable(model, O::VariableType::Continuous, 0.0, 100.0);
  add_row(model, {{open, -100.0}, {quantity, 1.0}}, -infinity, 0.0);
  add_row(model, {{quantity, 1.0}}, 40.0, infinity);
  model.objective.terms = {{open, 12.0}, {quantity, 0.5}};
  model.objective.offset = 7.0;
  return model;
}

void malformed(const O::ModelSnapshot& model, const std::string& description) {
  bool threw = false;
  try {
    O::validate_structure(model);
  } catch (const O::ModelError&) {
    threw = true;
  }
  require(threw, "structure accepted " + description);
  const auto report = O::validate(model, std::vector<double>(model.variables.size(), 0.0));
  require(!report.valid && !report.model_valid && !report.message.empty() && !report.objective,
          "malformed model did not return an invalid report: " + description);
}

void analytic_checks() {
  // The residual is one even when a platform rounds the row activity to 1e16.
  // Check both range sides and the independent original indicator predicate.
  for (double sign : {-1.0, 1.0}) {
    O::Model cancellation;
    auto x = cancellation.add_continuous(1, 1);
    auto y = cancellation.add_continuous(1e6, 1e6);
    auto active = cancellation.add_binary();
    auto row = cancellation.add_row({{x, sign}, {y, 1e10}}, 1e16, 1e16);
    auto snapshot = cancellation.snapshot();
    auto failed = O::validate(snapshot, {1, 1e6, 1}, 0.9, 0);
    require(!failed.valid, "small residual lost after large row cancellation");
    near(failed.max_row_violation, 1, "large row cancellation residual incorrect");
    require(O::validate(snapshot, {1, 1e6, 1}, 1, 0).valid,
            "absolute tolerance mishandled after large row cancellation");
    cancellation.remove(row);
    O::add_indicator(cancellation, active, true, {{x, sign}, {y, 1e10}}, 1e16, 1e16);
    snapshot = cancellation.snapshot();
    std::vector<double> values(snapshot.variables.size(), 0);
    values[x.id] = 1; values[y.id] = 1e6; values[active.id] = 1;
    failed = O::validate(snapshot, values, 0.9, 0);
    require(!failed.valid, "small residual lost after large indicator cancellation");
    near(failed.max_indicator_violation, 1, "indicator cancellation residual incorrect");
  }
  const auto model = production();
  O::validate_structure(model);
  auto report = O::validate(model, {1.0, 40.0});
  require(report.valid && report.model_valid && report.objective, "feasible production rejected");
  near(*report.objective, 39.0, "objective offset not included");
  near(report.max_bound_violation, 0.0, "valid bounds have violation");
  near(report.max_row_violation, 0.0, "valid rows have violation");
  near(report.max_integrality_violation, 0.0, "valid integers have violation");

  report = O::validate(model, {1.0, 39.0});
  require(!report.valid && report.model_valid, "demand violation accepted");
  near(report.max_row_violation, 1.0, "demand residual incorrect");
  report = O::validate(model, {0.5, 50.0});
  require(!report.valid, "fractional binary accepted");
  near(report.max_integrality_violation, 0.5, "integrality residual incorrect");
  report = O::validate(model, {1.25, 40.0});
  require(!report.valid, "binary bound violation accepted");
  near(report.max_bound_violation, 0.25, "binary bound residual incorrect");

  auto negative = empty_model();
  const auto x = add_variable(negative, O::VariableType::Integer, -10.0, -2.0);
  add_row(negative, {{x, -2.0}}, 6.0, 8.0);
  negative.objective.terms = {{x, 3.0}};
  negative.objective.offset = -1.0;
  negative.objective.sense = O::ObjectiveSense::Maximize;
  report = O::validate(negative, {-3.0});
  require(report.valid && report.objective, "negative integer/ranged row rejected");
  near(*report.objective, -10.0, "maximize objective was wrongly negated");
  report = O::validate(negative, {-5.0});
  require(!report.valid, "upper ranged-row violation accepted");
  near(report.max_row_violation, 2.0, "upper ranged-row residual incorrect");

  auto empty = empty_model();
  empty.objective.offset = 9.0;
  report = O::validate(empty, {});
  require(report.valid && report.objective, "empty feasible model rejected");
  near(*report.objective, 9.0, "empty objective offset incorrect");
  add_row(empty, {}, 1.0, infinity);
  O::validate_structure(empty);
  report = O::validate(empty, {});
  require(!report.valid && report.model_valid, "constant infeasible row accepted");
  near(report.max_row_violation, 1.0, "constant infeasible row residual incorrect");

  auto no_integer = empty_model();
  add_variable(no_integer, O::VariableType::Integer, 0.2, 0.8);
  O::validate_structure(no_integer);
  report = O::validate(no_integer, {0.5});
  require(report.model_valid && !report.valid, "empty integer interval is not treated as infeasible");
}

void semi_checks() {
  auto model = empty_model();
  add_variable(model, O::VariableType::SemiContinuous, 2.0, 5.0);
  add_variable(model, O::VariableType::SemiInteger, 3.0, 7.0);
  require(O::validate(model, {0.0, 0.0}).valid, "zero semi-variable alternatives rejected");
  require(O::validate(model, {2.25, 4.0}).valid, "nonzero semi-variable alternatives rejected");
  auto report = O::validate(model, {1.0, 2.0});
  require(!report.valid, "semi-variable holes accepted");
  near(report.max_bound_violation, 1.0, "semi-variable distance incorrect");
  report = O::validate(model, {2.25, 3.5});
  require(!report.valid, "fractional semi-integer accepted");
  near(report.max_integrality_violation, 0.5, "semi-integer residual incorrect");
  report = O::validate(model, {-0.25, 0.0});
  require(!report.valid, "negative semi-continuous accepted");
  near(report.max_bound_violation, 0.25, "semi-variable zero distance incorrect");
  require(O::validate(model, {-5e-8, 0.0}, 1e-7, 0.0).valid,
          "semi-continuous zero tolerance rejected");
  require(!O::validate(model, {0.0, 5e-8}, 1e-7, 0.0).valid,
          "semi-integer zero bypassed integrality tolerance");
  model.variables[0].upper = infinity;
  require(O::validate(model, {1e100, 0.0}).valid,
          "abstract semi-variable infinite upper bound rejected");

  auto semi_empty = empty_model();
  add_variable(semi_empty, O::VariableType::SemiInteger, 0.2, 0.8);
  require(O::validate(semi_empty, {0.0}).valid,
          "semi-integer with empty positive interval lost zero alternative");
}

void malformed_checks() {
  using Mutation = std::function<void(O::ModelSnapshot&)>;
  const std::vector<std::pair<std::string, Mutation>> cases = {
    {"zero model ID", [](auto& m) { m.model_id = 0; }},
    {"variable wrong owner", [](auto& m) { m.variables[0].variable.model_id = 999; }},
    {"variable wrong slot", [](auto& m) { m.variables[0].variable.id = 1; }},
    {"row wrong owner", [](auto& m) { m.rows[0].constraint.model_id = 999; }},
    {"row wrong slot", [](auto& m) { m.rows[0].constraint.id = 1; }},
    {"variable NaN lower", [](auto& m) { m.variables[1].lower = nan; }},
    {"variable NaN upper", [](auto& m) { m.variables[1].upper = nan; }},
    {"variable positive infinite lower", [](auto& m) { m.variables[1].lower = infinity; }},
    {"variable negative infinite upper", [](auto& m) { m.variables[1].upper = -infinity; }},
    {"reversed variable bounds", [](auto& m) { m.variables[1].lower = 101.0; }},
    {"binary lower below zero", [](auto& m) { m.variables[0].lower = -1.0; }},
    {"binary upper above one", [](auto& m) { m.variables[0].upper = 2.0; }},
    {"row NaN lower", [](auto& m) { m.rows[0].lower = nan; }},
    {"row NaN upper", [](auto& m) { m.rows[0].upper = nan; }},
    {"row positive infinite lower", [](auto& m) { m.rows[0].lower = infinity; }},
    {"row negative infinite upper", [](auto& m) { m.rows[0].upper = -infinity; }},
    {"reversed row bounds", [](auto& m) { m.rows[0].lower = 1.0; }},
    {"negative variable type", [](auto& m) { m.variables[0].type = static_cast<O::VariableType>(-1); }},
    {"unknown variable type", [](auto& m) { m.variables[0].type = static_cast<O::VariableType>(99); }},
    {"negative objective sense", [](auto& m) { m.objective.sense = static_cast<O::ObjectiveSense>(-1); }},
    {"unknown objective sense", [](auto& m) { m.objective.sense = static_cast<O::ObjectiveSense>(99); }},
    {"NaN offset", [](auto& m) { m.objective.offset = nan; }},
    {"infinite offset", [](auto& m) { m.objective.offset = infinity; }},
    {"row wrong term owner", [](auto& m) { m.rows[0].terms[0].variable.model_id = 999; }},
    {"row dangling term", [](auto& m) { m.rows[0].terms[1].variable.id = 99; }},
    {"objective wrong term owner", [](auto& m) { m.objective.terms[0].variable.model_id = 999; }},
    {"objective dangling term", [](auto& m) { m.objective.terms[1].variable.id = 99; }},
    {"row NaN coefficient", [](auto& m) { m.rows[0].terms[0].coefficient = nan; }},
    {"row infinite coefficient", [](auto& m) { m.rows[0].terms[0].coefficient = infinity; }},
    {"objective NaN coefficient", [](auto& m) { m.objective.terms[0].coefficient = nan; }},
    {"objective infinite coefficient", [](auto& m) { m.objective.terms[0].coefficient = infinity; }},
    {"row zero coefficient", [](auto& m) { m.rows[0].terms[0].coefficient = 0.0; }},
    {"objective zero coefficient", [](auto& m) { m.objective.terms[0].coefficient = -0.0; }},
    {"duplicate row terms", [](auto& m) { m.rows[0].terms.push_back(m.rows[0].terms.back()); }},
    {"duplicate objective terms", [](auto& m) { m.objective.terms.push_back(m.objective.terms.back()); }},
    {"unsorted row terms", [](auto& m) { std::swap(m.rows[0].terms[0], m.rows[0].terms[1]); }},
    {"unsorted objective terms", [](auto& m) { std::swap(m.objective.terms[0], m.objective.terms[1]); }},
    {"active deleted-variable reference", [](auto& m) { m.variables[0].active = false; }}
  };
  for (const auto& test : cases) {
    auto model = production();
    test.second(model);
    malformed(model, test.first);
  }
  for (const double lower : {0.0, -1.0, -infinity, infinity}) {
    auto model = empty_model();
    add_variable(model, O::VariableType::SemiContinuous, lower, infinity);
    malformed(model, "unsupported semi-variable lower bound");
  }
}

void assignment_and_tolerance_checks() {
  const auto model = production();
  for (const auto& values : std::vector<std::vector<double>>{
         {}, {1.0}, {1.0, 40.0, 0.0}, {nan, 40.0}, {1.0, infinity}, {1.0, -infinity}}) {
    const auto report = O::validate(model, values);
    require(!report.valid && report.model_valid && !report.objective,
            "invalid assignment accepted or mislabeled as malformed model");
  }
  for (const double tolerance : {-1.0, infinity, -infinity, nan}) {
    auto report = O::validate(model, {1.0, 40.0}, tolerance, 0.0);
    require(!report.valid && report.model_valid, "invalid feasibility tolerance accepted");
    report = O::validate(model, {1.0, 40.0}, 0.0, tolerance);
    require(!report.valid && report.model_valid, "invalid integrality tolerance accepted");
  }
  require(O::validate(model, {1.0 + 5e-8, 40.0}, 1e-7, 1e-6).valid,
          "absolute bound/integrality tolerance rejected");
  require(!O::validate(model, {1.0 + 5e-8, 40.0}, 0.0, 1e-6).valid,
          "zero feasibility tolerance ignored");
  auto scaled = empty_model();
  const auto x = add_variable(scaled, O::VariableType::Continuous, -infinity, infinity);
  add_row(scaled, {{x, 1e9}}, -infinity, 1e9);
  const auto report = O::validate(scaled, {1.0 + 1e-8}, 1e-7, 1e-6);
  require(!report.valid && report.max_row_violation > 9.0,
          "row tolerance was silently normalized by scale");
}

void deleted_slot_checks() {
  auto model = production();
  const auto removed = add_variable(model, O::VariableType::Continuous, -infinity, infinity, false);
  add_row(model, {{removed, 1.0}}, 99.0, 99.0, false);
  auto report = O::validate(model, {1.0, 40.0, nan});
  require(report.valid && report.objective, "unreferenced deleted NaN slot was evaluated");
  near(*report.objective, 39.0, "deleted row changed objective");
  require(!O::validate(model, {1.0, 40.0}).valid, "deleted slot omitted from assignment dimension");
  auto altered = model;
  altered.rows.back().active = true;
  malformed(altered, "reactivated row referencing deleted slot");
  altered = model;
  altered.objective.terms.push_back({removed, 1.0});
  malformed(altered, "objective references deleted slot");
  altered = model;
  altered.variables.back().variable.model_id = 999;
  malformed(altered, "deleted slot wrong owner");
  altered = model;
  altered.rows.back().constraint.id = 999;
  malformed(altered, "deleted row wrong slot");
}

void accumulation_checks() {
  auto model = empty_model();
  const auto x = add_variable(model, O::VariableType::Continuous, -infinity, infinity);
  model.objective.terms = {{x, std::numeric_limits<double>::max()}};
  model.objective.offset = std::numeric_limits<double>::max();
  auto report = O::validate(model, {1.0});
  require(!report.valid && report.model_valid && !report.objective,
          "objective offset accumulation overflow accepted");
  model.objective.offset = 0.0;
  report = O::validate(model, {std::numeric_limits<double>::max()});
  require(!report.valid && !report.objective, "objective product overflow accepted");
  model.objective.terms.clear();
  add_row(model, {{x, std::numeric_limits<double>::max()}}, -infinity, 1.0);
  report = O::validate(model, {std::numeric_limits<double>::max()});
  require(!report.valid, "large row activity accepted");

  // Many large positive terms, a small term, then many large negative terms
  // exercise cancellation independently of sparse coalescing in Model.
  auto cancellation = empty_model();
  std::vector<O::Term> terms;
  constexpr std::size_t side = 1024;
  for (std::size_t i = 0; i < 2 * side + 1; ++i) {
    const auto variable = add_variable(cancellation, O::VariableType::Continuous, 1.0, 1.0);
    terms.push_back({variable, i < side ? 1e16 : (i == side ? 1.0 : -1e16)});
  }
  add_row(cancellation, terms, 1.0, 1.0);
  cancellation.objective.terms = terms;
  cancellation.objective.offset = 11.0;
  report = O::validate(cancellation, std::vector<double>(terms.size(), 1.0), 0.0, 0.0);
  require(report.valid && report.objective, "compensated large-term cancellation failed");
  near(*report.objective, 12.0, "compensated objective incorrect");
}

}

int main() {
  try {
    analytic_checks();
    semi_checks();
    malformed_checks();
    assignment_and_tolerance_checks();
    deleted_slot_checks();
    accumulation_checks();
    std::cout << "PASS independent original-model validation\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }
}
