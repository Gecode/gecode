/* Compile this consumer as C99, then link the versioned shared C ABI. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/c_api.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define OK(call) do { int32_t code=(call); if(code)fprintf(stderr,"%s: %s\n",#call,gecode_opt_v1_last_error()); assert(code==GECODE_OPT_OK); } while(0)
int main(void) {
  gecode_opt_handle model=0,basis=0,session=0,solved=0,observed=0,ordinary=0,copy=0,exported=0;
  gecode_opt_id x,y,dead,row,deleted,foreign;
  gecode_opt_term terms[2];int32_t columns[3]={1,0,-1},rows[2]={-1,0},status=-9;
  gecode_opt_basis_info_v1 info;gecode_opt_basis_result_info_v1 result;
  gecode_opt_lp_capabilities_v1 caps;gecode_opt_lp_options_v1 options;
  uint64_t needed=0;int32_t small[2]={91,92};
  OK(gecode_opt_v1_lp_capabilities(&caps,sizeof(caps)));
  OK(gecode_opt_v1_model_create(&model));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,INFINITY,"x",&x));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,INFINITY,"y",&y));
  OK(gecode_opt_v1_model_add_variable(model,GECODE_OPT_CONTINUOUS,0,1,"deleted",&dead));
  OK(gecode_opt_v1_model_remove_variable(model,dead));
  OK(gecode_opt_v1_model_add_row(model,NULL,0,-1,1,"deleted",&deleted));
  OK(gecode_opt_v1_model_remove_row(model,deleted));
  terms[0].variable=x;terms[0].coefficient=1;terms[1].variable=y;terms[1].coefficient=1;
  OK(gecode_opt_v1_model_add_row(model,terms,2,4,INFINITY,"demand",&row));
  terms[0].coefficient=2;terms[1].coefficient=3;
  OK(gecode_opt_v1_model_set_objective(model,terms,2,0,7));
  assert(gecode_opt_v1_basis_from_model(model,columns,3,rows,2,NULL)==GECODE_OPT_INVALID_ARGUMENT);
  basis=99;assert(gecode_opt_v1_basis_from_model(model,NULL,3,rows,2,&basis)==GECODE_OPT_INVALID_ARGUMENT&&basis==0);
  assert(gecode_opt_v1_basis_from_model(model,columns,UINT64_MAX,rows,2,&basis)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_basis_from_model(model,columns,2,rows,2,&basis)==GECODE_OPT_MODEL_ERROR);
  columns[0]=-2;assert(gecode_opt_v1_basis_from_model(model,columns,3,rows,2,&basis)==GECODE_OPT_INVALID_ARGUMENT);
  columns[0]=1;columns[2]=0;assert(gecode_opt_v1_basis_from_model(model,columns,3,rows,2,&basis)==GECODE_OPT_MODEL_ERROR);
  columns[2]=-1;OK(gecode_opt_v1_basis_from_model(model,columns,3,rows,2,&basis));
  OK(gecode_opt_v1_basis_info(basis,&info,sizeof(info)));
  assert(info.struct_size==sizeof(info)&&info.reserved==0&&info.reserved_flags==0&&info.origin==GECODE_OPT_BASIS_CALLER);
  assert(info.column_slots==3&&info.row_slots==2&&info.model_id==x.model_id);
  assert(gecode_opt_v1_basis_info(basis,&info,sizeof(info)-1)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_basis_info(model,&info,sizeof(info))==GECODE_OPT_INVALID_HANDLE);
  assert(gecode_opt_v1_basis_statuses(basis,99,NULL,0,&needed)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_basis_statuses(basis,GECODE_OPT_VARIABLE_ID,NULL,0,&needed));assert(needed==3);
  assert(gecode_opt_v1_basis_statuses(basis,GECODE_OPT_VARIABLE_ID,small,2,&needed)==GECODE_OPT_BUFFER_TOO_SMALL);
  assert(small[0]==91&&small[1]==92&&needed==3);
  OK(gecode_opt_v1_basis_column(basis,x,&status));assert(status==GECODE_OPT_LP_BASIS_BASIC);
  OK(gecode_opt_v1_basis_row(basis,row,&status));assert(status==GECODE_OPT_LP_BASIS_LOWER);
  assert(gecode_opt_v1_basis_column(basis,dead,&status)==GECODE_OPT_MODEL_ERROR);
  assert(gecode_opt_v1_basis_row(basis,deleted,&status)==GECODE_OPT_MODEL_ERROR);
  foreign=x;foreign.model_id++;assert(gecode_opt_v1_basis_column(basis,foreign,&status)==GECODE_OPT_MODEL_ERROR);
  foreign=x;foreign.reserved=1;assert(gecode_opt_v1_basis_column(basis,foreign,&status)==GECODE_OPT_INVALID_ARGUMENT);
  assert(gecode_opt_v1_basis_row(basis,x,&status)==GECODE_OPT_INVALID_ARGUMENT);
  OK(gecode_opt_v1_session_create(&session));OK(gecode_opt_v1_lp_options_default(&options,sizeof(options)));
  options.reserved=1;assert(gecode_opt_v1_solve_lp_with_basis(model,basis,&options,&solved)==GECODE_OPT_INVALID_ARGUMENT&&solved==0);options.reserved=0;
  options.solve.reserved=1;assert(gecode_opt_v1_solve_lp_with_basis(model,basis,&options,&solved)==GECODE_OPT_INVALID_ARGUMENT);options.solve.reserved=0;
  assert(gecode_opt_v1_solve_lp_with_basis(model,model,&options,&solved)==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_session_solve_lp_with_basis(session,model,basis,&options,&solved));
  OK(gecode_opt_v1_basis_result_info(solved,&result,sizeof(result)));
  assert(result.struct_size==sizeof(result)&&result.reserved==0&&result.reserved_flags==0&&result.result.reserved==0);
  assert(result.has_requested_basis&&result.requested_model_id==x.model_id);
  assert(result.result.termination==(caps.available?GECODE_OPT_OPTIMAL:GECODE_OPT_UNSUPPORTED));
  assert(result.state==(caps.available?GECODE_OPT_BASIS_ACCEPTED:GECODE_OPT_BASIS_NOT_ATTEMPTED));
  assert(result.backend_attempted==caps.available&&result.has_statuses_changed==caps.available&&!result.statuses_changed);
  assert(gecode_opt_v1_basis_result_info(basis,&result,sizeof(result))==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_basis_result_message(solved,NULL,0,&needed));assert(needed>0);
  {char text[4096];assert(needed<=sizeof(text));OK(gecode_opt_v1_basis_result_message(solved,text,sizeof(text),&needed));assert(text[needed-1]=='\0');}
  OK(gecode_opt_v1_basis_result_copy_observed(solved,&observed));
  OK(gecode_opt_v1_basis_result_copy_basis(solved,&copy));
  OK(gecode_opt_v1_lp_observed_result_copy_result(observed,&ordinary));
  if(caps.available) {OK(gecode_opt_v1_basis_from_observed(observed,&exported));OK(gecode_opt_v1_basis_info(exported,&info,sizeof(info)));assert(info.origin==GECODE_OPT_BASIS_OBSERVATIONS);}
  else assert(gecode_opt_v1_basis_from_observed(observed,&exported)==GECODE_OPT_MODEL_ERROR&&exported==0);
  OK(gecode_opt_v1_basis_result_destroy(solved));OK(gecode_opt_v1_basis_destroy(basis));
  assert(gecode_opt_v1_basis_info(basis,&info,sizeof(info))==GECODE_OPT_INVALID_HANDLE);
  OK(gecode_opt_v1_lp_observed_result_destroy(observed));
  OK(gecode_opt_v1_model_set_row_bounds(model,row,5,INFINITY));
  OK(gecode_opt_v1_session_solve_lp_with_basis(session,model,copy,NULL,&solved));
  OK(gecode_opt_v1_basis_result_info(solved,&result,sizeof(result)));assert(result.result.termination==GECODE_OPT_INVALID_MODEL&&!result.backend_attempted);
  OK(gecode_opt_v1_model_destroy(model));OK(gecode_opt_v1_session_destroy(session));
  OK(gecode_opt_v1_basis_column(copy,x,&status));assert(status==1);
  if(caps.available) {int32_t present=0;double value=0;OK(gecode_opt_v1_result_number(ordinary,GECODE_OPT_OBJECTIVE,&present,&value));assert(present&&fabs(value-15)<1e-7);OK(gecode_opt_v1_result_value(ordinary,x,&value));assert(value==4);OK(gecode_opt_v1_basis_destroy(exported));}
  OK(gecode_opt_v1_result_destroy(ordinary));OK(gecode_opt_v1_basis_result_destroy(solved));OK(gecode_opt_v1_basis_destroy(copy));
  puts("C99 owning basis, submission, buffers, source identity and lifetime conformance passed");return 0;
}
