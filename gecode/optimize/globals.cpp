#include <gecode/optimize/globals.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <type_traits>
#include <utility>

namespace Gecode { namespace Optimize {
GlobalConstraint add_all_different(Model& m,const std::vector<Variable>& v,std::string name) {
  return m.add_global(AllDifferentData{v},std::move(name));
}
GlobalConstraint add_element(Model& m,Variable index,const std::vector<Variable>& elements,
                             Variable result,std::int64_t base,std::string name) {
  return m.add_global(ElementData{index,elements,result,base},std::move(name));
}
GlobalConstraint add_table(Model& m,const std::vector<Variable>& v,
                           const std::vector<std::vector<std::int64_t>>& tuples,std::string name) {
  return m.add_global(TableData{v,tuples},std::move(name));
}
GlobalConstraint add_cumulative(Model& m,const std::vector<Variable>& starts,
                                const std::vector<std::int64_t>& durations,
                                const std::vector<std::int64_t>& heights,
                                std::int64_t capacity,std::string name) {
  return m.add_global(CumulativeData{starts,durations,heights,capacity},std::move(name));
}
GlobalConstraint add_circuit(Model& m,const std::vector<Variable>& successors,
                             std::int64_t base,std::string name) {
  return m.add_global(CircuitData{successors,base},std::move(name));
}
GlobalConstraint add_regular(Model& m,const std::vector<Variable>& variables,
                             std::uint64_t state_count,std::uint64_t initial_state,
                             const std::vector<RegularTransition>& transitions,
                             const std::vector<std::uint64_t>& final_states,std::string name) {
  return m.add_global(RegularData{variables,state_count,initial_state,transitions,final_states},std::move(name));
}

namespace Detail {
namespace {
constexpr std::int64_t exact_limit=INT64_C(9007199254740992);
void constant(std::int64_t value) {
  if(value < -exact_limit || value > exact_limit)
    throw ModelError("Global integer constants must be exactly representable within +/-2^53");
}
void index_range(std::int64_t base,std::size_t size) {
  constant(base);
  if(size>static_cast<std::uint64_t>(exact_limit) ||
     (size && base>exact_limit-static_cast<std::int64_t>(size-1)))
    throw ModelError("Global index range exceeds exact integer representation");
}
}

std::vector<Variable> global_variables(const GlobalPayload& payload) {
  if(payload.valueless_by_exception()) throw ModelError("Global constraint has no payload");
  return std::visit([](const auto& data) {
    using T=std::decay_t<decltype(data)>;
    if constexpr(std::is_same_v<T,AllDifferentData> || std::is_same_v<T,TableData> || std::is_same_v<T,RegularData>) return data.variables;
    else if constexpr(std::is_same_v<T,ElementData>) {
      auto out=data.elements;out.push_back(data.index);out.push_back(data.result);return out;
    } else if constexpr(std::is_same_v<T,CumulativeData>) return data.starts;
    else return data.successors;
  },payload);
}

void validate_global_payload(const GlobalPayload& payload,ModelId owner,
                             const std::vector<VariableData>& variables,bool require_active) {
  for(const auto handle:global_variables(payload)) {
    if(handle.model_id!=owner || handle.id>=variables.size() ||
       (require_active && !variables[handle.id].active))
      throw ModelError("Global constraint references a foreign, invalid, or deleted variable");
    const auto type=variables[handle.id].type;
    if(type!=VariableType::Integer && type!=VariableType::Binary && type!=VariableType::SemiInteger)
      throw ModelError("Global constraints require integer, binary, or semi-integer variables");
  }
  std::visit([](const auto& data) {
    using T=std::decay_t<decltype(data)>;
    if constexpr(std::is_same_v<T,ElementData>) index_range(data.index_base,data.elements.size());
    else if constexpr(std::is_same_v<T,TableData>) {
      for(const auto& tuple:data.tuples) {
        if(tuple.size()!=data.variables.size()) throw ModelError("Table tuple arity mismatch");
        for(const auto value:tuple) constant(value);
      }
    } else if constexpr(std::is_same_v<T,CumulativeData>) {
      if(data.starts.size()!=data.durations.size() || data.starts.size()!=data.heights.size())
        throw ModelError("Cumulative starts, durations and heights must have equal lengths");
      constant(data.capacity);
      if(data.capacity<0) throw ModelError("Cumulative capacity must be nonnegative");
      for(const auto duration:data.durations) {
        constant(duration);if(duration<0) throw ModelError("Task durations must be nonnegative");
      }
      for(const auto height:data.heights) {
        constant(height);if(height<0) throw ModelError("Task heights must be nonnegative");
      }
    } else if constexpr(std::is_same_v<T,CircuitData>) {
      if(data.successors.empty()) throw ModelError("A circuit needs at least one successor");
      index_range(data.index_base,data.successors.size());
    } else if constexpr(std::is_same_v<T,RegularData>) {
      if(!data.state_count||data.initial_state>=data.state_count)
        throw ModelError("Regular state count must be positive and initial state in range");
      std::set<std::pair<std::uint64_t,std::int64_t>> keys;
      for(const auto& transition:data.transitions){
        constant(transition.symbol);
        if(transition.from>=data.state_count||transition.to>=data.state_count)
          throw ModelError("Regular transition state is outside the declared state range");
        if(!keys.emplace(transition.from,transition.symbol).second)
          throw ModelError("Regular transition keys (from,symbol) must be unique");
      }
      for(auto final:data.final_states)if(final>=data.state_count)
        throw ModelError("Regular final state is outside the declared state range");
    }
  },payload);
}

void validate_globals(const ModelSnapshot& model) {
  for(std::size_t i=0;i<model.globals.size();++i) {
    const auto& record=model.globals[i];
    if(record.global.model_id!=model.model_id || record.global.id!=i)
      throw ModelError("Global constraint identity does not match its original slot");
    validate_global_payload(record.payload,model.model_id,model.variables,record.active);
  }
}

bool global_satisfied(const GlobalPayload& payload,const std::vector<double>& values,
                      double tolerance,std::string& reason) {
  bool representable=true;
  const auto value=[&](Variable variable) {
    const double raw=values.at(static_cast<std::size_t>(variable.id));
    if(!std::isfinite(raw) || std::abs(raw)>static_cast<double>(exact_limit) ||
       std::abs(raw-std::round(raw))>tolerance) {
      representable=false;return std::int64_t{0};
    }
    return static_cast<std::int64_t>(std::round(raw));
  };
  // Check representability before branching/indexing on any rounded value.
  for(const auto variable:global_variables(payload)) (void)value(variable);
  if(!representable) {reason="global value is not an exact integer within +/-2^53";return false;}
  const bool valid=std::visit([&](const auto& data)->bool {
    using T=std::decay_t<decltype(data)>;
    if constexpr(std::is_same_v<T,AllDifferentData>) {
      std::vector<std::int64_t> assigned;
      for(const auto variable:data.variables) assigned.push_back(value(variable));
      std::sort(assigned.begin(),assigned.end());
      return std::adjacent_find(assigned.begin(),assigned.end())==assigned.end();
    } else if constexpr(std::is_same_v<T,ElementData>) {
      const auto index=value(data.index);
      if(index<data.index_base || static_cast<std::uint64_t>(index-data.index_base)>=data.elements.size()) return false;
      return value(data.result)==value(data.elements[static_cast<std::size_t>(index-data.index_base)]);
    } else if constexpr(std::is_same_v<T,TableData>) {
      for(const auto& tuple:data.tuples) {
        bool equal=true;
        for(std::size_t i=0;i<tuple.size();++i) if(value(data.variables[i])!=tuple[i]) {equal=false;break;}
        if(equal) return true;
      }
      return false;
    } else if constexpr(std::is_same_v<T,CumulativeData>) {
      std::vector<std::pair<std::int64_t,std::int64_t>> events;
      for(std::size_t i=0;i<data.starts.size();++i) {
        if(!data.durations[i] || !data.heights[i]) continue;
        const auto start=value(data.starts[i]);
        events.emplace_back(start,data.heights[i]);
        events.emplace_back(start+data.durations[i],-data.heights[i]);
      }
      // Ends precede starts at equal times: tasks use half-open intervals.
      std::sort(events.begin(),events.end());
      std::int64_t used=0;
      for(const auto& event:events) {
        if(event.second>0 && event.second>data.capacity-used) return false;
        used+=event.second;
      }
      return true;
    } else if constexpr(std::is_same_v<T,RegularData>) {
      std::map<std::pair<std::uint64_t,std::int64_t>,std::uint64_t> transitions;
      for(const auto& edge:data.transitions)transitions.emplace(std::make_pair(edge.from,edge.symbol),edge.to);
      auto state=data.initial_state;
      for(auto variable:data.variables){
        const auto edge=transitions.find({state,value(variable)});
        if(edge==transitions.end())return false;
        state=edge->second;
      }
      return std::find(data.final_states.begin(),data.final_states.end(),state)!=data.final_states.end();
    } else {
      std::vector<bool> visited(data.successors.size(),false);
      std::size_t node=0;
      for(std::size_t step=0;step<visited.size();++step) {
        if(visited[node]) return false;
        visited[node]=true;
        const auto successor=value(data.successors[node]);
        if(successor<data.index_base || static_cast<std::uint64_t>(successor-data.index_base)>=visited.size()) return false;
        node=static_cast<std::size_t>(successor-data.index_base);
      }
      return node==0;
    }
  },payload);
  if(!valid) reason="original global constraint violated";
  return valid;
}
}
}}
