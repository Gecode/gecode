/* Exact, bounded presolve composition for the automatic native coordinator. */
#include <gecode/optimize/native_preprocess.hpp>
#include <gecode/optimize/presolve.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <cmath>
#include <new>
#include <stdexcept>
#include <utility>

namespace Gecode { namespace Optimize { namespace Detail {
namespace {
std::optional<Termination> deadline_stop(const SolveBudget& budget) {
  if (budget.cancelled()) return Termination::Cancelled;
  if (budget.time_limit_reached()) return Termination::TimeLimit;
  return {};
}

SolveResult original_result(const ModelSnapshot& model, const SolveOptions& options) {
  SolveResult result;
  result.model_id=model.model_id; result.revision=model.revision;
  result.guarantee=options.guarantee;
  result.backend="Gecode native exact presolve";
  return result;
}
}

std::optional<SolveResult> native_objective_auxiliary(const ModelSnapshot& model,
    const SolveOptions& options, SolveBudget& budget,
    const NativeSolveContinuation& continuation) {
  // A single exact equality can hide a binary objective behind a nonbinary
  // FlatZinc auxiliary. This is one bounded substitution, never recursive
  // algebra or an assumption that a name/annotation implies equivalence.
  if (!options.primal_start.empty() || options.guarantee==Guarantee::Certified ||
      (options.backend!=Backend::Auto && options.backend!=Backend::Native) ||
      options.threads!=1 || options.random_seed!=0 ||
      !model.globals.empty() || !model.indicators.empty() ||
      model.variables.size()>4096 || model.rows.size()>4096 ||
      model.objective.terms.size()!=1 || model.objective.terms[0].coefficient!=1) return {};
  auto result=original_result(model,options);
  const auto finish=[&](SolveResult value) -> std::optional<SolveResult> {
    if(const auto stop=deadline_stop(budget)) {
      value=original_result(model,options);value.termination=*stop;
      value.message="Exact objective auxiliary substitution stopped before original-result publication";
    }
    value.elapsed_seconds=budget.elapsed_seconds();return value;
  };
  try {
    if(const auto stop=budget.stop_reason()) {result.termination=*stop;return finish(std::move(result));}
    validate_structure(model);
    const auto auxiliary=model.objective.terms[0].variable.id;
    const auto& variable=model.variables[auxiliary];
    if(variable.type!=VariableType::Integer ||
       (variable.lower>=0 && variable.upper<=1)) return {};
    // Independently bound every conversion/product below, even though callers
    // have already compiled the original model under stricter native limits.
    constexpr std::int64_t limit=INT64_C(2147483647), exact=INT64_C(9007199254740992);
    const auto integer=[](double value) {
      return std::isfinite(value) && std::trunc(value)==value && std::abs(value)<=2147483647.0;
    };
    for(const auto& v:model.variables) if(v.active &&
        ((v.type!=VariableType::Integer && v.type!=VariableType::Binary) ||
         !integer(v.lower) || !integer(v.upper))) return {};
    std::optional<std::size_t> equality;
    double sign=0;std::size_t nonzeros=0;
    for(std::size_t i=0;i<model.rows.size();++i) {
      if(const auto stop=deadline_stop(budget)) {result.termination=*stop;return finish(std::move(result));}
      const auto& row=model.rows[i];
      if(row.terms.size()>65536-nonzeros)return {};nonzeros+=row.terms.size();
      if(!row.active)continue;
      for(const auto& term:row.terms) if(term.variable.id==auxiliary) {
        if(equality || row.lower!=row.upper || !integer(row.lower) ||
           std::abs(term.coefficient)!=1)return {};
        equality=i;sign=term.coefficient;
      }
    }
    if(!equality)return {};
    const auto& equation=model.rows[*equality];
    const auto constant=static_cast<std::int64_t>(sign*equation.lower);
    std::int64_t lower=constant,upper=constant,magnitude=0;
    std::vector<Term> terms;
    for(const auto& term:equation.terms) if(term.variable.id!=auxiliary) {
      if(!integer(term.coefficient))return {};
      const auto coefficient=static_cast<std::int64_t>(-sign*term.coefficient);
      const auto& v=model.variables[term.variable.id];
      const auto a=coefficient*static_cast<std::int64_t>(v.lower);
      const auto b=coefficient*static_cast<std::int64_t>(v.upper);
      const auto size=std::max(std::abs(a),std::abs(b));
      if(size>limit-magnitude)return {};magnitude+=size;
      lower+=std::min(a,b);upper+=std::max(a,b);
      terms.push_back({term.variable,static_cast<double>(coefficient)});
    }
    if(!std::isfinite(model.objective.offset) || std::trunc(model.objective.offset)!=model.objective.offset ||
       std::abs(model.objective.offset)>static_cast<double>(exact))return {};
    const auto offset=static_cast<std::int64_t>(model.objective.offset);
    if(std::abs(offset+constant)>exact || std::abs(offset+lower)>exact || std::abs(offset+upper)>exact)return {};
    auto reduced=model;reduced.variables[auxiliary].active=false;
    reduced.objective.terms=terms;reduced.objective.offset=static_cast<double>(offset+constant);
    auto& row=reduced.rows[*equality];
    row.terms=terms;row.lower=variable.lower-constant;row.upper=variable.upper-constant;
    // Auxiliary domain bounds are real constraints. Remove the defining row
    // only when the remaining variable domain box independently implies both.
    if(lower>=variable.lower && upper<=variable.upper){row.active=false;row.terms.clear();}
    validate_structure(reduced);
    auto exact_options=options;exact_options.guarantee=Guarantee::Exact;
    auto solved=continuation(reduced,exact_options,budget);
    if(const auto stop=deadline_stop(budget)){result.termination=*stop;return finish(std::move(result));}
    if(solved.model_id!=reduced.model_id || solved.revision!=reduced.revision ||
       solved.guarantee!=Guarantee::Exact || solved.start_submitted)
      throw std::runtime_error("Objective auxiliary solve returned foreign identity, guarantee or start");
    if(solved.termination==Termination::Unsupported)return {}; // Preserve original numeric admission.
    if(solved.termination==Termination::InvalidModel || solved.termination==Termination::Unbounded ||
       solved.termination==Termination::InfeasibleOrUnbounded ||
       (solved.best_bound && !std::isfinite(*solved.best_bound)))
      throw std::runtime_error("Objective auxiliary solve returned inconsistent status or bound");
    if(solved.has_solution()) {
      if(solved.values.size()!=model.variables.size() || solved.active_variables.size()!=model.variables.size())
        throw std::runtime_error("Objective auxiliary solve returned incomplete coordinates");
      for(std::size_t i=0;i<reduced.variables.size();++i)
        if(solved.active_variables[i]!=reduced.variables[i].active)
          throw std::runtime_error("Objective auxiliary solve returned an inconsistent active-variable mask");
      const auto checked=validate(reduced,solved.values,0,0);
      if(!checked.valid || checked.objective!=solved.objective)
        throw std::runtime_error("Objective auxiliary reduced witness failed exact validation");
      std::int64_t value=constant;
      for(const auto& term:terms)value+=static_cast<std::int64_t>(term.coefficient)*
        static_cast<std::int64_t>(solved.values[term.variable.id]);
      solved.values[auxiliary]=static_cast<double>(value);solved.active_variables[auxiliary]=true;
      const auto restored=validate(model,solved.values,0,0);
      if(!restored.valid || restored.objective!=solved.objective)
        throw std::runtime_error("Objective auxiliary original witness failed exact validation");
      solved.solution_validated=true;
    }
    if((solved.termination==Termination::Optimal && !solved.has_solution()) ||
       (solved.termination==Termination::Infeasible && solved.has_solution()))
      throw std::runtime_error("Objective auxiliary solve returned inconsistent proof status");
    // Full objectives (including the constant) are equal, so original bounds
    // transfer unchanged. No transformed point/proof is published before QA.
    solved.guarantee=options.guarantee;solved.update_gaps(model.objective.sense);
    solved.message="Exact objective auxiliary substitution; "+solved.message;
    return finish(std::move(solved));
  } catch(const std::bad_alloc&) {
    result=original_result(model,options);result.termination=Termination::MemoryLimit;
    result.message="Exact objective auxiliary substitution allocation failed";
  } catch(const std::exception& error) {
    result=original_result(model,options);result.termination=Termination::BackendError;
    result.message=std::string("Exact objective auxiliary substitution: ")+error.what();
  }
  return finish(std::move(result));
}

std::optional<SolveResult> native_presolve(const ModelSnapshot& model,
    const SolveOptions& options, SolveBudget& budget,
    const NativeSolveContinuation& continuation) {
  // A previously supplied original start must remain available even when the
  // global deadline leaves no time for reconstruction. The existing native
  // start path owns that publication contract, so composition skips starts.
  if (!options.primal_start.empty() || options.guarantee==Guarantee::Certified ||
      (options.backend!=Backend::Auto && options.backend!=Backend::Native) ||
      options.threads!=1 || options.random_seed!=0 ||
      !model.globals.empty() || !model.indicators.empty()) return {};
  constexpr std::size_t max_columns=4096, max_rows=4096, max_nonzeros=65536;
  if (model.variables.size()>max_columns || model.rows.size()>max_rows ||
      model.objective.terms.size()>max_nonzeros) return {};

  auto result=original_result(model,options);
  const auto finish=[&](SolveResult value) -> std::optional<SolveResult> {
    // A reduced incumbent is not an original incumbent until exact postsolve
    // finishes. Never publish a newly reconstructed point or proof too late.
    // Node quotas intentionally do not enter this check: the final admitted
    // node may complete, and its timely result remains valid at the quota.
    if (const auto stop=deadline_stop(budget)) {
      auto stopped=original_result(model,options);
      stopped.backend=std::move(value.backend);
      stopped.backend_version=std::move(value.backend_version);
      stopped.termination=*stop;
      stopped.message="Exact integer presolve composition stopped before original-result publication";
      value=std::move(stopped);
    }
    value.elapsed_seconds=budget.elapsed_seconds();
    return value;
  };
  const auto stopped=[&](Termination reason) {
    result.termination=reason;
    result.message="Exact integer presolve composition budget stopped";
    return finish(std::move(result));
  };
  try {
    if (const auto reason=budget.stop_reason()) return stopped(*reason);
    for (const auto& variable:model.variables) {
      if (const auto reason=deadline_stop(budget)) return stopped(*reason);
      if (variable.active && variable.type!=VariableType::Integer &&
          variable.type!=VariableType::Binary) return {};
    }
    std::size_t nonzeros=model.objective.terms.size();
    for (const auto& row:model.rows) {
      if (const auto reason=deadline_stop(budget)) return stopped(*reason);
      if (row.terms.size()>max_nonzeros-nonzeros) return {};
      nonzeros+=row.terms.size();
    }
    PresolveOptions settings;
    settings.time_limit_seconds=budget.remaining_seconds();
    settings.cancellation=budget.cancellation();
    settings.max_passes=4; settings.max_row_visits=65536;
    const auto prepared=presolve_integer(model,settings);
    if (const auto reason=deadline_stop(budget)) return stopped(*reason);
    if (prepared.model_id!=model.model_id || prepared.revision!=model.revision ||
        prepared.guarantee!=Guarantee::Exact)
      throw std::runtime_error("Exact presolve returned foreign identity or guarantee");
    if (prepared.status==PresolveStatus::Infeasible &&
        prepared.termination==Termination::Infeasible) {
      result.termination=Termination::Infeasible;
      result.message="Exact integer presolve proved an original-model contradiction";
      return finish(std::move(result));
    }
    // Only finalized artifacts carry the equivalence contract. Work-limited
    // artifacts are valid; a deadline-interrupted partial working state is not.
    if (!prepared.model ||
        !((prepared.status==PresolveStatus::Fixpoint && prepared.termination==Termination::Optimal) ||
          (prepared.status==PresolveStatus::Incomplete && prepared.termination==Termination::IterationLimit)))
      return {};
    if (!prepared.fixed_variables && !prepared.removed_rows && prepared.changes.empty()) return {};
    const auto& artifact=*prepared.model;
    if (artifact.original().model_id!=model.model_id || artifact.original().revision!=model.revision)
      throw std::runtime_error("Exact presolve artifact has foreign original identity");
    const auto& reduced=artifact.reduced();
    auto exact_options=options;
    exact_options.guarantee=Guarantee::Exact;
    exact_options.time_limit_seconds=budget.remaining_seconds();
    exact_options.cancellation=budget.cancellation();
    auto solved=continuation(reduced,exact_options,budget);
    result.backend=solved.backend; result.backend_version=solved.backend_version;
    if (const auto reason=deadline_stop(budget)) return stopped(*reason);
    if (solved.model_id!=reduced.model_id || solved.revision!=reduced.revision ||
        solved.guarantee!=Guarantee::Exact || solved.start_submitted)
      throw std::runtime_error("Reduced native solve returned inconsistent identity, guarantee or start");
    // Reduction can produce row sides outside a narrower optional/native
    // compiler's admission range. Preserve the original route's acceptance.
    if (solved.termination==Termination::Unsupported) return {};
    if (solved.termination==Termination::InvalidModel)
      throw std::runtime_error("Reduced native model was rejected as invalid");
    if (solved.has_solution()) {
      auto restored=artifact.postsolve(solved,0);
      if (!restored.exact_witness_validated || !restored.solution.has_solution() ||
          restored.solution.objective!=solved.objective)
        throw std::runtime_error("Reduced native witness failed exact original postsolve: "+restored.solution.message);
      result=std::move(restored.solution);
      result.guarantee=options.guarantee;
    }
    if ((solved.termination==Termination::Optimal && !result.has_solution()) ||
        (solved.termination==Termination::Infeasible && result.has_solution()) ||
        solved.termination==Termination::Unbounded || solved.termination==Termination::InfeasibleOrUnbounded)
      throw std::runtime_error("Reduced finite native solve returned inconsistent proof status");
    // PresolveBuilder folds fixed costs into the reduced objective offset;
    // original and reduced FULL objective values are identical, including for
    // maximization. Bounds therefore transfer unchanged, never with a second
    // offset correction. Only the actual Exact native result supplies a proof;
    // PresolveStatus::Fixpoint's Optimal termination is not used here.
    result.termination=solved.termination;
    if (solved.best_bound && !std::isfinite(*solved.best_bound))
      throw std::runtime_error("Reduced native solve returned a nonfinite bound");
    if (solved.termination!=Termination::Infeasible) result.best_bound=solved.best_bound;
    if (solved.termination==Termination::Optimal) result.best_bound=result.objective;
    result.update_gaps(model.objective.sense);
    result.message=std::string("Exact integer presolve (")+
      (prepared.status==PresolveStatus::Fixpoint ? "fixpoint reduction" : "partial reduction")+
      "): "+solved.message;
    return finish(std::move(result));
  } catch (const std::bad_alloc&) {
    result=original_result(model,options); result.termination=Termination::MemoryLimit;
    result.message="Exact integer presolve composition allocation failed";
  } catch (const std::exception& error) {
    result=original_result(model,options); result.termination=Termination::BackendError;
    result.message=std::string("Exact integer presolve composition: ")+error.what();
  }
  return finish(std::move(result));
}

}}}
