/* Compile this file as C99, then link the shared C ABI library. */
#include <gecode/optimize/c_api.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define CHECK(test) do { if(!(test)){fprintf(stderr,"FAIL line %d: %s (%s)\n",__LINE__,#test,gecode_opt_v1_last_error());return 1;} } while(0)
#define OK(call) CHECK((call)==GECODE_OPT_OK)
static int near(double a,double b){return isfinite(a)&&fabs(a-b)<1e-6;}

static int bulk(void) {
  gecode_opt_handle m=0,foreign=0,historical=0,current=0;
  gecode_opt_id dead,other,vars[5],rows[2],sentinel[2],mapping[2];
  gecode_opt_variable_spec_v1 specs[5];gecode_opt_row_spec_v1 rs[2];gecode_opt_sparse_row_batch_v1 csr;
  gecode_opt_term ts[4];gecode_opt_options_v1 options;gecode_opt_result_info_v1 info;
  uint64_t owner=0,revision=0,after=0,start[3]={0,3,4},column[4]={1,1,1,0};
  double coefficient[4]={1e16,1,-1e16,1},lower[2]={2,0},upper[2]={INFINITY,4},value=0;
  int32_t available=0,lp=0,mip=0,present=0;size_t i;
  char name[]="retained";const char* names[2]={"demand",NULL};
  memset(specs,0,sizeof(specs));memset(rs,0,sizeof(rs));memset(&csr,0,sizeof(csr));
  memset(sentinel,0x5a,sizeof(sentinel));memcpy(rows,sentinel,sizeof(rows));
  for(i=0;i<5;++i){specs[i].struct_size=sizeof(specs[i]);specs[i].type=(int32_t)i;specs[i].upper=i==2?1:5;}
  specs[3].lower=2;specs[4].lower=2.5;specs[0].name=name;
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_create(&foreign));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,1,"hole",&dead));
  OK(gecode_opt_v1_model_remove_variable(m,dead));
  OK(gecode_opt_v1_model_add_variable(foreign,GECODE_OPT_CONTINUOUS,0,1,"",&other));
  OK(gecode_opt_v1_model_identity(m,&owner,&revision));
  CHECK(gecode_opt_v1_model_add_variables(m,specs,5,NULL,0)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(gecode_opt_v1_model_add_variables(m,specs,5,NULL,5)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_variables(m,specs,5,rows,2)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(memcmp(rows,sentinel,sizeof(rows))==0);
  CHECK(gecode_opt_v1_model_add_variables(m,NULL,1,vars,5)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_variables(m,specs,UINT64_MAX,vars,5)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,UINT64_MAX)==GECODE_OPT_INVALID_ARGUMENT);
  specs[4].struct_size--;CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5)==GECODE_OPT_INVALID_ARGUMENT);specs[4].struct_size++;
  specs[4].reserved=1;CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5)==GECODE_OPT_INVALID_ARGUMENT);specs[4].reserved=0;
  specs[4].type=99;CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5)==GECODE_OPT_INVALID_ARGUMENT);specs[4].type=4;
  specs[4].lower=NAN;CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5)==GECODE_OPT_MODEL_ERROR);specs[4].lower=2.5;
  {const char* bad[]={"\xc0\x80","\xed\xa0\x80","\xf4\x90\x80\x80","\x80","\xe2"};
    for(i=0;i<sizeof(bad)/sizeof(bad[0]);++i){specs[4].name=bad[i];
      CHECK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5)==GECODE_OPT_INVALID_ARGUMENT);}}
  specs[4].name="\xcf\x80 \xf0\x9f\x98\x80";
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision);
  OK(gecode_opt_v1_model_add_variables(m,NULL,0,NULL,0));
  OK(gecode_opt_v1_model_add_rows(m,NULL,0,NULL,0));
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision);
  OK(gecode_opt_v1_model_add_variables(m,specs,5,vars,5));name[0]='X';
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision+1);revision=after;
  for(i=0;i<5;++i)CHECK(vars[i].model_id==owner&&vars[i].slot==i+1&&vars[i].kind==GECODE_OPT_VARIABLE_ID);
  ts[0].variable=vars[1];ts[0].coefficient=1;ts[1].variable=vars[0];ts[1].coefficient=1;
  rs[0].struct_size=sizeof(rs[0]);rs[0].terms=ts;rs[0].term_count=2;rs[0].lower=3;rs[0].upper=INFINITY;
  rs[1].struct_size=sizeof(rs[1]);rs[1].lower=-INFINITY;rs[1].upper=INFINITY;
  CHECK(gecode_opt_v1_model_add_rows(m,rs,2,NULL,0)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(gecode_opt_v1_model_add_rows(m,rs,2,NULL,2)==GECODE_OPT_INVALID_ARGUMENT);
  rs[1].reserved=1;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_INVALID_ARGUMENT);rs[1].reserved=0;
  rs[1].struct_size--;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_INVALID_ARGUMENT);rs[1].struct_size++;
  rs[1].term_count=1;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_INVALID_ARGUMENT);rs[1].term_count=0;
  ts[0].variable=other;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_MODEL_ERROR);
  ts[0].variable=dead;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_MODEL_ERROR);ts[0].variable=vars[1];
  ts[0].variable.kind=GECODE_OPT_ROW_ID;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_INVALID_ARGUMENT);ts[0].variable=vars[1];
  ts[0].coefficient=INFINITY;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_INVALID_ARGUMENT);ts[0].coefficient=1;
  rs[1].lower=4;rs[1].upper=3;CHECK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2)==GECODE_OPT_MODEL_ERROR);rs[1].lower=-INFINITY;rs[1].upper=INFINITY;
  CHECK(memcmp(rows,sentinel,sizeof(rows))==0);
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision);
  OK(gecode_opt_v1_model_add_rows(m,rs,2,rows,2));
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision+1);revision=after;
  ts[0].coefficient=1;ts[1].coefficient=2;OK(gecode_opt_v1_model_set_objective(m,ts,2,GECODE_OPT_MINIMIZE,.25));
  OK(gecode_opt_v1_options_default(&options,sizeof(options)));options.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));
  OK(gecode_opt_v1_solve(m,&options,&historical));
  OK(gecode_opt_v1_result_info(historical,&info,sizeof(info)));CHECK(info.termination==(available?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  OK(gecode_opt_v1_model_identity(m,&owner,&revision));
  csr.struct_size=sizeof(csr);csr.columns=mapping;csr.columns_count=2;mapping[0]=vars[0];mapping[1]=vars[1];
  csr.row_start=start;csr.row_start_count=3;csr.column=column;csr.column_count=4;csr.coefficient=coefficient;csr.coefficient_count=4;
  csr.lower=lower;csr.lower_count=2;csr.upper=upper;csr.upper_count=2;csr.names=names;csr.names_count=2;
  CHECK(gecode_opt_v1_model_add_rows_sparse(m,NULL,rows,2)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,NULL,0)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,NULL,2)==GECODE_OPT_INVALID_ARGUMENT);
  csr.reserved=1;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.reserved=0;
  csr.struct_size--;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.struct_size++;
  csr.upper_count=1;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.upper_count=2;
  csr.row_start_count=0;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.row_start_count=3;
  csr.coefficient_count=3;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.coefficient_count=4;
  csr.names_count=1;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.names_count=2;
  csr.coefficient=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.coefficient=coefficient;
  csr.names=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.names=names;
  csr.row_start=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.row_start=start;
  csr.column=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.column=column;
  csr.lower=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.lower=lower;
  csr.upper=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.upper=upper;
  csr.columns=NULL;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.columns=mapping;
  csr.columns_count=UINT64_MAX;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);csr.columns_count=2;
  coefficient[3]=NAN;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_INVALID_ARGUMENT);coefficient[3]=1;
  column[3]=2;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_MODEL_ERROR);column[3]=0;
  start[1]=5;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_MODEL_ERROR);start[1]=3;
  mapping[0]=other;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_MODEL_ERROR);mapping[0]=vars[1];
  CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2)==GECODE_OPT_MODEL_ERROR);mapping[0]=vars[0];
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision);
  lower[0]=4;OK(gecode_opt_v1_model_add_rows_sparse(m,&csr,rows,2));
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision+1);revision=after;
  OK(gecode_opt_v1_solve(m,&options,&current));
  if(available){
    OK(gecode_opt_v1_result_number(current,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&near(value,4.25));
    OK(gecode_opt_v1_result_number(historical,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&near(value,3.25));
    OK(gecode_opt_v1_result_value(historical,vars[1],&value));CHECK(near(value,3));
  }
  /* Valid empty CSR still rejects unused foreign/deleted/duplicate mappings. */
  memset(&csr,0,sizeof(csr));csr.struct_size=sizeof(csr);csr.row_start=start;csr.row_start_count=1;
  csr.columns=mapping;csr.columns_count=1;mapping[0]=dead;
  CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,NULL,0)==GECODE_OPT_MODEL_ERROR);
  mapping[0]=other;CHECK(gecode_opt_v1_model_add_rows_sparse(m,&csr,NULL,0)==GECODE_OPT_MODEL_ERROR);
  mapping[0]=vars[0];OK(gecode_opt_v1_model_add_rows_sparse(m,&csr,NULL,0));
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(after==revision);
  OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_model_destroy(foreign));
  if(available){OK(gecode_opt_v1_result_value(historical,vars[1],&value));CHECK(near(value,3));}
  OK(gecode_opt_v1_result_destroy(current));OK(gecode_opt_v1_result_destroy(historical));
  CHECK(gecode_opt_v1_model_add_variables(m,NULL,0,NULL,0)==GECODE_OPT_INVALID_HANDLE);
  return 0;
}

static int workflows(void) {
  gecode_opt_handle m=0,foreign=0,pool=0,repair=0,entry=0,stage=0,final=0,token=0;
  gecode_opt_id x,y,dead,row,other,projection[2],mapped[3],sources[3],lock;
  gecode_opt_term terms[2];
  gecode_opt_pool_options_v1 po;gecode_opt_repair_options_v1 ro;
  gecode_opt_pool_info_v1 pi;gecode_opt_pool_entry_info_v1 ei;gecode_opt_pool_attempt_info_v1 ai;
  gecode_opt_repair_info_v1 ri;gecode_opt_repair_item_info_v1 item;gecode_opt_repair_stage_info_v1 si;
  gecode_opt_result_info_v1 result_info;gecode_opt_validation_info_v1 validation;
  gecode_opt_relaxation_selection_v1 selection;
  int32_t highs=0,native=0,lp=0,mip=0,present=0;
  uint64_t owner=0,revision=0,needed=0;int64_t point=99;
  double value=0,values[3]={99,99,99};uint8_t active[3]={9,9,9},available[3]={9,9,9};char message[1]={'!'};
  OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&highs,&lp,&mip));
  OK(gecode_opt_v1_capabilities(GECODE_OPT_NATIVE,&native,&lp,&mip));
  CHECK(gecode_opt_v1_pool_options_default(NULL,sizeof(po))==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_repair_options_default(&ro,sizeof(ro)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_pool_options_default(&po,sizeof(po)));OK(gecode_opt_v1_repair_options_default(&ro,sizeof(ro)));
  CHECK(po.struct_size==sizeof(po)&&po.solve.struct_size==sizeof(po.solve)&&po.max_solutions==10);
  CHECK(ro.struct_size==sizeof(ro)&&!ro.optimize_original_objective);
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_create(&foreign));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,-1,1,"x",&x));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_CONTINUOUS,0,4,"recourse",&y));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_BINARY,0,1,"deleted",&dead));
  OK(gecode_opt_v1_model_remove_variable(m,dead));
  OK(gecode_opt_v1_model_add_variable(foreign,GECODE_OPT_INTEGER,0,1,"foreign",&other));
  terms[0].variable=x;terms[0].coefficient=1;terms[1].variable=y;terms[1].coefficient=1;
  OK(gecode_opt_v1_model_add_row(m,terms,2,2,INFINITY,"demand",&row));
  terms[0].coefficient=2;OK(gecode_opt_v1_model_set_objective(m,terms,2,GECODE_OPT_MINIMIZE,5));
  OK(gecode_opt_v1_model_identity(m,&owner,&revision));
  po.reserved=1;pool=99;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT&&pool==0);po.reserved=0;
  po.solve.struct_size--;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);po.solve.struct_size++;
  po.max_solutions=UINT64_MAX;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);po.max_solutions=5;
  po.has_projection=2;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);po.has_projection=1;
  po.projection_count=1;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);
  projection[0]=row;po.projection=projection;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);
  po.projection_count=UINT64_MAX;CHECK(gecode_opt_v1_pool_solve(m,&po,&pool)==GECODE_OPT_INVALID_ARGUMENT);po.projection_count=1;
  projection[0]=other;OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));
  CHECK(pi.termination==GECODE_OPT_INVALID_MODEL);OK(gecode_opt_v1_pool_destroy(pool));
  projection[0]=dead;OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));
  CHECK(pi.termination==GECODE_OPT_INVALID_MODEL);OK(gecode_opt_v1_pool_destroy(pool));
  projection[0]=x;po.solve.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));
  CHECK(pi.model_id==owner&&pi.revision==revision);
  CHECK(pi.termination==(highs?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  CHECK(gecode_opt_v1_result_destroy(pool)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_pool_projection(pool,NULL,0,&needed));CHECK(needed==1);
  CHECK(gecode_opt_v1_pool_projection(pool,NULL,1,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_pool_entry_result(pool,UINT64_MAX,&entry)==GECODE_OPT_INVALID_ARGUMENT&&entry==0);
  OK(gecode_opt_v1_pool_message(pool,NULL,0,&needed));CHECK(needed>1);
  CHECK(gecode_opt_v1_pool_message(pool,message,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL&&message[0]=='!');
  if(highs){
    CHECK(pi.completion==GECODE_OPT_POOL_EXHAUSTED&&pi.entry_count==3&&pi.ranked_prefix==3&&pi.attempt_count==4);
    OK(gecode_opt_v1_pool_entry_info(pool,0,&ei,sizeof(ei)));CHECK(ei.rank_established&&ei.projection_count==1);
    CHECK(gecode_opt_v1_pool_entry_projection(pool,0,&point,0,&needed)==GECODE_OPT_BUFFER_TOO_SMALL&&point==99);
    OK(gecode_opt_v1_pool_entry_projection(pool,0,&point,1,&needed));CHECK(point==-1);
    OK(gecode_opt_v1_pool_attempt_info(pool,3,&ai,sizeof(ai)));CHECK(ai.termination==GECODE_OPT_INFEASIBLE&&!ai.candidate_accepted);
    OK(gecode_opt_v1_pool_entry_result(pool,0,&entry));
    OK(gecode_opt_v1_result_info(entry,&result_info,sizeof(result_info)));CHECK(result_info.termination==GECODE_OPT_UNKNOWN);
    OK(gecode_opt_v1_result_number(entry,GECODE_OPT_BEST_BOUND,&present,&value));CHECK(!present&&value==0);
    OK(gecode_opt_v1_result_values(entry,values,active,available,3,&needed));CHECK(!active[2]&&!available[2]&&values[2]==0);
  }else CHECK(pi.completion==GECODE_OPT_POOL_INCOMPLETE&&pi.entry_count==0);
  OK(gecode_opt_v1_pool_destroy(pool));CHECK(gecode_opt_v1_pool_destroy(pool)==GECODE_OPT_INVALID_HANDLE);
  po.max_solutions=1;OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));
  if(highs)CHECK(pi.completion==GECODE_OPT_POOL_REQUESTED_LIMIT&&pi.termination==GECODE_OPT_SOLUTION_LIMIT&&pi.entry_count==1);
  OK(gecode_opt_v1_pool_destroy(pool));
  /* Keep a historical entry across source edits and destruction. */
  OK(gecode_opt_v1_model_set_row_bounds(m,row,6,INFINITY));
  selection.source=row;selection.side=GECODE_OPT_RELAX_LOWER;selection.reserved=0;selection.penalty=2;
  ro.solve.backend=GECODE_OPT_HIGHS;ro.selections=&selection;ro.selection_count=1;ro.optimize_original_objective=1;
  ro.struct_size--;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT&&repair==0);ro.struct_size++;
  ro.reserved=1;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);ro.reserved=0;
  ro.selection_count=UINT64_MAX;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);ro.selection_count=1;
  ro.selections=NULL;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);ro.selections=&selection;
  selection.reserved=1;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);selection.reserved=0;
  selection.source.reserved=1;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);selection.source.reserved=0;
  selection.side=7;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);selection.side=GECODE_OPT_RELAX_LOWER;
  selection.penalty=NAN;CHECK(gecode_opt_v1_repair_solve(m,&ro,&repair)==GECODE_OPT_INVALID_ARGUMENT);selection.penalty=2;
  selection.source=other;OK(gecode_opt_v1_repair_solve(m,&ro,&repair));OK(gecode_opt_v1_repair_info(repair,&ri,sizeof(ri)));
  CHECK(ri.termination==GECODE_OPT_INVALID_MODEL);OK(gecode_opt_v1_repair_destroy(repair));selection.source=row;
  OK(gecode_opt_v1_repair_solve(m,&ro,&repair));OK(gecode_opt_v1_repair_info(repair,&ri,sizeof(ri)));
  CHECK(ri.termination==(highs?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));CHECK(ri.source_model_id==owner&&ri.variable_slots==3);
  CHECK(gecode_opt_v1_pool_destroy(repair)==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_repair_variable_map(repair,sources,mapped,active,3,&needed));CHECK(needed==3&&!active[2]);
  CHECK(sources[0].model_id==owner&&mapped[0].model_id!=owner&&mapped[0].model_id==ri.private_model_id);
  OK(gecode_opt_v1_repair_item_info(repair,0,&item,sizeof(item)));CHECK(item.source.kind==GECODE_OPT_ROW_ID&&item.source.slot==row.slot);
  CHECK(item.slack.model_id==ri.private_model_id&&near(item.penalty,2));
  if(highs){
    CHECK(ri.has_repair&&ri.minimum_violation_established&&ri.original_objective_optimized&&ri.completed_stages==2);
    OK(gecode_opt_v1_repair_number(repair,GECODE_OPT_REPAIR_VIOLATION,&present,&value));CHECK(present&&near(value,2));
    OK(gecode_opt_v1_repair_original_value(repair,x,&value));CHECK(near(value,1));
    CHECK(gecode_opt_v1_repair_original_value(repair,mapped[0],&value)==GECODE_OPT_MODEL_ERROR);
    CHECK(gecode_opt_v1_repair_original_value(repair,dead,&value)==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_repair_validation(repair,&validation,sizeof(validation)));CHECK(!validation.valid&&validation.model_valid&&near(validation.max_row_violation,1));
    OK(gecode_opt_v1_repair_stage_info(repair,0,&si,sizeof(si)));CHECK(si.completed&&si.retention_bound.present);
    OK(gecode_opt_v1_repair_stage_result(repair,0,&stage));OK(gecode_opt_v1_repair_final_result(repair,&final));
    OK(gecode_opt_v1_repair_violation_lock(repair,&present,&lock));CHECK(present&&lock.model_id==ri.private_model_id);
    OK(gecode_opt_v1_repair_original_values(repair,values,active,available,3,&needed));CHECK(available[0]&&!available[2]&&!active[2]);
  }else{
    CHECK(!ri.has_repair);CHECK(gecode_opt_v1_repair_final_result(repair,&final)==GECODE_OPT_NO_SOLUTION&&final==0);
    CHECK(!item.activity.present&&item.activity.value==0);
  }
  values[0]=99;active[0]=9;available[0]=9;
  CHECK(gecode_opt_v1_repair_original_values(repair,values,active,available,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(needed==3&&values[0]==99&&active[0]==9&&available[0]==9);
  CHECK(gecode_opt_v1_repair_text(repair,GECODE_OPT_REPAIR_MESSAGE,1,NULL,0,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_repair_stage_result(repair,UINT64_MAX,&token)==GECODE_OPT_INVALID_ARGUMENT&&token==0);
  OK(gecode_opt_v1_repair_destroy(repair));OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_model_destroy(foreign));
  if(highs){
    OK(gecode_opt_v1_result_number(entry,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&near(value,6));
    OK(gecode_opt_v1_result_value(entry,x,&value));CHECK(near(value,-1));OK(gecode_opt_v1_result_destroy(entry));
    OK(gecode_opt_v1_result_value(final,mapped[0],&value));CHECK(near(value,1));
    CHECK(gecode_opt_v1_result_value(final,x,&value)==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_result_info(stage,&result_info,sizeof(result_info)));CHECK(result_info.model_id==mapped[0].model_id);
    OK(gecode_opt_v1_result_destroy(stage));OK(gecode_opt_v1_result_destroy(final));
  }
  /* Cancellation and zero deadlines are workflow outcomes without a fallback. */
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_cancellation_create(&token));OK(gecode_opt_v1_cancellation_cancel(token));
  OK(gecode_opt_v1_pool_options_default(&po,sizeof(po)));po.solve.cancellation=token;
  OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));CHECK(pi.termination==GECODE_OPT_CANCELLED&&pi.entry_count==0);
  OK(gecode_opt_v1_pool_destroy(pool));OK(gecode_opt_v1_cancellation_destroy(token));
  OK(gecode_opt_v1_repair_options_default(&ro,sizeof(ro)));ro.solve.time_limit_seconds=0;
  OK(gecode_opt_v1_repair_solve(m,&ro,&repair));OK(gecode_opt_v1_repair_info(repair,&ri,sizeof(ri)));CHECK(ri.termination==GECODE_OPT_TIME_LIMIT&&!ri.has_repair);
  OK(gecode_opt_v1_repair_destroy(repair));
  /* Native exact enumeration is explicit and needs no persistent session. */
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,-1,0,"x",&x));
  OK(gecode_opt_v1_pool_options_default(&po,sizeof(po)));po.solve.backend=GECODE_OPT_NATIVE;po.solve.guarantee=GECODE_OPT_EXACT;
  OK(gecode_opt_v1_pool_solve(m,&po,&pool));OK(gecode_opt_v1_pool_info(pool,&pi,sizeof(pi)));
  CHECK(pi.termination==(native?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  if(native)CHECK(pi.entry_count==2&&pi.completion==GECODE_OPT_POOL_EXHAUSTED&&pi.guarantee==GECODE_OPT_EXACT);
  OK(gecode_opt_v1_pool_destroy(pool));OK(gecode_opt_v1_model_destroy(m));
  return 0;
}

static int globals_and_logic(void) {
  gecode_opt_handle m=0,foreign=0,r=0,session=0;
  gecode_opt_id x,y,index,other,g,table,element,cumulative,circuit,successors[2],ids[2];
  gecode_opt_result_info_v1 info;
  gecode_opt_options_v1 options;
  gecode_opt_term objective[2];
  uint64_t owner=0,before=0,after=0;
  int32_t native=0,highs=0,lp=0,mip=0,present=0;
  int64_t tuples[4]={0,1,1,0},durations[2]={0,1},heights[2]={100,0};
  double value=0;
  OK(gecode_opt_v1_capabilities(GECODE_OPT_NATIVE,&native,&lp,&mip));
  OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&highs,&lp,&mip));
  OK(gecode_opt_v1_model_create(&m));OK(gecode_opt_v1_model_create(&foreign));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,0,1,"x",&x));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,0,1,"y",&y));
  OK(gecode_opt_v1_model_add_variable(foreign,GECODE_OPT_INTEGER,0,1,"other",&other));
  ids[0]=x;ids[1]=other;
  OK(gecode_opt_v1_model_identity(m,&owner,&before));
  CHECK(gecode_opt_v1_model_add_all_different(m,ids,2,"foreign",&g)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_add_all_different(m,NULL,1,"null",&g)==GECODE_OPT_INVALID_ARGUMENT);
  ids[1]=y;
  CHECK(gecode_opt_v1_model_add_table(m,ids,2,tuples,3,2,"arity",&g)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_table(m,ids,2,NULL,4,2,"null",&g)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_table(m,ids,2,tuples,0,UINT64_MAX,"overflow",&g)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_cumulative(m,ids,2,durations,1,heights,2,0,"length",&g)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_cumulative(m,ids,2,durations,2,heights,2,-1,"capacity",&g)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_add_circuit(m,ids,2,INT64_MAX,"base",&g)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(before==after);
  OK(gecode_opt_v1_model_add_all_different(m,ids,2,"distinct",&g));CHECK(g.kind==GECODE_OPT_GLOBAL_ID);
  CHECK(gecode_opt_v1_model_remove_variable(m,x)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_remove_global(m,x)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_remove_global(foreign,g)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_set_global_name(m,g,"renamed"));
  OK(gecode_opt_v1_model_add_table(m,ids,2,tuples,4,2,"allowed",&table));
  /* One-based element index selects x, which also aliases the result. */
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,1,1,"index",&index));
  OK(gecode_opt_v1_model_add_element(m,index,ids,2,x,1,"element",&element));
  ids[1]=x;
  OK(gecode_opt_v1_model_add_cumulative(m,ids,2,durations,2,heights,2,0,"zero usage",&cumulative));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,2,2,"s0",&successors[0]));
  OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,1,1,"s1",&successors[1]));
  OK(gecode_opt_v1_model_add_circuit(m,successors,2,1,"cycle",&circuit));
  objective[0].variable=x;objective[0].coefficient=1;objective[1].variable=y;objective[1].coefficient=2;
  OK(gecode_opt_v1_model_set_objective(m,objective,2,GECODE_OPT_MINIMIZE,0));
  OK(gecode_opt_v1_options_default(&options,sizeof(options)));
  OK(gecode_opt_v1_solve(m,&options,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
  CHECK(info.termination==(native?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  if(native){OK(gecode_opt_v1_result_number(r,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&near(value,1));}
  OK(gecode_opt_v1_result_destroy(r));options.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_solve(m,&options,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
  CHECK(info.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_result_destroy(r));
  options.backend=GECODE_OPT_AUTO;OK(gecode_opt_v1_session_create(&session));
  OK(gecode_opt_v1_session_solve(session,m,&options,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
  CHECK(info.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_result_destroy(r));OK(gecode_opt_v1_session_destroy(session));
  OK(gecode_opt_v1_model_remove_global(m,table));
  CHECK(gecode_opt_v1_model_remove_global(m,table)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_set_global_name(m,table,"dead")==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_destroy(m));OK(gecode_opt_v1_model_destroy(foreign));

  /* Zero-arity table distinguishes no tuples (false) from one empty tuple (true). */
  {int rows;
    for(rows=0;rows<=1;++rows){
      OK(gecode_opt_v1_model_create(&m));
      OK(gecode_opt_v1_model_add_table(m,NULL,0,NULL,0,(uint64_t)rows,"zero arity",&g));
      OK(gecode_opt_v1_solve(m,NULL,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
      CHECK(info.termination==(native?(rows?GECODE_OPT_OPTIMAL:GECODE_OPT_INFEASIBLE):GECODE_OPT_UNSUPPORTED));
      OK(gecode_opt_v1_result_destroy(r));OK(gecode_opt_v1_model_destroy(m));
    }
  }
  /* A bounded indicator keeps its domain guard and exposes its gate lifetime. */
  {gecode_opt_id b,indicator,gate,a,o;gecode_opt_term term;int32_t has_gate=0;
    OK(gecode_opt_v1_model_create(&m));
    OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_BINARY,1,1,"b",&b));
    OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_INTEGER,0,4,"x",&x));
    term.variable=x;term.coefficient=1;
    OK(gecode_opt_v1_model_identity(m,&owner,&before));
    CHECK(gecode_opt_v1_model_add_indicator(m,b,2,&term,1,3,INFINITY,"bad",&indicator,&has_gate,&gate)==GECODE_OPT_INVALID_ARGUMENT);
    CHECK(gecode_opt_v1_model_add_indicator(m,b,1,&term,1,3,INFINITY,"bad",&indicator,NULL,&gate)==GECODE_OPT_INVALID_ARGUMENT);
    CHECK(gecode_opt_v1_model_add_boolean_and(m,b,&x,1,"integer")==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_model_identity(m,&owner,&after));CHECK(before==after);
    OK(gecode_opt_v1_model_add_indicator(m,b,1,&term,1,3,INFINITY,"guard",&indicator,&has_gate,&gate));
    CHECK(indicator.kind==GECODE_OPT_INDICATOR_ID&&has_gate&&gate.kind==GECODE_OPT_VARIABLE_ID);
    CHECK(gecode_opt_v1_model_set_variable_bounds(m,x,0,5)==GECODE_OPT_MODEL_ERROR);
    CHECK(gecode_opt_v1_model_remove_indicator(m,x)==GECODE_OPT_INVALID_ARGUMENT);
    OK(gecode_opt_v1_model_set_objective(m,&term,1,GECODE_OPT_MINIMIZE,0));options.backend=GECODE_OPT_HIGHS;
    OK(gecode_opt_v1_solve(m,&options,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
    CHECK(info.termination==(highs?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
    if(highs){OK(gecode_opt_v1_result_value(r,x,&value));CHECK(near(value,3));}
    OK(gecode_opt_v1_result_destroy(r));OK(gecode_opt_v1_model_remove_indicator(m,indicator));
    CHECK(gecode_opt_v1_model_remove_indicator(m,indicator)==GECODE_OPT_MODEL_ERROR);
    OK(gecode_opt_v1_model_remove_variable(m,gate));
    OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_BINARY,0,1,"and",&a));
    OK(gecode_opt_v1_model_add_variable(m,GECODE_OPT_BINARY,0,1,"or",&o));
    OK(gecode_opt_v1_model_add_boolean_and(m,a,NULL,0,"empty and"));
    OK(gecode_opt_v1_model_add_boolean_or(m,o,NULL,0,"empty or"));
    ids[0]=a;ids[1]=a;OK(gecode_opt_v1_model_add_boolean_and(m,a,ids,2,"alias"));
    OK(gecode_opt_v1_solve(m,&options,&r));OK(gecode_opt_v1_result_info(r,&info,sizeof(info)));
    CHECK(info.termination==(highs?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
    if(highs){OK(gecode_opt_v1_result_value(r,x,&value));CHECK(near(value,0));
      OK(gecode_opt_v1_result_value(r,a,&value));CHECK(near(value,1));OK(gecode_opt_v1_result_value(r,o,&value));CHECK(near(value,0));}
    OK(gecode_opt_v1_result_destroy(r));OK(gecode_opt_v1_model_destroy(m));
  }
  return 0;
}

static int native_starts(void) {
  gecode_opt_handle model=0,result=0,old=0;
  gecode_opt_id b,x,gate,indicator;
  gecode_opt_term terms[2];gecode_opt_start start[3];
  gecode_opt_options_v1 options;gecode_opt_result_info_v1 info;
  int32_t native=0,lp=0,mip=0,has_gate=0,present=0;int maximize;
  double value=0;
  OK(gecode_opt_v1_capabilities(GECODE_OPT_NATIVE,&native,&lp,&mip));
  OK(gecode_opt_v1_model_create(&model));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_BINARY,0,1,"b",&b));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,4,"x",&x));
  terms[0].variable=x;terms[0].coefficient=1;
  OK(gecode_opt_v1_model_add_indicator(model,b,1,terms,1,3,INFINITY,"enabled demand",&indicator,&has_gate,&gate));
  CHECK(has_gate);terms[1].variable=b;terms[1].coefficient=-10;
  OK(gecode_opt_v1_options_default(&options,sizeof(options)));
  options.backend=GECODE_OPT_NATIVE;options.guarantee=GECODE_OPT_EXACT;
  options.primal_start=start;options.primal_start_count=2;
  start[0].variable=b;start[1].variable=x;start[2].variable=gate;
  for(maximize=0;maximize!=2;++maximize) {
    /* The deliberately poor original point omits its live derived gate. */
    start[0].value=maximize?1:0;start[1].value=maximize?3:4;
    OK(gecode_opt_v1_model_set_objective(model,terms,2,maximize?GECODE_OPT_MAXIMIZE:GECODE_OPT_MINIMIZE,3));
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==(native?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
    CHECK(info.start_submitted==native&&info.has_solution==native);
    if(native) {
      CHECK(info.solution_validated&&info.guarantee==GECODE_OPT_EXACT);
      OK(gecode_opt_v1_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&value==(maximize?7:-4));
      OK(gecode_opt_v1_result_number(result,GECODE_OPT_BEST_BOUND,&present,&value));CHECK(present&&value==(maximize?7:-4));
      OK(gecode_opt_v1_result_value(result,b,&value));CHECK(value==(maximize?0:1));
      OK(gecode_opt_v1_result_value(result,x,&value));CHECK(value==(maximize?4:3));
      OK(gecode_opt_v1_result_value(result,gate,&value));CHECK(value==(maximize?1:0));
    }
    if(!maximize)old=result;else OK(gecode_opt_v1_result_destroy(result));
  }
  if(native) {
    start[0].value=0;start[1].value=4;start[2].value=0;options.primal_start_count=3;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_INVALID_MODEL&&!info.start_submitted&&!info.has_solution);OK(gecode_opt_v1_result_destroy(result));
    options.primal_start_count=1;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_UNSUPPORTED&&!info.start_submitted);OK(gecode_opt_v1_result_destroy(result));
    options.primal_start_count=2;start[0].value=1-1e-12;start[1].value=3;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_INVALID_MODEL&&!info.has_solution);OK(gecode_opt_v1_result_destroy(result));
    start[0].value=0;start[1].value=4;options.has_node_limit=1;options.node_limit=0;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_NODE_LIMIT&&!info.start_submitted&&!info.has_solution);OK(gecode_opt_v1_result_destroy(result));
    options.has_node_limit=0;
    OK(gecode_opt_v1_model_remove_indicator(model,indicator));
    OK(gecode_opt_v1_model_set_objective(model,NULL,0,GECODE_OPT_MINIMIZE,11));
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_result_destroy(result));
    /* Removed gate is ordinary state: accept explicit zero even though b=0. */
    options.primal_start_count=3;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==GECODE_OPT_OPTIMAL&&info.start_submitted);
    start[1].value=99;start[2].value=99;
    OK(gecode_opt_v1_result_value(result,x,&value));CHECK(value==4);
    OK(gecode_opt_v1_result_value(result,gate,&value));CHECK(value==0);
    OK(gecode_opt_v1_result_destroy(result));
  }
  OK(gecode_opt_v1_model_destroy(model));
  if(native){OK(gecode_opt_v1_result_value(old,x,&value));CHECK(value==3);
    OK(gecode_opt_v1_result_number(old,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&value==-4);}
  OK(gecode_opt_v1_result_destroy(old));return 0;
}

static int quadratic(void) {
  gecode_opt_handle m=0,other_model=0,linear=0,result=0,old=0,cancel=0,session=0;
  gecode_opt_id x,dead,other,row;
  gecode_opt_term term,linear_term;
  gecode_opt_weighted_square_v1 square;
  gecode_opt_quadratic_options_v1 options;
  gecode_opt_quadratic_info_v1 info;
  gecode_opt_quadratic_checks_v1 checks;
  gecode_opt_result_info_v1 linear_info;
  uint64_t owner=0,revision=0,after=0,needed=0;
  int32_t available=0,present=0;
  double value=0,values[2]={44,55};uint8_t active[2]={7,7},valid[2]={7,7};
  memset(&square,0,sizeof(square));square.struct_size=sizeof(square);square.weight=2;square.offset=-2;
  OK(gecode_opt_v1_quadratic_capabilities(&available));
  CHECK(gecode_opt_v1_quadratic_capabilities(NULL)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_options_default(&options,sizeof(options)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_quadratic_options_default(&options,sizeof(options)));
  CHECK(options.struct_size==sizeof(options)&&options.solve.struct_size==sizeof(options.solve));
  CHECK(options.reserved==0&&options.solve.reserved==0);
  CHECK(options.iteration_limit==100000&&near(options.optimality_tolerance,1e-6));
  CHECK(gecode_opt_v1_quadratic_model_create(NULL)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_quadratic_model_create(&m));OK(gecode_opt_v1_quadratic_model_create(&other_model));
  OK(gecode_opt_v1_model_create(&linear));OK(gecode_opt_v1_session_create(&session));
  OK(gecode_opt_v1_quadratic_model_add_continuous(m,0,1,"hole",&dead));
  OK(gecode_opt_v1_quadratic_model_remove_variable(m,dead));
  OK(gecode_opt_v1_quadratic_model_add_continuous(m,-4,4,"x",&x));
  OK(gecode_opt_v1_quadratic_model_add_continuous(other_model,-4,4,"foreign",&other));
  CHECK(gecode_opt_v1_quadratic_model_add_continuous(m,0,INFINITY,"",&row)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_quadratic_model_add_continuous(m,NAN,1,"",&row)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_destroy(m)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_quadratic_model_destroy(linear)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_solve(m,NULL,&result)==GECODE_OPT_INVALID_HANDLE&&result==0);
  CHECK(gecode_opt_v1_session_solve(session,m,NULL,&result)==GECODE_OPT_INVALID_HANDLE&&result==0);
  CHECK(gecode_opt_v1_pool_solve(m,NULL,&result)==GECODE_OPT_INVALID_HANDLE&&result==0);
  CHECK(gecode_opt_v1_repair_solve(m,NULL,&result)==GECODE_OPT_INVALID_HANDLE&&result==0);
  CHECK(gecode_opt_v1_quadratic_solve(linear,NULL,&result)==GECODE_OPT_INVALID_HANDLE&&result==0);
  CHECK(gecode_opt_v1_model_set_variable_bounds(m,x,0,1)==GECODE_OPT_INVALID_HANDLE);
  term.variable=x;term.coefficient=1;square.terms=&term;square.term_count=1;
  linear_term.variable=x;linear_term.coefficient=4;
  /* 2(x-2)^2 + 4x - 7 has minimum -1 at x=1. */
  OK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,&linear_term,1,GECODE_OPT_MINIMIZE,-7));
  OK(gecode_opt_v1_quadratic_model_identity(m,&owner,&revision));
  square.struct_size--;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);square.struct_size++;
  square.reserved=1;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);square.reserved=0;
  square.terms=NULL;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);square.terms=&term;
  CHECK(gecode_opt_v1_quadratic_model_set_objective(m,NULL,1,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,UINT64_MAX,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,1,0,0)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,99,0)==GECODE_OPT_INVALID_ARGUMENT);
  square.weight=0;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_MODEL_ERROR);square.weight=2;
  square.offset=INFINITY;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_MODEL_ERROR);square.offset=-2;
  square.name="\xe2";CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_INVALID_ARGUMENT);square.name=NULL;
  term.variable=other;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_MODEL_ERROR);
  term.variable=dead;CHECK(gecode_opt_v1_quadratic_model_set_objective(m,&square,1,NULL,0,0,0)==GECODE_OPT_MODEL_ERROR);term.variable=x;
  CHECK(gecode_opt_v1_quadratic_model_remove_variable(m,x)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_quadratic_model_identity(m,&owner,&after));CHECK(after==revision);
  OK(gecode_opt_v1_quadratic_solve(m,NULL,&result));
  OK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)));
  CHECK(info.result.model_id==owner&&info.result.revision==revision&&info.result.variable_slots==2);
  CHECK(info.result.reserved==0);
  CHECK(info.result.termination==(available?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  CHECK(gecode_opt_v1_result_info(result,&linear_info,sizeof(linear_info))==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_result_destroy(result)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)-1)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_result_checks(result,&checks,sizeof(checks)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_quadratic_result_checks(result,&checks,sizeof(checks)));
  CHECK(checks.reserved==0&&checks.max_stationarity.reserved==0&&checks.original_objective.reserved==0);
  OK(gecode_opt_v1_quadratic_result_values(result,NULL,NULL,NULL,0,&needed));CHECK(needed==2);
  CHECK(gecode_opt_v1_quadratic_result_values(result,values,active,valid,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
  CHECK(values[0]==44&&active[0]==7&&valid[0]==7);
  CHECK(gecode_opt_v1_quadratic_result_values(result,values,NULL,valid,2,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_quadratic_result_values(result,values,active,valid,2,&needed));
  CHECK(!active[0]&&!valid[0]&&values[0]==0&&active[1]);
  OK(gecode_opt_v1_quadratic_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));
  CHECK(present==available);
  if(available){
    CHECK(near(value,-1)&&near(values[1],1)&&valid[1]);
    CHECK(checks.primal_valid&&checks.objective_valid&&checks.kkt_available&&checks.kkt_valid&&checks.bound_valid);
    CHECK(checks.original_objective.present&&near(checks.original_objective.value,-1));
    CHECK(checks.gap_upper_bound.present&&checks.gap_upper_bound.value<=options.optimality_tolerance);
    CHECK(checks.max_stationarity.present&&checks.max_complementarity.present);
    CHECK(checks.square_count==1&&checks.gradient_slots==2);
    OK(gecode_opt_v1_quadratic_result_array(result,GECODE_OPT_QP_SQUARE_RESIDUALS,values,2,&needed));CHECK(needed==1&&near(values[0],-1));
    values[0]=44;CHECK(gecode_opt_v1_quadratic_result_array(result,GECODE_OPT_QP_ORIGINAL_GRADIENT,values,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);CHECK(values[0]==44);
    OK(gecode_opt_v1_quadratic_result_array(result,GECODE_OPT_QP_ORIGINAL_GRADIENT,values,2,&needed));CHECK(near(values[0],0)&&near(values[1],0));
    CHECK(gecode_opt_v1_quadratic_result_value(result,other,&value)==GECODE_OPT_MODEL_ERROR);
    CHECK(gecode_opt_v1_quadratic_result_value(result,dead,&value)==GECODE_OPT_MODEL_ERROR);
  }else{
    CHECK(!checks.primal_valid&&!checks.objective_valid&&!checks.kkt_available&&!checks.kkt_valid&&!checks.bound_valid);
    CHECK(!checks.original_objective.present&&!checks.normalized_lower_bound.present&&!checks.gap_upper_bound.present);
    CHECK(!checks.max_stationarity.present&&!checks.max_complementarity.present&&checks.gradient_slots==0);
    CHECK(gecode_opt_v1_quadratic_result_value(result,x,&value)==GECODE_OPT_NO_SOLUTION);
  }
  CHECK(gecode_opt_v1_quadratic_result_array(result,99,NULL,0,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_quadratic_result_number(result,99,&present,&value)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_quadratic_result_number(result,GECODE_OPT_QP_VENDOR_OBJECTIVE,&present,&value));
  CHECK(present==available);
  {char buffer[256];buffer[0]='Z';
    OK(gecode_opt_v1_quadratic_result_text(result,GECODE_OPT_BACKEND_NAME,NULL,0,&needed));CHECK(needed>1);
    CHECK(gecode_opt_v1_quadratic_result_text(result,GECODE_OPT_BACKEND_NAME,buffer,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);CHECK(buffer[0]=='Z');
    OK(gecode_opt_v1_quadratic_result_text(result,GECODE_OPT_QP_CHECK_MESSAGE,buffer,sizeof(buffer),&needed));}
  old=result;result=0;
  OK(gecode_opt_v1_quadratic_model_add_row(m,&term,1,2,INFINITY,"demand",&row));
  OK(gecode_opt_v1_quadratic_model_set_coefficient(m,row,x,2));
  OK(gecode_opt_v1_quadratic_model_set_row_bounds(m,row,4,INFINITY));
  OK(gecode_opt_v1_quadratic_solve(m,&options,&result));
  if(available){OK(gecode_opt_v1_quadratic_result_value(result,x,&value));CHECK(near(value,2));}
  OK(gecode_opt_v1_quadratic_result_destroy(result));
  OK(gecode_opt_v1_quadratic_model_remove_row(m,row));
  options.struct_size--;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_INVALID_ARGUMENT&&result==0);options.struct_size++;
  options.solve.struct_size--;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);options.solve.struct_size++;
  options.reserved=1;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);options.reserved=0;
  options.solve.reserved=1;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);options.solve.reserved=0;
  options.optimality_tolerance=NAN;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_MODEL_ERROR);options.optimality_tolerance=1e-6;
  options.max_lifted_nonzeros=UINT64_MAX;CHECK(gecode_opt_v1_quadratic_solve(m,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);options.max_lifted_nonzeros=2000000;
  options.solve.backend=GECODE_OPT_NATIVE;
  OK(gecode_opt_v1_quadratic_solve(m,&options,&result));OK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)));
  CHECK(info.result.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_quadratic_result_destroy(result));options.solve.backend=GECODE_OPT_AUTO;
  options.solve.guarantee=GECODE_OPT_EXACT;
  OK(gecode_opt_v1_quadratic_solve(m,&options,&result));OK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)));
  CHECK(info.result.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_quadratic_result_destroy(result));options.solve.guarantee=GECODE_OPT_NUMERICAL;
  OK(gecode_opt_v1_cancellation_create(&cancel));OK(gecode_opt_v1_cancellation_cancel(cancel));options.solve.cancellation=cancel;
  OK(gecode_opt_v1_quadratic_solve(m,&options,&result));OK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)));
  CHECK(info.result.termination==GECODE_OPT_CANCELLED);
  OK(gecode_opt_v1_quadratic_result_checks(result,&checks,sizeof(checks)));
  CHECK(!checks.kkt_available&&!checks.max_stationarity.present&&!checks.max_complementarity.present);
  OK(gecode_opt_v1_quadratic_result_destroy(result));
  OK(gecode_opt_v1_cancellation_destroy(cancel));options.solve.cancellation=0;
  options.solve.time_limit_seconds=0;
  OK(gecode_opt_v1_quadratic_solve(m,&options,&result));OK(gecode_opt_v1_quadratic_result_info(result,&info,sizeof(info)));
  CHECK(info.result.termination==GECODE_OPT_TIME_LIMIT);OK(gecode_opt_v1_quadratic_result_destroy(result));
  OK(gecode_opt_v1_quadratic_model_destroy(m));OK(gecode_opt_v1_quadratic_model_destroy(other_model));
  OK(gecode_opt_v1_model_destroy(linear));OK(gecode_opt_v1_session_destroy(session));
  OK(gecode_opt_v1_quadratic_result_info(old,&info,sizeof(info)));CHECK(info.result.revision==revision);
  if(available){OK(gecode_opt_v1_quadratic_result_value(old,x,&value));CHECK(near(value,1));}
  OK(gecode_opt_v1_quadratic_result_destroy(old));
  CHECK(gecode_opt_v1_quadratic_result_destroy(old)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_quadratic_model_identity(m,&owner,&revision)==GECODE_OPT_INVALID_HANDLE);
  return 0;
}

int main(void){
  gecode_opt_options_v1 options;
  gecode_opt_handle model=0,foreign=0,session=0,result=0,old=0,cancel=0;
  gecode_opt_id x,dead,other,row;
  gecode_opt_term term;
  gecode_opt_result_info_v1 info;
  int32_t available=0,lp=0,mip=0,present=0;
  uint64_t owner=0,revision=0,needed=0;
  double value=0;
  CHECK(gecode_opt_v1_abi_version()==1);
  CHECK(globals_and_logic()==0);
  CHECK(workflows()==0);CHECK(bulk()==0);CHECK(quadratic()==0);
  CHECK(gecode_opt_v1_options_default(NULL,sizeof(options))==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_options_default(&options,sizeof(options)-1)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_options_default(&options,sizeof(options)));
  CHECK(options.struct_size==sizeof(options)&&options.threads==1);
  CHECK(gecode_opt_v1_capabilities(-1,&available,&lp,&mip)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_capabilities(GECODE_OPT_HIGHS,&available,&lp,&mip));
  CHECK(gecode_opt_v1_model_create(NULL)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_model_create(&model));OK(gecode_opt_v1_model_create(&foreign));
  CHECK(model!=foreign&&model!=0);
  CHECK(gecode_opt_v1_model_add_variable(model,-1,0,1,"bad",&x)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,NAN,1,"bad",&x)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_INTEGER,0,6,"x",&x));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_BINARY,0,1,"dead",&dead));
  OK(gecode_opt_v1_model_remove_variable(model,dead));
  CHECK(gecode_opt_v1_model_set_variable_bounds(model,dead,0,1)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_add_variable(foreign,GECODE_OPT_INTEGER,0,6,"other",&other));
  term.variable=other;term.coefficient=1;
  CHECK(gecode_opt_v1_model_add_row(model,&term,1,2,INFINITY,"foreign",&row)==GECODE_OPT_MODEL_ERROR);
  CHECK(gecode_opt_v1_model_add_row(model,NULL,1,2,INFINITY,"null",&row)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_add_row(model,&term,UINT64_MAX,2,INFINITY,"count",&row)==GECODE_OPT_INVALID_ARGUMENT);
  term.variable=x;term.coefficient=INFINITY;
  CHECK(gecode_opt_v1_model_add_row(model,&term,1,2,INFINITY,"inf",&row)==GECODE_OPT_INVALID_ARGUMENT);
  term.coefficient=1;
  OK(gecode_opt_v1_model_add_row(model,&term,1,2,INFINITY,"demand",&row));
  CHECK(row.kind==GECODE_OPT_ROW_ID&&x.kind==GECODE_OPT_VARIABLE_ID);
  CHECK(gecode_opt_v1_model_set_variable_bounds(model,row,0,1)==GECODE_OPT_INVALID_ARGUMENT);
  term.coefficient=2;
  OK(gecode_opt_v1_model_set_objective(model,&term,1,GECODE_OPT_MINIMIZE,-3));
  CHECK(gecode_opt_v1_model_set_objective(model,&term,1,-1,0)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_model_remove_variable(model,x)==GECODE_OPT_MODEL_ERROR);
  OK(gecode_opt_v1_model_identity(model,&owner,&revision));CHECK(owner==x.model_id&&revision>0);
  OK(gecode_opt_v1_session_create(&session));
  CHECK(gecode_opt_v1_model_destroy(session)==GECODE_OPT_INVALID_HANDLE);
  options.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_session_solve(session,model,&options,&result));
  OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
  CHECK(info.termination==(available?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  CHECK(info.has_solution==available);
  OK(gecode_opt_v1_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));
  CHECK(present==available);CHECK(available?near(value,1):value==0);
  CHECK(gecode_opt_v1_result_info(result,&info,sizeof(info)-1)==GECODE_OPT_INVALID_ARGUMENT);
  CHECK(gecode_opt_v1_result_number(result,-1,&present,&value)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_result_text(result,GECODE_OPT_BACKEND_NAME,NULL,0,&needed));CHECK(needed>1);
  {char buffer[128];buffer[0]='Z';
    CHECK(gecode_opt_v1_result_text(result,GECODE_OPT_BACKEND_NAME,buffer,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
    CHECK(buffer[0]=='Z'&&gecode_opt_v1_last_error()[0]);
    OK(gecode_opt_v1_result_text(result,GECODE_OPT_BACKEND_NAME,buffer,sizeof(buffer),&needed));CHECK(strcmp(buffer,"HiGHS")==0);
    CHECK(gecode_opt_v1_last_error()[0]=='\0');}
  if(available){
    double values[2]={88,99};uint8_t active[2]={7,7},valid[2]={7,7};
    OK(gecode_opt_v1_result_values(result,NULL,NULL,NULL,0,&needed));CHECK(needed==2);
    CHECK(gecode_opt_v1_result_values(result,values,active,valid,1,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
    CHECK(values[0]==88&&active[0]==7&&valid[0]==7);
    OK(gecode_opt_v1_result_values(result,values,active,valid,2,&needed));
    CHECK(active[0]&&valid[0]&&near(values[0],2));CHECK(!active[1]&&!valid[1]&&values[1]==0);
    CHECK(gecode_opt_v1_result_value(result,other,&value)==GECODE_OPT_MODEL_ERROR);
    CHECK(gecode_opt_v1_result_value(result,dead,&value)==GECODE_OPT_MODEL_ERROR);
    old=result;result=0;
    OK(gecode_opt_v1_model_set_variable_bounds(model,x,4,6));
    OK(gecode_opt_v1_session_solve(session,model,&options,&result));
    OK(gecode_opt_v1_result_number(result,GECODE_OPT_OBJECTIVE,&present,&value));CHECK(present&&near(value,5));
    OK(gecode_opt_v1_result_value(old,x,&value));CHECK(near(value,2));
    {gecode_opt_session_statistics_v1 stats;
      OK(gecode_opt_v1_session_statistics(session,&stats,sizeof(stats)));CHECK(stats.model_loads==1&&stats.incremental_updates==1);}
    OK(gecode_opt_v1_result_destroy(old));
  }else CHECK(gecode_opt_v1_result_value(result,x,&value)==GECODE_OPT_NO_SOLUTION);
  OK(gecode_opt_v1_result_destroy(result));
  options.reserved=1;
  CHECK(gecode_opt_v1_solve(model,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);CHECK(result==0);
  options.reserved=0;options.backend=99;
  CHECK(gecode_opt_v1_solve(model,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);
  options.backend=GECODE_OPT_HIGHS;options.primal_start_count=1;options.primal_start=NULL;
  CHECK(gecode_opt_v1_solve(model,&options,&result)==GECODE_OPT_INVALID_ARGUMENT);
  options.primal_start_count=0;options.time_limit_seconds=NAN;
  CHECK(gecode_opt_v1_solve(model,&options,&result)==GECODE_OPT_MODEL_ERROR);
  options.time_limit_seconds=INFINITY;options.guarantee=GECODE_OPT_CERTIFIED;
  OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
  CHECK(info.termination==GECODE_OPT_UNSUPPORTED);OK(gecode_opt_v1_result_destroy(result));options.guarantee=GECODE_OPT_NUMERICAL;
  OK(gecode_opt_v1_cancellation_create(&cancel));OK(gecode_opt_v1_cancellation_cancel(cancel));options.cancellation=cancel;
  OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
  CHECK(info.termination==GECODE_OPT_CANCELLED);OK(gecode_opt_v1_result_destroy(result));
  OK(gecode_opt_v1_cancellation_destroy(cancel));options.cancellation=0;
  {int32_t native=0;OK(gecode_opt_v1_capabilities(GECODE_OPT_NATIVE,&native,&lp,&mip));options.backend=GECODE_OPT_NATIVE;
    OK(gecode_opt_v1_solve(model,&options,&result));OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));
    CHECK(info.termination==(native?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));OK(gecode_opt_v1_result_destroy(result));}
  options.backend=GECODE_OPT_HIGHS;
  OK(gecode_opt_v1_solve(model,&options,&result));
  OK(gecode_opt_v1_session_destroy(session));OK(gecode_opt_v1_model_destroy(model));OK(gecode_opt_v1_model_destroy(foreign));
  CHECK(gecode_opt_v1_model_destroy(model)==GECODE_OPT_INVALID_HANDLE);
  CHECK(gecode_opt_v1_model_identity(model,&owner,&revision)==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_result_info(result,&info,sizeof(info)));CHECK(info.model_id==x.model_id);
  if(available){OK(gecode_opt_v1_result_value(result,x,&value));CHECK(near(value,4));}
  OK(gecode_opt_v1_result_destroy(result));CHECK(gecode_opt_v1_result_destroy(result)==GECODE_OPT_INVALID_HANDLE);
  CHECK(native_starts()==0);
  puts("C99 ABI ownership, error, session and backend contracts passed");return 0;
}
