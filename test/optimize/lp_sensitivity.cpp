#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/lp_sensitivity.hpp>
#include <gecode/optimize/session.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include "lp_sensitivity_oracle.hpp"
#include "lp_sensitivity_fixture.hpp"

namespace O=Gecode::Optimize;
constexpr double inf=std::numeric_limits<double>::infinity();
static void near(double a,double b){assert(std::isfinite(a)&&std::abs(a-b)<1e-7*std::max(1.0,std::abs(b)));}
static void endpoint(const O::LpRangeEnd& a,double b){
  if(std::isinf(b)){assert(!a.value);assert(a.kind==(b>0?O::LpRangeEndKind::PositiveInfinity:O::LpRangeEndKind::NegativeInfinity));}
  else {assert(a.kind==O::LpRangeEndKind::Finite&&a.value);near(*a.value,b);}
}
static void bounds(const O::LpSensitivityEntry* e,double lo,double hi,double slope){
  assert(e);if(e->group.state!=O::LpSensitivityState::Available)std::cerr<<"range unavailable: "<<e->group.message<<'\n';
  assert(e->group.state==O::LpSensitivityState::Available&&e->interval&&e->interval->checks.accepted);
  endpoint(e->interval->lower,lo);endpoint(e->interval->upper,hi);assert(e->interval->objective_slope);near(*e->interval->objective_slope,slope);
}
static void complete(const O::LpSensitivityResult& r){
  if(r.completion!=O::LpSensitivityCompletion::Complete)std::cerr<<"sensitivity failure: "<<r.message<<'\n';
  assert(r.completion==O::LpSensitivityCompletion::Complete&&r.sensitivity);assert(r.sensitivity->checks().kkt.accepted);
}
static O::LpObservedResult selected(O::Model& m,std::vector<O::LpBasisStatus> columns,
                                    std::vector<O::LpBasisStatus> rows){
  O::LpBasisData data;data.source=m.snapshot();for(auto s:columns)data.columns.push_back(s);for(auto s:rows)data.rows.push_back(s);
  O::LpBasisSolveOptions options;options.basis=O::make_lp_basis(data);
  auto out=O::solve_lp_with_basis(m,options);
  assert(out.submission.state==O::LpBasisSubmissionState::Accepted);
  assert(out.observed.result.termination==O::Termination::Optimal);return out.observed;
}
template<class F> static void invalid(F f){bool threw=false;try{f();}catch(const O::ModelError&){threw=true;}assert(threw);}
static void historical_panel(){
  O::LpSensitivityResult history;O::LpObservedResult original;O::Variable x,y,deleted;O::Constraint row,removed,unrequested;
  {
    O::Model model;deleted=model.add_continuous();model.remove(deleted);
    x=model.add_continuous();y=model.add_continuous();removed=model.add_row({{x,1}},0,9);model.remove(removed);
    row=model.add_row({{x,1},{y,1}},3,3);unrequested=model.add_row({{x,1}},-inf,4);
    model.minimize({{x,2},{y,1}},7);O::SolveSession session;original=session.solve_lp_observed(model);
    O::LpSensitivityOptions options;options.parameters={O::LpObjectiveParameter{x},O::LpEqualityRhsParameter{row}};
    history=O::analyze_lp_sensitivity(original,options);complete(history);
    assert(!history.sensitivity->objective(y)&&!history.sensitivity->equality_rhs(unrequested));
    invalid([&]{history.sensitivity->objective(deleted);});invalid([&]{history.sensitivity->equality_rhs(removed);});
    invalid([&]{history.sensitivity->objective(O::Variable{model.id()+1,x.id});});
    invalid([&]{history.sensitivity->equality_rhs(O::Constraint{model.id()+1,row.id});});
    for(auto request:{O::LpSensitivityParameter{O::LpObjectiveParameter{deleted}},O::LpSensitivityParameter{O::LpEqualityRhsParameter{removed}}}){
      options.parameters={request};auto rejected=O::analyze_lp_sensitivity(original,options);
      assert(rejected.reason==O::LpSensitivityReason::InvalidSource&&!rejected.work.factor_setup_attempted);
    }
    model.set_objective_coefficient(x,-3);auto changed=session.solve_lp_observed(model);assert(changed.result.termination==O::Termination::Optimal);
    session.reset();assert(history.sensitivity->revision()!=model.revision());
  }
  bounds(history.sensitivity->objective(x),1,inf,0);bounds(history.sensitivity->equality_rhs(row),0,inf,1);
  assert(history.sensitivity->original().observations==original.observations);
  assert(history.sensitivity->active_columns()==std::vector<bool>({false,true,true}));
  assert(history.sensitivity->active_rows()==std::vector<bool>({false,true,true}));
  for(const auto& entity:history.sensitivity->factor_order())std::visit([&](const auto& e){assert(e.model_id==history.model_id&&e.id!=0);},entity);
  original={};assert(history.sensitivity->original().result.objective==10);
}
static void scaled_panel(){
  using B=O::LpBasisStatus;
  for(double scale:{1e-6,1e6})for(bool maximize:{false,true}){
    O::Model model;auto x=model.add_continuous(-2,5),y=model.add_continuous(0,4);
    auto row=model.add_row({{x,2*scale},{y,-4*scale}},-12*scale,-12*scale);
    model.set_objective({{x,maximize?-2.0:2.0},{y,maximize?-3.0:3.0}},maximize?O::ObjectiveSense::Maximize:O::ObjectiveSense::Minimize,17);
    auto source=selected(model,{B::Lower,B::Basic},{B::Lower});O::LpSensitivityOptions o;
    o.parameters={O::LpObjectiveParameter{x},O::LpObjectiveParameter{y},O::LpEqualityRhsParameter{row}};
    auto result=O::analyze_lp_sensitivity(source,o);complete(result);
    bounds(result.sensitivity->objective(x),maximize?-inf:-1.5,maximize?1.5:inf,-2);
    bounds(result.sensitivity->objective(y),maximize?-inf:-4,maximize?4:inf,2);
    bounds(result.sensitivity->equality_rhs(row),-20*scale,-4*scale,(maximize?3.0:-3.0)/(4*scale));
  }
}
static void cancellation_panel(){
  // Exact original witness: w=1, x=0, y=1-10^14; dual=(1,100),
  // reduced cost of w = 10^16 - 1 - 10^16 = -1. Computing the
  // dot product first rounds away the 1 and changes x's lower endpoint.
  O::Model model;auto x=model.add_continuous(-inf,inf),y=model.add_continuous(-inf,inf),w=model.add_continuous(0,1);
  model.add_row({{x,1},{w,1}},1,1);model.add_row({{y,1},{w,1e14}},1,1);
  model.minimize({{x,1},{y,100},{w,1e16}});auto source=model.snapshot();
  O::LpObservedResult original;original.result.model_id=source.model_id;original.result.revision=source.revision;
  original.result.termination=O::Termination::Optimal;original.result.guarantee=O::Guarantee::Numerical;
  original.result.values={0,1-1e14,1};original.result.active_variables={true,true,true};original.result.solution_validated=true;
  original.result.objective=original.result.best_bound=100;original.result.absolute_gap=original.result.relative_gap=0;
  O::LpObservationOptions options;auto data=O::Detail::LpObservationAccess::create(source,options);original.observations=data;
  O::Detail::LpBackendObservations raw;raw.attempted=raw.timely=raw.complete=true;
  raw.info_valid=raw.value_valid=raw.primal_feasible=raw.dual_valid=raw.dual_feasible=raw.basis_valid=raw.info_basis_valid=true;
  raw.model_id=source.model_id;raw.revision=source.revision;raw.column_slots={0,1,2};raw.row_slots={0,1};
  raw.column_duals={0,0,-1};raw.row_duals={1,100};
  raw.column_basis={O::LpBasisStatus::Basic,O::LpBasisStatus::Basic,O::LpBasisStatus::Upper};
  raw.row_basis={O::LpBasisStatus::Lower,O::LpBasisStatus::Lower};O::SolveBudget budget(options.solve);
  O::Detail::LpObservationAccess::finish(*data,original.result,raw,budget);assert(data->checks().accepted);
  O::LpSensitivityOptions requested;requested.parameters={O::LpObjectiveParameter{x}};
  auto result=O::analyze_lp_sensitivity(original,requested);complete(result);bounds(result.sensitivity->objective(x),0,inf,0);
  assert(result.sensitivity->checks().kkt.normalized_gap==0);
}
static void partial_panel(){
  // The coefficient-y endpoint 100-2e-14 cannot be represented closely enough
  // to retain its original-unit limiting equality; the w endpoint is exact.
  O::Model model;auto y=model.add_continuous(-inf,inf),w=model.add_continuous(0,1);
  model.add_row({{y,1},{w,1e14}},1,1);model.minimize({{y,100},{w,1e16-2}});
  auto original=O::solve_lp_observed(model);assert(original.result.termination==O::Termination::Optimal&&original.observations->checks().accepted);
  assert(original.result.objective==98);O::LpSensitivityOptions options;
  options.parameters={O::LpObjectiveParameter{y},O::LpObjectiveParameter{w}};
  auto result=O::analyze_lp_sensitivity(original,options);assert(result.completion==O::LpSensitivityCompletion::Partial&&result.sensitivity);
  auto rejected=result.sensitivity->objective(y);assert(rejected->group.state==O::LpSensitivityState::Rejected&&rejected->group.reason==O::LpSensitivityReason::FailedIntervalChecks&&!rejected->interval);
  bounds(result.sensitivity->objective(w),-inf,1e16,1);
}
static void oracle_panel(){
  using namespace SensitivityOracle;using B=O::LpBasisStatus;
  for(bool maximize:{false,true})for(bool at_lower:{false,true})for(int a:{-3,2})for(int b:{-4,2}){
    BoxLine q;q.a=Q(a);q.b=Q(b);q.lx=Q(-2);q.ux=Q(5);q.ly=Q(0);q.uy=Q(4);q.cy=Q(3);
    q.reference={at_lower?q.lx:q.ux,Q(2)};q.rhs=q.a*q.reference[0]+q.b*q.reference[1];q.maximize=maximize;
    q.cx=q.a*q.cy/q.b+Q((maximize?-1:1)*(at_lower?1:-1));
    O::Model m;auto x=m.add_continuous(q.lx.value(),q.ux.value()),y=m.add_continuous(q.ly.value(),q.uy.value());
    auto row=m.add_row({{x,q.a.value()},{y,q.b.value()}},q.rhs.value(),q.rhs.value());
    m.set_objective({{x,q.cx.value()},{y,q.cy.value()}},maximize?O::ObjectiveSense::Maximize:O::ObjectiveSense::Minimize,1e16);
    auto original=selected(m,{at_lower?B::Lower:B::Upper,B::Basic},{B::Lower});
    O::LpSensitivityOptions options;options.parameters={O::LpObjectiveParameter{x},O::LpObjectiveParameter{y},O::LpEqualityRhsParameter{row}};
    auto result=O::analyze_lp_sensitivity(original,options);complete(result);
    auto expected=[&](Range r,const O::LpSensitivityEntry* e,double slope){bounds(e,r.lower?r.lower->value():-inf,r.upper?r.upper->value():inf,slope);};
    expected(q.objective(0),result.sensitivity->objective(x),q.reference[0].value());
    expected(q.objective(1),result.sensitivity->objective(y),q.reference[1].value());
    expected(q.equality_with_x_fixed(),result.sensitivity->equality_rhs(row),(q.cy/q.b).value());
    for(std::size_t j=0;j<2;++j){const auto range=q.objective(j);const auto anchor=j?q.cy:q.cx;
      assert(q.optimal_at(anchor,j));
      if(range.lower){assert(q.optimal_at(*range.lower,j));assert(q.optimal_at((*range.lower+anchor)/Q(2),j));assert(!q.optimal_at(*range.lower-Q(1,4),j));}
      if(range.upper){assert(q.optimal_at(*range.upper,j));assert(q.optimal_at((*range.upper+anchor)/Q(2),j));assert(!q.optimal_at(*range.upper+Q(1,4),j));}
      if(!range.lower)assert(q.optimal_at(anchor-Q(100),j));
      if(!range.upper)assert(q.optimal_at(anchor+Q(100),j));
    }
  }
  // Free basic column, fixed nonbasic column, unreferenced zero-cost variable.
  O::Model free;auto x=free.add_continuous(-inf,inf),fixed=free.add_continuous(2,2),zero=free.add_continuous(-5,5);
  auto row=free.add_row({{x,1},{fixed,1}},3,3);free.minimize({{x,1}},9);
  auto original=selected(free,{B::Basic,B::Lower,B::Lower},{B::Lower});
  O::LpSensitivityOptions options;options.parameters={O::LpObjectiveParameter{x},O::LpObjectiveParameter{fixed},O::LpObjectiveParameter{zero},O::LpEqualityRhsParameter{row}};
  auto r=O::analyze_lp_sensitivity(original,options);complete(r);
  bounds(r.sensitivity->objective(x),-inf,inf,1);bounds(r.sensitivity->objective(fixed),-inf,inf,2);
  bounds(r.sensitivity->objective(zero),0,inf,-5);bounds(r.sensitivity->equality_rhs(row),-inf,inf,1);
  // A redundant equality has a basic fixed logical; its RHS range is singleton.
  O::Model redundant;auto v=redundant.add_continuous(0,10);auto a=redundant.add_row({{v,1}},2,2);auto b=redundant.add_row({{v,2}},4,4);redundant.minimize({{v,1}});
  original=selected(redundant,{B::Basic},{B::Lower,B::Basic});options.parameters={O::LpEqualityRhsParameter{a},O::LpEqualityRhsParameter{b},O::LpObjectiveParameter{v}};
  r=O::analyze_lp_sensitivity(original,options);complete(r);bounds(r.sensitivity->equality_rhs(a),2,2,1);bounds(r.sensitivity->equality_rhs(b),4,4,0);bounds(r.sensitivity->objective(v),-inf,inf,2);
  // A ranged source row limits equality changes; varying its side is unsupported.
  O::Model ranged;v=ranged.add_continuous(-inf,inf);a=ranged.add_row({{v,1}},2,2);b=ranged.add_row({{v,1}},1,3);ranged.minimize({{v,1}});
  original=selected(ranged,{B::Basic},{B::Lower,B::Basic});options.parameters={O::LpEqualityRhsParameter{a}};
  r=O::analyze_lp_sensitivity(original,options);complete(r);bounds(r.sensitivity->equality_rhs(a),1,3,1);
  options.parameters={O::LpEqualityRhsParameter{b}};r=O::analyze_lp_sensitivity(original,options);assert(r.reason==O::LpSensitivityReason::Unsupported&&!r.work.factor_setup_attempted);
  // A free nonbasic column requires both reduced-cost signs: coefficient zero.
  O::Model free_zero;v=free_zero.add_continuous();auto z=free_zero.add_continuous(-inf,inf);
  a=free_zero.add_row({{v,1}},2,2);free_zero.minimize({{v,1}});
  original=selected(free_zero,{B::Basic,B::Zero},{B::Lower});options.parameters={O::LpObjectiveParameter{z},O::LpEqualityRhsParameter{a}};
  r=O::analyze_lp_sensitivity(original,options);complete(r);bounds(r.sensitivity->objective(z),0,0,0);bounds(r.sensitivity->equality_rhs(a),0,inf,1);
  // The same degenerate optimum has different ranges for different bases.
  O::Model degenerate;auto dx=degenerate.add_continuous(),dy=degenerate.add_continuous();
  degenerate.add_row({{dx,1},{dy,1}},0,0);degenerate.minimize({});
  options.parameters={O::LpObjectiveParameter{dx}};
  auto x_basic=O::analyze_lp_sensitivity(selected(degenerate,{B::Basic,B::Lower},{B::Lower}),options);
  auto y_basic=O::analyze_lp_sensitivity(selected(degenerate,{B::Lower,B::Basic},{B::Lower}),options);
  complete(x_basic);complete(y_basic);bounds(x_basic.sensitivity->objective(dx),-inf,0,0);bounds(y_basic.sensitivity->objective(dx),0,inf,0);
}
int main(){
  O::Model m;auto x=m.add_continuous(),y=m.add_continuous();auto row=m.add_row({{x,1},{y,1}},3,3);m.minimize({{x,2},{y,1}},7);
  O::LpSensitivityOptions options;options.parameters={O::LpObjectiveParameter{x},O::LpObjectiveParameter{y},O::LpEqualityRhsParameter{row}};
  auto original=O::solve_lp_observed(m);
  if(!O::capabilities().available){
    auto missing=O::analyze_lp_sensitivity(original,options);assert(missing.completion==O::LpSensitivityCompletion::Rejected);
    auto explicit_source=SensitivityFixture::two_column_observed(m);
    auto unavailable=O::analyze_lp_sensitivity(explicit_source,options);
    assert(unavailable.completion==O::LpSensitivityCompletion::Rejected&&unavailable.reason==O::LpSensitivityReason::Unsupported);
    assert(unavailable.sensitivity&&unavailable.work.factor_setup_attempted&&unavailable.work.basis_solves==0);
    for(const auto& e:unavailable.sensitivity->entries())assert(e.group.state!=O::LpSensitivityState::Available&&!e.interval);
    std::cout<<"sensitivity missing-observation and missing-backend admission passed\n";return 0;
  }
  auto r=O::analyze_lp_sensitivity(original,options);complete(r);
  bounds(r.sensitivity->objective(x),1,inf,0);bounds(r.sensitivity->objective(y),-inf,2,3);bounds(r.sensitivity->equality_rhs(row),0,inf,1);
  assert(r.work.factor_setup_attempted&&r.work.basis_solves==4);
  m.maximize({{x,-2},{y,-1}},11);auto maximal=O::solve_lp_observed(m);auto max=O::analyze_lp_sensitivity(maximal,options);complete(max);
  bounds(max.sensitivity->objective(x),-inf,-1,0);bounds(max.sensitivity->objective(y),-2,inf,3);bounds(max.sensitivity->equality_rhs(row),0,inf,-1);
  // Historical source/result is copied unchanged even after edits.
  assert(r.sensitivity->original().result.objective==original.result.objective);
  assert(r.sensitivity->original().observations==original.observations);
  assert(r.sensitivity->revision()!=m.revision());
  auto again=O::analyze_lp_sensitivity(original,options);complete(again);
  oracle_panel();
  historical_panel();scaled_panel();cancellation_panel();partial_panel();
  for(int fault=0;fault<8;++fault){auto bad=original;
    if(fault==0)bad.result.termination=static_cast<O::Termination>(99);
    if(fault==1)bad.result.revision++;
    if(fault==2)bad.result.active_variables[0]=false;
    if(fault==3)bad.result.values[1]=4;
    if(fault==4)bad.result.objective=inf;
    if(fault==5)bad.result.best_bound=-10;
    if(fault==6)bad.result.solution_validated=false;
    if(fault==7)bad.result.absolute_gap=-1;
    auto rejected=O::analyze_lp_sensitivity(bad,options);assert(rejected.completion==O::LpSensitivityCompletion::Rejected&&!rejected.work.factor_setup_attempted);}
  auto limited=options;limited.time_limit_seconds=0;
  assert(O::analyze_lp_sensitivity(original,limited).stop_reason==O::Termination::TimeLimit);
  limited=options;limited.limits.max_basis_solves=2;auto stopped=O::analyze_lp_sensitivity(original,limited);
  assert(stopped.completion==O::LpSensitivityCompletion::Interrupted&&stopped.sensitivity);
  for(const auto& e:stopped.sensitivity->entries())assert(e.group.state!=O::LpSensitivityState::Available&&!e.interval);
  limited=options;limited.backend=O::Backend::Native;assert(O::analyze_lp_sensitivity(original,limited).reason==O::LpSensitivityReason::Unsupported);
  limited=options;limited.parameters.push_back(O::LpObjectiveParameter{x});assert(O::analyze_lp_sensitivity(original,limited).reason==O::LpSensitivityReason::InvalidSource);
  std::cout<<"LP sensitivity analytic min/max, provenance and admission checks passed\n";
}
