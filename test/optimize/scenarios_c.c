/* C99 owning serial scenarios: source/private IDs, missing evidence and buffers. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define OK(call) do { int32_t code_=(call); if(code_){fprintf(stderr,"%s: %d %s\n",#call,(int)code_,gecode_opt_v1_last_error());assert(code_==0);} } while(0)
static gecode_opt_scenario_definition_v1 definition(void){
  gecode_opt_scenario_definition_v1 d;memset(&d,0,sizeof(d));d.struct_size=sizeof(d);return d;
}
static gecode_opt_scenario_bounds_v1 bound(gecode_opt_id id,int32_t lo,double lower,int32_t up,double upper){
  gecode_opt_scenario_bounds_v1 d;memset(&d,0,sizeof(d));d.struct_size=sizeof(d);d.entity=id;
  d.has_lower=lo;d.lower=lower;d.has_upper=up;d.upper=upper;return d;
}
static gecode_opt_scenario_info_v1 info(gecode_opt_handle h){
  gecode_opt_scenario_info_v1 i;memset(&i,0xa5,sizeof(i));OK(gecode_opt_v1_scenario_batch_info(h,&i,sizeof(i)));assert(!i.reserved);return i;
}
static gecode_opt_handle solve(gecode_opt_handle m,const gecode_opt_scenario_definition_v1* d,uint64_t n,const gecode_opt_scenario_options_v1* o){
  gecode_opt_handle r=0;OK(gecode_opt_v1_solve_scenarios(m,d,n,sizeof(*d),o,&r));assert(r);return r;
}
static void run(int backend){
  gecode_opt_handle model=0,result=0,child=0;gecode_opt_id x,y,dead,row,gone,mapped,mapped_row;
  gecode_opt_term terms[2];gecode_opt_scenario_options_v1 options;
  gecode_opt_scenario_definition_v1 defs[3];gecode_opt_scenario_bounds_v1 bounds[2];
  gecode_opt_scenario_id id,id1,id2,bad;gecode_opt_scenario_outcome_v1 outcome;gecode_opt_scenario_check_v1 checks;
  gecode_opt_scenario_info_v1 summary;gecode_opt_result_info_v1 ordinary;
  int32_t available,lp,mip,present;double value;uint64_t owner,revision,needed;char name[]="copied name";
  OK(gecode_opt_v1_capabilities(backend,&available,&lp,&mip));
  OK(gecode_opt_v1_model_create(&model));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,8,"x",&x));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,8,"y",&y));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,8,"dead",&dead));
  OK(gecode_opt_v1_model_remove_variable(model,dead));
  OK(gecode_opt_v1_model_add_row(model,NULL,0,-INFINITY,INFINITY,"gone",&gone));
  OK(gecode_opt_v1_model_remove_row(model,gone));
  terms[0].variable=x;terms[0].coefficient=1;terms[1].variable=y;terms[1].coefficient=1;
  OK(gecode_opt_v1_model_add_row(model,terms,2,4,INFINITY,"demand",&row));
  terms[0].coefficient=2;terms[1].coefficient=3;
  OK(gecode_opt_v1_model_set_objective(model,terms,2,GECODE_OPT_MINIMIZE,7));
  OK(gecode_opt_v1_model_identity(model,&owner,&revision));
  OK(gecode_opt_v1_scenario_options_default(&options,sizeof(options)));assert(options.struct_size==sizeof(options));
  assert(!options.reserved&&!options.reserved_flags&&!options.solve.reserved);options.solve.backend=backend;
  if(backend==GECODE_OPT_NATIVE)options.solve.guarantee=GECODE_OPT_EXACT;
  defs[0]=definition();defs[0].name=name;
  defs[1]=definition();bounds[0]=bound(x,0,NAN,1,1);defs[1].variable_bounds=bounds;defs[1].variable_count=1;
  defs[2]=definition();bounds[1]=bound(y,0,0,1,1);defs[2].variable_bounds=bounds;defs[2].variable_count=2;
  result=solve(model,defs,3,&options);name[0]='X';bounds[0].upper=7;
  summary=info(result);assert(summary.model_id==owner&&summary.revision==revision&&summary.has_batch);
  assert(summary.batch_id!=owner);assert(summary.scenario_count==3&&summary.outcome_count==3);
  OK(gecode_opt_v1_scenario_batch_id(result,0,&id));OK(gecode_opt_v1_scenario_batch_id(result,1,&id1));OK(gecode_opt_v1_scenario_batch_id(result,2,&id2));
  OK(gecode_opt_v1_scenario_batch_map(result,x,&mapped));OK(gecode_opt_v1_scenario_batch_map(result,row,&mapped_row));
  assert(mapped.model_id==id.batch_id&&mapped.slot==x.slot&&mapped.kind==GECODE_OPT_VARIABLE_ID);
  assert(mapped_row.model_id==id.batch_id&&mapped_row.slot==row.slot&&mapped_row.kind==GECODE_OPT_ROW_ID);
  assert(gecode_opt_v1_scenario_batch_map(result,dead,&mapped_row)==GECODE_OPT_MODEL_ERROR);
  assert(gecode_opt_v1_scenario_batch_map(result,gone,&mapped_row)==GECODE_OPT_MODEL_ERROR);
  {char buffer[32];OK(gecode_opt_v1_scenario_batch_text(result,id,GECODE_OPT_SCENARIO_NAME,buffer,sizeof(buffer),&needed));assert(!strcmp(buffer,"copied name"));}
  {gecode_opt_scenario_bounds_v1 b;OK(gecode_opt_v1_scenario_batch_bounds(result,id1,GECODE_OPT_VARIABLE_ID,&b,sizeof(b),1,&needed));
    assert(needed==1&&b.upper==1&&!b.has_lower&&b.lower==0&&b.reserved==0);}
  {gecode_opt_scenario_bounds_v1 b;memset(&b,0xa5,sizeof(b));
    assert(gecode_opt_v1_scenario_batch_bounds(result,id2,GECODE_OPT_VARIABLE_ID,&b,sizeof(b),1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
    assert(needed==2&&b.struct_size==UINT64_C(0xa5a5a5a5a5a5a5a5));
    assert(gecode_opt_v1_scenario_batch_bounds(result,id,GECODE_OPT_VARIABLE_ID,NULL,sizeof(b)-1,0,&needed)==GECODE_OPT_INVALID_ARGUMENT);}
  bad=id;bad.batch_id++;
  assert(gecode_opt_v1_scenario_batch_outcome(result,bad,&outcome,sizeof(outcome))==GECODE_OPT_MODEL_ERROR);
  bad=id;bad.index=3;assert(gecode_opt_v1_scenario_batch_check(result,bad,&checks,sizeof(checks))==GECODE_OPT_MODEL_ERROR);
  assert(gecode_opt_v1_scenario_batch_info(model,&summary,sizeof(summary))==GECODE_OPT_INVALID_HANDLE);
  assert(gecode_opt_v1_result_info(result,&ordinary,sizeof(ordinary))==GECODE_OPT_INVALID_HANDLE);
  assert(gecode_opt_v1_scenario_batch_outcome(result,id,&outcome,sizeof(outcome)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_scenario_batch_outcome(result,id,&outcome,sizeof(outcome)));
  assert(outcome.has_result&&outcome.state==GECODE_OPT_SCENARIO_ATTEMPTED&&!outcome.reserved&&!outcome.reserved_flags);
  OK(gecode_opt_v1_scenario_batch_check(result,id,&checks,sizeof(checks)));assert(!checks.reserved&&!checks.reserved_flags);
  if(available){
    summary=info(result);assert(summary.all_resolved&&summary.resolved==3&&summary.completion==GECODE_OPT_SCENARIO_COMPLETE);
    assert(!summary.has_stop_reason&&!summary.has_offending_scenario);
    assert(outcome.result.termination==GECODE_OPT_OPTIMAL&&checks.has_check&&checks.candidate_examined&&checks.validation.valid);
    OK(gecode_opt_v1_scenario_batch_value(result,id,x,&value));assert(value==4);
    OK(gecode_opt_v1_scenario_batch_value(result,id1,x,&value));assert(value==1);
    OK(gecode_opt_v1_scenario_batch_check(result,id2,&checks,sizeof(checks)));
    assert(checks.has_check&&!checks.candidate_examined&&!checks.validation.valid&&!checks.validation.objective.present);
    assert(gecode_opt_v1_scenario_batch_value(result,id2,x,&value)==GECODE_OPT_NO_SOLUTION);
  }else{
    assert(outcome.result.termination==GECODE_OPT_UNSUPPORTED);assert(!checks.candidate_examined&&!checks.validation.objective.present);
    OK(gecode_opt_v1_scenario_batch_outcome(result,id1,&outcome,sizeof(outcome)));
    assert(!outcome.has_result&&!outcome.has_check&&outcome.state==GECODE_OPT_SCENARIO_NOT_STARTED);
    memset(&checks,0xa5,sizeof(checks));OK(gecode_opt_v1_scenario_batch_check(result,id1,&checks,sizeof(checks)));
    assert(!checks.has_check&&!checks.candidate_examined&&!checks.validation.valid&&!checks.validation.objective.present);
    child=99;assert(gecode_opt_v1_scenario_batch_copy_result(result,id1,&child)==GECODE_OPT_NO_SOLUTION&&child==0);
  }
  OK(gecode_opt_v1_scenario_batch_copy_result(result,id,&child));OK(gecode_opt_v1_model_destroy(model));
  OK(gecode_opt_v1_scenario_batch_destroy(result));OK(gecode_opt_v1_result_info(child,&ordinary,sizeof(ordinary)));
  assert(ordinary.model_id==id.batch_id&&ordinary.revision==1);
  if(available){OK(gecode_opt_v1_result_value(child,mapped,&value));assert(value==4);
    assert(gecode_opt_v1_result_value(child,x,&value)==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_result_number(child,GECODE_OPT_OBJECTIVE,&present,&value));assert(present&&value==15);}
  OK(gecode_opt_v1_result_destroy(child));assert(gecode_opt_v1_scenario_batch_destroy(result)==GECODE_OPT_INVALID_HANDLE);
}
static void malformed_and_limits(void){
  gecode_opt_handle m=0,r=0,cancel=0;gecode_opt_id x;gecode_opt_scenario_options_v1 o,bad;
  gecode_opt_scenario_definition_v1 d[2];gecode_opt_scenario_bounds_v1 bounds;gecode_opt_term t[2];
  gecode_opt_scenario_info_v1 summary;gecode_opt_scenario_definition_info_v1 definition_info;
  uint64_t owner,revision,needed;int i;gecode_opt_scenario_id id;
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,0,2,"x",&x));
  OK(gecode_opt_v1_model_identity(m,&owner,&revision));OK(gecode_opt_v1_scenario_options_default(&o,sizeof(o)));
  d[0]=definition();d[1]=definition();
  for(i=0;i<7;i++){
    bad=o;switch(i){case 0:bad.struct_size--;break;case 1:bad.reserved=1;break;case 2:bad.reserved_flags=1;break;
      case 3:bad.solve.struct_size--;break;case 4:bad.solve.reserved=1;break;case 5:bad.reuse=9;break;case 6:bad.solve.has_node_limit=2;break;}
    r=99;assert(gecode_opt_v1_solve_scenarios(m,d,2,sizeof(d[0]),&bad,&r)==GECODE_OPT_INVALID_ARGUMENT&&r==0);
  }
  r=99;assert(gecode_opt_v1_solve_scenarios(m,d,1,sizeof(d[0])-1,&o,&r)==GECODE_OPT_INVALID_ARGUMENT&&r==0);
  r=99;assert(gecode_opt_v1_solve_scenarios(m,NULL,1,sizeof(d[0]),&o,&r)==GECODE_OPT_INVALID_ARGUMENT&&r==0);
  r=99;assert(gecode_opt_v1_solve_scenarios(m,d,UINT64_MAX,sizeof(d[0]),&o,&r)==GECODE_OPT_INVALID_ARGUMENT&&r==0);
  for(i=0;i<5;i++){
    d[0]=definition();bounds=bound(x,1,1,0,0);d[0].variable_bounds=&bounds;d[0].variable_count=1;
    switch(i){case 0:d[0].struct_size=0;break;case 1:d[0].reserved=1;break;case 2:d[0].objective_offset.present=2;break;
      case 3:bounds.struct_size=0;break;case 4:bounds.has_lower=2;break;}
    assert(gecode_opt_v1_solve_scenarios(m,d,1,sizeof(d[0]),&o,&r)==GECODE_OPT_INVALID_ARGUMENT&&r==0);
  }
  d[0]=definition();d[1]=definition();t[0].variable=x;t[0].coefficient=1;t[1]=t[0];
  d[1].objective_coefficients=t;d[1].objective_count=2;r=solve(m,d,2,&o);summary=info(r);
  assert(summary.completion==GECODE_OPT_SCENARIO_REJECTED&&summary.stop_reason==GECODE_OPT_INVALID_MODEL);
  assert(summary.has_offending_scenario&&summary.offending_scenario==1&&!summary.attempted&&!summary.has_batch);
  assert(summary.model_id==owner&&summary.revision==revision);OK(gecode_opt_v1_scenario_batch_message(r,NULL,0,&needed));assert(needed>1);
  OK(gecode_opt_v1_scenario_batch_destroy(r));d[1]=definition();
  for(i=0;i<5;i++){
    int expected;bad=o;switch(i){case 0:bad.solve.time_limit_seconds=0;expected=GECODE_OPT_TIME_LIMIT;break;
      case 1:bad.solve.has_node_limit=1;bad.solve.node_limit=0;expected=GECODE_OPT_NODE_LIMIT;break;
      case 2:bad.solve.has_node_limit=1;bad.solve.node_limit=1;expected=GECODE_OPT_UNSUPPORTED;break;
      case 3:bad.max_scenarios=1;expected=GECODE_OPT_MEMORY_LIMIT;break;
      default:bad.max_work=0;expected=GECODE_OPT_ITERATION_LIMIT;break;}
    r=solve(m,d,2,&bad);summary=info(r);assert(summary.has_stop_reason&&summary.stop_reason==expected&&!summary.attempted);
    OK(gecode_opt_v1_scenario_batch_destroy(r));
  }
  OK(gecode_opt_v1_cancellation_create(&cancel));OK(gecode_opt_v1_cancellation_cancel(cancel));bad=o;bad.solve.cancellation=cancel;
  r=solve(m,d,2,&bad);summary=info(r);assert(summary.stop_reason==GECODE_OPT_CANCELLED);OK(gecode_opt_v1_cancellation_destroy(cancel));OK(gecode_opt_v1_scenario_batch_destroy(r));
  r=solve(m,NULL,0,&o);summary=info(r);assert(summary.all_resolved&&summary.has_batch&&!summary.scenario_count);
  assert(gecode_opt_v1_scenario_batch_id(r,0,&id)==GECODE_OPT_MODEL_ERROR);OK(gecode_opt_v1_scenario_batch_destroy(r));
  d[0].objective_coefficients=t;d[0].objective_count=1;d[0].objective_offset.present=1;d[0].objective_offset.value=0;
  r=solve(m,d,1,&o);OK(gecode_opt_v1_scenario_batch_id(r,0,&id));
  memset(&definition_info,0xa5,sizeof(definition_info));OK(gecode_opt_v1_scenario_batch_definition(r,id,&definition_info,sizeof(definition_info)));
  assert(definition_info.objective_count==1&&definition_info.objective_offset.present&&definition_info.objective_offset.value==0&&!definition_info.reserved);
  OK(gecode_opt_v1_scenario_batch_destroy(r));OK(gecode_opt_v1_model_destroy(m));
}
int main(void){run(GECODE_OPT_HIGHS);run(GECODE_OPT_NATIVE);malformed_and_limits();puts("C99 scenario ownership/boundary tests passed");return 0;}
