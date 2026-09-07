#include <gecode/flatzinc/capture.hh>
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/registry.hh>
#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
using namespace Gecode::FlatZinc;
namespace C=Gecode::FlatZinc::Capture;
namespace {
unsigned checked=0;
std::shared_ptr<const C::Records> good(const std::string& text) {
  auto r=C::parse_string(text);++checked;
  if(r.status!=C::Status::Complete){
    std::cerr<<text<<"\n";for(const auto& d:r.diagnostics)std::cerr<<d.message<<"\n";
  }
  assert(r.status==C::Status::Complete&&r.records&&r.diagnostics.empty());return r.records;
}
void reject(const std::string& text,C::Status expected=C::Status::InvalidInput,C::Options options={}) {
  auto r=C::parse_string(text,options);++checked;
  if(r.status!=expected)std::cerr<<"unexpected status "<<int(r.status)<<" for "<<text<<"\n";
  assert(r.status==expected&&!r.records&&!r.diagnostics.empty());
}
const C::Variable& variable(const C::Records& r,C::Type type,std::size_t slot,bool raw=true){
  for(const auto& v:raw?r.raw_variables:r.variables)if(v.reference.type==type&&v.reference.index==slot)return v;
  throw std::runtime_error("missing variable");
}
int posted=0;
void poster(FlatZincSpace&,const ConExpr&,AST::Node*){++posted;}
void semantic_cases(){
  auto r=good("var {1,3,8}: x :: output_var; var 2..9: y = x; var int: z = 3; constraint int_eq(x,z); constraint int_le(y,8); solve minimize y;");
  assert(r->raw_variables.size()==3&&r->raw_domains.size()==1&&r->raw_constraints.size()==2);
  assert(variable(*r,C::Type::Integer,1).alias&&variable(*r,C::Type::Integer,1).target.index==0);
  assert(variable(*r,C::Type::Integer,2).assigned&&variable(*r,C::Type::Integer,2).value.integer==3);
  assert(variable(*r,C::Type::Integer,2,false).alias&&!variable(*r,C::Type::Integer,2,false).assigned);
  assert(r->raw_domains[0].arguments[0].reference.index==1&&r->raw_domains[0].arguments[1].set.lower==2);
  assert(r->domains.size()==2&&r->coverage.size()==2&&r->coverage[0].kind==C::CoverageKind::AliasEquality);
  assert(r->coverage[1].raw_constraint==1&&r->constraints.size()==1);
  assert(r->solve.method==C::Method::Minimize&&r->solve.objective.reference.index==1);
  assert(r->output.size()==1&&r->output[0].name=="x");
  auto b=good("var bool: b; var bool: c=true; var {false}: d=b; constraint bool_eq(b,c); constraint bool2int(b,1); solve satisfy;");
  assert(variable(*b,C::Type::Boolean,1).value.boolean);
  assert(b->raw_domains.size()==1&&b->raw_domains[0].arguments[0].reference.type==C::Type::Boolean);
  assert(b->domains.back().arguments[1].set.lower==1&&!b->solve.has_objective);
  auto typed=good("int: n=7; bool: p=true; float: f=1.25; set of int: s={2,7}; array[1..1] of var int: a=[n]; array[1..1] of var bool: b=[p]; array[1..1] of var float: c=[f]; array[1..1] of var set of int: d=[s]; solve maximize f;");
  assert(variable(*typed,C::Type::Integer,0).value.integer==7);
  assert(variable(*typed,C::Type::Boolean,0).value.boolean);
  assert(variable(*typed,C::Type::Float,0).value.floating==1.25);
  assert(variable(*typed,C::Type::Set,0).value.set.values.size()==2);
  assert(typed->solve.objective.floating==1.25&&typed->raw_variables.size()==4);
  for(const auto& literal:std::vector<std::pair<std::string,int>>{{"23",23},{"-23",-23},{"0o10",8},{"010",10},{"08",8},{"0x1a",26}}){
    auto k=good("solve minimize "+literal.first+";");assert(k->raw_variables.empty()&&k->solve.objective.integer==literal.second);
  }
  auto f=good("var 1.0..2.0: f; var 1.5..3.0: g=f; var set of {1,3}: s; var set of int: t=s; solve maximize g;");
  assert(f->raw_variables.size()==4&&f->raw_domains.size()==2&&f->solve.objective.reference.type==C::Type::Float);
  auto a=good("var 0..5: x; array[1..3] of var 1..4: a :: output_array([0..2])=[x,x,3]; constraint custom(a, [x,3], true); solve minimize a[3];");
  assert(a->raw_variables.size()==2&&a->raw_domains.size()==3&&a->raw_constraints[0].arguments[0].elements.size()==3);
  assert(a->solve.objective.reference.index==1&&a->output.size()==1);
  assert(a->output[0].expression.elements[1].elements[0].reference.index==0);
  assert(a->output[0].expression.elements[1].elements[1].reference.index==0);
  auto empty=good("array[1..0] of var 1..2: a :: output_array([1..0]); solve satisfy;");
  assert(empty->raw_variables.empty()&&empty->output[0].expression.elements[1].elements.empty());
  auto reif=good("var bool: b; var 0..3: x; constraint int_le_reif(x,2,b); solve :: int_search([x],input_order,indomain_min,complete) satisfy;");
  assert(reif->raw_constraints[0].id=="int_le_reif"&&reif->solve.annotations.size()==1);
  auto annotations=good(R"(var 0..1: x :: one([1,2]) :: two(1,2) :: quoted("Hello,\n World\t\"quoted\"!"); solve satisfy;)");
  const auto& anns=annotations->declaration_annotations[0].annotations;
  assert(anns[0].elements.size()==1&&anns[0].elements[0].kind==C::ValueKind::Array&&anns[0].elements[0].elements.size()==2);
  assert(anns[1].elements.size()==2&&anns[1].elements[0].kind==C::ValueKind::Integer);
  assert(anns[2].elements[0].text=="Hello,\n World\t\"quoted\"!");
  const auto& output_ann=a->declaration_annotations.back().annotations[0];
  assert(output_ann.elements.size()==1&&output_ann.elements[0].kind==C::ValueKind::Array);
  auto canonical=good("var {3,1,3}: x; solve satisfy;");
  assert((variable(*canonical,C::Type::Integer,0).domain.integers.values==std::vector<std::int64_t>{1,3}));
  auto params=good("array[1..2] of bool: flags=[true,false]; constraint custom(flags[2]); solve satisfy;");
  assert(params->raw_constraints[0].arguments[0].kind==C::ValueKind::Boolean&&!params->raw_constraints[0].arguments[0].boolean);
  std::ostringstream chain;chain<<"array[1..512] of var int: a;";
  for(int i=512;i>1;--i)chain<<"constraint int_eq(a["<<i<<"],a["<<i-1<<"]);";
  for(int i=0;i<512;++i)chain<<"constraint int_eq(a[512],a[1]);";
  chain<<"solve satisfy;";auto normalized=good(chain.str());
  assert(normalized->variables.size()==512&&normalized->coverage.size()==1023);
  for(std::size_t i=1;i<512;++i)assert(normalized->variables[i].alias&&normalized->variables[i].target.index==0&&!normalized->raw_variables[i].alias);
  registry().add("capture_test_never_post",poster);
  auto unknown=good("constraint capture_test_never_post(1); solve satisfy;");
  assert(posted==0&&unknown->raw_constraints[0].id=="capture_test_never_post");
}
void malformed_cases(){
  for(const auto& s:std::vector<std::string>{
      "", "var int: x;", "constraint int_eq(); solve satisfy;", "constraint int_eq(1); solve satisfy;",
      "constraint int_eq(true,1); solve satisfy;", "constraint bool2int(1,true); solve satisfy;",
      "constraint foo(missing); solve satisfy;", "var int: x=missing; solve satisfy;",
      "int: x :: ignored = 1; solve satisfy;",
      "array[1..1] of int: x :: ignored = [1]; solve satisfy;",
      "int: x=true; solve satisfy;", "bool: x=1; solve satisfy;", "float: x=false; solve satisfy;", "set of int: x=1; solve satisfy;",
      "var int: x=true; solve satisfy;", "var bool: x=1; solve satisfy;", "var float: x=false; solve satisfy;", "var set of int: x=1; solve satisfy;",
      "var int: x; var int: x; solve satisfy;", "solve minimize missing;", "solve minimize missing[-1];",
      "array[1..1] of var int: a; solve minimize a[-1];", "array[1..1] of var int: a; solve minimize a[2];",
      "int: a=1; solve minimize a[1];", "array[1..1] of var bool: a; solve minimize a[1];",
      "array[1..2] of var int: a=[1]; solve satisfy;", "array[1..1] of var int: a=[true]; solve satisfy;",
      "array[0..1] of var int: a; solve satisfy;", "array[1..1] of var int: a :: output_array([true]); solve satisfy;",
      "array[1..1] of var int: a :: output_array([1..2]); solve satisfy;",
      "var int: a :: output_array([1..1]); solve satisfy;",
      "array[1..0] of var int: a :: output_array([]); solve satisfy;",
      "constraint foo(missing[missing]); solve satisfy;", "var int: x :: foo([1,2; solve satisfy;",
      "solve :: name(\"unfinished) satisfy;",
      "solve minimize 9999999999999999999999999;", "solve minimize 1.0e999;", "solve minimize 1.0e-999;"})reject(s);
  reject("constraint gecode_on_restart_status(1); solve satisfy;",C::Status::Unsupported);
  reject(std::string("solve satisfy;\0",15));
  C::Options o;o.max_input_bytes=4;reject("solve satisfy;",C::Status::ResourceLimit,o);
  o={};o.max_array_elements=1;reject("array[1..2] of var int: a; solve satisfy;",C::Status::ResourceLimit,o);
  o={};o.max_variables=1;reject("array[1..2] of var int: a; solve satisfy;",C::Status::ResourceLimit,o);
  reject("var int: x; var int: y; solve satisfy;",C::Status::ResourceLimit,o);
  o={};o.max_constraints=0;reject("constraint int_eq(1,1); solve satisfy;",C::Status::ResourceLimit,o);
  reject("var 1..2: x=1; solve satisfy;",C::Status::ResourceLimit,o);
  o.max_constraints=1;reject("var 1..2: x=1; constraint int_eq(x,1); solve satisfy;",C::Status::ResourceLimit,o);
  o.max_constraints=0;reject("var 1.0..2.0: x=1.5; solve satisfy;",C::Status::ResourceLimit,o);
  o={};o.max_value_depth=2;reject("solve :: a(b(c(1))) satisfy;",C::Status::ResourceLimit,o);
  reject(R"(solve :: unsupported("\123") satisfy;)",C::Status::Unsupported);
  reject(std::string("solve :: text(\"\xc0\xaf\") satisfy;"));
  auto unicode=good(u8"solve :: text(\"λ😀\") satisfy;");assert(unicode->solve.annotations[0].elements[0].text==u8"λ😀");
  std::istringstream input("solve maximize 9;");input.exceptions(std::ios::failbit|std::ios::badbit);
  auto r=C::parse(input);assert(r.status==C::Status::Complete&&r.records->solve.objective.integer==9);
  struct BadBuffer:std::streambuf{int_type underflow()override{throw std::runtime_error("broken source");}} buffer;
  std::istream broken(&buffer);broken.exceptions(std::ios::badbit);
  auto failed=C::parse(broken);assert(failed.status==C::Status::InvalidInput&&!failed.records);
  std::istringstream exact("solve satisfy;");C::Options exact_options;exact_options.max_input_bytes=14;
  auto equal=C::parse(exact,exact_options);assert(equal.status==C::Status::Complete);
  std::istringstream over("solve satisfy; ");auto too_long=C::parse(over,exact_options);assert(too_long.status==C::Status::ResourceLimit&&!too_long.records);
}
void legacy_case(){
  std::istringstream input("var 1..3: x :: output_var; constraint int_eq(x,2); solve satisfy;");
  Printer printer;std::ostringstream errors;std::unique_ptr<FlatZincSpace> space(Gecode::FlatZinc::parse(input,printer,errors));
  assert(space&&errors.str().empty()&&space->iv.size()==1&&space->iv[0].assigned()&&space->iv[0].val()==2);
}
}
int main(int argc,char**){
  semantic_cases();malformed_cases();if(argc==1)legacy_case();
  for(int n=0;n<20;++n){good("var int: x; array[1..2] of var int: a=[x,x]; solve satisfy;");reject("var int: x; array[1..2] of var int: a=[x]; solve satisfy;");}
  const std::string source="var {1,3}: x :: output_var; array[1..2] of var int: a :: output_array([1..2])=[x,2]; constraint int_le(x,3); solve minimize x;";
  for(std::size_t n=0;n<source.size();++n)reject(source.substr(0,n));
  std::atomic<bool> correct{true};std::vector<std::thread> workers;
  for(int n=0;n<4;++n)workers.emplace_back([&]{for(int k=0;k<8;++k){auto r=C::parse_string(source);if(r.status!=C::Status::Complete||!r.records||r.records->raw_variables.size()!=2)correct=false;}});
  for(auto& worker:workers)worker.join();assert(correct);
  std::cout<<"FlatZinc capture "<<checked<<" checks passed\n";
}
