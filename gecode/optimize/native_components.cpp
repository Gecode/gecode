/* Exact additive decomposition; components share the original solve budget. */
#include <gecode/optimize/native_preprocess.hpp>
#include <gecode/optimize/validate.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace Gecode { namespace Optimize { namespace Detail {
namespace {
bool integral(double value) { return std::isfinite(value) && std::floor(value)==value; }
struct Disjoint {
  std::vector<std::size_t> parent, size;
  explicit Disjoint(std::size_t n):parent(n),size(n,1) {std::iota(parent.begin(),parent.end(),0);}
  std::size_t root(std::size_t x) {
    while(parent[x]!=x) {parent[x]=parent[parent[x]];x=parent[x];}return x;
  }
  void join(std::size_t a,std::size_t b) {
    a=root(a);b=root(b);if(a==b)return;
    if(size[a]<size[b])std::swap(a,b);parent[b]=a;size[a]+=size[b];
  }
};
}

std::optional<SolveResult> native_components(const ModelSnapshot& model,
    const SolveOptions& options,SolveBudget& budget,const NativeSolveContinuation& continuation) {
  // Original native validation/compilation is the caller's admission gate.
  // These additional checks keep decomposition finite and its arithmetic exact.
  constexpr std::size_t columns_cap=4096, rows_cap=4096, nonzeros_cap=65536, components_cap=64;
  if(!options.primal_start.empty() || options.guarantee==Guarantee::Certified ||
     (options.backend!=Backend::Native && options.backend!=Backend::Auto) ||
     options.threads!=1 || options.random_seed!=0 || !model.globals.empty() || !model.indicators.empty() ||
     model.variables.size()>columns_cap || model.rows.size()>rows_cap ||
     model.objective.terms.size()>nonzeros_cap || !integral(model.objective.offset)) return {};
  auto result=[&](Termination reason,const std::string& message) {
    SolveResult out;out.model_id=model.model_id;out.revision=model.revision;
    out.backend="Gecode native components";out.guarantee=options.guarantee;
    out.termination=reason;out.message=message;out.elapsed_seconds=budget.elapsed_seconds();return out;
  };
  auto stopped=[&]() -> std::optional<SolveResult> {
    if(auto reason=budget.stop_reason())return result(*reason,"Independent components stopped by shared budget");
    return {};
  };
  if(auto stop=stopped())return stop;
  std::size_t nonzeros=model.objective.terms.size(),active=0;
  for(const auto& v:model.variables) {
    if(auto stop=stopped())return stop;
    if(!v.active)continue;
    ++active;
    if(v.indicator_origin || (v.type!=VariableType::Integer && v.type!=VariableType::Binary) ||
       !integral(v.lower) || !integral(v.upper))return {};
  }
  if(active<2)return {};
  for(const auto& t:model.objective.terms)if(!integral(t.coefficient))return {};
  for(const auto& row:model.rows) {
    if(auto stop=stopped())return stop;
    if(!row.active)continue;
    if(row.indicator_origin || row.terms.size()>nonzeros_cap-nonzeros ||
       (std::isfinite(row.lower) && !integral(row.lower)) ||
       (std::isfinite(row.upper) && !integral(row.upper)))return {};
    nonzeros+=row.terms.size();
    for(const auto& t:row.terms)if(!integral(t.coefficient))return {};
  }
  // The function is internal, but reject corrupt slot references before indexing.
  validate_structure(model);
  Disjoint sets(model.variables.size());
  for(const auto& row:model.rows)if(row.active && !row.terms.empty()) {
    for(const auto& term:row.terms) {
      if(auto stop=stopped())return stop;
      sets.join(row.terms.front().variable.id,term.variable.id);
    }
  }
  const auto missing=std::numeric_limits<std::size_t>::max();
  std::vector<std::size_t> component(model.variables.size(),missing),root_component(model.variables.size(),missing);
  std::vector<std::vector<std::size_t>> slots;
  for(std::size_t i=0;i<model.variables.size();++i)if(model.variables[i].active) {
    if(auto stop=stopped())return stop;
    const auto r=sets.root(i);
    if(root_component[r]==missing) {
      if(slots.size()==components_cap)return {};
      root_component[r]=slots.size();slots.emplace_back();
    }
    component[i]=root_component[r];slots[component[i]].push_back(i);
  }
  if(slots.size()<2)return {};
  std::vector<std::vector<std::size_t>> row_ids(slots.size());
  std::vector<std::vector<Term>> objective(slots.size());
  for(std::size_t i=0;i<model.rows.size();++i)if(model.rows[i].active) {
    if(auto stop=stopped())return stop;
    const auto& terms=model.rows[i].terms;
    // Constant constraints belong to the first component and are still proved.
    row_ids[terms.empty()?0:component[terms.front().variable.id]].push_back(i);
  }
  for(const auto& t:model.objective.terms)objective[component[t.variable.id]].push_back(t);
  std::vector<double> assembled(model.variables.size(),std::numeric_limits<double>::quiet_NaN());
  std::vector<Variable> mapped(model.variables.size());
  std::string backend_version;
  for(std::size_t c=0;c<slots.size();++c) {
    if(auto stop=stopped())return stop;
    Model submodel;
    for(auto slot:slots[c]) {
      if(auto stop=stopped())return stop;
      const auto& v=model.variables[slot];
      mapped[slot]=submodel.add_variable(v.type,v.lower,v.upper);
    }
    for(auto row_id:row_ids[c]) {
      if(auto stop=stopped())return stop;
      const auto& row=model.rows[row_id];std::vector<Term> terms;terms.reserve(row.terms.size());
      for(const auto& t:row.terms)terms.push_back({mapped[t.variable.id],t.coefficient});
      submodel.add_row(terms,row.lower,row.upper);
    }
    std::vector<Term> terms;terms.reserve(objective[c].size());
    for(const auto& t:objective[c])terms.push_back({mapped[t.variable.id],t.coefficient});
    // The original offset is included only in the final original evaluation.
    submodel.set_objective(terms,model.objective.sense,0);
    const auto snapshot=submodel.snapshot();
    if(auto stop=stopped())return stop;
    const auto solved=continuation(snapshot,options,budget);
    if(budget.cancelled() || budget.time_limit_reached())return stopped();
    if(solved.model_id!=snapshot.model_id || solved.revision!=snapshot.revision ||
       solved.guarantee!=options.guarantee)
      return result(Termination::BackendError,"Independent component result identity or guarantee differs");
    if(solved.termination==Termination::Infeasible)
      return result(Termination::Infeasible,"An independent component proved the original model infeasible");
    if(solved.termination!=Termination::Optimal)
      return result(solved.termination,"Independent component stopped; no complete original incumbent assembled");
    if(!solved.has_solution() || solved.best_bound!=solved.objective ||
       solved.values.size()!=slots[c].size() || solved.active_variables.size()!=slots[c].size())
      return result(Termination::BackendError,"Incomplete independent component optimum");
    const auto checked=validate(snapshot,solved.values,0,0);
    if(!checked.valid || checked.objective!=solved.objective)
      return result(Termination::BackendError,"Independent component witness failed exact validation");
    for(auto slot:slots[c])assembled[slot]=solved.values[mapped[slot].id];
    if(backend_version.empty())backend_version=solved.backend_version;
  }
  const auto checked=validate(model,assembled,0,0);
  if(budget.cancelled() || budget.time_limit_reached())return stopped();
  if(!checked.valid || !checked.objective)
    return result(Termination::BackendError,"Assembled original witness failed exact validation");
  auto out=result(Termination::Optimal,"Exact independent components: "+std::to_string(slots.size()));
  out.backend_version=std::move(backend_version);out.values=std::move(assembled);
  for(const auto& v:model.variables)out.active_variables.push_back(v.active);
  out.objective=checked.objective;out.best_bound=checked.objective;
  out.solution_validated=true;out.update_gaps(model.objective.sense);
  if(budget.cancelled() || budget.time_limit_reached())return stopped();
  out.elapsed_seconds=budget.elapsed_seconds();return out;
}

}}}
