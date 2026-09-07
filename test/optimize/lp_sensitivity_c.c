#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define OK(call) do {int32_t c_=(call);if(c_){fprintf(stderr,"%s: %s\n",#call,gecode_opt_v1_last_error());assert(!c_);}}while(0)
static int32_t available;
static void near(double a,double b){assert(isfinite(a)&&fabs(a-b)<1e-7);}
static gecode_opt_sensitivity_request_v1 request(int32_t kind,gecode_opt_id id){
  gecode_opt_sensitivity_request_v1 r;memset(&r,0,sizeof(r));r.struct_size=sizeof(r);r.kind=kind;r.entity=id;return r;
}
static gecode_opt_handle analyze(gecode_opt_handle source,const gecode_opt_sensitivity_options_v1* o){
  gecode_opt_handle h=0;OK(gecode_opt_v1_analyze_lp_sensitivity(source,o,&h));assert(h);return h;
}
static void model(gecode_opt_handle* m,gecode_opt_handle* observed,gecode_opt_id ids[6]){
  gecode_opt_term t[2];OK(gecode_opt_v1_model_create(m));
  OK(gecode_opt_v1_model_add_variable(*m,GECODE_OPT_CONTINUOUS,0,1,"dead",&ids[0]));OK(gecode_opt_v1_model_remove_variable(*m,ids[0]));
  OK(gecode_opt_v1_model_add_variable(*m,GECODE_OPT_CONTINUOUS,0,INFINITY,"x",&ids[1]));
  OK(gecode_opt_v1_model_add_variable(*m,GECODE_OPT_CONTINUOUS,0,INFINITY,"y",&ids[2]));
  OK(gecode_opt_v1_model_add_variable(*m,GECODE_OPT_CONTINUOUS,0,1,"unrequested",&ids[3]));
  OK(gecode_opt_v1_model_add_row(*m,NULL,0,-INFINITY,INFINITY,"gone",&ids[4]));OK(gecode_opt_v1_model_remove_row(*m,ids[4]));
  t[0].variable=ids[1];t[0].coefficient=1;t[1].variable=ids[2];t[1].coefficient=1;
  OK(gecode_opt_v1_model_add_row(*m,t,2,3,3,"balance",&ids[5]));t[0].coefficient=2;
  OK(gecode_opt_v1_model_set_objective(*m,t,2,GECODE_OPT_MINIMIZE,7));OK(gecode_opt_v1_solve_lp_observed(*m,NULL,observed));
}
static void analytic_history(void){
  gecode_opt_handle m=0,observed=0,h=0,copy=0,basis=0,ordinary=0;gecode_opt_id ids[6],bad,order[1];
  gecode_opt_sensitivity_options_v1 o;gecode_opt_sensitivity_request_v1 requests[3];
  gecode_opt_sensitivity_info_v1 info;gecode_opt_sensitivity_work_v1 work,before;
  gecode_opt_sensitivity_entry_v1 entries[3],single;gecode_opt_sensitivity_reference_checks_v1 checks;
  gecode_opt_lp_info_v1 original;gecode_opt_result_info_v1 result;uint64_t n,owner,revision;uint8_t mask[4];int32_t status;
  model(&m,&observed,ids);OK(gecode_opt_v1_model_identity(m,&owner,&revision));
  requests[0]=request(0,ids[1]);requests[1]=request(0,ids[2]);requests[2]=request(1,ids[5]);
  OK(gecode_opt_v1_sensitivity_options_default(&o,sizeof(o)));o.requests=requests;o.request_count=3;
  assert(!o.reserved&&!o.reserved_flags&&!o.checks.reserved&&!o.limits.reserved);h=analyze(observed,&o);
  memset(&info,0xa5,sizeof(info));OK(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));
  assert(info.model_id==owner&&info.revision==revision&&info.guarantee==GECODE_OPT_NUMERICAL&&!info.reserved&&!info.reserved_flags);
  OK(gecode_opt_v1_sensitivity_work(h,&before,sizeof(before)));assert(before.preparation_visits==3);
  assert(gecode_opt_v1_result_info(h,&result,sizeof(result))==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_sensitivity_copy_source_observed(h,&copy));OK(gecode_opt_v1_lp_observed_result_info(copy,&original,sizeof(original)));
  assert(original.result.model_id==owner&&original.result.revision==revision);
  if(available){
    assert(info.completion==GECODE_OPT_SENSITIVITY_COMPLETE&&info.has_sensitivity&&info.has_basis&&!info.has_stop_reason);
    assert(info.entry_count==3&&info.factor_order_count==1&&info.column_slots==4&&info.row_slots==2);
    assert(before.factor_setup_attempted&&before.basis_solves>0&&before.coordinator_visits>0);
    OK(gecode_opt_v1_sensitivity_entries(h,NULL,sizeof(entries[0]),0,&n));assert(n==3);
    memset(entries,0xa5,sizeof(entries));assert(gecode_opt_v1_sensitivity_entries(h,entries,sizeof(entries[0]),2,&n)==GECODE_OPT_BUFFER_TOO_SMALL);
    assert(entries[0].struct_size==UINT64_C(0xa5a5a5a5a5a5a5a5));
    OK(gecode_opt_v1_sensitivity_entries(h,entries,sizeof(entries[0]),3,&n));
    assert(n==3&&entries[0].has_interval&&entries[0].requested&&entries[0].index==0&&entries[2].index==2);
    assert(entries[0].group.state==GECODE_OPT_SENSITIVITY_AVAILABLE&&entries[0].checks.accepted);
    assert(!entries[0].reserved&&!entries[0].lower.reserved&&!entries[0].checks.reserved_flags);
    near(entries[0].anchor,2);near(entries[0].lower.value.value,1);assert(entries[0].lower.value.present);
    assert(entries[0].upper.kind==GECODE_OPT_RANGE_POSITIVE_INFINITY&&!entries[0].upper.value.present&&entries[0].upper.value.value==0);
    assert(entries[1].lower.kind==GECODE_OPT_RANGE_NEGATIVE_INFINITY&&!entries[1].lower.value.present);near(entries[1].upper.value.value,2);
    near(entries[2].lower.value.value,0);assert(entries[2].upper.kind==GECODE_OPT_RANGE_POSITIVE_INFINITY);near(entries[2].objective_slope.value,1);
    OK(gecode_opt_v1_sensitivity_objective(h,ids[3],&single,sizeof(single)));assert(!single.requested&&!single.has_interval&&single.group.state==GECODE_OPT_SENSITIVITY_NOT_REQUESTED);
    OK(gecode_opt_v1_sensitivity_equality_rhs(h,ids[5],&single,sizeof(single)));assert(single.index==2);near(single.anchor,3);
    bad=ids[1];bad.model_id++;assert(gecode_opt_v1_sensitivity_objective(h,bad,&single,sizeof(single))==GECODE_OPT_MODEL_ERROR);
    assert(gecode_opt_v1_sensitivity_objective(h,ids[0],&single,sizeof(single))==GECODE_OPT_MODEL_ERROR);
    assert(gecode_opt_v1_sensitivity_equality_rhs(h,ids[4],&single,sizeof(single))==GECODE_OPT_MODEL_ERROR);
    assert(gecode_opt_v1_sensitivity_objective(h,ids[5],&single,sizeof(single))==GECODE_OPT_INVALID_ARGUMENT);
    OK(gecode_opt_v1_sensitivity_factor_order(h,order,1,&n));assert(n==1&&order[0].model_id==owner&&order[0].slot==ids[2].slot&&order[0].kind==GECODE_OPT_VARIABLE_ID);
    OK(gecode_opt_v1_sensitivity_active_slots(h,GECODE_OPT_VARIABLE_ID,mask,4,&n));assert(n==4&&!mask[0]&&mask[1]&&mask[2]&&mask[3]);
    OK(gecode_opt_v1_sensitivity_reference_checks(h,&checks,sizeof(checks)));assert(checks.primal.valid&&checks.primal.model_valid&&checks.kkt.accepted&&checks.basis_point_matches);
    assert(checks.max_system_residual.present&&!checks.reserved&&!checks.kkt.reserved);
    OK(gecode_opt_v1_sensitivity_copy_basis(h,&basis));
    {char text[1];assert(gecode_opt_v1_sensitivity_text(h,GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE,0,text,1,&n)==GECODE_OPT_BUFFER_TOO_SMALL);assert(n>1);}
    assert(gecode_opt_v1_sensitivity_text(h,GECODE_OPT_SENSITIVITY_MESSAGE,1,NULL,0,&n)==GECODE_OPT_INVALID_ARGUMENT);
    assert(gecode_opt_v1_sensitivity_entries(h,NULL,sizeof(single)-1,0,&n)==GECODE_OPT_INVALID_ARGUMENT);
  }else{
    assert(info.completion==GECODE_OPT_SENSITIVITY_ANALYSIS_REJECTED&&!info.has_sensitivity&&!before.factor_setup_attempted&&!before.basis_solves);
    assert(gecode_opt_v1_sensitivity_entry(h,0,&single,sizeof(single))==GECODE_OPT_NO_SENSITIVITY);
    assert(gecode_opt_v1_sensitivity_copy_basis(h,&basis)==GECODE_OPT_NO_BASIS&&!basis);
  }
  OK(gecode_opt_v1_sensitivity_work(h,&work,sizeof(work)));assert(!memcmp(&work,&before,sizeof(work)));
  OK(gecode_opt_v1_model_set_objective_offset(m,999));OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_lp_observed_result_destroy(observed));
  OK(gecode_opt_v1_sensitivity_destroy(h));OK(gecode_opt_v1_lp_observed_result_copy_result(copy,&ordinary));
  OK(gecode_opt_v1_result_info(ordinary,&result,sizeof(result)));assert(result.model_id==owner&&result.revision==revision);
  if(available){double value;int32_t present;OK(gecode_opt_v1_result_number(ordinary,GECODE_OPT_OBJECTIVE,&present,&value));assert(present);near(value,10);
    OK(gecode_opt_v1_basis_column(basis,ids[2],&status));assert(status==GECODE_OPT_LP_BASIS_BASIC);OK(gecode_opt_v1_basis_destroy(basis));}
  OK(gecode_opt_v1_lp_observed_result_destroy(copy));OK(gecode_opt_v1_result_destroy(ordinary));
}
static void admission(void){
  gecode_opt_handle m=0,observed=0,h=0,token=0;gecode_opt_id ids[6];gecode_opt_sensitivity_request_v1 r,saved;
  gecode_opt_sensitivity_options_v1 o,bad;gecode_opt_sensitivity_info_v1 info;gecode_opt_sensitivity_work_v1 work;int i;int32_t cancelled;
  model(&m,&observed,ids);r=request(0,ids[1]);saved=r;OK(gecode_opt_v1_sensitivity_options_default(&o,sizeof(o)));o.requests=&r;o.request_count=1;
  for(i=0;i<13;i++){
    bad=o;r=saved;switch(i){case 0:bad.struct_size--;break;case 1:bad.reserved=1;break;case 2:bad.reserved_flags=1;break;
      case 3:bad.checks.struct_size--;break;case 4:bad.checks.reserved=1;break;case 5:bad.limits.struct_size--;break;
      case 6:bad.limits.reserved=1;break;case 7:bad.backend=99;break;case 8:bad.requests=NULL;break;
      case 9:r.struct_size--;break;case 10:r.reserved=1;break;case 11:r.kind=99;break;default:r.entity.kind=GECODE_OPT_ROW_ID;break;}
    h=99;assert(gecode_opt_v1_analyze_lp_sensitivity(observed,&bad,&h)==GECODE_OPT_INVALID_ARGUMENT&&!h);
  }
  r=saved;h=99;assert(gecode_opt_v1_analyze_lp_sensitivity(m,&o,&h)==GECODE_OPT_INVALID_HANDLE&&!h);
  for(i=0;i<6;i++){
    bad=o;switch(i){case 0:bad.request_count=0;break;case 1:bad.checks.stationarity=NAN;break;case 2:bad.time_limit_seconds=-1;break;
      case 3:bad.time_limit_seconds=0;break;case 4:bad.limits.max_work=0;break;default:bad.limits.max_requests=0;break;}
    h=analyze(observed,&bad);OK(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));OK(gecode_opt_v1_sensitivity_work(h,&work,sizeof(work)));
    assert(!work.factor_setup_attempted&&!work.basis_solves&&!work.preparation_visits);
    if(i<3){assert(info.completion==GECODE_OPT_SENSITIVITY_ANALYSIS_REJECTED&&info.reason==GECODE_OPT_SENSITIVITY_REASON_INVALID_SOURCE&&!info.has_stop_reason);}
    else{assert(info.completion==GECODE_OPT_SENSITIVITY_INTERRUPTED&&info.has_stop_reason);
      assert(info.stop_reason==(i==3?GECODE_OPT_TIME_LIMIT:i==4?GECODE_OPT_ITERATION_LIMIT:GECODE_OPT_MEMORY_LIMIT));}
    OK(gecode_opt_v1_sensitivity_destroy(h));
  }
  OK(gecode_opt_v1_cancellation_create(&token));OK(gecode_opt_v1_cancellation_is_cancelled(token,&cancelled));assert(!cancelled);
  assert(gecode_opt_v1_cancellation_is_cancelled(token,NULL)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_cancellation_is_cancelled(m,&cancelled)==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_cancellation_cancel(token));OK(gecode_opt_v1_cancellation_is_cancelled(token,&cancelled));assert(cancelled);
  bad=o;bad.cancellation=token;h=analyze(observed,&bad);OK(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));assert(info.stop_reason==GECODE_OPT_CANCELLED);OK(gecode_opt_v1_sensitivity_destroy(h));
  bad.time_limit_seconds=-1;h=analyze(observed,&bad);OK(gecode_opt_v1_sensitivity_info(h,&info,sizeof(info)));assert(info.reason==GECODE_OPT_SENSITIVITY_REASON_INVALID_SOURCE&&!info.has_stop_reason);OK(gecode_opt_v1_sensitivity_destroy(h));
  {gecode_opt_handle copied=0;OK(gecode_opt_v1_cancellation_copy(token,&copied));OK(gecode_opt_v1_cancellation_destroy(token));
    OK(gecode_opt_v1_cancellation_is_cancelled(copied,&cancelled));assert(cancelled);OK(gecode_opt_v1_cancellation_destroy(copied));
    copied=99;assert(gecode_opt_v1_cancellation_copy(m,&copied)==GECODE_OPT_INVALID_HANDLE&&!copied);}
  OK(gecode_opt_v1_lp_observed_result_destroy(observed));OK(gecode_opt_v1_model_destroy(m));
}
int main(void){int32_t lp,mip;OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));analytic_history();admission();puts("C99 sensitivity endpoints, source/basis history, counted buffers and admission passed");return 0;}
