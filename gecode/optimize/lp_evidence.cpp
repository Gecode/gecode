#include <gecode/optimize/lp_evidence.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>
#include <type_traits>

namespace Gecode { namespace Optimize {
#ifdef GECODE_OPTIMIZE_TEST_LP_EVIDENCE
namespace Detail {
SolveResult lp_evidence_test_solve(LpEvidencePhase,const ModelSnapshot&,const SolveOptions&);
void lp_evidence_test_checkpoint(const char*,std::size_t);
}
#endif
namespace {
using Wide=long double;
using Clock=std::chrono::steady_clock;
constexpr double inf=std::numeric_limits<double>::infinity();
struct Failure : std::runtime_error {
  Termination termination;LpEvidenceReason reason;
  Failure(Termination t,LpEvidenceReason r,const char* text):std::runtime_error(text),termination(t),reason(r){}
};
[[noreturn]] void bad(const char* text){throw Failure(Termination::NumericalFailure,LpEvidenceReason::InvalidBackendData,text);}
[[noreturn]] void unsupported(const char* text){throw Failure(Termination::Unsupported,LpEvidenceReason::Unsupported,text);}
struct Budget {
  SolveBudget shared;Clock::time_point started;double seconds;
  Budget(const SolveOptions& options,Clock::time_point start):shared(options),started(start),seconds(options.time_limit_seconds){}
  double remaining() const {return std::max(0.0,seconds-std::chrono::duration<double>(Clock::now()-started).count());}
  std::optional<Termination> stop() const {
    if(shared.cancelled())return Termination::Cancelled;
    if(remaining()==0)return Termination::TimeLimit;
    if(shared.node_limit_reached())return Termination::NodeLimit;
    return {};
  }
  void check() const {if(const auto reason=stop())throw Failure(*reason,LpEvidenceReason::Stopped,"Whole LP evidence allowance stopped");}
};
struct Meter {
  Budget& budget;const LpEvidenceLimits& limits;std::size_t work=0,retained=0;
  void tick(std::size_t n=1){budget.check();if(n>limits.max_work-work)
    throw Failure(Termination::IterationLimit,LpEvidenceReason::ResourceLimit,"LP evidence coordinator work limit reached");work+=n;}
  void keep(std::size_t n){tick(0);if(n>limits.max_retained_slots-retained)
    throw Failure(Termination::MemoryLimit,LpEvidenceReason::ResourceLimit,"LP evidence retained-slot limit reached");retained+=n;}
};
void event(const char* point,std::size_t stage) {
#ifdef GECODE_OPTIMIZE_TEST_LP_EVIDENCE
  Detail::lp_evidence_test_checkpoint(point,stage);
#else
  (void)point;(void)stage;
#endif
}
struct Sum {
  Wide sum=0,correction=0;
  void add(Wide value){if(!std::isfinite(value))bad("Nonfinite original evidence arithmetic");
    const Wide next=sum+value;if(!std::isfinite(next))bad("Original evidence sum overflow");
    correction+=std::abs(sum)>=std::abs(value)?(sum-next)+value:(value-next)+sum;
    if(!std::isfinite(correction))bad("Original evidence compensation overflow");sum=next;}
  Wide value() const {const Wide v=sum+correction;if(!std::isfinite(v))bad("Original evidence sum overflow");return v;}
};
double narrow(Wide value){if(!std::isfinite(value)||std::abs(value)>std::numeric_limits<double>::max())bad("Evidence exceeds finite double representation");
  const double out=static_cast<double>(value);if(value!=0&&out==0)bad("Evidence conversion would erase a nonzero value");return out;}
void range(double value,double cap,const char* text){if(std::isfinite(value)&&std::abs(value)>=cap)unsupported(text);}
std::size_t add(std::size_t a,std::size_t b){if(b>std::numeric_limits<std::size_t>::max()-a)
  throw Failure(Termination::MemoryLimit,LpEvidenceReason::ResourceLimit,"LP evidence size arithmetic overflow");return a+b;}
void cap(std::size_t n,std::size_t limit,const char* text){if(n>limit)throw Failure(Termination::MemoryLimit,LpEvidenceReason::ResourceLimit,text);}
void visit(const ModelSnapshot& model,Meter& meter,bool retain=false){
  const auto charge=[&](std::size_t n){meter.tick(n);if(retain)meter.keep(n);};
  charge(model.variables.size());charge(model.rows.size());charge(model.objective.terms.size());
  for(const auto& v:model.variables)charge(v.name.size());
  for(const auto& r:model.rows){charge(r.terms.size());charge(r.name.size());}
  // Metadata is validated and owned, but active logical semantics are unsupported.
  for(const auto& i:model.indicators){charge(1);charge(i.terms.size());charge(i.generated_rows.size());charge(i.domains.size());}
  for(const auto& g:model.globals){charge(1);charge(g.name.size());
    if(g.payload.valueless_by_exception())throw ModelError("Global constraint has no payload");
    std::visit([&](const auto& data){using T=std::decay_t<decltype(data)>;
      if constexpr(std::is_same_v<T,AllDifferentData>||std::is_same_v<T,TableData>||std::is_same_v<T,RegularData>)charge(data.variables.size());
      if constexpr(std::is_same_v<T,ElementData>)charge(add(data.elements.size(),2));
      if constexpr(std::is_same_v<T,CumulativeData>){charge(data.starts.size());charge(data.durations.size());charge(data.heights.size());}
      if constexpr(std::is_same_v<T,CircuitData>)charge(data.successors.size());
      if constexpr(std::is_same_v<T,TableData>)for(const auto& tuple:data.tuples)charge(tuple.size());
      if constexpr(std::is_same_v<T,RegularData>){charge(data.transitions.size());charge(data.final_states.size());}
    },g.payload);}
}
void admit(const ModelSnapshot& source,const LpEvidenceOptions& options,Meter& meter){
  visit(source,meter);validate_structure(source);meter.tick(0);
  if(options.solve.backend==Backend::Native||options.solve.guarantee!=Guarantee::Numerical)
    unsupported("LP evidence requires Auto/HiGHS and Numerical guarantee");
  if(!options.solve.primal_start.empty())unsupported("LP evidence does not map caller primal starts to auxiliary models");
  for(const auto& i:source.indicators){meter.tick();if(i.active)unsupported("LP evidence does not relax active indicators");}
  for(const auto& g:source.globals){meter.tick();if(g.active)unsupported("LP evidence does not relax active globals");}
  for(const auto& v:source.variables){meter.tick();if(!v.active)continue;
    if(v.type!=VariableType::Continuous)unsupported("LP evidence requires original Continuous variables");
    range(v.lower,1e20,"Original variable bound exceeds auxiliary backend range");range(v.upper,1e20,"Original variable bound exceeds auxiliary backend range");}
  for(const auto& r:source.rows){meter.tick();if(!r.active)continue;
    range(r.lower,1e20,"Original row side exceeds auxiliary backend range");range(r.upper,1e20,"Original row side exceeds auxiliary backend range");
    for(const auto& t:r.terms){meter.tick();range(t.coefficient,1e15,"Original matrix coefficient exceeds auxiliary backend range");
      if(std::abs(t.coefficient)<=1e-12)unsupported("Nonzero original matrix coefficients <=1e-12 require rescaling");}}
  range(source.objective.offset,1e20,"Original objective offset exceeds supported source range");
  for(const auto& t:source.objective.terms){meter.tick();range(t.coefficient,1e20,"Original objective coefficient exceeds auxiliary backend range");}
}
struct Sizes {std::size_t variables=0,rows=0,nonzeros=0;};
Sizes sizes(const ModelSnapshot& source,LpEvidencePhase phase,Meter& meter){
  Sizes out;
  for(const auto& v:source.variables){meter.tick();if(!v.active)continue;
    if(phase==LpEvidencePhase::Farkas){out.rows=add(out.rows,1);out.variables=add(out.variables,std::isfinite(v.lower)+std::isfinite(v.upper));
      out.nonzeros=add(out.nonzeros,2*(std::isfinite(v.lower)+std::isfinite(v.upper)));}
    else out.variables=add(out.variables,1);}
  for(const auto& r:source.rows){meter.tick();if(!r.active)continue;
    if(phase==LpEvidencePhase::Farkas){const auto n=std::isfinite(r.lower)+std::isfinite(r.upper);out.variables=add(out.variables,n);
      for(int i=0;i<n;++i){out.nonzeros=add(out.nonzeros,r.terms.size());out.nonzeros=add(out.nonzeros,1);}}
    else {out.rows=add(out.rows,1);out.nonzeros=add(out.nonzeros,r.terms.size());}}
  if(phase==LpEvidencePhase::Farkas)out.rows=add(out.rows,1);
  cap(out.variables,meter.limits.max_auxiliary_variables,"LP evidence auxiliary variable limit reached");
  cap(out.rows,meter.limits.max_auxiliary_rows,"LP evidence auxiliary row limit reached");
  cap(out.nonzeros,meter.limits.max_auxiliary_nonzeros,"LP evidence auxiliary nonzero limit reached");
  const auto index_max=static_cast<std::size_t>(std::numeric_limits<int>::max());
  if(out.variables>=index_max||out.rows>=index_max||out.nonzeros>=index_max)unsupported("LP evidence auxiliary dimensions exceed backend index range");
  return out;
}
LpEvidenceStage build(const ModelSnapshot& source,LpEvidencePhase phase,Meter& meter){
  const auto dimensions=sizes(source,phase,meter);
  // Charge snapshots, copied result values/masks, mappings and construction scratch.
  meter.keep(add(add(dimensions.nonzeros,dimensions.rows),add(dimensions.variables,dimensions.variables)));
  meter.keep(add(dimensions.variables,dimensions.variables));
  meter.keep(dimensions.variables); // objective terms upper bound
  Model model;if(model.id()==source.model_id){Model distinct;model=std::move(distinct);}
  LpEvidenceStage stage;stage.phase=phase;stage.nonzeros=dimensions.nonzeros;stage.columns.reserve(dimensions.variables);
  std::vector<Variable> mapping(source.variables.size());meter.tick(mapping.size());
  if(phase!=LpEvidencePhase::Farkas){
    for(const auto& v:source.variables){meter.tick();if(!v.active)continue;double lower=v.lower,upper=v.upper;
      if(phase==LpEvidencePhase::Recession){lower=std::isfinite(v.lower)?0:-1;upper=std::isfinite(v.upper)?0:1;}
      mapping[v.variable.id]=model.add_continuous(lower,upper);stage.columns.push_back({LpEvidenceColumnKind::SourceVariable,static_cast<std::size_t>(v.variable.id),{}});}
    for(const auto& r:source.rows){meter.tick();if(!r.active)continue;std::vector<Term> terms;terms.reserve(r.terms.size());
      for(const auto& t:r.terms){meter.tick();terms.push_back({mapping[t.variable.id],t.coefficient});}
      model.add_row(terms,phase==LpEvidencePhase::Recession?(std::isfinite(r.lower)?0:-inf):r.lower,
                           phase==LpEvidencePhase::Recession?(std::isfinite(r.upper)?0:inf):r.upper);}
    if(phase==LpEvidencePhase::Recession){std::vector<Term> terms;terms.reserve(source.objective.terms.size());
      const double sign=source.objective.sense==ObjectiveSense::Minimize?1:-1;
      for(const auto& t:source.objective.terms){meter.tick();terms.push_back({mapping[t.variable.id],sign*t.coefficient});}model.minimize(terms);}
  } else {
    std::vector<std::vector<Term>> stationarity(source.variables.size());std::vector<Term> normalization,objective;
    normalization.reserve(dimensions.variables);objective.reserve(dimensions.variables);
    const auto multiplier=[&](LpEvidenceColumnKind kind,std::size_t slot,LpEvidenceSide side,double endpoint){
      meter.tick();const auto v=model.add_continuous(0,1);stage.columns.push_back({kind,slot,side});normalization.push_back({v,1});
      const double sign=side==LpEvidenceSide::Lower?1:-1;if(endpoint!=0)objective.push_back({v,sign*endpoint});return v;};
    for(const auto& r:source.rows){meter.tick();if(!r.active)continue;
      for(const auto side:{LpEvidenceSide::Lower,LpEvidenceSide::Upper}){const double endpoint=side==LpEvidenceSide::Lower?r.lower:r.upper;
        if(!std::isfinite(endpoint))continue;const auto v=multiplier(LpEvidenceColumnKind::RowSide,r.constraint.id,side,endpoint);
        for(const auto& t:r.terms){meter.tick();stationarity[t.variable.id].push_back({v,(side==LpEvidenceSide::Lower?1:-1)*t.coefficient});}}}
    for(const auto& v:source.variables){meter.tick();if(!v.active)continue;
      for(const auto side:{LpEvidenceSide::Lower,LpEvidenceSide::Upper}){const double endpoint=side==LpEvidenceSide::Lower?v.lower:v.upper;
        if(!std::isfinite(endpoint))continue;const auto multiplier_var=multiplier(LpEvidenceColumnKind::VariableSide,v.variable.id,side,endpoint);
        stationarity[v.variable.id].push_back({multiplier_var,side==LpEvidenceSide::Lower?1.0:-1.0});}}
    for(const auto& v:source.variables){meter.tick();if(v.active)model.add_row(stationarity[v.variable.id],0,0);}
    model.add_row(normalization,-inf,1);model.maximize(objective);
  }
  meter.tick(dimensions.nonzeros);stage.auxiliary_model=std::make_shared<const ModelSnapshot>(model.snapshot());
  meter.tick(0);validate_structure(*stage.auxiliary_model);meter.tick(0);return stage;
}
bool known(Termination t){switch(t){case Termination::Unknown:case Termination::Optimal:case Termination::Infeasible:case Termination::Unbounded:
  case Termination::InfeasibleOrUnbounded:case Termination::TimeLimit:case Termination::NodeLimit:case Termination::MemoryLimit:case Termination::IterationLimit:
  case Termination::SolutionLimit:case Termination::ObjectiveLimit:case Termination::Cancelled:case Termination::NumericalFailure:case Termination::Unsupported:
  case Termination::InvalidModel:case Termination::BackendError:return true;}return false;}
bool complete(Termination t){return t==Termination::Optimal||t==Termination::Infeasible;}
void check_stage(LpEvidenceStage& stage,const SolveOptions& options,Meter& meter){
  const auto& model=*stage.auxiliary_model;auto& r=*stage.auxiliary_result;
  if(!known(r.termination)||r.guarantee!=Guarantee::Numerical)bad("Invalid auxiliary status/guarantee");
  if(r.model_id!=model.model_id||r.revision!=model.revision)bad("Auxiliary result owner/revision mismatch");
  if(r.active_variables.size()!=model.variables.size())bad("Auxiliary result active-mask dimension mismatch");
  for(std::size_t i=0;i<model.variables.size();++i){meter.tick();if(r.active_variables[i]!=model.variables[i].active)bad("Auxiliary active mask mismatch");}
  if((r.objective&&!std::isfinite(*r.objective))||(r.best_bound&&std::isnan(*r.best_bound))||!std::isfinite(r.elapsed_seconds)||r.elapsed_seconds<0)
    bad("Nonfinite auxiliary scalar evidence");
  if((r.absolute_gap&&(std::isnan(*r.absolute_gap)||*r.absolute_gap<0))||(r.relative_gap&&(!std::isfinite(*r.relative_gap)||*r.relative_gap<0))||
      (r.native_backend_gap&&(!std::isfinite(*r.native_backend_gap)||*r.native_backend_gap<0)))bad("Malformed auxiliary gap");
  const bool supplied=!r.values.empty()||r.objective||r.solution_validated;
  if(supplied||model.variables.empty()){
    stage.candidate_examined=true;if(r.values.size()!=model.variables.size())bad("Auxiliary candidate dimension mismatch");
    visit(model,meter);stage.auxiliary_check=validate(model,r.values,options.feasibility_tolerance,options.integrality_tolerance);meter.tick(0);
    if(!stage.auxiliary_check.valid&&supplied)bad("Auxiliary candidate fails independent validation");
    if(r.objective&&(!stage.auxiliary_check.objective||*r.objective!=*stage.auxiliary_check.objective))bad("Auxiliary objective differs from independent evaluation");
    if(stage.auxiliary_check.valid&&r.termination==Termination::Infeasible)bad("Auxiliary infeasibility contradicts a feasible assignment");
    if(r.solution_validated&&(!r.objective||!stage.auxiliary_check.valid))bad("Auxiliary solution flag has no checked objective/witness");
  }
  if(r.termination==Termination::Optimal&&(!r.has_solution()||!stage.auxiliary_check.valid))bad("Auxiliary Optimal has no validated finite candidate");
  if(r.termination==Termination::Unbounded||(stage.phase!=LpEvidencePhase::FeasibleBase&&
      (r.termination==Termination::Infeasible||r.termination==Termination::InfeasibleOrUnbounded)))
    throw Failure(Termination::BackendError,LpEvidenceReason::InvalidBackendData,"Auxiliary status contradicts its zero feasible point or bounded objective");
  try{const auto absolute=r.absolute_gap,relative=r.relative_gap;r.update_gaps(model.objective.sense);
    if((absolute&&absolute!=r.absolute_gap)||(relative&&relative!=r.relative_gap))bad("Auxiliary supplied gap disagrees with objective and bound");}
  catch(const ModelError&){bad("Auxiliary bound contradicts its objective");}
}
}
namespace Detail {
struct LpEvidenceAccess {
  static std::shared_ptr<LpEvidence> create(const ModelSnapshot& source,const LpEvidenceOptions& options,Meter& meter){
    auto out=std::shared_ptr<LpEvidence>(new LpEvidence);visit(source,meter,true);out->source_=source;
    out->tolerances_=options.checks;out->primal_tolerance_=options.solve.feasibility_tolerance;
    if(options.request!=LpEvidenceRequest::Farkas)out->primal_={LpEvidenceState::Unavailable,LpEvidenceReason::NoFeasibleBase,"No checked base point and direction"};
    if(options.request!=LpEvidenceRequest::PrimalRay)out->farkas_={LpEvidenceState::Unavailable,LpEvidenceReason::NoContradiction,"No checked contradiction"};
    return out;
  }
  static void prepare(LpEvidence& out,const LpEvidenceOptions& options,Meter& meter){
    const auto& source=out.source_;
    meter.keep(add(add(source.variables.size(),source.variables.size()),source.rows.size()));
    meter.keep(add(source.rows.size(),source.variables.size()));
    // Preflight all potentially requested transforms before any backend work.
    if(options.request!=LpEvidenceRequest::Farkas){sizes(source,LpEvidencePhase::FeasibleBase,meter);sizes(source,LpEvidencePhase::Recession,meter);}
    if(options.request!=LpEvidenceRequest::PrimalRay)sizes(source,LpEvidencePhase::Farkas,meter);
    out.stages_.reserve(3);
    if(options.request!=LpEvidenceRequest::Farkas){out.stages_.push_back(build(source,LpEvidencePhase::FeasibleBase,meter));out.stages_.push_back(build(source,LpEvidencePhase::Recession,meter));}
    if(options.request!=LpEvidenceRequest::PrimalRay)out.stages_.push_back(build(source,LpEvidencePhase::Farkas,meter));
  }
  static bool stage(LpEvidence& out,std::size_t index,const LpEvidenceOptions& options,Meter& meter,LpEvidenceResult& result){
    auto& stage=out.stages_[index];event("before_solve",index);meter.tick(0);
    if(result.attempted_calls>=options.limits.max_auxiliary_solves)throw Failure(Termination::IterationLimit,LpEvidenceReason::ResourceLimit,"LP evidence auxiliary-solve call limit reached");
    {
      auto adjusted=options.solve;adjusted.backend=Backend::Highs;adjusted.guarantee=Guarantee::Numerical;
      adjusted.time_limit_seconds=meter.budget.remaining();adjusted.cancellation=meter.budget.shared.cancellation();
      stage.attempted=true;++result.attempted_calls;
#ifdef GECODE_OPTIMIZE_TEST_LP_EVIDENCE
      stage.auxiliary_result=lp_evidence_test_solve(stage.phase,*stage.auxiliary_model,adjusted);
#else
      stage.auxiliary_result=solve(*stage.auxiliary_model,adjusted);
#endif
      event("after_solve",index);meter.tick(0);check_stage(stage,options.solve,meter);event("after_check",index);meter.tick(0);
    }
    event("stage_cleanup",index);meter.tick(0);return stage.candidate_examined&&stage.auxiliary_check.valid;
  }
  static void base(LpEvidence& out,const LpEvidenceStage& stage,Meter& meter){
    auto& data=out.primal_data_;const auto& source=out.source_;
    std::vector<double> point(source.variables.size(),std::numeric_limits<double>::quiet_NaN());meter.tick(point.size());
    for(std::size_t i=0;i<stage.columns.size();++i){meter.tick();point[stage.columns[i].original_slot]=stage.auxiliary_result->values[i];}
    visit(source,meter);auto check=validate(source,point,out.primal_tolerance_,1e-6);meter.tick(0);
    if(!check.valid)throw Failure(Termination::NumericalFailure,LpEvidenceReason::FailedOriginalChecks,"Auxiliary base fails original source validation");
    data.base_point=std::move(point);data.base_check=std::move(check);
  }
  static void primal(LpEvidence& out,const LpEvidenceStage& stage,Meter& meter){
    auto& data=out.primal_data_;const auto& source=out.source_;
    if(!data.base_check.valid){out.primal_={LpEvidenceState::Unavailable,LpEvidenceReason::NoFeasibleBase,"An improving direction alone does not establish a feasible source"};return;}
    Wide scale=0;for(const auto value:stage.auxiliary_result->values){meter.tick();if(!std::isfinite(value))bad("Nonfinite direction candidate");scale=std::max(scale,std::abs(static_cast<Wide>(value)));}
    if(scale==0){data.direction_scale=0;out.primal_={LpEvidenceState::Unavailable,LpEvidenceReason::NoImprovingDirection,"Auxiliary returned the zero direction"};return;}
    std::vector<double> direction(source.variables.size(),std::numeric_limits<double>::quiet_NaN());meter.tick(direction.size());
    for(std::size_t i=0;i<stage.columns.size();++i){meter.tick();direction[stage.columns[i].original_slot]=narrow(static_cast<Wide>(stage.auxiliary_result->values[i])/scale);}
    Wide variable_error=0,row_error=0;std::vector<double> activity(source.rows.size(),std::numeric_limits<double>::quiet_NaN());
    for(const auto& v:source.variables){meter.tick();if(!v.active)continue;const Wide value=direction[v.variable.id];
      if(std::isfinite(v.lower))variable_error=std::max(variable_error,-value);if(std::isfinite(v.upper))variable_error=std::max(variable_error,value);}
    for(const auto& r:source.rows){meter.tick();if(!r.active)continue;Sum sum;for(const auto& t:r.terms){meter.tick();sum.add(static_cast<Wide>(t.coefficient)*direction[t.variable.id]);}
      const Wide value=sum.value();activity[r.constraint.id]=narrow(value);if(std::isfinite(r.lower))row_error=std::max(row_error,-value);if(std::isfinite(r.upper))row_error=std::max(row_error,value);}
    Sum slope;const Wide sign=source.objective.sense==ObjectiveSense::Minimize?1:-1;
    for(const auto& t:source.objective.terms){meter.tick();slope.add(sign*static_cast<Wide>(t.coefficient)*direction[t.variable.id]);}
    data.direction_scale=narrow(scale);data.normalized_objective_slope=narrow(slope.value());
    data.max_variable_recession_violation=narrow(variable_error);data.max_row_recession_violation=narrow(row_error);
    if(variable_error>out.tolerances_.recession||row_error>out.tolerances_.recession)
      throw Failure(Termination::NumericalFailure,LpEvidenceReason::FailedOriginalChecks,"Normalized direction fails original recession checks");
    if(slope.value()>=-out.tolerances_.minimum_improvement){out.primal_={LpEvidenceState::Unavailable,LpEvidenceReason::NoImprovingDirection,"No strict normalized objective improvement"};return;}
    data.direction=std::move(direction);data.row_direction=std::move(activity);out.primal_={LpEvidenceState::Available,LpEvidenceReason::None,"Checked numerical base point and improving recession direction; not an exact proof"};
  }
  static void farkas(LpEvidence& out,const LpEvidenceStage& stage,Meter& meter){
    auto& data=out.farkas_data_;const auto& source=out.source_;std::vector<Sum> signed_rows(source.rows.size());meter.tick(signed_rows.size());
    for(std::size_t i=0;i<stage.columns.size();++i){meter.tick();const auto& map=stage.columns[i];if(map.kind!=LpEvidenceColumnKind::RowSide)continue;
      signed_rows[map.original_slot].add((map.side==LpEvidenceSide::Lower?1.0L:-1.0L)*stage.auxiliary_result->values[i]);}
    Wide scale=0;for(const auto& row:signed_rows){meter.tick();scale=std::max(scale,std::abs(row.value()));}
    if(scale==0){data.multiplier_scale=0;out.farkas_={LpEvidenceState::Unavailable,LpEvidenceReason::NoContradiction,"No nonzero signed original row multiplier"};return;}
    std::vector<LpFarkasEntry> rows(source.rows.size()),columns(source.variables.size());std::vector<Sum> transpose(source.variables.size());
    for(const auto& r:source.rows){meter.tick();auto& entry=rows[r.constraint.id];entry.active=r.active;if(!r.active)continue;
      entry.multiplier=narrow(signed_rows[r.constraint.id].value()/scale);
      for(const auto& t:r.terms){meter.tick();transpose[t.variable.id].add(static_cast<Wide>(t.coefficient)*entry.multiplier);}}
    Wide max_stationarity=0;for(const auto& v:source.variables){meter.tick();auto& entry=columns[v.variable.id];entry.active=v.active;if(!v.active)continue;
      entry.multiplier=narrow(-transpose[v.variable.id].value());Sum residual;residual.add(transpose[v.variable.id].sum);residual.add(transpose[v.variable.id].correction);residual.add(entry.multiplier);
      max_stationarity=std::max(max_stationarity,std::abs(residual.value()));}
    Sum margin;
    const auto contribution=[&](LpFarkasEntry& entry,double lower,double upper){meter.tick();if(!entry.active||entry.multiplier==0)return;
      entry.side=entry.multiplier>0?LpEvidenceSide::Lower:LpEvidenceSide::Upper;const double endpoint=entry.multiplier>0?lower:upper;
      if(!std::isfinite(endpoint))throw Failure(Termination::NumericalFailure,LpEvidenceReason::FailedOriginalChecks,"Nonzero Farkas multiplier requires an infinite original side");
      const Wide term=static_cast<Wide>(entry.multiplier)*endpoint;entry.contribution=narrow(term);margin.add(term);};
    data.multiplier_scale=narrow(scale);data.max_stationarity=narrow(max_stationarity);
    for(const auto& r:source.rows)contribution(rows[r.constraint.id],r.lower,r.upper);
    for(const auto& v:source.variables)contribution(columns[v.variable.id],v.lower,v.upper);
    data.contradiction_margin=narrow(margin.value());data.rows=std::move(rows);data.columns=std::move(columns);
    if(max_stationarity>out.tolerances_.stationarity)throw Failure(Termination::NumericalFailure,LpEvidenceReason::FailedOriginalChecks,"Published Farkas stationarity exceeds original tolerance");
    if(margin.value()<=out.tolerances_.minimum_contradiction){out.farkas_={LpEvidenceState::Unavailable,LpEvidenceReason::NoContradiction,"No strict positive original contradiction margin"};return;}
    out.farkas_={LpEvidenceState::Available,LpEvidenceReason::None,"Checked numerical original-side contradiction; not an exact certificate"};
    if(out.primal_data_.base_check.valid)throw Failure(Termination::NumericalFailure,LpEvidenceReason::InconsistentEvidence,"Accepted original feasible point and positive Farkas margin are numerically inconsistent");
  }
  static void fail(LpEvidence& out,LpEvidenceReason reason,const char* message,bool rejected) noexcept {
    for(auto* group:{&out.primal_,&out.farkas_})if(group->state!=LpEvidenceState::NotRequested){
      group->state=rejected?LpEvidenceState::Rejected:LpEvidenceState::Unavailable;group->reason=reason;group->message.clear();
      try{group->message=message;}catch(...){}
    }
  }
};
}
namespace {
template<class Snapshot>
LpEvidenceResult run(Snapshot snapshot,ModelId owner,Revision revision,const LpEvidenceOptions& options){
  const auto started=Clock::now();LpEvidenceResult result;result.model_id=owner;result.revision=revision;
  std::optional<Budget> budget;std::optional<Meter> meter;std::shared_ptr<LpEvidence> artifact;
  const auto fail=[&](Termination termination,LpEvidenceReason reason,const char* message){
    const bool rejected=reason==LpEvidenceReason::InvalidModel||reason==LpEvidenceReason::InvalidBackendData||reason==LpEvidenceReason::FailedOriginalChecks||reason==LpEvidenceReason::InconsistentEvidence||reason==LpEvidenceReason::Unsupported;
    result.completion=rejected?LpEvidenceCompletion::Rejected:LpEvidenceCompletion::Interrupted;result.stop_reason=termination;
    if(artifact)Detail::LpEvidenceAccess::fail(*artifact,reason,message,rejected);
    try{result.message=message;}catch(...){result.message.clear();}
  };
  try{
    options.validate();budget.emplace(options.solve,started);meter.emplace(Meter{*budget,options.limits});meter->tick(0);
    {
      const auto& source=snapshot();event("source_copy",0);meter->tick(0);admit(source,options,*meter);
      artifact=Detail::LpEvidenceAccess::create(source,options,*meter);result.evidence=artifact;
      Detail::LpEvidenceAccess::prepare(*artifact,options,*meter);event("admission",0);meter->tick(0);
#ifndef GECODE_OPTIMIZE_TEST_LP_EVIDENCE
      if(!capabilities(Backend::Highs).available)unsupported("Numerical HiGHS backend is unavailable for LP evidence recovery");
#endif
      result.completion=LpEvidenceCompletion::Complete;
      bool base=false;
      const auto process=[&](std::size_t index){
        const bool candidate=Detail::LpEvidenceAccess::stage(*artifact,index,options,*meter,result);
        const auto& stage=artifact->stages()[index];
        if(candidate){if(stage.phase==LpEvidencePhase::FeasibleBase){Detail::LpEvidenceAccess::base(*artifact,stage,*meter);base=true;}
          if(stage.phase==LpEvidencePhase::Recession)Detail::LpEvidenceAccess::primal(*artifact,stage,*meter);
          if(stage.phase==LpEvidencePhase::Farkas)Detail::LpEvidenceAccess::farkas(*artifact,stage,*meter);}
        event("evidence_check",index);meter->tick(0);
        if(!complete(stage.auxiliary_result->termination)){result.completion=LpEvidenceCompletion::Interrupted;result.stop_reason=stage.auxiliary_result->termination;result.message="Auxiliary stage stopped before definitive completion";return false;}
        return true;
      };
      if(options.request==LpEvidenceRequest::Farkas)process(0);
      else if(process(0)){
        if(base){if(process(1)&&options.request==LpEvidenceRequest::Both)process(2);}
        else if(options.request==LpEvidenceRequest::Automatic||options.request==LpEvidenceRequest::Both)process(2);
      }
      event("publication",artifact->stages().size());meter->tick(0);
    }
    event("source_cleanup",artifact?artifact->stages().size():0);meter->tick(0);
  }catch(const Failure& e){fail(e.termination,e.reason,e.what());}
  catch(const ModelError& e){fail(Termination::InvalidModel,LpEvidenceReason::InvalidModel,e.what());}
  catch(const std::bad_alloc&){fail(Termination::MemoryLimit,LpEvidenceReason::AllocationFailure,"LP evidence allocation failed");}
  catch(const std::exception& e){fail(Termination::BackendError,LpEvidenceReason::InvalidBackendData,e.what());}
  if(budget)if(const auto stopped=budget->stop())fail(*stopped,LpEvidenceReason::Stopped,"Whole LP evidence budget stopped during final cleanup");
  if(meter)result.work=meter->work;
  result.elapsed_seconds=std::chrono::duration<double>(Clock::now()-started).count();return result;
}
std::size_t variable_slot(const LpEvidence& out,Variable variable){
  if(variable.model_id!=out.id()||variable.id>=out.source().variables.size()||!out.source().variables[variable.id].active)throw ModelError("LP evidence variable is foreign, missing or deleted");return variable.id;}
std::size_t row_slot(const LpEvidence& out,Constraint row){
  if(row.model_id!=out.id()||row.id>=out.source().rows.size()||!out.source().rows[row.id].active)throw ModelError("LP evidence row is foreign, missing or deleted");return row.id;}
}
void LpEvidenceTolerances::validate() const {for(double value:{recession,stationarity,minimum_improvement,minimum_contradiction})
  if(!std::isfinite(value)||value<0)throw ModelError("LP evidence tolerances must be finite and nonnegative");}
void LpEvidenceOptions::validate() const {solve.validate();checks.validate();switch(request){case LpEvidenceRequest::Automatic:case LpEvidenceRequest::PrimalRay:case LpEvidenceRequest::Farkas:case LpEvidenceRequest::Both:return;}throw ModelError("Unknown LP evidence request");}
const ModelSnapshot& LpEvidence::source() const noexcept{return source_;}
ModelId LpEvidence::id() const noexcept{return source_.model_id;}
Revision LpEvidence::revision() const noexcept{return source_.revision;}
const LpEvidenceTolerances& LpEvidence::tolerances() const noexcept{return tolerances_;}
double LpEvidence::primal_tolerance() const noexcept{return primal_tolerance_;}
const LpEvidenceGroup& LpEvidence::primal_ray() const noexcept{return primal_;}
const LpEvidenceGroup& LpEvidence::farkas() const noexcept{return farkas_;}
const LpPrimalEvidence& LpEvidence::primal_data() const noexcept{return primal_data_;}
const LpFarkasEvidence& LpEvidence::farkas_data() const noexcept{return farkas_data_;}
const std::vector<LpEvidenceStage>& LpEvidence::stages() const noexcept{return stages_;}
double LpEvidence::base_value(Variable variable) const {const auto slot=variable_slot(*this,variable);if(!primal_data_.base_check.valid||slot>=primal_data_.base_point.size())throw ModelError("No checked original LP evidence base point");return primal_data_.base_point[slot];}
double LpEvidence::direction_value(Variable variable) const {const auto slot=variable_slot(*this,variable);if(primal_.state!=LpEvidenceState::Available||slot>=primal_data_.direction.size())throw ModelError("No available LP primal-ray evidence");return primal_data_.direction[slot];}
const LpFarkasEntry& LpEvidence::row_multiplier(Constraint row) const {const auto slot=row_slot(*this,row);if(farkas_.state!=LpEvidenceState::Available||slot>=farkas_data_.rows.size())throw ModelError("No available LP Farkas evidence");return farkas_data_.rows[slot];}
const LpFarkasEntry& LpEvidence::column_multiplier(Variable variable) const {const auto slot=variable_slot(*this,variable);if(farkas_.state!=LpEvidenceState::Available||slot>=farkas_data_.columns.size())throw ModelError("No available LP Farkas evidence");return farkas_data_.columns[slot];}
LpEvidenceResult analyze_lp_evidence(const ModelSnapshot& model,const LpEvidenceOptions& options){return run([&]()->const ModelSnapshot&{return model;},model.model_id,model.revision,options);}
LpEvidenceResult analyze_lp_evidence(const Model& model,const LpEvidenceOptions& options){return run([&]{return model.snapshot();},model.id(),model.revision(),options);}
}}
