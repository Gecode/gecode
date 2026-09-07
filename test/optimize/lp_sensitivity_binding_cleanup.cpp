#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
static gecode_opt_handle cancellation=0;
extern "C" void gecode_opt_test_sensitivity_binding_checkpoint(void){
  if(cancellation)assert(gecode_opt_v1_cancellation_cancel(cancellation)==GECODE_OPT_OK);
}
static void ok(int32_t c){if(c)std::cerr<<gecode_opt_v1_last_error()<<'\n';assert(!c);}
int main(){
  int32_t available,lp,mip;ok(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));
  if(!available){std::cout<<"Sensitivity binding cleanup: no available backend to produce ranges\n";return 0;}
  const double inf=std::numeric_limits<double>::infinity();gecode_opt_handle m=0,source=0,h=0,copied=0,basis=0;
  gecode_opt_id x{},y{},unrequested{},row{};ok(gecode_opt_v1_model_create(&m));
  ok(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,inf,"x",&x));
  ok(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,inf,"y",&y));
  ok(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,1,"unrequested",&unrequested));
  gecode_opt_term terms[2]={{x,1},{y,1}};ok(gecode_opt_v1_model_add_row(m,terms,2,3,3,"balance",&row));terms[0].coefficient=2;
  ok(gecode_opt_v1_model_set_objective(m,terms,2,GECODE_OPT_MINIMIZE,7));ok(gecode_opt_v1_solve_lp_observed(m,nullptr,&source));
  gecode_opt_sensitivity_request_v1 requests[2]={{sizeof(requests[0]),0,GECODE_OPT_SENSITIVITY_OBJECTIVE,0,x},
    {sizeof(requests[0]),0,GECODE_OPT_SENSITIVITY_EQUALITY_RHS,0,row}};
  gecode_opt_sensitivity_options_v1 options{};ok(gecode_opt_v1_sensitivity_options_default(&options,sizeof(options)));options.requests=requests;options.request_count=2;
  ok(gecode_opt_v1_analyze_lp_sensitivity(source,&options,&h));gecode_opt_sensitivity_info_v1 info{};ok(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));
  assert(info.completion==GECODE_OPT_SENSITIVITY_COMPLETE);ok(gecode_opt_v1_sensitivity_destroy(h));
  ok(gecode_opt_v1_cancellation_create(&cancellation));options.cancellation=cancellation;
  ok(gecode_opt_v1_analyze_lp_sensitivity(source,&options,&h));ok(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));
  assert(info.completion==GECODE_OPT_SENSITIVITY_INTERRUPTED&&info.reason==GECODE_OPT_SENSITIVITY_REASON_STOPPED);
  assert(info.has_sensitivity&&info.has_basis&&info.has_stop_reason&&info.stop_reason==GECODE_OPT_CANCELLED);
  gecode_opt_sensitivity_entry_v1 entries[2]{},entry{};uint64_t needed;
  ok(gecode_opt_v1_sensitivity_entries(h,entries,sizeof(entries[0]),2,&needed));assert(needed==2);
  for(const auto& e:entries){assert(e.requested&&!e.has_interval&&e.group.state==GECODE_OPT_SENSITIVITY_UNAVAILABLE&&e.group.reason==GECODE_OPT_SENSITIVITY_REASON_STOPPED);
    assert(!e.objective_slope.present&&!e.checks.accepted&&!e.lower.value.present&&!e.has_lower_limiter);}
  ok(gecode_opt_v1_sensitivity_entry(h,0,&entry,sizeof(entry)));assert(!entry.has_interval);
  ok(gecode_opt_v1_sensitivity_objective(h,x,&entry,sizeof(entry)));assert(!entry.has_interval);
  ok(gecode_opt_v1_sensitivity_equality_rhs(h,row,&entry,sizeof(entry)));assert(!entry.has_interval);
  ok(gecode_opt_v1_sensitivity_objective(h,unrequested,&entry,sizeof(entry)));assert(!entry.requested&&entry.group.state==GECODE_OPT_SENSITIVITY_NOT_REQUESTED);
  char message[160];for(int field:{GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE,GECODE_OPT_SENSITIVITY_INTERVAL_MESSAGE}){
    ok(gecode_opt_v1_sensitivity_text(h,field,0,message,sizeof(message),&needed));assert(std::strstr(message,"stopped"));}
  gecode_opt_sensitivity_reference_checks_v1 checks{};ok(gecode_opt_v1_sensitivity_reference_checks(h,&checks,sizeof(checks)));
  assert(checks.primal.valid&&checks.kkt.accepted&&checks.basis_point_matches); // retained diagnostics, not intervals
  ok(gecode_opt_v1_sensitivity_copy_source_observed(h,&copied));ok(gecode_opt_v1_sensitivity_copy_basis(h,&basis));
  // Invalid options retain their original admission reason even at this hook.
  options.time_limit_seconds=-1;gecode_opt_handle invalid=0;ok(gecode_opt_v1_analyze_lp_sensitivity(source,&options,&invalid));
  ok(gecode_opt_v1_sensitivity_info(invalid,&info,sizeof(info)));assert(info.reason==GECODE_OPT_SENSITIVITY_REASON_INVALID_SOURCE&&!info.has_stop_reason);
  ok(gecode_opt_v1_sensitivity_destroy(invalid));ok(gecode_opt_v1_sensitivity_destroy(h));ok(gecode_opt_v1_lp_observed_result_destroy(source));
  ok(gecode_opt_v1_model_destroy(m));ok(gecode_opt_v1_cancellation_destroy(cancellation));cancellation=0;
  gecode_opt_lp_info_v1 original{};ok(gecode_opt_v1_lp_observed_result_info(copied,&original,sizeof(original)));assert(original.result.termination==GECODE_OPT_OPTIMAL);
  int32_t status;ok(gecode_opt_v1_basis_column(basis,y,&status));assert(status==GECODE_OPT_LP_BASIS_BASIC);
  ok(gecode_opt_v1_lp_observed_result_destroy(copied));ok(gecode_opt_v1_basis_destroy(basis));
  std::cout<<"Sensitivity binding cleanup revokes every range view and retains owning source/basis diagnostics\n";
}
