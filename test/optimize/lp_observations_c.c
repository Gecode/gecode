/* C99 owning LP-observation ABI conformance and independent analytic duals. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define OK(call) do { int code=(call); if(code)fprintf(stderr,"%s: %s\n",#call,gecode_opt_v1_last_error());assert(code==GECODE_OPT_OK); } while(0)
#define LPINFO(h,i) OK(gecode_opt_v1_lp_observed_result_info(h,&i,sizeof(i)))
static void absent(gecode_opt_optional_number_v1 n){assert(!n.present&&n.value==0&&!n.reserved);}
static void present(gecode_opt_optional_number_v1 n,double x){assert(n.present&&!n.reserved&&fabs(n.value-x)<1e-6);}
static int available;
static void analytic(int maximize,double offset){
  gecode_opt_handle model=0,observed=0,copy=0,session=0;
  gecode_opt_id x,y,removed,dead,row;gecode_opt_lp_options_v1 options;
  gecode_opt_lp_info_v1 info;gecode_opt_lp_row_v1 r,rows[2];gecode_opt_lp_column_v1 c;
  gecode_opt_lp_checks_v1 checks;gecode_opt_lp_group_v1 group;gecode_opt_lp_metadata_v1 metadata;
  uint64_t needed=0;double value=0;gecode_opt_result_info_v1 ordinary;
  OK(gecode_opt_v1_model_create(&model));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,10,"x",&x));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,10,"y",&y));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,10,"removed",&removed));
  OK(gecode_opt_v1_model_remove_variable(model,removed));
  OK(gecode_opt_v1_model_add_row(model,NULL,0,-INFINITY,INFINITY,"dead",&dead));
  OK(gecode_opt_v1_model_remove_row(model,dead));
  {gecode_opt_term terms[2]={{x,1},{y,1}};
    OK(gecode_opt_v1_model_add_row(model,terms,2,maximize?-INFINITY:3,maximize?3:INFINITY,"demand",&row));
    terms[0].coefficient=maximize?4:1;terms[1].coefficient=maximize?1:2;
    OK(gecode_opt_v1_model_set_objective(model,terms,2,maximize?GECODE_OPT_MAXIMIZE:GECODE_OPT_MINIMIZE,offset));}
  OK(gecode_opt_v1_lp_options_default(&options,sizeof(options)));
  assert(options.struct_size==sizeof(options)&&!options.reserved&&options.duals&&options.basis);
  options.solve.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_session_create(&session));
  OK(gecode_opt_v1_session_solve_lp_observed(session,model,&options,&observed));
  LPINFO(observed,info);assert(info.struct_size==sizeof(info)&&!info.reserved&&!info.reserved_flags&&info.has_observations);
  assert(info.row_slots==2&&info.column_slots==3&&info.model_id==x.model_id);
  assert(gecode_opt_v1_result_info(observed,&ordinary,sizeof(ordinary))==GECODE_OPT_INVALID_HANDLE);
  assert(gecode_opt_v1_lp_observed_result_copy_result(model,&copy)==GECODE_OPT_INVALID_HANDLE&&copy==0);
  assert(gecode_opt_v1_lp_observed_result_info(observed,&info,sizeof(info)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_lp_observed_result_group(observed,GECODE_OPT_LP_PRIMAL_ROWS,&group,sizeof(group)));
  assert(gecode_opt_v1_lp_observed_result_group(observed,99,&group,sizeof(group))==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_lp_observed_result_row(observed,row,&r,sizeof(r)));
  assert(gecode_opt_v1_lp_observed_result_row(observed,dead,&r,sizeof(r))==GECODE_OPT_MODEL_ERROR);
  {gecode_opt_id foreign=row;foreign.model_id++;assert(gecode_opt_v1_lp_observed_result_row(observed,foreign,&r,sizeof(r))==GECODE_OPT_MODEL_ERROR);}
  assert(gecode_opt_v1_lp_observed_result_row(observed,x,&r,sizeof(r))==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_lp_observed_result_column(observed,removed,&c,sizeof(c))==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_lp_observed_result_rows(observed,NULL,sizeof(r),0,&needed));assert(needed==2);
  memset(rows,0xa5,sizeof(rows));
  assert(gecode_opt_v1_lp_observed_result_rows(observed,rows,sizeof(r),1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
  {unsigned char expected[sizeof(rows)];memset(expected,0xa5,sizeof(expected));assert(!memcmp(rows,expected,sizeof(rows)));}
  assert(gecode_opt_v1_lp_observed_result_rows(observed,rows,sizeof(r)-1,2,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_lp_observed_result_rows(observed,rows,sizeof(r),UINT64_MAX,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_lp_observed_result_rows(observed,rows,sizeof(r),2,&needed));assert(!rows[0].active&&rows[1].active);absent(rows[0].activity);
  OK(gecode_opt_v1_lp_observed_result_checks(observed,&checks,sizeof(checks)));
  OK(gecode_opt_v1_lp_observed_result_metadata(observed,&metadata,sizeof(metadata)));
  assert(!checks.reserved&&!metadata.reserved&&metadata.objective_gap==options.objective_gap);
  OK(gecode_opt_v1_lp_observed_result_copy_result(observed,&copy));
  if(available){
    assert(info.result.termination==GECODE_OPT_OPTIMAL&&info.result.has_solution&&checks.accepted);
    present(rows[1].activity,3);present(rows[1].dual,maximize?4:1);
    if(maximize){absent(rows[1].lower_slack);present(rows[1].upper_slack,0);}else{present(rows[1].lower_slack,0);absent(rows[1].upper_slack);}
    assert(rows[1].dual_source==GECODE_OPT_LP_DUAL_BACKEND&&rows[1].has_basis);
    OK(gecode_opt_v1_lp_observed_result_column(observed,y,&c,sizeof(c)));present(c.reduced_cost,maximize?-3:1);assert(c.has_basis&&c.basis==GECODE_OPT_LP_BASIS_LOWER);
    present(checks.normalized_gap,0);assert(metadata.backend_primal_tolerance.present&&metadata.backend_dual_tolerance.present);
    OK(gecode_opt_v1_result_value(copy,x,&value));assert(value==3);
  }else{assert(info.result.termination==GECODE_OPT_UNSUPPORTED&&!info.result.has_solution&&!checks.accepted);absent(rows[1].dual);absent(checks.normalized_gap);absent(metadata.backend_dual_tolerance);}
  /* Editing and a stopped call must never expose the preceding basis/duals. */
  OK(gecode_opt_v1_model_set_row_bounds(model,row,maximize?-INFINITY:4,maximize?4:INFINITY));
  {gecode_opt_handle stopped=0;gecode_opt_lp_options_v1 zero=options;zero.solve.time_limit_seconds=0;
    OK(gecode_opt_v1_session_solve_lp_observed(session,model,&zero,&stopped));LPINFO(stopped,info);
    assert(info.result.termination==(available?GECODE_OPT_TIME_LIMIT:GECODE_OPT_UNSUPPORTED));
    OK(gecode_opt_v1_lp_observed_result_row(stopped,row,&r,sizeof(r)));absent(r.activity);absent(r.dual);assert(!r.has_basis);
    OK(gecode_opt_v1_lp_observed_result_destroy(stopped));}
  OK(gecode_opt_v1_model_destroy(model));OK(gecode_opt_v1_session_destroy(session));
  OK(gecode_opt_v1_lp_observed_result_row(observed,row,&r,sizeof(r)));if(available)present(r.activity,3);
  OK(gecode_opt_v1_lp_observed_result_destroy(observed));
  if(available){OK(gecode_opt_v1_result_value(copy,x,&value));assert(value==3);}
  OK(gecode_opt_v1_result_destroy(copy));
  assert(gecode_opt_v1_lp_observed_result_destroy(observed)==GECODE_OPT_INVALID_HANDLE);
}
static void states(void){
  gecode_opt_handle model=0,result=0;gecode_opt_id x,row,constant,global;
  gecode_opt_lp_options_v1 options;gecode_opt_lp_group_v1 group;gecode_opt_lp_row_v1 observation;
  gecode_opt_lp_info_v1 info;
  OK(gecode_opt_v1_model_create(&model));OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,10,"x",&x));
  {gecode_opt_term term={x,1};OK(gecode_opt_v1_model_add_row(model,&term,1,2,INFINITY,"r",&row));OK(gecode_opt_v1_model_set_objective(model,&term,1,GECODE_OPT_MINIMIZE,7));}
  OK(gecode_opt_v1_model_add_row(model,NULL,0,-1,1,"constant",&constant));
  OK(gecode_opt_v1_solve_lp_observed(model,NULL,&result));
  OK(gecode_opt_v1_lp_observed_result_group(result,GECODE_OPT_LP_BASIS,&group,sizeof(group)));
  if(available){assert(group.state==GECODE_OPT_LP_UNAVAILABLE&&group.reason==GECODE_OPT_LP_REASON_ELIDED_CONSTANT_ROWS);
    OK(gecode_opt_v1_lp_observed_result_row(result,constant,&observation,sizeof(observation)));present(observation.dual,0);assert(observation.dual_source==GECODE_OPT_LP_DUAL_DERIVED_CONSTANT_ROW&&!observation.has_basis);}
  OK(gecode_opt_v1_lp_observed_result_destroy(result));
  for(int mode=0;mode<7;++mode){
    OK(gecode_opt_v1_lp_options_default(&options,sizeof(options)));
    if(mode==0)options.duals=options.basis=0;
    if(mode==1)options.solve.backend=GECODE_OPT_NATIVE;
    if(mode==2)options.solve.guarantee=GECODE_OPT_EXACT;
    if(mode==3){OK(gecode_opt_v1_model_add_all_different(model,NULL,0,"empty",&global));}
    if(mode==4){OK(gecode_opt_v1_model_remove_global(model,global));OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,0,"fixed integer",&global));}
    if(mode==5){options.reserved=1;result=123;assert(gecode_opt_v1_solve_lp_observed(model,&options,&result)==GECODE_OPT_INVALID_ARGUMENT&&result==0);continue;}
    if(mode==6){options.solve.struct_size--;result=123;assert(gecode_opt_v1_solve_lp_observed(model,&options,&result)==GECODE_OPT_INVALID_ARGUMENT&&result==0);continue;}
    OK(gecode_opt_v1_solve_lp_observed(model,&options,&result));LPINFO(result,info);
    if(mode==0){OK(gecode_opt_v1_lp_observed_result_group(result,GECODE_OPT_LP_DUAL_POINT,&group,sizeof(group)));assert(group.state==GECODE_OPT_LP_NOT_REQUESTED);}
    else assert(info.result.termination==GECODE_OPT_UNSUPPORTED&&!info.result.has_solution);
    OK(gecode_opt_v1_lp_observed_result_destroy(result));
  }
  OK(gecode_opt_v1_model_destroy(model));
}
int main(void){
  gecode_opt_lp_capabilities_v1 caps;char buffer[128];uint64_t n=0;
  OK(gecode_opt_v1_lp_capabilities(&caps,sizeof(caps)));available=caps.available;
  assert(caps.struct_size==sizeof(caps)&&!caps.reserved&&!caps.reserved_flags&&caps.limitation_count>0);
  OK(gecode_opt_v1_lp_capability_text(GECODE_OPT_LP_CAP_BACKEND,0,buffer,sizeof(buffer),&n));assert(n>1);
  assert(gecode_opt_v1_lp_capability_text(GECODE_OPT_LP_CAP_BACKEND,1,buffer,sizeof(buffer),&n)==GECODE_OPT_INVALID_ARGUMENT);
  for(int sense=0;sense<2;++sense){analytic(sense,7);analytic(sense,1e16);}states();
  puts("LP observation C99 conformance passed");return 0;
}
