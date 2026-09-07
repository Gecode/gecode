#include <gecode/optimize/lp_observations_detail.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Gecode { namespace Optimize {
namespace {
using Wide = long double;
struct Interrupted {};
struct BadObservation : std::runtime_error {
  using std::runtime_error::runtime_error;
};
void tick(const SolveBudget& budget) {
  if (budget.expired()) throw Interrupted{};
}
void finite(double value, const char* message) {
  if (!std::isfinite(value)) throw BadObservation(message);
}
double narrow(Wide value) {
  if (!std::isfinite(value) ||
      std::abs(value) > static_cast<Wide>(std::numeric_limits<double>::max()))
    throw BadObservation("Original observation arithmetic exceeds finite double range");
  const double result = static_cast<double>(value);
  if (!std::isfinite(result))
    throw BadObservation("Nonfinite original observation arithmetic");
  return result;
}
struct Sum {
  Wide sum = 0, correction = 0;
  void add(Wide value) {
    if (!std::isfinite(value)) throw BadObservation("Nonfinite observation product");
    const Wide next = sum + value;
    if (!std::isfinite(next)) throw BadObservation("Observation sum overflow");
    correction += std::abs(sum) >= std::abs(value)
      ? (sum-next)+value : (value-next)+sum;
    if (!std::isfinite(correction)) throw BadObservation("Observation compensation overflow");
    sum = next;
  }
  Wide value() const {
    const Wide result = sum+correction;
    if (!std::isfinite(result)) throw BadObservation("Observation sum overflow");
    return result;
  }
};
Wide difference(Wide value,double bound,const Sum* accumulated=nullptr) {
  Sum result;
  if (accumulated) {
    result.add(accumulated->sum);result.add(accumulated->correction);
  } else result.add(value);
  result.add(-static_cast<Wide>(bound));
  return result.value();
}
LpObservationGroup group(LpObservationState state, LpObservationReason reason,
                         const std::string& message) {
  return {state,reason,message};
}
LpObservationGroup available() {
  return group(LpObservationState::Available,LpObservationReason::None,"");
}
void not_requested(LpObservationGroup& out) {
  out = group(LpObservationState::NotRequested,LpObservationReason::NotRequested,"");
}
void unavailable_group(LpObservationGroup& out, LpObservationReason reason,
                       const std::string& message) {
  if (out.state == LpObservationState::NotRequested) return;
  out = group(LpObservationState::Unavailable,reason,message);
}
void rejected_group(LpObservationGroup& out, LpObservationReason reason,
                    const std::string& message) {
  if (out.state == LpObservationState::NotRequested) return;
  out = group(LpObservationState::Rejected,reason,message);
}
bool supported_basis(LpBasisStatus value) {
  switch (value) {
  case LpBasisStatus::Lower: case LpBasisStatus::Basic:
  case LpBasisStatus::Upper: case LpBasisStatus::Zero:
  case LpBasisStatus::NonbasicUnspecified: return true;
  }
  return false;
}
void check_basis_position(LpBasisStatus status, Wide value,
                          double lower, double upper, double tolerance,
                          const Sum* accumulated=nullptr) {
  if (!supported_basis(status)) throw BadObservation("Unknown backend basis status");
  switch (status) {
  case LpBasisStatus::Lower:
    if (!std::isfinite(lower) || std::abs(difference(value,lower,accumulated)) > tolerance)
      throw BadObservation("Lower basis status disagrees with original activity/bound");
    break;
  case LpBasisStatus::Upper:
    if (!std::isfinite(upper) || std::abs(difference(value,upper,accumulated)) > tolerance)
      throw BadObservation("Upper basis status disagrees with original activity/bound");
    break;
  case LpBasisStatus::Zero:
    if (std::isfinite(lower) || std::isfinite(upper) || std::abs(value) > tolerance)
      throw BadObservation("Zero basis status requires a free nonbasic entity at zero");
    break;
  case LpBasisStatus::NonbasicUnspecified:
    if (!(std::isfinite(lower) && std::abs(difference(value,lower,accumulated)) <= tolerance) &&
        !(std::isfinite(upper) && std::abs(difference(value,upper,accumulated)) <= tolerance) &&
        !(!std::isfinite(lower) && !std::isfinite(upper) && std::abs(value) <= tolerance))
      throw BadObservation("Unspecified nonbasic entity is not at an original nonbasic position");
    break;
  case LpBasisStatus::Basic: break;
  }
}
Wide sign_violation(Wide multiplier, Wide value, double lower, double upper,
                    double primal_tolerance,const Sum* accumulated) {
  const bool at_lower = std::isfinite(lower) &&
    difference(value,lower,accumulated) <= primal_tolerance;
  const bool at_upper = std::isfinite(upper) &&
    -difference(value,upper,accumulated) <= primal_tolerance;
  if (at_lower && at_upper) return 0;
  if (at_lower) return std::max(-multiplier,Wide(0));
  if (at_upper) return std::max(multiplier,Wide(0));
  return std::abs(multiplier);
}
void check_result_layout(const ModelSnapshot& model, const SolveResult& result) {
  if (result.model_id != model.model_id || result.revision != model.revision)
    throw BadObservation("Result identity/revision differs from observation source");
  if (result.active_variables.size() != model.variables.size())
    throw BadObservation("Result active mask has the wrong original dimension");
  for (std::size_t i=0;i<model.variables.size();++i)
    if (result.active_variables[i] != model.variables[i].active)
      throw BadObservation("Result active mask differs from observation source");
}
void check_mapping(const ModelSnapshot& model,
                   const Detail::LpBackendObservations& raw,
                   const SolveBudget& budget) {
  if (raw.model_id != model.model_id || raw.revision != model.revision)
    throw BadObservation("Backend observation identity/revision differs from source");
  std::size_t column=0,row=0;
  for (std::size_t i=0;i<model.variables.size();++i) {
    if (!(i%256)) tick(budget);
    if (!model.variables[i].active) continue;
    if (column >= raw.column_slots.size() || raw.column_slots[column++] != i)
      throw BadObservation("Backend column map differs from original live slots");
  }
  if (column != raw.column_slots.size()) throw BadObservation("Extra backend column map entries");
  for (std::size_t i=0;i<model.rows.size();++i) {
    if (!(i%256)) tick(budget);
    if (!model.rows[i].active || model.rows[i].terms.empty()) continue;
    if (row >= raw.row_slots.size() || raw.row_slots[row++] != i)
      throw BadObservation("Backend row map differs from retained original rows");
  }
  if (row != raw.row_slots.size()) throw BadObservation("Extra backend row map entries");
}

LpKktReport check_duals(const ModelSnapshot& model, const SolveResult& result,
                       const std::vector<Sum>& activities,
                       const Detail::LpBackendObservations& raw,
                       const LpObservationMetadata& metadata,
                       const SolveBudget& budget) {
  if (raw.column_duals.size() != raw.column_slots.size() ||
      raw.row_duals.size() != raw.row_slots.size())
    throw BadObservation("Backend dual vector dimension mismatch");
  const auto& tolerance = metadata.checks;
  const Wide sense = model.objective.sense == ObjectiveSense::Minimize ? 1 : -1;
  std::vector<Sum> transpose(model.variables.size());
  std::vector<double> costs(model.variables.size(),0);
  std::vector<double> row_duals(model.rows.size(),0);
  for (const auto& term : model.objective.terms) costs[term.variable.id] = term.coefficient;
  for (std::size_t i=0;i<raw.row_slots.size();++i) {
    finite(raw.row_duals[i],"Nonfinite backend row dual");
    row_duals[raw.row_slots[i]] = raw.row_duals[i];
  }
  Sum dual_terms, gap;
  Wide max_sign=0,max_stationarity=0,max_complementarity=0;
  bool finite_dual=true;
  auto endpoint = [&](Wide multiplier,Wide value,double lower,double upper,const Sum* accumulated) {
    max_sign = std::max(max_sign,sign_violation(multiplier,value,lower,upper,
                                               metadata.primal_check_tolerance,accumulated));
    if (multiplier == 0) return;
    const double bound = multiplier > 0 ? lower : upper;
    if (!std::isfinite(bound)) { finite_dual=false; return; }
    dual_terms.add(multiplier*static_cast<Wide>(bound));
    gap.add(-multiplier*static_cast<Wide>(bound));
    const Wide complementarity = multiplier*difference(value,bound,accumulated);
    if (!std::isfinite(complementarity))
      throw BadObservation("Complementarity product overflow");
    max_complementarity = std::max(max_complementarity,std::abs(complementarity));
  };
  std::size_t work=0;
  for (std::size_t i=0;i<model.rows.size();++i) {
    if (!(i%256)) tick(budget);
    const auto& row = model.rows[i];
    if (!row.active) continue;
    const Wide alpha = sense*row_duals[i];
    endpoint(alpha,activities[i].value(),row.lower,row.upper,&activities[i]);
    for (const auto& term : row.terms) {
      if (!(work++%256)) tick(budget);
      transpose[term.variable.id].add(alpha*term.coefficient);
    }
  }
  for (std::size_t j=0;j<raw.column_slots.size();++j) {
    if (!(j%256)) tick(budget);
    const std::size_t i=raw.column_slots[j];
    const auto& variable=model.variables[i];
    finite(raw.column_duals[j],"Nonfinite backend reduced cost");
    const Wide beta=sense*raw.column_duals[j], value=result.values[i];
    Sum stationarity;
    stationarity.add(sense*costs[i]);
    // Add the uncollapsed components to preserve large cancellation.
    stationarity.add(-transpose[i].sum);
    stationarity.add(-transpose[i].correction);
    stationarity.add(-beta);
    max_stationarity=std::max(max_stationarity,std::abs(stationarity.value()));
    endpoint(beta,value,variable.lower,variable.upper,nullptr);
    gap.add(sense*static_cast<Wide>(costs[i])*value);
  }
  LpKktReport out;
  out.primal_valid=true;
  out.max_dual_sign_violation=narrow(max_sign);
  out.max_stationarity=narrow(max_stationarity);
  out.dual_signs_valid=max_sign <= tolerance.dual_feasibility;
  out.stationarity_valid=max_stationarity <= tolerance.stationarity;
  if (finite_dual) {
    out.max_complementarity=narrow(max_complementarity);
    out.complementarity_valid=max_complementarity <= tolerance.complementarity;
    Sum objective;
    objective.add(model.objective.offset);
    objective.add(sense*dual_terms.sum);
    objective.add(sense*dual_terms.correction);
    out.dual_objective_estimate=narrow(objective.value());
    const Wide normalized_gap=gap.value();
    out.normalized_gap=narrow(normalized_gap);
    out.gap_valid=std::abs(normalized_gap) <= tolerance.objective_gap;
  }
  out.accepted=out.dual_signs_valid && out.stationarity_valid &&
    out.complementarity_valid && out.gap_valid;
  if (!finite_dual) out.message="A nonzero multiplier requires an infinite endpoint";
  else if (!out.accepted) out.message="Original numerical LP KKT checks failed";
  tick(budget);
  return out;
}
}

void LpCheckTolerances::validate() const {
  for (double value : {dual_feasibility,stationarity,complementarity,objective_gap})
    if (!std::isfinite(value) || value < 0)
      throw ModelError("LP check tolerances must be finite and nonnegative");
}
void LpObservationOptions::validate() const { solve.validate(); checks.validate(); }
const LpRowObservation& LpObservations::row(Constraint row) const {
  if (!id() || row.model_id != id() || row.id >= rows_.size() || !rows_[row.id].active)
    throw ModelError("Observation row is foreign, absent or deleted");
  return rows_[row.id];
}
const LpColumnObservation& LpObservations::column(Variable column) const {
  if (!id() || column.model_id != id() || column.id >= columns_.size() || !columns_[column.id].active)
    throw ModelError("Observation variable is foreign, absent or deleted");
  return columns_[column.id];
}
LpObservationCapabilities lp_observation_capabilities() {
  const auto backend=capabilities(Backend::Highs);
  LpObservationCapabilities out;
  out.available=out.duals=out.basis_export=backend.available;
  out.backend=backend.name;out.backend_version=backend.version;
  out.limitations={"Ordinary Continuous linear models only; no active indicators/globals",
    "Numerical observations only; duals require a timely optimal primal/dual point",
    "Basis unavailable after adapter constant-row elision or without a vendor basis",
    "No basis submission, rays, ranging or sensitivity analysis"};
  return out;
}

namespace Detail {
std::shared_ptr<LpObservations> LpObservationAccess::create(
    ModelSnapshot model, const LpObservationOptions& options) {
  options.validate();
  validate_structure(model);
  auto out=std::shared_ptr<LpObservations>(new LpObservations);
  out->source_=std::move(model);
  out->metadata_.checks=options.checks;
  out->metadata_.primal_check_tolerance=options.solve.feasibility_tolerance;
  out->rows_.resize(out->source_.rows.size());
  out->columns_.resize(out->source_.variables.size());
  for (std::size_t i=0;i<out->rows_.size();++i) out->rows_[i].active=out->source_.rows[i].active;
  for (std::size_t i=0;i<out->columns_.size();++i) out->columns_[i].active=out->source_.variables[i].active;
  if (!options.duals) not_requested(out->dual_point_);
  if (!options.basis) not_requested(out->basis_);
  return out;
}
std::string LpObservationAccess::unsupported(const ModelSnapshot& model,
                                            const SolveOptions& options) {
  if (options.backend == Backend::Native) return "LP observations require the HiGHS backend";
  if (options.guarantee != Guarantee::Numerical) return "LP observations support Numerical guarantees only";
  for (const auto& v : model.variables)
    if (v.active && v.type != VariableType::Continuous)
      return "LP observations require every active variable to be Continuous; no relaxation is performed";
  for (const auto& i : model.indicators)
    if (i.active) return "LP observations do not support active original indicators";
  for (const auto& g : model.globals)
    if (g.active) return "LP observations do not support active native globals";
  return {};
}
void LpObservationAccess::unavailable(LpObservations& out,LpObservationReason reason,
                                     const std::string& message) {
  for (auto& row : out.rows_) { const bool active=row.active; row={};row.active=active; }
  for (auto& col : out.columns_) { const bool active=col.active;col={};col.active=active; }
  out.checks_={};
  unavailable_group(out.primal_rows_,reason,message);
  unavailable_group(out.dual_point_,reason,message);
  unavailable_group(out.basis_,reason,message);
}

void LpObservationAccess::final_budget(LpObservedResult& out,
    std::shared_ptr<LpObservations>& observations,const SolveBudget& budget) noexcept {
  if (!budget.expired()) return;
  out.result.termination=budget.stop_reason().value_or(Termination::Unknown);
  try {
    out.result.message="Budget expired before LP observations completed";
    if (observations)
      unavailable(*observations,LpObservationReason::Interrupted,out.result.message);
  } catch (...) {
    observations.reset();out.observations.reset();out.result.message.clear();
  }
}

void LpObservationAccess::finish(LpObservations& out,const SolveResult& result,
                                const LpBackendObservations& raw,
                                const SolveBudget& budget) {
  out.metadata_.backend=result.backend;
  out.metadata_.backend_version=result.backend_version;
  auto clear_unpublished = [&]() {
    if (out.primal_rows_.state != LpObservationState::Available)
      for (auto& row : out.rows_) {row.activity.reset();row.lower_slack.reset();row.upper_slack.reset();}
    if (out.dual_point_.state != LpObservationState::Available) {
      for (auto& row : out.rows_) {row.dual.reset();row.dual_source=LpDualSource::None;}
      for (auto& col : out.columns_) col.reduced_cost.reset();
    }
    if (out.basis_.state != LpObservationState::Available) {
      for (auto& row : out.rows_) row.basis.reset();
      for (auto& col : out.columns_) col.basis.reset();
    }
  };
  auto interrupt_pending = [&]() {
    for (auto* item : {&out.primal_rows_,&out.dual_point_,&out.basis_})
      if (item->state == LpObservationState::Unavailable)
        unavailable_group(*item,LpObservationReason::Interrupted,"Budget expired before observation group completed");
    clear_unpublished();
  };
  try {
    tick(budget);
    check_result_layout(out.source_,result);
    if (!result.has_solution()) {
      unavailable(out,LpObservationReason::NoPrimalPoint,"No validated original primal point");
      return;
    }
    const auto primal=validate(out.source_,result.values,out.metadata_.primal_check_tolerance);
    if (!primal.valid || !primal.objective || !result.objective ||
        std::abs(static_cast<Wide>(*primal.objective)-*result.objective) > out.metadata_.checks.objective_gap)
      throw BadObservation("Original primal/objective check rejected observation point");
    std::vector<Sum> activities(out.rows_.size());
    std::size_t work=0;
    for (std::size_t i=0;i<out.source_.rows.size();++i) {
      if (!(i%256)) tick(budget);
      const auto& row=out.source_.rows[i];
      if (!row.active) continue;
      Sum activity;
      for (const auto& term : row.terms) {
        if (!(work++%256)) tick(budget);
        activity.add(static_cast<Wide>(term.coefficient)*result.values[term.variable.id]);
      }
      activities[i]=activity;
      out.rows_[i].activity=narrow(activity.value());
      if (std::isfinite(row.lower)) out.rows_[i].lower_slack=narrow(difference(activity.value(),row.lower,&activity));
      if (std::isfinite(row.upper)) out.rows_[i].upper_slack=narrow(-difference(activity.value(),row.upper,&activity));
    }
    tick(budget);
    out.primal_rows_=available();out.checks_.primal_valid=true;
    if (!raw.attempted) {
      unavailable_group(out.dual_point_,LpObservationReason::NoBackendSolve,"No backend solve produced duals");
      unavailable_group(out.basis_,LpObservationReason::NoBackendSolve,"No backend solve produced a basis");
      return;
    }
    if (!raw.timely) throw Interrupted{};
    if (raw.failure != LpCaptureFailure::None) {
      const bool allocation=raw.failure == LpCaptureFailure::Allocation;
      rejected_group(out.dual_point_,allocation ? LpObservationReason::AllocationFailure : LpObservationReason::InvalidBackendData,
                     "Backend observation capture failed");
      rejected_group(out.basis_,allocation ? LpObservationReason::AllocationFailure : LpObservationReason::InvalidBackendData,
                     "Backend observation capture failed");
      return;
    }
    if (!raw.complete) throw BadObservation("Incomplete backend observation capture");
    check_mapping(out.source_,raw,budget);
    for (auto tolerance : {raw.primal_tolerance,raw.dual_tolerance})
      if (tolerance && (!std::isfinite(*tolerance) || *tolerance < 0))
        throw BadObservation("Invalid effective backend tolerance");
    out.metadata_.backend_primal_tolerance=raw.primal_tolerance;
    out.metadata_.backend_dual_tolerance=raw.dual_tolerance;
    if (result.termination != Termination::Optimal) {
      unavailable_group(out.dual_point_,LpObservationReason::NotOptimal,"Dual observations currently require final Optimal status");
      unavailable_group(out.basis_,LpObservationReason::NotOptimal,"Basis observations currently require final Optimal status");
      return;
    }
    if (!raw.info_valid || !raw.value_valid || !raw.primal_feasible) {
      unavailable_group(out.dual_point_,LpObservationReason::NoPrimalPoint,"Backend primal/info validity flags are unavailable");
      unavailable_group(out.basis_,LpObservationReason::NoPrimalPoint,"Backend primal/info validity flags are unavailable");
      return;
    }
    if (out.dual_point_.state != LpObservationState::NotRequested) {
      if (!raw.dual_valid || !raw.dual_feasible) {
        unavailable_group(out.dual_point_,LpObservationReason::NoDualPoint,"Backend has no valid feasible dual point");
      } else {
        try {
          out.checks_=check_duals(out.source_,result,activities,raw,out.metadata_,budget);
          if (!out.checks_.accepted) {
            rejected_group(out.dual_point_,LpObservationReason::FailedChecks,out.checks_.message);
          } else {
            for (std::size_t i=0;i<raw.column_slots.size();++i)
              out.columns_[raw.column_slots[i]].reduced_cost=raw.column_duals[i];
            for (std::size_t i=0;i<raw.row_slots.size();++i) {
              auto& row=out.rows_[raw.row_slots[i]];
              row.dual=raw.row_duals[i];row.dual_source=LpDualSource::Backend;
            }
            for (std::size_t i=0;i<out.source_.rows.size();++i)
              if (out.source_.rows[i].active && out.source_.rows[i].terms.empty()) {
                out.rows_[i].dual=0;out.rows_[i].dual_source=LpDualSource::DerivedConstantRow;
              }
            tick(budget);out.dual_point_=available();
          }
        } catch (const BadObservation& error) {
          rejected_group(out.dual_point_,LpObservationReason::InvalidBackendData,error.what());
        }
      }
    }
    if (out.basis_.state != LpObservationState::NotRequested) {
      if (!raw.basis_valid || !raw.info_basis_valid) {
        unavailable_group(out.basis_,LpObservationReason::NoBasis,"Backend has no valid basis");
      } else if (std::any_of(out.source_.rows.begin(),out.source_.rows.end(),
                 [](const RowData& row){return row.active && row.terms.empty();})) {
        unavailable_group(out.basis_,LpObservationReason::ElidedConstantRows,"Full original basis unavailable after constant-row elision");
      } else {
        try {
          if (raw.column_basis.size() != raw.column_slots.size() || raw.row_basis.size() != raw.row_slots.size())
            throw BadObservation("Backend basis dimension mismatch");
          std::size_t basics=0;
          for (std::size_t i=0;i<raw.column_slots.size();++i) {
            if (!(i%256)) tick(budget);
            const auto slot=raw.column_slots[i];const auto& variable=out.source_.variables[slot];
            check_basis_position(raw.column_basis[i],result.values[slot],variable.lower,variable.upper,out.metadata_.primal_check_tolerance);
            basics += raw.column_basis[i] == LpBasisStatus::Basic;
          }
          for (std::size_t i=0;i<raw.row_slots.size();++i) {
            if (!(i%256)) tick(budget);
            const auto slot=raw.row_slots[i];const auto& row=out.source_.rows[slot];
            check_basis_position(raw.row_basis[i],activities[slot].value(),row.lower,row.upper,
                                 out.metadata_.primal_check_tolerance,&activities[slot]);
            basics += raw.row_basis[i] == LpBasisStatus::Basic;
          }
          if (basics != raw.row_slots.size()) throw BadObservation("Wrong number of basic entities");
          for (std::size_t i=0;i<raw.column_slots.size();++i) out.columns_[raw.column_slots[i]].basis=raw.column_basis[i];
          for (std::size_t i=0;i<raw.row_slots.size();++i) out.rows_[raw.row_slots[i]].basis=raw.row_basis[i];
          tick(budget);out.basis_=available();
        } catch (const BadObservation& error) {
          rejected_group(out.basis_,LpObservationReason::InvalidBackendData,error.what());
        }
      }
    }
    clear_unpublished();
  } catch (const Interrupted&) {
    interrupt_pending();
  } catch (const BadObservation& error) {
    if (out.primal_rows_.state != LpObservationState::Available)
      rejected_group(out.primal_rows_,LpObservationReason::InvalidBackendData,error.what());
    rejected_group(out.dual_point_,LpObservationReason::InvalidBackendData,error.what());
    rejected_group(out.basis_,LpObservationReason::InvalidBackendData,error.what());
    clear_unpublished();
  }
}
}
}}
