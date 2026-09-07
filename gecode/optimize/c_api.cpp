#include <gecode/optimize/c_api.h>
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/globals.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/pool.hpp>
#include <gecode/optimize/relaxation.hpp>
#include <gecode/optimize/quadratic.hpp>
#include <gecode/optimize/lp_observations.hpp>
#include <gecode/optimize/scenarios.hpp>
#include <gecode/optimize/lp_evidence.hpp>
#include <gecode/optimize/lp_sensitivity.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace O=Gecode::Optimize;
namespace {
thread_local char last_error[4096] = {};
struct ApiError { int32_t code; const char* message; };
[[noreturn]] void argument(const char* message){throw ApiError{GECODE_OPT_INVALID_ARGUMENT,message};}
void required(const void* pointer){if(!pointer)argument("required pointer is NULL");}
template<class F> int32_t boundary(F&& call) noexcept {
  last_error[0]='\0';
  try{call();return GECODE_OPT_OK;}
  catch(const ApiError& e){std::snprintf(last_error,sizeof(last_error),"%s",e.message);return e.code;}
  catch(const O::ModelError& e){std::snprintf(last_error,sizeof(last_error),"%s",e.what());return GECODE_OPT_MODEL_ERROR;}
  catch(const std::bad_alloc&){std::snprintf(last_error,sizeof(last_error),"allocation failed");return GECODE_OPT_OUT_OF_MEMORY;}
  catch(const std::exception& e){std::snprintf(last_error,sizeof(last_error),"C++ backend exception: %s",e.what());return GECODE_OPT_INTERNAL_ERROR;}
  catch(...){std::snprintf(last_error,sizeof(last_error),"unknown C++ backend exception");return GECODE_OPT_INTERNAL_ERROR;}
}
enum class Kind { Model, Session, Result, Cancellation, Pool, Repair, QuadraticModel, QuadraticResult, LpObservedResult, Basis, BasisSolveResult, ScenarioBatchResult, LpEvidenceResult, LpEvidenceStage, SensitivityResult };
struct Entry {Kind kind;std::shared_ptr<void> value;};
struct Registry {std::mutex mutex;uint64_t next=1;std::unordered_map<uint64_t,Entry> entries;};
Registry& registry(){static Registry value;return value;}
struct ModelBox {std::mutex mutex;O::Model model;ModelBox()=default;explicit ModelBox(O::Model&& value):model(std::move(value)){};};
struct QuadraticBox {std::mutex mutex;O::QuadraticModel model;};
struct SessionBox {std::mutex mutex;O::SolveSession session;};
struct BasisBox {std::shared_ptr<const O::LpBasis> basis;};
struct RepairBox {O::RelaxationResult result;std::vector<uint8_t> active;};
template<class T> std::shared_ptr<T> get(uint64_t handle,Kind kind){
  auto& r=registry();std::lock_guard<std::mutex> lock(r.mutex);const auto found=r.entries.find(handle);
  if(!handle||found==r.entries.end()||found->second.kind!=kind)throw ApiError{GECODE_OPT_INVALID_HANDLE,"unknown, destroyed, or wrong-kind handle"};
  return std::static_pointer_cast<T>(found->second.value);
}
template<class T> uint64_t put(Kind kind,std::shared_ptr<T> value){
  auto& r=registry();std::lock_guard<std::mutex> lock(r.mutex);
  if(r.next==std::numeric_limits<uint64_t>::max())throw ApiError{GECODE_OPT_INTERNAL_ERROR,"handle token space exhausted"};
  const auto token=r.next++;r.entries.emplace(token,Entry{kind,std::move(value)});return token;
}
void destroy(uint64_t handle,Kind kind){
  std::shared_ptr<void> doomed;
  {auto& r=registry();std::lock_guard<std::mutex> lock(r.mutex);const auto found=r.entries.find(handle);
    if(!handle||found==r.entries.end()||found->second.kind!=kind)throw ApiError{GECODE_OPT_INVALID_HANDLE,"unknown, destroyed, or wrong-kind handle"};
    doomed=std::move(found->second.value);r.entries.erase(found);}
}
template<class F> void mutate(uint64_t handle,F&& f){auto box=get<ModelBox>(handle,Kind::Model);std::lock_guard<std::mutex> lock(box->mutex);f(box->model);}
O::ModelSnapshot snapshot(uint64_t handle){auto box=get<ModelBox>(handle,Kind::Model);std::lock_guard<std::mutex> lock(box->mutex);return box->model.snapshot();}
template<class F> void mutate_quadratic(uint64_t handle,F&& f){
  auto box=get<QuadraticBox>(handle,Kind::QuadraticModel);
  std::lock_guard<std::mutex> lock(box->mutex);f(box->model);
}
O::QuadraticSnapshot quadratic_snapshot(uint64_t handle){
  auto box=get<QuadraticBox>(handle,Kind::QuadraticModel);
  std::lock_guard<std::mutex> lock(box->mutex);return box->model.snapshot();
}
O::Variable variable(gecode_opt_id id){if(id.kind!=GECODE_OPT_VARIABLE_ID||id.reserved)argument("wrong variable ID kind/reserved field");return {id.model_id,id.slot};}
O::Constraint row(gecode_opt_id id){if(id.kind!=GECODE_OPT_ROW_ID||id.reserved)argument("wrong row ID kind/reserved field");return {id.model_id,id.slot};}
O::GlobalConstraint global(gecode_opt_id id){if(id.kind!=GECODE_OPT_GLOBAL_ID||id.reserved)argument("wrong global ID kind/reserved field");return {id.model_id,id.slot};}
O::Indicator indicator(gecode_opt_id id){if(id.kind!=GECODE_OPT_INDICATOR_ID||id.reserved)argument("wrong indicator ID kind/reserved field");return {id.model_id,id.slot};}
gecode_opt_id identifier(O::GlobalConstraint id) noexcept {return {id.model_id,id.id,GECODE_OPT_GLOBAL_ID,0};}
gecode_opt_id identifier(O::Variable id) noexcept {return {id.model_id,id.id,GECODE_OPT_VARIABLE_ID,0};}
gecode_opt_id identifier(O::Constraint id) noexcept {return {id.model_id,id.id,GECODE_OPT_ROW_ID,0};}
template<class T> std::size_t length(uint64_t n){
  if(n>std::numeric_limits<std::size_t>::max()||n>static_cast<uint64_t>(std::numeric_limits<std::ptrdiff_t>::max())/sizeof(T))
    argument("count exceeds representable address/vector range");
  if(n>std::vector<T>().max_size())argument("count exceeds vector maximum size");
  return static_cast<std::size_t>(n);
}
template<class T> std::size_t count(uint64_t n,const T* pointer){
  const auto size=length<T>(n);if(n&&!pointer)argument("nonzero count requires a non-NULL array");return size;
}
std::vector<O::Variable> variables(const gecode_opt_id* input,uint64_t n){
  const auto size=count(n,input);std::vector<O::Variable> result;result.reserve(size);
  for(std::size_t i=0;i<size;++i)result.push_back(variable(input[i]));return result;
}
std::vector<std::int64_t> integers(const int64_t* input,uint64_t n){
  const auto size=count(n,input);if(!size)return {};return {input,input+size};
}
std::vector<O::Term> terms(const gecode_opt_term* input,uint64_t n){std::vector<O::Term> result;const auto size=count(n,input);result.reserve(size);
  for(std::size_t i=0;i<size;++i){if(!std::isfinite(input[i].coefficient))argument("coefficient must be finite");result.push_back({variable(input[i].variable),input[i].coefficient});}return result;}
O::VariableType type(int32_t value){switch(value){
  case GECODE_OPT_CONTINUOUS:return O::VariableType::Continuous;case GECODE_OPT_INTEGER:return O::VariableType::Integer;
  case GECODE_OPT_BINARY:return O::VariableType::Binary;case GECODE_OPT_SEMI_CONTINUOUS:return O::VariableType::SemiContinuous;
  case GECODE_OPT_SEMI_INTEGER:return O::VariableType::SemiInteger;default:argument("unknown variable type enum");}}
// Bulk and Regular builders enforce UTF-8; older scalar builders are unchanged.
std::string bulk_name(const char* name){
  if(!name)return {};
  const auto* p=reinterpret_cast<const unsigned char*>(name);
  while(*p){
    const unsigned char first=*p++;
    if(first<0x80)continue;
    unsigned count=0;uint32_t code=0,min=0;
    if(first>=0xc2&&first<=0xdf){count=1;code=first&0x1f;min=0x80;}
    else if(first>=0xe0&&first<=0xef){count=2;code=first&0x0f;min=0x800;}
    else if(first>=0xf0&&first<=0xf4){count=3;code=first&7;min=0x10000;}
    else argument("bulk name is not valid UTF-8");
    for(unsigned i=0;i<count;++i){
      const unsigned char next=*p; // Stop at NUL without reading beyond it.
      if(next<0x80||next>0xbf)argument("bulk name is not valid UTF-8");
      ++p;code=(code<<6)|(next&0x3f);
    }
    if(code<min||code>0x10ffff||(code>=0xd800&&code<=0xdfff))argument("bulk name is not valid UTF-8");
  }
  return name;
}
void bulk_output(gecode_opt_id* output,uint64_t capacity,uint64_t needed){
  length<gecode_opt_id>(capacity);
  if(capacity<needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"bulk output capacity is smaller than input entity count; nothing posted"};
  if(capacity&&!output)argument("nonzero output capacity requires a non-NULL array");
}
std::vector<std::size_t> indices(const uint64_t* input,uint64_t n){
  const auto size=count(n,input);std::vector<std::size_t> out;out.reserve(size);
  for(std::size_t i=0;i<size;++i){
    if(input[i]>std::numeric_limits<std::size_t>::max())argument("CSR index exceeds size_t range");
    out.push_back(static_cast<std::size_t>(input[i]));
  }
  return out;
}
std::vector<double> numbers(const double* input,uint64_t n,bool finite=false){
  const auto size=count(n,input);std::vector<double> out;out.reserve(size);
  for(std::size_t i=0;i<size;++i){
    if(finite&&!std::isfinite(input[i]))argument("coefficient must be finite");out.push_back(input[i]);
  }
  return out;
}
O::ObjectiveSense sense(int32_t value){switch(value){case GECODE_OPT_MINIMIZE:return O::ObjectiveSense::Minimize;case GECODE_OPT_MAXIMIZE:return O::ObjectiveSense::Maximize;default:argument("unknown objective sense enum");}}
O::Backend backend(int32_t value){switch(value){case GECODE_OPT_AUTO:return O::Backend::Auto;case GECODE_OPT_HIGHS:return O::Backend::Highs;case GECODE_OPT_NATIVE:return O::Backend::Native;default:argument("unknown backend enum");}}
O::Guarantee guarantee(int32_t value){switch(value){case GECODE_OPT_NUMERICAL:return O::Guarantee::Numerical;case GECODE_OPT_EXACT:return O::Guarantee::Exact;case GECODE_OPT_CERTIFIED:return O::Guarantee::Certified;default:argument("unknown guarantee enum");}}
int32_t guarantee(O::Guarantee value){switch(value){case O::Guarantee::Numerical:return GECODE_OPT_NUMERICAL;case O::Guarantee::Exact:return GECODE_OPT_EXACT;case O::Guarantee::Certified:return GECODE_OPT_CERTIFIED;}argument("unknown backend guarantee");}
int32_t termination(O::Termination value){switch(value){
  case O::Termination::Unknown:return GECODE_OPT_UNKNOWN;case O::Termination::Optimal:return GECODE_OPT_OPTIMAL;
  case O::Termination::Infeasible:return GECODE_OPT_INFEASIBLE;case O::Termination::Unbounded:return GECODE_OPT_UNBOUNDED;
  case O::Termination::InfeasibleOrUnbounded:return GECODE_OPT_INFEASIBLE_OR_UNBOUNDED;case O::Termination::TimeLimit:return GECODE_OPT_TIME_LIMIT;
  case O::Termination::NodeLimit:return GECODE_OPT_NODE_LIMIT;case O::Termination::MemoryLimit:return GECODE_OPT_MEMORY_LIMIT;
  case O::Termination::IterationLimit:return GECODE_OPT_ITERATION_LIMIT;case O::Termination::SolutionLimit:return GECODE_OPT_SOLUTION_LIMIT;
  case O::Termination::ObjectiveLimit:return GECODE_OPT_OBJECTIVE_LIMIT;case O::Termination::Cancelled:return GECODE_OPT_CANCELLED;
  case O::Termination::NumericalFailure:return GECODE_OPT_NUMERICAL_FAILURE;case O::Termination::Unsupported:return GECODE_OPT_UNSUPPORTED;
  case O::Termination::InvalidModel:return GECODE_OPT_INVALID_MODEL;case O::Termination::BackendError:return GECODE_OPT_BACKEND_ERROR;}
  argument("unknown backend termination");
}
O::SolveOptions options(const gecode_opt_options_v1* input){
  O::SolveOptions result;if(!input)return result;
  if(input->struct_size!=sizeof(*input))argument("options struct_size does not match ABI v1");
  if(input->reserved||input->has_node_limit<0||input->has_node_limit>1)argument("invalid reserved field or node-limit presence flag");
  result.backend=backend(input->backend);result.guarantee=guarantee(input->guarantee);
  result.threads=input->threads;result.random_seed=input->random_seed;result.time_limit_seconds=input->time_limit_seconds;
  result.relative_gap=input->relative_gap;result.absolute_gap=input->absolute_gap;
  result.feasibility_tolerance=input->feasibility_tolerance;result.integrality_tolerance=input->integrality_tolerance;
  if(input->has_node_limit)result.node_limit=input->node_limit;
  if(input->cancellation)result.cancellation=get<O::CancellationToken>(input->cancellation,Kind::Cancellation);
  const auto n=count(input->primal_start_count,input->primal_start);result.primal_start.reserve(n);
  for(std::size_t i=0;i<n;++i)result.primal_start.push_back({variable(input->primal_start[i].variable),input->primal_start[i].value});
  result.validate();return result;
}
void size_check(const void* pointer,uint64_t actual,std::size_t expected){required(pointer);if(actual!=expected)argument("output struct size does not match ABI v1");}
using Clock=std::chrono::steady_clock;
void account_preparation(O::SolveOptions& options,Clock::time_point start){
  if(std::isfinite(options.time_limit_seconds))options.time_limit_seconds=std::max(0.0,options.time_limit_seconds-std::chrono::duration<double>(Clock::now()-start).count());
}
gecode_opt_options_v1 default_options(){
  O::SolveOptions o;gecode_opt_options_v1 out{};out.struct_size=sizeof(out);
  out.backend=GECODE_OPT_AUTO;out.guarantee=GECODE_OPT_NUMERICAL;
  out.threads=o.threads;out.random_seed=o.random_seed;out.time_limit_seconds=o.time_limit_seconds;
  out.relative_gap=o.relative_gap;out.absolute_gap=o.absolute_gap;
  out.feasibility_tolerance=o.feasibility_tolerance;out.integrality_tolerance=o.integrality_tolerance;return out;
}
gecode_opt_optional_number_v1 optional(std::optional<double> value){return {value.has_value(),0,value.value_or(0)};}
int32_t completion(O::PoolCompletion value){switch(value){
  case O::PoolCompletion::Incomplete:return GECODE_OPT_POOL_INCOMPLETE;
  case O::PoolCompletion::RequestedLimit:return GECODE_OPT_POOL_REQUESTED_LIMIT;
  case O::PoolCompletion::Exhausted:return GECODE_OPT_POOL_EXHAUSTED;}
  argument("unknown pool completion");
}
O::RelaxationSide side(int32_t value){switch(value){case GECODE_OPT_RELAX_LOWER:return O::RelaxationSide::Lower;
  case GECODE_OPT_RELAX_UPPER:return O::RelaxationSide::Upper;default:argument("unknown relaxation side");}}
int32_t side(O::RelaxationSide value){switch(value){case O::RelaxationSide::Lower:return GECODE_OPT_RELAX_LOWER;
  case O::RelaxationSide::Upper:return GECODE_OPT_RELAX_UPPER;}argument("unknown relaxation side");}
template<class T> const T& at(const std::vector<T>& values,uint64_t index){
  if(index>=values.size())argument("workflow index is out of range");return values[static_cast<std::size_t>(index)];
}
template<class T> bool output_array(T* buffer,uint64_t capacity,uint64_t* needed,std::size_t n){
  required(needed);*needed=static_cast<uint64_t>(n);if(!capacity&&!buffer)return false;
  count(capacity,buffer);if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"workflow buffer is smaller than required count"};return true;
}
void output_text(const std::string& text,char* buffer,uint64_t capacity,uint64_t* needed){
  if(text.size()==std::numeric_limits<std::size_t>::max()||text.size()==std::numeric_limits<uint64_t>::max())argument("text length cannot include NUL");
  if(output_array(buffer,capacity,needed,text.size()+1))std::memcpy(buffer,text.c_str(),text.size()+1);
}
void original_variable(const RepairBox& box,O::Variable variable){
  if(variable.model_id!=box.result.source_model_id||variable.id>=box.active.size()||!box.active[variable.id])
    throw O::ModelError("repair lookup variable is foreign, absent, or deleted");
}
}

#define API(name, signature, ...) extern "C" int32_t gecode_opt_v1_##name signature noexcept {return boundary([&](){__VA_ARGS__;});}
extern "C" uint32_t gecode_opt_v1_abi_version(void) noexcept {return 1;}
extern "C" const char* gecode_opt_v1_last_error(void) noexcept {return last_error;}
API(options_default,(gecode_opt_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::SolveOptions o;*out={};out->struct_size=sizeof(*out);out->backend=GECODE_OPT_AUTO;out->guarantee=GECODE_OPT_NUMERICAL;
  out->threads=o.threads;out->random_seed=o.random_seed;out->time_limit_seconds=o.time_limit_seconds;out->relative_gap=o.relative_gap;out->absolute_gap=o.absolute_gap;
  out->feasibility_tolerance=o.feasibility_tolerance;out->integrality_tolerance=o.integrality_tolerance)
API(capabilities,(int32_t selection,int32_t* available,int32_t* lp,int32_t* mip),
  required(available);required(lp);required(mip);auto c=O::capabilities(backend(selection));*available=c.available;*lp=c.linear_programming;*mip=c.mixed_integer_linear)
API(model_create,(gecode_opt_handle* out),required(out);*out=0;*out=put(Kind::Model,std::make_shared<ModelBox>()))
API(model_destroy,(gecode_opt_handle handle),destroy(handle,Kind::Model))
API(model_identity,(gecode_opt_handle handle,uint64_t* owner,uint64_t* revision),
  required(owner);required(revision);mutate(handle,[&](O::Model& m){*owner=m.id();*revision=m.revision();}))
API(model_read,(const char* filename,gecode_opt_handle* out),required(filename);required(out);*out=0;*out=put(Kind::Model,std::make_shared<ModelBox>(O::read_model(filename))))
API(model_write,(gecode_opt_handle handle,const char* filename),required(filename);O::write_model(snapshot(handle),filename))
API(model_add_variable,(gecode_opt_handle handle,int32_t kind,double lower,double upper,const char* name,gecode_opt_id* out),
  required(out);*out={};const auto t=type(kind);mutate(handle,[&](O::Model& m){auto v=m.add_variable(t,lower,upper,name?name:"");*out={v.model_id,v.id,GECODE_OPT_VARIABLE_ID,0};}))
API(model_add_row,(gecode_opt_handle handle,const gecode_opt_term* input,uint64_t n,double lower,double upper,const char* name,gecode_opt_id* out),
  required(out);*out={};auto ts=terms(input,n);mutate(handle,[&](O::Model& m){auto r=m.add_row(ts,lower,upper,name?name:"");*out={r.model_id,r.id,GECODE_OPT_ROW_ID,0};}))
API(model_add_variables,(gecode_opt_handle h,const gecode_opt_variable_spec_v1* input,uint64_t n,gecode_opt_id* output,uint64_t capacity),
  const auto size=count(n,input);length<O::VariableSpec>(n);bulk_output(output,capacity,n);
  std::vector<O::VariableSpec> specs;specs.reserve(size);
  for(std::size_t i=0;i<size;++i){const auto& v=input[i];
    if(v.struct_size!=sizeof(v)||v.reserved)argument("invalid bulk variable spec size or reserved field");
    specs.push_back({type(v.type),v.lower,v.upper,bulk_name(v.name)});}
  mutate(h,[&](O::Model& m){auto ids=m.add_variables(specs);
    for(std::size_t i=0;i<ids.size();++i)output[i]=identifier(ids[i]);}))
API(model_add_rows,(gecode_opt_handle h,const gecode_opt_row_spec_v1* input,uint64_t n,gecode_opt_id* output,uint64_t capacity),
  const auto size=count(n,input);length<O::RowSpec>(n);bulk_output(output,capacity,n);
  std::vector<O::RowSpec> specs;specs.reserve(size);
  for(std::size_t i=0;i<size;++i){const auto& r=input[i];
    if(r.struct_size!=sizeof(r)||r.reserved)argument("invalid bulk row spec size or reserved field");
    specs.push_back({terms(r.terms,r.term_count),r.lower,r.upper,bulk_name(r.name)});}
  mutate(h,[&](O::Model& m){auto ids=m.add_rows(specs);
    for(std::size_t i=0;i<ids.size();++i)output[i]=identifier(ids[i]);}))
API(model_add_rows_sparse,(gecode_opt_handle h,const gecode_opt_sparse_row_batch_v1* input,gecode_opt_id* output,uint64_t capacity),
  required(input);const auto& b=*input;
  if(b.struct_size!=sizeof(b)||b.reserved)argument("invalid CSR batch size or reserved field");
  length<O::RowSpec>(b.lower_count);bulk_output(output,capacity,b.lower_count);
  if(b.lower_count!=b.upper_count||!b.row_start_count||b.row_start_count-1!=b.lower_count||
     b.column_count!=b.coefficient_count||(b.names_count&&b.names_count!=b.lower_count))
    argument("CSR independent array dimensions do not agree");
  O::SparseRowBatch batch;
  batch.columns=variables(b.columns,b.columns_count);batch.row_start=indices(b.row_start,b.row_start_count);
  batch.column=indices(b.column,b.column_count);batch.coefficient=numbers(b.coefficient,b.coefficient_count,true);
  batch.lower=numbers(b.lower,b.lower_count);batch.upper=numbers(b.upper,b.upper_count);
  const auto names=count(b.names_count,b.names);length<std::string>(b.names_count);batch.names.reserve(names);
  for(std::size_t i=0;i<names;++i)batch.names.push_back(bulk_name(b.names[i]));
  mutate(h,[&](O::Model& m){auto ids=m.add_rows_sparse(batch);
    for(std::size_t i=0;i<ids.size();++i)output[i]=identifier(ids[i]);}))
API(model_set_objective,(gecode_opt_handle handle,const gecode_opt_term* input,uint64_t n,int32_t direction,double offset),
  auto ts=terms(input,n);auto s=sense(direction);mutate(handle,[&](O::Model& m){m.set_objective(ts,s,offset);}))
API(model_set_variable_bounds,(gecode_opt_handle h,gecode_opt_id v,double lb,double ub),mutate(h,[&](O::Model& m){m.set_bounds(variable(v),lb,ub);}))
API(model_set_row_bounds,(gecode_opt_handle h,gecode_opt_id r,double lb,double ub),mutate(h,[&](O::Model& m){m.set_bounds(row(r),lb,ub);}))
API(model_set_coefficient,(gecode_opt_handle h,gecode_opt_id r,gecode_opt_id v,double value),mutate(h,[&](O::Model& m){m.set_coefficient(row(r),variable(v),value);}))
API(model_set_objective_coefficient,(gecode_opt_handle h,gecode_opt_id v,double value),mutate(h,[&](O::Model& m){m.set_objective_coefficient(variable(v),value);}))
API(model_set_objective_offset,(gecode_opt_handle h,double offset),mutate(h,[&](O::Model& m){m.set_objective_offset(offset);}))
API(model_set_variable_name,(gecode_opt_handle h,gecode_opt_id v,const char* name),required(name);mutate(h,[&](O::Model& m){m.set_name(variable(v),name);}))
API(model_set_row_name,(gecode_opt_handle h,gecode_opt_id r,const char* name),required(name);mutate(h,[&](O::Model& m){m.set_name(row(r),name);}))
API(model_remove_variable,(gecode_opt_handle h,gecode_opt_id v),mutate(h,[&](O::Model& m){m.remove(variable(v));}))
API(model_remove_row,(gecode_opt_handle h,gecode_opt_id r),mutate(h,[&](O::Model& m){m.remove(row(r));}))
API(model_add_all_different,(gecode_opt_handle h,const gecode_opt_id* input,uint64_t n,const char* name,gecode_opt_id* out),
  required(out);*out={};auto vs=variables(input,n);mutate(h,[&](O::Model& m){*out=identifier(O::add_all_different(m,vs,name?name:""));}))
API(model_add_element,(gecode_opt_handle h,gecode_opt_id index,const gecode_opt_id* input,uint64_t n,gecode_opt_id result,int64_t base,const char* name,gecode_opt_id* out),
  required(out);*out={};auto vs=variables(input,n);auto i=variable(index);auto r=variable(result);
  mutate(h,[&](O::Model& m){*out=identifier(O::add_element(m,i,vs,r,base,name?name:""));}))
API(model_add_table,(gecode_opt_handle h,const gecode_opt_id* input,uint64_t arity,const int64_t* flat,uint64_t n,uint64_t rows,const char* name,gecode_opt_id* out),
  required(out);*out={};if(arity&&rows>std::numeric_limits<uint64_t>::max()/arity)argument("table dimensions overflow");
  if(n!=arity*rows)argument("table value_count must equal arity times tuple_count");
  const auto row_count=length<std::vector<std::int64_t>>(rows);auto vs=variables(input,arity);auto values=integers(flat,n);
  std::vector<std::vector<std::int64_t>> tuples;tuples.reserve(row_count);
  if(vs.empty())tuples.resize(row_count);
  else for(std::size_t i=0;i<row_count;++i){const auto begin=values.begin()+i*vs.size();tuples.emplace_back(begin,begin+vs.size());}
  mutate(h,[&](O::Model& m){*out=identifier(O::add_table(m,vs,tuples,name?name:""));}))
API(model_add_cumulative,(gecode_opt_handle h,const gecode_opt_id* input,uint64_t n,const int64_t* durations,uint64_t nd,const int64_t* heights,uint64_t nh,int64_t capacity,const char* name,gecode_opt_id* out),
  required(out);*out={};if(n!=nd||n!=nh)argument("cumulative array counts must agree");auto vs=variables(input,n);auto ds=integers(durations,nd);auto hs=integers(heights,nh);
  mutate(h,[&](O::Model& m){*out=identifier(O::add_cumulative(m,vs,ds,hs,capacity,name?name:""));}))
API(model_add_circuit,(gecode_opt_handle h,const gecode_opt_id* input,uint64_t n,int64_t base,const char* name,gecode_opt_id* out),
  required(out);*out={};auto vs=variables(input,n);mutate(h,[&](O::Model& m){*out=identifier(O::add_circuit(m,vs,base,name?name:""));}))
API(model_remove_global,(gecode_opt_handle h,gecode_opt_id id),mutate(h,[&](O::Model& m){m.remove(global(id));}))
API(model_set_global_name,(gecode_opt_handle h,gecode_opt_id id,const char* name),required(name);mutate(h,[&](O::Model& m){m.set_name(global(id),name);}))
API(model_add_regular,(gecode_opt_handle h,const gecode_opt_id* input,uint64_t n,uint64_t states,uint64_t initial,
    const gecode_opt_regular_transition_v1* edges,uint64_t edge_count,uint64_t element_size,
    const uint64_t* finals,uint64_t final_count,const char* name,gecode_opt_id* out),
  required(out);*out={};
  if(element_size!=sizeof(*edges))argument("regular transition element size does not match ABI v1");
  count(n,input);const auto ne=count(edge_count,edges);const auto nf=count(final_count,finals);length<O::RegularTransition>(edge_count);
  for(std::size_t i=0;i<ne;++i)if(edges[i].struct_size!=sizeof(*edges)||edges[i].reserved)
    argument("regular transition struct_size/reserved field does not match ABI v1");
  O::RegularData data;data.variables=variables(input,n);data.state_count=states;data.initial_state=initial;
  data.transitions.reserve(ne);for(std::size_t i=0;i<ne;++i)data.transitions.push_back({edges[i].from,edges[i].symbol,edges[i].to});
  if(nf)data.final_states.assign(finals,finals+nf);auto label=bulk_name(name);
  mutate(h,[&](O::Model& m){*out=identifier(m.add_global(std::move(data),std::move(label)));}))
API(model_add_indicator,(gecode_opt_handle h,gecode_opt_id activator,int32_t active_value,const gecode_opt_term* input,uint64_t n,double lower,double upper,const char* name,gecode_opt_id* out,int32_t* has_gate,gecode_opt_id* gate),
  required(out);required(has_gate);required(gate);*out={};*gate={};*has_gate=0;
  if(active_value!=0&&active_value!=1)argument("indicator active_value must be zero or one");auto ts=terms(input,n);auto v=variable(activator);
  mutate(h,[&](O::Model& m){auto data=O::add_indicator(m,v,active_value==1,ts,lower,upper,name?name:"");
    *out={data.indicator.model_id,data.indicator.id,GECODE_OPT_INDICATOR_ID,0};
    if(data.inactive_gate){*has_gate=1;*gate={data.inactive_gate->model_id,data.inactive_gate->id,GECODE_OPT_VARIABLE_ID,0};}}))
API(model_remove_indicator,(gecode_opt_handle h,gecode_opt_id id),mutate(h,[&](O::Model& m){O::remove_indicator(m,indicator(id));}))
API(model_add_boolean_and,(gecode_opt_handle h,gecode_opt_id result,const gecode_opt_id* input,uint64_t n,const char* name),
  auto vs=variables(input,n);auto r=variable(result);mutate(h,[&](O::Model& m){O::add_boolean_and(m,r,vs,name?name:"");}))
API(model_add_boolean_or,(gecode_opt_handle h,gecode_opt_id result,const gecode_opt_id* input,uint64_t n,const char* name),
  auto vs=variables(input,n);auto r=variable(result);mutate(h,[&](O::Model& m){O::add_boolean_or(m,r,vs,name?name:"");}))
API(cancellation_create,(gecode_opt_handle* out),required(out);*out=0;*out=put(Kind::Cancellation,std::make_shared<O::CancellationToken>()))
API(cancellation_cancel,(gecode_opt_handle h),get<O::CancellationToken>(h,Kind::Cancellation)->cancel())
API(cancellation_is_cancelled,(gecode_opt_handle h,int32_t* out),required(out);*out=get<O::CancellationToken>(h,Kind::Cancellation)->cancelled())
API(cancellation_copy,(gecode_opt_handle h,gecode_opt_handle* out),required(out);*out=0;auto token=get<O::CancellationToken>(h,Kind::Cancellation);*out=put(Kind::Cancellation,std::move(token)))
API(cancellation_destroy,(gecode_opt_handle h),destroy(h,Kind::Cancellation))
API(session_create,(gecode_opt_handle* out),required(out);*out=0;*out=put(Kind::Session,std::make_shared<SessionBox>()))
API(session_destroy,(gecode_opt_handle h),destroy(h,Kind::Session))
API(session_reset,(gecode_opt_handle h),auto box=get<SessionBox>(h,Kind::Session);std::lock_guard<std::mutex> lock(box->mutex);box->session.reset())
API(session_statistics,(gecode_opt_handle h,gecode_opt_session_statistics_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto box=get<SessionBox>(h,Kind::Session);std::lock_guard<std::mutex> lock(box->mutex);auto s=box->session.statistics();
  *out={s.solve_calls,s.model_loads,s.incremental_updates,s.unchanged_models,s.basis_warm_starts,s.incumbent_starts})
API(solve,(gecode_opt_handle h,const gecode_opt_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=options(input);auto model=snapshot(h);account_preparation(opts,start);*out=put(Kind::Result,std::make_shared<O::SolveResult>(O::solve(model,opts))))
API(session_solve,(gecode_opt_handle session,gecode_opt_handle h,const gecode_opt_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=options(input);auto model=snapshot(h);auto box=get<SessionBox>(session,Kind::Session);
  std::lock_guard<std::mutex> lock(box->mutex);account_preparation(opts,start);*out=put(Kind::Result,std::make_shared<O::SolveResult>(box->session.solve(model,opts))))
API(result_destroy,(gecode_opt_handle h),destroy(h,Kind::Result))
API(result_info,(gecode_opt_handle h,gecode_opt_result_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto r=get<O::SolveResult>(h,Kind::Result);
  *out={r->model_id,r->revision,static_cast<uint64_t>(r->active_variables.size()),termination(r->termination),guarantee(r->guarantee),r->has_solution(),r->solution_validated,r->start_submitted,0,r->elapsed_seconds})
API(result_number,(gecode_opt_handle h,int32_t field,int32_t* present,double* value),
  required(present);required(value);*present=0;*value=0;auto r=get<O::SolveResult>(h,Kind::Result);std::optional<double> number;
  switch(field){case GECODE_OPT_OBJECTIVE:number=r->objective;break;case GECODE_OPT_BEST_BOUND:number=r->best_bound;break;
    case GECODE_OPT_ABSOLUTE_GAP:number=r->absolute_gap;break;case GECODE_OPT_RELATIVE_GAP:number=r->relative_gap;break;
    case GECODE_OPT_NATIVE_GAP:number=r->native_backend_gap;break;default:argument("unknown result number field");}
  if(number){*present=1;*value=*number;})
API(result_value,(gecode_opt_handle h,gecode_opt_id v,double* out),required(out);auto r=get<O::SolveResult>(h,Kind::Result);
  if(!r->has_solution())throw ApiError{GECODE_OPT_NO_SOLUTION,"result has no validated solution"};*out=r->value(variable(v)))
API(result_values,(gecode_opt_handle h,double* values,uint8_t* active,uint8_t* present,uint64_t capacity,uint64_t* needed),
  required(needed);auto r=get<O::SolveResult>(h,Kind::Result);*needed=static_cast<uint64_t>(r->active_variables.size());
  if(capacity==0&&!values&&!active&&!present)return;
  count(capacity,values);required(active);required(present);
  if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"value buffers are smaller than required slot count"};
  const bool solution=r->has_solution();for(std::size_t i=0;i<r->active_variables.size();++i){active[i]=r->active_variables[i];present[i]=active[i]&&solution;values[i]=present[i]?r->values[i]:0;})
API(result_text,(gecode_opt_handle h,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  required(needed);auto r=get<O::SolveResult>(h,Kind::Result);const std::string* text=nullptr;
  switch(field){case GECODE_OPT_BACKEND_NAME:text=&r->backend;break;case GECODE_OPT_BACKEND_VERSION:text=&r->backend_version;break;case GECODE_OPT_MESSAGE:text=&r->message;break;default:argument("unknown result text field");}
  if(text->size()==std::numeric_limits<uint64_t>::max())argument("text length cannot include NUL");*needed=static_cast<uint64_t>(text->size())+1;
  if(!capacity&&!buffer)return;count(capacity,buffer);if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"text buffer is smaller than required NUL-terminated length"};
  std::memcpy(buffer,text->c_str(),static_cast<std::size_t>(*needed)))
API(pool_options_default,(gecode_opt_pool_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));*out={};out->struct_size=sizeof(*out);out->solve=default_options();out->max_solutions=10)
API(pool_solve,(gecode_opt_handle h,const gecode_opt_pool_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;O::PoolOptions opts;
  if(input){if(input->struct_size!=sizeof(*input)||input->reserved||input->has_projection<0||input->has_projection>1)
      argument("invalid pool options size, reserved field, or projection presence flag");
    opts.solve=options(&input->solve);opts.max_solutions=length<O::PoolEntry>(input->max_solutions);
    if(input->has_projection)opts.projection=variables(input->projection,input->projection_count);
    else if(input->projection||input->projection_count)argument("absent projection requires NULL pointer and zero count");}
  auto model=snapshot(h);account_preparation(opts.solve,start);
  *out=put(Kind::Pool,std::make_shared<O::PoolResult>(O::solve_pool(model,opts))))
API(pool_destroy,(gecode_opt_handle h),destroy(h,Kind::Pool))
API(pool_info,(gecode_opt_handle h,gecode_opt_pool_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto r=get<O::PoolResult>(h,Kind::Pool);
  *out={r->model_id,r->revision,static_cast<uint64_t>(r->projection.size()),static_cast<uint64_t>(r->entries.size()),
    static_cast<uint64_t>(r->attempts.size()),static_cast<uint64_t>(r->ranked_prefix),termination(r->termination),completion(r->completion),guarantee(r->guarantee),0,r->elapsed_seconds})
API(pool_projection,(gecode_opt_handle h,gecode_opt_id* buffer,uint64_t capacity,uint64_t* needed),
  auto r=get<O::PoolResult>(h,Kind::Pool);if(output_array(buffer,capacity,needed,r->projection.size()))
    for(std::size_t i=0;i<r->projection.size();++i)buffer[i]=identifier(r->projection[i]))
API(pool_entry_info,(gecode_opt_handle h,uint64_t index,gecode_opt_pool_entry_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto r=get<O::PoolResult>(h,Kind::Pool);const auto& e=at(r->entries,index);
  *out={static_cast<uint64_t>(e.projection_values.size()),e.rank_established,0})
API(pool_entry_result,(gecode_opt_handle h,uint64_t index,gecode_opt_handle* out),
  required(out);*out=0;auto r=get<O::PoolResult>(h,Kind::Pool);*out=put(Kind::Result,std::make_shared<O::SolveResult>(at(r->entries,index).solution)))
API(pool_entry_projection,(gecode_opt_handle h,uint64_t index,int64_t* buffer,uint64_t capacity,uint64_t* needed),
  auto r=get<O::PoolResult>(h,Kind::Pool);const auto& values=at(r->entries,index).projection_values;
  if(output_array(buffer,capacity,needed,values.size()))std::copy(values.begin(),values.end(),buffer))
API(pool_attempt_info,(gecode_opt_handle h,uint64_t index,gecode_opt_pool_attempt_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto r=get<O::PoolResult>(h,Kind::Pool);const auto& a=at(r->attempts,index);
  *out={termination(a.termination),guarantee(a.guarantee),a.candidate_accepted,a.rank_established,optional(a.objective),optional(a.remaining_bound)})
API(pool_message,(gecode_opt_handle h,char* buffer,uint64_t capacity,uint64_t* needed),
  auto r=get<O::PoolResult>(h,Kind::Pool);output_text(r->message,buffer,capacity,needed))
API(repair_options_default,(gecode_opt_repair_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));*out={};out->struct_size=sizeof(*out);out->solve=default_options())
API(repair_solve,(gecode_opt_handle h,const gecode_opt_repair_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;O::RelaxationOptions opts;
  if(input){if(input->struct_size!=sizeof(*input)||input->reserved||input->optimize_original_objective<0||input->optimize_original_objective>1)
      argument("invalid repair options size, reserved field, or objective refinement flag");
    opts.solve=options(&input->solve);opts.optimize_original_objective=input->optimize_original_objective;
    const auto n=count(input->selection_count,input->selections);
    for(std::size_t i=0;i<n;++i){const auto& selection=input->selections[i];
      if(selection.reserved)argument("relaxation selection reserved field must be zero");const auto s=side(selection.side);
      if(!std::isfinite(selection.penalty)||selection.penalty<=0)argument("relaxation penalty must be positive and finite");
      if(selection.source.kind==GECODE_OPT_VARIABLE_ID)opts.bounds.push_back({variable(selection.source),s,selection.penalty});
      else if(selection.source.kind==GECODE_OPT_ROW_ID)opts.rows.push_back({row(selection.source),s,selection.penalty});
      else argument("relaxation selection requires a Variable or Row ID");}}
  auto model=snapshot(h);auto box=std::make_shared<RepairBox>();box->active.reserve(model.variables.size());
  for(const auto& v:model.variables)box->active.push_back(v.active);
  account_preparation(opts.solve,start);box->result=O::relax_feasibility(model,opts);*out=put(Kind::Repair,std::move(box)))
API(repair_destroy,(gecode_opt_handle h),destroy(h,Kind::Repair))
API(repair_info,(gecode_opt_handle h,gecode_opt_repair_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<RepairBox>(h,Kind::Repair);const auto& r=b->result;
  *out={r.source_model_id,r.source_revision,r.private_model?r.private_model->model_id:0,r.private_model?r.private_model->revision:0,
    static_cast<uint64_t>(b->active.size()),static_cast<uint64_t>(r.items.size()),static_cast<uint64_t>(r.workflow.stages.size()),
    static_cast<uint64_t>(r.workflow.completed_stages),termination(r.termination),guarantee(r.guarantee),r.private_model.has_value(),r.has_repair(),
    r.minimum_violation_established,r.original_objective_optimized,termination(r.workflow.termination),guarantee(r.workflow.guarantee),r.elapsed_seconds,r.workflow.elapsed_seconds})
API(repair_number,(gecode_opt_handle h,int32_t field,int32_t* present,double* value),
  required(present);required(value);*present=0;*value=0;auto b=get<RepairBox>(h,Kind::Repair);const auto& r=b->result;std::optional<double> number;
  switch(field){case GECODE_OPT_REPAIR_MINIMUM_VIOLATION:number=r.minimum_weighted_violation;break;
    case GECODE_OPT_REPAIR_VIOLATION:number=r.weighted_violation;break;case GECODE_OPT_REPAIR_ORIGINAL_OBJECTIVE:number=r.original_objective;break;
    default:argument("unknown repair number field");}if(number){*present=1;*value=*number;})
API(repair_original_value,(gecode_opt_handle h,gecode_opt_id id,double* out),
  required(out);auto b=get<RepairBox>(h,Kind::Repair);const auto v=variable(id);original_variable(*b,v);
  if(!b->result.has_repair())throw ApiError{GECODE_OPT_NO_SOLUTION,"repair has no independently validated repaired assignment"};
  *out=at(b->result.original_values,v.id))
API(repair_original_values,(gecode_opt_handle h,double* values,uint8_t* active,uint8_t* present,uint64_t capacity,uint64_t* needed),
  required(needed);auto b=get<RepairBox>(h,Kind::Repair);*needed=static_cast<uint64_t>(b->active.size());
  if(!capacity&&!values&&!active&&!present)return;count(capacity,values);required(active);required(present);
  if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"repair value buffers are smaller than source slot count"};
  const bool solution=b->result.has_repair();if(solution&&b->result.original_values.size()!=b->active.size())throw std::runtime_error("malformed repair assignment size");
  for(std::size_t i=0;i<b->active.size();++i){active[i]=b->active[i];present[i]=active[i]&&solution;values[i]=present[i]?b->result.original_values[i]:0;})
API(repair_variable_map,(gecode_opt_handle h,gecode_opt_id* source,gecode_opt_id* private_ids,uint8_t* active,uint64_t capacity,uint64_t* needed),
  required(needed);auto b=get<RepairBox>(h,Kind::Repair);*needed=static_cast<uint64_t>(b->active.size());
  if(!capacity&&!source&&!private_ids&&!active)return;count(capacity,source);required(private_ids);required(active);
  if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"repair mapping buffers are smaller than source slot count"};
  const auto& ids=b->result.private_variables;if(!ids.empty()&&ids.size()!=b->active.size())throw std::runtime_error("malformed repair variable mapping");
  for(std::size_t i=0;i<b->active.size();++i){source[i]={b->result.source_model_id,static_cast<uint64_t>(i),GECODE_OPT_VARIABLE_ID,0};
    private_ids[i]=ids.empty()?gecode_opt_id{}:identifier(ids[i]);active[i]=b->active[i];})
API(repair_validation,(gecode_opt_handle h,gecode_opt_validation_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<RepairBox>(h,Kind::Repair);const auto& v=b->result.original_validation;
  *out={v.valid,v.model_valid,static_cast<uint64_t>(v.violated_globals),v.max_bound_violation,v.max_row_violation,
    v.max_integrality_violation,v.max_indicator_violation,optional(v.objective)})
API(repair_item_info,(gecode_opt_handle h,uint64_t index,gecode_opt_repair_item_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<RepairBox>(h,Kind::Repair);const auto& item=at(b->result.items,index);
  *out={item.source_row?identifier(*item.source_row):item.source_variable?identifier(*item.source_variable):gecode_opt_id{},
    identifier(item.slack),identifier(item.penalty_row),side(item.side),0,item.original_bound,item.penalty,
    optional(item.activity),optional(item.violation),optional(item.weighted_violation),optional(item.slack_value)})
API(repair_stage_info,(gecode_opt_handle h,uint64_t index,gecode_opt_repair_stage_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<RepairBox>(h,Kind::Repair);const auto& stage=at(b->result.workflow.stages,index);
  *out={static_cast<uint64_t>(stage.index),stage.completed,0,optional(stage.retention_bound)})
API(repair_stage_result,(gecode_opt_handle h,uint64_t index,gecode_opt_handle* out),
  required(out);*out=0;auto b=get<RepairBox>(h,Kind::Repair);*out=put(Kind::Result,std::make_shared<O::SolveResult>(at(b->result.workflow.stages,index).result)))
API(repair_final_result,(gecode_opt_handle h,gecode_opt_handle* out),
  required(out);*out=0;auto b=get<RepairBox>(h,Kind::Repair);
  if(!b->result.has_repair())throw ApiError{GECODE_OPT_NO_SOLUTION,"repair has no independently validated repaired assignment"};
  *out=put(Kind::Result,std::make_shared<O::SolveResult>(b->result.workflow.final_solution)))
API(repair_violation_lock,(gecode_opt_handle h,int32_t* present,gecode_opt_id* out),
  required(present);required(out);*present=0;*out={};auto b=get<RepairBox>(h,Kind::Repair);
  if(b->result.violation_lock){*present=1;*out=identifier(*b->result.violation_lock);})
API(repair_objective_values,(gecode_opt_handle h,double* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<RepairBox>(h,Kind::Repair);const auto& values=b->result.workflow.objective_values;
  if(output_array(buffer,capacity,needed,values.size()))std::copy(values.begin(),values.end(),buffer))
API(repair_text,(gecode_opt_handle h,int32_t field,uint64_t index,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<RepairBox>(h,Kind::Repair);const auto& r=b->result;const std::string* text=nullptr;
  if(field>=GECODE_OPT_REPAIR_MESSAGE&&field<=GECODE_OPT_REPAIR_VALIDATION_MESSAGE&&index)argument("non-indexed repair text requires index zero");
  switch(field){case GECODE_OPT_REPAIR_MESSAGE:text=&r.message;break;case GECODE_OPT_REPAIR_WORKFLOW_MESSAGE:text=&r.workflow.message;break;
    case GECODE_OPT_REPAIR_VALIDATION_MESSAGE:text=&r.original_validation.message;break;
    case GECODE_OPT_REPAIR_ITEM_NAME:text=&at(r.items,index).name;break;case GECODE_OPT_REPAIR_STAGE_NAME:text=&at(r.workflow.stages,index).name;break;
    default:argument("unknown repair text field");}output_text(*text,buffer,capacity,needed))
API(quadratic_capabilities,(int32_t* available),
  required(available);*available=O::quadratic_capabilities().available)
API(quadratic_options_default,(gecode_opt_quadratic_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::QuadraticOptions defaults;*out={};out->struct_size=sizeof(*out);
  out->solve=default_options();out->iteration_limit=defaults.iteration_limit;
  out->max_auxiliary_variables=defaults.max_auxiliary_variables;out->max_lifted_nonzeros=defaults.max_lifted_nonzeros;
  out->stationarity_tolerance=defaults.stationarity_tolerance;out->complementarity_tolerance=defaults.complementarity_tolerance;
  out->optimality_tolerance=defaults.optimality_tolerance)
API(quadratic_model_create,(gecode_opt_handle* out),
  required(out);*out=0;*out=put(Kind::QuadraticModel,std::make_shared<QuadraticBox>()))
API(quadratic_model_destroy,(gecode_opt_handle h),destroy(h,Kind::QuadraticModel))
API(quadratic_model_identity,(gecode_opt_handle h,uint64_t* owner,uint64_t* revision),
  required(owner);required(revision);mutate_quadratic(h,[&](O::QuadraticModel& m){*owner=m.id();*revision=m.revision();}))
API(quadratic_model_add_continuous,(gecode_opt_handle h,double l,double u,const char* name,gecode_opt_id* out),
  required(out);*out={};auto label=bulk_name(name);
  mutate_quadratic(h,[&](O::QuadraticModel& m){*out=identifier(m.add_continuous(l,u,std::move(label)));}))
API(quadratic_model_add_row,(gecode_opt_handle h,const gecode_opt_term* input,uint64_t n,double l,double u,const char* name,gecode_opt_id* out),
  required(out);*out={};auto ts=terms(input,n);auto label=bulk_name(name);
  mutate_quadratic(h,[&](O::QuadraticModel& m){*out=identifier(m.add_row(ts,l,u,std::move(label)));}))
API(quadratic_model_set_objective,(gecode_opt_handle h,const gecode_opt_weighted_square_v1* input,uint64_t n,
    const gecode_opt_term* linear,uint64_t linear_n,int32_t objective_sense,double offset),
  const auto selected=sense(objective_sense);const auto size=count(n,input);
  length<O::WeightedSquare>(n);
  std::vector<O::WeightedSquare> squares;squares.reserve(size);
  for(std::size_t i=0;i<size;++i){const auto& q=input[i];
    if(q.struct_size!=sizeof(q))argument("square struct_size does not match ABI v1");
    if(q.reserved)argument("square reserved field must be zero");
    squares.push_back({terms(q.terms,q.term_count),q.offset,q.weight,bulk_name(q.name)});}
  auto ts=terms(linear,linear_n);
  mutate_quadratic(h,[&](O::QuadraticModel& m){
    if(selected==O::ObjectiveSense::Minimize)m.minimize_squares(squares,ts,offset);
    else m.maximize_concave_squares(squares,ts,offset);}))
API(quadratic_model_set_variable_bounds,(gecode_opt_handle h,gecode_opt_id id,double l,double u),
  const auto v=variable(id);mutate_quadratic(h,[&](O::QuadraticModel& m){m.set_bounds(v,l,u);}))
API(quadratic_model_set_row_bounds,(gecode_opt_handle h,gecode_opt_id id,double l,double u),
  const auto r=row(id);mutate_quadratic(h,[&](O::QuadraticModel& m){m.set_bounds(r,l,u);}))
API(quadratic_model_set_coefficient,(gecode_opt_handle h,gecode_opt_id r,gecode_opt_id v,double value),
  const auto ri=row(r);const auto vi=variable(v);
  mutate_quadratic(h,[&](O::QuadraticModel& m){m.set_coefficient(ri,vi,value);}))
API(quadratic_model_remove_variable,(gecode_opt_handle h,gecode_opt_id id),
  const auto v=variable(id);mutate_quadratic(h,[&](O::QuadraticModel& m){m.remove(v);}))
API(quadratic_model_remove_row,(gecode_opt_handle h,gecode_opt_id id),
  const auto r=row(id);mutate_quadratic(h,[&](O::QuadraticModel& m){m.remove(r);}))
API(quadratic_solve,(gecode_opt_handle h,const gecode_opt_quadratic_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;O::QuadraticOptions opts;
  if(input){
    if(input->struct_size!=sizeof(*input))argument("quadratic options struct_size does not match ABI v1");
    if(input->solve.struct_size!=sizeof(input->solve))argument("nested options struct_size does not match ABI v1");
    if(input->reserved)argument("quadratic options reserved field must be zero");
    opts.solve=options(&input->solve);opts.iteration_limit=input->iteration_limit;
    opts.max_auxiliary_variables=length<O::VariableData>(input->max_auxiliary_variables);
    opts.max_lifted_nonzeros=length<O::Term>(input->max_lifted_nonzeros);
    opts.stationarity_tolerance=input->stationarity_tolerance;opts.complementarity_tolerance=input->complementarity_tolerance;
    opts.optimality_tolerance=input->optimality_tolerance;opts.validate();}
  const auto model=quadratic_snapshot(h);account_preparation(opts.solve,start);
  auto result=std::make_shared<O::QuadraticResult>(O::solve_quadratic(model,opts));
  result->result.elapsed_seconds=std::chrono::duration<double>(Clock::now()-start).count();
  *out=put(Kind::QuadraticResult,std::move(result)))
API(quadratic_result_destroy,(gecode_opt_handle h),destroy(h,Kind::QuadraticResult))
API(quadratic_result_info,(gecode_opt_handle h,gecode_opt_quadratic_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const auto& r=q->result;
  *out={{r.model_id,r.revision,static_cast<uint64_t>(r.active_variables.size()),termination(r.termination),guarantee(r.guarantee),
    r.has_solution(),r.solution_validated,r.start_submitted,0,r.elapsed_seconds},q->qp_iterations,q->regularization})
API(quadratic_result_checks,(gecode_opt_handle h,gecode_opt_quadratic_checks_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const auto& c=q->checks;
  *out={c.primal_valid,c.objective_valid,c.kkt_available,c.kkt_valid,c.bound_valid,0,
    optional(c.kkt_available?std::optional<double>(c.max_stationarity):std::nullopt),
    optional(c.kkt_available?std::optional<double>(c.max_complementarity):std::nullopt),
    optional(c.original_objective),optional(c.normalized_lower_bound),optional(c.gap_upper_bound),
    static_cast<uint64_t>(c.objective_valid?c.square_values.size():0),
    static_cast<uint64_t>(c.objective_valid?c.original_gradient.size():0)})
API(quadratic_result_number,(gecode_opt_handle h,int32_t field,int32_t* present,double* value),
  required(present);required(value);*present=0;*value=0;auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);
  const auto& r=q->result;std::optional<double> number;
  switch(field){case GECODE_OPT_OBJECTIVE:number=r.objective;break;case GECODE_OPT_BEST_BOUND:number=r.best_bound;break;
    case GECODE_OPT_ABSOLUTE_GAP:number=r.absolute_gap;break;case GECODE_OPT_RELATIVE_GAP:number=r.relative_gap;break;
    case GECODE_OPT_NATIVE_GAP:number=r.native_backend_gap;break;case GECODE_OPT_QP_VENDOR_OBJECTIVE:number=q->vendor_objective;break;
    case GECODE_OPT_QP_VENDOR_DUAL_ESTIMATE:number=q->vendor_dual_estimate;break;default:argument("unknown quadratic number field");}
  if(number){*present=1;*value=*number;})
API(quadratic_result_value,(gecode_opt_handle h,gecode_opt_id v,double* out),
  required(out);auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const auto& r=q->result;
  if(!r.has_solution())throw ApiError{GECODE_OPT_NO_SOLUTION,"QP result has no validated original solution"};*out=r.value(variable(v)))
API(quadratic_result_values,(gecode_opt_handle h,double* values,uint8_t* active,uint8_t* present,uint64_t capacity,uint64_t* needed),
  required(needed);auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const auto& r=q->result;
  *needed=static_cast<uint64_t>(r.active_variables.size());if(!capacity&&!values&&!active&&!present)return;
  count(capacity,values);count(capacity,active);count(capacity,present);
  if(capacity<*needed)throw ApiError{GECODE_OPT_BUFFER_TOO_SMALL,"QP value buffers are smaller than required slot count"};
  const bool solution=r.has_solution();for(std::size_t i=0;i<r.active_variables.size();++i){
    active[i]=r.active_variables[i];present[i]=active[i]&&solution;values[i]=present[i]?r.values[i]:0;})
API(quadratic_result_array,(gecode_opt_handle h,int32_t field,double* values,uint64_t capacity,uint64_t* needed),
  auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const auto& c=q->checks;const std::vector<double>* data=nullptr;
  switch(field){case GECODE_OPT_QP_SQUARE_RESIDUALS:data=&c.square_values;break;
    case GECODE_OPT_QP_ORIGINAL_GRADIENT:data=&c.original_gradient;break;default:argument("unknown quadratic array field");}
  const auto size=c.objective_valid?data->size():0;
  if(output_array(values,capacity,needed,size)&&size)std::copy(data->begin(),data->end(),values))
API(quadratic_result_text,(gecode_opt_handle h,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  auto q=get<O::QuadraticResult>(h,Kind::QuadraticResult);const std::string* value=nullptr;
  switch(field){case GECODE_OPT_BACKEND_NAME:value=&q->result.backend;break;case GECODE_OPT_BACKEND_VERSION:value=&q->result.backend_version;break;
    case GECODE_OPT_MESSAGE:value=&q->result.message;break;case GECODE_OPT_QP_CHECK_MESSAGE:value=&q->checks.message;break;
    default:argument("unknown quadratic text field");}output_text(*value,buffer,capacity,needed))
namespace {
O::LpObservationOptions lp_options(const gecode_opt_lp_options_v1* input) {
  O::LpObservationOptions out;if(!input)return out;
  if(input->struct_size!=sizeof(*input))argument("LP options struct_size does not match ABI v1");
  if(input->reserved || input->duals<0 || input->duals>1 || input->basis<0 || input->basis>1)
    argument("invalid LP reserved field or request flag");
  out.solve=options(&input->solve);out.duals=input->duals;out.basis=input->basis;
  out.checks={input->dual_feasibility,input->stationarity,input->complementarity,input->objective_gap};
  out.validate();return out;
}
std::shared_ptr<const O::LpObservations> lp_observations(uint64_t handle) {
  auto result=get<O::LpObservedResult>(handle,Kind::LpObservedResult);
  if(!result->observations)throw ApiError{GECODE_OPT_NO_OBSERVATIONS,"result has no owning LP observations"};
  return result->observations;
}
int32_t lp_state(O::LpObservationState value) {switch(value){
  case O::LpObservationState::NotRequested:return GECODE_OPT_LP_NOT_REQUESTED;
  case O::LpObservationState::Available:return GECODE_OPT_LP_AVAILABLE;
  case O::LpObservationState::Unavailable:return GECODE_OPT_LP_UNAVAILABLE;
  case O::LpObservationState::Rejected:return GECODE_OPT_LP_REJECTED;
}argument("unknown LP LpObservationState enum");}
int32_t lp_reason(O::LpObservationReason value) {switch(value){
  case O::LpObservationReason::None:return GECODE_OPT_LP_REASON_NONE;
  case O::LpObservationReason::NotRequested:return GECODE_OPT_LP_REASON_NOT_REQUESTED;
  case O::LpObservationReason::Unsupported:return GECODE_OPT_LP_REASON_UNSUPPORTED;
  case O::LpObservationReason::NoBackendSolve:return GECODE_OPT_LP_REASON_NO_BACKEND_SOLVE;
  case O::LpObservationReason::NoPrimalPoint:return GECODE_OPT_LP_REASON_NO_PRIMAL_POINT;
  case O::LpObservationReason::NotOptimal:return GECODE_OPT_LP_REASON_NOT_OPTIMAL;
  case O::LpObservationReason::NoDualPoint:return GECODE_OPT_LP_REASON_NO_DUAL_POINT;
  case O::LpObservationReason::NoBasis:return GECODE_OPT_LP_REASON_NO_BASIS;
  case O::LpObservationReason::ElidedConstantRows:return GECODE_OPT_LP_REASON_ELIDED_CONSTANT_ROWS;
  case O::LpObservationReason::Interrupted:return GECODE_OPT_LP_REASON_INTERRUPTED;
  case O::LpObservationReason::InvalidBackendData:return GECODE_OPT_LP_REASON_INVALID_BACKEND_DATA;
  case O::LpObservationReason::FailedChecks:return GECODE_OPT_LP_REASON_FAILED_CHECKS;
  case O::LpObservationReason::AllocationFailure:return GECODE_OPT_LP_REASON_ALLOCATION_FAILURE;
  case O::LpObservationReason::InvalidModel:return GECODE_OPT_LP_REASON_INVALID_MODEL;
}argument("unknown LP LpObservationReason enum");}
int32_t lp_basis(O::LpBasisStatus value) {switch(value){
  case O::LpBasisStatus::Lower:return GECODE_OPT_LP_BASIS_LOWER;
  case O::LpBasisStatus::Basic:return GECODE_OPT_LP_BASIS_BASIC;
  case O::LpBasisStatus::Upper:return GECODE_OPT_LP_BASIS_UPPER;
  case O::LpBasisStatus::Zero:return GECODE_OPT_LP_BASIS_ZERO;
  case O::LpBasisStatus::NonbasicUnspecified:return GECODE_OPT_LP_BASIS_NONBASIC_UNSPECIFIED;
}argument("unknown LP LpBasisStatus enum");}
int32_t lp_source(O::LpDualSource value) {switch(value){
  case O::LpDualSource::None:return GECODE_OPT_LP_DUAL_NONE;
  case O::LpDualSource::Backend:return GECODE_OPT_LP_DUAL_BACKEND;
  case O::LpDualSource::DerivedConstantRow:return GECODE_OPT_LP_DUAL_DERIVED_CONSTANT_ROW;
}argument("unknown LP LpDualSource enum");}
const O::LpObservationGroup& lp_group(const O::LpObservations& data,int32_t field) {
  switch(field){case GECODE_OPT_LP_PRIMAL_ROWS:return data.primal_rows();
    case GECODE_OPT_LP_DUAL_POINT:return data.dual_point();case GECODE_OPT_LP_BASIS:return data.basis();
    default:argument("unknown LP group selector");}
}
gecode_opt_lp_row_v1 lp_row(const O::LpRowObservation& data) {
  gecode_opt_lp_row_v1 out{};out.struct_size=sizeof(out);out.active=data.active;
  out.dual_source=lp_source(data.dual_source);out.has_basis=data.basis.has_value();
  if(data.basis)out.basis=lp_basis(*data.basis);
  out.activity=optional(data.activity);out.lower_slack=optional(data.lower_slack);
  out.upper_slack=optional(data.upper_slack);out.dual=optional(data.dual);return out;
}
gecode_opt_lp_column_v1 lp_column(const O::LpColumnObservation& data) {
  gecode_opt_lp_column_v1 out{};out.struct_size=sizeof(out);out.active=data.active;
  out.has_basis=data.basis.has_value();if(data.basis)out.basis=lp_basis(*data.basis);
  out.reduced_cost=optional(data.reduced_cost);return out;
}
}
API(lp_options_default,(gecode_opt_lp_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::LpObservationOptions o;*out={};out->struct_size=sizeof(*out);out->solve=default_options();
  out->duals=o.duals;out->basis=o.basis;out->dual_feasibility=o.checks.dual_feasibility;out->stationarity=o.checks.stationarity;
  out->complementarity=o.checks.complementarity;out->objective_gap=o.checks.objective_gap)
API(lp_capabilities,(gecode_opt_lp_capabilities_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));const auto c=O::lp_observation_capabilities();*out={};out->struct_size=sizeof(*out);
  out->available=c.available;out->duals=c.duals;out->basis_export=c.basis_export;out->limitation_count=c.limitations.size())
API(lp_capability_text,(int32_t field,uint64_t index,char* buffer,uint64_t capacity,uint64_t* needed),
  const auto c=O::lp_observation_capabilities();const std::string* value=nullptr;
  if(field!=GECODE_OPT_LP_CAP_LIMITATION&&index)argument("LP capability text index must be zero");
  switch(field){case GECODE_OPT_LP_CAP_BACKEND:value=&c.backend;break;case GECODE_OPT_LP_CAP_VERSION:value=&c.backend_version;break;
    case GECODE_OPT_LP_CAP_LIMITATION:value=&at(c.limitations,index);break;default:argument("unknown LP capability text selector");}
  output_text(*value,buffer,capacity,needed))
API(solve_lp_observed,(gecode_opt_handle h,const gecode_opt_lp_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=lp_options(input);auto model=snapshot(h);
  account_preparation(opts.solve,start);*out=put(Kind::LpObservedResult,std::make_shared<O::LpObservedResult>(O::solve_lp_observed(model,opts))))
API(session_solve_lp_observed,(gecode_opt_handle session,gecode_opt_handle h,const gecode_opt_lp_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=lp_options(input);auto model=snapshot(h);auto box=get<SessionBox>(session,Kind::Session);
  std::lock_guard<std::mutex> lock(box->mutex);account_preparation(opts.solve,start);
  *out=put(Kind::LpObservedResult,std::make_shared<O::LpObservedResult>(box->session.solve_lp_observed(model,opts))))
API(lp_observed_result_destroy,(gecode_opt_handle h),destroy(h,Kind::LpObservedResult))
API(lp_observed_result_copy_result,(gecode_opt_handle h,gecode_opt_handle* out),
  required(out);*out=0;auto r=get<O::LpObservedResult>(h,Kind::LpObservedResult);
  *out=put(Kind::Result,std::make_shared<O::SolveResult>(r->result)))
API(lp_observed_result_info,(gecode_opt_handle h,gecode_opt_lp_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto observed=get<O::LpObservedResult>(h,Kind::LpObservedResult);const auto& r=observed->result;
  *out={};out->struct_size=sizeof(*out);out->result={r.model_id,r.revision,static_cast<uint64_t>(r.active_variables.size()),termination(r.termination),
    guarantee(r.guarantee),r.has_solution(),r.solution_validated,r.start_submitted,0,r.elapsed_seconds};
  if(observed->observations){const auto& d=*observed->observations;out->has_observations=1;out->model_id=d.id();out->revision=d.revision();out->row_slots=d.rows().size();out->column_slots=d.columns().size();})
API(lp_observed_result_metadata,(gecode_opt_handle h,gecode_opt_lp_metadata_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto data=lp_observations(h);const auto& m=data->metadata();*out={};out->struct_size=sizeof(*out);
  out->dual_feasibility=m.checks.dual_feasibility;out->stationarity=m.checks.stationarity;out->complementarity=m.checks.complementarity;
  out->objective_gap=m.checks.objective_gap;out->primal_check_tolerance=m.primal_check_tolerance;
  out->backend_primal_tolerance=optional(m.backend_primal_tolerance);out->backend_dual_tolerance=optional(m.backend_dual_tolerance))
API(lp_observed_result_group,(gecode_opt_handle h,int32_t field,gecode_opt_lp_group_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto data=lp_observations(h);const auto& g=lp_group(*data,field);
  *out={};out->struct_size=sizeof(*out);out->state=lp_state(g.state);out->reason=lp_reason(g.reason))
API(lp_observed_result_checks,(gecode_opt_handle h,gecode_opt_lp_checks_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto data=lp_observations(h);const auto& c=data->checks();*out={};out->struct_size=sizeof(*out);
  out->primal_valid=c.primal_valid;out->dual_signs_valid=c.dual_signs_valid;out->stationarity_valid=c.stationarity_valid;
  out->complementarity_valid=c.complementarity_valid;out->gap_valid=c.gap_valid;out->accepted=c.accepted;
  out->max_dual_sign_violation=optional(c.max_dual_sign_violation);out->max_stationarity=optional(c.max_stationarity);
  out->max_complementarity=optional(c.max_complementarity);out->dual_objective_estimate=optional(c.dual_objective_estimate);out->normalized_gap=optional(c.normalized_gap))
API(lp_observed_result_row,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_lp_row_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto data=lp_observations(h);*out=lp_row(data->row(row(id))))
API(lp_observed_result_column,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_lp_column_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto data=lp_observations(h);*out=lp_column(data->column(variable(id))))
API(lp_observed_result_rows,(gecode_opt_handle h,gecode_opt_lp_row_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("LP row element size does not match ABI v1");auto data=lp_observations(h);
  if(output_array(buffer,capacity,needed,data->rows().size()))for(std::size_t i=0;i<data->rows().size();++i)buffer[i]=lp_row(data->rows()[i]))
API(lp_observed_result_columns,(gecode_opt_handle h,gecode_opt_lp_column_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("LP column element size does not match ABI v1");auto data=lp_observations(h);
  if(output_array(buffer,capacity,needed,data->columns().size()))for(std::size_t i=0;i<data->columns().size();++i)buffer[i]=lp_column(data->columns()[i]))
API(lp_observed_result_text,(gecode_opt_handle h,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  auto d=lp_observations(h);const std::string* value=nullptr;
  switch(field){case GECODE_OPT_LP_BACKEND_NAME:value=&d->metadata().backend;break;case GECODE_OPT_LP_BACKEND_VERSION:value=&d->metadata().backend_version;break;
    case GECODE_OPT_LP_PRIMAL_MESSAGE:value=&d->primal_rows().message;break;case GECODE_OPT_LP_DUAL_MESSAGE:value=&d->dual_point().message;break;
    case GECODE_OPT_LP_BASIS_MESSAGE:value=&d->basis().message;break;case GECODE_OPT_LP_CHECK_MESSAGE:value=&d->checks().message;break;
    default:argument("unknown LP observation text selector");}output_text(*value,buffer,capacity,needed))

namespace {
std::vector<std::optional<O::LpBasisStatus>> basis_status_input(const int32_t* values,uint64_t n) {
  const auto size=count(n,values);length<std::optional<O::LpBasisStatus>>(n);
  std::vector<std::optional<O::LpBasisStatus>> out;out.reserve(size);
  for(std::size_t i=0;i<size;++i)switch(values[i]) {
    case -1:out.push_back(std::nullopt);break;
    case GECODE_OPT_LP_BASIS_LOWER:out.push_back(O::LpBasisStatus::Lower);break;
    case GECODE_OPT_LP_BASIS_BASIC:out.push_back(O::LpBasisStatus::Basic);break;
    case GECODE_OPT_LP_BASIS_UPPER:out.push_back(O::LpBasisStatus::Upper);break;
    case GECODE_OPT_LP_BASIS_ZERO:out.push_back(O::LpBasisStatus::Zero);break;
    case GECODE_OPT_LP_BASIS_NONBASIC_UNSPECIFIED:out.push_back(O::LpBasisStatus::NonbasicUnspecified);break;
    default:argument("unknown LP basis status; only -1 denotes an inactive slot");
  }
  return out;
}
int32_t basis_origin(O::LpBasisOrigin origin) {switch(origin) {
  case O::LpBasisOrigin::Caller:return GECODE_OPT_BASIS_CALLER;
  case O::LpBasisOrigin::Observations:return GECODE_OPT_BASIS_OBSERVATIONS;
}argument("unknown LP basis origin");}
int32_t basis_submission(O::LpBasisSubmissionState state) {switch(state) {
  case O::LpBasisSubmissionState::NotAttempted:return GECODE_OPT_BASIS_NOT_ATTEMPTED;
  case O::LpBasisSubmissionState::Accepted:return GECODE_OPT_BASIS_ACCEPTED;
  case O::LpBasisSubmissionState::Repaired:return GECODE_OPT_BASIS_REPAIRED;
  case O::LpBasisSubmissionState::Rejected:return GECODE_OPT_BASIS_REJECTED;
  case O::LpBasisSubmissionState::Interrupted:return GECODE_OPT_BASIS_INTERRUPTED;
}argument("unknown LP basis submission state");}
uint64_t own_basis(std::shared_ptr<const O::LpBasis> basis) {
  if(!basis)throw ApiError{GECODE_OPT_NO_BASIS,"result has no requested LP basis"};
  auto box=std::make_shared<BasisBox>();box->basis=std::move(basis);return put(Kind::Basis,std::move(box));
}
O::LpBasisSolveOptions basis_options(uint64_t h,const gecode_opt_lp_options_v1* input) {
  O::LpBasisSolveOptions out;out.observations=lp_options(input);out.basis=get<BasisBox>(h,Kind::Basis)->basis;
  out.validate();return out;
}
}
API(basis_from_observed,(gecode_opt_handle h,gecode_opt_handle* out),
  required(out);*out=0;*out=own_basis(O::make_lp_basis(*lp_observations(h))))
API(basis_from_model,(gecode_opt_handle h,const int32_t* columns,uint64_t column_count,const int32_t* rows,uint64_t row_count,gecode_opt_handle* out),
  required(out);*out=0;count(column_count,columns);count(row_count,rows);
  O::LpBasisData data;data.columns=basis_status_input(columns,column_count);data.rows=basis_status_input(rows,row_count);
  data.source=snapshot(h);*out=own_basis(O::make_lp_basis(data)))
API(basis_destroy,(gecode_opt_handle h),destroy(h,Kind::Basis))
API(basis_info,(gecode_opt_handle h,gecode_opt_basis_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<BasisBox>(h,Kind::Basis)->basis;*out={};out->struct_size=sizeof(*out);
  out->model_id=b->id();out->revision=b->revision();out->row_slots=b->rows().size();out->column_slots=b->columns().size();out->origin=basis_origin(b->origin()))
API(basis_statuses,(gecode_opt_handle h,int32_t entity_kind,int32_t* buffer,uint64_t capacity,uint64_t* needed),
  if(entity_kind!=GECODE_OPT_VARIABLE_ID&&entity_kind!=GECODE_OPT_ROW_ID)argument("basis statuses require VARIABLE_ID or ROW_ID selector");
  auto b=get<BasisBox>(h,Kind::Basis)->basis;const auto& values=entity_kind==GECODE_OPT_VARIABLE_ID?b->columns():b->rows();
  if(output_array(buffer,capacity,needed,values.size()))for(std::size_t i=0;i<values.size();++i)buffer[i]=values[i]?lp_basis(*values[i]):-1)
API(basis_row,(gecode_opt_handle h,gecode_opt_id id,int32_t* out),
  required(out);const auto key=row(id);auto b=get<BasisBox>(h,Kind::Basis)->basis;
  if(key.model_id!=b->id()||key.id>=b->rows().size()||!b->rows()[key.id])throw O::ModelError("basis row is foreign, absent, or deleted");
  *out=lp_basis(*b->rows()[key.id]))
API(basis_column,(gecode_opt_handle h,gecode_opt_id id,int32_t* out),
  required(out);const auto key=variable(id);auto b=get<BasisBox>(h,Kind::Basis)->basis;
  if(key.model_id!=b->id()||key.id>=b->columns().size()||!b->columns()[key.id])throw O::ModelError("basis variable is foreign, absent, or deleted");
  *out=lp_basis(*b->columns()[key.id]))
API(solve_lp_with_basis,(gecode_opt_handle h,gecode_opt_handle basis,const gecode_opt_lp_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=basis_options(basis,input);auto model=snapshot(h);
  account_preparation(opts.observations.solve,start);
  *out=put(Kind::BasisSolveResult,std::make_shared<O::LpBasisSolveResult>(O::solve_lp_with_basis(model,opts))))
API(session_solve_lp_with_basis,(gecode_opt_handle session,gecode_opt_handle h,gecode_opt_handle basis,const gecode_opt_lp_options_v1* input,gecode_opt_handle* out),
  const auto start=Clock::now();required(out);*out=0;auto opts=basis_options(basis,input);auto model=snapshot(h);auto box=get<SessionBox>(session,Kind::Session);
  std::lock_guard<std::mutex> lock(box->mutex);account_preparation(opts.observations.solve,start);
  *out=put(Kind::BasisSolveResult,std::make_shared<O::LpBasisSolveResult>(box->session.solve_lp_with_basis(model,opts))))
API(basis_result_destroy,(gecode_opt_handle h),destroy(h,Kind::BasisSolveResult))
API(basis_result_info,(gecode_opt_handle h,gecode_opt_basis_result_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<O::LpBasisSolveResult>(h,Kind::BasisSolveResult);const auto& r=b->observed.result;
  *out={};out->struct_size=sizeof(*out);out->result={r.model_id,r.revision,static_cast<uint64_t>(r.active_variables.size()),termination(r.termination),
    guarantee(r.guarantee),r.has_solution(),r.solution_validated,r.start_submitted,0,r.elapsed_seconds};
  if(b->requested_basis){out->has_requested_basis=1;out->requested_model_id=b->requested_basis->id();out->requested_revision=b->requested_basis->revision();}
  out->state=basis_submission(b->submission.state);out->backend_attempted=b->submission.backend_attempted;
  out->has_statuses_changed=b->submission.statuses_changed.has_value();if(b->submission.statuses_changed)out->statuses_changed=*b->submission.statuses_changed)
API(basis_result_message,(gecode_opt_handle h,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<O::LpBasisSolveResult>(h,Kind::BasisSolveResult);output_text(b->submission.message,buffer,capacity,needed))
API(basis_result_copy_observed,(gecode_opt_handle h,gecode_opt_handle* out),
  required(out);*out=0;auto b=get<O::LpBasisSolveResult>(h,Kind::BasisSolveResult);
  *out=put(Kind::LpObservedResult,std::make_shared<O::LpObservedResult>(b->observed)))
API(basis_result_copy_basis,(gecode_opt_handle h,gecode_opt_handle* out),
  required(out);*out=0;auto b=get<O::LpBasisSolveResult>(h,Kind::BasisSolveResult);*out=own_basis(b->requested_basis))


namespace {
O::ScenarioId scenario_id(gecode_opt_scenario_id id){return {id.batch_id,id.index};}
gecode_opt_scenario_id scenario_id(O::ScenarioId id){return {id.batch_id,id.index};}
const O::ScenarioBatch& scenario_batch(const O::ScenarioBatchResult& result){
  if(!result.batch)throw O::ModelError("scenario batch was not admitted");return *result.batch;
}
const O::ScenarioOutcome& scenario_outcome(const O::ScenarioBatchResult& result,gecode_opt_scenario_id id){
  scenario_batch(result).definition(scenario_id(id));return at(result.outcomes,id.index);
}
gecode_opt_session_statistics_v1 scenario_statistics(const O::SessionStatistics& s){
  return {s.solve_calls,s.model_loads,s.incremental_updates,s.unchanged_models,s.basis_warm_starts,s.incumbent_starts};
}
int32_t scenario_completion(O::ScenarioBatchCompletion value){switch(value){
  case O::ScenarioBatchCompletion::Rejected:return GECODE_OPT_SCENARIO_REJECTED;
  case O::ScenarioBatchCompletion::Interrupted:return GECODE_OPT_SCENARIO_INTERRUPTED;
  case O::ScenarioBatchCompletion::Complete:return GECODE_OPT_SCENARIO_COMPLETE;
}argument("unknown scenario completion");}
int32_t scenario_state(O::ScenarioRunState value){switch(value){
  case O::ScenarioRunState::NotStarted:return GECODE_OPT_SCENARIO_NOT_STARTED;
  case O::ScenarioRunState::Attempted:return GECODE_OPT_SCENARIO_ATTEMPTED;
}argument("unknown scenario run state");}
O::ScenarioBatchOptions scenario_options(const gecode_opt_scenario_options_v1* input){
  O::ScenarioBatchOptions result;if(!input)return result;
  if(input->struct_size!=sizeof(*input))argument("scenario options struct_size does not match v1");
  if(input->reserved||input->reserved_flags)argument("scenario options reserved fields must be zero");
  result.solve=options(&input->solve);
  switch(input->reuse){case GECODE_OPT_SCENARIO_AUTOMATIC:result.reuse=O::ScenarioReuse::Automatic;break;
    case GECODE_OPT_SCENARIO_COLD:result.reuse=O::ScenarioReuse::Cold;break;default:argument("unknown scenario reuse");}
  result.max_scenarios=length<O::ScenarioDefinition>(input->max_scenarios);
  result.max_patch_entries=length<O::Term>(input->max_patch_entries);
  result.max_saved_value_slots=length<double>(input->max_saved_value_slots);
  if(input->max_work>std::numeric_limits<std::size_t>::max())argument("scenario work limit exceeds size_t");
  result.max_work=static_cast<std::size_t>(input->max_work);result.validate();return result;
}
void scenario_presence(int32_t present){if(present!=0&&present!=1)argument("scenario presence flag must be zero or one");}
void scenario_bounds(const gecode_opt_scenario_bounds_v1& input){
  if(input.struct_size!=sizeof(input)||input.reserved)argument("invalid scenario bound size/reserved");
  scenario_presence(input.has_lower);scenario_presence(input.has_upper);
}
std::optional<double> scenario_side(int32_t present,double value){return present?std::optional<double>(value):std::nullopt;}
gecode_opt_scenario_bounds_v1 scenario_bound(gecode_opt_id id,std::optional<double> lower,std::optional<double> upper){
  return {sizeof(gecode_opt_scenario_bounds_v1),0,id,lower.has_value(),upper.has_value(),lower.value_or(0),upper.value_or(0)};
}
std::vector<O::ScenarioDefinition> scenario_definitions(const gecode_opt_scenario_definition_v1* input,std::size_t n){
  std::vector<O::ScenarioDefinition> out;out.reserve(n);
  for(std::size_t i=0;i<n;++i){const auto& item=input[i];O::ScenarioDefinition def;
    def.name=bulk_name(item.name);
    // Semantic nonfinite/duplicate/foreign patches are rejected by the owning
    // C++ coordinator, producing an attributed Rejected batch, not API misuse.
    const auto nt=count(item.objective_count,item.objective_coefficients);def.objective_coefficients.reserve(nt);
    for(std::size_t j=0;j<nt;++j){const auto& t=item.objective_coefficients[j];def.objective_coefficients.push_back({variable(t.variable),t.coefficient});}
    def.objective_offset=scenario_side(item.objective_offset.present,item.objective_offset.value);
    const auto nv=count(item.variable_count,item.variable_bounds);def.variable_bounds.reserve(nv);
    for(std::size_t j=0;j<nv;++j){const auto& b=item.variable_bounds[j];scenario_bounds(b);
      def.variable_bounds.push_back({variable(b.entity),scenario_side(b.has_lower,b.lower),scenario_side(b.has_upper,b.upper)});}
    const auto nr=count(item.row_count,item.row_bounds);def.row_bounds.reserve(nr);
    for(std::size_t j=0;j<nr;++j){const auto& b=item.row_bounds[j];scenario_bounds(b);
      def.row_bounds.push_back({row(b.entity),scenario_side(b.has_lower,b.lower),scenario_side(b.has_upper,b.upper)});}
    out.push_back(std::move(def));
  }return out;
}
}
API(scenario_options_default,(gecode_opt_scenario_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::ScenarioBatchOptions o;*out={};out->struct_size=sizeof(*out);out->solve=default_options();
  out->reuse=GECODE_OPT_SCENARIO_AUTOMATIC;out->max_scenarios=o.max_scenarios;out->max_patch_entries=o.max_patch_entries;
  out->max_saved_value_slots=o.max_saved_value_slots;out->max_work=o.max_work)
API(solve_scenarios,(gecode_opt_handle h,const gecode_opt_scenario_definition_v1* input,uint64_t n,uint64_t element_size,const gecode_opt_scenario_options_v1* option_input,gecode_opt_handle* output),
  const auto start=Clock::now();required(output);*output=0;
  if(element_size!=sizeof(*input))argument("scenario definition element_size does not match v1");
  const auto size=count(n,input);length<O::ScenarioDefinition>(n);auto opts=scenario_options(option_input);const auto total_seconds=opts.solve.time_limit_seconds;
  std::shared_ptr<O::ScenarioBatchResult> result;
  {auto model=snapshot(h);
  bool capped=size>opts.max_scenarios;std::size_t patches=0;
  // Preflight counts and versioned headers before allocating owned patch arrays.
  if(!capped)for(std::size_t i=0;i<size;++i){const auto& item=input[i];
    if(item.struct_size!=sizeof(item)||item.reserved)argument("invalid scenario definition size/reserved");
    scenario_presence(item.objective_offset.present);if(item.objective_offset.reserved)argument("scenario offset reserved must be zero");
    const auto nt=count(item.objective_count,item.objective_coefficients);length<O::Term>(item.objective_count);
    const auto nv=count(item.variable_count,item.variable_bounds);length<O::ScenarioVariableBounds>(item.variable_count);
    const auto nr=count(item.row_count,item.row_bounds);length<O::ScenarioRowBounds>(item.row_count);
    for(auto add:{nt,nv,nr}){if(add>opts.max_patch_entries-patches){capped=true;break;}patches+=add;}
    if(capped)break;
  }
  if(capped){result=std::make_shared<O::ScenarioBatchResult>();result->model_id=model.model_id;result->revision=model.revision;
    result->stop_reason=O::Termination::MemoryLimit;result->message="scenario binding input storage limit";
    if(opts.solve.cancellation&&opts.solve.cancellation->cancelled()){
      result->stop_reason=O::Termination::Cancelled;result->message="scenario binding cancelled before input copies";
    }else if(std::isfinite(total_seconds)&&std::chrono::duration<double>(Clock::now()-start).count()>=total_seconds){
      result->stop_reason=O::Termination::TimeLimit;result->message="scenario binding deadline reached before input copies";
    }}
  else {auto definitions=scenario_definitions(input,size);account_preparation(opts.solve,start);
    result=std::make_shared<O::ScenarioBatchResult>(O::solve_scenarios(model,definitions,opts));}}
  // The binding's temporary source and copied input arrays are now gone. Keep
  // timely individual histories if only whole-call cleanup exceeded the budget.
  if(result->completion==O::ScenarioBatchCompletion::Complete){
    std::optional<O::Termination> stopped;
    if(opts.solve.cancellation&&opts.solve.cancellation->cancelled())stopped=O::Termination::Cancelled;
    else if(std::isfinite(total_seconds)&&std::chrono::duration<double>(Clock::now()-start).count()>=total_seconds)stopped=O::Termination::TimeLimit;
    if(stopped){result->completion=O::ScenarioBatchCompletion::Interrupted;result->stop_reason=stopped;
      result->message="Whole scenario binding budget stopped during final input cleanup";}
  }
  result->elapsed_seconds=std::chrono::duration<double>(Clock::now()-start).count();
  *output=put(Kind::ScenarioBatchResult,std::move(result)))
API(scenario_batch_destroy,(gecode_opt_handle h),destroy(h,Kind::ScenarioBatchResult))
API(scenario_batch_info,(gecode_opt_handle h,gecode_opt_scenario_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto r=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);*out={};out->struct_size=sizeof(*out);
  out->model_id=r->model_id;out->revision=r->revision;out->has_batch=bool(r->batch);out->completion=scenario_completion(r->completion);
  if(r->batch){out->batch_id=r->batch->id();out->scenario_count=r->batch->size();}out->outcome_count=r->outcomes.size();
  out->has_stop_reason=r->stop_reason.has_value();if(r->stop_reason)out->stop_reason=termination(*r->stop_reason);
  out->has_offending_scenario=r->offending_scenario.has_value();if(r->offending_scenario)out->offending_scenario=*r->offending_scenario;
  out->all_resolved=r->all_resolved();out->attempted=r->attempted;out->resolved=r->resolved;out->work=r->work;
  out->elapsed_seconds=r->elapsed_seconds;out->reuse_statistics=scenario_statistics(r->reuse_statistics))
API(scenario_batch_id,(gecode_opt_handle h,uint64_t index,gecode_opt_scenario_id* out),
  required(out);auto r=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);
  if(index>std::numeric_limits<std::size_t>::max())argument("scenario index exceeds size_t");
  *out=scenario_id(scenario_batch(*r).scenario(static_cast<std::size_t>(index))))
API(scenario_batch_outcome,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_scenario_outcome_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& o=scenario_outcome(*b,id);
  *out={};out->struct_size=sizeof(*out);out->scenario=scenario_id(o.scenario);out->state=scenario_state(o.state);
  out->has_result=o.result.has_value();out->has_check=o.check.has_value();out->reuse_delta=scenario_statistics(o.reuse_delta);out->elapsed_seconds=o.elapsed_seconds;
  if(o.result){const auto& r=*o.result;out->result={r.model_id,r.revision,static_cast<uint64_t>(r.active_variables.size()),
    termination(r.termination),guarantee(r.guarantee),r.has_solution(),r.solution_validated,r.start_submitted,0,r.elapsed_seconds};})
API(scenario_batch_check,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_scenario_check_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& o=scenario_outcome(*b,id);
  *out={};out->struct_size=sizeof(*out);out->has_check=o.check.has_value();if(o.check){const auto& c=*o.check;
    out->identity_valid=c.identity_valid;out->candidate_examined=c.candidate_examined;
    if(c.candidate_examined){out->objective_matches=c.objective_matches;out->exact_witness_validated=c.exact_witness_validated;
      const auto& v=c.validation;out->validation={v.valid,v.model_valid,static_cast<uint64_t>(v.violated_globals),
        v.max_bound_violation,v.max_row_violation,v.max_integrality_violation,v.max_indicator_violation,optional(v.objective)};}})
API(scenario_batch_copy_result,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_handle* out),
  required(out);*out=0;auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& o=scenario_outcome(*b,id);
  if(!o.result)throw ApiError{GECODE_OPT_NO_SOLUTION,"scenario has no attempted result"};
  *out=put(Kind::Result,std::make_shared<O::SolveResult>(*o.result)))
API(scenario_batch_map,(gecode_opt_handle h,gecode_opt_id original,gecode_opt_id* out),
  required(out);auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& source=scenario_batch(*b);
  if(original.kind==GECODE_OPT_VARIABLE_ID)*out=identifier(source.map(variable(original)));
  else if(original.kind==GECODE_OPT_ROW_ID)*out=identifier(source.map(row(original)));else argument("scenario map requires variable or row ID"))
API(scenario_batch_value,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_id original,double* out),
  required(out);auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& o=scenario_outcome(*b,id);
  const auto v=variable(original);scenario_batch(*b).map(v);
  if(!o.result||!o.result->has_solution())throw ApiError{GECODE_OPT_NO_SOLUTION,"scenario has no validated solution"};
  *out=b->value(scenario_id(id),v))
API(scenario_batch_message,(gecode_opt_handle h,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);output_text(b->message,buffer,capacity,needed))
API(scenario_batch_text,(gecode_opt_handle h,gecode_opt_scenario_id id,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& def=scenario_batch(*b).definition(scenario_id(id));
  switch(field){case GECODE_OPT_SCENARIO_NAME:output_text(def.name,buffer,capacity,needed);break;
    case GECODE_OPT_SCENARIO_VALIDATION_MESSAGE:{const auto& o=scenario_outcome(*b,id);
      output_text(o.check&&o.check->candidate_examined?o.check->validation.message:std::string(),buffer,capacity,needed);break;}
    default:argument("unknown scenario text selector");})
API(scenario_batch_definition,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_scenario_definition_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& d=scenario_batch(*b).definition(scenario_id(id));
  *out={sizeof(*out),0,static_cast<uint64_t>(d.objective_coefficients.size()),static_cast<uint64_t>(d.variable_bounds.size()),
    static_cast<uint64_t>(d.row_bounds.size()),optional(d.objective_offset)})
API(scenario_batch_objective,(gecode_opt_handle h,gecode_opt_scenario_id id,gecode_opt_term* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& d=scenario_batch(*b).definition(scenario_id(id));
  if(output_array(buffer,capacity,needed,d.objective_coefficients.size()))for(std::size_t i=0;i<d.objective_coefficients.size();++i)
    buffer[i]={identifier(d.objective_coefficients[i].variable),d.objective_coefficients[i].coefficient})
API(scenario_batch_bounds,(gecode_opt_handle h,gecode_opt_scenario_id id,int32_t kind,gecode_opt_scenario_bounds_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("scenario bounds element size does not match v1");
  auto b=get<O::ScenarioBatchResult>(h,Kind::ScenarioBatchResult);const auto& d=scenario_batch(*b).definition(scenario_id(id));
  if(kind==GECODE_OPT_VARIABLE_ID){if(output_array(buffer,capacity,needed,d.variable_bounds.size()))for(std::size_t i=0;i<d.variable_bounds.size();++i){
    const auto& item=d.variable_bounds[i];buffer[i]=scenario_bound(identifier(item.variable),item.lower,item.upper);}}
  else if(kind==GECODE_OPT_ROW_ID){if(output_array(buffer,capacity,needed,d.row_bounds.size()))for(std::size_t i=0;i<d.row_bounds.size();++i){
    const auto& item=d.row_bounds[i];buffer[i]=scenario_bound(identifier(item.row),item.lower,item.upper);}}
  else argument("scenario bounds requires variable or row kind"))

#ifdef GECODE_OPTIMIZE_TEST_EVIDENCE_BINDING
extern "C" void gecode_opt_test_evidence_binding_checkpoint(void);
#endif
namespace {
void evidence_binding_checkpoint(){
#ifdef GECODE_OPTIMIZE_TEST_EVIDENCE_BINDING
  gecode_opt_test_evidence_binding_checkpoint();
#endif
}
struct EvidenceBox {
  O::LpEvidenceResult result;
  bool cleanup_stopped=false;
};
struct EvidenceStageBox {std::shared_ptr<const EvidenceBox> owner;std::size_t index;};
const O::LpEvidence& evidence(const EvidenceBox& box){
  if(!box.result.evidence)throw ApiError{GECODE_OPT_NO_EVIDENCE,"LP evidence artifact is unavailable"};return *box.result.evidence;
}
O::LpEvidenceOptions evidence_options(const gecode_opt_evidence_options_v1* input){
  O::LpEvidenceOptions out;if(!input)return out;
  if(input->struct_size!=sizeof(*input)||input->reserved||input->reserved_flags)argument("invalid LP evidence options size/reserved");
  out.solve=options(&input->solve);
  switch(input->request){case GECODE_OPT_EVIDENCE_AUTOMATIC:out.request=O::LpEvidenceRequest::Automatic;break;
    case GECODE_OPT_EVIDENCE_PRIMAL_RAY:out.request=O::LpEvidenceRequest::PrimalRay;break;
    case GECODE_OPT_EVIDENCE_FARKAS:out.request=O::LpEvidenceRequest::Farkas;break;
    case GECODE_OPT_EVIDENCE_BOTH:out.request=O::LpEvidenceRequest::Both;break;default:argument("unknown LP evidence request");}
  out.checks={input->recession,input->stationarity,input->minimum_improvement,input->minimum_contradiction};
  const auto limit=[](uint64_t value){if(value>std::numeric_limits<std::size_t>::max())argument("LP evidence limit exceeds size_t");return static_cast<std::size_t>(value);};
  out.limits.max_auxiliary_variables=limit(input->max_auxiliary_variables);out.limits.max_auxiliary_rows=limit(input->max_auxiliary_rows);
  out.limits.max_auxiliary_nonzeros=limit(input->max_auxiliary_nonzeros);out.limits.max_retained_slots=limit(input->max_retained_slots);
  out.limits.max_work=limit(input->max_work);out.limits.max_auxiliary_solves=limit(input->max_auxiliary_solves);out.validate();return out;
}
int32_t evidence_state(O::LpEvidenceState value){switch(value){
  case O::LpEvidenceState::NotRequested:return GECODE_OPT_EVIDENCE_NOT_REQUESTED;
  case O::LpEvidenceState::Available:return GECODE_OPT_EVIDENCE_AVAILABLE;
  case O::LpEvidenceState::Unavailable:return GECODE_OPT_EVIDENCE_UNAVAILABLE;
  case O::LpEvidenceState::Rejected:return GECODE_OPT_EVIDENCE_REJECTED;
}argument("unknown LP evidence state");}
int32_t evidence_reason(O::LpEvidenceReason value){switch(value){
  case O::LpEvidenceReason::None:return GECODE_OPT_EVIDENCE_REASON_NONE;
  case O::LpEvidenceReason::NotRequested:return GECODE_OPT_EVIDENCE_REASON_NOT_REQUESTED;
  case O::LpEvidenceReason::Unsupported:return GECODE_OPT_EVIDENCE_REASON_UNSUPPORTED;
  case O::LpEvidenceReason::NoFeasibleBase:return GECODE_OPT_EVIDENCE_REASON_NO_FEASIBLE_BASE;
  case O::LpEvidenceReason::NoImprovingDirection:return GECODE_OPT_EVIDENCE_REASON_NO_IMPROVEMENT;
  case O::LpEvidenceReason::NoContradiction:return GECODE_OPT_EVIDENCE_REASON_NO_CONTRADICTION;
  case O::LpEvidenceReason::Stopped:return GECODE_OPT_EVIDENCE_REASON_STOPPED;
  case O::LpEvidenceReason::InvalidBackendData:return GECODE_OPT_EVIDENCE_REASON_INVALID_BACKEND;
  case O::LpEvidenceReason::FailedOriginalChecks:return GECODE_OPT_EVIDENCE_REASON_FAILED_CHECKS;
  case O::LpEvidenceReason::InconsistentEvidence:return GECODE_OPT_EVIDENCE_REASON_INCONSISTENT;
  case O::LpEvidenceReason::InvalidModel:return GECODE_OPT_EVIDENCE_REASON_INVALID_MODEL;
  case O::LpEvidenceReason::ResourceLimit:return GECODE_OPT_EVIDENCE_REASON_RESOURCE_LIMIT;
  case O::LpEvidenceReason::AllocationFailure:return GECODE_OPT_EVIDENCE_REASON_ALLOCATION;
}argument("unknown LP evidence reason");}
int32_t evidence_completion(O::LpEvidenceCompletion value){switch(value){
  case O::LpEvidenceCompletion::Complete:return GECODE_OPT_EVIDENCE_COMPLETE;
  case O::LpEvidenceCompletion::Interrupted:return GECODE_OPT_EVIDENCE_INTERRUPTED;
  case O::LpEvidenceCompletion::Rejected:return GECODE_OPT_EVIDENCE_ANALYSIS_REJECTED;
}argument("unknown LP evidence completion");}
int32_t evidence_phase(O::LpEvidencePhase value){switch(value){
  case O::LpEvidencePhase::FeasibleBase:return GECODE_OPT_EVIDENCE_FEASIBLE_BASE;
  case O::LpEvidencePhase::Recession:return GECODE_OPT_EVIDENCE_RECESSION;
  case O::LpEvidencePhase::Farkas:return GECODE_OPT_EVIDENCE_FARKAS_PHASE;
}argument("unknown LP evidence phase");}
int32_t evidence_side(O::LpEvidenceSide value){switch(value){case O::LpEvidenceSide::Lower:return GECODE_OPT_EVIDENCE_LOWER;
  case O::LpEvidenceSide::Upper:return GECODE_OPT_EVIDENCE_UPPER;}argument("unknown LP evidence side");}
int32_t evidence_column(O::LpEvidenceColumnKind value){switch(value){
  case O::LpEvidenceColumnKind::SourceVariable:return GECODE_OPT_EVIDENCE_SOURCE_VARIABLE;
  case O::LpEvidenceColumnKind::RowSide:return GECODE_OPT_EVIDENCE_ROW_SIDE;
  case O::LpEvidenceColumnKind::VariableSide:return GECODE_OPT_EVIDENCE_VARIABLE_SIDE;
}argument("unknown LP evidence column kind");}
const O::LpEvidenceGroup& evidence_group(const EvidenceBox& box,int32_t field){
  const auto& data=evidence(box);switch(field){case GECODE_OPT_EVIDENCE_PRIMAL_GROUP:return data.primal_ray();
    case GECODE_OPT_EVIDENCE_FARKAS_GROUP:return data.farkas();default:argument("unknown LP evidence group");}
}
bool evidence_available(const EvidenceBox& box,int32_t field){return evidence_group(box,field).state==O::LpEvidenceState::Available&&!box.cleanup_stopped;}
void require_evidence_available(const EvidenceBox& box,int32_t field){if(!evidence_available(box,field))throw ApiError{GECODE_OPT_NO_EVIDENCE,"LP evidence group is not Available"};}
std::size_t evidence_slot(const O::LpEvidence& data,gecode_opt_id id){
  const auto& source=data.source();
  if(id.kind==GECODE_OPT_VARIABLE_ID){const auto v=variable(id);
    if(v.model_id!=data.id()||v.id>=source.variables.size()||!source.variables[v.id].active)throw O::ModelError("LP evidence variable is foreign, absent or deleted");return static_cast<std::size_t>(v.id);}
  if(id.kind==GECODE_OPT_ROW_ID){const auto r=row(id);
    if(r.model_id!=data.id()||r.id>=source.rows.size()||!source.rows[r.id].active)throw O::ModelError("LP evidence row is foreign, absent or deleted");return static_cast<std::size_t>(r.id);}
  argument("LP evidence source must be variable or row");
}
gecode_opt_validation_info_v1 evidence_validation(const O::ValidationReport& v){return {v.valid,v.model_valid,static_cast<uint64_t>(v.violated_globals),
  v.max_bound_violation,v.max_row_violation,v.max_integrality_violation,v.max_indicator_violation,optional(v.objective)};}
gecode_opt_evidence_slot_v1 evidence_slot_record(const O::LpEvidence& data,int32_t kind,std::size_t index){
  gecode_opt_evidence_slot_v1 out{};out.struct_size=sizeof(out);const bool column=kind==GECODE_OPT_VARIABLE_ID;
  const auto& source=data.source();const auto& primal=data.primal_data();const auto& farkas=data.farkas_data();
  out.source={data.id(),static_cast<uint64_t>(index),static_cast<uint32_t>(kind),0};
  double lower,upper;
  if(column){const auto& v=at(source.variables,index);out.active=v.active;lower=v.lower;upper=v.upper;
    if(out.active&&index<primal.base_point.size())out.base_value=optional(primal.base_point[index]);
    if(out.active&&index<primal.direction.size())out.direction=optional(primal.direction[index]);
  }else{const auto& r=at(source.rows,index);out.active=r.active;lower=r.lower;upper=r.upper;
    if(out.active&&index<primal.row_direction.size())out.direction=optional(primal.row_direction[index]);}
  const auto& entries=column?farkas.columns:farkas.rows;
  if(out.active&&index<entries.size()&&entries[index].active){const auto& entry=entries[index];
    out.multiplier=optional(entry.multiplier);out.contribution=optional(entry.contribution);out.has_side=entry.side.has_value();
    if(entry.side){out.side=evidence_side(*entry.side);out.selected_bound=optional(*entry.side==O::LpEvidenceSide::Lower?lower:upper);}}
  return out;
}
const O::LpEvidenceStage& evidence_stage(const EvidenceStageBox& box){return at(evidence(*box.owner).stages(),box.index);}
}
API(evidence_options_default,(gecode_opt_evidence_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::LpEvidenceOptions o;*out={};out->struct_size=sizeof(*out);out->solve=default_options();out->request=GECODE_OPT_EVIDENCE_AUTOMATIC;
  out->recession=o.checks.recession;out->stationarity=o.checks.stationarity;out->minimum_improvement=o.checks.minimum_improvement;out->minimum_contradiction=o.checks.minimum_contradiction;
  out->max_auxiliary_variables=o.limits.max_auxiliary_variables;out->max_auxiliary_rows=o.limits.max_auxiliary_rows;out->max_auxiliary_nonzeros=o.limits.max_auxiliary_nonzeros;
  out->max_retained_slots=o.limits.max_retained_slots;out->max_work=o.limits.max_work;out->max_auxiliary_solves=o.limits.max_auxiliary_solves)
API(analyze_lp_evidence,(gecode_opt_handle h,const gecode_opt_evidence_options_v1* input,gecode_opt_handle* output),
  const auto start=Clock::now();required(output);*output=0;auto opts=evidence_options(input);const auto seconds=opts.solve.time_limit_seconds;
  auto box=std::make_shared<EvidenceBox>();{auto source=snapshot(h);account_preparation(opts.solve,start);box->result=O::analyze_lp_evidence(source,opts);}
  evidence_binding_checkpoint();
  std::optional<O::Termination> stopped;
  if(opts.solve.cancellation&&opts.solve.cancellation->cancelled())stopped=O::Termination::Cancelled;
  else if(std::isfinite(seconds)&&std::chrono::duration<double>(Clock::now()-start).count()>=seconds)stopped=O::Termination::TimeLimit;
  if(stopped){box->cleanup_stopped=true;box->result.completion=O::LpEvidenceCompletion::Interrupted;box->result.stop_reason=stopped;
    box->result.message="Whole LP evidence binding allowance stopped during input cleanup";}
  box->result.elapsed_seconds=std::chrono::duration<double>(Clock::now()-start).count();*output=put(Kind::LpEvidenceResult,std::move(box)))
API(lp_evidence_destroy,(gecode_opt_handle h),destroy(h,Kind::LpEvidenceResult))
API(lp_evidence_info,(gecode_opt_handle h,gecode_opt_evidence_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& r=b->result;
  *out={};out->struct_size=sizeof(*out);out->model_id=r.model_id;out->revision=r.revision;out->has_evidence=bool(r.evidence);
  out->completion=evidence_completion(r.completion);out->has_stop_reason=r.stop_reason.has_value();if(r.stop_reason)out->stop_reason=termination(*r.stop_reason);
  if(r.evidence){out->row_slots=r.evidence->source().rows.size();out->column_slots=r.evidence->source().variables.size();out->stage_count=r.evidence->stages().size();}
  out->attempted_calls=r.attempted_calls;out->work=r.work;out->elapsed_seconds=r.elapsed_seconds)
API(lp_evidence_group,(gecode_opt_handle h,int32_t field,gecode_opt_evidence_group_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& g=evidence_group(*b,field);
  *out={sizeof(*out),0,evidence_state(g.state),evidence_reason(g.reason)};
  if(b->cleanup_stopped&&g.state!=O::LpEvidenceState::NotRequested){out->state=GECODE_OPT_EVIDENCE_UNAVAILABLE;out->reason=GECODE_OPT_EVIDENCE_REASON_STOPPED;})
API(lp_evidence_metadata,(gecode_opt_handle h,gecode_opt_evidence_metadata_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& e=evidence(*b);const auto& t=e.tolerances();
  *out={sizeof(*out),0,t.recession,t.stationarity,t.minimum_improvement,t.minimum_contradiction,e.primal_tolerance()})
API(lp_evidence_primal,(gecode_opt_handle h,gecode_opt_evidence_primal_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& p=evidence(*b).primal_data();
  *out={};out->struct_size=sizeof(*out);out->has_base_check=p.base_check.model_valid;
  if(out->has_base_check)out->base_check=evidence_validation(p.base_check);
  out->direction_scale=optional(p.direction_scale);out->normalized_objective_slope=optional(p.normalized_objective_slope);
  out->max_variable_recession_violation=optional(p.max_variable_recession_violation);out->max_row_recession_violation=optional(p.max_row_recession_violation))
API(lp_evidence_farkas,(gecode_opt_handle h,gecode_opt_evidence_farkas_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& f=evidence(*b).farkas_data();
  *out={sizeof(*out),0,optional(f.multiplier_scale),optional(f.contradiction_margin),optional(f.max_stationarity)})
API(lp_evidence_slot,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_evidence_slot_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& e=evidence(*b);
  *out=evidence_slot_record(e,static_cast<int32_t>(id.kind),evidence_slot(e,id)))
API(lp_evidence_slots,(gecode_opt_handle h,int32_t kind,gecode_opt_evidence_slot_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("LP evidence slot element size does not match v1");
  auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& e=evidence(*b);std::size_t n;
  if(kind==GECODE_OPT_VARIABLE_ID)n=e.source().variables.size();else if(kind==GECODE_OPT_ROW_ID)n=e.source().rows.size();else argument("LP evidence slots require variable/row kind");
  if(output_array(buffer,capacity,needed,n))for(std::size_t i=0;i<n;++i)buffer[i]=evidence_slot_record(e,kind,i))
API(lp_evidence_value,(gecode_opt_handle h,gecode_opt_id id,int32_t field,double* out),
  required(out);auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto v=variable(id);const auto& e=evidence(*b);evidence_slot(e,id);
  if(field!=GECODE_OPT_EVIDENCE_BASE_VALUE&&field!=GECODE_OPT_EVIDENCE_DIRECTION_VALUE)argument("unknown LP evidence value selector");
  require_evidence_available(*b,GECODE_OPT_EVIDENCE_PRIMAL_GROUP);
  *out=field==GECODE_OPT_EVIDENCE_BASE_VALUE?e.base_value(v):e.direction_value(v))
API(lp_evidence_multiplier,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_evidence_slot_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);const auto& e=evidence(*b);const auto slot=evidence_slot(e,id);
  require_evidence_available(*b,GECODE_OPT_EVIDENCE_FARKAS_GROUP);*out=evidence_slot_record(e,static_cast<int32_t>(id.kind),slot))
API(lp_evidence_text,(gecode_opt_handle h,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);
  if(field==GECODE_OPT_EVIDENCE_MESSAGE)output_text(b->result.message,buffer,capacity,needed);
  else if(field==GECODE_OPT_EVIDENCE_BASE_CHECK_MESSAGE)output_text(evidence(*b).primal_data().base_check.message,buffer,capacity,needed);
  else if(field==GECODE_OPT_EVIDENCE_PRIMAL_MESSAGE||field==GECODE_OPT_EVIDENCE_FARKAS_MESSAGE){const auto& g=evidence_group(*b,field==GECODE_OPT_EVIDENCE_PRIMAL_MESSAGE?GECODE_OPT_EVIDENCE_PRIMAL_GROUP:GECODE_OPT_EVIDENCE_FARKAS_GROUP);
    output_text(b->cleanup_stopped&&g.state!=O::LpEvidenceState::NotRequested?b->result.message:g.message,buffer,capacity,needed);}
  else argument("unknown LP evidence text selector"))
API(lp_evidence_copy_stage,(gecode_opt_handle h,uint64_t index,gecode_opt_handle* output),
  required(output);*output=0;auto b=get<EvidenceBox>(h,Kind::LpEvidenceResult);at(evidence(*b).stages(),index);
  auto child=std::make_shared<EvidenceStageBox>();child->owner=std::move(b);child->index=static_cast<std::size_t>(index);
  *output=put(Kind::LpEvidenceStage,std::move(child)))
API(lp_evidence_stage_destroy,(gecode_opt_handle h),destroy(h,Kind::LpEvidenceStage))
API(lp_evidence_stage_info,(gecode_opt_handle h,gecode_opt_evidence_stage_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<EvidenceStageBox>(h,Kind::LpEvidenceStage);const auto& s=evidence_stage(*b);
  *out={};out->struct_size=sizeof(*out);out->index=b->index;out->phase=evidence_phase(s.phase);out->attempted=s.attempted;
  if(s.auxiliary_model){out->private_model_id=s.auxiliary_model->model_id;out->private_revision=s.auxiliary_model->revision;
    out->row_count=s.auxiliary_model->rows.size();out->column_count=s.auxiliary_model->variables.size();}
  out->nonzeros=s.nonzeros;out->candidate_examined=s.candidate_examined;if(s.candidate_examined)out->check=evidence_validation(s.auxiliary_check);
  out->has_raw_result=s.auxiliary_result.has_value();if(s.auxiliary_result){const auto& r=*s.auxiliary_result;auto& raw=out->raw_result;
    raw.struct_size=sizeof(raw);raw.model_id=r.model_id;raw.revision=r.revision;raw.value_count=r.values.size();raw.mask_count=r.active_variables.size();
    // Raw diagnostics preserve unknown integer codes instead of asserting that
    // a rejected backend result satisfies the ordinary Result contract.
    raw.termination_code=static_cast<int32_t>(r.termination);raw.guarantee_code=static_cast<int32_t>(r.guarantee);
    raw.reported_solution_validated=r.solution_validated;raw.reported_start_submitted=r.start_submitted;raw.elapsed_seconds=r.elapsed_seconds;
    raw.objective=optional(r.objective);raw.best_bound=optional(r.best_bound);raw.absolute_gap=optional(r.absolute_gap);
    raw.relative_gap=optional(r.relative_gap);raw.native_gap=optional(r.native_backend_gap);})
API(lp_evidence_stage_columns,(gecode_opt_handle h,gecode_opt_evidence_column_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("LP evidence column element size does not match v1");
  auto b=get<EvidenceStageBox>(h,Kind::LpEvidenceStage);const auto& s=evidence_stage(*b);const auto& e=evidence(*b->owner);
  if(output_array(buffer,capacity,needed,s.columns.size()))for(std::size_t i=0;i<s.columns.size();++i){const auto& c=s.columns[i];
    gecode_opt_evidence_column_v1 out{};out.struct_size=sizeof(out);out.kind=evidence_column(c.kind);out.has_side=c.side.has_value();if(c.side)out.side=evidence_side(*c.side);
    if(!s.auxiliary_model||i>=s.auxiliary_model->variables.size())throw O::ModelError("Malformed auxiliary column mapping");
    out.private_variable=identifier(s.auxiliary_model->variables[i].variable);
    out.source={e.id(),static_cast<uint64_t>(c.original_slot),c.kind==O::LpEvidenceColumnKind::RowSide?GECODE_OPT_ROW_ID:GECODE_OPT_VARIABLE_ID,0};buffer[i]=out;})
API(lp_evidence_stage_raw_values,(gecode_opt_handle h,gecode_opt_evidence_raw_value_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("LP evidence raw value element size does not match v1");
  auto b=get<EvidenceStageBox>(h,Kind::LpEvidenceStage);const auto& s=evidence_stage(*b);
  const auto n=s.auxiliary_result?std::max(s.auxiliary_result->values.size(),s.auxiliary_result->active_variables.size()):0;
  if(output_array(buffer,capacity,needed,n))for(std::size_t i=0;i<n;++i){const auto& r=*s.auxiliary_result;gecode_opt_evidence_raw_value_v1 out{};
    out.struct_size=sizeof(out);out.slot=i;if(i<r.values.size())out.reported_value=optional(r.values[i]);
    out.has_reported_mask=i<r.active_variables.size();if(out.has_reported_mask)out.reported_mask=r.active_variables[i];buffer[i]=out;})
API(lp_evidence_stage_text,(gecode_opt_handle h,int32_t field,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<EvidenceStageBox>(h,Kind::LpEvidenceStage);const auto& s=evidence_stage(*b);const std::string* text=nullptr;
  switch(field){case GECODE_OPT_EVIDENCE_RAW_BACKEND:if(s.auxiliary_result)text=&s.auxiliary_result->backend;break;
    case GECODE_OPT_EVIDENCE_RAW_BACKEND_VERSION:if(s.auxiliary_result)text=&s.auxiliary_result->backend_version;break;
    case GECODE_OPT_EVIDENCE_RAW_MESSAGE:if(s.auxiliary_result)text=&s.auxiliary_result->message;break;
    case GECODE_OPT_EVIDENCE_STAGE_CHECK_MESSAGE:if(s.candidate_examined)text=&s.auxiliary_check.message;break;
    default:argument("unknown LP evidence stage text selector");}output_text(text?*text:std::string(),buffer,capacity,needed))

#ifdef GECODE_OPTIMIZE_TEST_SENSITIVITY_BINDING
extern "C" void gecode_opt_test_sensitivity_binding_checkpoint(void);
#endif
namespace {
void sensitivity_binding_checkpoint(){
#ifdef GECODE_OPTIMIZE_TEST_SENSITIVITY_BINDING
  gecode_opt_test_sensitivity_binding_checkpoint();
#endif
}
struct SensitivityBox {
  std::shared_ptr<const O::LpObservedResult> source;
  O::LpSensitivityResult result;
  std::size_t preparation_visits=0;
  bool cleanup_stopped=false;
};
const O::LpSensitivity& sensitivity(const SensitivityBox& box){
  if(!box.result.sensitivity)throw ApiError{GECODE_OPT_NO_SENSITIVITY,"No historical LP sensitivity artifact"};
  return *box.result.sensitivity;
}
std::size_t sensitivity_limit(uint64_t n){
  if(n>std::numeric_limits<std::size_t>::max())argument("sensitivity limit exceeds size_t");return static_cast<std::size_t>(n);
}
template<class T> void sensitivity_input(const T& r){
  if(r.struct_size!=sizeof(r)||r.reserved)argument("sensitivity record size/reserved mismatch");
}
O::LpSensitivityOptions sensitivity_options(const gecode_opt_sensitivity_options_v1* input){
  O::LpSensitivityOptions o;if(!input)return o;
  sensitivity_input(*input);if(input->reserved_flags)argument("sensitivity options reserved flags must be zero");
  sensitivity_input(input->checks);sensitivity_input(input->limits);
  o.backend=backend(input->backend);o.time_limit_seconds=input->time_limit_seconds;
  if(input->cancellation)o.cancellation=get<O::CancellationToken>(input->cancellation,Kind::Cancellation);
  const auto& c=input->checks;o.checks.primal_feasibility=c.primal_feasibility;
  o.checks.kkt={c.dual_feasibility,c.stationarity,c.complementarity,c.objective_gap};
  o.checks.system_absolute=c.system_absolute;o.checks.system_relative=c.system_relative;
  const auto& l=input->limits;
  o.limits={sensitivity_limit(l.max_rows),sensitivity_limit(l.max_columns),sensitivity_limit(l.max_nonzeros),
    sensitivity_limit(l.max_requests),sensitivity_limit(l.max_basis_solves),sensitivity_limit(l.max_factor_entries),
    sensitivity_limit(l.max_retained_slots),sensitivity_limit(l.max_work)};return o;
}
gecode_opt_sensitivity_checks_options_v1 sensitivity_checks_options(const O::LpSensitivityTolerances& c){
  return {sizeof(gecode_opt_sensitivity_checks_options_v1),0,c.primal_feasibility,c.kkt.dual_feasibility,
    c.kkt.stationarity,c.kkt.complementarity,c.kkt.objective_gap,c.system_absolute,c.system_relative};
}
int32_t sensitivity_reason(O::LpSensitivityReason r){switch(r){
  case O::LpSensitivityReason::None:return GECODE_OPT_SENSITIVITY_REASON_NONE;
  case O::LpSensitivityReason::NotRequested:return GECODE_OPT_SENSITIVITY_REASON_NOT_REQUESTED;
  case O::LpSensitivityReason::Unsupported:return GECODE_OPT_SENSITIVITY_REASON_UNSUPPORTED;
  case O::LpSensitivityReason::NotOptimal:return GECODE_OPT_SENSITIVITY_REASON_NOT_OPTIMAL;
  case O::LpSensitivityReason::NoBasis:return GECODE_OPT_SENSITIVITY_REASON_NO_BASIS;
  case O::LpSensitivityReason::InvalidSource:return GECODE_OPT_SENSITIVITY_REASON_INVALID_SOURCE;
  case O::LpSensitivityReason::InvalidBasis:return GECODE_OPT_SENSITIVITY_REASON_INVALID_BASIS;
  case O::LpSensitivityReason::ChangedBasis:return GECODE_OPT_SENSITIVITY_REASON_CHANGED_BASIS;
  case O::LpSensitivityReason::FailedReferenceChecks:return GECODE_OPT_SENSITIVITY_REASON_REFERENCE_CHECKS;
  case O::LpSensitivityReason::FailedLinearSolveChecks:return GECODE_OPT_SENSITIVITY_REASON_SYSTEM_CHECKS;
  case O::LpSensitivityReason::FailedIntervalChecks:return GECODE_OPT_SENSITIVITY_REASON_INTERVAL_CHECKS;
  case O::LpSensitivityReason::ResourceLimit:return GECODE_OPT_SENSITIVITY_REASON_RESOURCE_LIMIT;
  case O::LpSensitivityReason::Stopped:return GECODE_OPT_SENSITIVITY_REASON_STOPPED;
  case O::LpSensitivityReason::AllocationFailure:return GECODE_OPT_SENSITIVITY_REASON_ALLOCATION;
  case O::LpSensitivityReason::BackendFailure:return GECODE_OPT_SENSITIVITY_REASON_BACKEND;
}argument("unknown sensitivity reason");}
int32_t sensitivity_completion(O::LpSensitivityCompletion c){switch(c){
  case O::LpSensitivityCompletion::Complete:return GECODE_OPT_SENSITIVITY_COMPLETE;
  case O::LpSensitivityCompletion::Partial:return GECODE_OPT_SENSITIVITY_PARTIAL;
  case O::LpSensitivityCompletion::Interrupted:return GECODE_OPT_SENSITIVITY_INTERRUPTED;
  case O::LpSensitivityCompletion::Rejected:return GECODE_OPT_SENSITIVITY_ANALYSIS_REJECTED;
}argument("unknown sensitivity completion");}
int32_t sensitivity_state(O::LpSensitivityState s){switch(s){
  case O::LpSensitivityState::NotRequested:return GECODE_OPT_SENSITIVITY_NOT_REQUESTED;
  case O::LpSensitivityState::Available:return GECODE_OPT_SENSITIVITY_AVAILABLE;
  case O::LpSensitivityState::Unavailable:return GECODE_OPT_SENSITIVITY_UNAVAILABLE;
  case O::LpSensitivityState::Rejected:return GECODE_OPT_SENSITIVITY_REJECTED;
}argument("unknown sensitivity state");}
gecode_opt_id sensitivity_entity(const O::LpSensitivityEntity& e){return std::visit([](const auto& v){return identifier(v);},e);}
gecode_opt_sensitivity_request_v1 sensitivity_request(const O::LpSensitivityParameter& p){
  gecode_opt_sensitivity_request_v1 r{};r.struct_size=sizeof(r);
  if(const auto* v=std::get_if<O::LpObjectiveParameter>(&p)){r.kind=GECODE_OPT_SENSITIVITY_OBJECTIVE;r.entity=identifier(v->variable);}
  else {r.kind=GECODE_OPT_SENSITIVITY_EQUALITY_RHS;r.entity=identifier(std::get<O::LpEqualityRhsParameter>(p).row);}return r;
}
O::LpSensitivityParameter sensitivity_request(const gecode_opt_sensitivity_request_v1& r){
  sensitivity_input(r);if(r.reserved_flags)argument("sensitivity request reserved flags must be zero");
  switch(r.kind){case GECODE_OPT_SENSITIVITY_OBJECTIVE:return O::LpObjectiveParameter{variable(r.entity)};
    case GECODE_OPT_SENSITIVITY_EQUALITY_RHS:return O::LpEqualityRhsParameter{row(r.entity)};
    default:argument("unknown sensitivity request kind");}
}
gecode_opt_sensitivity_end_v1 sensitivity_end(const O::LpRangeEnd& e){
  gecode_opt_sensitivity_end_v1 r{};r.struct_size=sizeof(r);
  switch(e.kind){case O::LpRangeEndKind::Finite:r.kind=GECODE_OPT_RANGE_FINITE;
      if(!e.value||!std::isfinite(*e.value))throw O::ModelError("Malformed finite sensitivity endpoint");break;
    case O::LpRangeEndKind::NegativeInfinity:r.kind=GECODE_OPT_RANGE_NEGATIVE_INFINITY;break;
    case O::LpRangeEndKind::PositiveInfinity:r.kind=GECODE_OPT_RANGE_POSITIVE_INFINITY;break;
    default:throw O::ModelError("Unknown sensitivity endpoint kind");}
  if(e.kind!=O::LpRangeEndKind::Finite&&e.value)throw O::ModelError("Infinite endpoint has a numeric value");r.value=optional(e.value);return r;
}
gecode_opt_sensitivity_limiter_v1 sensitivity_limiter(const O::LpSensitivityLimiter& l){
  gecode_opt_sensitivity_limiter_v1 r{};r.struct_size=sizeof(r);r.entity=sensitivity_entity(l.entity);r.dual_condition=l.dual_condition;
  switch(l.side){case O::LpSensitivitySide::Lower:r.side=GECODE_OPT_SENSITIVITY_LOWER;break;
    case O::LpSensitivitySide::Upper:r.side=GECODE_OPT_SENSITIVITY_UPPER;break;
    case O::LpSensitivitySide::Fixed:r.side=GECODE_OPT_SENSITIVITY_FIXED;break;
    case O::LpSensitivitySide::Free:r.side=GECODE_OPT_SENSITIVITY_FREE;break;
    default:throw O::ModelError("Unknown sensitivity limiter side");}return r;
}
gecode_opt_sensitivity_entry_v1 sensitivity_entry(const SensitivityBox& box,const O::LpSensitivityParameter& p,const O::LpSensitivityEntry* entry){
  gecode_opt_sensitivity_entry_v1 r{};r.struct_size=sizeof(r);r.request=sensitivity_request(p);r.group.struct_size=sizeof(r.group);
  r.lower.struct_size=sizeof(r.lower);r.upper.struct_size=sizeof(r.upper);r.lower_limiter.struct_size=sizeof(r.lower_limiter);
  r.upper_limiter.struct_size=sizeof(r.upper_limiter);r.checks.struct_size=sizeof(r.checks);
  r.group.state=GECODE_OPT_SENSITIVITY_NOT_REQUESTED;r.group.reason=GECODE_OPT_SENSITIVITY_REASON_NOT_REQUESTED;
  if(!entry)return r;r.requested=1;r.index=static_cast<uint64_t>(entry-sensitivity(box).entries().data());
  r.group.state=sensitivity_state(entry->group.state);r.group.reason=sensitivity_reason(entry->group.reason);
  if(box.cleanup_stopped){r.group.state=GECODE_OPT_SENSITIVITY_UNAVAILABLE;r.group.reason=GECODE_OPT_SENSITIVITY_REASON_STOPPED;return r;}
  if(!entry->interval){if(entry->group.state==O::LpSensitivityState::Available)throw O::ModelError("Available sensitivity has no interval");return r;}
  if(entry->group.state!=O::LpSensitivityState::Available)throw O::ModelError("Sensitivity interval is present without Available state");
  const auto& v=*entry->interval;r.has_interval=1;r.anchor=v.anchor;r.lower=sensitivity_end(v.lower);r.upper=sensitivity_end(v.upper);
  r.objective_slope=optional(v.objective_slope);r.has_lower_limiter=v.lower_limiter.has_value();r.has_upper_limiter=v.upper_limiter.has_value();
  if(v.lower_limiter)r.lower_limiter=sensitivity_limiter(*v.lower_limiter);if(v.upper_limiter)r.upper_limiter=sensitivity_limiter(*v.upper_limiter);
  r.checks.inequalities=v.checks.inequalities;r.checks.accepted=v.checks.accepted;r.checks.lower_direction_checked=v.checks.lower_direction_checked;
  r.checks.upper_direction_checked=v.checks.upper_direction_checked;r.checks.max_endpoint_violation=optional(v.checks.max_endpoint_violation);return r;
}
std::optional<O::Termination> sensitivity_stop(const std::shared_ptr<O::CancellationToken>& token,double seconds,Clock::time_point start){
  if(token&&token->cancelled())return O::Termination::Cancelled;
  if(std::chrono::duration<double>(Clock::now()-start).count()>=seconds)return O::Termination::TimeLimit;return {};
}
const char* sensitivity_cleanup_message="Whole LP sensitivity binding allowance stopped during input cleanup";
void sensitivity_failure(SensitivityBox& b,O::LpSensitivityReason reason,const char* message,std::optional<O::Termination> stop={}){
  b.result.reason=reason;b.result.message=message;b.result.stop_reason=stop;
  b.result.completion=stop?O::LpSensitivityCompletion::Interrupted:O::LpSensitivityCompletion::Rejected;
}
}
API(sensitivity_options_default,(gecode_opt_sensitivity_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));O::LpSensitivityOptions o;*out={};out->struct_size=sizeof(*out);out->backend=GECODE_OPT_AUTO;
  out->time_limit_seconds=o.time_limit_seconds;out->checks=sensitivity_checks_options(o.checks);const auto& l=o.limits;
  out->limits={sizeof(out->limits),0,l.max_rows,l.max_columns,l.max_nonzeros,l.max_requests,l.max_basis_solves,l.max_factor_entries,l.max_retained_slots,l.max_work})
API(analyze_lp_sensitivity,(gecode_opt_handle h,const gecode_opt_sensitivity_options_v1* input,gecode_opt_handle* output),
  const auto start=Clock::now();required(output);*output=0;auto source=get<O::LpObservedResult>(h,Kind::LpObservedResult);
  auto b=std::make_shared<SensitivityBox>();b->source=std::move(source);b->result.model_id=b->source->result.model_id;b->result.revision=b->source->result.revision;
  double seconds=std::numeric_limits<double>::infinity();std::shared_ptr<O::CancellationToken> token;bool valid_options=false;
  {auto o=sensitivity_options(input);seconds=o.time_limit_seconds;token=o.cancellation;
    const auto n=input?count(input->request_count,input->requests):0;length<O::LpSensitivityParameter>(n);
    try {o.checks.validate();if(std::isnan(seconds)||seconds<0)throw O::ModelError("Invalid sensitivity time limit");
      if(!n)throw O::ModelError("Sensitivity requires a nonempty explicit parameter list");valid_options=true;
    } catch(const O::ModelError& e){sensitivity_failure(*b,O::LpSensitivityReason::InvalidSource,e.what());}
    if(valid_options){
      if(const auto stop=sensitivity_stop(token,seconds,start))sensitivity_failure(*b,O::LpSensitivityReason::Stopped,"Sensitivity binding input preparation stopped",stop);
      else if(n>o.limits.max_requests)sensitivity_failure(*b,O::LpSensitivityReason::ResourceLimit,"Sensitivity request limit exceeded before marshalling",O::Termination::MemoryLimit);
      else if(n>o.limits.max_work)sensitivity_failure(*b,O::LpSensitivityReason::ResourceLimit,"Sensitivity work limit exceeded before marshalling",O::Termination::IterationLimit);
      else {o.parameters.reserve(n);
        for(std::size_t i=0;i<n;++i){if(const auto stop=sensitivity_stop(token,seconds,start)){sensitivity_failure(*b,O::LpSensitivityReason::Stopped,"Sensitivity input preparation stopped",stop);break;}
          ++b->preparation_visits;o.parameters.push_back(sensitivity_request(input->requests[i]));}
        if(o.parameters.size()==n){o.limits.max_work-=b->preparation_visits;
          if(std::isfinite(seconds))o.time_limit_seconds=std::max(0.0,seconds-std::chrono::duration<double>(Clock::now()-start).count());
          b->result=O::analyze_lp_sensitivity(*b->source,o);}
      }
    }
  }
  sensitivity_binding_checkpoint();
  if(valid_options)if(const auto stop=sensitivity_stop(token,seconds,start)){b->cleanup_stopped=true;
    sensitivity_failure(*b,O::LpSensitivityReason::Stopped,sensitivity_cleanup_message,stop);}
  b->result.elapsed_seconds=std::chrono::duration<double>(Clock::now()-start).count();*output=put(Kind::SensitivityResult,std::move(b)))
API(sensitivity_destroy,(gecode_opt_handle h),destroy(h,Kind::SensitivityResult))
API(sensitivity_copy_source_observed,(gecode_opt_handle h,gecode_opt_handle* output),
  required(output);*output=0;auto b=get<SensitivityBox>(h,Kind::SensitivityResult);
  *output=put(Kind::LpObservedResult,std::make_shared<O::LpObservedResult>(*b->source)))
API(sensitivity_copy_basis,(gecode_opt_handle h,gecode_opt_handle* output),
  required(output);*output=0;auto b=get<SensitivityBox>(h,Kind::SensitivityResult);
  if(!b->result.sensitivity||!b->result.sensitivity->basis())throw ApiError{GECODE_OPT_NO_BASIS,"No selected sensitivity basis"};
  auto out=std::make_shared<BasisBox>();out->basis=b->result.sensitivity->basis();*output=put(Kind::Basis,std::move(out)))
API(sensitivity_info,(gecode_opt_handle h,gecode_opt_sensitivity_info_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& r=b->result;*out={};out->struct_size=sizeof(*out);
  out->model_id=r.model_id;out->revision=r.revision;out->completion=sensitivity_completion(r.completion);out->reason=sensitivity_reason(r.reason);
  out->has_stop_reason=r.stop_reason.has_value();if(r.stop_reason)out->stop_reason=termination(*r.stop_reason);out->guarantee=GECODE_OPT_NUMERICAL;
  out->has_sensitivity=bool(r.sensitivity);out->elapsed_seconds=r.elapsed_seconds;
  if(r.sensitivity){const auto& s=*r.sensitivity;out->has_basis=bool(s.basis());out->entry_count=s.entries().size();out->factor_order_count=s.factor_order().size();
    out->row_slots=s.active_rows().size();out->column_slots=s.active_columns().size();})
API(sensitivity_work,(gecode_opt_handle h,gecode_opt_sensitivity_work_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& w=b->result.work;
  *out={sizeof(*out),0,w.factor_setup_attempted,0,w.basis_solves,w.coordinator_visits,w.retained_slots,b->preparation_visits})
API(sensitivity_checks_options,(gecode_opt_handle h,gecode_opt_sensitivity_checks_options_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);*out=sensitivity_checks_options(sensitivity(*b).tolerances()))
API(sensitivity_reference_checks,(gecode_opt_handle h,gecode_opt_sensitivity_reference_checks_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& c=sensitivity(*b).checks();*out={};out->struct_size=sizeof(*out);
  out->primal=evidence_validation(c.primal);out->basis_point_matches=c.basis_point_matches;out->max_point_difference=optional(c.max_point_difference);
  out->max_system_residual=optional(c.max_system_residual);out->max_scaled_system_residual=optional(c.max_scaled_system_residual);
  const auto& k=c.kkt;auto& r=out->kkt;r.struct_size=sizeof(r);r.primal_valid=k.primal_valid;r.dual_signs_valid=k.dual_signs_valid;
  r.stationarity_valid=k.stationarity_valid;r.complementarity_valid=k.complementarity_valid;r.gap_valid=k.gap_valid;r.accepted=k.accepted;
  r.max_dual_sign_violation=optional(k.max_dual_sign_violation);r.max_stationarity=optional(k.max_stationarity);
  r.max_complementarity=optional(k.max_complementarity);r.dual_objective_estimate=optional(k.dual_objective_estimate);r.normalized_gap=optional(k.normalized_gap))
API(sensitivity_entry,(gecode_opt_handle h,uint64_t index,gecode_opt_sensitivity_entry_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& e=at(sensitivity(*b).entries(),index);*out=sensitivity_entry(*b,e.parameter,&e))
API(sensitivity_entries,(gecode_opt_handle h,gecode_opt_sensitivity_entry_v1* buffer,uint64_t element_size,uint64_t capacity,uint64_t* needed),
  if(element_size!=sizeof(*buffer))argument("Sensitivity entry element size mismatch");auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& es=sensitivity(*b).entries();
  if(output_array(buffer,capacity,needed,es.size()))for(std::size_t i=0;i<es.size();++i)buffer[i]=sensitivity_entry(*b,es[i].parameter,&es[i]))
API(sensitivity_objective,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_sensitivity_entry_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto v=variable(id);
  *out=sensitivity_entry(*b,O::LpObjectiveParameter{v},sensitivity(*b).objective(v)))
API(sensitivity_equality_rhs,(gecode_opt_handle h,gecode_opt_id id,gecode_opt_sensitivity_entry_v1* out,uint64_t size),
  size_check(out,size,sizeof(*out));auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto r=row(id);
  *out=sensitivity_entry(*b,O::LpEqualityRhsParameter{r},sensitivity(*b).equality_rhs(r)))
API(sensitivity_factor_order,(gecode_opt_handle h,gecode_opt_id* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& order=sensitivity(*b).factor_order();
  if(output_array(buffer,capacity,needed,order.size()))for(std::size_t i=0;i<order.size();++i)buffer[i]=sensitivity_entity(order[i]))
API(sensitivity_active_slots,(gecode_opt_handle h,int32_t kind,uint8_t* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const auto& s=sensitivity(*b);
  if(kind!=GECODE_OPT_VARIABLE_ID&&kind!=GECODE_OPT_ROW_ID)argument("Sensitivity mask must select Variable or Row");
  const auto& mask=kind==GECODE_OPT_VARIABLE_ID?s.active_columns():s.active_rows();
  if(output_array(buffer,capacity,needed,mask.size()))for(std::size_t i=0;i<mask.size();++i)buffer[i]=mask[i])
API(sensitivity_text,(gecode_opt_handle h,int32_t field,uint64_t index,char* buffer,uint64_t capacity,uint64_t* needed),
  auto b=get<SensitivityBox>(h,Kind::SensitivityResult);const std::string* value=nullptr;
  if(field!=GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE&&field!=GECODE_OPT_SENSITIVITY_INTERVAL_MESSAGE&&index)argument("Sensitivity text index must be zero");
  switch(field){case GECODE_OPT_SENSITIVITY_MESSAGE:value=&b->result.message;break;
    case GECODE_OPT_SENSITIVITY_BACKEND_VERSION:value=&sensitivity(*b).backend_version();break;
    case GECODE_OPT_SENSITIVITY_PRIMAL_MESSAGE:value=&sensitivity(*b).checks().primal.message;break;
    case GECODE_OPT_SENSITIVITY_KKT_MESSAGE:value=&sensitivity(*b).checks().kkt.message;break;
    case GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE:case GECODE_OPT_SENSITIVITY_INTERVAL_MESSAGE:{const auto& e=at(sensitivity(*b).entries(),index);
      if(b->cleanup_stopped){output_text(sensitivity_cleanup_message,buffer,capacity,needed);return;}
      if(field==GECODE_OPT_SENSITIVITY_ENTRY_MESSAGE)value=&e.group.message;else if(e.interval)value=&e.interval->checks.message;break;}
    default:argument("Unknown sensitivity text field");}output_text(value?*value:std::string(),buffer,capacity,needed))
#undef API
