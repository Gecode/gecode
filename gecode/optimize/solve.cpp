#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/native.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/lp_observations_detail.hpp>
#include <gecode/optimize/lp_basis_detail.hpp>

#include <algorithm>
#include <cmath>
#include <chrono>
#include <limits>
#include <stdexcept>
#include <utility>

#ifdef GECODE_OPTIMIZE_WITH_HIGHS
#include <Highs.h>
#include <gecode/optimize/lp_sensitivity_highs_detail.hpp>
#include <gecode/optimize/lp_sensitivity_backend.hpp>
#endif

namespace Gecode { namespace Optimize {
#ifdef GECODE_OPTIMIZE_TEST_LP_BASIS_FAILURE
namespace Detail { bool lp_basis_test_cancel() noexcept; }
#endif
namespace {

class Unsupported : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};
class BasisStopped : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

#ifdef GECODE_OPTIMIZE_WITH_HIGHS

void require_ok(HighsStatus status, const char* operation) {
  if (status != HighsStatus::kOk)
    throw ModelError(std::string("HiGHS rejected or modified input during ") + operation);
}

void numerical_range(double value, double maximum, const char* kind) {
  if (std::isfinite(value) && std::abs(value) >= maximum)
    throw Unsupported(std::string(kind) + " exceeds supported HiGHS numerical range");
}

HighsVarType variable_type(VariableType type) {
  switch (type) {
    case VariableType::Continuous: return HighsVarType::kContinuous;
    case VariableType::Integer:
    case VariableType::Binary: return HighsVarType::kInteger;
    case VariableType::SemiContinuous: return HighsVarType::kSemiContinuous;
    case VariableType::SemiInteger: return HighsVarType::kSemiInteger;
  }
  throw ModelError("Invalid variable type");
}

HighsBasisStatus basis_status(LpBasisStatus value) {
  switch(value) {
  case LpBasisStatus::Lower: return HighsBasisStatus::kLower;
  case LpBasisStatus::Basic: return HighsBasisStatus::kBasic;
  case LpBasisStatus::Upper: return HighsBasisStatus::kUpper;
  case LpBasisStatus::Zero: return HighsBasisStatus::kZero;
  case LpBasisStatus::NonbasicUnspecified: return HighsBasisStatus::kNonbasic;
  }
  throw ModelError("Unknown original basis status");
}
LpBasisStatus basis_status(HighsBasisStatus value) {
  switch(value) {
  case HighsBasisStatus::kLower: return LpBasisStatus::Lower;
  case HighsBasisStatus::kBasic: return LpBasisStatus::Basic;
  case HighsBasisStatus::kUpper: return LpBasisStatus::Upper;
  case HighsBasisStatus::kZero: return LpBasisStatus::Zero;
  case HighsBasisStatus::kNonbasic: return LpBasisStatus::NonbasicUnspecified;
  }
  throw std::runtime_error("Unknown returned backend basis status");
}

struct Compiled {
  HighsLp lp;
  std::vector<std::size_t> slots;
  std::vector<std::size_t> row_slots;
  // Unlike row_slots (all active-row topology), this maps emitted backend rows.
  std::vector<std::size_t> backend_row_slots;
  std::vector<VariableType> original_types;
  bool discrete = false;
  bool contradictory_constant = false;
};

Compiled compile(const ModelSnapshot& model, double feasibility_tolerance) {
  validate_structure(model);
  Compiled compiled;
  auto& lp = compiled.lp;
  std::vector<HighsInt> columns(model.variables.size(), -1);
  const auto limit = static_cast<std::size_t>(std::numeric_limits<HighsInt>::max());
  for (const auto& v : model.variables) {
    if (!v.active) continue;
    if (compiled.slots.size() >= limit) throw Unsupported("Too many columns for HiGHS");
    numerical_range(v.lower, 1e20, "Variable bound");
    numerical_range(v.upper, 1e20, "Variable bound");
    if ((v.type == VariableType::SemiContinuous || v.type == VariableType::SemiInteger)
        && (!std::isfinite(v.upper) || v.upper > 1e5))
      throw Unsupported("Semi variables require a finite upper bound <= 100000 in this adapter");
    columns[v.variable.id] = static_cast<HighsInt>(compiled.slots.size());
    compiled.slots.push_back(static_cast<std::size_t>(v.variable.id));
    compiled.original_types.push_back(v.type);
    lp.col_lower_.push_back(v.lower);
    lp.col_upper_.push_back(v.upper);
    lp.col_cost_.push_back(0.0);
    lp.col_names_.push_back(v.name.empty() ? "x" + std::to_string(v.variable.id) : v.name);
    lp.integrality_.push_back(variable_type(v.type));
    compiled.discrete |= v.type != VariableType::Continuous;
  }
  lp.num_col_ = static_cast<HighsInt>(compiled.slots.size());
  lp.sense_ = model.objective.sense == ObjectiveSense::Minimize
            ? ObjSense::kMinimize : ObjSense::kMaximize;
  numerical_range(model.objective.offset, 1e20, "Objective offset");
  lp.offset_ = model.objective.offset;
  for (const auto& term : model.objective.terms) {
    numerical_range(term.coefficient, 1e20, "Objective coefficient");
    lp.col_cost_[columns[term.variable.id]] = term.coefficient;
  }
  auto& matrix = lp.a_matrix_;
  matrix.format_ = MatrixFormat::kRowwise;
  matrix.start_.assign(1, 0);
  for (const auto& row : model.rows) {
    if (!row.active) continue;
    compiled.row_slots.push_back(static_cast<std::size_t>(row.constraint.id));
    if (lp.row_lower_.size() >= limit) throw Unsupported("Too many rows for HiGHS");
    numerical_range(row.lower, 1e20, "Row bound");
    numerical_range(row.upper, 1e20, "Row bound");
    if (row.terms.empty()) {
      // HiGHS MIP presolve requires a nonempty matrix when rows exist. Constant
      // rows need no backend column: decide them using the original checker's
      // absolute feasibility convention and retain them in the original model.
      compiled.contradictory_constant |= row.lower > feasibility_tolerance ||
        row.upper < -feasibility_tolerance;
      continue;
    }
    compiled.backend_row_slots.push_back(static_cast<std::size_t>(row.constraint.id));
    lp.row_lower_.push_back(row.lower);
    lp.row_upper_.push_back(row.upper);
    lp.row_names_.push_back(row.name.empty() ? "r" + std::to_string(row.constraint.id) : row.name);
    for (const auto& term : row.terms) {
      if (matrix.value_.size() >= limit) throw Unsupported("Too many nonzeros for HiGHS");
      // HiGHS discards coefficients at/below this minimum threshold. Refuse
      // such input rather than silently changing the original problem.
      if (std::abs(term.coefficient) <= 1e-12)
        throw Unsupported("Nonzero matrix coefficients <= 1e-12 require rescaling");
      numerical_range(term.coefficient, 1e15, "Matrix coefficient");
      matrix.index_.push_back(columns[term.variable.id]);
      matrix.value_.push_back(term.coefficient);
    }
    matrix.start_.push_back(static_cast<HighsInt>(matrix.value_.size()));
  }
  lp.num_row_ = static_cast<HighsInt>(lp.row_lower_.size());
  matrix.num_col_ = lp.num_col_;
  matrix.num_row_ = lp.num_row_;
  if (!compiled.discrete) lp.integrality_.clear();
  return compiled;
}

HighsBasis prepare_basis(const LpBasis& source,const Compiled& model) {
  HighsBasis out;
  // The pinned alien zero-row path indexes internal status vectors before its
  // general size checks. A fully checked basis with zero basic entities and
  // zero rows has no singularity issue and uses the non-alien consistency path.
  out.alien=!model.backend_row_slots.empty();out.valid=false;out.useful=true;
  out.col_status.reserve(model.slots.size());out.row_status.reserve(model.backend_row_slots.size());
  for(auto slot:model.slots)out.col_status.push_back(basis_status(*source.columns()[slot]));
  for(auto slot:model.backend_row_slots)out.row_status.push_back(basis_status(*source.rows()[slot]));
  return out;
}

void submit_basis(Highs& highs,const HighsBasis& requested,const Compiled& compiled,
                  const ModelSnapshot& model,const SolveBudget& budget,LpBasisSubmission& report) {
  report.backend_attempted=true;
  try {
    auto status=highs.setBasis(requested,"Gecode original LP basis start");
#ifdef GECODE_OPTIMIZE_TEST_LP_BASIS_FAILURE
    // Separately compiled fault panel only: dirty the REAL backend first, then
    // exercise rejection or cancellation cleanup. Absent in production builds.
    if(status==HighsStatus::kOk) {
      if(Detail::lp_basis_test_cancel())budget.cancellation()->cancel();
      else status=HighsStatus::kError;
    }
#endif
    if(budget.expired())throw BasisStopped("Budget expired during LP basis factorization/repair");
    if(status!=HighsStatus::kOk)throw std::runtime_error("HiGHS rejected the submitted LP basis");
    const auto& actual=highs.getBasis();
    if(!actual.valid||actual.alien||actual.col_status.size()!=compiled.slots.size()||
       actual.row_status.size()!=compiled.backend_row_slots.size())
      throw std::runtime_error("HiGHS returned an invalid or incomplete submitted basis");
    {
      std::vector<std::optional<LpBasisStatus>> rows(model.rows.size()),cols(model.variables.size());
      for(std::size_t i=0;i<compiled.slots.size();++i)cols[compiled.slots[i]]=basis_status(actual.col_status[i]);
      for(std::size_t i=0;i<compiled.backend_row_slots.size();++i)rows[compiled.backend_row_slots[i]]=basis_status(actual.row_status[i]);
      Detail::LpBasisAccess::validate_statuses(model,rows,cols,&budget);
    }
    if(budget.expired())throw BasisStopped("Budget expired checking/releasing submitted LP basis data");
    const bool changed=actual.col_status!=requested.col_status||actual.row_status!=requested.row_status;
    report.message=changed ? "HiGHS accepted basis with changed statuses during factorization/repair" :
      "HiGHS accepted submitted basis statuses before optimization";
    report.statuses_changed=changed;
    report.state=changed ? LpBasisSubmissionState::Repaired : LpBasisSubmissionState::Accepted;
  } catch(const std::bad_alloc&) {
    report.statuses_changed.reset();report.state=LpBasisSubmissionState::Rejected;
    throw;
  } catch(const std::exception& error) {
    report.statuses_changed.reset();
    if(budget.expired()) {
      report.state=LpBasisSubmissionState::Interrupted;
      report.message="LP basis submission exceeded the cooperative solve budget";
      throw BasisStopped(report.message);
    }
    report.state=LpBasisSubmissionState::Rejected;report.message=error.what();
    throw std::runtime_error(report.message);
  }
}

void configure(Highs& highs, const SolveOptions& options) {
  // HiGHS owns a process-wide scheduler. Keep this first adapter's concurrency
  // contract fixed until mixed worker-count sessions are implemented/tested.
  if (options.threads != 1)
    throw Unsupported("This adapter currently supports one HiGHS worker per solve");
  require_ok(highs.setOptionValue("output_flag", false), "output setup");
  require_ok(highs.setOptionValue("threads", options.threads), "thread setup");
  require_ok(highs.setOptionValue("random_seed", options.random_seed), "seed setup");
  require_ok(highs.setOptionValue("small_matrix_value", 1e-12), "matrix threshold setup");
  require_ok(highs.setOptionValue("mip_rel_gap", options.relative_gap), "relative gap setup");
  require_ok(highs.setOptionValue("mip_abs_gap", options.absolute_gap), "absolute gap setup");
  require_ok(highs.setOptionValue("primal_feasibility_tolerance", options.feasibility_tolerance), "LP tolerance setup");
  require_ok(highs.setOptionValue("mip_feasibility_tolerance", std::min(options.feasibility_tolerance, options.integrality_tolerance)), "MIP tolerance setup");
  if (options.node_limit) {
    if (*options.node_limit > static_cast<std::uint64_t>(std::numeric_limits<HighsInt>::max()))
      throw Unsupported("Node limit exceeds HiGHS integer range");
    require_ok(highs.setOptionValue("mip_max_nodes", static_cast<HighsInt>(*options.node_limit)), "node limit setup");
  } else {
    require_ok(highs.setOptionValue("mip_max_nodes", kHighsIInf), "node limit reset");
  }
}

HighsSolution prepare_start(const ModelSnapshot& model,
                           const Compiled& compiled, const SolveOptions& options) {
  std::vector<HighsInt> column(model.variables.size(), -1);
  for (std::size_t i=0; i<compiled.slots.size(); ++i)
    column[compiled.slots[i]] = static_cast<HighsInt>(i);
  std::vector<double> values(model.variables.size(), std::numeric_limits<double>::quiet_NaN());
  std::vector<HighsInt> indices;
  std::vector<double> entries;
  for (const auto& entry : options.primal_start) {
    const auto handle = entry.variable;
    if (handle.model_id != model.model_id || handle.id >= model.variables.size()
        || !model.variables[handle.id].active)
      throw ModelError("Primal start contains a foreign or deleted variable");
    if (!std::isnan(values[handle.id]))
      throw ModelError("Primal start contains a duplicate variable");
    const auto& v = model.variables[handle.id];
    const long double value = entry.value;
    const bool semi = v.type == VariableType::SemiContinuous || v.type == VariableType::SemiInteger;
    const bool zero = semi && std::abs(value) <= options.feasibility_tolerance;
    if (!zero && (value < static_cast<long double>(v.lower)-options.feasibility_tolerance
        || value > static_cast<long double>(v.upper)+options.feasibility_tolerance))
      throw ModelError("Primal start violates a variable bound");
    if (v.type != VariableType::Continuous && v.type != VariableType::SemiContinuous
        && std::abs(value-std::round(value)) > options.integrality_tolerance)
      throw ModelError("Primal start violates integrality");
    values[handle.id] = entry.value;
    indices.push_back(column[handle.id]);
    entries.push_back(entry.value);
  }
  if (entries.size() == compiled.slots.size()) {
    const auto checked = validate(model, values, options.feasibility_tolerance,
                                  options.integrality_tolerance);
    if (!checked.valid) throw ModelError("Complete primal start is infeasible: " + checked.message);
  }
  // HiGHS's sparse overload checks only the nonzero interval of semi domains,
  // incorrectly rejecting their valid zero alternative. Its dense overload
  // uses kHighsUndefined for unprovided values and handles the MIP hint normally.
  HighsSolution start;
  start.col_value.assign(compiled.slots.size(), kHighsUndefined);
  for (std::size_t i=0; i<indices.size(); ++i) start.col_value[indices[i]] = entries[i];
  return start;
}

Termination status(HighsModelStatus s) {
  switch (s) {
    case HighsModelStatus::kOptimal:
    case HighsModelStatus::kModelEmpty: return Termination::Optimal;
    case HighsModelStatus::kInfeasible: return Termination::Infeasible;
    case HighsModelStatus::kUnbounded: return Termination::Unbounded;
    case HighsModelStatus::kUnboundedOrInfeasible: return Termination::InfeasibleOrUnbounded;
    case HighsModelStatus::kTimeLimit: return Termination::TimeLimit;
    case HighsModelStatus::kIterationLimit: return Termination::IterationLimit;
    case HighsModelStatus::kSolutionLimit: return Termination::SolutionLimit;
    case HighsModelStatus::kObjectiveBound:
    case HighsModelStatus::kObjectiveTarget: return Termination::ObjectiveLimit;
    case HighsModelStatus::kInterrupt:
    case HighsModelStatus::kHighsInterrupt: return Termination::Cancelled;
    case HighsModelStatus::kMemoryLimit: return Termination::MemoryLimit;
    case HighsModelStatus::kModelError:
    case HighsModelStatus::kLoadError: return Termination::InvalidModel;
    case HighsModelStatus::kPresolveError:
    case HighsModelStatus::kSolveError:
    case HighsModelStatus::kPostsolveError: return Termination::BackendError;
    default: return Termination::Unknown;
  }
}

std::vector<double> original_values(const ModelSnapshot& model,
                                  const std::vector<std::size_t>& slots,
                                  const std::vector<double>& columns) {
  if (columns.size() != slots.size()) throw ModelError("HiGHS returned wrong solution dimension");
  std::vector<double> values(model.variables.size(), std::numeric_limits<double>::quiet_NaN());
  for (std::size_t i = 0; i < slots.size(); ++i) values[slots[i]] = columns[i];
  return values;
}

#endif

struct SessionState {
  SessionStatistics statistics;
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
  std::unique_ptr<Highs> highs;
  std::optional<Compiled> compiled;
  ModelId model_id = 0;
  std::optional<SolveResult> previous;
  void invalidate() noexcept {
    previous.reset();
    compiled.reset();
    highs.reset();
    model_id = 0;
  }
#endif
};

#ifdef GECODE_OPTIMIZE_WITH_HIGHS
bool compatible(const Compiled& a, const Compiled& b) {
  return a.slots == b.slots && a.row_slots == b.row_slots &&
    a.backend_row_slots == b.backend_row_slots &&
    a.original_types == b.original_types &&
    a.lp.integrality_ == b.lp.integrality_ &&
    a.lp.a_matrix_.start_ == b.lp.a_matrix_.start_ &&
    a.lp.a_matrix_.index_ == b.lp.a_matrix_.index_ &&
    a.lp.a_matrix_.value_ == b.lp.a_matrix_.value_;
}

bool prepare(SessionState& state, Compiled&& compiled, ModelId owner) {
  auto& highs = *state.highs;
  const bool reuse = state.compiled && state.model_id == owner &&
    compatible(*state.compiled, compiled);
  if (!reuse) {
    require_ok(highs.passModel(compiled.lp), "model load");
    state.previous.reset();
    ++state.statistics.model_loads;
  } else {
    const auto& old = state.compiled->lp;
    const auto& next = compiled.lp;
    bool changed = false;
    if (old.col_cost_ != next.col_cost_) {
      require_ok(highs.changeColsCost(0, next.num_col_-1, next.col_cost_.data()), "cost update");
      changed = true;
    }
    if (old.col_lower_ != next.col_lower_ || old.col_upper_ != next.col_upper_) {
      require_ok(highs.changeColsBounds(0, next.num_col_-1, next.col_lower_.data(),
                                      next.col_upper_.data()), "column bound update");
      changed = true;
    }
    if (old.row_lower_ != next.row_lower_ || old.row_upper_ != next.row_upper_) {
      require_ok(highs.changeRowsBounds(0, next.num_row_-1, next.row_lower_.data(),
                                      next.row_upper_.data()), "row bound update");
      changed = true;
    }
    if (old.sense_ != next.sense_) {
      require_ok(highs.changeObjectiveSense(next.sense_), "sense update");
      changed = true;
    }
    if (old.offset_ != next.offset_) {
      require_ok(highs.changeObjectiveOffset(next.offset_), "offset update");
      changed = true;
    }
    if (changed) ++state.statistics.incremental_updates;
    else ++state.statistics.unchanged_models;
  }
  state.compiled = std::move(compiled);
  state.model_id = owner;
  return reuse;
}

struct CallbackScope {
  SessionState& state;
  ~CallbackScope() noexcept {
    // A persistent backend must never retain references to a completed call's
    // budget or local vectors, including exception and early-return paths.
    try { state.highs->setCallback(HighsCallbackFunctionType{}, nullptr); }
    catch (...) { state.invalidate(); }
  }
};
#endif
} // namespace

#ifdef GECODE_OPTIMIZE_WITH_HIGHS
Detail::LpSensitivityCompiled Detail::compile_lp_sensitivity(
  const ModelSnapshot& model,double primal_tolerance) {
  try {
    auto compiled=compile(model,primal_tolerance);
    if(compiled.row_slots.size()!=compiled.backend_row_slots.size())
      throw LpSensitivityBackendError(LpSensitivityReason::NoBasis,
                                     "LP sensitivity cannot map elided constant rows");
    return {std::move(compiled.lp),std::move(compiled.slots),
            std::move(compiled.backend_row_slots)};
  } catch(const Unsupported& error) {
    throw LpSensitivityBackendError(LpSensitivityReason::Unsupported,error.what());
  }
}
#endif

struct SolveSession::Impl : SessionState {};

BackendCapabilities capabilities(Backend backend) {
  if (backend == Backend::Native) return native_capabilities();
  BackendCapabilities c;
  if (backend != Backend::Auto && backend != Backend::Highs) {
    c.limitations.push_back("Unknown backend");
    return c;
  }
  c.name = "HiGHS";
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
  c.available = c.linear_programming = c.mixed_integer_linear = true;
  c.version = Highs().version();
#else
  c.limitations.push_back("Built without HiGHS");
#endif
  c.limitations.push_back("Numerical linear LP/MILP only; no exact/certified or nonlinear solve");
  c.limitations.push_back("No action callbacks; sessions retain LP bases and revalidated MIP hints only");
  c.limitations.push_back("One HiGHS worker per solve in this initial adapter");
  c.limitations.push_back("Finite bounds/costs below 1e20; nonzero matrix magnitudes in (1e-12,1e15)");
  c.limitations.push_back("Time/cancellation is cooperative and may overrun during backend calls");
  return c;
}

static SolveResult solve_highs(const ModelSnapshot& model, const SolveOptions& options,
                              SessionState* persistent = nullptr,
                              const SolveBudget* shared_budget = nullptr,
                              Detail::LpBackendObservations* observations = nullptr,
                              const LpBasis* basis_start = nullptr,
                              LpBasisSubmission* basis_report = nullptr) {
#ifndef GECODE_OPTIMIZE_WITH_HIGHS
  (void)observations;
  (void)basis_start;(void)basis_report;
#endif
  const auto started = std::chrono::steady_clock::now();
  SolveResult result;
  result.model_id = model.model_id;
  result.revision = model.revision;
  result.backend = "HiGHS";
  // Start the budget before structural validation and conversion.
  std::optional<SolveBudget> budget_storage;
  try {
    if (shared_budget) budget_storage.emplace(*shared_budget);
    else budget_storage.emplace(options);
  }
  catch (const ModelError& e) {
    result.termination = Termination::InvalidModel;
    result.message = e.what();
    result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    return result;
  }
  catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit;
    result.message = "Budget allocation failed";
    result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    return result;
  }
  auto& budget = *budget_storage;
  SessionState local;
  auto& state = persistent ? *persistent : local;
#ifndef GECODE_OPTIMIZE_WITH_HIGHS
  (void)state;
#endif
  try {
    validate_structure(model);
    if (std::any_of(model.globals.begin(),model.globals.end(),[](const auto& record){return record.active;}))
      throw Unsupported("HiGHS linear adapter cannot preserve native global constraints; choose Native");
    if (options.guarantee != Guarantee::Numerical)
      throw Unsupported("HiGHS adapter cannot satisfy an Exact or Certified guarantee");
    result.active_variables.reserve(model.variables.size());
    for (const auto& variable : model.variables) result.active_variables.push_back(variable.active);
    if (budget.expired()) {
      result.termination = budget.stop_reason().value_or(Termination::Unknown);
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    // Complete source admission before mutating a retained backend instance.
    auto conversion = compile(model, options.feasibility_tolerance);
    std::optional<HighsSolution> explicit_start;
    if (!options.primal_start.empty()) explicit_start = prepare_start(model, conversion, options);
    std::optional<HighsBasis> external_basis;
    if (basis_start) external_basis=prepare_basis(*basis_start,conversion);
    if (budget.expired()) {
      result.termination = budget.stop_reason().value_or(Termination::Unknown);
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
    if (!state.highs) state.highs = std::make_unique<Highs>();
    auto& highs = *state.highs;
    result.backend_version = highs.version();
    configure(highs, options);
    if (budget.expired()) {
      result.termination = budget.stop_reason().value_or(Termination::Unknown);
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
    if (conversion.contradictory_constant) {
      state.invalidate();
      result.termination = Termination::Infeasible;
      result.message = "An original constant row violates the feasibility tolerance";
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
    if (conversion.slots.empty()) {
      if (!options.primal_start.empty())
        throw ModelError("A model without active variables cannot accept a primal start");
      // HiGHS's empty-model status does not check constant rows for us.
      auto values = original_values(model, conversion.slots, {});
      auto checked = validate(model, values, options.feasibility_tolerance, options.integrality_tolerance);
      state.invalidate();
      result.termination = checked.valid ? Termination::Optimal : Termination::Infeasible;
      if (budget.expired()) {
        result.termination = budget.stop_reason().value_or(Termination::Unknown);
        result.elapsed_seconds = budget.elapsed_seconds();
        return result;
      }
      if (checked.valid) {
        result.values = std::move(values);
        result.objective = checked.objective;
        result.best_bound = checked.objective;
        result.solution_validated = true;
        result.update_gaps(model.objective.sense);
      }
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
    const bool reused = prepare(state, std::move(conversion), model.model_id);
    const auto& compiled = *state.compiled;
    if (compiled.discrete)
      require_ok(highs.clearSolver(), "MIP search state reset");
    if (explicit_start) {
      require_ok(highs.setSolution(*explicit_start), "primal start submission");
      result.start_submitted = true;
    } else if (compiled.discrete && state.previous && state.previous->has_solution()) {
      const auto checked = validate(model, state.previous->values,
        options.feasibility_tolerance, options.integrality_tolerance);
      if (checked.valid) {
        HighsSolution hint;
        for (const auto slot : compiled.slots) hint.col_value.push_back(state.previous->values[slot]);
        require_ok(highs.setSolution(hint), "session incumbent submission");
        result.start_submitted = true;
        ++state.statistics.incumbent_starts;
      }
    }
    if (budget.expired()) {
      result.termination = budget.stop_reason().value_or(Termination::Unknown);
      result.elapsed_seconds = budget.elapsed_seconds();
      return result;
    }
    std::vector<double> timely_columns;
    std::optional<double> timely_bound;
    bool callback_failed = false;
    auto callback = [&](int type, const std::string&, const HighsCallbackOutput* out,
                        HighsCallbackInput* in, void*) {
      const bool can_interrupt = type == kCallbackSimplexInterrupt ||
        type == kCallbackIpmInterrupt || type == kCallbackMipInterrupt;
      try {
        // Improving-solution callbacks are observations only. HiGHS asserts
        // that they never request actions, including user_interrupt.
        if (budget.expired() || callback_failed) {
          if (in && can_interrupt) in->user_interrupt = true;
          return;
        }
        if (!out) return;
        if (type == kCallbackMipImprovingSolution && out->mip_solution.size() == compiled.slots.size())
          timely_columns = out->mip_solution;
        if (compiled.discrete && (type == kCallbackMipInterrupt || type == kCallbackMipImprovingSolution)
            && std::isfinite(out->mip_dual_bound)) timely_bound = out->mip_dual_bound;
      } catch (...) {
        callback_failed = true;
        if (in && can_interrupt) in->user_interrupt = true;
      }
    };
    CallbackScope callback_scope{state};
    require_ok(highs.setCallback(callback, nullptr), "callback setup");
    for (auto type : {kCallbackSimplexInterrupt, kCallbackIpmInterrupt,
                      kCallbackMipInterrupt, kCallbackMipImprovingSolution})
      require_ok(highs.startCallback(type), "callback activation");
    highs.zeroAllClocks();
    require_ok(highs.setOptionValue("time_limit", budget.remaining_seconds()), "time limit setup");
    if (external_basis) {
      if (budget.expired()) throw BasisStopped("Budget expired before LP basis submission");
      submit_basis(highs,*external_basis,compiled,model,budget,*basis_report);
      if (budget.expired()) {
        basis_report->state=LpBasisSubmissionState::Interrupted;
        basis_report->statuses_changed.reset();
        throw BasisStopped("Budget expired before LP basis submission returned");
      }
      highs.zeroAllClocks();
      require_ok(highs.setOptionValue("time_limit",budget.remaining_seconds()),"post-basis time limit setup");
    }
    if (!external_basis && reused && !compiled.discrete && highs.getBasis().valid)
      ++state.statistics.basis_warm_starts;
    const auto run_status = highs.run();
    const bool returned_in_budget = !budget.expired();
    result.termination = status(highs.getModelStatus());
    result.message = highs.modelStatusToString(highs.getModelStatus());
    if (!returned_in_budget) result.termination = budget.stop_reason().value_or(Termination::Unknown);
    const auto& info = highs.getInfo();
    if (result.termination == Termination::SolutionLimit && options.node_limit && info.valid
        && info.mip_node_count >= 0 && static_cast<std::uint64_t>(info.mip_node_count) >= *options.node_limit)
      result.termination = Termination::NodeLimit;
    if (run_status == HighsStatus::kError || callback_failed) {
      result.termination = Termination::BackendError;
      result.message = callback_failed ? "Callback storage failed" : "HiGHS solve failed";
    }
    const auto& candidate = highs.getSolution();
    if (observations) {
      // Passive copies only. In particular, do not call ray/ranging/tableau
      // methods here: those can solve or mutate the backend.
      auto& raw=*observations;
      raw.attempted=true;raw.timely=returned_in_budget;
      raw.model_id=model.model_id;raw.revision=model.revision;
      raw.info_valid=info.valid;raw.value_valid=candidate.value_valid;
      raw.primal_feasible=info.valid && info.primal_solution_status == kSolutionStatusFeasible;
      raw.dual_valid=candidate.dual_valid;
      raw.dual_feasible=info.valid && info.dual_solution_status == kSolutionStatusFeasible;
      try {
        if (returned_in_budget) {
          raw.column_slots=compiled.slots;raw.row_slots=compiled.backend_row_slots;
          double tolerance=0;
          if (highs.getOptionValue("primal_feasibility_tolerance",tolerance) == HighsStatus::kOk)
            raw.primal_tolerance=tolerance;
          if (highs.getOptionValue("dual_feasibility_tolerance",tolerance) == HighsStatus::kOk)
            raw.dual_tolerance=tolerance;
          if (raw.requested_duals && raw.dual_valid) {
            raw.column_duals=candidate.col_dual;raw.row_duals=candidate.row_dual;
          }
          if (raw.requested_basis) {
            const auto& basis=highs.getBasis();
            raw.basis_valid=basis.valid;
            raw.info_basis_valid=info.valid && info.basis_validity == kBasisValidityValid;
            auto convert_basis=[](HighsBasisStatus value) {
              switch (value) {
              case HighsBasisStatus::kLower: return LpBasisStatus::Lower;
              case HighsBasisStatus::kBasic: return LpBasisStatus::Basic;
              case HighsBasisStatus::kUpper: return LpBasisStatus::Upper;
              case HighsBasisStatus::kZero: return LpBasisStatus::Zero;
              case HighsBasisStatus::kNonbasic: return LpBasisStatus::NonbasicUnspecified;
              }
              throw ModelError("Unknown HiGHS basis status");
            };
            if (basis.valid) {
              raw.column_basis.reserve(basis.col_status.size());
              raw.row_basis.reserve(basis.row_status.size());
              for (auto value : basis.col_status) raw.column_basis.push_back(convert_basis(value));
              for (auto value : basis.row_status) raw.row_basis.push_back(convert_basis(value));
            }
          }
          raw.complete=true;
        }
      } catch (const std::bad_alloc&) { raw.failure=Detail::LpCaptureFailure::Allocation; }
      catch (...) { raw.failure=Detail::LpCaptureFailure::InvalidData; }
      // An observation copy cannot turn a late primal into a timely incumbent.
      if (budget.expired()) raw.timely=false;
    }
    std::vector<double> columns;
    if (returned_in_budget && candidate.value_valid && info.valid
        && info.primal_solution_status == kSolutionStatusFeasible)
      columns = candidate.col_value;
    else if (!timely_columns.empty()) columns = std::move(timely_columns);
    if (!columns.empty()) {
      auto values = original_values(model, compiled.slots, columns);
      const auto checked = validate(model, values, options.feasibility_tolerance, options.integrality_tolerance);
      if (checked.valid) {
        result.values = std::move(values);
        result.objective = checked.objective;
        result.solution_validated = true;
      } else {
        result.termination = Termination::NumericalFailure;
        result.message = "Original-model validation rejected HiGHS candidate: " + checked.message;
      }
    }
    if (returned_in_budget && info.valid && compiled.discrete) {
      if (std::isfinite(info.mip_dual_bound)) result.best_bound = info.mip_dual_bound;
      if (std::isfinite(info.mip_gap)) result.native_backend_gap = info.mip_gap;
    } else if (returned_in_budget && result.termination == Termination::Optimal && info.valid
               && std::isfinite(info.objective_function_value)) {
      result.best_bound = info.objective_function_value;
    } else if (timely_bound) result.best_bound = timely_bound;
    if (result.termination == Termination::Optimal && !result.has_solution()) {
      result.termination = Termination::NumericalFailure;
      result.message = "HiGHS reported optimal without a validated original-model solution";
    }
    try { result.update_gaps(model.objective.sense); }
    catch (const ModelError&) {
      // Preserve the observed numerical discrepancy, never manufacture zero.
      result.best_bound.reset();
      result.absolute_gap.reset();
      result.relative_gap.reset();
      result.message += "; bound inconsistent with independently recomputed objective";
    }
    state.previous.reset();
    if (persistent && compiled.discrete && result.has_solution()) {
      // A cache is optional: allocation pressure must not erase a valid result.
      try { state.previous.emplace(result); }
      catch (const std::bad_alloc&) { state.previous.reset(); }
    }
#else
    throw Unsupported("This build has no HiGHS backend; configure GECODE_OPTIMIZE_WITH_HIGHS=ON");
#endif
  } catch (const BasisStopped& e) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    state.invalidate();
#endif
    result.termination=budget.stop_reason().value_or(Termination::Unknown);
    result.message=e.what();
  } catch (const Unsupported& e) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    state.invalidate();
#endif
    result.termination = Termination::Unsupported;
    result.message = e.what();
  } catch (const ModelError& e) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    state.invalidate();
#endif
    result.termination = Termination::InvalidModel;
    result.message = e.what();
  } catch (const std::bad_alloc&) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    state.invalidate();
#endif
    result.termination = Termination::MemoryLimit;
    result.message = "Allocation failed";
  } catch (const std::exception& e) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
    state.invalidate();
#endif
    result.termination = Termination::BackendError;
    result.message = e.what();
  }
  result.elapsed_seconds = budget.elapsed_seconds();
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
  if (result.termination == Termination::BackendError || result.termination == Termination::NumericalFailure)
    state.invalidate();
#endif
  if (result.termination == Termination::Optimal && budget.expired())
    result.termination = budget.stop_reason().value_or(Termination::Unknown);
  return result;
}

template<class Snapshot>
static LpObservedResult observed_solve(Snapshot&& snapshot, ModelId owner,
                                      Revision revision,
                                      const LpObservationOptions& options,
                                      SessionState* persistent = nullptr,
                                      bool session_live = true,
                                      const LpBasisSolveOptions* basis_options = nullptr,
                                      LpBasisSubmission* basis_report = nullptr) {
#ifndef GECODE_OPTIMIZE_WITH_HIGHS
  (void)persistent;
#endif
  const auto started=std::chrono::steady_clock::now();
  LpObservedResult out;
  out.result.model_id=owner;out.result.revision=revision;out.result.backend="HiGHS";
  std::shared_ptr<LpObservations> observations;
  auto failure=[&](Termination termination,LpObservationReason reason,const char* message) noexcept {
    out.result.termination=termination;
    try {
      out.result.message=message;
      if (observations) Detail::LpObservationAccess::unavailable(*observations,reason,message);
    } catch (...) {
      observations.reset();out.observations.reset();out.result.message.clear();
    }
  };
  try {
    // The same shared clock covers cold snapshot ownership, backend work and
    // independent observation checks. Never restart it for collection.
    SolveBudget budget(options.solve);
    options.checks.validate();
    if (basis_options) basis_options->validate();
    out.result.guarantee=options.solve.guarantee;
    observations=Detail::LpObservationAccess::create(snapshot(),options);
    out.observations=observations;
    const auto& original=observations->source();
    out.result.active_variables.reserve(original.variables.size());
    for (const auto& variable : original.variables) out.result.active_variables.push_back(variable.active);
    const auto unsupported=Detail::LpObservationAccess::unsupported(original,options.solve);
    if (!session_live) {
      failure(Termination::InvalidModel,LpObservationReason::InvalidModel,"Moved-from solve session");
    } else if (!unsupported.empty()) {
      // Admission must precede any persistent backend/statistics mutation.
      failure(Termination::Unsupported,LpObservationReason::Unsupported,unsupported.c_str());
    } else {
      if (basis_options && !budget.expired()) {
        try { Detail::LpBasisAccess::compatible(*basis_options->basis,original,budget); }
        catch (const ModelError&) { if (!budget.expired()) throw; }
      }
#ifndef GECODE_OPTIMIZE_WITH_HIGHS
      failure(Termination::Unsupported,LpObservationReason::Unsupported,"This build has no HiGHS LP observation backend");
#else
      if (budget.expired()) {
        failure(budget.stop_reason().value_or(Termination::Unknown),LpObservationReason::Interrupted,
                "Budget expired before LP observation solve");
      } else {
        {
          Detail::LpBackendObservations raw;
          raw.requested_duals=options.duals;raw.requested_basis=options.basis;
          if (persistent) ++persistent->statistics.solve_calls;
          out.result=solve_highs(original,options.solve,persistent,&budget,&raw,
                                basis_options ? basis_options->basis.get() : nullptr,basis_report);
          if (out.result.termination == Termination::Unsupported)
            Detail::LpObservationAccess::unavailable(*observations,LpObservationReason::Unsupported,out.result.message);
          else if (out.result.termination == Termination::InvalidModel)
            Detail::LpObservationAccess::unavailable(*observations,LpObservationReason::InvalidModel,out.result.message);
          else
            Detail::LpObservationAccess::finish(*observations,out.result,raw,budget);
        }
        // Raw vector release is part of the same cooperative call budget.
        Detail::LpObservationAccess::final_budget(out,observations,budget);
      }
#endif
    }
  } catch (const ModelError& error) {
    failure(Termination::InvalidModel,LpObservationReason::InvalidModel,error.what());
  } catch (const std::bad_alloc&) {
    failure(Termination::MemoryLimit,LpObservationReason::AllocationFailure,"LP observation allocation failed");
  } catch (const std::exception& error) {
    failure(Termination::BackendError,LpObservationReason::InvalidBackendData,error.what());
  }
  out.result.elapsed_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
  return out;
}

LpObservedResult solve_lp_observed(const ModelSnapshot& model,const LpObservationOptions& options) {
  return observed_solve([&] {return model;},model.model_id,model.revision,options);
}
LpObservedResult solve_lp_observed(const Model& model,const LpObservationOptions& options) {
  return observed_solve([&] {return model.snapshot();},model.id(),model.revision(),options);
}

template<class Snapshot>
static LpBasisSolveResult basis_solve(Snapshot&& snapshot,ModelId owner,Revision revision,
    const LpBasisSolveOptions& options,SessionState* session=nullptr,bool live=true) {
  LpBasisSolveResult out;out.requested_basis=options.basis;
  out.observed=observed_solve(std::forward<Snapshot>(snapshot),owner,revision,
    options.observations,session,live,&options,&out.submission);
  return out;
}
LpBasisSolveResult solve_lp_with_basis(const ModelSnapshot& model,const LpBasisSolveOptions& options) {
  return basis_solve([&]{return model;},model.model_id,model.revision,options);
}
LpBasisSolveResult solve_lp_with_basis(const Model& model,const LpBasisSolveOptions& options) {
  return basis_solve([&]{return model.snapshot();},model.id(),model.revision(),options);
}

SolveResult solve(const ModelSnapshot& model, const SolveOptions& options) {
  if (options.backend == Backend::Native || (options.backend == Backend::Auto &&
      std::any_of(model.globals.begin(),model.globals.end(),[](const auto& record){return record.active;})))
    return solve_native_auto(model, options);
  return solve_highs(model, options);
}

SolveResult solve(const Model& model, const SolveOptions& options) {
  if (options.backend == Backend::Native) return solve_native_auto(model, options);
  const auto start = std::chrono::steady_clock::now();
  SolveResult result;
  result.model_id = model.id();
  result.revision = model.revision();
  result.backend = "HiGHS";
  try {
    auto snapshot = model.snapshot();
    auto adjusted = options;
    const double copy_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    if (std::isfinite(adjusted.time_limit_seconds) && adjusted.time_limit_seconds >= 0)
      adjusted.time_limit_seconds = std::max(0.0, adjusted.time_limit_seconds-copy_seconds);
    result = solve(snapshot, adjusted);
  } catch (const ModelError& error) {
    result.termination = Termination::InvalidModel;
    result.message = error.what();
  } catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit;
    result.message = "Snapshot/options allocation failed";
  }
  result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
  return result;
}

SolveSession::SolveSession() : impl_(std::make_unique<Impl>()) {}
SolveSession::~SolveSession() = default;
SolveSession::SolveSession(SolveSession&&) noexcept = default;
SolveSession& SolveSession::operator=(SolveSession&&) noexcept = default;

SessionStatistics SolveSession::statistics() const noexcept {
  return impl_ ? impl_->statistics : SessionStatistics{};
}

void SolveSession::reset() { impl_ = std::make_unique<Impl>(); }

SolveResult SolveSession::solve(const ModelSnapshot& model, const SolveOptions& options) {
  if (!impl_ || options.backend == Backend::Native) {
    SolveResult result;
    result.model_id = model.model_id;
    result.revision = model.revision;
    result.backend = "HiGHS";
    result.termination = impl_ ? Termination::Unsupported : Termination::InvalidModel;
    result.message = impl_ ? "Persistent Native sessions are not implemented" : "Moved-from solve session";
    return result;
  }
  ++impl_->statistics.solve_calls;
  return solve_highs(model, options, impl_.get());
}

SolveResult SolveSession::solve(const Model& model, const SolveOptions& options) {
  const auto started = std::chrono::steady_clock::now();
  SolveResult result;
  result.model_id = model.id();
  result.revision = model.revision();
  result.backend = "HiGHS";
  try {
    auto snapshot = model.snapshot();
    auto adjusted = options;
    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
    if (std::isfinite(adjusted.time_limit_seconds) && adjusted.time_limit_seconds >= 0)
      adjusted.time_limit_seconds = std::max(0.0, adjusted.time_limit_seconds-elapsed);
    result = solve(snapshot, adjusted);
  } catch (const ModelError& error) {
    result.termination = Termination::InvalidModel;
    result.message = error.what();
  } catch (const std::bad_alloc&) {
    result.termination = Termination::MemoryLimit;
    result.message = "Snapshot/options allocation failed";
  }
  result.elapsed_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
  return result;
}

LpObservedResult SolveSession::solve_lp_observed(const ModelSnapshot& model,
                                                const LpObservationOptions& options) {
  return observed_solve([&] {return model;},model.model_id,model.revision,options,
                        impl_.get(),static_cast<bool>(impl_));
}
LpObservedResult SolveSession::solve_lp_observed(const Model& model,
                                                const LpObservationOptions& options) {
  return observed_solve([&] {return model.snapshot();},model.id(),model.revision(),options,
                        impl_.get(),static_cast<bool>(impl_));
}
LpBasisSolveResult SolveSession::solve_lp_with_basis(const ModelSnapshot& model,
                                                    const LpBasisSolveOptions& options) {
  return basis_solve([&]{return model;},model.model_id,model.revision,options,
                     impl_.get(),static_cast<bool>(impl_));
}
LpBasisSolveResult SolveSession::solve_lp_with_basis(const Model& model,
                                                    const LpBasisSolveOptions& options) {
  return basis_solve([&]{return model.snapshot();},model.id(),model.revision(),options,
                     impl_.get(),static_cast<bool>(impl_));
}

}}
