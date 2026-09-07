#include <gecode/optimize/scenarios.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Gecode { namespace Optimize {
#ifdef GECODE_OPTIMIZE_TEST_SCENARIOS
namespace Detail {
SolveResult scenario_test_solve(const ModelSnapshot&, const SolveOptions&, SolveSession*);
void scenario_test_checkpoint(const char*, std::size_t);
}
#endif
namespace {
struct Stopped { Termination reason; const char* message; };
struct Unsupported : std::runtime_error { using std::runtime_error::runtime_error; };
struct EvidenceError : std::runtime_error { using std::runtime_error::runtime_error; };
// SolveBudget owns the shared cancellation/node state. This enclosing clock
// also includes option validation and its construction in the total allowance.
class BatchBudget {
  SolveBudget shared_;
  std::chrono::steady_clock::time_point started_;
  double seconds_;
public:
  BatchBudget(const SolveOptions& options,std::chrono::steady_clock::time_point started)
    : shared_(options),started_(started),seconds_(options.time_limit_seconds) {}
  double remaining_seconds() const noexcept {
    const double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-started_).count();
    return std::max(0.0,seconds_-elapsed);
  }
  std::optional<Termination> stop_reason() const noexcept {
    if(shared_.cancelled())return Termination::Cancelled;
    if(remaining_seconds()==0)return Termination::TimeLimit;
    if(shared_.node_limit_reached())return Termination::NodeLimit;
    return {};
  }
  std::shared_ptr<CancellationToken> cancellation() const noexcept{return shared_.cancellation();}
};
void checkpoint(const BatchBudget& budget) {
  if (const auto reason=budget.stop_reason()) throw Stopped{*reason,"Whole scenario batch budget stopped"};
}
struct Meter {
  const BatchBudget& budget;
  std::size_t limit, used=0;
  void tick(std::size_t count=1) {
    checkpoint(budget);
    if(count>limit-used)throw Stopped{Termination::IterationLimit,"Scenario coordinator element-visit cap reached"};
    used+=count;
  }
};
void tick(Meter* meter,std::size_t count=1){if(meter)meter->tick(count);}
void event(const char* point,std::size_t index) {
#ifdef GECODE_OPTIMIZE_TEST_SCENARIOS
  Detail::scenario_test_checkpoint(point,index);
#else
  (void)point;(void)index;
#endif
}
void storage(std::size_t count,std::size_t limit,const char* message) {
  if(count>limit)throw Stopped{Termination::MemoryLimit,message};
}
void product(std::size_t a,std::size_t b,std::size_t limit,const char* message) {
  if(a && b>limit/a)throw Stopped{Termination::MemoryLimit,message};
}
void visit(const ModelSnapshot& model,Meter* meter) {
  tick(meter,model.variables.size());
  for(const auto& v:model.variables)tick(meter,v.name.size());
  tick(meter,model.rows.size());
  for(const auto& row:model.rows){tick(meter,row.terms.size());tick(meter,row.name.size());}
  tick(meter,model.objective.terms.size());
}
std::size_t variable(const ModelSnapshot& model,Variable handle) {
  if(handle.model_id!=model.model_id || handle.id>=model.variables.size() ||
     !model.variables[static_cast<std::size_t>(handle.id)].active)
    throw ModelError("Scenario variable is foreign, missing or deleted");
  return static_cast<std::size_t>(handle.id);
}
std::size_t row(const ModelSnapshot& model,Constraint handle) {
  if(handle.model_id!=model.model_id || handle.id>=model.rows.size() ||
     !model.rows[static_cast<std::size_t>(handle.id)].active)
    throw ModelError("Scenario row is foreign, missing or deleted");
  return static_cast<std::size_t>(handle.id);
}
void bounds(double lower,double upper) {
  const double inf=std::numeric_limits<double>::infinity();
  if(std::isnan(lower)||std::isnan(upper)||lower==inf||upper==-inf||lower>upper)
    throw ModelError("Scenario has invalid or reversed bounds");
}
void definition(const ModelSnapshot& base,const ScenarioDefinition& def,Meter& meter) {
  if(def.objective_offset && !std::isfinite(*def.objective_offset))
    throw ModelError("Scenario objective offset must be finite");
  meter.tick(base.variables.size());
  std::vector<bool> seen(base.variables.size(),false);
  for(const auto& term:def.objective_coefficients) {
    meter.tick();const auto slot=variable(base,term.variable);
    if(seen[slot])throw ModelError("Duplicate scenario objective coefficient override");
    seen[slot]=true;
    if(!std::isfinite(term.coefficient))throw ModelError("Scenario objective coefficient must be finite");
  }
  meter.tick(base.variables.size());std::fill(seen.begin(),seen.end(),false);
  for(const auto& change:def.variable_bounds) {
    meter.tick();const auto slot=variable(base,change.variable);
    if(seen[slot])throw ModelError("Duplicate scenario variable-bound override");
    seen[slot]=true;
    if(!change.lower&&!change.upper)throw ModelError("Scenario variable override has no bound side");
    const auto& v=base.variables[slot];
    const double lo=change.lower.value_or(v.lower),hi=change.upper.value_or(v.upper);
    bounds(lo,hi);
    if(v.type==VariableType::Binary&&(lo<0||hi>1))throw ModelError("Scenario binary bounds must remain in [0,1]");
  }
  meter.tick(base.rows.size());seen.assign(base.rows.size(),false);
  for(const auto& change:def.row_bounds) {
    meter.tick();const auto slot=row(base,change.row);
    if(seen[slot])throw ModelError("Duplicate scenario row-side override");
    seen[slot]=true;
    if(!change.lower&&!change.upper)throw ModelError("Scenario row override has no bound side");
    const auto& r=base.rows[slot];
    bounds(change.lower.value_or(r.lower),change.upper.value_or(r.upper));
  }
}
ModelSnapshot materialize(const ModelSnapshot& base,const ScenarioDefinition& def,
                          ModelId owner,Revision revision,Meter* meter) {
  visit(base,meter); // precharge the owned source copy
  ModelSnapshot out=base;out.model_id=owner;out.revision=revision;
  for(auto& v:out.variables){tick(meter);v.variable.model_id=owner;}
  for(auto& r:out.rows){tick(meter);r.constraint.model_id=owner;
    for(auto& t:r.terms){tick(meter);t.variable.model_id=owner;}}
  tick(meter,base.variables.size());std::vector<double> coefficients(base.variables.size(),0);
  for(const auto& t:base.objective.terms){tick(meter);coefficients[t.variable.id]=t.coefficient;}
  for(const auto& t:def.objective_coefficients){tick(meter);coefficients[t.variable.id]=t.coefficient;}
  out.objective.terms.clear();
  for(std::size_t i=0;i<coefficients.size();++i){tick(meter);
    if(coefficients[i]!=0)out.objective.terms.push_back({Variable{owner,i},coefficients[i]});}
  if(def.objective_offset)out.objective.offset=*def.objective_offset;
  for(const auto& change:def.variable_bounds){tick(meter);auto& v=out.variables[change.variable.id];
    if(change.lower)v.lower=*change.lower;if(change.upper)v.upper=*change.upper;}
  for(const auto& change:def.row_bounds){tick(meter);auto& r=out.rows[change.row.id];
    if(change.lower)r.lower=*change.lower;if(change.upper)r.upper=*change.upper;}
  visit(out,meter);validate_structure(out);tick(meter,0);
  return out;
}
bool definitive(Termination status) {
  return status==Termination::Optimal||status==Termination::Infeasible||status==Termination::Unbounded;
}
void known_status(Termination status) {
  switch(status) {
  case Termination::Unknown:case Termination::Optimal:case Termination::Infeasible:
  case Termination::Unbounded:case Termination::InfeasibleOrUnbounded:
  case Termination::TimeLimit:case Termination::NodeLimit:case Termination::MemoryLimit:
  case Termination::IterationLimit:case Termination::SolutionLimit:case Termination::ObjectiveLimit:
  case Termination::Cancelled:case Termination::NumericalFailure:case Termination::Unsupported:
  case Termination::InvalidModel:case Termination::BackendError:return;
  }
  throw EvidenceError("Scenario result contains an unknown termination value");
}
std::uint64_t difference(std::uint64_t next,std::uint64_t before) {
  if(next<before)throw EvidenceError("Scenario session counter decreased or overflowed");
  return next-before;
}
SessionStatistics difference(const SessionStatistics& next,const SessionStatistics& before) {
  return {difference(next.solve_calls,before.solve_calls),difference(next.model_loads,before.model_loads),
    difference(next.incremental_updates,before.incremental_updates),difference(next.unchanged_models,before.unchanged_models),
    difference(next.basis_warm_starts,before.basis_warm_starts),difference(next.incumbent_starts,before.incumbent_starts)};
}
void clear(SolveResult& result,Termination reason,const char* message) {
  result.termination=reason;result.solution_validated=false;
  result.objective.reset();result.best_bound.reset();result.absolute_gap.reset();
  result.relative_gap.reset();result.native_backend_gap.reset();result.values.clear();
  result.message=message;
}
constexpr std::int64_t exact_limit=INT64_C(9007199254740992);
std::int64_t integer(double value) {
  if(!std::isfinite(value)||value!=std::trunc(value)||std::abs(value)>static_cast<double>(exact_limit))
    throw EvidenceError("Exact scenario witness/data is not a supported exact integer");
  return static_cast<std::int64_t>(value);
}
std::int64_t add(std::int64_t a,std::int64_t b) {
  constexpr auto hi=std::numeric_limits<std::int64_t>::max(),lo=std::numeric_limits<std::int64_t>::min();
  if((b>0&&a>hi-b)||(b<0&&a<lo-b))throw EvidenceError("Exact scenario arithmetic overflow");
  return a+b;
}
std::int64_t multiply(std::int64_t a,std::int64_t b) {
  constexpr auto hi=std::numeric_limits<std::int64_t>::max(),lo=std::numeric_limits<std::int64_t>::min();
  if((a>0&&((b>0&&a>hi/b)||(b<0&&b<lo/a))) ||
     (a<0&&((b>0&&a<lo/b)||(b<0&&a<hi/b))))
    throw EvidenceError("Exact scenario arithmetic overflow");
  return a*b;
}
double exact(const ModelSnapshot& model,const std::vector<double>& values,Meter& meter) {
  for(const auto& v:model.variables){meter.tick();if(!v.active)continue;
    if(v.type!=VariableType::Integer&&v.type!=VariableType::Binary)
      throw EvidenceError("Exact scenario result has a nondiscrete original variable");
    const auto x=integer(values[v.variable.id]);
    if(x<integer(v.lower)||x>integer(v.upper))throw EvidenceError("Exact scenario variable violation");}
  const auto activity=[&](const std::vector<Term>& terms,std::int64_t offset) {
    auto sum=offset;for(const auto& t:terms){meter.tick();sum=add(sum,multiply(integer(t.coefficient),integer(values[t.variable.id])));}return sum;
  };
  for(const auto& r:model.rows){meter.tick();if(!r.active)continue;
    const auto sum=activity(r.terms,0);
    if((std::isfinite(r.lower)&&sum<integer(r.lower))||(std::isfinite(r.upper)&&sum>integer(r.upper)))
      throw EvidenceError("Exact scenario row violation");}
  const auto value=activity(model.objective.terms,integer(model.objective.offset));
  if(value < -exact_limit||value>exact_limit)throw EvidenceError("Exact scenario objective is not an exact double integer");
  return static_cast<double>(value);
}
ScenarioCheck check(const ModelSnapshot& model,SolveResult& result,const SolveOptions& options,Meter& meter) {
  ScenarioCheck out;
  known_status(result.termination);
  if(result.model_id!=model.model_id||result.revision!=model.revision)
    throw EvidenceError("Scenario result owner or revision differs from the materialized scenario");
  if(result.guarantee!=options.guarantee)throw EvidenceError("Scenario result guarantee differs from request");
  if((result.objective&&!std::isfinite(*result.objective)) || (result.best_bound&&std::isnan(*result.best_bound)))
    throw EvidenceError("Scenario result contains malformed objective or bound");
  if(result.native_backend_gap&&(!std::isfinite(*result.native_backend_gap)||*result.native_backend_gap<0))
    throw EvidenceError("Scenario result contains a malformed vendor gap");
  const bool supplied_candidate=!result.values.empty() || result.objective || result.solution_validated;
  const bool values_present=supplied_candidate ||
    (model.variables.empty()&&definitive(result.termination));
  if(values_present || !result.active_variables.empty()) {
    if(result.active_variables.size()!=model.variables.size())throw EvidenceError("Scenario active-mask dimension mismatch");
    for(std::size_t i=0;i<model.variables.size();++i){meter.tick();
      if(result.active_variables[i]!=model.variables[i].active)throw EvidenceError("Scenario active mask differs from original slots");}
  }
  out.identity_valid=true;
  if(values_present) {
    out.candidate_examined=true;
    if(result.values.size()!=model.variables.size())throw EvidenceError("Scenario value dimension mismatch");
    visit(model,&meter);out.validation=validate(model,result.values,options.feasibility_tolerance,options.integrality_tolerance);
    meter.tick(0);
    if(out.validation.valid && result.termination==Termination::Infeasible)
      throw EvidenceError("A feasible original-scenario witness contradicts reported infeasibility");
    if(!out.validation.valid&&supplied_candidate)throw EvidenceError("Scenario raw candidate failed original-model validation");
    if(result.objective) {
      out.objective_matches=out.validation.objective && *result.objective==*out.validation.objective;
      if(!out.objective_matches)throw EvidenceError("Scenario objective differs from original-model reevaluation");
    }
    if(result.solution_validated) {
      if(!out.validation.valid || !out.validation.objective || !result.objective)
        throw EvidenceError("Scenario incumbent failed independent original-model validation");
      if(options.backend==Backend::Native || options.guarantee==Guarantee::Exact) {
        if(exact(model,result.values,meter)!=*result.objective)throw EvidenceError("Scenario exact objective mismatch");
        out.exact_witness_validated=true;
      }
    }
  }
  if(result.termination==Termination::Optimal&&!result.has_solution())
    throw EvidenceError("Scenario reported Optimal without a validated original solution");
  if(result.termination==Termination::Optimal&&options.guarantee==Guarantee::Exact &&
     (!result.best_bound||!std::isfinite(*result.best_bound)||*result.best_bound!=*result.objective))
    throw EvidenceError("Exact scenario Optimal requires a finite global bound equal to its exact objective");
  if(result.termination==Termination::Unbounded) {
    bool bounded=true;
    for(const auto& t:model.objective.terms) {
      meter.tick();const auto& v=model.variables[t.variable.id];
      const bool lower=(model.objective.sense==ObjectiveSense::Minimize)==(t.coefficient>0);
      bounded=bounded&&std::isfinite(lower?v.lower:v.upper);
    }
    if(bounded)throw EvidenceError("Original variable bounds contradict objective unboundedness");
  }
  try{result.update_gaps(model.objective.sense);}catch(const ModelError& e){throw EvidenceError(e.what());}
  return out;
}
}

void ScenarioBatchOptions::validate() const {
  solve.validate();
  if(reuse!=ScenarioReuse::Automatic&&reuse!=ScenarioReuse::Cold)throw ModelError("Invalid scenario reuse mode");
}
ModelId ScenarioBatch::id() const noexcept{return owner_;}
const ModelSnapshot& ScenarioBatch::base() const noexcept{return base_;}
std::size_t ScenarioBatch::size() const noexcept{return definitions_.size();}
ScenarioId ScenarioBatch::scenario(std::size_t index) const {
  if(index>=size())throw ModelError("Scenario index is outside its batch");return {owner_,index};
}
const ScenarioDefinition& ScenarioBatch::definition(ScenarioId id) const {
  if(id.batch_id!=owner_||id.index>=size())throw ModelError("Scenario ID is foreign or out of range");
  return definitions_[static_cast<std::size_t>(id.index)];
}
ModelSnapshot ScenarioBatch::materialize(ScenarioId id) const {
  return Optimize::materialize(base_,definition(id),owner_,id.index+1,nullptr);
}
Variable ScenarioBatch::map(Variable original) const{return {owner_,variable(base_,original)};}
Constraint ScenarioBatch::map(Constraint original) const{return {owner_,row(base_,original)};}
bool ScenarioBatchResult::all_resolved() const noexcept {
  if(!batch||completion!=ScenarioBatchCompletion::Complete||outcomes.size()!=batch->size()||resolved!=outcomes.size())return false;
  for(const auto& outcome:outcomes)if(!outcome.result||!definitive(outcome.result->termination))return false;
  return true;
}
double ScenarioBatchResult::value(ScenarioId id,Variable original) const {
  if(!batch)throw ModelError("Scenario batch was not admitted");
  batch->definition(id);
  if(id.index>=outcomes.size())throw ModelError("Scenario outcome is missing");
  const auto& outcome=outcomes[id.index];
  if(!outcome.result||outcome.scenario.batch_id!=id.batch_id||outcome.scenario.index!=id.index||
     outcome.result->model_id!=batch->id()||outcome.result->revision!=id.index+1)
    throw ModelError("Scenario result identity is absent or inconsistent");
  return outcome.result->value(batch->map(original));
}
namespace Detail {
struct ScenarioBatchAccess {
  static std::shared_ptr<ScenarioBatch> create(const ModelSnapshot& base,
      const std::vector<ScenarioDefinition>& definitions,const ScenarioBatchOptions& options,
      Meter& meter,ScenarioBatchResult& output) {
    storage(definitions.size(),options.max_scenarios,"Scenario count cap reached");
    if(definitions.size()>=std::numeric_limits<Revision>::max())
      throw Unsupported("Too many scenario revisions");
    product(definitions.size(),base.variables.size(),options.max_saved_value_slots,"Scenario saved-value slot cap reached");
    if(!base.indicators.empty()||!base.globals.empty())
      throw Unsupported("Scenario batches currently reject all indicator/global metadata, including inactive history");
    visit(base,&meter);validate_structure(base);meter.tick(0);
    // Admission is independent of backend availability and batch size. These
    // routes cannot meet this guarantee, even if no scenario is requested.
    if(options.solve.guarantee==Guarantee::Certified ||
       (options.solve.guarantee==Guarantee::Exact&&options.solve.backend!=Backend::Native))
      throw Unsupported("Scenario batches require Numerical, or explicit Native with Exact");
    for(const auto& v:base.variables){meter.tick();if(v.active&&v.type!=VariableType::Continuous&&v.type!=VariableType::Integer&&v.type!=VariableType::Binary)
      throw Unsupported("Scenario batches currently support Continuous, Integer and Binary variables only");}
    if(!options.solve.primal_start.empty())throw Unsupported("Scenario batches do not yet admit common primal starts");
    if(definitions.size()>1&&options.solve.node_limit&&*options.solve.node_limit>0)
      throw Unsupported("Multi-scenario positive node limits require consumed-node accounting");
    std::size_t count=0;
    for(std::size_t i=0;i<definitions.size();++i){
      output.offending_scenario=i;const auto& def=definitions[i];
      for(auto size:{def.objective_coefficients.size(),def.variable_bounds.size(),def.row_bounds.size()}) {
        storage(size,options.max_patch_entries-count,"Scenario patch-entry cap reached");count+=size;
      }
      meter.tick();meter.tick(def.name.size());definition(base,def,meter);event("admission",i);meter.tick(0);
    }
    output.offending_scenario.reset();
    visit(base,&meter);meter.tick(count);meter.tick(definitions.size());
    for(const auto& def:definitions)meter.tick(def.name.size());
    auto batch=std::shared_ptr<ScenarioBatch>(new ScenarioBatch);
    Model owner;batch->owner_=owner.id();
    // Public snapshots can contain a caller-selected positive owner number.
    // Keep original/private identity distinct even if it predicts our next ID.
    if(batch->owner_==base.model_id){Model distinct;batch->owner_=distinct.id();}
    batch->base_=base;batch->definitions_=definitions;
    for(std::size_t i=0;i<definitions.size();++i){
      output.offending_scenario=i;
      {auto tested=Optimize::materialize(base,definitions[i],batch->owner_,i+1,&meter);(void)tested;}
      meter.tick(0);
    }
    output.offending_scenario.reset();return batch;
  }
};
}
namespace {
template<class Snapshot>
ScenarioBatchResult run(Snapshot snapshot,ModelId owner,Revision revision,
                        const std::vector<ScenarioDefinition>& definitions,const ScenarioBatchOptions& options) {
  const auto started=std::chrono::steady_clock::now();
  ScenarioBatchResult out;out.model_id=owner;out.revision=revision;
  std::optional<BatchBudget> budget;
  std::optional<Meter> meter;
  std::optional<std::size_t> current;
  auto fail=[&](Termination reason,const char* message) noexcept {
    out.completion=out.batch?ScenarioBatchCompletion::Interrupted:ScenarioBatchCompletion::Rejected;
    out.stop_reason=reason;
    try {out.message=message;}catch(...){out.message.clear();}
    if(current && out.outcomes[*current].result) {
      out.outcomes[*current].check.reset();
      try {clear(*out.outcomes[*current].result,reason,message);}
      catch(...){out.outcomes[*current].result->message.clear();}
    }
  };
  try {
    options.validate();budget.emplace(options.solve,started);meter.emplace(Meter{*budget,options.max_work,0});
    checkpoint(*budget);
    const auto& original=snapshot();checkpoint(*budget);
    auto batch=Detail::ScenarioBatchAccess::create(original,definitions,options,*meter,out);
    meter->tick(definitions.size());std::vector<ScenarioOutcome> outcomes(definitions.size());
    for(std::size_t i=0;i<outcomes.size();++i)outcomes[i].scenario=batch->scenario(i);
    checkpoint(*budget);out.batch=std::move(batch);out.outcomes=std::move(outcomes);
    out.completion=ScenarioBatchCompletion::Interrupted;
    {
      std::optional<SolveSession> session;
      if(options.reuse==ScenarioReuse::Automatic && options.solve.backend!=Backend::Native && !out.outcomes.empty())session.emplace();
      for(std::size_t i=0;i<out.outcomes.size();++i) {
        checkpoint(*budget);current=i;out.offending_scenario=i;
        auto& outcome=out.outcomes[i];const auto stage_started=std::chrono::steady_clock::now();
        {
          auto scenario=Optimize::materialize(out.batch->base(),out.batch->definition(outcome.scenario),out.batch->id(),i+1,&*meter);
          auto adjusted=options.solve;adjusted.time_limit_seconds=budget->remaining_seconds();adjusted.cancellation=budget->cancellation();
          const auto before=session?session->statistics():SessionStatistics{};
          event("before_solve",i);checkpoint(*budget);outcome.state=ScenarioRunState::Attempted;++out.attempted;
          outcome.result.emplace();outcome.result->model_id=scenario.model_id;
          outcome.result->revision=scenario.revision;outcome.result->guarantee=options.solve.guarantee;
#ifdef GECODE_OPTIMIZE_TEST_SCENARIOS
          auto result=Detail::scenario_test_solve(scenario,adjusted,session?&*session:nullptr);
#else
          auto result=session?session->solve(scenario,adjusted):solve(scenario,adjusted);
#endif
          const auto after=session?session->statistics():SessionStatistics{};
          outcome.reuse_delta=difference(after,before);out.reuse_statistics=after;
          event("after_solve",i);checkpoint(*budget);
          auto checked=check(scenario,result,options.solve,*meter);
          event("after_check",i);checkpoint(*budget);
          // All allocating checked fields are staged before publication.
          outcome.result=std::move(result);outcome.check=std::move(checked);
        }
        event("after_cleanup",i);checkpoint(*budget);
        outcome.elapsed_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-stage_started).count();
        if(!definitive(outcome.result->termination)) {
          out.stop_reason=outcome.result->termination;out.message="Scenario did not reach a definitive outcome";
          current.reset();break;
        }
        ++out.resolved;current.reset();out.offending_scenario.reset();
      }
    }
    event("batch_cleanup",out.outcomes.size());checkpoint(*budget);
    if(out.resolved==out.outcomes.size()) {out.completion=ScenarioBatchCompletion::Complete;out.stop_reason.reset();}
  } catch(const Stopped& e) {fail(e.reason,e.message);}
  catch(const Unsupported& e){fail(Termination::Unsupported,e.what());}
  catch(const ModelError& e){fail(Termination::InvalidModel,e.what());}
  catch(const EvidenceError& e){fail(Termination::NumericalFailure,e.what());}
  catch(const std::bad_alloc&){fail(Termination::MemoryLimit,"Scenario allocation failed");}
  catch(const std::exception& e){fail(Termination::BackendError,e.what());}
  // The Model overload's temporary snapshot has now been released as well.
  // Earlier stage witnesses remain timely historical results if cleanup used
  // the remaining allowance; only batch completion is downgraded here.
  if(budget&&out.completion==ScenarioBatchCompletion::Complete)
    if(const auto reason=budget->stop_reason())fail(*reason,"Whole batch budget stopped during final source cleanup");
  if(meter)out.work=meter->used;
  out.elapsed_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count();
  return out;
}
}
ScenarioBatchResult solve_scenarios(const ModelSnapshot& model,const std::vector<ScenarioDefinition>& definitions,const ScenarioBatchOptions& options) {
  return run([&]() -> const ModelSnapshot& {return model;},model.model_id,model.revision,definitions,options);
}
ScenarioBatchResult solve_scenarios(const Model& model,const std::vector<ScenarioDefinition>& definitions,const ScenarioBatchOptions& options) {
  return run([&]{return model.snapshot();},model.id(),model.revision(),definitions,options);
}
}}
