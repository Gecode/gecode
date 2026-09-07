#include <gecode/optimize/lp_sensitivity.hpp>
#include <gecode/optimize/lp_sensitivity_backend.hpp>
#include <gecode/optimize/lp_basis_detail.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <set>
#include <type_traits>

namespace Gecode { namespace Optimize {
namespace {
using Wide=long double;
using Clock=std::chrono::steady_clock;
constexpr std::size_t missing=std::numeric_limits<std::size_t>::max();
struct Failure : std::runtime_error {
  LpSensitivityReason reason;std::optional<Termination> stop;
  Failure(LpSensitivityReason r,const char* text,std::optional<Termination> s={})
    :std::runtime_error(text),reason(r),stop(s){}
};
[[noreturn]] void fail(LpSensitivityReason r,const char* text) {throw Failure(r,text);}
struct Sum {
  Wide sum=0,correction=0;
  void add(Wide x) {
    if(!std::isfinite(x))fail(LpSensitivityReason::FailedLinearSolveChecks,"Nonfinite sensitivity arithmetic");
    Wide n=sum+x;correction+=std::abs(sum)>=std::abs(x)?(sum-n)+x:(x-n)+sum;sum=n;
    if(!std::isfinite(sum)||!std::isfinite(correction))
      fail(LpSensitivityReason::FailedLinearSolveChecks,"Sensitivity accumulation overflow");
  }
  Wide value() const {return sum+correction;}
};
double narrow(Wide x,LpSensitivityReason reason=LpSensitivityReason::FailedLinearSolveChecks) {
  if(!std::isfinite(x)||std::abs(x)>std::numeric_limits<double>::max())fail(reason,"Finite sensitivity value exceeds double representation");
  const double v=static_cast<double>(x);
  if(x!=0&&v==0)fail(reason,"Sensitivity conversion would erase a nonzero value");
  return v;
}
std::size_t plus(std::size_t a,std::size_t b) {
  if(b>missing-a)fail(LpSensitivityReason::ResourceLimit,"Sensitivity size overflow");return a+b;
}
SolveOptions budget_options(const LpSensitivityOptions& o) {
  SolveOptions s;s.time_limit_seconds=o.time_limit_seconds;s.cancellation=o.cancellation;return s;
}
bool known(Termination t) {
  switch(t) {
  case Termination::Unknown:case Termination::Optimal:case Termination::Infeasible:
  case Termination::Unbounded:case Termination::InfeasibleOrUnbounded:case Termination::TimeLimit:
  case Termination::NodeLimit:case Termination::MemoryLimit:case Termination::IterationLimit:
  case Termination::SolutionLimit:case Termination::ObjectiveLimit:case Termination::Cancelled:
  case Termination::NumericalFailure:case Termination::Unsupported:case Termination::InvalidModel:
  case Termination::BackendError:return true;
  }return false;
}
struct Inequality {Wide a,b;LpSensitivityLimiter limiter;double tolerance;};
}
namespace Detail {
#ifdef GECODE_OPTIMIZE_TEST_LP_SENSITIVITY
std::unique_ptr<LpSensitivityFactor> lp_sensitivity_test_factor(const ModelSnapshot&,const LpBasis&,double,const SolveBudget&);
void lp_sensitivity_test_checkpoint(const char*,std::size_t);
#endif
struct LpSensitivityAccess {
  const LpSensitivityOptions& opts;Clock::time_point started;SolveBudget budget;
  LpSensitivityWork work;
  std::shared_ptr<LpSensitivity> out;
  std::unique_ptr<LpSensitivityFactor> factor;
  std::vector<std::vector<std::pair<std::size_t,double>>> matrix;
  std::vector<LpSensitivityEntity> entities;
  std::vector<std::size_t> col_map,row_map,basic,position;
  std::vector<LpBasisStatus> status;
  std::vector<double> lower,upper;
  std::vector<Wide> point,cost,dual,reduced;
  Wide sense=1;
  LpSensitivityAccess(const LpSensitivityOptions& options,Clock::time_point start)
    :opts(options),started(start),budget(budget_options(options)){}
  const ModelSnapshot& source() const {return out->original_.observations->source();}
  void check() const {
    if(budget.cancelled())throw Failure(LpSensitivityReason::Stopped,"LP sensitivity cancelled",Termination::Cancelled);
    if(std::chrono::duration<double>(Clock::now()-started).count()>=opts.time_limit_seconds)
      throw Failure(LpSensitivityReason::Stopped,"LP sensitivity time limit reached",Termination::TimeLimit);
  }
  void tick(std::size_t n=1) {
    check();if(n>opts.limits.max_work-work.coordinator_visits)
      throw Failure(LpSensitivityReason::ResourceLimit,"LP sensitivity coordinator work limit reached",Termination::IterationLimit);
    work.coordinator_visits+=n;
  }
  void keep(std::size_t n) {
    tick(0);if(n>opts.limits.max_retained_slots-work.retained_slots)
      throw Failure(LpSensitivityReason::ResourceLimit,"LP sensitivity retained-slot limit reached",Termination::MemoryLimit);
    work.retained_slots+=n;
  }
  void cap(std::size_t n,std::size_t maximum,const char* text) {
    tick(0);if(n>maximum)throw Failure(LpSensitivityReason::ResourceLimit,text,Termination::MemoryLimit);
    if(n>static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()))
      fail(LpSensitivityReason::ResourceLimit,"Sensitivity count exceeds ptrdiff_t");
  }
  void event(const char* text,std::size_t index=0) {
#ifdef GECODE_OPTIMIZE_TEST_LP_SENSITIVITY
    lp_sensitivity_test_checkpoint(text,index);
#else
    (void)text;(void)index;
#endif
    check();
  }
  std::size_t entity_slot(const LpSensitivityEntity& entity) const {
    if(const auto* v=std::get_if<Variable>(&entity)) {
      if(v->model_id!=source().model_id||v->id>=col_map.size()||col_map[v->id]==missing)
        fail(LpSensitivityReason::ChangedBasis,"Foreign/deleted factor column");return col_map[v->id];
    }
    const auto& r=std::get<Constraint>(entity);
    if(r.model_id!=source().model_id||r.id>=row_map.size()||row_map[r.id]==missing)
      fail(LpSensitivityReason::ChangedBasis,"Foreign/deleted factor row");return row_map[r.id];
  }
  void prepare(const LpObservedResult& original) {
    opts.validate();event("before_copy");
    if(!original.observations)fail(LpSensitivityReason::InvalidSource,"Missing owning LP observations");
    const auto& m=original.observations->source();
    cap(opts.parameters.size(),opts.limits.max_requests,"Too many sensitivity requests");
    std::size_t nr=0,nc=0,nz=0,slots=plus(plus(m.variables.size(),m.rows.size()),plus(m.indicators.size(),m.globals.size()));
    slots=plus(slots,m.objective.terms.size());tick(slots);
    for(const auto& v:m.variables){tick(v.name.size());slots=plus(slots,v.name.size());nc+=v.active;}
    for(const auto& r:m.rows){tick(plus(r.name.size(),r.terms.size()));slots=plus(slots,plus(r.name.size(),r.terms.size()));nr+=r.active;if(r.active)nz=plus(nz,r.terms.size());}
    for(const auto& g:m.globals){tick();if(g.active)fail(LpSensitivityReason::Unsupported,"Sensitivity does not support original globals");}
    for(const auto& i:m.indicators){tick();if(i.active)fail(LpSensitivityReason::Unsupported,"Sensitivity does not support original indicators");}
    // Inactive semantic payloads are still copied by the owning basis factory.
    for(const auto& i:m.indicators){const auto n=plus(plus(i.terms.size(),i.generated_rows.size()),i.domains.size());tick(n);slots=plus(slots,n);}
    for(const auto& g:m.globals){tick(g.name.size());slots=plus(slots,g.name.size());
      std::visit([&](const auto& d){using T=std::decay_t<decltype(d)>;std::size_t n=0;
        if constexpr(std::is_same_v<T,AllDifferentData>||std::is_same_v<T,TableData>||std::is_same_v<T,RegularData>)n=d.variables.size();
        if constexpr(std::is_same_v<T,ElementData>)n=plus(d.elements.size(),2);
        if constexpr(std::is_same_v<T,CumulativeData>)n=plus(plus(d.starts.size(),d.durations.size()),d.heights.size());
        if constexpr(std::is_same_v<T,CircuitData>)n=d.successors.size();
        if constexpr(std::is_same_v<T,TableData>){n=plus(n,d.tuples.size());for(const auto& tuple:d.tuples){tick();n=plus(n,tuple.size());}}
        if constexpr(std::is_same_v<T,RegularData>)n=plus(n,plus(d.transitions.size(),d.final_states.size()));
        tick(n);slots=plus(slots,n);
      },g.payload);}
    cap(nr,opts.limits.max_rows,"Too many sensitivity basis rows");cap(nc,opts.limits.max_columns,"Too many sensitivity columns");
    cap(nz,opts.limits.max_nonzeros,"Too many sensitivity nonzeros");
    if(nr&&nr>opts.limits.max_factor_entries/nr)fail(LpSensitivityReason::ResourceLimit,"Sensitivity factor-fill admission exceeded");
    keep(plus(slots,slots));keep(plus(original.result.values.size(),original.result.active_variables.size()));
    cap(plus(nr,nc),missing/32,"Sensitivity workspace size overflow");keep(32*plus(nr,nc));
    keep(plus(nz,nz));keep(plus(m.variables.size(),m.rows.size()));keep(opts.parameters.size());
    validate_structure(m);tick(0);
    if(opts.backend!=Backend::Auto&&opts.backend!=Backend::Highs)
      fail(LpSensitivityReason::Unsupported,"Sensitivity requires the HiGHS numerical factorization backend");
    for(const auto& v:m.variables)if(v.active&&v.type!=VariableType::Continuous)
      fail(LpSensitivityReason::Unsupported,"Sensitivity requires original Continuous variables");
    for(const auto& r:m.rows)if(r.active&&r.terms.empty())fail(LpSensitivityReason::NoBasis,"Sensitivity cannot map elided constant rows");
    const auto& r=original.result;
    if(!known(r.termination)||r.guarantee!=Guarantee::Numerical||r.model_id!=m.model_id||r.revision!=m.revision)
      fail(LpSensitivityReason::InvalidSource,"Invalid LP result status/guarantee/identity");
    if(r.termination!=Termination::Optimal)fail(LpSensitivityReason::NotOptimal,"Sensitivity requires an optimal LP result");
    const auto& o=*original.observations;
    if(o.basis().state!=LpObservationState::Available)fail(LpSensitivityReason::NoBasis,"Sensitivity requires an available original basis");
    if(o.primal_rows().state!=LpObservationState::Available||o.dual_point().state!=LpObservationState::Available||!o.checks().accepted)
      fail(LpSensitivityReason::FailedReferenceChecks,"Sensitivity requires accepted O1 primal/dual/KKT observations");
    if(!nr||!nc)fail(LpSensitivityReason::NoBasis,"Zero-row/column sensitivity is not implemented");
    if(!r.solution_validated||r.values.size()!=m.variables.size()||r.active_variables.size()!=m.variables.size()||!r.objective||!std::isfinite(*r.objective))
      fail(LpSensitivityReason::InvalidSource,"Missing valid original LP assignment/objective");
    if(!std::isfinite(r.elapsed_seconds)||r.elapsed_seconds<0)fail(LpSensitivityReason::InvalidSource,"Invalid original LP elapsed time");
    for(const auto* scalar:{&r.best_bound,&r.absolute_gap,&r.relative_gap,&r.native_backend_gap})
      if(*scalar&&!std::isfinite(**scalar))fail(LpSensitivityReason::InvalidSource,"Nonfinite original LP scalar");
    if((r.absolute_gap&&*r.absolute_gap<0)||(r.relative_gap&&*r.relative_gap<0)||(r.native_backend_gap&&*r.native_backend_gap<0))fail(LpSensitivityReason::InvalidSource,"Negative original LP gap");
    for(std::size_t i=0;i<m.variables.size();++i){tick();if(r.active_variables[i]!=m.variables[i].active||(m.variables[i].active&&!std::isfinite(r.values[i])))
      fail(LpSensitivityReason::InvalidSource,"Original LP masks/values disagree with source");}
    const auto checked=validate(m,r.values,opts.checks.primal_feasibility,0);
    if(!checked.valid||checked.objective!=r.objective)fail(LpSensitivityReason::InvalidSource,"Original LP values/objective failed independent validation");
    sense=m.objective.sense==ObjectiveSense::Minimize?1:-1;
    if(r.best_bound) {
      const Wide gap=sense*(static_cast<Wide>(*r.objective)-*r.best_bound);
      const Wide scalar_rounding=4*std::numeric_limits<double>::epsilon()*
        std::max({Wide(1),std::abs(static_cast<Wide>(*r.objective)),std::abs(static_cast<Wide>(*r.best_bound))});
      if(gap< -std::max(static_cast<Wide>(opts.checks.kkt.objective_gap),scalar_rounding))
        fail(LpSensitivityReason::InvalidSource,"Original LP bound is on the wrong side");
      const Wide absolute=std::abs(static_cast<Wide>(*r.objective)-*r.best_bound);
      if(r.absolute_gap&&std::abs(absolute-*r.absolute_gap)>opts.checks.kkt.objective_gap)
        fail(LpSensitivityReason::InvalidSource,"Original LP absolute gap is inconsistent");
      if(r.relative_gap&&std::abs(absolute/std::max(Wide(1),std::abs(static_cast<Wide>(*r.objective)))-*r.relative_gap)>opts.checks.kkt.objective_gap)
        fail(LpSensitivityReason::InvalidSource,"Original LP relative gap is inconsistent");
    }
    std::set<std::pair<int,std::uint64_t>> requested;
    for(const auto& p:opts.parameters){tick();
      if(const auto* v=std::get_if<LpObjectiveParameter>(&p)){
        if(v->variable.model_id!=m.model_id||v->variable.id>=m.variables.size()||!m.variables[v->variable.id].active)
          fail(LpSensitivityReason::InvalidSource,"Sensitivity variable is foreign/deleted");
        if(!requested.emplace(0,v->variable.id).second)fail(LpSensitivityReason::InvalidSource,"Duplicate sensitivity request");
      }else if(const auto* q=std::get_if<LpEqualityRhsParameter>(&p)){
        if(q->row.model_id!=m.model_id||q->row.id>=m.rows.size()||!m.rows[q->row.id].active)
          fail(LpSensitivityReason::InvalidSource,"Sensitivity row is foreign/deleted");
        const auto& row=m.rows[q->row.id];if(!std::isfinite(row.lower)||row.lower!=row.upper)
          fail(LpSensitivityReason::Unsupported,"Only equality common RHS sensitivity is supported");
        if(!requested.emplace(1,q->row.id).second)fail(LpSensitivityReason::InvalidSource,"Duplicate sensitivity request");
      }else fail(LpSensitivityReason::InvalidSource,"Sensitivity request has no payload");}
    out=std::shared_ptr<LpSensitivity>(new LpSensitivity);out->original_=original;out->tolerances_=opts.checks;
    for(const auto& v:m.variables)out->columns_.push_back(v.active);
    for(const auto& row:m.rows)out->rows_.push_back(row.active);
    for(const auto& p:opts.parameters)out->entries_.push_back({p,{LpSensitivityState::Unavailable,LpSensitivityReason::NotRequested,"Not yet analyzed"},{}});
    out->basis_=make_lp_basis(o);event("after_copy");
  }
  void setup() {
    event("before_factor");work.factor_setup_attempted=true;
#ifdef GECODE_OPTIMIZE_TEST_LP_SENSITIVITY
    factor=lp_sensitivity_test_factor(source(),*out->basis_,opts.checks.primal_feasibility,budget);
#else
    factor=make_lp_sensitivity_factor(source(),*out->basis_,opts.checks.primal_feasibility,budget);
#endif
    event("after_factor");if(!factor)fail(LpSensitivityReason::BackendFailure,"Missing private factorization");
    const auto& m=source();col_map.assign(m.variables.size(),missing);row_map.assign(m.rows.size(),missing);
    const std::size_t n=factor->columns.size(),r=factor->rows.size(),total=plus(n,r);
    std::size_t count=0;for(const auto& v:m.variables)count+=v.active;if(count!=n)fail(LpSensitivityReason::ChangedBasis,"Factor column count mismatch");
    count=0;for(const auto& row:m.rows)count+=row.active;if(count!=r)fail(LpSensitivityReason::ChangedBasis,"Factor row count mismatch");
    for(auto slot:factor->columns){tick();if(slot>=col_map.size()||!m.variables[slot].active||col_map[slot]!=missing)
      fail(LpSensitivityReason::ChangedBasis,"Invalid factor column mapping");col_map[slot]=entities.size();entities.push_back(m.variables[slot].variable);
      lower.push_back(m.variables[slot].lower);upper.push_back(m.variables[slot].upper);status.push_back(*out->basis_->columns()[slot]);}
    for(auto slot:factor->rows){tick();if(slot>=row_map.size()||!m.rows[slot].active||row_map[slot]!=missing)
      fail(LpSensitivityReason::ChangedBasis,"Invalid factor row mapping");row_map[slot]=entities.size();entities.push_back(m.rows[slot].constraint);
      lower.push_back(-m.rows[slot].upper);upper.push_back(-m.rows[slot].lower);auto s=*out->basis_->rows()[slot];
      status.push_back(s==LpBasisStatus::Lower?LpBasisStatus::Upper:s==LpBasisStatus::Upper?LpBasisStatus::Lower:s);}
    matrix.resize(total);for(std::size_t i=0;i<r;++i){for(const auto& t:m.rows[factor->rows[i]].terms){tick();matrix[col_map[t.variable.id]].push_back({i,t.coefficient});}matrix[n+i].push_back({i,1});}
    position.assign(total,missing);
    if(factor->order.size()!=r)fail(LpSensitivityReason::ChangedBasis,"Wrong factor basis size");
    for(const auto& entity:factor->order){tick();const auto k=entity_slot(entity);
      if(position[k]!=missing||status[k]!=LpBasisStatus::Basic)fail(LpSensitivityReason::ChangedBasis,"Duplicate/nonbasic factor entity");
      position[k]=basic.size();basic.push_back(k);}
    for(std::size_t k=0;k<total;++k)if((status[k]==LpBasisStatus::Basic)!=(position[k]!=missing))fail(LpSensitivityReason::ChangedBasis,"Factor omitted a basic entity");
    out->order_=factor->order;out->backend_version_=factor->version;
    cost.assign(total,0);for(const auto& t:m.objective.terms)cost[col_map[t.variable.id]]=sense*t.coefficient;
    point.assign(total,0);for(std::size_t k=0;k<total;++k)if(position[k]==missing){tick();switch(status[k]){
      case LpBasisStatus::Lower:point[k]=lower[k];break;case LpBasisStatus::Upper:point[k]=upper[k];break;
      case LpBasisStatus::Zero:point[k]=0;break;default:fail(LpSensitivityReason::InvalidBasis,"Unspecified nonbasic status");}}
  }
  Wide subtract_dot(Wide anchor,std::size_t column,const std::vector<Wide>& v) {
    Sum s;s.add(anchor);for(auto [row,a]:matrix[column]){tick();s.add(-a*v[row]);}return s.value();
  }
  std::vector<Wide> solve_system(const std::vector<Wide>& rhs,bool transpose) {
    event("before_system",work.basis_solves);
    if(work.basis_solves>=opts.limits.max_basis_solves)throw Failure(LpSensitivityReason::ResourceLimit,"Basis linear-system call limit reached",Termination::IterationLimit);
    std::vector<double> input;for(Wide v:rhs){tick();input.push_back(narrow(v));}
    ++work.basis_solves;auto raw=factor->solve(input,transpose);event("after_system",work.basis_solves);
    if(raw.size()!=basic.size())fail(LpSensitivityReason::FailedLinearSolveChecks,"Wrong basis solution count");
    std::vector<Wide> result;for(double v:raw){tick();if(!std::isfinite(v))fail(LpSensitivityReason::FailedLinearSolveChecks,"Nonfinite basis solution");result.push_back(v);}
    std::vector<Sum> residual(basic.size()),magnitudes(basic.size());
    if(transpose){for(std::size_t p=0;p<basic.size();++p)for(auto [i,a]:matrix[basic[p]]){tick();residual[p].add(a*result[i]);magnitudes[p].add(std::abs(a*result[i]));}}
    else {for(std::size_t p=0;p<basic.size();++p)for(auto [i,a]:matrix[basic[p]]){tick();residual[i].add(a*result[p]);magnitudes[i].add(std::abs(a*result[p]));}}
    Wide max_absolute=0,max_scaled=0;
    for(std::size_t i=0;i<rhs.size();++i){tick();residual[i].add(-rhs[i]);const Wide error=std::abs(residual[i].value());
      const Wide scale=magnitudes[i].value()+std::abs(rhs[i]);max_absolute=std::max(max_absolute,error);max_scaled=std::max(max_scaled,error/std::max(Wide(1),scale));
      if(error>opts.checks.system_absolute+opts.checks.system_relative*scale)
        fail(LpSensitivityReason::FailedLinearSolveChecks,"Original basis-system residual failed");}
    auto& checks=out->checks_;checks.max_system_residual=std::max(checks.max_system_residual.value_or(0),narrow(max_absolute));
    checks.max_scaled_system_residual=std::max(checks.max_scaled_system_residual.value_or(0),narrow(max_scaled));return result;
  }
  void reference() {
    std::vector<Sum> sums(basic.size());
    for(std::size_t k=0;k<point.size();++k)if(position[k]==missing)for(auto [i,a]:matrix[k]){tick();sums[i].add(-a*point[k]);}
    std::vector<Wide> rhs;for(const auto& s:sums)rhs.push_back(s.value());
    const auto primal=solve_system(rhs,false);for(std::size_t p=0;p<basic.size();++p)point[basic[p]]=primal[p];
    rhs.clear();for(auto k:basic)rhs.push_back(cost[k]);dual=solve_system(rhs,true);
    std::vector<double> values(source().variables.size(),std::numeric_limits<double>::quiet_NaN());
    Wide difference=0;for(std::size_t j=0;j<factor->columns.size();++j){tick();values[factor->columns[j]]=narrow(point[j]);difference=std::max(difference,std::abs(point[j]-out->original_.result.values[factor->columns[j]]));}
    auto& checks=out->checks_;checks.max_point_difference=narrow(difference);checks.basis_point_matches=difference<=opts.checks.primal_feasibility;
    checks.primal=validate(source(),values,opts.checks.primal_feasibility,0);
    auto& kkt=checks.kkt;kkt.primal_valid=checks.primal.valid;
    if(!checks.primal.valid||!checks.basis_point_matches)fail(LpSensitivityReason::FailedReferenceChecks,"Selected basis point disagrees with original feasible LP point");
    reduced.resize(point.size());Wide max_stationarity=0,max_sign=0,max_comp=0;Sum normalized_gap,dual_terms;
    for(std::size_t k=0;k<point.size();++k){tick();const Wide raw=subtract_dot(cost[k],k,dual);
      // Basic reduced cost is mathematically zero; retain the computed residual.
      reduced[k]=position[k]!=missing?0:raw;if(position[k]!=missing)max_stationarity=std::max(max_stationarity,std::abs(raw));
      const Wide value=reduced[k];
      if(lower[k]!=upper[k]) {
        if(status[k]==LpBasisStatus::Lower)max_sign=std::max(max_sign,-value);
        if(status[k]==LpBasisStatus::Upper)max_sign=std::max(max_sign,value);
        if(status[k]==LpBasisStatus::Zero)max_sign=std::max(max_sign,std::abs(value));
      }
      normalized_gap.add(cost[k]*point[k]);
      if(value!=0){const double side=value>0?lower[k]:upper[k];if(!std::isfinite(side))fail(LpSensitivityReason::FailedReferenceChecks,"Basis dual requires an infinite bound");
        dual_terms.add(value*side);normalized_gap.add(-value*side);max_comp=std::max(max_comp,std::abs(value*(point[k]-side)));}
    }
    // Public primal activities are independently recomputed by validate; also
    // check all retained logical coordinates against A*x, not only B residuals.
    std::vector<Sum> activity(basic.size());
    for(std::size_t k=0;k<point.size();++k)for(auto [row,a]:matrix[k]){tick();activity[row].add(a*point[k]);}
    for(const auto& value:activity)if(std::abs(value.value())>opts.checks.primal_feasibility)
      fail(LpSensitivityReason::FailedReferenceChecks,"Logical basis coordinate disagrees with original row activity");
    kkt.max_stationarity=narrow(max_stationarity);kkt.max_dual_sign_violation=narrow(std::max(Wide(0),max_sign));kkt.max_complementarity=narrow(max_comp);
    kkt.normalized_gap=narrow(normalized_gap.value());Sum objective;objective.add(source().objective.offset);objective.add(sense*dual_terms.sum);objective.add(sense*dual_terms.correction);kkt.dual_objective_estimate=narrow(objective.value());
    kkt.stationarity_valid=max_stationarity<=opts.checks.kkt.stationarity;kkt.dual_signs_valid=max_sign<=opts.checks.kkt.dual_feasibility;
    kkt.complementarity_valid=max_comp<=opts.checks.kkt.complementarity;kkt.gap_valid=std::abs(normalized_gap.value())<=opts.checks.kkt.objective_gap;
    kkt.accepted=kkt.primal_valid&&kkt.stationarity_valid&&kkt.dual_signs_valid&&kkt.complementarity_valid&&kkt.gap_valid;
    if(!kkt.accepted)fail(LpSensitivityReason::FailedReferenceChecks,"Selected basis failed original numerical KKT checks");event("after_reference");
  }
  LpSensitivityLimiter limiter(std::size_t k,LpSensitivitySide side,bool dual_condition) {
    if(k>=factor->columns.size()&&(side==LpSensitivitySide::Lower||side==LpSensitivitySide::Upper))
      side=side==LpSensitivitySide::Lower?LpSensitivitySide::Upper:LpSensitivitySide::Lower;
    return {entities[k],side,dual_condition};
  }
  LpParameterInterval interval(const LpSensitivityParameter& parameter) {
    const std::size_t total=point.size();std::vector<Inequality> inequalities;
    inequalities.reserve(2*total);LpParameterInterval output;
    if(const auto* p=std::get_if<LpObjectiveParameter>(&parameter)) {
      const auto j=col_map[p->variable.id];output.anchor=narrow(sense*cost[j]);output.objective_slope=narrow(point[j]);
      std::vector<Wide> v(basic.size(),0);if(position[j]!=missing){std::vector<Wide> rhs(basic.size(),0);rhs[position[j]]=sense;v=solve_system(rhs,true);}
      for(std::size_t k=0;k<total;++k){tick();if(position[k]!=missing||lower[k]==upper[k])continue;
        const Wide b=subtract_dot(k==j?sense:0,k,v),a=reduced[k];
        if(status[k]==LpBasisStatus::Lower||status[k]==LpBasisStatus::Zero)inequalities.push_back({a,b,limiter(k,status[k]==LpBasisStatus::Zero?LpSensitivitySide::Free:LpSensitivitySide::Lower,true),opts.checks.kkt.dual_feasibility});
        if(status[k]==LpBasisStatus::Upper||status[k]==LpBasisStatus::Zero)inequalities.push_back({-a,-b,limiter(k,status[k]==LpBasisStatus::Zero?LpSensitivitySide::Free:LpSensitivitySide::Upper,true),opts.checks.kkt.dual_feasibility});
      }
    } else {
      const auto& q=std::get<LpEqualityRhsParameter>(parameter);const auto selected=row_map[q.row.id];
      output.anchor=source().rows[q.row.id].lower;std::vector<Wide> direction(total,0);
      if(position[selected]==missing){std::vector<Wide> rhs(basic.size(),0);rhs[selected-factor->columns.size()]=1;
        const auto v=solve_system(rhs,false);for(std::size_t k=0;k<basic.size();++k)direction[basic[k]]=v[k];direction[selected]=-1;}
      Sum slope;for(std::size_t j=0;j<factor->columns.size();++j){tick();slope.add(sense*cost[j]*direction[j]);}output.objective_slope=narrow(slope.value());
      if(position[selected]==missing&&std::abs(slope.value()-sense*dual[selected-factor->columns.size()])>opts.checks.kkt.stationarity)
        fail(LpSensitivityReason::FailedIntervalChecks,"Equality objective slope disagrees with original row dual");
      for(std::size_t k=0;k<total;++k){tick();const Wide bound_derivative=k==selected?-1:0;
        if(std::isfinite(lower[k]))inequalities.push_back({point[k]-lower[k],direction[k]-bound_derivative,limiter(k,LpSensitivitySide::Lower,false),opts.checks.primal_feasibility});
        if(std::isfinite(upper[k]))inequalities.push_back({upper[k]-point[k],bound_derivative-direction[k],limiter(k,LpSensitivitySide::Upper,false),opts.checks.primal_feasibility});}
    }
    Wide lo=-std::numeric_limits<Wide>::infinity(),hi=std::numeric_limits<Wide>::infinity();
    for(const auto& q:inequalities){tick();if(!std::isfinite(q.a)||!std::isfinite(q.b))fail(LpSensitivityReason::FailedIntervalChecks,"Nonfinite interval inequality");
      if(q.b==0){if(q.a<0)fail(LpSensitivityReason::FailedIntervalChecks,"Numerical basis has no fixed-basis interval");continue;}
      const Wide endpoint=-q.a/q.b;if(!std::isfinite(endpoint))fail(LpSensitivityReason::FailedIntervalChecks,"Finite interval ratio overflowed");
      if(q.b>0&&endpoint>lo){lo=endpoint;output.lower_limiter=q.limiter;}
      if(q.b<0&&endpoint<hi){hi=endpoint;output.upper_limiter=q.limiter;}}
    if(lo>0||hi<0||lo>hi)fail(LpSensitivityReason::FailedIntervalChecks,"Numerical interval excludes its source anchor");
    auto end=[&](Wide delta,bool lower_end) {
      if(!std::isfinite(delta))return LpRangeEnd{lower_end?LpRangeEndKind::NegativeInfinity:LpRangeEndKind::PositiveInfinity,{}};
      Sum value;value.add(output.anchor);value.add(delta);return LpRangeEnd{LpRangeEndKind::Finite,narrow(value.value(),LpSensitivityReason::FailedIntervalChecks)};
    };
    output.lower=end(lo,true);output.upper=end(hi,false);
    // Recheck the published absolute values, including rounding during narrowing.
    const Wide published_lo=output.lower.value?static_cast<Wide>(*output.lower.value)-output.anchor:lo;
    const Wide published_hi=output.upper.value?static_cast<Wide>(*output.upper.value)-output.anchor:hi;
    Wide max_violation=0;bool lower_limited=!std::isfinite(lo),upper_limited=!std::isfinite(hi);
    for(const auto& q:inequalities){tick();
      if(std::isfinite(published_lo)){Sum v;v.add(q.a);v.add(q.b*published_lo);const Wide x=v.value();max_violation=std::max(max_violation,-x);
        if(x< -q.tolerance)fail(LpSensitivityReason::FailedIntervalChecks,"Published lower endpoint fails original affine checks");
        if(q.b>0&&std::abs(x)<=q.tolerance)lower_limited=true;
      }else if(q.b>0)fail(LpSensitivityReason::FailedIntervalChecks,"Invalid negative-infinity interval direction");
      if(std::isfinite(published_hi)){Sum v;v.add(q.a);v.add(q.b*published_hi);const Wide x=v.value();max_violation=std::max(max_violation,-x);
        if(x< -q.tolerance)fail(LpSensitivityReason::FailedIntervalChecks,"Published upper endpoint fails original affine checks");
        if(q.b<0&&std::abs(x)<=q.tolerance)upper_limited=true;
      }else if(q.b<0)fail(LpSensitivityReason::FailedIntervalChecks,"Invalid positive-infinity interval direction");}
    if(!lower_limited||!upper_limited)fail(LpSensitivityReason::FailedIntervalChecks,"Finite endpoint has no checked limiting condition");
    output.checks={true,inequalities.size(),narrow(std::max(Wide(0),max_violation)),!std::isfinite(lo),!std::isfinite(hi),"Original affine interval checks accepted numerically"};return output;
  }
  static void clear_owned(const std::shared_ptr<LpSensitivity>& data,LpSensitivityReason reason,const char* text) noexcept {
    if(!data)return;
    for(auto& entry:data->entries_){entry.group.state=LpSensitivityState::Unavailable;entry.group.reason=reason;entry.interval.reset();}
    try {for(auto& entry:data->entries_)entry.group.message=text;}catch(...){for(auto& entry:data->entries_)entry.group.message.clear();}
  }
  void clear(LpSensitivityReason reason,const char* text) noexcept {clear_owned(out,reason,text);}
  void release() {factor.reset();matrix.clear();entities.clear();col_map.clear();row_map.clear();basic.clear();position.clear();status.clear();lower.clear();upper.clear();point.clear();cost.clear();dual.clear();reduced.clear();event("after_cleanup");}
  void run(const LpObservedResult& original,LpSensitivityResult& result) {
    try {
      prepare(original);result.sensitivity=out;setup();reference();std::size_t accepted=0;
      for(std::size_t i=0;i<out->entries_.size();++i){event("before_interval",i);auto& entry=out->entries_[i];
        try {entry.interval=interval(entry.parameter);entry.group={LpSensitivityState::Available,LpSensitivityReason::None,"Fixed-basis numerical interval available"};++accepted;}
        catch(const Failure& e){if(e.stop||e.reason==LpSensitivityReason::ResourceLimit||e.reason==LpSensitivityReason::Stopped)throw;
          entry.interval.reset();entry.group={LpSensitivityState::Rejected,e.reason,e.what()};}
        event("after_interval",i);}
      result.completion=accepted==out->entries_.size()?LpSensitivityCompletion::Complete:accepted?LpSensitivityCompletion::Partial:LpSensitivityCompletion::Rejected;
      result.reason=result.completion==LpSensitivityCompletion::Complete?LpSensitivityReason::None:LpSensitivityReason::FailedIntervalChecks;
      result.message=accepted==out->entries_.size()?"Requested fixed-basis numerical intervals accepted":"Some requested intervals failed numerical checks";
      check();
    } catch(const Failure& e) {
      factor.reset();clear(e.reason,e.what());result.completion=e.stop?LpSensitivityCompletion::Interrupted:LpSensitivityCompletion::Rejected;
      result.reason=e.reason;result.stop_reason=e.stop;result.message=e.what();
    } catch(const LpSensitivityBackendError& e) {
      factor.reset();clear(e.reason,e.what());result.reason=e.reason;result.message=e.what();result.completion=LpSensitivityCompletion::Rejected;
    } catch(const ModelError& e) {
      factor.reset();clear(LpSensitivityReason::InvalidSource,e.what());result.reason=LpSensitivityReason::InvalidSource;result.message=e.what();result.completion=LpSensitivityCompletion::Rejected;
    }
    result.sensitivity=out;result.work=work;
  }
};
}
void LpSensitivityTolerances::validate() const {
  kkt.validate();for(double v:{primal_feasibility,system_absolute,system_relative})
    if(!std::isfinite(v)||v<0)throw ModelError("LP sensitivity tolerances must be finite and nonnegative");
}
void LpSensitivityOptions::validate() const {
  checks.validate();if(std::isnan(time_limit_seconds)||time_limit_seconds<0)throw ModelError("Invalid sensitivity time limit");
  if(parameters.empty())throw ModelError("Sensitivity requires a nonempty explicit parameter list");
  if(backend!=Backend::Auto&&backend!=Backend::Highs&&backend!=Backend::Native)throw ModelError("Unknown sensitivity backend");
}
const LpSensitivityEntry* LpSensitivity::objective(Variable v) const {
  if(v.model_id!=id()||v.id>=columns_.size()||!columns_[v.id])throw ModelError("Sensitivity variable is foreign, absent or deleted");
  for(const auto& e:entries_)if(const auto* p=std::get_if<LpObjectiveParameter>(&e.parameter))if(p->variable==v)return &e;return nullptr;
}
const LpSensitivityEntry* LpSensitivity::equality_rhs(Constraint r) const {
  if(r.model_id!=id()||r.id>=rows_.size()||!rows_[r.id])throw ModelError("Sensitivity row is foreign, absent or deleted");
  for(const auto& e:entries_)if(const auto* p=std::get_if<LpEqualityRhsParameter>(&e.parameter))if(p->row.model_id==r.model_id&&p->row.id==r.id)return &e;return nullptr;
}
LpSensitivityResult analyze_lp_sensitivity(const LpObservedResult& original,const LpSensitivityOptions& options) {
  const auto start=Clock::now();LpSensitivityResult result;result.model_id=original.result.model_id;result.revision=original.result.revision;
  std::unique_ptr<Detail::LpSensitivityAccess> access;bool options_valid=false;
  try {options.validate();options_valid=true;access=std::make_unique<Detail::LpSensitivityAccess>(options,start);access->run(original,result);}
  catch(const std::bad_alloc&){if(access)access->clear(LpSensitivityReason::AllocationFailure,"LP sensitivity allocation failed");
    result.completion=LpSensitivityCompletion::Interrupted;result.reason=LpSensitivityReason::AllocationFailure;result.stop_reason=Termination::MemoryLimit;result.message.clear();}
  catch(const ModelError& e){if(access)access->clear(LpSensitivityReason::InvalidSource,e.what());
    result.completion=LpSensitivityCompletion::Rejected;result.reason=LpSensitivityReason::InvalidSource;try{result.message=e.what();}catch(...){result.message.clear();}}
  catch(const std::exception& e){if(access)access->clear(LpSensitivityReason::BackendFailure,e.what());
    result.completion=LpSensitivityCompletion::Rejected;result.reason=LpSensitivityReason::BackendFailure;try{result.message=e.what();}catch(...){result.message.clear();}}
  std::shared_ptr<LpSensitivity> owned;
  if(access){owned=access->out;result.sensitivity=owned;result.work=access->work;
    try{access->release();access->check();}catch(const Failure& e){access->clear(e.reason,e.what());result.completion=LpSensitivityCompletion::Interrupted;result.reason=e.reason;result.stop_reason=e.stop;try{result.message=e.what();}catch(...){result.message.clear();}}
    catch(...){access->clear(LpSensitivityReason::BackendFailure,"LP sensitivity cleanup failed");result.completion=LpSensitivityCompletion::Rejected;result.reason=LpSensitivityReason::BackendFailure;result.message.clear();}
    access.reset();
  }
  // Includes destruction of retained vector capacity, backend state and budget.
  std::optional<Termination> final_stop;
  if(options_valid&&options.cancellation&&options.cancellation->cancelled())final_stop=Termination::Cancelled;
  else if(options_valid&&std::chrono::duration<double>(Clock::now()-start).count()>=options.time_limit_seconds)final_stop=Termination::TimeLimit;
  if(final_stop){Detail::LpSensitivityAccess::clear_owned(owned,LpSensitivityReason::Stopped,"Whole LP sensitivity allowance stopped during cleanup");
    result.completion=LpSensitivityCompletion::Interrupted;result.reason=LpSensitivityReason::Stopped;result.stop_reason=final_stop;
    try{result.message="Whole LP sensitivity allowance stopped during cleanup";}catch(...){result.message.clear();}
  }
  result.elapsed_seconds=std::chrono::duration<double>(Clock::now()-start).count();return result;
}
}}
