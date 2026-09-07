#include <gecode/optimize/native_preprocess.hpp>
#include <gecode/optimize/validate.hpp>
#include <cmath>
#include <algorithm>
#include <map>
#include <tuple>

namespace Gecode { namespace Optimize { namespace Detail {
std::optional<SolveResult> native_symmetry(const ModelSnapshot& model,
    const SolveOptions& options, SolveBudget& budget,
    const NativeSolveContinuation& continuation) {
  // Exact duplicate columns only: arbitrary permutation of each group must
  // preserve every original row, domain, variable type and objective coefficient.
  if (!options.primal_start.empty() || !model.globals.empty() || !model.indicators.empty() ||
      model.variables.size()>4096 || model.rows.size()>4096 || model.objective.terms.size()>65536)
    return {};
  const auto stopped=[&]{return budget.cancelled() || budget.time_limit_reached();};
  if(stopped())return {};
  std::size_t nonzeros=model.objective.terms.size();
  for(const auto& row:model.rows){
    if(row.terms.size()>65536-nonzeros)return {};
    nonzeros+=row.terms.size();
  }
  for(const auto& v:model.variables)if(v.active &&
      (v.type!=VariableType::Integer && v.type!=VariableType::Binary))return {};
  validate_structure(model);
  using Column=std::vector<std::pair<std::size_t,double>>;
  std::vector<Column> columns(model.variables.size());
  std::vector<double> costs(model.variables.size(),0);
  for(const auto& t:model.objective.terms)costs[t.variable.id]=t.coefficient;
  for(std::size_t i=0;i<model.rows.size();++i){
    if(stopped())return {};
    if(model.rows[i].active)for(const auto& t:model.rows[i].terms)
      columns[t.variable.id].push_back({i,t.coefficient});
  }
  using Key=std::tuple<VariableType,double,double,double,Column>;
  std::map<Key,std::size_t> last;
  std::vector<std::pair<std::size_t,std::size_t>> order;
  for(std::size_t i=0;i<model.variables.size();++i){
    if(stopped())return {};
    const auto& v=model.variables[i];
    if(!v.active || v.lower==v.upper ||
        std::max(std::fabs(v.lower),std::fabs(v.upper))>500000000)continue;
    Key key{v.type,v.lower,v.upper,costs[i],std::move(columns[i])};
    auto added=last.emplace(std::move(key),i);
    if(!added.second){order.push_back({added.first->second,i});added.first->second=i;}
  }
  if(order.empty())return {};
  auto reduced=model; // Private derived snapshot; original handles remain stable.
  for(auto pair:order){
    if(stopped())return {};
    RowData row;row.constraint={model.model_id,reduced.rows.size()};
    row.terms={{model.variables[pair.first].variable,1},{model.variables[pair.second].variable,-1}};
    row.upper=0;row.name="automatic interchangeable-column order";
    reduced.rows.push_back(std::move(row));
  }
  auto result=continuation(reduced,options,budget);
  if(result.has_solution()){
    // The outer native admission bounds all integer row/objective activities;
    // tolerance-zero original evaluation is exact within that admitted range.
    const auto checked=validate(model,result.values,0,0);
    if(!checked.valid || checked.objective!=result.objective){
      SolveResult failed;failed.model_id=model.model_id;failed.revision=model.revision;
      failed.backend=result.backend;failed.guarantee=options.guarantee;
      failed.termination=Termination::BackendError;
      failed.message="Automatic symmetry original-model validation failed";
      return failed;
    }
    result.solution_validated=true;
  }
  if(stopped()){
    const auto why=budget.cancelled()?Termination::Cancelled:Termination::TimeLimit;
    const auto backend=result.backend;
    result=SolveResult{};result.model_id=model.model_id;result.revision=model.revision;
    result.backend=backend;result.guarantee=options.guarantee;result.termination=why;
  }
  result.message="Automatic duplicate-column symmetry: "+std::to_string(order.size())+
    " ordering constraints; "+result.message;
  result.elapsed_seconds=budget.elapsed_seconds();
  return result;
}
}}}
