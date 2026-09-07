#include <gecode/optimize/presolve.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
using Integer = std::int64_t;
constexpr Integer minimum = std::numeric_limits<Integer>::min();
constexpr Integer maximum = std::numeric_limits<Integer>::max();
constexpr Integer exact_limit = INT64_C(9007199254740992);
constexpr double infinity = std::numeric_limits<double>::infinity();
class Unsupported : public std::runtime_error { using std::runtime_error::runtime_error; };
class InvalidWitness : public std::runtime_error { using std::runtime_error::runtime_error; };
struct Interrupted { Termination reason; };
struct Contradiction { Constraint row; std::optional<Variable> variable; };
void checkpoint(const SolveBudget& budget) {
  if (auto reason = budget.stop_reason()) throw Interrupted{*reason};
}
Integer add(Integer a, Integer b) {
  if ((b > 0 && a > maximum-b) || (b < 0 && a < minimum-b))
    throw Unsupported("Exact presolve addition exceeds int64 range");
  return a+b;
}
Integer subtract(Integer a, Integer b) {
  if ((b > 0 && a < minimum+b) || (b < 0 && a > maximum+b))
    throw Unsupported("Exact presolve subtraction exceeds int64 range");
  return a-b;
}
Integer multiply(Integer a, Integer b) {
  if (a && b && ((a > 0 && ((b > 0 && a > maximum/b) || (b < 0 && b < minimum/a))) ||
                (a < 0 && ((b > 0 && a < minimum/b) || (b < 0 && a < maximum/b)))))
    throw Unsupported("Exact presolve multiplication exceeds int64 range");
  return a*b;
}
Integer divide(Integer value, Integer divisor, bool ceiling) {
  if (!divisor || (value == minimum && divisor == -1))
    throw Unsupported("Exact presolve division exceeds int64 range");
  Integer quotient = value/divisor, remainder = value%divisor;
  if (remainder && ((remainder > 0) == (divisor > 0))) {
    if (ceiling) quotient = add(quotient, 1);
  } else if (remainder && !ceiling) quotient = subtract(quotient, 1);
  return quotient;
}
Integer integer(double value) {
  if (!std::isfinite(value) || value != std::trunc(value) || std::fabs(value) > exact_limit)
    throw Unsupported("Exact presolve requires integral data of magnitude at most 2^53");
  return static_cast<Integer>(value);
}
double exported(Integer value) {
  if (value < -exact_limit || value > exact_limit)
    throw Unsupported("Reduced model or reconstructed objective exceeds exact double integer range");
  return static_cast<double>(value);
}
struct IntegerTerm { std::size_t slot; Integer coefficient; };
struct IntegerRow {
  Constraint original;
  std::vector<IntegerTerm> terms;
  std::optional<Integer> lower, upper;
  bool active = false, redundant = false;
};
struct Data {
  std::vector<Integer> lower, upper;
  std::vector<bool> active;
  std::vector<IntegerRow> rows;
  std::vector<IntegerTerm> objective;
  Integer offset = 0;
};
Data parse(const ModelSnapshot& source, const SolveBudget* budget = nullptr) {
  validate_structure(source);
  for (const auto& indicator : source.indicators)
    if (indicator.active) throw Unsupported("Exact presolve does not yet map active indicators");
  for (const auto& global : source.globals)
    if (global.active) throw Unsupported("Exact presolve does not yet map active globals");
  Data data;
  data.lower.resize(source.variables.size()); data.upper.resize(source.variables.size());
  for (const auto& variable : source.variables) {
    if (budget) checkpoint(*budget);
    data.active.push_back(variable.active);
    if (!variable.active) continue;
    if (variable.type != VariableType::Integer && variable.type != VariableType::Binary)
      throw Unsupported("Exact presolve requires bounded Integer/Binary variables");
    data.lower[variable.variable.id] = integer(variable.lower);
    data.upper[variable.variable.id] = integer(variable.upper);
  }
  for (const auto& original : source.rows) {
    if (budget) checkpoint(*budget);
    IntegerRow row; row.original = original.constraint; row.active = original.active;
    if (row.active) {
      if (std::isfinite(original.lower)) row.lower = integer(original.lower);
      if (std::isfinite(original.upper)) row.upper = integer(original.upper);
      for (const auto& term : original.terms) row.terms.push_back({static_cast<std::size_t>(term.variable.id), integer(term.coefficient)});
    }
    data.rows.push_back(std::move(row));
  }
  data.offset = integer(source.objective.offset);
  for (const auto& term : source.objective.terms)
    data.objective.push_back({static_cast<std::size_t>(term.variable.id), integer(term.coefficient)});
  return data;
}

bool propagate(Data& data, const PresolveOptions& options, PresolveResult& output, const SolveBudget& budget) {
  for (std::size_t pass = 0; pass < options.max_passes; ++pass) {
    bool changed = false;
    for (auto& row : data.rows) {
      checkpoint(budget);
      if (!row.active || row.redundant) continue;
      if (options.max_row_visits && output.row_visits >= *options.max_row_visits) return false;
      if (output.row_visits == std::numeric_limits<std::size_t>::max())
        throw Unsupported("Presolve row-visit counter exhausted");
      ++output.row_visits;
      if (!row.lower && !row.upper) { row.redundant = true; continue; }
      Integer lo = 0, hi = 0;
      std::vector<Integer> minima, maxima;
      for (const auto& term : row.terms) {
        checkpoint(budget);
        const auto a = multiply(term.coefficient, data.lower[term.slot]);
        const auto b = multiply(term.coefficient, data.upper[term.slot]);
        minima.push_back(std::min(a,b)); maxima.push_back(std::max(a,b));
        lo = add(lo, minima.back()); hi = add(hi, maxima.back());
      }
      if ((row.lower && hi < *row.lower) || (row.upper && lo > *row.upper))
        throw Contradiction{row.original, {}};
      if ((!row.lower || lo >= *row.lower) && (!row.upper || hi <= *row.upper)) {
        row.redundant = true; continue;
      }
      struct Proposal { std::size_t slot; Integer lower, upper; };
      std::vector<Proposal> proposals;
      // Every proposal uses the unchanged box seen at the beginning of this row.
      for (std::size_t i = 0; i < row.terms.size(); ++i) {
        checkpoint(budget);
        const auto& term = row.terms[i];
        Integer lower = data.lower[term.slot], upper = data.upper[term.slot];
        if (row.lower) {
          const auto rhs = subtract(*row.lower, subtract(hi, maxima[i]));
          if (term.coefficient > 0) lower = std::max(lower, divide(rhs, term.coefficient, true));
          else upper = std::min(upper, divide(rhs, term.coefficient, false));
        }
        if (row.upper) {
          const auto rhs = subtract(*row.upper, subtract(lo, minima[i]));
          if (term.coefficient > 0) upper = std::min(upper, divide(rhs, term.coefficient, false));
          else lower = std::max(lower, divide(rhs, term.coefficient, true));
        }
        if (lower > upper) throw Contradiction{row.original, Variable{row.original.model_id, term.slot}};
        proposals.push_back({term.slot, lower, upper});
      }
      for (const auto& proposal : proposals) {
        checkpoint(budget);
        const Variable variable{row.original.model_id, proposal.slot};
        if (proposal.lower != data.lower[proposal.slot]) {
          output.changes.push_back({variable, row.original, PresolveBoundSide::Lower,
                                    data.lower[proposal.slot], proposal.lower, pass});
          data.lower[proposal.slot] = proposal.lower; changed = true;
        }
        if (proposal.upper != data.upper[proposal.slot]) {
          output.changes.push_back({variable, row.original, PresolveBoundSide::Upper,
                                    data.upper[proposal.slot], proposal.upper, pass});
          data.upper[proposal.slot] = proposal.upper; changed = true;
        }
      }
    }
    ++output.passes;
    if (!changed) return true;
  }
  return false;
}

Integer exact_check(const ModelSnapshot& model, const std::vector<double>& values) {
  const auto data = parse(model);
  if (values.size() != model.variables.size()) throw InvalidWitness("Postsolve assignment has incorrect slot count");
  std::vector<Integer> point(values.size());
  for (std::size_t i = 0; i < values.size(); ++i) if (data.active[i]) {
    point[i] = integer(values[i]);
    if (point[i] < data.lower[i] || point[i] > data.upper[i])
      throw InvalidWitness("Postsolve assignment violates an exact original variable interval");
  }
  for (const auto& row : data.rows) if (row.active) {
    Integer value = 0;
    for (const auto& term : row.terms) value = add(value, multiply(term.coefficient, point[term.slot]));
    if ((row.lower && value < *row.lower) || (row.upper && value > *row.upper))
      throw InvalidWitness("Postsolve assignment violates an exact original linear row");
  }
  Integer objective = data.offset;
  for (const auto& term : data.objective) objective = add(objective, multiply(term.coefficient, point[term.slot]));
  return objective;
}
SolveOptions budget_options(const PresolveOptions& options) {
  SolveOptions result; result.time_limit_seconds = options.time_limit_seconds;
  result.cancellation = options.cancellation; return result;
}
}

namespace Detail {
struct PresolveBuilder {
  static std::shared_ptr<const PresolvedModel> build(const ModelSnapshot& source, const Data& data,
                                                    PresolveResult& output, const SolveBudget& budget) {
    Model reduced;
    if (reduced.id() == source.model_id) reduced = Model{};
    std::vector<PresolveVariableMap> variables;
    std::vector<PresolveRowMap> rows;
    for (std::size_t i = 0; i < source.variables.size(); ++i) {
      checkpoint(budget);
      const auto& original = source.variables[i];
      PresolveVariableMap mapping;
      mapping.original = original.variable; mapping.active = original.active;
      if (original.active) {
        mapping.lower = data.lower[i]; mapping.upper = data.upper[i];
        if (mapping.lower == mapping.upper) { mapping.fixed_value = mapping.lower; ++output.fixed_variables; }
        else mapping.reduced = reduced.add_variable(original.type, exported(mapping.lower), exported(mapping.upper), original.name);
      }
      variables.push_back(mapping);
    }
    for (std::size_t i = 0; i < data.rows.size(); ++i) {
      checkpoint(budget);
      const auto& row = data.rows[i];
      PresolveRowMap mapping; mapping.original = row.original; mapping.active = row.active;
      if (row.active) {
        if (row.redundant) { mapping.redundant = true; ++output.removed_rows; }
        else {
          Integer shift = 0;
          std::vector<Term> terms;
          for (const auto& term : row.terms) {
            checkpoint(budget);
            if (variables[term.slot].fixed_value) shift = add(shift, multiply(term.coefficient, *variables[term.slot].fixed_value));
            else terms.push_back({*variables[term.slot].reduced, exported(term.coefficient)});
          }
          const auto lower = row.lower ? std::optional<Integer>(subtract(*row.lower, shift)) : std::nullopt;
          const auto upper = row.upper ? std::optional<Integer>(subtract(*row.upper, shift)) : std::nullopt;
          mapping.substituted_constant = shift;
          if (terms.empty()) {
            if ((lower && *lower > 0) || (upper && *upper < 0)) throw Contradiction{row.original, {}};
            mapping.redundant = true; ++output.removed_rows;
          } else {
            mapping.reduced = reduced.add_row(terms, lower ? exported(*lower) : -infinity,
              upper ? exported(*upper) : infinity, source.rows[i].name);
          }
        }
      }
      rows.push_back(mapping);
    }
    Integer offset = data.offset;
    std::vector<Term> objective;
    for (const auto& term : data.objective) {
      checkpoint(budget);
      if (variables[term.slot].fixed_value) offset = add(offset, multiply(term.coefficient, *variables[term.slot].fixed_value));
      else objective.push_back({*variables[term.slot].reduced, exported(term.coefficient)});
    }
    reduced.set_objective(objective, source.objective.sense, exported(offset));
    auto snapshot = reduced.snapshot();
    validate_structure(snapshot);
    checkpoint(budget);
    return std::shared_ptr<const PresolvedModel>(new PresolvedModel(source, std::move(snapshot), std::move(variables), std::move(rows)));
  }
};
}

PresolvedModel::PresolvedModel(ModelSnapshot original, ModelSnapshot reduced,
    std::vector<PresolveVariableMap> variables, std::vector<PresolveRowMap> rows)
  : original_(std::move(original)), reduced_(std::move(reduced)), variables_(std::move(variables)), rows_(std::move(rows)) {}

PostsolveResult PresolvedModel::postsolve(const SolveResult& candidate, double tolerance) const {
  PostsolveResult output;
  auto& result = output.solution;
  result.model_id = original_.model_id; result.revision = original_.revision;
  try {
    if (!std::isfinite(tolerance) || tolerance < 0 || tolerance >= 0.5)
      throw ModelError("Postsolve integer tolerance must be finite and in [0,0.5)");
    if (candidate.model_id != reduced_.model_id || candidate.revision != reduced_.revision ||
        candidate.values.size() != reduced_.variables.size() || candidate.active_variables.size() != reduced_.variables.size())
      throw ModelError("Postsolve candidate has foreign identity, stale revision, or incorrect slot layout");
    if (candidate.guarantee == Guarantee::Certified)
      throw Unsupported("Postsolve does not transfer certificate guarantees");
    if (candidate.guarantee != Guarantee::Numerical && candidate.guarantee != Guarantee::Exact)
      throw ModelError("Unknown postsolve input guarantee");
    auto values = candidate.values;
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (candidate.active_variables[i] != reduced_.variables[i].active)
        throw ModelError("Postsolve candidate has an inconsistent active mask");
      if (!std::isfinite(values[i]) || std::fabs(values[i]) > exact_limit ||
          std::fabs(values[i]-std::round(values[i])) > tolerance)
        throw InvalidWitness("Postsolve candidate is not a finite integer assignment within tolerance");
      values[i] = std::round(values[i]);
    }
    const auto reduced_objective = exact_check(reduced_, values);
    std::vector<double> original_values(original_.variables.size(), std::numeric_limits<double>::quiet_NaN());
    for (const auto& mapping : variables_) if (mapping.active)
      original_values[mapping.original.id] = mapping.fixed_value ? exported(*mapping.fixed_value) : values[mapping.reduced->id];
    const auto original_objective = exact_check(original_, original_values);
    if (original_objective != reduced_objective) throw InvalidWitness("Postsolve objective does not agree under the owned transformation");
    result.objective = exported(original_objective);
    result.values = std::move(original_values);
    for (const auto& variable : original_.variables) result.active_variables.push_back(variable.active);
    result.guarantee = candidate.guarantee; result.backend = candidate.backend; result.backend_version = candidate.backend_version;
    result.solution_validated = true; result.termination = Termination::Unknown;
    result.message = "Exact original feasible witness reconstructed; no scalar optimum status or global bound transferred";
    output.exact_witness_validated = true;
  } catch (const Unsupported& error) { result.termination = Termination::Unsupported; result.message = error.what(); }
    catch (const InvalidWitness& error) { result.termination = Termination::NumericalFailure; result.message = error.what(); }
    catch (const ModelError& error) { result.termination = Termination::InvalidModel; result.message = error.what(); }
    catch (const std::bad_alloc&) { result.termination = Termination::MemoryLimit; result.message = "Allocation failed"; }
  if (!output.exact_witness_validated) result.solution_validated = false;
  return output;
}

namespace {
PresolveResult run(const ModelSnapshot& source, const PresolveOptions& options, SolveBudget& budget) {
  PresolveResult output; output.model_id = source.model_id; output.revision = source.revision;
  try {
    checkpoint(budget);
    auto data = parse(source, &budget);
    const bool fixed = propagate(data, options, output, budget);
    auto model = Detail::PresolveBuilder::build(source, data, output, budget);
    checkpoint(budget);
    output.model = std::move(model);
    output.status = fixed ? PresolveStatus::Fixpoint : PresolveStatus::Incomplete;
    output.termination = fixed ? Termination::Optimal : Termination::IterationLimit;
    output.message = fixed ? "Exact propagation reached its fixpoint; the owned reduction preserves the integer feasible set"
                           : "Propagation work limit reached; the finalized partial reduction is exactly equivalent";
  } catch (const Contradiction& proof) {
    output.status = PresolveStatus::Infeasible; output.termination = Termination::Infeasible;
    output.infeasible_row = proof.row; output.infeasible_variable = proof.variable;
    output.message = "Exact integer interval arithmetic establishes an original-model contradiction";
  } catch (const Interrupted& stopped) { output.status = PresolveStatus::Incomplete; output.termination = stopped.reason; }
    catch (const Unsupported& error) { output.status = PresolveStatus::Unsupported; output.termination = Termination::Unsupported; output.message = error.what(); }
    catch (const ModelError& error) { output.status = PresolveStatus::InvalidModel; output.termination = Termination::InvalidModel; output.message = error.what(); }
    catch (const std::bad_alloc&) { output.status = PresolveStatus::Error; output.termination = Termination::MemoryLimit; output.message = "Allocation failed"; }
    catch (const std::exception& error) { output.status = PresolveStatus::Error; output.termination = Termination::BackendError; output.message = error.what(); }
  if (output.status != PresolveStatus::Fixpoint &&
      !(output.status == PresolveStatus::Incomplete && output.termination == Termination::IterationLimit))
    output.model.reset();
  if (auto reason = budget.stop_reason()) {
    output.model.reset(); output.infeasible_row.reset(); output.infeasible_variable.reset();
    output.status = PresolveStatus::Incomplete; output.termination = *reason;
    output.message = "Presolve budget stopped; no new reduction or infeasibility result was published";
  }
  output.elapsed_seconds = budget.elapsed_seconds(); return output;
}
PresolveResult failure(ModelId id, Revision revision, Termination reason, const std::string& message) {
  PresolveResult output; output.model_id = id; output.revision = revision;
  output.status = reason == Termination::InvalidModel ? PresolveStatus::InvalidModel : PresolveStatus::Error;
  output.termination = reason; output.message = message; return output;
}
}
PresolveResult presolve_integer(const ModelSnapshot& model, const PresolveOptions& options) {
  try { SolveBudget budget(budget_options(options)); return run(model, options, budget); }
  catch (const ModelError& error) { return failure(model.model_id, model.revision, Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.model_id, model.revision, Termination::MemoryLimit, "Allocation failed"); }
}
PresolveResult presolve_integer(const Model& model, const PresolveOptions& options) {
  try { SolveBudget budget(budget_options(options)); return run(model.snapshot(), options, budget); }
  catch (const ModelError& error) { return failure(model.id(), model.revision(), Termination::InvalidModel, error.what()); }
  catch (const std::bad_alloc&) { return failure(model.id(), model.revision(), Termination::MemoryLimit, "Allocation failed"); }
}
}}
