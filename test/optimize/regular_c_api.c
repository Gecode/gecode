/* Independent C99 consumer: deterministic signed word language plus API guards. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define OK(call) do {int32_t code=(call);if(code)fprintf(stderr,"%s: %s\n",#call,gecode_opt_v1_last_error());assert(code==GECODE_OPT_OK);}while(0)
static gecode_opt_regular_transition_v1 edge(uint64_t from,int64_t symbol,uint64_t to) {
  gecode_opt_regular_transition_v1 out;memset(&out,0,sizeof(out));out.struct_size=sizeof(out);out.from=from;out.symbol=symbol;out.to=to;return out;
}
static int accepted(int x,int y,int z) {
  /* This language is exactly signed words with an even count of +1 symbols. */
  return (x==-1||x==1)&&(y==-1||y==1)&&(z==-1||z==1)&&((x==1)+(y==1)+(z==1))%2==0;
}
int main(void) {
  int32_t native=0,lp=0,mip=0,maximize;
  OK(gecode_opt_v1_capabilities(GECODE_OPT_NATIVE,&native,&lp,&mip));
  for(maximize=0;maximize<2;++maximize) {
    gecode_opt_handle model=0,foreign_model=0,result=0,rejected=0;
    gecode_opt_id vars[3],dead,foreign,global,row,other,bad,out;
    gecode_opt_regular_transition_v1 edges[4],duplicate[2];uint64_t finals[2]={0,0},owner,revision,after;
    gecode_opt_term terms[3];gecode_opt_options_v1 options;gecode_opt_result_info_v1 info;
    int x,y,z,found=0,best=0;double value=0;int32_t present=0;size_t i;
    char label[]="parity";
    edges[0]=edge(0,-1,0);edges[1]=edge(0,1,1);edges[2]=edge(1,-1,1);edges[3]=edge(1,1,0);
    OK(gecode_opt_v1_model_create(&model));OK(gecode_opt_v1_model_create(&foreign_model));
    for(i=0;i<3;++i){OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,-1,1,"",&vars[i]));terms[i].variable=vars[i];terms[i].coefficient=1;}
    OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,1,"deleted",&dead));OK(gecode_opt_v1_model_remove_variable(model,dead));
    OK(gecode_opt_v1_model_add_variable(foreign_model,GECODE_OPT_INTEGER,0,1,"foreign",&foreign));
    OK(gecode_opt_v1_model_identity(model,&owner,&revision));
#define ADD(v,n,st,in,es,en,sz,fs,fn,name,target) gecode_opt_v1_model_add_regular(model,v,n,st,in,es,en,sz,fs,fn,name,target)
#define BAD(call,expected) do {memset(&out,0x55,sizeof(out));assert((call)==(expected));assert(out.model_id==0&&out.slot==0&&out.kind==0&&out.reserved==0);OK(gecode_opt_v1_model_identity(model,&owner,&after));assert(after==revision);}while(0)
    assert(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,NULL)==GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(NULL,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,UINT64_MAX,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,NULL,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,edges,UINT64_MAX,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0])-1,finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),NULL,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,UINT64_MAX,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    edges[3].reserved=1;BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);edges[3].reserved=0;
    edges[3].struct_size--;BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);edges[3].struct_size++;
    BAD(ADD(vars,3,0,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    BAD(ADD(vars,3,2,2,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    edges[3].to=2;BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);edges[3].to=0;
    edges[3].symbol=INT64_C(9007199254740993);BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);edges[3].symbol=1;
    finals[1]=2;BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);finals[1]=0;
    duplicate[0]=duplicate[1]=edges[0];BAD(ADD(vars,3,2,0,duplicate,2,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    duplicate[1].to=1;BAD(ADD(vars,3,2,0,duplicate,2,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    BAD(ADD(&foreign,1,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    BAD(ADD(&dead,1,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_MODEL_ERROR);
    bad=vars[0];bad.reserved=1;BAD(ADD(&bad,1,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    bad=vars[0];bad.kind=GECODE_OPT_ROW_ID;BAD(ADD(&bad,1,2,0,edges,4,sizeof(edges[0]),finals,2,label,&out),GECODE_OPT_INVALID_ARGUMENT);
    BAD(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,"\xed\xa0\x80",&out),GECODE_OPT_INVALID_ARGUMENT);
    OK(ADD(vars,3,2,0,edges,4,sizeof(edges[0]),finals,2,label,&global));
    OK(gecode_opt_v1_model_identity(model,&owner,&after));assert(after==revision+1&&global.kind==GECODE_OPT_GLOBAL_ID&&global.model_id==owner);
    memset(edges,0,sizeof(edges));memset(finals,0xff,sizeof(finals));label[0]='X'; /* Caller storage cannot alter posted meaning. */
    OK(gecode_opt_v1_model_add_all_different(model,vars,2,"",&other));
    OK(gecode_opt_v1_model_add_row(model,terms,3,-1,INFINITY,"",&row));
    terms[0].coefficient=2;terms[1].coefficient=-1;terms[2].coefficient=3;
    OK(gecode_opt_v1_model_set_objective(model,terms,3,maximize,7));
    for(x=-1;x<=1;++x)for(y=-1;y<=1;++y)for(z=-1;z<=1;++z)if(accepted(x,y,z)&&x!=y&&x+y+z>=-1) {
      int objective=2*x-y+3*z+7;if(!found||(maximize?objective>best:objective<best))best=objective;found=1;
    }
    assert(found&&best==(maximize?13:7));
    OK(gecode_opt_v1_options_default(&options,sizeof(options)));options.backend=GECODE_OPT_AUTO;options.guarantee=GECODE_OPT_EXACT;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    assert(info.termination==(native?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
    if(native){double point[3];OK(gecode_opt_v1_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));assert(present&&value==best);
      for(i=0;i<3;++i)OK(gecode_opt_v1_result_value(result,vars[i],&point[i]));assert(accepted((int)point[0],(int)point[1],(int)point[2]));}
    options.backend=GECODE_OPT_HIGHS;options.guarantee=GECODE_OPT_NUMERICAL;OK(gecode_opt_v1_solve(model,&options,&rejected));
    OK(gecode_opt_v1_result_info(rejected,&info,sizeof(info)));assert(info.termination==GECODE_OPT_UNSUPPORTED&&!info.has_solution);
    assert(gecode_opt_v1_model_remove_variable(model,vars[2])==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_model_set_global_name(model,global,"renamed"));OK(gecode_opt_v1_model_remove_global(model,global));
    assert(gecode_opt_v1_model_remove_global(model,global)==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_model_destroy(model));OK(gecode_opt_v1_model_destroy(foreign_model));
    if(native){OK(gecode_opt_v1_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));assert(present&&value==best);}
    OK(gecode_opt_v1_result_destroy(result));OK(gecode_opt_v1_result_destroy(rejected));
#undef ADD
#undef BAD
  }
  puts("C99 Regular word oracle, mixed constraints, copying and atomic boundary checks passed");return 0;
}
