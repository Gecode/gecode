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
static int32_t available;
static void near(double x,double y){assert(isfinite(x)&&fabs(x-y)<1e-7);}
static gecode_opt_handle analyze(gecode_opt_handle m,const gecode_opt_evidence_options_v1* options){
  gecode_opt_handle h=0;OK(gecode_opt_v1_analyze_lp_evidence(m,options,&h));assert(h);return h;
}
static void ray(void){
  gecode_opt_handle m=0,out=0,child=0;gecode_opt_id dead,x,y,row,gone,bad;
  gecode_opt_term terms[2];gecode_opt_evidence_options_v1 options;gecode_opt_evidence_info_v1 info;
  gecode_opt_evidence_group_v1 group;gecode_opt_evidence_primal_v1 primal;gecode_opt_evidence_slot_v1 slot;
  gecode_opt_evidence_stage_v1 stage;gecode_opt_evidence_column_v1 map[2];gecode_opt_evidence_raw_value_v1 raw[2];
  gecode_opt_result_info_v1 ordinary;uint64_t owner,revision,needed;double value;
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,INFINITY,"dead",&dead));
  OK(gecode_opt_v1_model_remove_variable(m,dead));OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,INFINITY,"x",&x));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,-INFINITY,INFINITY,"y",&y));
  OK(gecode_opt_v1_model_add_row(m,NULL,0,-INFINITY,INFINITY,"gone",&gone));OK(gecode_opt_v1_model_remove_row(m,gone));
  terms[0].variable=x;terms[0].coefficient=1;terms[1].variable=y;terms[1].coefficient=-1;
  OK(gecode_opt_v1_model_add_row(m,terms,2,0,0,"same",&row));terms[0].coefficient=-1;
  OK(gecode_opt_v1_model_set_objective(m,terms,1,GECODE_OPT_MINIMIZE,7));OK(gecode_opt_v1_model_identity(m,&owner,&revision));
  OK(gecode_opt_v1_evidence_options_default(&options,sizeof(options)));options.request=GECODE_OPT_EVIDENCE_BOTH;
  assert(!options.reserved&&!options.reserved_flags&&options.max_auxiliary_solves==3);
  out=analyze(m,&options);memset(&info,0xa5,sizeof(info));OK(gecode_opt_v1_lp_evidence_info(out,&info,sizeof(info)));
  assert(!info.reserved&&info.model_id==owner&&info.revision==revision&&info.has_evidence&&info.stage_count==3);
  assert(info.row_slots==2&&info.column_slots==3);OK(gecode_opt_v1_lp_evidence_group(out,GECODE_OPT_EVIDENCE_PRIMAL_GROUP,&group,sizeof(group)));
  assert(group.state==(available?GECODE_OPT_EVIDENCE_AVAILABLE:GECODE_OPT_EVIDENCE_REJECTED));
  if(available){assert(info.completion==GECODE_OPT_EVIDENCE_COMPLETE&&!info.has_stop_reason&&info.attempted_calls==3);
    OK(gecode_opt_v1_lp_evidence_value(out,x,GECODE_OPT_EVIDENCE_DIRECTION_VALUE,&value));near(value,1);
    OK(gecode_opt_v1_lp_evidence_value(out,y,GECODE_OPT_EVIDENCE_DIRECTION_VALUE,&value));near(value,1);
    OK(gecode_opt_v1_lp_evidence_value(out,x,GECODE_OPT_EVIDENCE_BASE_VALUE,&value));near(value,0);
  }else{assert(info.stop_reason==GECODE_OPT_UNSUPPORTED&&!info.attempted_calls);
    assert(gecode_opt_v1_lp_evidence_value(out,x,GECODE_OPT_EVIDENCE_DIRECTION_VALUE,&value)==GECODE_OPT_NO_EVIDENCE);}
  OK(gecode_opt_v1_lp_evidence_primal(out,&primal,sizeof(primal)));assert(!primal.reserved&&!primal.reserved_flags);
  assert(primal.has_base_check==available&&primal.normalized_objective_slope.present==available);
  if(available){assert(primal.base_check.valid);near(primal.normalized_objective_slope.value,-1);}
  bad=x;bad.model_id++;assert(gecode_opt_v1_lp_evidence_slot(out,bad,&slot,sizeof(slot))==GECODE_OPT_MODEL_ERROR);
  assert(gecode_opt_v1_lp_evidence_slot(out,dead,&slot,sizeof(slot))==GECODE_OPT_MODEL_ERROR);
  assert(gecode_opt_v1_lp_evidence_slot(out,gone,&slot,sizeof(slot))==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_lp_evidence_slot(out,row,&slot,sizeof(slot)));assert(slot.active&&slot.source.kind==GECODE_OPT_ROW_ID&&!slot.base_value.present);
  if(available){assert(slot.direction.present);near(slot.direction.value,0);}
  {gecode_opt_evidence_slot_v1 slots[3];OK(gecode_opt_v1_lp_evidence_slots(out,GECODE_OPT_VARIABLE_ID,slots,sizeof(slots[0]),3,&needed));
    assert(needed==3&&!slots[0].active&&!slots[0].direction.present&&!slots[0].base_value.present&&slots[0].direction.value==0);
    assert(!slots[1].reserved&&!slots[1].reserved_flags);}
  memset(&slot,0xa5,sizeof(slot));assert(gecode_opt_v1_lp_evidence_slots(out,GECODE_OPT_VARIABLE_ID,&slot,sizeof(slot),1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
  assert(needed==3&&slot.struct_size==UINT64_C(0xa5a5a5a5a5a5a5a5));
  assert(gecode_opt_v1_lp_evidence_slots(out,GECODE_OPT_VARIABLE_ID,NULL,sizeof(slot)-1,0,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_lp_evidence_slots(out,99,NULL,sizeof(slot),0,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_lp_evidence_info(out,&info,sizeof(info)-1)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_result_info(out,&ordinary,sizeof(ordinary))==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_lp_evidence_copy_stage(out,1,&child));OK(gecode_opt_v1_lp_evidence_stage_info(child,&stage,sizeof(stage)));
  assert(stage.phase==GECODE_OPT_EVIDENCE_RECESSION&&stage.private_model_id!=owner&&stage.column_count==2);
  assert(stage.attempted==available&&stage.has_raw_result==available&&stage.candidate_examined==available);
  if(available){assert(stage.check.valid&&stage.raw_result.reported_solution_validated);near(stage.raw_result.objective.value,-1);
    assert(stage.raw_result.model_id==stage.private_model_id&&stage.raw_result.revision==stage.private_revision);}
  assert(gecode_opt_v1_result_info(child,&ordinary,sizeof(ordinary))==GECODE_OPT_INVALID_HANDLE);
  assert(gecode_opt_v1_lp_evidence_stage_info(out,&stage,sizeof(stage))==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_lp_evidence_stage_columns(child,map,sizeof(map[0]),2,&needed));assert(needed==2);
  assert(map[0].source.model_id==owner&&map[0].source.slot==x.slot&&map[0].private_variable.model_id!=owner&&!map[0].has_side);
  OK(gecode_opt_v1_model_set_objective_offset(m,123));OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_lp_evidence_destroy(out));
  OK(gecode_opt_v1_lp_evidence_stage_info(child,&stage,sizeof(stage)));assert(stage.attempted==available);
  OK(gecode_opt_v1_lp_evidence_stage_raw_values(child,raw,sizeof(raw[0]),2,&needed));assert(needed==(available?2:0));
  if(available){near(raw[0].reported_value.value,1);assert(raw[0].reported_value.present&&raw[0].has_reported_mask&&raw[0].reported_mask==1);}
  OK(gecode_opt_v1_lp_evidence_stage_destroy(child));assert(gecode_opt_v1_lp_evidence_stage_destroy(child)==GECODE_OPT_INVALID_HANDLE);
}
static void farkas(void){
  gecode_opt_handle m=0,out=0,child=0;gecode_opt_id x,row;gecode_opt_term t;gecode_opt_evidence_options_v1 options;
  gecode_opt_evidence_group_v1 group;gecode_opt_evidence_farkas_v1 summary;gecode_opt_evidence_slot_v1 slot;
  gecode_opt_evidence_column_v1 maps[2];uint64_t needed;
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,-INFINITY,0,"x",&x));
  t.variable=x;t.coefficient=1;OK(gecode_opt_v1_model_add_row(m,&t,1,1,INFINITY,"impossible",&row));
  OK(gecode_opt_v1_evidence_options_default(&options,sizeof(options)));options.request=GECODE_OPT_EVIDENCE_FARKAS;
  out=analyze(m,&options);OK(gecode_opt_v1_lp_evidence_group(out,GECODE_OPT_EVIDENCE_PRIMAL_GROUP,&group,sizeof(group)));
  assert(group.state==GECODE_OPT_EVIDENCE_NOT_REQUESTED&&group.reason==GECODE_OPT_EVIDENCE_REASON_NOT_REQUESTED);
  OK(gecode_opt_v1_lp_evidence_farkas(out,&summary,sizeof(summary)));assert(summary.contradiction_margin.present==available);
  if(available){near(summary.contradiction_margin.value,1);OK(gecode_opt_v1_lp_evidence_multiplier(out,row,&slot,sizeof(slot)));
    assert(slot.has_side&&slot.side==GECODE_OPT_EVIDENCE_LOWER&&slot.selected_bound.present);near(slot.multiplier.value,1);near(slot.selected_bound.value,1);
    OK(gecode_opt_v1_lp_evidence_multiplier(out,x,&slot,sizeof(slot)));assert(slot.side==GECODE_OPT_EVIDENCE_UPPER);
    near(slot.multiplier.value,-1);assert(slot.contribution.present);near(slot.contribution.value,0);near(slot.selected_bound.value,0);
  }else assert(gecode_opt_v1_lp_evidence_multiplier(out,row,&slot,sizeof(slot))==GECODE_OPT_NO_EVIDENCE);
  OK(gecode_opt_v1_lp_evidence_copy_stage(out,0,&child));OK(gecode_opt_v1_lp_evidence_stage_columns(child,maps,sizeof(maps[0]),2,&needed));
  assert(needed==2&&maps[0].kind==GECODE_OPT_EVIDENCE_ROW_SIDE&&maps[0].source.slot==row.slot&&maps[0].has_side);
  assert(maps[1].kind==GECODE_OPT_EVIDENCE_VARIABLE_SIDE&&maps[1].source.slot==x.slot&&maps[1].side==GECODE_OPT_EVIDENCE_UPPER);
  OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_lp_evidence_destroy(out));OK(gecode_opt_v1_lp_evidence_stage_destroy(child));
}
static void options_and_stops(void){
  gecode_opt_handle m=0,out=0,cancel=0;gecode_opt_id x;gecode_opt_evidence_options_v1 options,bad;
  gecode_opt_evidence_info_v1 info;gecode_opt_evidence_group_v1 group;int i;
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,INFINITY,"x",&x));
  OK(gecode_opt_v1_evidence_options_default(&options,sizeof(options)));
  for(i=0;i<7;i++){
    bad=options;switch(i){case 0:bad.struct_size--;break;case 1:bad.reserved=1;break;case 2:bad.reserved_flags=1;break;
      case 3:bad.solve.struct_size--;break;case 4:bad.solve.has_node_limit=2;break;case 5:bad.request=99;break;default:bad.solve.reserved=1;break;}
    out=99;assert(gecode_opt_v1_analyze_lp_evidence(m,&bad,&out)==GECODE_OPT_INVALID_ARGUMENT&&out==0);
  }
  bad=options;bad.minimum_contradiction=NAN;out=99;assert(gecode_opt_v1_analyze_lp_evidence(m,&bad,&out)==GECODE_OPT_MODEL_ERROR&&out==0);
  for(i=0;i<8;i++){
    int expected;bad=options;switch(i){case 0:bad.solve.backend=GECODE_OPT_NATIVE;expected=GECODE_OPT_UNSUPPORTED;break;
      case 1:bad.solve.guarantee=GECODE_OPT_EXACT;expected=GECODE_OPT_UNSUPPORTED;break;
      case 2:bad.solve.time_limit_seconds=0;expected=GECODE_OPT_TIME_LIMIT;break;
      case 3:bad.solve.has_node_limit=1;bad.solve.node_limit=0;expected=GECODE_OPT_NODE_LIMIT;break;
      case 4:bad.max_work=0;expected=GECODE_OPT_ITERATION_LIMIT;break;
      case 5:bad.max_auxiliary_variables=0;expected=GECODE_OPT_MEMORY_LIMIT;break;
      case 6:bad.max_retained_slots=0;expected=GECODE_OPT_MEMORY_LIMIT;break;
      default:bad.max_auxiliary_solves=0;expected=available?GECODE_OPT_ITERATION_LIMIT:GECODE_OPT_UNSUPPORTED;break;}
    out=analyze(m,&bad);OK(gecode_opt_v1_lp_evidence_info(out,&info,sizeof(info)));
    assert(info.has_stop_reason&&info.stop_reason==expected&&!info.attempted_calls);
    if(!info.has_evidence)assert(gecode_opt_v1_lp_evidence_group(out,0,&group,sizeof(group))==GECODE_OPT_NO_EVIDENCE);
    OK(gecode_opt_v1_lp_evidence_destroy(out));
  }
  OK(gecode_opt_v1_cancellation_create(&cancel));OK(gecode_opt_v1_cancellation_cancel(cancel));bad=options;bad.solve.cancellation=cancel;
  out=analyze(m,&bad);OK(gecode_opt_v1_lp_evidence_info(out,&info,sizeof(info)));assert(info.stop_reason==GECODE_OPT_CANCELLED);
  OK(gecode_opt_v1_cancellation_destroy(cancel));OK(gecode_opt_v1_lp_evidence_destroy(out));OK(gecode_opt_v1_model_destroy(m));
}
int main(void){int32_t lp,mip;OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));
  ray();farkas();options_and_stops();puts("C99 LP evidence ownership, raw stages, groups and boundary tests passed");return 0;}
