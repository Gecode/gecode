/* Original-record compiler and checker, independent of native parser posting. */
#include <gecode/optimize/flatzinc.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/globals.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <set>
#include <sstream>
#include <tuple>

namespace Gecode { namespace Optimize {
namespace F=Gecode::FlatZinc::Capture;
namespace Detail {
struct FlatZincArtifact {
  F::Records source;
  ModelSnapshot model;
  std::vector<FlatZincVariableMapping> mapping;
};
namespace {
using I=std::int64_t;
constexpr I exact_limit=9007199254740992LL;
struct Failure { FlatZincCompileStatus status; std::string message; F::Location location; };
[[noreturn]] void bad(const std::string& s) {throw Failure{FlatZincCompileStatus::InvalidInput,s,{}};}
[[noreturn]] void unsupported(const std::string& s) {throw Failure{FlatZincCompileStatus::Unsupported,s,{}};}
[[noreturn]] void limit(const std::string& s) {throw Failure{FlatZincCompileStatus::ResourceLimit,s,{}};}
I plus(I a,I b) {
  if((b>0&&a>std::numeric_limits<I>::max()-b)||(b<0&&a<std::numeric_limits<I>::min()-b))
    unsupported("integer arithmetic exceeds the exact compiler range");
  return a+b;
}
I neg(I a) {if(a==std::numeric_limits<I>::min())unsupported("integer negation overflow");return -a;}
I times(I a,I b) {
  if(!a||!b)return 0;
  if((a==-1&&b==std::numeric_limits<I>::min())||(b==-1&&a==std::numeric_limits<I>::min()))unsupported("integer product overflow");
  if(a>0?(b>0?a>std::numeric_limits<I>::max()/b:b<std::numeric_limits<I>::min()/a):
         (b>0?a<std::numeric_limits<I>::min()/b:a<std::numeric_limits<I>::max()/b))unsupported("integer product overflow");
  return a*b;
}
double number(I a) {if(a < -exact_limit || a > exact_limit)unsupported("integer cannot be represented exactly as a model number");return static_cast<double>(a);}
using Key=std::pair<int,std::size_t>;
Key key(F::Reference r) {
  switch(r.type){case F::Type::Integer:return {0,r.index};case F::Type::Boolean:return {1,r.index};
    case F::Type::Float:case F::Type::Set:unsupported("FlatZinc float and set variables are not supported by this compiler");}
  bad("invalid variable namespace");
}
struct Meter {
  const FlatZincCompileOptions& options; SolveBudget budget; std::size_t work=0;
  static SolveOptions solve_options(const FlatZincCompileOptions& o) {SolveOptions s;s.time_limit_seconds=o.time_limit_seconds;s.cancellation=o.cancellation;return s;}
  explicit Meter(const FlatZincCompileOptions& o):options(o),budget(solve_options(o)) {}
  void check() const {
    if(budget.cancelled())throw Failure{FlatZincCompileStatus::Cancelled,"FlatZinc compilation cancelled",{}};
    if(budget.time_limit_reached())throw Failure{FlatZincCompileStatus::TimeLimit,"FlatZinc compilation deadline",{}};
  }
  void add(std::size_t n=1) {check();if(n>options.max_work-work)limit("FlatZinc compiler work limit");work+=n;}
};
void shape(const F::Value& v,Meter& m,std::size_t depth=0) {
  if(depth>m.options.max_value_depth)limit("FlatZinc value nesting limit");
  m.add();m.add(v.text.size());m.add(v.set.values.size());
  switch(v.kind){case F::ValueKind::Integer:case F::ValueKind::Boolean:case F::ValueKind::Float:
    case F::ValueKind::Set:case F::ValueKind::Reference:case F::ValueKind::Array:case F::ValueKind::Atom:
    case F::ValueKind::String:case F::ValueKind::Call:break;default:bad("invalid captured value tag");}
  // Account for even inactive public fields before copying untrusted records.
  for(const auto& item:v.elements)shape(item,m,depth+1);
}
void constraint_shape(const F::Constraint& c,Meter& m) {
  m.add();m.add(c.id.size());m.add(c.location.source.size());
  for(const auto& a:c.arguments)shape(a,m);
  for(const auto& a:c.annotations)shape(a,m);
}
void all_shapes(const F::Records& r,Meter& m) {
  if(r.raw_variables.size()>m.options.max_variables||r.variables.size()>m.options.max_variables)limit("FlatZinc variable limit");
  for(const auto* vars:{&r.raw_variables,&r.variables})for(const auto& v:*vars) {
    m.add();m.add(v.name.size());m.add(v.domain.integers.values.size());shape(v.value,m);
  }
  for(const auto* rows:{&r.raw_domains,&r.raw_constraints,&r.domains,&r.constraints}) {
    if(rows->size()>m.options.max_constraints)limit("FlatZinc constraint record limit");
    for(const auto& c:*rows)constraint_shape(c,m);
  }
  m.add(r.coverage.size());m.add(r.source.size());shape(r.solve.objective,m);m.add(r.solve.location.source.size());
  for(const auto& a:r.solve.annotations)shape(a,m);
  for(const auto& a:r.declaration_annotations) {
    m.add();m.add(a.name.size());m.add(a.location.source.size());for(const auto& x:a.annotations)shape(x,m);
  }
  for(const auto& o:r.output){m.add();m.add(o.name.size());shape(o.expression,m);}
}
std::pair<I,I> interval(const F::SetLiteral& s,Meter* meter=nullptr) {
  if(s.interval){number(s.lower);number(s.upper);return {s.lower,s.upper};}
  auto values=s.values;
  if(meter)meter->add(values.size());
  if(values.empty())return {1,0};
  std::sort(values.begin(),values.end());values.erase(std::unique(values.begin(),values.end()),values.end());
  for(std::size_t i=0;i<values.size();++i){number(values[i]);if(i&&values[i]!=plus(values[i-1],1))unsupported("noncontiguous FlatZinc output-array index sets are unsupported");}
  return {values.front(),values.back()};
}
const std::vector<F::Value>& array(const F::Value& v) {if(v.kind!=F::ValueKind::Array)bad("expected a captured array");return v.elements;}
I literal(const F::Value& v) {if(v.kind!=F::ValueKind::Integer)bad("expected an integer literal");number(v.integer);return v.integer;}
void arity(const F::Constraint& c,std::size_t n){if(c.arguments.size()!=n)bad("wrong arity for "+c.id);}
void annotation(const F::Value& v,bool declaration=false) {
  if(v.kind!=F::ValueKind::Atom&&v.kind!=F::ValueKind::Call)unsupported("unrecognized FlatZinc annotation form");
  const auto& n=v.text;
  // MiniZinc 2.10.1 stdlib/stdlib_ann.mzn, context annotations:
  // https://github.com/MiniZinc/libminizinc/blob/2.10.1/share/minizinc/std/stdlib/stdlib_ann.mzn
  // Compiler context markers do not change the posted relation or its exact
  // original-source check. Admit only the named atoms, never unknown calls.
  if(n=="var_is_introduced"||n=="is_defined_var"||n=="domain"||n=="bounds"||n=="boundsD"||n=="boundsR"||n=="boundsZ"||
     n=="ctx_pos"||n=="ctx_neg"||n=="ctx_mix") {
    if(v.kind!=F::ValueKind::Atom||!v.elements.empty())bad("malformed marker annotation");return;
  }
  if(n=="defines_var") {if(v.kind!=F::ValueKind::Call||v.elements.size()!=1||v.elements[0].kind!=F::ValueKind::Reference)bad("malformed defines_var annotation");return;}
  if(n=="mzn_path") {if(v.kind!=F::ValueKind::Call||v.elements.size()!=1||v.elements[0].kind!=F::ValueKind::String)bad("malformed mzn_path annotation");return;}
  if(declaration&&n=="output_var") {if(v.kind!=F::ValueKind::Atom||!v.elements.empty())bad("malformed output_var annotation");return;}
  if(declaration&&n=="output_array") {
    if(v.kind!=F::ValueKind::Call||v.elements.size()!=1)bad("malformed output_array annotation");
    const auto& dimensions=array(v.elements[0]);if(dimensions.empty())bad("output_array needs at least one dimension");
    for(const auto& d:dimensions){if(d.kind!=F::ValueKind::Set)bad("output dimension is not an integer set");interval(d.set);}
    return;
  }
  unsupported("unhandled FlatZinc annotation: "+n);
}
struct Bounds {
  std::optional<I> lower,upper; bool empty=false;
  // A finite member list is never replaced by its hull until contiguity is proved.
  // Intervals stay symbolic, so a sparse set can span the full exact source range.
  std::optional<std::vector<I>> members;
  void intersect(I lo,I hi){empty=empty||lo>hi;lower=lower?std::max(*lower,lo):lo;upper=upper?std::min(*upper,hi):hi;if(*lower>*upper)empty=true;}
  void intersect(const F::SetLiteral& input,Meter& meter) {
    if(input.interval){number(input.lower);number(input.upper);intersect(input.lower,input.upper);return;}
    if(input.values.size()>meter.options.max_nonzeros)limit("finite domain member storage limit");
    meter.add(input.values.size());
    for(const auto value:input.values){meter.check();number(value);}
    auto allowed=input.values;
    // Comparisons and filtering are charged, not just the final member count.
    // Any thrown limit discards private compiler state, never the source records.
    std::sort(allowed.begin(),allowed.end(),[&](I a,I b){meter.add();return a<b;});
    std::size_t unique=0;
    for(const auto value:allowed){meter.add();if(!unique||allowed[unique-1]!=value)allowed[unique++]=value;}
    allowed.resize(unique);
    if(members) {
      const auto reserve=std::min(members->size(),allowed.size());meter.add(reserve);
      std::vector<I> common;common.reserve(reserve);
      std::size_t i=0,j=0;
      while(i<members->size()&&j<allowed.size()){
        meter.add();const auto a=(*members)[i],b=allowed[j];
        if(a<b)++i;else if(b<a)++j;else{common.push_back(a);++i;++j;}
      }
      members=std::move(common);
    } else members=std::move(allowed);
    if(members->empty())intersect(1,0);
    else intersect(members->front(),members->back());
  }
  void finish(Meter& meter) {
    if(!members)return;
    std::size_t kept=0;
    for(const auto value:*members){meter.add();if(!empty&&(!lower||value>=*lower)&&(!upper||value<=*upper))(*members)[kept++]=value;}
    members->resize(kept);
    if(!kept){intersect(1,0);members.reset();return;}
    intersect(members->front(),members->back());
    bool contiguous=true;
    for(std::size_t i=1;i<kept;++i){meter.add();if((*members)[i]!=plus((*members)[i-1],1))contiguous=false;}
    if(contiguous)members.reset();
  }
};
struct Form {std::map<std::size_t,I> coefficient; I constant=0;};
void combine(Form& a,const Form& b,I multiplier=1) {
  a.constant=plus(a.constant,times(b.constant,multiplier));
  for(const auto& e:b.coefficient) {auto& value=a.coefficient[e.first];value=plus(value,times(e.second,multiplier));if(!value)a.coefficient.erase(e.first);}
}
bool identifier(const std::string& s) {
  if(s.empty())return false;
  const auto first=[](unsigned char c){return(c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_';};
  if(!first(s[0]))return false;
  for(unsigned char c:s)if(!first(c)&&!(c>='0'&&c<='9'))return false;
  return true;
}
const F::Value* output_array(const F::Records& source,const std::string& name) {
  const F::Value* found=nullptr;
  for(const auto& d:source.declaration_annotations)if(d.name==name)for(const auto& a:d.annotations)if(a.text=="output_array") {
    if(found)bad("duplicate output_array annotation");found=&a;
  }
  return found;
}
const std::vector<F::Value>& output_values(const F::Value& value) {
  const auto& a=array(value);
  // Legacy printer layout contains fixed string fragments around one value array.
  if(std::all_of(a.begin(),a.end(),[](const F::Value& x){return x.kind==F::ValueKind::Reference||x.kind==F::ValueKind::Integer||x.kind==F::ValueKind::Boolean;}))return a;
  const std::vector<F::Value>* found=nullptr;
  for(const auto& v:a){if(v.kind==F::ValueKind::Array){if(found)bad("ambiguous output value arrays");found=&v.elements;}else if(v.kind!=F::ValueKind::String)bad("unsupported output layout");}
  if(!found)bad("output layout has no typed values");return *found;
}
}

struct FlatZincCompiler {
  const F::Records& source; const FlatZincCompileOptions& options; Meter meter;
  std::map<Key,std::size_t> position;
  std::vector<std::size_t> parent;
  std::vector<Bounds> domains;
  std::vector<Variable> handles;
  Model model;std::size_t nonzeros=0,rows=0,variables=0;
  std::map<I,Variable> constants;
  explicit FlatZincCompiler(const F::Records& s,const FlatZincCompileOptions& o):source(s),options(o),meter(o) {}
  std::size_t lookup(F::Reference ref) const {auto p=position.find(key(ref));if(p==position.end())bad("unknown captured variable reference");return p->second;}
  std::size_t representative(std::size_t n) {
    auto root=n;std::size_t steps=0;
    while(parent[root]!=root){meter.add();if(++steps>parent.size())bad("cyclic captured variable alias");root=parent[root];}
    while(parent[n]!=n){meter.add();auto next=parent[n];parent[n]=root;n=next;}return root;
  }
  void typed(const F::Value& v,F::Type type) const {
    if(v.kind==F::ValueKind::Reference){if(v.reference.type!=type)bad("captured operand has the wrong variable namespace");lookup(v.reference);return;}
    if(type==F::Type::Integer&&v.kind==F::ValueKind::Integer){number(v.integer);return;}
    if(type==F::Type::Boolean&&v.kind==F::ValueKind::Boolean)return;
    bad("captured operand has the wrong literal type");
  }
  Form term(const F::Value& v,F::Type type) {
    typed(v,type);meter.add();Form f;
    if(v.kind==F::ValueKind::Reference)f.coefficient.emplace(handles[lookup(v.reference)].id,1);
    else f.constant=type==F::Type::Integer?v.integer:static_cast<I>(v.boolean);
    return f;
  }
  Form difference(const F::Value& a,const F::Value& b,F::Type type) {auto f=term(a,type);combine(f,term(b,type),-1);return f;}
  void post(Form f,std::optional<I> lo,std::optional<I> hi) {
    meter.add();if(rows==options.max_constraints)limit("generated FlatZinc row limit");
    if(f.coefficient.size()>options.max_nonzeros-nonzeros)limit("generated FlatZinc nonzero limit");
    meter.add(f.coefficient.size());std::vector<Term> terms;terms.reserve(f.coefficient.size());
    for(const auto& e:f.coefficient)terms.push_back({{model.id(),e.first},number(e.second)});
    const auto low=lo?number(plus(*lo,neg(f.constant))):-std::numeric_limits<double>::infinity();
    const auto high=hi?number(plus(*hi,neg(f.constant))):std::numeric_limits<double>::infinity();
    model.add_row(terms,low,high);++rows;nonzeros+=terms.size();
  }
  void capacity(std::size_t added_variables,std::size_t added_rows,std::size_t added_nonzeros) {
    meter.add();
    if(added_variables>options.max_variables-variables)limit("generated FlatZinc variable limit");
    if(added_rows>options.max_constraints-rows)limit("generated FlatZinc constraint limit");
    if(added_nonzeros>options.max_nonzeros-nonzeros)limit("generated FlatZinc nonzero limit");
  }
  Variable global_term(const F::Value& value) {
    typed(value,F::Type::Integer);meter.add();
    if(value.kind==F::ValueKind::Reference)return handles[lookup(value.reference)];
    const auto found=constants.find(value.integer);if(found!=constants.end())return found->second;
    capacity(1,0,0);auto v=model.add_integer(number(value.integer),number(value.integer));++variables;
    constants.emplace(value.integer,v);return v;
  }
  I fixed_integer(const F::Value& value,const char* role) {
    typed(value,F::Type::Integer);meter.add();
    if(value.kind==F::ValueKind::Integer)return value.integer;
    const auto& original=domains[representative(lookup(value.reference))];
    // A contradictory domain uses a dummy model slot; it proves no fixed
    // source parameter. Ordinary equality rows are never propagated here.
    if(original.empty||!original.lower||!original.upper||*original.lower!=*original.upper)
      unsupported(std::string("cumulative ")+role+" must be a literal or an original singleton integer domain");
    return *original.lower;
  }
  void implication(const F::Value& guard,bool active,Form form,std::optional<I> lo,std::optional<I> hi) {
    typed(guard,F::Type::Boolean);meter.add();
    if(guard.kind==F::ValueKind::Boolean){if(guard.boolean==active)post(std::move(form),lo,hi);return;}
    const auto control=handles[lookup(guard.reference)];
    const auto& domain=model.variable(control);
    if(domain.lower==domain.upper){if((domain.lower==1)==active)post(std::move(form),lo,hi);return;}
    // Reserve the maximum footprint before the transactional helper allocates:
    // one gate, its two-entry equality, and one row per finite side.
    const std::size_t sides=std::size_t(bool(lo))+std::size_t(bool(hi));
    if(form.coefficient.size()>options.max_nonzeros)limit("indicator expression exceeds nonzero limit");
    const std::size_t width=form.coefficient.size()+1;
    if(width>std::numeric_limits<std::size_t>::max()/sides || sides*width>std::numeric_limits<std::size_t>::max()-2)
      limit("indicator nonzero count overflow");
    capacity(1,1+sides,2+sides*width);meter.add(2+sides*width);
    std::vector<Term> terms;terms.reserve(form.coefficient.size());
    for(const auto& e:form.coefficient)terms.push_back({{model.id(),e.first},number(e.second)});
    const double lower=lo?number(plus(*lo,neg(form.constant))):-std::numeric_limits<double>::infinity();
    const double upper=hi?number(plus(*hi,neg(form.constant))):std::numeric_limits<double>::infinity();
    const auto added=add_indicator(model,control,active,terms,lower,upper);
    if(added.inactive_gate)++variables;
    rows+=added.rows.size();for(const auto r:added.rows)nonzeros+=model.row(r).terms.size();
  }
  void restrict_domain(const F::Constraint& c) {
    if(c.id!="int_in")return;arity(c,2);
    if(c.arguments[1].kind!=F::ValueKind::Set)bad("int_in requires an integer set literal");
    const auto& value=c.arguments[0];
    if(value.kind==F::ValueKind::Reference)domains[representative(lookup(value.reference))].intersect(c.arguments[1].set,meter);
    else {
      if(value.kind!=F::ValueKind::Integer&&value.kind!=F::ValueKind::Boolean)bad("int_in has a noninteger operand");
      // Admit every set value even when the eventual constant predicate is false.
      Bounds checked;checked.intersect(c.arguments[1].set,meter);
    }
  }
  void declarations() {
    parent.resize(source.raw_variables.size());std::iota(parent.begin(),parent.end(),0);domains.resize(parent.size());handles.resize(parent.size());
    for(std::size_t i=0;i<parent.size();++i){meter.add();if(!position.emplace(key(source.raw_variables[i].reference),i).second)bad("duplicate captured variable slot");}
    for(std::size_t i=0;i<parent.size();++i){const auto& v=source.raw_variables[i];if(v.alias){if(v.target.type!=v.reference.type)bad("cross-namespace alias");parent[i]=lookup(v.target);if(parent[i]==i)bad("self-referencing captured alias");}}
    for(std::size_t i=0;i<parent.size();++i){meter.add();const auto root=representative(i);const auto& v=source.raw_variables[i];
      if(v.reference.type==F::Type::Boolean)domains[root].intersect(0,1);
      if(v.alias)continue;
      if(v.domain.present)domains[root].intersect(v.domain.integers,meter);
      if(v.assigned){typed(v.value,v.reference.type);if(v.value.kind==F::ValueKind::Reference)bad("assigned variable must contain a literal");I value=v.reference.type==F::Type::Integer?v.value.integer:static_cast<I>(v.value.boolean);domains[root].intersect(value,value);}
    }
    for(const auto* input:{&source.raw_domains,&source.raw_constraints})for(const auto& c:*input) {
      try {restrict_domain(c);}catch(Failure& f){f.location=c.location;throw;}
    }
    for(std::size_t i=0;i<parent.size();++i)if(representative(i)==i) {
      auto& d=domains[i];const auto& v=source.raw_variables[i];d.finish(meter);
      if(!d.lower||!d.upper)unsupported("FlatZinc integer variable has no finite explicit domain");
      const auto lo=d.empty?0:*d.lower,hi=d.empty?0:*d.upper;
      // MiniZinc commonly emits binary decisions as integer 0..1 declarations.
      // Retain their source integer type in the owning records/output mapping,
      // while exposing the equivalent binary domain to native heuristics.
      const bool binary=v.reference.type==F::Type::Boolean || (!d.empty && lo>=0 && hi<=1);
      handles[i]=model.add_variable(binary?VariableType::Binary:VariableType::Integer,number(lo),number(hi),v.name);
      ++variables;
      if(d.empty)post({},1,1);
      else if(d.members) {
        if(d.members->size()==std::numeric_limits<std::size_t>::max())limit("finite domain cell count overflow");
        const auto cells=d.members->size()+1;capacity(0,1,cells);meter.add(cells);
        std::vector<std::vector<I>> tuples;tuples.reserve(d.members->size());
        for(const auto value:*d.members){meter.add();tuples.push_back({value});}
        add_table(model,{handles[i]},tuples);++rows;nonzeros+=cells;
      }
    }
    for(std::size_t i=0;i<parent.size();++i)handles[i]=handles[representative(i)];
  }
  void annotation_references(const F::Value& v) const {
    if(v.kind==F::ValueKind::Reference)lookup(v.reference);
    for(const auto& x:v.elements)annotation_references(x);
  }
  void compile_constraint(const F::Constraint& c) {
    meter.add();for(const auto& a:c.annotations){annotation(a);annotation_references(a);}
    const auto& id=c.id;const auto& a=c.arguments;
    if(id=="int_le_reif"||id=="int_le_imp"||id=="int_eq_imp") {
      arity(c,3);auto f=difference(a[0],a[1],F::Type::Integer);typed(a[2],F::Type::Boolean);
      implication(a[2],true,f,id=="int_eq_imp"?std::optional<I>(0):std::nullopt,0);
      if(id=="int_le_reif")implication(a[2],false,std::move(f),1,{});
      return;
    }
    if(id=="int_lin_le_reif"||id=="int_lin_le_imp") {
      arity(c,4);const auto& coefficients=array(a[0]);const auto& operands=array(a[1]);
      if(coefficients.size()!=operands.size())bad("linear coefficient/operand lengths differ");
      Form f;for(std::size_t i=0;i<coefficients.size();++i)
        combine(f,term(operands[i],F::Type::Integer),literal(coefficients[i]));
      const I rhs=literal(a[2]);typed(a[3],F::Type::Boolean);
      implication(a[3],true,f,{},rhs);
      if(id=="int_lin_le_reif")implication(a[3],false,std::move(f),plus(rhs,1),{});
      return;
    }
    if(id=="gecode_table_int") {
      arity(c,2);const auto& values=array(a[0]);const auto& flat=array(a[1]);
      if(values.empty()) {
        if(!flat.empty())bad("a zero-arity table cannot have relation cells");
        unsupported("zero-arity flat tables do not preserve the original tuple count");
      }
      if(flat.size()%values.size())bad("flat table cell count is not divisible by its arity");
      if(values.size()>options.max_nonzeros||flat.size()>options.max_nonzeros-values.size())limit("table payload cell limit");
      const auto cells=values.size()+flat.size();capacity(0,1,cells);meter.add(cells);
      // Validate all parameter cells before creating private literal slots.
      for(const auto& entry:flat){meter.add();literal(entry);}
      std::vector<Variable> arguments;arguments.reserve(values.size());
      for(const auto& value:values)arguments.push_back(global_term(value));
      const auto tuple_count=flat.size()/values.size();meter.add(tuple_count);
      std::vector<std::vector<I>> tuples;tuples.reserve(tuple_count);
      for(std::size_t offset=0;offset<flat.size();offset+=values.size()) {
        meter.add(values.size());std::vector<I> tuple;tuple.reserve(values.size());
        for(std::size_t j=0;j<values.size();++j)tuple.push_back(flat[offset+j].integer);
        tuples.push_back(std::move(tuple));
      }
      add_table(model,arguments,tuples);++rows;nonzeros+=cells;return;
    }
    if(id=="gecode_regular") {
      arity(c,6);const auto& word=array(a[0]);const I states=literal(a[1]),symbols=literal(a[2]);
      const auto& flat=array(a[3]);const I initial=literal(a[4]);
      if(states<1||symbols<1)bad("regular state and alphabet counts must be positive");
      if(initial<1||initial>states)bad("regular initial state is outside 1..Q");
      if(a[5].kind!=F::ValueKind::Set)bad("regular final states must be a literal integer set");
      const I expected=times(states,symbols);
      if(static_cast<std::uint64_t>(expected)>std::numeric_limits<std::size_t>::max())
        unsupported("regular transition matrix count exceeds native size arithmetic");
      if(flat.size()!=static_cast<std::size_t>(expected))bad("regular transition cell count must equal Q*S");
      const auto& final_set=a[5].set;std::size_t final_count=0;
      if(final_set.interval){
        number(final_set.lower);number(final_set.upper);
        if(final_set.lower<=final_set.upper){
          if(final_set.lower<1||final_set.upper>states)bad("regular final state is outside 1..Q");
          const auto count=static_cast<std::uint64_t>(plus(plus(final_set.upper,neg(final_set.lower)),1));
          if(count>std::numeric_limits<std::size_t>::max())unsupported("regular final set exceeds native size arithmetic");
          final_count=static_cast<std::size_t>(count);
        }
      }else{
        final_count=final_set.values.size();
        for(const auto state:final_set.values){meter.add();number(state);if(state<1||state>states)bad("regular final state is outside 1..Q");}
      }
      // Bound retained input cells and worst-case expanded sparse edges before
      // any payload allocation or parameter-set interval expansion.
      std::size_t cells=0;
      for(const auto count:{std::size_t(3),word.size(),flat.size(),final_count}){
        if(count>options.max_nonzeros-cells)limit("regular payload cell limit");cells+=count;
      }
      capacity(0,1,cells);meter.add(cells);
      std::size_t edge_count=0;
      for(const auto& value:flat){meter.add();const I target=literal(value);if(target<0||target>states)bad("regular transition target is outside 0..Q");if(target)++edge_count;}
      if(edge_count>(options.max_nonzeros-cells)/3)limit("regular sparse transition cell limit");
      cells+=3*edge_count;capacity(0,1,cells);meter.add(3*edge_count);
      for(const auto& value:word){meter.add();typed(value,F::Type::Integer);}
      std::vector<RegularTransition> transitions;transitions.reserve(edge_count);
      for(std::size_t i=0;i<flat.size();++i){meter.add();const auto target=flat[i].integer;if(target)
        transitions.push_back({static_cast<std::uint64_t>(i/static_cast<std::size_t>(symbols)),
          static_cast<I>(i%static_cast<std::size_t>(symbols))+1,static_cast<std::uint64_t>(target-1)});
      }
      std::vector<std::uint64_t> finals;finals.reserve(final_count);
      for(std::size_t i=0;i<final_count;++i){meter.add();const I state=final_set.interval?plus(final_set.lower,static_cast<I>(i)):final_set.values[i];finals.push_back(static_cast<std::uint64_t>(state-1));}
      std::vector<Variable> variables;variables.reserve(word.size());for(const auto& value:word)variables.push_back(global_term(value));
      add_regular(model,variables,static_cast<std::uint64_t>(states),static_cast<std::uint64_t>(initial-1),transitions,finals);
      ++rows;nonzeros+=cells;return;
    }
    if(id=="gecode_circuit") {
      arity(c,2);const I offset=literal(a[0]);const auto& values=array(a[1]);
      if(values.empty())bad("gecode_circuit requires a nonempty successor array");
      if(offset<0)unsupported("gecode_circuit requires a nonnegative literal offset");
      if(values.size()-1>static_cast<std::uint64_t>(std::numeric_limits<I>::max()))
        unsupported("circuit index count exceeds exact integer arithmetic");
      number(plus(offset,static_cast<I>(values.size()-1)));
      capacity(0,1,values.size());meter.add(values.size());
      // Every position is a source node, including repeated handles/literals.
      // Do not deduplicate successors or infer a base from output dimensions.
      for(const auto& value:values){meter.add();typed(value,F::Type::Integer);}
      std::vector<Variable> successors;successors.reserve(values.size());
      for(const auto& value:values)successors.push_back(global_term(value));
      add_circuit(model,successors,offset);++rows;nonzeros+=successors.size();return;
    }
    if(id=="gecode_cumulatives"||id=="cumulatives") {
      if(a.size()==6||a.size()==7)unsupported("multi-machine cumulative signatures are not supported");
      arity(c,4);const auto& starts=array(a[0]);const auto& duration_values=array(a[1]);const auto& height_values=array(a[2]);
      if(starts.size()!=duration_values.size()||starts.size()!=height_values.size())
        bad("cumulative starts, durations and heights must have equal lengths");
      if(!options.max_nonzeros||starts.size()>(options.max_nonzeros-1)/3)
        limit("cumulative payload cell limit");
      const auto cells=3*starts.size()+1;capacity(0,1,cells);meter.add(cells);
      const I bound=fixed_integer(a[3],"capacity");
      if(bound<0)unsupported("cumulative capacity must be nonnegative in the fixed-data interface");
      std::vector<I> durations,heights;durations.reserve(starts.size());heights.reserve(starts.size());
      for(std::size_t i=0;i<starts.size();++i){
        typed(starts[i],F::Type::Integer);meter.add();
        const auto duration=fixed_integer(duration_values[i],"duration"),height=fixed_integer(height_values[i],"height");
        if(duration<0||height<0)bad("cumulative durations and heights must be nonnegative");
        durations.push_back(duration);heights.push_back(height);
      }
      std::vector<Variable> arguments;arguments.reserve(starts.size());
      for(const auto& start:starts)arguments.push_back(global_term(start));
      add_cumulative(model,arguments,durations,heights,bound);++rows;nonzeros+=cells;return;
    }
    if(id=="all_different_int") {
      arity(c,1);const auto& values=array(a[0]);capacity(0,1,values.size());meter.add(values.size());
      std::vector<Variable> terms;terms.reserve(values.size());for(const auto& v:values)terms.push_back(global_term(v));
      add_all_different(model,terms);++rows;nonzeros+=terms.size();return;
    }
    if(id=="array_int_element"||id=="array_var_int_element") {
      arity(c,3);const auto& values=array(a[1]);
      if(id=="array_int_element")for(const auto& v:values)
        if(v.kind!=F::ValueKind::Integer)bad("array_int_element requires a parameter integer array");
      if(values.size()>options.max_nonzeros || values.size()>std::numeric_limits<std::size_t>::max()-2)
        limit("element expression exceeds nonzero limit");
      capacity(0,1,values.size()+2);meter.add(values.size()+2);
      auto index=global_term(a[0]),result=global_term(a[2]);
      std::vector<Variable> terms;terms.reserve(values.size());for(const auto& v:values)terms.push_back(global_term(v));
      add_element(model,index,terms,result,1);++rows;nonzeros+=terms.size()+2;return;
    }
    if(id=="int_in") {
      arity(c,2);if(a[1].kind!=F::ValueKind::Set)bad("int_in requires a set");
      const auto type=a[0].kind==F::ValueKind::Boolean?F::Type::Boolean:a[0].kind==F::ValueKind::Reference?a[0].reference.type:F::Type::Integer;
      const auto f=term(a[0],type);
      // Every reference restriction was intersected exactly before declarations.
      // Its raw predicate remains in the owning source and independent checker.
      if(a[0].kind==F::ValueKind::Reference)return;
      bool member=false;const auto& set=a[1].set;
      if(set.interval)member=f.constant>=set.lower&&f.constant<=set.upper;
      else for(const auto value:set.values){meter.add();if(value==f.constant)member=true;}
      if(!member)post({},1,1);return;
    }
    if(id=="int_eq"||id=="int_le"||id=="int_lt"||id=="int_ge"||id=="int_gt"||id=="bool_eq"||id=="bool_le") {
      arity(c,2);auto f=difference(a[0],a[1],id.rfind("bool_",0)==0?F::Type::Boolean:F::Type::Integer);
      if(id=="int_eq"||id=="bool_eq")post(f,0,0);
      else if(id=="int_le"||id=="bool_le")post(f,{},0);
      else if(id=="int_lt")post(f,{},-1);
      else if(id=="int_ge")post(f,0,{});else post(f,1,{});return;
    }
    if(id=="int_plus"||id=="int_minus"||id=="bool2int"||id=="bool_not") {
      if(id=="int_plus"||id=="int_minus"){arity(c,3);auto f=term(a[0],F::Type::Integer);combine(f,term(a[1],F::Type::Integer),id=="int_plus"?1:-1);combine(f,term(a[2],F::Type::Integer),-1);post(f,0,0);}
      else {arity(c,2);auto f=term(a[0],F::Type::Boolean);combine(f,term(a[1],id=="bool2int"?F::Type::Integer:F::Type::Boolean),id=="bool2int"?-1:1);post(f,id=="bool2int"?0:1,id=="bool2int"?0:1);}return;
    }
    if(id=="int_lin_eq"||id=="int_lin_le"||id=="bool_lin_eq"||id=="bool_lin_le") {
      arity(c,3);const auto& coeff=array(a[0]);const auto& values=array(a[1]);if(coeff.size()!=values.size())bad("linear coefficient/operand lengths differ");
      Form f;for(std::size_t i=0;i<coeff.size();++i)combine(f,term(values[i],id.rfind("bool_",0)==0?F::Type::Boolean:F::Type::Integer),literal(coeff[i]));
      if(id!="bool_lin_eq"&&a[2].kind!=F::ValueKind::Integer)bad("this FlatZinc linear RHS must be a parameter literal");
      combine(f,term(a[2],F::Type::Integer),-1);post(f,id.find("_eq")!=std::string::npos?std::optional<I>(0):std::nullopt,0);return;
    }
    if(id=="bool_and"||id=="bool_or"||id=="array_bool_and"||id=="array_bool_or") {
      const bool is_array=id.rfind("array_",0)==0,and_op=id.find("and")!=std::string::npos;
      arity(c,is_array?2:3);const auto values=is_array?array(a[0]):std::vector<F::Value>{a[0],a[1]};auto result=term(a[is_array?1:2],F::Type::Boolean);Form sum;
      for(const auto& value:values){auto operand=term(value,F::Type::Boolean);combine(sum,operand);auto f=result;combine(f,operand,-1);if(and_op)post(f,{},0);else post(f,0,{});}
      auto last=result;combine(last,sum,-1);
      if(and_op)post(last,plus(1,neg(static_cast<I>(values.size()))),{});else post(last,{},0);return;
    }
    if(id=="bool_clause") {
      arity(c,2);const auto& positive=array(a[0]);const auto& negative=array(a[1]);Form f;
      for(const auto& x:positive)combine(f,term(x,F::Type::Boolean));
      for(const auto& x:negative)combine(f,term(x,F::Type::Boolean),-1);
      post(f,plus(1,neg(static_cast<I>(negative.size()))),{});return;
    }
    unsupported("unsupported complete FlatZinc predicate: "+id);
  }
  void outputs() {
    std::set<std::string> names;
    for(const auto& d:source.declaration_annotations)for(const auto& a:d.annotations){annotation(a,true);annotation_references(a);}
    for(const auto& o:source.output) {
      if(!identifier(o.name)||!names.insert(o.name).second)bad("invalid or duplicate FlatZinc output name");
      const auto scalar=[&](const F::Value& value){
        if(value.kind==F::ValueKind::Reference)lookup(value.reference);
        else if(value.kind==F::ValueKind::Integer)number(value.integer);
        else if(value.kind!=F::ValueKind::Boolean)unsupported("unsupported FlatZinc output value");
      };
      if(const auto* ann=output_array(source,o.name)) {
        const auto& dimensions=array(ann->elements[0]);I count=1;
        for(const auto& d:dimensions){auto range=interval(d.set,&meter);count=times(count,range.first>range.second?0:plus(plus(range.second,neg(range.first)),1));}
        const auto& values=output_values(o.expression);
        if(count<0||static_cast<std::uint64_t>(count)!=values.size())bad("FlatZinc output array dimensions do not match its values");
        for(const auto& v:values)scalar(v);
      }else scalar(o.expression);
    }
    for(const auto& d:source.declaration_annotations)for(const auto& a:d.annotations)
      if((a.text=="output_var"||a.text=="output_array")&&!names.count(d.name))bad("requested FlatZinc output is missing from the captured plan");
  }
  CompiledFlatZinc run() {
    all_shapes(source,meter);meter.check();declarations();outputs();
    // Search controls are not implemented by recording their text.
    if(!source.solve.annotations.empty())unsupported("FlatZinc search annotations are not supported by the explicit compiler");
    for(const auto* input:{&source.raw_domains,&source.raw_constraints})for(const auto& c:*input) {
      try {compile_constraint(c);}catch(Failure& f){f.location=c.location;throw;}
    }
    switch(source.solve.method){
      case F::Method::Satisfy:if(source.solve.has_objective)bad("satisfaction request has an objective");model.minimize({});break;
      case F::Method::Minimize:case F::Method::Maximize:{
        if(!source.solve.has_objective)bad("optimization request lacks its original objective");
        auto f=term(source.solve.objective,F::Type::Integer);std::vector<Term> terms;
        for(const auto& e:f.coefficient)terms.push_back({{model.id(),e.first},number(e.second)});
        model.set_objective(terms,source.solve.method==F::Method::Minimize?ObjectiveSense::Minimize:ObjectiveSense::Maximize,number(f.constant));break;
      }
      default:bad("invalid FlatZinc solve method");
    }
    // Reserve copy work before publishing owned source/model data.
    meter.add(meter.work);auto data=std::make_shared<FlatZincArtifact>();data->source=source;data->model=model.snapshot();
    data->mapping.reserve(handles.size());for(std::size_t i=0;i<handles.size();++i)data->mapping.push_back({source.raw_variables[i].reference,handles[i]});
    meter.check();return CompiledFlatZinc(std::move(data));
  }
};

namespace {
struct SourceEvaluator {
  std::map<Key,I> values;
  I value(const F::Value& v) const {
    switch(v.kind){
      case F::ValueKind::Integer:return v.integer;
      case F::ValueKind::Boolean:return v.boolean?1:0;
      case F::ValueKind::Reference:{auto p=values.find(key(v.reference));if(p==values.end())bad("missing original variable value");return p->second;}
      default:bad("noninteger original expression");
    }
  }
  bool member(I x,const F::SetLiteral& set) const {
    return set.interval?(x>=set.lower&&x<=set.upper):std::find(set.values.begin(),set.values.end(),x)!=set.values.end();
  }
  bool relation(const F::Constraint& c) const {
    const auto& a=c.arguments;const auto& id=c.id;
    if(id=="int_le_reif")return (value(a[0])<=value(a[1]))==(value(a[2])!=0);
    if(id=="int_le_imp")return value(a[2])==0||value(a[0])<=value(a[1]);
    if(id=="int_eq_imp")return value(a[2])==0||value(a[0])==value(a[1]);
    if(id=="int_lin_le_reif"||id=="int_lin_le_imp") {
      if(id=="int_lin_le_imp"&&value(a[3])==0)return true;
      I sum=0;for(std::size_t i=0;i<a[0].elements.size();++i)
        sum=plus(sum,times(a[0].elements[i].integer,value(a[1].elements[i])));
      const bool holds=sum<=value(a[2]);return id=="int_lin_le_imp"?holds:holds==(value(a[3])!=0);
    }
    if(id=="gecode_table_int") {
      const auto& original=a[0].elements;const auto& relation=a[1].elements;
      if(original.empty()||relation.size()%original.size())bad("unadmitted original flat table shape");
      for(std::size_t offset=0;offset<relation.size();offset+=original.size()) {
        bool equal=true;
        for(std::size_t j=0;j<original.size();++j)
          if(value(original[j])!=relation[offset+j].integer){equal=false;break;}
        if(equal)return true;
      }
      return false;
    }
    if(id=="gecode_regular") {
      // Follow the original one-based row-major transition table. In particular,
      // zero is the source failure state, not the typed API's ordinary state 0.
      const I states=a[1].integer,symbols=a[2].integer;I state=a[4].integer;
      const auto& flat=a[3].elements;
      for(const auto& input:a[0].elements){
        const I symbol=value(input);if(symbol<1||symbol>symbols)return false;
        if(state<1||state>states)bad("unadmitted original regular state");
        const I index=plus(times(state-1,symbols),symbol-1);
        if(index<0||static_cast<std::uint64_t>(index)>=flat.size())bad("unadmitted original regular matrix shape");
        state=flat[static_cast<std::size_t>(index)].integer;if(!state)return false;
      }
      return member(state,a[5].set);
    }
    if(id=="gecode_circuit") {
      const I offset=a[0].integer;const auto& successors=a[1].elements;
      if(successors.empty())bad("unadmitted empty original circuit");
      // Read raw source terms in their original order. Range-check before
      // subtraction/indexing; never use the compiled CircuitData as evidence.
      std::vector<bool> visited(successors.size(),false);std::size_t node=0;
      for(std::size_t step=0;step<successors.size();++step) {
        if(visited[node])return false;
        visited[node]=true;const I next=value(successors[node]);
        if(next<offset)return false;
        const I index=plus(next,neg(offset));
        if(static_cast<std::uint64_t>(index)>=successors.size())return false;
        node=static_cast<std::size_t>(index);
      }
      return node==0;
    }
    if(id=="gecode_cumulatives"||id=="cumulatives") {
      // Fixed parameters are read again from their original source slots. The
      // compiler's embedded constants and generated global are not evidence.
      struct Event {I time,height;bool start;};
      const auto size=a[0].elements.size();const I bound=value(a[3]);
      if(size>std::numeric_limits<std::size_t>::max()/2)bad("original cumulative event count overflow");
      std::vector<Event> events;events.reserve(2*size);
      for(std::size_t i=0;i<size;++i){
        const I start=value(a[0].elements[i]),duration=value(a[1].elements[i]),height=value(a[2].elements[i]);
        if(!duration||!height)continue;
        events.push_back({start,height,true});events.push_back({plus(start,duration),height,false});
      }
      std::sort(events.begin(),events.end(),[](const Event& x,const Event& y){return x.time<y.time||(x.time==y.time&&x.start<y.start);});
      I used=0;
      for(const auto& event:events){
        // Ends precede starts at a shared time. Nonnegative demands make each
        // intermediate start sum no greater than that interval's final demand.
        if(event.start){if(event.height>bound-used)return false;used=plus(used,event.height);}
        else used=plus(used,neg(event.height));
      }
      return true;
    }
    if(id=="all_different_int") {
      std::set<I> assigned;for(const auto& v:a[0].elements)if(!assigned.insert(value(v)).second)return false;return true;
    }
    if(id=="array_int_element"||id=="array_var_int_element") {
      const I index=value(a[0]);if(index<1||static_cast<std::uint64_t>(index)>a[1].elements.size())return false;
      return value(a[1].elements[static_cast<std::size_t>(index-1)])==value(a[2]);
    }
    if(id=="int_in")return member(value(a[0]),a[1].set);
    if(id=="int_eq"||id=="bool_eq")return value(a[0])==value(a[1]);
    if(id=="int_le"||id=="bool_le")return value(a[0])<=value(a[1]);
    if(id=="int_lt")return value(a[0])<value(a[1]);
    if(id=="int_ge")return value(a[0])>=value(a[1]);
    if(id=="int_gt")return value(a[0])>value(a[1]);
    if(id=="int_plus")return plus(value(a[0]),value(a[1]))==value(a[2]);
    if(id=="int_minus")return plus(value(a[0]),neg(value(a[1])))==value(a[2]);
    if(id=="bool2int")return value(a[0])==value(a[1]);
    if(id=="bool_not")return value(a[0])!=value(a[1]);
    if(id=="int_lin_eq"||id=="int_lin_le"||id=="bool_lin_eq"||id=="bool_lin_le") {
      I sum=0;for(std::size_t i=0;i<a[0].elements.size();++i)sum=plus(sum,times(a[0].elements[i].integer,value(a[1].elements[i])));
      return id.find("_eq")!=std::string::npos?sum==value(a[2]):sum<=value(a[2]);
    }
    if(id=="bool_and"||id=="bool_or") {
      const bool left=value(a[0])!=0,right=value(a[1])!=0;
      return value(a[2])==static_cast<I>(id=="bool_and"?(left&&right):(left||right));
    }
    if(id=="array_bool_and"||id=="array_bool_or") {
      const bool conjunction=id=="array_bool_and";bool actual=conjunction;
      for(const auto& x:a[0].elements)actual=conjunction?(actual&&value(x)!=0):(actual||value(x)!=0);
      return value(a[1])==static_cast<I>(actual);
    }
    if(id=="bool_clause") {
      for(const auto& x:a[0].elements)if(value(x)==1)return true;
      for(const auto& x:a[1].elements)if(value(x)==0)return true;
      return false;
    }
    bad("unadmitted predicate reached original-source evaluator");
  }
  void declarations(const F::Records& source) const {
    for(const auto& v:source.raw_variables) {
      const auto current=values.at(key(v.reference));
      if(v.reference.type==F::Type::Boolean&&(current<0||current>1))bad("original Boolean domain violation");
      if(v.alias){if(current!=values.at(key(v.target)))bad("original declaration alias violation");continue;}
      if(v.domain.present&&!member(current,v.domain.integers))bad("original declared domain violation");
      if(v.assigned&&current!=value(v.value))bad("original fixed value violation");
    }
    for(const auto* input:{&source.raw_domains,&source.raw_constraints})for(const auto& c:*input)
      if(!relation(c))bad("original FlatZinc predicate is false: "+c.id);
  }
};
}
} // Detail

CompiledFlatZinc::CompiledFlatZinc(std::shared_ptr<const Detail::FlatZincArtifact> data):data_(std::move(data)) {}
const ModelSnapshot& CompiledFlatZinc::model() const {if(!data_)throw ModelError("moved-from compiled FlatZinc artifact");return data_->model;}
const F::Records& CompiledFlatZinc::source() const {if(!data_)throw ModelError("moved-from compiled FlatZinc artifact");return data_->source;}
const std::vector<FlatZincVariableMapping>& CompiledFlatZinc::variables() const {if(!data_)throw ModelError("moved-from compiled FlatZinc artifact");return data_->mapping;}
FlatZincCompileResult compile_flatzinc(const F::Records& source,const FlatZincCompileOptions& options) {
  FlatZincCompileResult result;
  try {
    if(std::isnan(options.time_limit_seconds)||options.time_limit_seconds<0)throw ModelError("invalid FlatZinc compile time limit");
    Detail::FlatZincCompiler compiler(source,options);
    try {result.compiled=compiler.run();result.status=FlatZincCompileStatus::Complete;result.message="complete bounded integer/Boolean FlatZinc source compilation";}
    catch(...){result.work=compiler.meter.work;throw;}
    result.work=compiler.meter.work;
  }catch(const Detail::Failure& error){result.compiled.reset();result.status=error.status;result.message=error.message;result.location=error.location;}
  catch(const std::bad_alloc&){result.compiled.reset();result.status=FlatZincCompileStatus::ResourceLimit;result.message="FlatZinc compiler allocation failed";}
  catch(const ModelError& e){result.compiled.reset();result.status=FlatZincCompileStatus::InvalidInput;result.message=e.what();}
  catch(const std::exception& e){result.compiled.reset();result.status=FlatZincCompileStatus::Error;result.message=e.what();}
  return result;
}
FlatZincValidation validate_flatzinc(const CompiledFlatZinc& compiled,const SolveResult& result,double tolerance) {
  FlatZincValidation check;
  try {
    if(!std::isfinite(tolerance)||tolerance<0||tolerance>=0.5)throw ModelError("invalid original-source rounding tolerance");
    const auto& model=compiled.model();
    if(result.model_id!=model.model_id||result.revision!=model.revision)Detail::bad("foreign or stale FlatZinc result");
    if(!result.has_solution()||result.values.size()!=model.variables.size()||result.active_variables.size()!=model.variables.size())Detail::bad("missing full FlatZinc witness");
    std::vector<double> rounded=result.values;
    for(std::size_t i=0;i<model.variables.size();++i) {
      if(result.active_variables[i]!=model.variables[i].active)Detail::bad("FlatZinc witness active mask mismatch");
      if(!model.variables[i].active)continue;
      const double x=rounded[i],integer=std::round(x);
      if(!std::isfinite(x)||integer < -static_cast<double>(Detail::exact_limit)||integer>static_cast<double>(Detail::exact_limit)||std::abs(integer-x)>tolerance)Detail::bad("FlatZinc witness is not a representable integer");
      rounded[i]=integer;
    }
    if(!validate(model,rounded,0,0).valid)Detail::bad("rounded witness violates the complete compiled model");
    Detail::SourceEvaluator evaluator;check.source_values.reserve(compiled.variables().size());
    for(const auto& map:compiled.variables()) {
      if(map.variable.model_id!=model.model_id||map.variable.id>=rounded.size())Detail::bad("invalid compiled source mapping");
      auto value=static_cast<Detail::I>(rounded[map.variable.id]);evaluator.values.emplace(Detail::key(map.source),value);check.source_values.push_back(value);
    }
    evaluator.declarations(compiled.source());
    const auto objective=compiled.source().solve.method==F::Method::Satisfy?Detail::I(0):evaluator.value(compiled.source().solve.objective);
    const auto expected=Detail::number(objective);
    if(!result.objective||std::abs(*result.objective-expected)>tolerance)Detail::bad("solver objective disagrees with the original FlatZinc objective");
    check.original_objective=expected;check.valid=true;check.message="all original domains and predicates hold on the rounded source assignment";
  }catch(const Detail::Failure& e){check.message=e.message;}
  catch(const std::exception& e){check.message=e.what();}
  if(!check.valid){check.original_objective.reset();check.source_values.clear();}
  return check;
}
std::string format_flatzinc_solution(const CompiledFlatZinc& compiled,const SolveResult& result,double tolerance) {
  const auto checked=validate_flatzinc(compiled,result,tolerance);
  if(!checked.valid)throw ModelError("cannot format an invalid FlatZinc solution: "+checked.message);
  Detail::SourceEvaluator evaluator;
  for(std::size_t i=0;i<compiled.variables().size();++i)evaluator.values.emplace(Detail::key(compiled.variables()[i].source),checked.source_values[i]);
  const auto text=[&](const F::Value& value){
    const auto x=evaluator.value(value);const bool boolean=value.kind==F::ValueKind::Boolean||(value.kind==F::ValueKind::Reference&&value.reference.type==F::Type::Boolean);
    return boolean?std::string(x?"true":"false"):std::to_string(x);
  };
  std::ostringstream stream;
  for(const auto& out:compiled.source().output) {
    stream<<out.name<<" = ";
    if(const auto* ann=Detail::output_array(compiled.source(),out.name)) {
      const auto& dims=ann->elements[0].elements;stream<<"array"<<dims.size()<<"d(";
      for(const auto& d:dims){auto range=Detail::interval(d.set);stream<<range.first<<".."<<range.second<<", ";}
      stream<<'[';const auto& values=Detail::output_values(out.expression);
      for(std::size_t i=0;i<values.size();++i){if(i)stream<<", ";stream<<text(values[i]);}
      stream<<"])";
    }else stream<<text(out.expression);
    stream<<";\n";
  }
  stream<<"----------\n";return stream.str();
}
}}
