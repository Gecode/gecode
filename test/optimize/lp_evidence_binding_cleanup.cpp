#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
static gecode_opt_handle cancellation=0;
extern "C" void gecode_opt_test_evidence_binding_checkpoint(void){
  if(cancellation)assert(gecode_opt_v1_cancellation_cancel(cancellation)==GECODE_OPT_OK);
}
static void ok(int32_t code){if(code)std::cerr<<gecode_opt_v1_last_error()<<'\n';assert(code==GECODE_OPT_OK);}
int main(){
  int32_t available,lp,mip;ok(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));
  if(!available){std::cout<<"LP evidence cleanup fixture: backend unavailable, no accepted evidence to revoke\n";return 0;}
  const double inf=std::numeric_limits<double>::infinity();
  for(bool farkas:{false,true}){
    gecode_opt_handle model=0,analysis=0,child=0;gecode_opt_id x{},row{};
    ok(gecode_opt_v1_model_create(&model));ok(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,farkas?-inf:0,farkas?0:inf,"x",&x));
    gecode_opt_term t{x,farkas?1.0:-1.0};
    if(farkas)ok(gecode_opt_v1_model_add_row(model,&t,1,1,inf,"contradiction",&row));
    else ok(gecode_opt_v1_model_set_objective(model,&t,1,GECODE_OPT_MINIMIZE,7));
    gecode_opt_evidence_options_v1 options{};ok(gecode_opt_v1_evidence_options_default(&options,sizeof(options)));
    options.request=farkas?GECODE_OPT_EVIDENCE_FARKAS:GECODE_OPT_EVIDENCE_PRIMAL_RAY;
    ok(gecode_opt_v1_analyze_lp_evidence(model,&options,&analysis));
    gecode_opt_evidence_group_v1 group{};
    ok(gecode_opt_v1_lp_evidence_group(analysis,farkas?1:0,&group,sizeof(group)));assert(group.state==GECODE_OPT_EVIDENCE_AVAILABLE);
    ok(gecode_opt_v1_lp_evidence_destroy(analysis));
    ok(gecode_opt_v1_cancellation_create(&cancellation));options.solve.cancellation=cancellation;
    ok(gecode_opt_v1_analyze_lp_evidence(model,&options,&analysis));
    gecode_opt_evidence_info_v1 info{};ok(gecode_opt_v1_lp_evidence_info(analysis,&info,sizeof(info)));
    assert(info.has_evidence&&info.completion==GECODE_OPT_EVIDENCE_INTERRUPTED&&info.has_stop_reason&&info.stop_reason==GECODE_OPT_CANCELLED);
    ok(gecode_opt_v1_lp_evidence_group(analysis,farkas?1:0,&group,sizeof(group)));
    assert(group.state==GECODE_OPT_EVIDENCE_UNAVAILABLE&&group.reason==GECODE_OPT_EVIDENCE_REASON_STOPPED);
    ok(gecode_opt_v1_lp_evidence_group(analysis,farkas?0:1,&group,sizeof(group)));assert(group.state==GECODE_OPT_EVIDENCE_NOT_REQUESTED);
    double value=0;gecode_opt_evidence_slot_v1 slot{};
    assert(gecode_opt_v1_lp_evidence_value(analysis,x,GECODE_OPT_EVIDENCE_BASE_VALUE,&value)==GECODE_OPT_NO_EVIDENCE);
    assert(gecode_opt_v1_lp_evidence_value(analysis,x,GECODE_OPT_EVIDENCE_DIRECTION_VALUE,&value)==GECODE_OPT_NO_EVIDENCE);
    assert(gecode_opt_v1_lp_evidence_multiplier(analysis,farkas?row:x,&slot,sizeof(slot))==GECODE_OPT_NO_EVIDENCE);
    ok(gecode_opt_v1_lp_evidence_slot(analysis,farkas?row:x,&slot,sizeof(slot)));
    assert(farkas?slot.multiplier.present:slot.direction.present);
    ok(gecode_opt_v1_lp_evidence_copy_stage(analysis,farkas?0:1,&child));
    ok(gecode_opt_v1_lp_evidence_destroy(analysis));ok(gecode_opt_v1_model_destroy(model));ok(gecode_opt_v1_cancellation_destroy(cancellation));cancellation=0;
    gecode_opt_evidence_stage_v1 stage{};ok(gecode_opt_v1_lp_evidence_stage_info(child,&stage,sizeof(stage)));
    assert(stage.attempted&&stage.candidate_examined&&stage.check.valid&&stage.has_raw_result);
    assert(stage.raw_result.reported_solution_validated&&stage.raw_result.termination_code==GECODE_OPT_OPTIMAL);
    gecode_opt_result_info_v1 ordinary{};assert(gecode_opt_v1_result_info(child,&ordinary,sizeof(ordinary))==GECODE_OPT_INVALID_HANDLE);
    ok(gecode_opt_v1_lp_evidence_stage_destroy(child));
  }
  std::cout<<"LP evidence binding final cancellation revokes accepted getters; raw child history survives\n";
}
