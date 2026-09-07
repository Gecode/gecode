#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/lp_sensitivity.hpp>
#include <gecode/optimize/lp_sensitivity_backend.hpp>
#include <gecode/optimize/lp_observations_detail.hpp>
#include <cassert>
#include <cmath>
#include <iostream>
#include <new>
#include "lp_sensitivity_fixture.hpp"

namespace O=Gecode::Optimize;
namespace {
int fault=0;std::size_t calls=0;std::string stop_event;
std::shared_ptr<O::CancellationToken> token;
struct Factor : O::Detail::LpSensitivityFactor {
  ~Factor() override {if(fault==10&&token)token->cancel();}
  std::vector<double> solve(const std::vector<double>& rhs,bool) override {
    ++calls;assert(rhs.size()==1);
    if(fault==5)return {};
    if(fault==6)return {std::numeric_limits<double>::quiet_NaN()};
    if(fault==7&&calls==1)return {rhs[0]+1};
    if(fault==8&&calls==2)return {rhs[0]+1};
    if(fault==9&&calls==3)return {rhs[0]+1};
    if(fault==12&&calls==3)return {std::numeric_limits<double>::denorm_min()};
    if(fault==13&&calls==3)return {1e-200};
    return rhs; // Selected B=[1]. This is not an optimization backend.
  }
};
void clear(const O::LpSensitivityResult& out) {
  if(out.sensitivity)for(const auto& entry:out.sensitivity->entries())
    assert(entry.group.state!=O::LpSensitivityState::Available&&!entry.interval);
}
}
namespace Gecode {namespace Optimize {namespace Detail {
std::unique_ptr<LpSensitivityFactor> lp_sensitivity_test_factor(
  const ModelSnapshot& source,const LpBasis&,double,const SolveBudget&) {
  calls=0;if(fault==1)return {};
  auto out=std::make_unique<Factor>();out->version="test factor";
  out->columns={0,1};out->rows={0};out->order={source.variables[1].variable};
  if(fault==2)out->columns={0};
  if(fault==3)out->order={Variable{source.model_id+1,1}};
  if(fault==4)out->order={source.variables[0].variable};
  return out;
}
void lp_sensitivity_test_checkpoint(const char* point,std::size_t index) {
  if(stop_event==point&&token)token->cancel();
  if(fault==11&&std::string(point)=="after_interval"&&index==0)throw std::bad_alloc();
  if(fault==14&&std::string(point)=="after_interval"&&index==2)throw std::runtime_error("test backend boundary exception");
}
}}}
int main() {
  O::Model model;auto x=model.add_continuous(),y=model.add_continuous();auto row=model.add_row({{x,1},{y,1}},3,3);model.minimize({{x,2},{y,1}},7);
  const auto source=SensitivityFixture::two_column_observed(model);O::LpSensitivityOptions options;
  options.parameters={O::LpObjectiveParameter{x},O::LpObjectiveParameter{y},O::LpEqualityRhsParameter{row}};
  auto normal=O::analyze_lp_sensitivity(source,options);assert(normal.completion==O::LpSensitivityCompletion::Complete);
  for(fault=1;fault<=8;++fault){auto out=O::analyze_lp_sensitivity(source,options);assert(out.completion==O::LpSensitivityCompletion::Rejected);clear(out);}
  fault=9;auto partial=O::analyze_lp_sensitivity(source,options);
  assert(partial.completion==O::LpSensitivityCompletion::Partial);
  assert(partial.sensitivity->entries()[0].group.state==O::LpSensitivityState::Available);
  assert(partial.sensitivity->entries()[1].group.state==O::LpSensitivityState::Rejected&&!partial.sensitivity->entries()[1].interval);
  assert(partial.sensitivity->entries()[2].group.state==O::LpSensitivityState::Available);
  token=std::make_shared<O::CancellationToken>();options.cancellation=token;stop_event="after_cleanup";
  auto stopped_partial=O::analyze_lp_sensitivity(source,options);
  assert(stopped_partial.completion==O::LpSensitivityCompletion::Interrupted&&stopped_partial.stop_reason==O::Termination::Cancelled);clear(stopped_partial);
  stop_event.clear();token.reset();options.cancellation.reset();
  // Deliberately loosen the linear-system tolerance to admit a corrupt tiny
  // derivative. Finite endpoint overflow must still never become infinity.
  auto loose=options;loose.checks.system_absolute=2;fault=12;
  auto overflow=O::analyze_lp_sensitivity(source,loose);
  assert(overflow.completion==O::LpSensitivityCompletion::Partial);
  assert(overflow.sensitivity->entries()[1].group.reason==O::LpSensitivityReason::FailedIntervalChecks);
  assert(!overflow.sensitivity->entries()[1].interval);
  fault=13;auto tiny=O::analyze_lp_sensitivity(source,loose);
  assert(tiny.completion==O::LpSensitivityCompletion::Complete);
  const auto& finite=tiny.sensitivity->entries()[1].interval->upper;
  assert(finite.kind==O::LpRangeEndKind::Finite&&finite.value&&*finite.value>1e199);
  fault=0;
  for(const char* phase:{"before_copy","after_copy","before_factor","after_factor","before_system","after_system","after_reference","before_interval","after_interval","after_cleanup"}) {
    token=std::make_shared<O::CancellationToken>();options.cancellation=token;stop_event=phase;
    auto stopped=O::analyze_lp_sensitivity(source,options);assert(stopped.completion==O::LpSensitivityCompletion::Interrupted&&stopped.stop_reason==O::Termination::Cancelled);clear(stopped);
  }
  stop_event.clear();token=std::make_shared<O::CancellationToken>();options.cancellation=token;fault=10;
  auto cleanup=O::analyze_lp_sensitivity(source,options);assert(cleanup.stop_reason==O::Termination::Cancelled);clear(cleanup);
  options.cancellation.reset();token.reset();fault=11;
  auto allocation=O::analyze_lp_sensitivity(source,options);assert(allocation.reason==O::LpSensitivityReason::AllocationFailure&&allocation.stop_reason==O::Termination::MemoryLimit);clear(allocation);
  fault=14;auto exception=O::analyze_lp_sensitivity(source,options);
  assert(exception.reason==O::LpSensitivityReason::BackendFailure&&exception.completion==O::LpSensitivityCompletion::Rejected);clear(exception);
  fault=0;
  for(int limit=0;limit<7;++limit){auto o=options;
    if(limit==0)o.limits.max_rows=0;
    if(limit==1)o.limits.max_columns=1;
    if(limit==2)o.limits.max_nonzeros=1;
    if(limit==3)o.limits.max_factor_entries=0;
    if(limit==4)o.limits.max_requests=2;
    if(limit==5)o.limits.max_retained_slots=1;
    if(limit==6)o.limits.max_work=0;
    auto rejected=O::analyze_lp_sensitivity(source,o);assert(rejected.reason==O::LpSensitivityReason::ResourceLimit&&!rejected.work.factor_setup_attempted);clear(rejected);}
  for(int option=0;option<7;++option){auto o=options;
    if(option==0)o.checks.system_absolute=-1;
    if(option==1)o.checks.system_relative=std::numeric_limits<double>::quiet_NaN();
    if(option==2)o.parameters.clear();
    if(option==3)o.backend=static_cast<O::Backend>(99);
    if(option==4)o.time_limit_seconds=-1;
    if(option==5)o.time_limit_seconds=std::numeric_limits<double>::quiet_NaN();
    if(option==6){o.time_limit_seconds=-1;o.cancellation=std::make_shared<O::CancellationToken>();o.cancellation->cancel();}
    auto rejected=O::analyze_lp_sensitivity(source,o);assert(rejected.completion==O::LpSensitivityCompletion::Rejected&&!rejected.work.factor_setup_attempted);clear(rejected);}
  // Inactive semantic payloads are still owning copied history. Empty table
  // tuple containers therefore consume retained slots even with no entries.
  auto historical=model.snapshot();O::GlobalData inactive;
  inactive.global={model.id(),0};inactive.active=false;
  O::TableData table;table.tuples.resize(1000);inactive.payload=std::move(table);historical.globals.push_back(std::move(inactive));
  auto history=SensitivityFixture::two_column_observed(std::move(historical));auto retention=options;
  retention.limits.max_retained_slots=normal.work.retained_slots+1000;
  auto retained=O::analyze_lp_sensitivity(history,retention);
  assert(retained.reason==O::LpSensitivityReason::ResourceLimit&&!retained.work.factor_setup_attempted);clear(retained);
  assert(O::analyze_lp_sensitivity(source,options).completion==O::LpSensitivityCompletion::Complete);
  std::cout<<"LP sensitivity malformed-factor, partial, caps, allocation and cleanup coordinator passed\n";
}
