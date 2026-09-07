/* Independent source-language truth tables; no native parser/backend required. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <gecode/optimize/flatzinc.hpp>
#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>
#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <limits>
#include <set>

namespace O=Gecode::Optimize;
namespace F=Gecode::FlatZinc::Capture;
using I=std::int64_t;
using Point=std::vector<I>;
static std::size_t configurations=0,assignments=0;
F::Value integer(I x){F::Value v;v.integer=x;return v;}
F::Value boolean(bool x){F::Value v;v.kind=F::ValueKind::Boolean;v.boolean=x;return v;}
F::Value ref(std::size_t index,F::Type type=F::Type::Integer){F::Value v;v.kind=F::ValueKind::Reference;v.reference={type,index};return v;}
F::Value array(std::vector<F::Value> x){F::Value v;v.kind=F::ValueKind::Array;v.elements=std::move(x);return v;}
F::Value domain(I lo,I hi){F::Value v;v.kind=F::ValueKind::Set;v.set.interval=true;v.set.lower=lo;v.set.upper=hi;return v;}
F::Value members(std::vector<I> values){auto v=domain(0,0);v.set.interval=false;v.set.values=std::move(values);return v;}
F::Value atom(const std::string& x){F::Value v;v.kind=F::ValueKind::Atom;v.text=x;return v;}
F::Value call(const std::string& x,std::vector<F::Value> args){auto v=atom(x);v.kind=F::ValueKind::Call;v.elements=std::move(args);return v;}
F::Variable variable(std::size_t index,I lo,I hi,F::Type type=F::Type::Integer){F::Variable v;v.reference={type,index};v.name="v"+std::to_string(index);v.domain.present=true;v.domain.integers=domain(lo,hi).set;return v;}
F::Constraint row(const std::string& name,std::vector<F::Value> args){F::Constraint c;c.id=name;c.arguments=std::move(args);c.location={"fixture.fzn",3,1};return c;}
O::CompiledFlatZinc compiled(const F::Records& source){auto result=O::compile_flatzinc(source);if(!result.compiled)std::cerr<<"compile failed: "<<result.message<<'\n';assert(result.status==O::FlatZincCompileStatus::Complete&&result.compiled);++configurations;return *result.compiled;}
std::optional<std::vector<double>> mapped(const O::CompiledFlatZinc& c,const Point& point){
  std::vector<double> values(c.model().variables.size());std::vector<bool> assigned(values.size());assert(point.size()==c.variables().size());
  for(std::size_t i=0;i<point.size();++i){auto slot=c.variables()[i].variable.id;if(assigned[slot]&&values[slot]!=point[i])return {};assigned[slot]=true;values[slot]=point[i];}
  // Private indicator gates and fixed global literals are existential witnesses,
  // not source variables and not implicitly zero. Enumerate every helper slot.
  const auto complete=[&](const auto& self,std::size_t slot)->bool {
    if(slot==values.size())return O::validate(c.model(),values,0,0).valid;
    if(assigned[slot])return self(self,slot+1);
    const auto& v=c.model().variables[slot];assert(v.active&&v.upper-v.lower<=2);
    for(I x=static_cast<I>(v.lower);x<=static_cast<I>(v.upper);++x){values[slot]=static_cast<double>(x);if(self(self,slot+1))return true;}
    return false;
  };
  if(complete(complete,0))return values;return {};
}
O::SolveResult witness(const O::CompiledFlatZinc& c,const Point& point,double objective=0){
  O::SolveResult result;result.model_id=c.model().model_id;result.revision=c.model().revision;
  result.values=*mapped(c,point);result.active_variables.assign(result.values.size(),true);result.objective=objective;
  result.solution_validated=true;result.termination=O::Termination::Optimal;result.guarantee=O::Guarantee::Exact;return result;
}
void oracle(const F::Records& source,const std::vector<std::pair<I,I>>& ranges,const std::function<bool(const Point&)>& predicate){
  const auto c=compiled(source);Point point(ranges.size());
  const auto visit=[&](const auto& self,std::size_t j)->void{
    if(j<point.size()){for(I x=ranges[j].first;x<=ranges[j].second;++x){point[j]=x;self(self,j+1);}return;}
    ++assignments;const auto values=mapped(c,point);const bool actual=values&&O::validate(c.model(),*values,0,0).valid;const bool expected=predicate(point);
    if(actual!=expected)std::cerr<<"oracle mismatch on "<<(source.raw_constraints.empty()?"domains":source.raw_constraints[0].id)<<'\n';assert(actual==expected);
    if(actual){auto result=witness(c,point);assert(O::validate_flatzinc(c,result,0).valid);}
  };visit(visit,0);
}
void rejected(const F::Records& source,O::FlatZincCompileStatus status=O::FlatZincCompileStatus::Unsupported){auto r=O::compile_flatzinc(source);if(r.status!=status)std::cerr<<"unexpected rejection status: "<<r.message<<" expected "<<static_cast<int>(status)<<" actual "<<static_cast<int>(r.status)<<"\n";assert(r.status==status&&!r.compiled);++configurations;}
void comparisons(){
  for(const auto& id:{"int_eq","int_le","int_lt","int_ge","int_gt"})for(bool left_literal:{false,true})for(bool right_literal:{false,true}){
    F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2)};
    r.raw_constraints={row(id,{left_literal?integer(-1):ref(0),right_literal?integer(1):ref(1)})};
    oracle(r,{{-2,2},{-2,2}},[&](const Point& p){auto a=left_literal?-1:p[0],b=right_literal?1:p[1];std::string s=id;return s=="int_eq"?a==b:s=="int_le"?a<=b:s=="int_lt"?a<b:s=="int_ge"?a>=b:a>b;});
  }
  for(const auto& id:{"int_plus","int_minus"}){F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2),variable(2,-2,2)};
    r.raw_constraints={row(id,{ref(0),ref(1),ref(2)})};oracle(r,{{-2,2},{-2,2},{-2,2}},[&](const Point& p){return (std::string(id)=="int_plus"?p[0]+p[1]:p[0]-p[1])==p[2];});}
  for(int a=-3;a<=3;++a)for(int b=-3;b<=3;++b)for(bool equality:{false,true}){
    F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2)};
    r.raw_constraints={row(equality?"int_lin_eq":"int_lin_le",{array({integer(a),integer(b),integer(-1)}),array({ref(0),ref(1),ref(0)}),integer(1)})};
    oracle(r,{{-2,2},{-2,2}},[&](const Point& p){auto sum=(a-1)*p[0]+b*p[1];return equality?sum==1:sum<=1;});
  }
}
void booleans(){
  const auto B=F::Type::Boolean;
  for(const auto& id:{"bool_eq","bool_le","bool_not","bool_and","bool_or"}){
    F::Records r;r.raw_variables={variable(0,0,1,B),variable(1,0,1,B),variable(2,0,1,B)};
    std::vector<F::Value> args{ref(0,B),ref(1,B)};if(std::string(id)=="bool_and"||std::string(id)=="bool_or")args.push_back(ref(2,B));
    r.raw_constraints={row(id,args)};oracle(r,{{0,1},{0,1},{0,1}},[&](const Point& p){std::string s=id;return s=="bool_eq"?p[0]==p[1]:s=="bool_le"?p[0]<=p[1]:s=="bool_not"?p[0]!=p[1]:s=="bool_and"?((p[0]&&p[1])==p[2]):((p[0]||p[1])==p[2]);});
  }
  for(bool conjunction:{false,true})for(int size=0;size<=4;++size)for(bool literal_result:{false,true}){
    F::Records r;r.raw_variables={variable(0,0,1,B),variable(1,0,1,B)};std::vector<F::Value> args;
    for(int i=0;i<size;++i)args.push_back(i==2?boolean(false):ref(0,B));
    r.raw_constraints={row(conjunction?"array_bool_and":"array_bool_or",{array(args),literal_result?boolean(true):ref(1,B)})};
    oracle(r,{{0,1},{0,1}},[&](const Point& p){bool value=conjunction;for(int i=0;i<size;++i){bool x=i==2?false:p[0];value=conjunction?(value&&x):(value||x);}return value==(literal_result?1:p[1]);});
  }
  for(int n=0;n<4;++n)for(int k=0;k<4;++k){F::Records r;r.raw_variables={variable(0,0,1,B),variable(1,0,1,B)};
    std::vector<F::Value> a(n,ref(0,B)),b(k,ref(1,B));r.raw_constraints={row("bool_clause",{array(a),array(b)})};oracle(r,{{0,1},{0,1}},[&](const Point& p){return(n&&p[0])||(k&&!p[1]);});}
  F::Records channel;channel.raw_variables={variable(0,0,1,B),variable(0,-1,2)};channel.raw_constraints={row("bool2int",{ref(0,B),ref(0)})};
  oracle(channel,{{0,1},{-1,2}},[](const Point& p){return p[0]==p[1];});
  channel.raw_constraints={row("bool_lin_eq",{array({integer(2),integer(-1)}),array({ref(0,B),boolean(true)}),ref(0)})};
  oracle(channel,{{0,1},{-1,2}},[](const Point& p){return 2*p[0]-1==p[1];});
}
void aliases_domains(){
  // Domain-based binary recognition must not change source typing, aliases,
  // singleton restrictions, or acceptance of assignments outside the domain.
  F::Records binary;binary.raw_variables={variable(0,0,1),variable(1,-2,3),variable(2,1,1),variable(3,0,2)};
  binary.raw_variables[1].alias=true;binary.raw_variables[1].target={F::Type::Integer,0};
  binary.output={{"integer_decision",ref(0)}};
  const auto classified=compiled(binary);
  assert(classified.model().variables[0].type==O::VariableType::Binary);
  assert(classified.variables()[0].variable==classified.variables()[1].variable);
  assert(classified.model().variables[1].type==O::VariableType::Binary);
  assert(classified.model().variables[2].type==O::VariableType::Integer);
  assert(O::format_flatzinc_solution(classified,witness(classified,{1,1,1,2}))=="integer_decision = 1;\n----------\n");
  oracle(binary,{{-1,2},{-1,2},{0,2},{-1,3}},[](const Point& p){
    return p[0]>=0&&p[0]<=1&&p[0]==p[1]&&p[2]==1&&p[3]>=0&&p[3]<=2;
  });
  F::Records r;r.raw_variables={variable(0,-2,3),variable(1,-9,9),variable(2,-9,9)};
  r.raw_variables[1].alias=true;r.raw_variables[1].target={F::Type::Integer,0};r.raw_variables[2].alias=true;r.raw_variables[2].target={F::Type::Integer,1};
  r.raw_domains={row("int_in",{ref(1),domain(0,2)})};
  // Normalized audit data must never replace a raw relation.
  r.constraints={row("unsupported_audit_only",{})};
  r.raw_constraints={row("int_le",{ref(2),integer(1)})};
  oracle(r,{{-2,3},{-2,3},{-2,3}},[](const Point& p){return p[0]==p[1]&&p[1]==p[2]&&p[0]>=0&&p[0]<=1;});
  r.raw_domains.push_back(row("int_in",{ref(2),domain(3,4)}));oracle(r,{{-2,3},{-2,3},{-2,3}},[](const Point&){return false;});
  F::Records empty;empty.raw_variables={variable(0,1,0)};oracle(empty,{{-1,1}},[](const Point&){return false;});
  empty.raw_variables[0].domain.integers=domain(0,1).set;empty.raw_variables[0].assigned=true;empty.raw_variables[0].value=integer(1);
  oracle(empty,{{-1,2}},[](const Point& p){return p[0]==1;});
  empty.raw_variables[0].assigned=false;empty.raw_variables[0].domain.present=false;empty.raw_domains={row("int_in",{ref(0),domain(-1,1)})};
  oracle(empty,{{-2,2}},[](const Point& p){return p[0]>=-1&&p[0]<=1;});
  auto set=domain(0,0);set.set.interval=false;set.set.values={2,0,1,1};empty.raw_domains={row("int_in",{ref(0),set})};
  oracle(empty,{{-1,3}},[](const Point& p){return p[0]>=0&&p[0]<=2;});
  set.set.values={0,2};empty.raw_domains={row("int_in",{ref(0),set})};oracle(empty,{{-1,3}},[](const Point& p){return p[0]==0||p[0]==2;});
}
void failures(){
  F::Records r;r.raw_variables={variable(0,0,2)};
  for(const auto& id:{"unknown_plugin","int_eq_reif","int_lin_eq_reif","int_times","regular","gecode_int_le","array_bool_element"}){r.raw_constraints={row(id,{})};rejected(r);}
  r.raw_constraints={row("cumulatives",{})};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_constraints={row("int_eq",{})};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_constraints={row("int_eq",{ref(3),integer(0)})};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_constraints={row("int_eq",{boolean(false),integer(0)})};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_constraints.clear();r.raw_variables[0].alias=true;r.raw_variables[0].target={F::Type::Integer,0};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_variables={variable(0,0,1),variable(0,0,1)};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_variables={variable(0,0,1,F::Type::Float)};rejected(r);
  r.raw_variables={variable(0,0,1)};r.solve.annotations={call("int_search",{})};rejected(r);
  r.solve.annotations.clear();r.raw_constraints={row("int_le",{ref(0),integer(0)})};r.raw_constraints[0].annotations={atom("unknown_annotation")};rejected(r);
  for(const auto& name:{"ctx_pos","ctx_neg","ctx_mix"}) {
    r.raw_constraints[0].annotations={atom(name)};
    oracle(r,{{0,1}},[](const Point& p){return p[0]<=0;});
    for(const auto& args:{std::vector<F::Value>{},std::vector<F::Value>{integer(0)}}) {
      r.raw_constraints[0].annotations={call(name,args)};rejected(r,O::FlatZincCompileStatus::InvalidInput);
    }
    auto malformed=atom(name);malformed.elements={integer(0)};
    r.raw_constraints[0].annotations={malformed};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  }
  r.raw_constraints[0].annotations={atom("bounds")};assert(O::compile_flatzinc(r).compiled);
  r.raw_constraints[0].annotations={call("defines_var",{ref(99)})};rejected(r,O::FlatZincCompileStatus::InvalidInput);
  r.raw_constraints[0].annotations={atom("bounds")};
  r.declaration_annotations={{"missing",{atom("output_var")},{}}};rejected(r,O::FlatZincCompileStatus::InvalidInput);r.declaration_annotations.clear();
  for(int mode=0;mode<5;++mode){O::FlatZincCompileOptions opts;
    if(mode==0)opts.max_work=0;if(mode==1)opts.max_variables=0;if(mode==2)opts.max_constraints=0;if(mode==3)opts.max_nonzeros=0;if(mode==4)opts.max_value_depth=0;
    if(mode==4)r.raw_constraints[0].arguments[0]=array({ref(0)});
    auto result=O::compile_flatzinc(r,opts);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);
  }
  r.raw_constraints.clear();O::FlatZincCompileOptions opts;opts.time_limit_seconds=0;assert(O::compile_flatzinc(r,opts).status==O::FlatZincCompileStatus::TimeLimit);
  opts.time_limit_seconds=10;opts.cancellation=std::make_shared<O::CancellationToken>();opts.cancellation->cancel();assert(O::compile_flatzinc(r,opts).status==O::FlatZincCompileStatus::Cancelled);
}
void history_output(){
  F::Records r;r.raw_variables={variable(0,-2,3),variable(0,0,1,F::Type::Boolean)};
  r.output={{"x",ref(0)},{"b",ref(0,F::Type::Boolean)},{"a",array({ref(0),ref(0),integer(7),integer(-3)})}};
  r.declaration_annotations={{"a",{call("output_array",{array({domain(-1,0),domain(2,3)})})},{}}};
  for(auto method:{F::Method::Minimize,F::Method::Maximize})for(bool constant:{false,true}) {
    r.solve.method=method;r.solve.has_objective=true;r.solve.objective=constant?integer(7):ref(0);
    auto c=compiled(r);auto result=witness(c,{2,1},constant?7:2);auto checked=O::validate_flatzinc(c,result);assert(checked.valid&&checked.original_objective==(constant?7:2));
    assert(O::format_flatzinc_solution(c,result)=="x = 2;\nb = true;\na = array2d(-1..0, 2..3, [2, 2, 7, -3]);\n----------\n");
    result.revision++;assert(!O::validate_flatzinc(c,result).valid);result.revision--;
    result.objective=99;assert(!O::validate_flatzinc(c,result).valid);result.objective=constant?7:2;
    result.values[0]=2.0000001;assert(O::validate_flatzinc(c,result).valid);result.values[0]=2.1;assert(!O::validate_flatzinc(c,result).valid);
    result.values[0]=2;result.active_variables[0]=false;assert(!O::validate_flatzinc(c,result).valid);
    bool threw=false;try{O::format_flatzinc_solution(c,result);}catch(const O::ModelError&){threw=true;}assert(threw);
  }
  auto c=compiled(r);r.raw_variables.clear();r.output.clear();assert(c.source().raw_variables.size()==2&&c.source().output.size()==3);
  F::Records empty;auto e=compiled(empty);assert(O::format_flatzinc_solution(e,witness(e,{}))=="----------\n");
}
void actual_backends(){
  F::Records r;r.raw_variables={variable(0,-2,4),variable(1,-2,4)};r.raw_constraints={row("int_lin_le",{array({integer(-2),integer(-1)}),array({ref(0),ref(1)}),integer(-5)})};
  r.solve.method=F::Method::Minimize;r.solve.has_objective=true;r.solve.objective=ref(0);auto c=compiled(r);
  for(auto backend:{O::Backend::Highs,O::Backend::Native})if(O::capabilities(backend).available){O::SolveOptions opts;opts.backend=backend;opts.relative_gap=0;opts.absolute_gap=0;opts.guarantee=backend==O::Backend::Native?O::Guarantee::Exact:O::Guarantee::Numerical;auto solved=O::solve(c.model(),opts);assert(solved.termination==O::Termination::Optimal&&solved.objective==1&&O::validate_flatzinc(c,solved).valid);}
}
void guarded(){
  const auto B=F::Type::Boolean;
  for(const auto& id:{"int_le_reif","int_le_imp","int_eq_imp"})for(int guard=0;guard<4;++guard)for(int operands=0;operands<4;++operands){
    F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2),variable(0,0,1,B)};
    if(guard==3){r.raw_variables[2].assigned=true;r.raw_variables[2].value=boolean(false);}
    r.raw_constraints={row(id,{operands&1?integer(-1):ref(0),operands&2?integer(1):ref(1),guard==1?boolean(false):guard==2?boolean(true):ref(0,B)})};
    oracle(r,{{-2,2},{-2,2},{0,1}},[&](const Point& p){const auto a=operands&1?-1:p[0],b=operands&2?1:p[1];const bool g=guard==1?false:guard==2?true:p[2];
      const bool condition=std::string(id)=="int_eq_imp"?a==b:a<=b;
      return(guard!=3||p[2]==0)&&(std::string(id)=="int_le_reif"?condition==g:!g||condition);});
  }
  for(bool equivalence:{false,true})for(int a=-3;a<=3;++a)for(int b=-3;b<=3;++b)for(int guard=0;guard<3;++guard){
    F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2),variable(0,0,1,B)};
    r.raw_constraints={row(equivalence?"int_lin_le_reif":"int_lin_le_imp",{array({integer(a),integer(b),integer(-1)}),array({ref(0),ref(1),ref(0)}),integer(1),guard==1?boolean(false):guard==2?boolean(true):ref(0,B)})};
    oracle(r,{{-2,2},{-2,2},{0,1}},[&](const Point& p){const bool g=guard==1?false:guard==2?true:p[2],condition=(a-1)*p[0]+b*p[1]<=1;return equivalence?condition==g:!g||condition;});
  }
  for(I rhs=-1;rhs<=1;++rhs){F::Records r;r.raw_variables={variable(0,0,1,B)};r.raw_constraints={row("int_lin_le_reif",{array({}),array({}),integer(rhs),ref(0,B)})};oracle(r,{{0,1}},[&](const Point& p){return(0<=rhs)==bool(p[0]);});}
  F::Records literals;literals.raw_variables={variable(0,-2,2),variable(0,0,1,B)};
  literals.raw_constraints={row("int_lin_le_reif",{array({integer(-2),integer(3)}),array({ref(0),integer(-1)}),integer(-2),ref(0,B)})};
  oracle(literals,{{-2,2},{0,1}},[](const Point& p){return(-2*p[0]-3<=-2)==bool(p[1]);});
  O::FlatZincCompileOptions no_helpers;no_helpers.max_variables=2;
  assert(O::compile_flatzinc(literals,no_helpers).status==O::FlatZincCompileStatus::ResourceLimit);
  F::Records alias;alias.raw_variables={variable(0,-1,1),variable(1,-1,1),variable(0,0,1,B),variable(1,0,1,B)};
  alias.raw_variables[1].alias=true;alias.raw_variables[1].target={F::Type::Integer,0};alias.raw_variables[3].alias=true;alias.raw_variables[3].target={B,0};
  alias.raw_constraints={row("int_le_reif",{ref(0),ref(1),ref(1,B)})};
  oracle(alias,{{-1,1},{-1,1},{0,1},{0,1}},[](const Point& p){return p[0]==p[1]&&p[2]==1&&p[3]==1;});
  auto c=compiled(alias);auto proof=witness(c,{0,0,1,1});
  std::set<std::uint64_t> visible;for(const auto& m:c.variables())visible.insert(m.variable.id);
  for(std::size_t i=0;i<proof.values.size();++i)if(!visible.count(i)&&c.model().variables[i].lower!=c.model().variables[i].upper){auto wrong=proof;wrong.values[i]=1-wrong.values[i];assert(!O::validate_flatzinc(c,wrong,0).valid);}
  F::Records huge;huge.raw_variables={variable(0,0,1),variable(0,0,1,B)};
  huge.raw_constraints={row("int_lin_le_reif",{array({integer(1)}),array({ref(0)}),integer(9007199254740992LL),ref(0,B)})};rejected(huge);
  huge.raw_constraints[0].arguments[3]=integer(0);rejected(huge,O::FlatZincCompileStatus::InvalidInput);
}
void globals(){
  for(int variant=0;variant<5;++variant){F::Records r;r.raw_variables={variable(0,-1,2),variable(1,-1,2)};
    std::vector<F::Value> values=variant==0?std::vector<F::Value>{}:variant==1?std::vector<F::Value>{ref(0)}:variant==2?std::vector<F::Value>{ref(0),ref(0)}:variant==3?std::vector<F::Value>{ref(0),ref(1),integer(1)}:std::vector<F::Value>{integer(1),integer(1)};
    r.raw_constraints={row("all_different_int",{array(values)})};
    oracle(r,{{-1,2},{-1,2}},[&](const Point& p){return variant<2?true:variant==2||variant==4?false:p[0]!=p[1]&&p[0]!=1&&p[1]!=1;});
  }
  for(const auto& id:{"array_int_element","array_var_int_element"})for(int variant=0;variant<5;++variant){
    if(std::string(id)=="array_int_element"&&variant>=2)continue;
    F::Records r;r.raw_variables={variable(0,0,3),variable(1,-1,2),variable(2,-1,2),variable(3,-1,2)};
    const std::vector<F::Value> elements=variant==0?std::vector<F::Value>{}:variant==1?std::vector<F::Value>{integer(-1),integer(2)}:variant==2?std::vector<F::Value>{ref(2),ref(2)}:std::vector<F::Value>{ref(2),integer(1),ref(3)};
    r.raw_constraints={row(id,{variant==4?integer(2):ref(0),array(elements),variant==3?ref(0):ref(1)})};
    oracle(r,{{0,3},{-1,2},{-1,2},{-1,2}},[&](const Point& p){const I index=variant==4?2:p[0],result=variant==3?p[0]:p[1];
      if(index<1||static_cast<std::uint64_t>(index)>elements.size())return false;
      const I selected=variant==1?(index==1?-1:2):variant==2?p[2]:index==1?p[2]:index==2?1:p[3];return selected==result;});
  }
  F::Records aliases;aliases.raw_variables={variable(0,-1,2),variable(1,-1,2)};aliases.raw_variables[1].alias=true;aliases.raw_variables[1].target={F::Type::Integer,0};
  aliases.raw_constraints={row("all_different_int",{array({ref(0),ref(1)})})};oracle(aliases,{{-1,2},{-1,2}},[](const Point&){return false;});
  aliases.raw_constraints={row("array_int_element",{integer(1),array({ref(0)}),ref(1)})};rejected(aliases,O::FlatZincCompileStatus::InvalidInput);
  F::Records limits;limits.raw_variables={variable(0,0,2)};limits.raw_constraints={row("all_different_int",{array({ref(0),integer(1)})})};
  O::FlatZincCompileOptions cap;cap.max_variables=1;assert(O::compile_flatzinc(limits,cap).status==O::FlatZincCompileStatus::ResourceLimit);
  cap={};cap.max_nonzeros=1;assert(O::compile_flatzinc(limits,cap).status==O::FlatZincCompileStatus::ResourceLimit);
  auto c=compiled(limits);
  if(O::capabilities(O::Backend::Highs).available){O::SolveOptions opt;opt.backend=O::Backend::Highs;auto r=O::solve(c.model(),opt);assert(r.termination==O::Termination::Unsupported&&!r.has_solution());}
  if(O::capabilities(O::Backend::Native).available){O::SolveOptions opt;opt.backend=O::Backend::Native;opt.guarantee=O::Guarantee::Exact;auto r=O::solve(c.model(),opt);assert(r.termination==O::Termination::Optimal&&O::validate_flatzinc(c,r,0).valid);}
  F::Records combined;combined.raw_variables={variable(0,1,3),variable(1,1,3),variable(0,0,1,F::Type::Boolean)};
  combined.raw_constraints={row("all_different_int",{array({ref(0),ref(1)})}),row("array_var_int_element",{integer(1),array({ref(0),integer(3)}),ref(1)}),row("int_le_reif",{ref(0),ref(1),ref(0,F::Type::Boolean)})};
  oracle(combined,{{1,3},{1,3},{0,1}},[](const Point&){return false;});
  if(O::capabilities(O::Backend::Native).available){
    O::SolveOptions opt;opt.backend=O::Backend::Native;opt.guarantee=O::Guarantee::Exact;
    auto impossible=compiled(combined);assert(O::solve(impossible.model(),opt).termination==O::Termination::Infeasible);
    combined.raw_constraints[1]=row("array_var_int_element",{integer(2),array({ref(0),ref(1)}),ref(1)});
    auto possible=compiled(combined);auto r=O::solve(possible.model(),opt);
    assert(r.termination==O::Termination::Optimal&&O::validate_flatzinc(possible,r,0).valid);
  }
}

void holey_domains(){
  // Every pair of subsets of a signed five-point universe: intersection is
  // checked through bit membership, independently of normalization/lowering.
  for(unsigned first=0;first<32;++first)for(unsigned second=0;second<32;++second){
    F::Records r;r.raw_variables={variable(7,-2,2)};std::vector<I> a,b;
    for(int bit=4;bit>=0;--bit){if(first&(1u<<bit)){a.push_back(bit-2);a.push_back(bit-2);}if(second&(1u<<bit))b.push_back(bit-2);}
    r.raw_variables[0].domain.integers=members(a).set;
    r.raw_domains={row("int_in",{ref(7),members(b)})};
    oracle(r,{{-2,2}},[&](const Point& p){return bool(first&second&(1u<<unsigned(p[0]+2)));});
  }
  F::Records aliases;aliases.raw_variables={variable(2,-5,5),variable(9,-9,9),variable(15,-9,9)};
  aliases.raw_variables[1].alias=true;aliases.raw_variables[1].target={F::Type::Integer,2};
  aliases.raw_variables[2].alias=true;aliases.raw_variables[2].target={F::Type::Integer,9};
  aliases.raw_domains={row("int_in",{ref(15),members({3,-3,1,1})}),row("int_in",{ref(9),domain(-1,4)})};
  oracle(aliases,{{-3,3},{-3,3},{-3,3}},[](const Point& p){return p[0]==p[1]&&p[1]==p[2]&&(p[0]==1||p[0]==3);});
  aliases.raw_variables[0].assigned=true;aliases.raw_variables[0].value=integer(2);
  oracle(aliases,{{-1,3},{-1,3},{-1,3}},[](const Point&){return false;});
  for(bool constant:{false,true})for(bool allowed:{false,true}){
    F::Records r;r.raw_constraints={row("int_in",{constant?integer(2):boolean(true),members(allowed?std::vector<I>{2,1}:std::vector<I>{-2,0})})};
    oracle(r,{},[&](const Point&){return allowed;});
  }
  F::Records empty;empty.raw_variables={variable(0,0,1)};empty.raw_variables[0].domain.present=false;
  empty.raw_domains={row("int_in",{ref(0),members({})})};oracle(empty,{{-1,1}},[](const Point&){return false;});
  empty.raw_variables[0].domain.integers=members({0,2,3,4,6}).set;empty.raw_variables[0].domain.present=true;
  empty.raw_domains={row("int_in",{ref(0),domain(2,4)})};
  auto contiguous=compiled(empty);assert(contiguous.model().globals.empty());
  oracle(empty,{{0,6}},[](const Point& p){return p[0]>=2&&p[0]<=4;});
  empty.raw_domains={row("int_in",{ref(0),members({3})})};assert(compiled(empty).model().globals.empty());
  // Sparse gaps stay symbolic even at the limits of exact model representation.
  const I large=9007199254740992LL;
  F::Records sparse;sparse.raw_variables={variable(8,-large,large)};
  sparse.raw_variables[0].domain.integers=members({large,-large,large}).set;
  auto c=compiled(sparse);assert(c.model().variables.size()==1&&c.model().globals.size()==1);
  assert(c.model().variables[0].lower==-large&&c.model().variables[0].upper==large);
  assert(mapped(c,{-large})&&mapped(c,{large})&&!mapped(c,{0}));
  auto historical=witness(c,{large});assert(O::validate_flatzinc(c,historical,0).valid);
  sparse.raw_variables.clear();assert(O::validate_flatzinc(c,historical,0).valid);
  historical.values[0]=0;assert(!O::validate_flatzinc(c,historical,0).valid);
  historical.values[0]=large;historical.active_variables[0]=false;assert(!O::validate_flatzinc(c,historical,0).valid);
  empty.raw_domains={row("int_in",{ref(0),members({large+1})})};rejected(empty);
  // A contradictory earlier domain must not hide an inadmissible later value.
  empty.raw_variables[0].domain.integers=members({}).set;rejected(empty);
  F::Records boolean_domain;boolean_domain.raw_variables={variable(0,0,1,F::Type::Boolean)};
  boolean_domain.raw_domains={row("int_in",{ref(0,F::Type::Boolean),members({3,1,-1})})};
  oracle(boolean_domain,{{0,1}},[](const Point& p){return p[0]==1;});
}
void tables(){
  // All relations of arities one through three on {0,1}; order and duplicate
  // rows are deliberately perturbed. The oracle interprets a relation bitset.
  for(unsigned arity=1;arity<=3;++arity)for(unsigned relation=0;relation<(1u<<(1u<<arity));++relation){
    F::Records r;std::vector<F::Value> arguments,flat;
    for(unsigned j=0;j<arity;++j){r.raw_variables.push_back(variable(j,0,1));arguments.push_back(ref(j));}
    for(int tuple=int((1u<<arity)-1);tuple>=0;--tuple)if(relation&(1u<<unsigned(tuple)))
      for(unsigned repeat=0;repeat<1+unsigned(tuple==0);++repeat)for(unsigned j=0;j<arity;++j)flat.push_back(integer((unsigned(tuple)>>j)&1u));
    r.raw_constraints={row("gecode_table_int",{array(arguments),array(flat)})};
    oracle(r,std::vector<std::pair<I,I>>(arity,{0,1}),[&](const Point& p){unsigned index=0;for(unsigned j=0;j<arity;++j)index|=unsigned(p[j])<<j;return bool(relation&(1u<<index));});
  }
  F::Records repeated;repeated.raw_variables={variable(0,-1,2),variable(1,-1,2)};
  repeated.raw_variables[1].alias=true;repeated.raw_variables[1].target={F::Type::Integer,0};
  repeated.raw_constraints={row("gecode_table_int",{array({ref(0),ref(1),integer(-1)}),array({integer(0),integer(1),integer(-1),integer(2),integer(2),integer(-1)})})};
  oracle(repeated,{{-1,2},{-1,2}},[](const Point& p){return p[0]==2&&p[1]==2;});
  auto historical=compiled(repeated);auto proof=witness(historical,{2,2});
  assert(proof.values.size()==2&&historical.variables().size()==2);
  repeated.raw_constraints.clear();repeated.raw_variables.clear();assert(O::validate_flatzinc(historical,proof,0).valid);
  proof.values.back()=0;assert(!O::validate_flatzinc(historical,proof,0).valid);
  proof=witness(historical,{2,2});proof.values.push_back(0);proof.active_variables.push_back(true);assert(!O::validate_flatzinc(historical,proof,0).valid);
  for(bool nonempty:{false,true})for(bool allowed:{false,true}){
    F::Records fixed;fixed.raw_constraints={row("gecode_table_int",{array({integer(-2),integer(-2)}),array(nonempty?std::vector<F::Value>{integer(-2),integer(allowed?-2:1)}:std::vector<F::Value>{})})};
    oracle(fixed,{},[&](const Point&){return nonempty&&allowed;});
  }
  F::Records malformed;malformed.raw_variables={variable(0,0,1)};
  const auto reject=[&](std::vector<F::Value> args,O::FlatZincCompileStatus status=O::FlatZincCompileStatus::InvalidInput){malformed.raw_constraints={row("gecode_table_int",std::move(args))};rejected(malformed,status);};
  reject({});reject({array({ref(0)})});reject({ref(0),array({})});
  reject({array({ref(0),ref(0)}),array({integer(0)})});
  reject({array({ref(0)}),array({ref(0)})});reject({array({ref(0)}),array({boolean(true)})});
  reject({array({boolean(true)}),array({integer(1)})});reject({array({ref(9)}),array({integer(0)})});
  reject({array({}),array({})},O::FlatZincCompileStatus::Unsupported);reject({array({}),array({integer(0)})});
  reject({array({ref(0)}),array({integer(9007199254740993LL)})},O::FlatZincCompileStatus::Unsupported);
  for(const auto& id:{"table_int","fzn_table_int","gecode_gecode_table_int","gecode_table_bool"}){malformed.raw_constraints={row(id,{array({ref(0)}),array({integer(0)})})};rejected(malformed);}
}
void table_domain_limits(){
  F::Records r;r.raw_variables={variable(0,-2,2)};r.raw_variables[0].domain.integers=members({2,-2,0,2}).set;
  r.raw_constraints={row("gecode_table_int",{array({ref(0),integer(0)}),array({integer(-2),integer(0),integer(2),integer(0)})})};
  auto full=O::compile_flatzinc(r);assert(full.compiled&&full.work>0);
  for(const auto cap:{std::size_t(0),std::size_t(1),full.work/2,full.work-1}){
    O::FlatZincCompileOptions options;options.max_work=cap;auto result=O::compile_flatzinc(r,options);
    assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled&&result.work<=cap);
  }
  O::FlatZincCompileOptions options;options.max_work=full.work;assert(O::compile_flatzinc(r,options).compiled);
  for(int mode=0;mode<3;++mode){options={};if(mode==0)options.max_variables=1;if(mode==1)options.max_constraints=1;if(mode==2)options.max_nonzeros=9;
    auto result=O::compile_flatzinc(r,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);
  }
  // Unary domain: one operand plus three distinct values; binary table: two
  // operands plus four cells. Both footprint limits are reserved before posting.
  options={};options.max_variables=2;options.max_constraints=2;options.max_nonzeros=10;assert(O::compile_flatzinc(r,options).compiled);
  F::Records duplicates;duplicates.raw_variables={variable(0,0,1)};duplicates.raw_variables[0].domain.integers=members({0,0,0,0}).set;
  options={};options.max_nonzeros=3;auto result=O::compile_flatzinc(duplicates,options);
  assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);
  options={};options.time_limit_seconds=0;assert(O::compile_flatzinc(r,options).status==O::FlatZincCompileStatus::TimeLimit);
  options={};options.cancellation=std::make_shared<O::CancellationToken>();options.cancellation->cancel();assert(O::compile_flatzinc(r,options).status==O::FlatZincCompileStatus::Cancelled);
}
void table_domain_backends(){
  F::Records r;r.raw_variables={variable(0,-3,3),variable(1,-3,3),variable(0,0,1,F::Type::Boolean)};
  r.raw_variables[0].domain.integers=members({3,-3,1}).set;
  r.raw_constraints={row("gecode_table_int",{array({ref(0),ref(1)}),array({integer(-3),integer(2),integer(1),integer(-2),integer(3),integer(1)})}),
    row("all_different_int",{array({ref(0),ref(1)})}),
    row("array_var_int_element",{integer(1),array({ref(0),ref(1)}),ref(0)}),
    row("int_le_reif",{ref(0),ref(1),ref(0,F::Type::Boolean)})};
  oracle(r,{{-3,3},{-3,3},{0,1}},[](const Point& p){return ((p[0]==-3&&p[1]==2)||(p[0]==1&&p[1]==-2)||(p[0]==3&&p[1]==1))&&bool(p[2])==(p[0]<=p[1]);});
  for(auto method:{F::Method::Minimize,F::Method::Maximize}){
    r.solve.method=method;r.solve.has_objective=true;r.solve.objective=ref(0);auto c=compiled(r);
    O::SolveOptions options;options.backend=O::Backend::Highs;
    auto unsupported=O::solve(c.model(),options);assert(unsupported.termination==O::Termination::Unsupported&&!unsupported.has_solution());
    for(auto backend:{O::Backend::Native,O::Backend::Auto}){options.backend=backend;options.guarantee=O::Guarantee::Exact;
      auto solved=O::solve(c.model(),options);
      if(O::capabilities(O::Backend::Native).available)assert(solved.termination==O::Termination::Optimal&&solved.objective==(method==F::Method::Minimize?-3:3)&&O::validate_flatzinc(c,solved,0).valid);
      else assert(solved.termination==O::Termination::Unsupported&&!solved.has_solution());
    }
  }
  // Eliminating all holes restores the linear backend path without weakening
  // the original list: only the exact interval intersection remains feasible.
  F::Records contiguous;contiguous.raw_variables={variable(0,0,6)};
  contiguous.raw_variables[0].domain.integers=members({0,2,3,4,6}).set;
  contiguous.raw_domains={row("int_in",{ref(0),domain(2,4)})};
  contiguous.solve.method=F::Method::Minimize;contiguous.solve.has_objective=true;contiguous.solve.objective=ref(0);
  auto linear=compiled(contiguous);assert(linear.model().globals.empty());
  O::SolveOptions linear_options;linear_options.backend=O::Backend::Highs;
  auto linear_result=O::solve(linear.model(),linear_options);
  if(O::capabilities(O::Backend::Highs).available)assert(linear_result.termination==O::Termination::Optimal&&linear_result.objective==2&&O::validate_flatzinc(linear,linear_result,0).valid);
  else assert(linear_result.termination==O::Termination::Unsupported&&!linear_result.has_solution());
  F::Records large;large.raw_variables={variable(0,-9007199254740992LL,9007199254740992LL)};
  large.raw_variables[0].domain.integers=members({-9007199254740992LL,9007199254740992LL}).set;
  auto wide=compiled(large);O::SolveOptions native;native.backend=O::Backend::Native;native.guarantee=O::Guarantee::Exact;
  auto native_result=O::solve(wide.model(),native);assert(native_result.termination==O::Termination::Unsupported&&!native_result.has_solution());
  F::Records empty;empty.raw_variables={variable(0,0,1)};empty.raw_constraints={row("gecode_table_int",{array({ref(0)}),array({})})};
  auto c=compiled(empty);O::SolveOptions options;options.backend=O::Backend::Native;options.guarantee=O::Guarantee::Exact;
  const auto result=O::solve(c.model(),options);assert(result.termination==(O::capabilities(O::Backend::Native).available?O::Termination::Infeasible:O::Termination::Unsupported));
}

// Independent circuit oracle: generate Hamiltonian cycles from permutations of
// nodes 1..n-1, rather than reusing either checker's successor-walk algorithm.
std::set<Point> circuit_points(std::size_t size,I offset){
  assert(size>0);std::vector<std::size_t> order;
  for(std::size_t i=1;i<size;++i)order.push_back(i);
  std::set<Point> points;
  do{Point successor(size);std::size_t previous=0;
    for(auto next:order){successor[previous]=offset+static_cast<I>(next);previous=next;}
    successor[previous]=offset;points.insert(std::move(successor));
  }while(std::next_permutation(order.begin(),order.end()));
  return points;
}
void circuits(){
  for(I offset:{I(0),I(1),I(3)})for(std::size_t size=1;size<=5;++size){
    F::Records r;std::vector<F::Value> arguments;
    for(std::size_t i=0;i<size;++i){r.raw_variables.push_back(variable(i,offset-1,offset+static_cast<I>(size)));arguments.push_back(ref(i));}
    r.raw_constraints={row("gecode_circuit",{integer(offset),array(arguments)})};
    const auto allowed=circuit_points(size,offset);
    oracle(r,std::vector<std::pair<I,I>>(size,{offset-1,offset+static_cast<I>(size)}),[&](const Point& p){return allowed.count(p)!=0;});
    const auto artifact=compiled(r);const auto& data=std::get<O::CircuitData>(artifact.model().globals.at(0).payload);
    assert(data.index_base==offset&&data.successors.size()==size);
    for(std::size_t i=0;i<size;++i)assert(data.successors[i]==artifact.variables()[i].variable);
    // Explicit Native and Auto must retain the full circuit, including the
    // singleton case; a linear backend may not drop even a redundant global.
    for(auto backend:{O::Backend::Native,O::Backend::Auto,O::Backend::Highs}){
      O::SolveOptions options;options.backend=backend;options.guarantee=backend==O::Backend::Highs?O::Guarantee::Numerical:O::Guarantee::Exact;
      const auto result=O::solve(artifact.model(),options);
      if(backend!=O::Backend::Highs&&O::capabilities(O::Backend::Native).available){
        assert(result.termination==O::Termination::Optimal);const auto checked=O::validate_flatzinc(artifact,result,0);
        assert(checked.valid&&allowed.count(checked.source_values));
      }else assert(result.termination==O::Termination::Unsupported&&!result.has_solution());
    }
  }
  for(int variant=0;variant<4;++variant){
    F::Records r;r.raw_variables={variable(0,2,5),variable(1,2,5),variable(2,2,5)};
    if(variant==2){r.raw_variables[1].alias=true;r.raw_variables[1].target={F::Type::Integer,0};}
    std::vector<F::Value> arguments{ref(0),variant==1?ref(0):ref(1),variant==0?ref(2):integer(3)};
    r.raw_constraints={row("gecode_circuit",{integer(3),array(arguments)})};const auto allowed=circuit_points(3,3);
    oracle(r,{{2,5},{2,5},{2,5}},[&](const Point& p){
      return(variant!=2||p[0]==p[1])&&allowed.count({p[0],variant==1?p[0]:p[1],variant==0?p[2]:3});
    });
  }
  for(const auto& tuple:std::vector<Point>{{3},{2},{4,3},{4,5,3},{4,3,5},{3,4,5}}){
    F::Records r;std::vector<F::Value> args;for(auto value:tuple)args.push_back(integer(value));
    r.raw_constraints={row("gecode_circuit",{integer(3),array(args)})};const auto allowed=circuit_points(tuple.size(),3);
    oracle(r,{},[&](const Point&){return allowed.count(tuple)!=0;});
  }
  // Owning source, hidden constant slots and output positions remain intact.
  F::Records r;r.raw_variables={variable(0,3,5),variable(1,3,5)};
  r.raw_constraints={row("gecode_circuit",{integer(3),array({ref(0),ref(1),integer(3)})})};r.output={{"first",ref(0)}};
  auto saved=compiled(r);auto proof=witness(saved,{4,5});assert(proof.values.size()==3&&saved.variables().size()==2);
  r.raw_variables.clear();r.raw_constraints.clear();r.output.clear();
  assert(O::validate_flatzinc(saved,proof,0).valid&&O::format_flatzinc_solution(saved,proof,0)=="first = 4;\n----------\n");
  proof.values[0]=3;assert(!O::validate_flatzinc(saved,proof,0).valid);
  proof=witness(saved,{4,5});proof.values.back()=4;assert(!O::validate_flatzinc(saved,proof,0).valid);
}
void circuit_boundaries(){
  F::Records r;r.raw_variables={variable(0,0,1),variable(0,0,1,F::Type::Boolean)};
  const auto reject=[&](std::vector<F::Value> args,O::FlatZincCompileStatus status=O::FlatZincCompileStatus::InvalidInput){r.raw_constraints={row("gecode_circuit",std::move(args))};rejected(r,status);};
  reject({});reject({integer(0)});reject({integer(0),array({ref(0)}),integer(0)});
  reject({array({ref(0)}),integer(0)});reject({ref(0),array({ref(0)})});
  reject({boolean(false),array({ref(0)})});reject({integer(0),ref(0)});reject({integer(0),array({})});
  reject({integer(0),array({boolean(true)})});reject({integer(0),array({ref(0,F::Type::Boolean)})});
  reject({integer(0),array({ref(7)})});reject({integer(0),array({array({integer(0)})})});
  reject({integer(-1),array({integer(-1)})},O::FlatZincCompileStatus::Unsupported);
  constexpr I large=9007199254740992LL;
  reject({integer(large+1),array({integer(0)})},O::FlatZincCompileStatus::Unsupported);
  reject({integer(large),array({integer(0),integer(0)})},O::FlatZincCompileStatus::Unsupported);
  reject({integer(0),array({integer(large+1)})},O::FlatZincCompileStatus::Unsupported);
  for(const auto& id:{"circuit","fzn_circuit","fzn_gecode_circuit","gecode_gecode_circuit","gecode_subcircuit","gecode_circuit_cost","gecode_circuit_reif"}){
    r.raw_constraints={row(id,{integer(0),array({ref(0)})})};rejected(r);
  }
  // Exact frontend range and native implementation range are separate gates.
  for(const auto& tuple:std::vector<std::pair<I,Point>>{{large,{large}},{large-1,{large,large-1}}}){
    F::Records wide;std::vector<F::Value> args;for(auto v:tuple.second)args.push_back(integer(v));
    wide.raw_constraints={row("gecode_circuit",{integer(tuple.first),array(args)})};
    oracle(wide,{},[](const Point&){return true;});auto c=compiled(wide);
    O::SolveOptions options;options.backend=O::Backend::Native;options.guarantee=O::Guarantee::Exact;
    assert(O::solve(c.model(),options).termination==O::Termination::Unsupported);
  }
  F::Records bounded;bounded.raw_variables={variable(0,0,1)};
  bounded.raw_constraints={row("gecode_circuit",{integer(0),array({ref(0),integer(0)})})};
  const auto full=O::compile_flatzinc(bounded);assert(full.compiled&&full.work>0);
  for(std::size_t cap:{std::size_t(0),full.work/2,full.work-1}){
    O::FlatZincCompileOptions options;options.max_work=cap;auto result=O::compile_flatzinc(bounded,options);
    assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled&&result.work<=cap);
  }
  O::FlatZincCompileOptions options;options.max_work=full.work;assert(O::compile_flatzinc(bounded,options).compiled);
  for(int mode=0;mode<3;++mode){options={};if(mode==0)options.max_variables=1;if(mode==1)options.max_constraints=0;if(mode==2)options.max_nonzeros=1;
    auto result=O::compile_flatzinc(bounded,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);
  }
  options={};options.max_variables=2;options.max_constraints=1;options.max_nonzeros=2;assert(O::compile_flatzinc(bounded,options).compiled);
  options={};options.time_limit_seconds=0;auto stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::TimeLimit&&!stopped.compiled);
  options={};options.cancellation=std::make_shared<O::CancellationToken>();options.cancellation->cancel();stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::Cancelled&&!stopped.compiled);
  // An earlier contradiction does not excuse an unsupported later signature.
  bounded.raw_variables[0].domain.integers=domain(1,0).set;bounded.raw_constraints[0].arguments[0]=integer(-1);rejected(bounded);
}
void circuit_combined(){
  F::Records r;r.raw_variables={variable(0,3,5),variable(1,3,5),variable(2,3,5),variable(0,0,1,F::Type::Boolean),variable(9,-4,-2)};
  r.raw_variables[1].domain.integers=members({5,3,5}).set;
  r.raw_constraints={row("gecode_circuit",{integer(3),array({ref(0),ref(1),ref(2)})}),
    row("gecode_table_int",{array({ref(0),ref(1),ref(2)}),array({integer(4),integer(5),integer(3),integer(5),integer(3),integer(4)})}),
    row("all_different_int",{array({ref(0),ref(1),ref(2)})}),
    row("array_var_int_element",{integer(1),array({ref(0),ref(1)}),ref(0)}),
    row("int_le_reif",{ref(0),ref(1),ref(0,F::Type::Boolean)}),
    row("int_plus",{ref(9),integer(7),ref(0)})};
  const auto allowed=circuit_points(3,3);
  oracle(r,{{3,5},{3,5},{3,5},{0,1},{-4,-2}},[&](const Point& p){return allowed.count({p[0],p[1],p[2]})&&bool(p[3])==(p[0]<=p[1])&&p[4]+7==p[0];});
  for(auto method:{F::Method::Minimize,F::Method::Maximize}){
    r.solve.method=method;r.solve.has_objective=true;r.solve.objective=ref(9);auto c=compiled(r);
    for(auto backend:{O::Backend::Native,O::Backend::Auto,O::Backend::Highs}){
      O::SolveOptions options;options.backend=backend;options.guarantee=backend==O::Backend::Highs?O::Guarantee::Numerical:O::Guarantee::Exact;
      auto result=O::solve(c.model(),options);
      if(backend!=O::Backend::Highs&&O::capabilities(O::Backend::Native).available)
        assert(result.termination==O::Termination::Optimal&&result.objective==(method==F::Method::Minimize?-3:-2)&&O::validate_flatzinc(c,result,0).valid);
      else assert(result.termination==O::Termination::Unsupported&&!result.has_solution());
    }
  }
}

// The tiny cumulative oracle samples every integer time in a bounded horizon.
// Integer endpoints make this exhaustive for half-open continuous-time usage,
// independently of both production event-sweep implementations.
bool cumulative_point(const Point& starts,const Point& durations,const Point& heights,I bound){
  assert(starts.size()==durations.size()&&starts.size()==heights.size()&&bound>=0);
  I first=0,last=0;
  for(std::size_t i=0;i<starts.size();++i){assert(durations[i]>=0&&heights[i]>=0);first=std::min(first,starts[i]);last=std::max(last,starts[i]+durations[i]);}
  assert(last-first<100);
  for(I time=first;time<last;++time){I used=0;
    for(std::size_t i=0;i<starts.size();++i)if(starts[i]<=time&&time<starts[i]+durations[i])used+=heights[i];
    if(used>bound)return false;
  }
  return true;
}
void cumulatives(){
  for(const auto& id:{"gecode_cumulatives","cumulatives"})for(std::size_t size=0;size<=4;++size){
    std::size_t patterns=size==0?1:size==1?9:size==2?81:18;
    for(std::size_t pattern=0;pattern<patterns;++pattern)for(I bound=0;bound<=4;++bound){
      F::Records r;std::vector<F::Value> starts,duration_values,height_values;Point durations,heights;auto code=pattern;
      for(std::size_t i=0;i<size;++i){
        r.raw_variables.push_back(variable(i,-2,2));starts.push_back(ref(i));
        I duration,height;
        if(size<=2){duration=code%3;code/=3;height=code%3;code/=3;}
        else {duration=(pattern+i)%3;height=(pattern/3+2*i)%3;}
        durations.push_back(duration);heights.push_back(height);duration_values.push_back(integer(duration));height_values.push_back(integer(height));
      }
      r.raw_constraints={row(id,{array(starts),array(duration_values),array(height_values),integer(bound)})};
      oracle(r,std::vector<std::pair<I,I>>(size,{-2,2}),[&](const Point& p){return cumulative_point(p,durations,heights,bound);});
    }
  }
  // Repeated/aliased starts remain separate demands; literals get fixed slots.
  for(int variant=0;variant<4;++variant){F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2)};
    if(variant==1){r.raw_variables[1].alias=true;r.raw_variables[1].target={F::Type::Integer,0};}
    r.raw_constraints={row("gecode_cumulatives",{array({ref(0),variant==0?ref(0):variant==2?integer(0):ref(1)}),array({integer(2),integer(1)}),array({integer(2),integer(1)}),integer(2)})};
    oracle(r,{{-2,2},{-2,2}},[&](const Point& p){return(variant!=1||p[0]==p[1])&&cumulative_point({p[0],variant==0?p[0]:variant==2?0:p[1]},{2,1},{2,1},2);});
  }
  // Legacy p_cumulatives has a singleton height<=bound shortcut which rejects
  // this zero-duration task. The independent half-open oracle and typed/native
  // route must accept it; a positive duration must remain infeasible.
  for(I duration:{I(0),I(1)})for(const auto& id:{"gecode_cumulatives","cumulatives"}){
    F::Records r;r.raw_variables={variable(0,-2,2)};
    r.raw_constraints={row(id,{array({ref(0)}),array({integer(duration)}),array({integer(2)}),integer(1)})};
    oracle(r,{{-2,2}},[&](const Point& p){return cumulative_point(p,{duration},{2},1);});
    auto c=compiled(r);O::SolveOptions options;options.backend=O::Backend::Native;options.guarantee=O::Guarantee::Exact;auto result=O::solve(c.model(),options);
    if(O::capabilities(O::Backend::Native).available){
      assert(result.termination==(duration?O::Termination::Infeasible:O::Termination::Optimal));
      if(!duration)assert(O::validate_flatzinc(c,result,0).valid);
    }else assert(result.termination==O::Termination::Unsupported);
  }
}
void cumulative_parameters(){
  F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2),variable(10,0,3),variable(11,0,3),variable(12,0,3),variable(13,0,3)};
  r.raw_variables[2].domain.integers=members({2,0,2}).set;
  r.raw_variables[3].assigned=true;r.raw_variables[3].value=integer(1);
  r.raw_variables[5].alias=true;r.raw_variables[5].target={F::Type::Integer,12};
  r.raw_domains={row("int_in",{ref(10),domain(1,3)}),row("int_in",{ref(13),members({2,2})})};
  r.raw_constraints={row("gecode_cumulatives",{array({ref(0),ref(1)}),array({ref(10),integer(1)}),array({ref(11),ref(11)}),ref(13)})};
  oracle(r,{{-2,2},{-2,2},{0,3},{0,3},{0,3},{0,3}},[](const Point& p){return p[2]==2&&p[3]==1&&p[4]==2&&p[5]==2&&cumulative_point({p[0],p[1]},{2,1},{1,1},2);});
  auto c=compiled(r);const auto& payload=std::get<O::CumulativeData>(c.model().globals.back().payload);
  assert(payload.durations==Point({2,1})&&payload.heights==Point({1,1})&&payload.capacity==2);
  assert(c.variables().size()==6&&c.model().variables.size()==5); // alias only, no hidden parameter slots
  r.output={{"duration",ref(10)},{"height",ref(11)},{"capacity",ref(13)}};auto output=compiled(r);auto historical=witness(output,{-1,0,2,1,2,2});
  r.raw_variables.clear();r.raw_constraints.clear();r.raw_domains.clear();r.output.clear();
  assert(O::validate_flatzinc(output,historical,0).valid&&O::format_flatzinc_solution(output,historical,0)=="duration = 2;\nheight = 1;\ncapacity = 2;\n----------\n");
  historical.values[output.variables()[2].variable.id]=0;assert(!O::validate_flatzinc(output,historical,0).valid);
  // No inference from an ordinary equality row, a lower bound, or an empty
  // original domain which happened to produce the compiler's dummy zero slot.
  for(int parameter=0;parameter<3;++parameter){F::Records source;source.raw_variables={variable(0,-1,1),variable(1,0,2)};
    source.raw_constraints={row("gecode_cumulatives",{array({ref(0)}),array({parameter==0?ref(1):integer(1)}),array({parameter==1?ref(1):integer(1)}),parameter==2?ref(1):integer(1)}),row("int_eq",{ref(1),integer(1)})};
    rejected(source);source.raw_variables[1].domain.integers=members({}).set;rejected(source);
  }
}
void cumulative_boundaries(){
  F::Records r;r.raw_variables={variable(0,-1,1),variable(1,0,1,F::Type::Boolean)};
  const auto valid=std::vector<F::Value>{array({ref(0)}),array({integer(1)}),array({integer(1)}),integer(1)};
  const auto reject=[&](std::vector<F::Value> args,O::FlatZincCompileStatus status=O::FlatZincCompileStatus::InvalidInput){r.raw_constraints={row("gecode_cumulatives",std::move(args))};rejected(r,status);};
  reject({});reject({array({}),array({}),array({})});
  auto changed=valid;changed.push_back(boolean(true));reject(changed);
  changed.push_back(integer(0));reject(changed,O::FlatZincCompileStatus::Unsupported);
  changed.push_back(integer(0));reject(changed,O::FlatZincCompileStatus::Unsupported);
  changed.push_back(integer(0));reject(changed);
  for(int index=0;index<4;++index){changed=valid;changed[index]=index==3?boolean(true):integer(0);reject(changed);}
  for(int index=0;index<3;++index){changed=valid;changed[index]=array({});reject(changed);}
  for(int index=0;index<3;++index){changed=valid;changed[index]=array({boolean(true)});reject(changed);
    changed[index]=array({ref(1,F::Type::Boolean)});reject(changed);changed[index]=array({ref(9)});reject(changed);}
  for(int index:{1,2}){changed=valid;changed[index]=array({integer(-1)});reject(changed);}
  changed=valid;changed[3]=integer(-1);reject(changed,O::FlatZincCompileStatus::Unsupported);
  constexpr I large=9007199254740992LL;
  for(int index=0;index<4;++index){changed=valid;changed[index]=index==3?integer(large+1):array({integer(large+1)});reject(changed,O::FlatZincCompileStatus::Unsupported);}
  for(const auto& id:{"fzn_cumulative","fzn_cumulatives","gecode_cumulative","gecode_gecode_cumulatives","gecode_schedule_cumulative_optional"}){r.raw_constraints={row(id,valid)};rejected(r);}
  // Both selected names reject multi-machine six-/seven-argument forms rather
  // than truncating machine/bound/polarity data to a single resource.
  for(const auto& id:{"gecode_cumulatives","cumulatives"})for(int size:{6,7}){r.raw_constraints={row(id,std::vector<F::Value>(size,integer(0)))};rejected(r);}
  F::Records wide;wide.raw_constraints={row("gecode_cumulatives",{array({integer(large)}),array({integer(large)}),array({integer(1)}),integer(1)})};
  auto c=compiled(wide);assert(O::validate_flatzinc(c,witness(c,{}),0).valid);
  O::SolveOptions native;native.backend=O::Backend::Native;native.guarantee=O::Guarantee::Exact;
  assert(O::solve(c.model(),native).termination==O::Termination::Unsupported);
  // The existing native energy/activity envelope remains a separate gate.
  F::Records energy;energy.raw_variables={variable(0,0,1073741823),variable(1,0,1073741823)};
  energy.raw_constraints={row("gecode_cumulatives",{array({ref(0),ref(1)}),array({integer(1),integer(1)}),array({integer(1073741823),integer(1073741823)}),integer(1073741823)})};
  auto guarded=compiled(energy);assert(O::validate_flatzinc(guarded,witness(guarded,{0,1}),0).valid);
  assert(O::solve(guarded.model(),native).termination==O::Termination::Unsupported);
  F::Records bounded;bounded.raw_variables={variable(0,-1,1)};
  bounded.raw_constraints={row("gecode_cumulatives",{array({ref(0),integer(0)}),array({integer(1),integer(2)}),array({integer(1),integer(1)}),integer(2)})};
  auto full=O::compile_flatzinc(bounded);assert(full.compiled&&full.work>0);
  for(std::size_t cap:{std::size_t(0),full.work/2,full.work-1}){O::FlatZincCompileOptions options;options.max_work=cap;auto result=O::compile_flatzinc(bounded,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled&&result.work<=cap);}
  O::FlatZincCompileOptions options;options.max_work=full.work;assert(O::compile_flatzinc(bounded,options).compiled);
  for(int mode=0;mode<3;++mode){options={};if(mode==0)options.max_variables=1;if(mode==1)options.max_constraints=0;if(mode==2)options.max_nonzeros=6;auto result=O::compile_flatzinc(bounded,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);}
  options={};options.max_variables=2;options.max_constraints=1;options.max_nonzeros=7;assert(O::compile_flatzinc(bounded,options).compiled);
  options={};options.time_limit_seconds=0;auto stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::TimeLimit&&!stopped.compiled);
  options={};options.cancellation=std::make_shared<O::CancellationToken>();options.cancellation->cancel();stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::Cancelled&&!stopped.compiled);
  F::Records empty;empty.raw_constraints={row("cumulatives",{array({}),array({}),array({}),integer(0)})};options={};options.max_nonzeros=0;
  assert(O::compile_flatzinc(empty,options).status==O::FlatZincCompileStatus::ResourceLimit);options.max_nonzeros=1;assert(O::compile_flatzinc(empty,options).compiled);
}
void cumulative_combined(){
  F::Records r;r.raw_variables={variable(0,-2,2),variable(1,-2,2),variable(0,0,1,F::Type::Boolean)};
  r.raw_variables[0].domain.integers=members({-2,0,2}).set;
  r.raw_constraints={row("gecode_cumulatives",{array({ref(0),ref(1)}),array({integer(2),integer(1)}),array({integer(2),integer(1)}),integer(2)}),
    row("all_different_int",{array({ref(0),ref(1)})}),row("array_var_int_element",{integer(2),array({ref(0),ref(1)}),ref(1)}),
    row("gecode_table_int",{array({ref(1)}),array({integer(-2),integer(0),integer(2)})}),row("int_le_reif",{ref(0),ref(1),ref(0,F::Type::Boolean)})};
  oracle(r,{{-2,2},{-2,2},{0,1}},[](const Point& p){return p[0]%2==0&&p[1]%2==0&&p[0]!=p[1]&&bool(p[2])==(p[0]<=p[1])&&cumulative_point({p[0],p[1]},{2,1},{2,1},2);});
  for(auto method:{F::Method::Minimize,F::Method::Maximize}){r.solve.method=method;r.solve.has_objective=true;r.solve.objective=ref(0);auto c=compiled(r);
    for(auto backend:{O::Backend::Native,O::Backend::Auto,O::Backend::Highs}){O::SolveOptions options;options.backend=backend;options.guarantee=backend==O::Backend::Highs?O::Guarantee::Numerical:O::Guarantee::Exact;auto result=O::solve(c.model(),options);
      if(backend!=O::Backend::Highs&&O::capabilities(O::Backend::Native).available)assert(result.termination==O::Termination::Optimal&&result.objective==(method==F::Method::Minimize?-2:2)&&O::validate_flatzinc(c,result,0).valid);
      else assert(result.termination==O::Termination::Unsupported&&!result.has_solution());
    }
  }
}

// Generate accepted words by enumerating source-labeled paths, independently
// of the compiler's sparse zero-based graph and the source checker's indexing.
std::set<Point> regular_words(I states,I symbols,const Point& transitions,I initial,
                             const std::set<I>& finals,std::size_t length){
  assert(transitions.size()==static_cast<std::size_t>(states*symbols));std::set<Point> accepted;Point word;
  const auto paths=[&](const auto& self,I state)->void {
    if(word.size()==length){if(finals.count(state))accepted.insert(word);return;}
    for(I symbol=1;symbol<=symbols;++symbol){I next=0;std::size_t cell=0;
      for(I from=1;from<=states;++from)for(I label=1;label<=symbols;++label,++cell)
        if(from==state&&label==symbol)next=transitions[cell];
      if(next){word.push_back(symbol);self(self,next);word.pop_back();}
    }
  };paths(paths,initial);return accepted;
}
void regulars(){
  for(unsigned pattern=0;pattern<81;++pattern)for(I initial=1;initial<=2;++initial)for(unsigned final=0;final<4;++final)for(std::size_t size=0;size<=3;++size){
    F::Records r;std::vector<F::Value> word,flat;Point edges,final_values;std::set<I> finals;auto code=pattern;
    for(unsigned i=0;i<4;++i){edges.push_back(code%3);flat.push_back(integer(code%3));code/=3;}
    for(I state=1;state<=2;++state)if(final&(1u<<(state-1))){finals.insert(state);final_values.push_back(state);}
    for(std::size_t i=0;i<size;++i){r.raw_variables.push_back(variable(i,0,3));word.push_back(ref(i));}
    r.raw_constraints={row("gecode_regular",{array(word),integer(2),integer(2),array(flat),integer(initial),members(final_values)})};
    const auto accepted=regular_words(2,2,edges,initial,finals,size);
    oracle(r,std::vector<std::pair<I,I>>(size,{0,3}),[&](const Point& p){return accepted.count(p)!=0;});
    if(pattern%19==0&&initial==2&&final==2){auto c=compiled(r);O::SolveOptions options;options.backend=O::Backend::Native;options.guarantee=O::Guarantee::Exact;auto result=O::solve(c.model(),options);
      if(O::capabilities(O::Backend::Native).available){assert(result.termination==(accepted.empty()?O::Termination::Infeasible:O::Termination::Optimal));if(!accepted.empty())assert(O::validate_flatzinc(c,result,0).valid);}
      else assert(result.termination==O::Termination::Unsupported);
    }
  }
  // Every source position survives aliasing and fixed-literal lowering.
  for(int mode=0;mode<4;++mode){F::Records r;r.raw_variables={variable(0,0,3),variable(1,0,3)};
    if(mode==1){r.raw_variables[1].alias=true;r.raw_variables[1].target={F::Type::Integer,0};}
    r.raw_constraints={row("gecode_regular",{array({ref(0),mode==0?ref(0):mode==2?integer(2):ref(1)}),integer(2),integer(2),array({integer(1),integer(2),integer(2),integer(0)}),integer(1),members({2,2})})};
    const auto accepted=regular_words(2,2,{1,2,2,0},1,{2},2);
    oracle(r,{{0,3},{0,3}},[&](const Point& p){return(mode!=1||p[0]==p[1])&&accepted.count({p[0],mode==0?p[0]:mode==2?2:p[1]});});
  }
  for(const auto& finals:{members({}),members({2,1,2}),domain(1,2),domain(3,2)})for(std::size_t size:{0u,1u}){
    F::Records r;std::vector<F::Value> word;if(size){r.raw_variables={variable(0,0,3)};word={ref(0)};}
    r.raw_constraints={row("gecode_regular",{array(word),integer(2),integer(2),array({integer(0),integer(0),integer(0),integer(0)}),integer(1),finals})};
    const bool initial_final=finals.set.interval?(finals.set.lower<=1&&1<=finals.set.upper):std::find(finals.set.values.begin(),finals.set.values.end(),1)!=finals.set.values.end();
    oracle(r,std::vector<std::pair<I,I>>(size,{0,3}),[&](const Point&){return!size&&initial_final;});
  }
}
void regular_boundaries(){
  F::Records r;r.raw_variables={variable(0,1,2),variable(1,1,1),variable(0,0,1,F::Type::Boolean)};
  const std::vector<F::Value> valid={array({ref(0)}),integer(2),integer(2),array({integer(1),integer(2),integer(2),integer(1)}),integer(1),members({2})};
  const auto reject=[&](std::vector<F::Value> args,O::FlatZincCompileStatus status=O::FlatZincCompileStatus::InvalidInput){r.raw_constraints={row("gecode_regular",std::move(args))};rejected(r,status);};
  for(std::size_t n=0;n<=8;++n)if(n!=6)reject(std::vector<F::Value>(n,integer(0)));
  for(std::size_t i=0;i<6;++i){auto changed=valid;changed[i]=boolean(true);reject(changed);}
  for(std::size_t i:{1u,2u,4u}){auto changed=valid;changed[i]=ref(1);reject(changed);}
  for(std::size_t i:{1u,2u})for(I value:{I(-1),I(0)}){auto changed=valid;changed[i]=integer(value);reject(changed);}
  for(I initial:{I(0),I(3)}){auto changed=valid;changed[4]=integer(initial);reject(changed);}
  for(std::size_t n:{0u,3u,5u}){auto changed=valid;changed[3]=array(std::vector<F::Value>(n,integer(0)));reject(changed);}
  for(const auto& cell:{integer(-1),integer(3),boolean(true),ref(1),ref(0,F::Type::Boolean)}){auto changed=valid;changed[3].elements[0]=cell;reject(changed);}
  for(const auto& final:{members({0}),members({3}),domain(0,1),domain(1,3),ref(1)}){auto changed=valid;changed[5]=final;reject(changed);}
  for(const auto& term:{boolean(true),ref(0,F::Type::Boolean),ref(9),array({integer(1)})}){auto changed=valid;changed[0]=array({term});reject(changed);}
  constexpr I huge=9007199254740992LL;
  for(std::size_t i:{1u,2u,4u}){auto changed=valid;changed[i]=integer(huge+1);reject(changed,O::FlatZincCompileStatus::Unsupported);}
  auto changed=valid;changed[1]=integer(huge);changed[2]=integer(huge);reject(changed,O::FlatZincCompileStatus::Unsupported);
  changed=valid;changed[3].elements[0]=integer(huge+1);reject(changed,O::FlatZincCompileStatus::Unsupported);
  changed=valid;changed[5]=members({huge+1});reject(changed,O::FlatZincCompileStatus::Unsupported);
  for(const auto& id:{"regular","fzn_regular","gecode_regular_set","gecode_regular_reif","gecode_gecode_regular","gecode_regular_nfa"}){r.raw_constraints={row(id,valid)};rejected(r);}
  // Even an empty word or an earlier contradiction cannot bypass admission.
  changed=valid;changed[0]=array({});changed[3].elements[0]=integer(-1);reject(changed);
  r.raw_variables[0].domain.integers=domain(1,0).set;reject(changed);r.raw_variables[0].domain.integers=domain(1,2).set;
  // Exact payload accounting: three scalar parameters, word positions, input
  // matrix cells, three cells per nonzero transition and expanded final IDs.
  F::Records bounded;bounded.raw_variables={variable(0,1,2)};
  bounded.raw_constraints={row("gecode_regular",{array({ref(0),integer(1)}),integer(2),integer(2),array({integer(1),integer(2),integer(0),integer(0)}),integer(1),domain(1,2)})};
  auto full=O::compile_flatzinc(bounded);assert(full.compiled&&full.work>0);
  for(auto cap:{std::size_t(0),full.work/2,full.work-1}){O::FlatZincCompileOptions options;options.max_work=cap;auto result=O::compile_flatzinc(bounded,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled&&result.work<=cap);}
  O::FlatZincCompileOptions options;options.max_work=full.work;assert(O::compile_flatzinc(bounded,options).compiled);
  for(int mode=0;mode<3;++mode){options={};if(mode==0)options.max_variables=1;if(mode==1)options.max_constraints=0;if(mode==2)options.max_nonzeros=16;auto result=O::compile_flatzinc(bounded,options);assert(result.status==O::FlatZincCompileStatus::ResourceLimit&&!result.compiled);}
  options={};options.max_variables=2;options.max_constraints=1;options.max_nonzeros=17;assert(O::compile_flatzinc(bounded,options).compiled);
  options={};options.time_limit_seconds=0;auto stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::TimeLimit&&!stopped.compiled);
  options={};options.cancellation=std::make_shared<O::CancellationToken>();options.cancellation->cancel();stopped=O::compile_flatzinc(bounded,options);assert(stopped.status==O::FlatZincCompileStatus::Cancelled&&!stopped.compiled);
  bounded.raw_constraints[0].arguments[5]=domain(1,2);options={};options.max_nonzeros=1;assert(O::compile_flatzinc(bounded,options).status==O::FlatZincCompileStatus::ResourceLimit);
}
void regular_combined(){
  F::Records r;r.raw_variables={variable(0,0,3),variable(1,0,3),variable(0,0,1,F::Type::Boolean),variable(9,-3,-2)};
  r.raw_variables[0].domain.integers=members({2,1,2}).set;
  r.raw_constraints={row("gecode_regular",{array({ref(0),ref(1)}),integer(3),integer(2),array({integer(2),integer(3),integer(0),integer(3),integer(3),integer(0)}),integer(1),members({3})}),
    row("gecode_table_int",{array({ref(1)}),array({integer(1),integer(2)})}),row("all_different_int",{array({ref(0),ref(1)})}),
    row("int_le_reif",{ref(0),ref(1),ref(0,F::Type::Boolean)}),row("int_plus",{ref(9),integer(4),ref(0)})};
  oracle(r,{{0,3},{0,3},{0,1},{-3,-2}},[](const Point& p){return((p[0]==1&&p[1]==2)||(p[0]==2&&p[1]==1))&&bool(p[2])==(p[0]<=p[1])&&p[3]+4==p[0];});
  for(auto method:{F::Method::Minimize,F::Method::Maximize}){
    r.solve.method=method;r.solve.has_objective=true;r.solve.objective=ref(9);r.output={{"first",ref(0)},{"objective",ref(9)}};auto c=compiled(r);
    for(auto backend:{O::Backend::Native,O::Backend::Auto,O::Backend::Highs}){O::SolveOptions options;options.backend=backend;options.guarantee=backend==O::Backend::Highs?O::Guarantee::Numerical:O::Guarantee::Exact;auto result=O::solve(c.model(),options);
      if(backend!=O::Backend::Highs&&O::capabilities(O::Backend::Native).available){assert(result.termination==O::Termination::Optimal&&result.objective==(method==F::Method::Minimize?-3:-2)&&O::validate_flatzinc(c,result,0).valid);assert(!O::format_flatzinc_solution(c,result,0).empty());}
      else assert(result.termination==O::Termination::Unsupported&&!result.has_solution());
    }
  }
  r.solve={};auto c=compiled(r);auto historical=witness(c,{1,2,1,-3});r.raw_variables.clear();r.raw_constraints.clear();assert(O::validate_flatzinc(c,historical,0).valid);
  historical.values[c.variables()[1].variable.id]=0;assert(!O::validate_flatzinc(c,historical,0).valid);
}

int main(){regulars();regular_boundaries();regular_combined();cumulatives();cumulative_parameters();cumulative_boundaries();cumulative_combined();circuits();circuit_boundaries();circuit_combined();holey_domains();tables();table_domain_limits();table_domain_backends();comparisons();booleans();aliases_domains();guarded();globals();failures();history_output();actual_backends();assert(configurations>500&&assignments>20000);std::cout<<"FlatZinc source compiler: "<<configurations<<" configurations, "<<assignments<<" independent assignments\n";}
