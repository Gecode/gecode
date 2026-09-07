#include <gecode/flatzinc/capture.hh>
#include <gecode/flatzinc/parser.hh>
#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <array>

int yyparse(void*);
int yylex_init(void**);
int yylex_destroy(void*);
int yyget_lineno(void*);
void yyset_extra(void*,void*);

namespace Gecode { namespace FlatZinc {
namespace C=Capture;
namespace {
struct CaptureFailure { C::Status status; std::string message; };
[[noreturn]] void bad(const std::string& text){throw CaptureFailure{C::Status::InvalidInput,text};}
[[noreturn]] void limit(const std::string& text){throw CaptureFailure{C::Status::ResourceLimit,text};}
C::Location location(ParserState& p,std::size_t ordinal=0){
  int line=p.yyscanner?yyget_lineno(p.yyscanner):0;
  return {p.capture->options.source,static_cast<std::size_t>(std::max(0,line)),ordinal};
}
C::Reference ref(C::Type type,int index){if(index<0)bad("negative variable reference");return {type,static_cast<std::size_t>(index)};}
C::SetLiteral set(AST::SetLit* s){
  C::SetLiteral out;out.interval=s->interval;
  if(s->interval){out.lower=s->min;out.upper=s->max;}
  else {
    for(int v:s->s)out.values.push_back(v);
    std::sort(out.values.begin(),out.values.end());
    out.values.erase(std::unique(out.values.begin(),out.values.end()),out.values.end());
  }
  return out;
}
std::string string_value(const std::string& source){
  std::string out;
  for(std::size_t i=0;i<source.size();++i){
    if(source[i]!='\\'){out.push_back(source[i]);continue;}
    if(++i==source.size())bad("unterminated string escape");
    switch(source[i]){
      case '\\':out.push_back('\\');break;case '"':out.push_back('"');break;
      case '\'':out.push_back('\'');break;case '?':out.push_back('?');break;
      case 'a':out.push_back('\a');break;case 'b':out.push_back('\b');break;
      case 'f':out.push_back('\f');break;case 'n':out.push_back('\n');break;
      case 'r':out.push_back('\r');break;case 't':out.push_back('\t');break;
      case 'v':out.push_back('\v');break;
      default:throw CaptureFailure{C::Status::Unsupported,"numeric or unknown string escape is unsupported by capture"};
    }
  }
  return out;
}
C::Value value(ParserState& p,AST::Node* n,std::size_t depth=0){
  if(!n)bad("missing expression");
  if(depth>p.capture->options.max_value_depth)limit("capture expression nesting exceeds limit");
  C::Value out;
  if(n->isInt()){out.kind=C::ValueKind::Integer;out.integer=n->getInt();}
  else if(n->isBool()){out.kind=C::ValueKind::Boolean;out.boolean=n->getBool();}
  else if(n->isFloat()){out.kind=C::ValueKind::Float;out.floating=n->getFloat();if(!std::isfinite(out.floating))bad("float literal must be finite");}
  else if(n->isSet()){out.kind=C::ValueKind::Set;out.set=set(n->getSet());}
  else if(n->isIntVar()){out.kind=C::ValueKind::Reference;out.reference=ref(C::Type::Integer,n->getIntVar());}
  else if(n->isBoolVar()){out.kind=C::ValueKind::Reference;out.reference=ref(C::Type::Boolean,n->getBoolVar());}
  else if(n->isFloatVar()){out.kind=C::ValueKind::Reference;out.reference=ref(C::Type::Float,n->getFloatVar());}
  else if(n->isSetVar()){out.kind=C::ValueKind::Reference;out.reference=ref(C::Type::Set,n->getSetVar());}
  else if(n->isString()){out.kind=C::ValueKind::String;out.text=string_value(n->getString());}
  else if(n->isAtom()){out.kind=C::ValueKind::Atom;out.text=n->getAtom()->id;}
  else if(auto* a=dynamic_cast<AST::Array*>(n)){
    out.kind=C::ValueKind::Array;if(a->a.size()>p.capture->options.max_array_elements)limit("expression array exceeds capture limit");
    for(auto* x:a->a)out.elements.push_back(value(p,x,depth+1));
  }else if(auto* call=dynamic_cast<AST::Call*>(n)){
    out.kind=C::ValueKind::Call;out.text=call->id;
    if(auto* a=dynamic_cast<AST::Array*>(call->args)){
      if(a->a.size()>p.capture->options.max_array_elements)limit("call arity exceeds capture limit");
      for(auto* x:a->a)out.elements.push_back(value(p,x,depth+1));
    }else out.elements.push_back(value(p,call->args,depth+1));
  }else bad("unsupported AST expression in capture");
  return out;
}
std::vector<C::Value> values(ParserState& p,AST::Array* nodes){
  std::vector<C::Value> out;if(!nodes)return out;
  if(nodes->a.size()>p.capture->options.max_array_elements)limit("argument count exceeds capture limit");
  for(auto* node:nodes->a)out.push_back(value(p,node));return out;
}
C::Constraint constraint(ParserState& p,const ConExpr& ce,bool synthesized){
  return {ce.id,values(p,ce.args),values(p,ce.ann),location(p),synthesized};
}
std::vector<C::Variable> variables(ParserState& p){
  if(p.intvars.size()+p.boolvars.size()+p.floatvars.size()+p.setvars.size()>p.capture->options.max_variables)limit("variable count exceeds capture limit");
  std::vector<C::Variable> out;
  const auto add=[&](const std::vector<varspec>& source,C::Type type){
    for(std::size_t i=0;i<source.size();++i){
      VarSpec* spec=source[i].second;if(!spec)bad("missing variable specification");
      C::Variable v;v.reference={type,i};v.name=source[i].first;v.alias=spec->alias;v.introduced=spec->introduced;v.functionally_defined=spec->funcDep;
      if(v.alias){v.target=ref(type,spec->i);if(v.target.index>=i)bad("variable alias must name an earlier variable in its namespace");}
      else {
        v.assigned=spec->assigned;
        if(type==C::Type::Integer||type==C::Type::Boolean){
          const auto domain=type==C::Type::Integer?static_cast<IntVarSpec*>(spec)->domain:static_cast<BoolVarSpec*>(spec)->domain;
          v.domain.present=domain();if(domain())v.domain.integers=set(domain.some());
          if(v.assigned){v.value.kind=type==C::Type::Integer?C::ValueKind::Integer:C::ValueKind::Boolean;v.value.integer=spec->i;v.value.boolean=spec->i!=0;}
        }else if(type==C::Type::Float){
          auto domain=static_cast<FloatVarSpec*>(spec)->domain;v.domain.present=domain();
          if(domain()){v.domain.lower=domain.some().first;v.domain.upper=domain.some().second;
            if(!std::isfinite(v.domain.lower)||!std::isfinite(v.domain.upper))bad("float domain must be finite");}
          if(v.assigned){v.value.kind=C::ValueKind::Float;v.value.floating=v.domain.lower;}
        }else {
          auto domain=static_cast<SetVarSpec*>(spec)->upperBound;v.domain.present=domain();if(domain())v.domain.integers=set(domain.some());
          if(v.assigned){v.value.kind=C::ValueKind::Set;v.value.set=v.domain.integers;}
        }
      }
      out.push_back(std::move(v));
    }
  };
  add(p.intvars,C::Type::Integer);add(p.boolvars,C::Type::Boolean);add(p.setvars,C::Type::Set);add(p.floatvars,C::Type::Float);return out;
}
template<class F> void guard(CaptureParser& c,ParserState& p,F&& f){
  if(p.hadError)return;
  try{f();}catch(const CaptureFailure& e){c.fail(p,e.status,e.message);}
  catch(const AST::TypeError& e){c.fail(p,C::Status::InvalidInput,e.what());}
  catch(const std::out_of_range&){c.fail(p,C::Status::InvalidInput,"reference outside captured namespace");}
}
using Counts=std::array<std::size_t,4>;
Counts counts(const std::vector<C::Variable>& vars){
  Counts result{};for(const auto& v:vars)++result.at(static_cast<std::size_t>(v.reference.type));return result;
}
void check_ref(const C::Value& v,const Counts& ns){
  if(v.kind==C::ValueKind::Reference &&
     v.reference.index>=ns.at(static_cast<std::size_t>(v.reference.type)))
    bad("expression references a missing variable");
  for(const auto& child:v.elements)check_ref(child,ns);
}
void validate_signature(const C::Constraint& c){
  const auto& a=c.arguments;
  const auto is_int=[](const C::Value& v){return v.kind==C::ValueKind::Integer||(v.kind==C::ValueKind::Reference&&v.reference.type==C::Type::Integer);};
  const auto is_bool=[](const C::Value& v){return v.kind==C::ValueKind::Boolean||(v.kind==C::ValueKind::Reference&&v.reference.type==C::Type::Boolean);};
  const auto arity=[&](std::size_t n){if(a.size()!=n)bad("wrong argument count for "+c.id);};
  if(c.id=="int_eq"||c.id=="int_ne"||c.id=="int_le"||c.id=="int_lt"||c.id=="int_ge"||c.id=="int_gt"){
    arity(2);if(!is_int(a[0])||!is_int(a[1]))bad("integer comparison arguments have wrong types");
  }else if(c.id=="bool_eq"||c.id=="bool_le"||c.id=="bool_lt"||c.id=="bool_not"){
    arity(2);if(!is_bool(a[0])||!is_bool(a[1]))bad("Boolean comparison arguments have wrong types");
  }else if(c.id=="bool2int"){
    arity(2);if(!is_bool(a[0])||!is_int(a[1]))bad("bool2int arguments have wrong types");
  }
}
void normalize(C::Records& r){
  r.variables=r.raw_variables;r.domains=r.raw_domains;
  std::array<std::vector<std::size_t>,4> indices;
  for(std::size_t i=0;i<r.variables.size();++i)indices.at(static_cast<std::size_t>(r.variables[i].reference.type)).push_back(i);
  const auto index=[&](C::Reference ref){return indices.at(static_cast<std::size_t>(ref.type)).at(ref.index);};
  std::vector<std::size_t> parent(r.variables.size());
  for(std::size_t i=0;i<parent.size();++i)parent[i]=r.variables[i].alias?index(r.variables[i].target):i;
  const auto base=[&](C::Reference ref){
    std::size_t i=index(ref),root=i,steps=0;
    while(parent[root]!=root){
      if(++steps>parent.size())bad("cyclic variable alias");
      root=parent[root];
    }
    while(parent[i]!=i){const auto next=parent[i];parent[i]=root;i=next;}
    return root;
  };
  for(std::size_t n=0;n<r.raw_constraints.size();++n){const auto& c=r.raw_constraints[n];
    if((c.id=="int_eq"||c.id=="bool_eq")&&c.arguments.size()==2&&c.arguments[0].kind==C::ValueKind::Reference&&c.arguments[1].kind==C::ValueKind::Reference){
      auto i=base(c.arguments[0].reference),j=base(c.arguments[1].reference);if(i>j)std::swap(i,j);
      if(i!=j){auto& old=r.variables[j];
        if(old.assigned||old.domain.present){C::Value variable;variable.kind=C::ValueKind::Reference;variable.reference=r.variables[i].reference;
          C::Value domain;domain.kind=C::ValueKind::Set;
          if(old.assigned){domain.set.interval=true;domain.set.lower=domain.set.upper=old.reference.type==C::Type::Boolean?old.value.boolean:old.value.integer;}
          else domain.set=old.domain.integers;
          r.domains.push_back({"int_in",{variable,domain},{},c.location,true});
        }
        parent[j]=i;old.alias=true;old.target=r.variables[i].reference;old.assigned=false;old.domain={};old.value={};
      }
      r.coverage.push_back({C::CoverageKind::AliasEquality,n});
    }else {r.constraints.push_back(c);r.coverage.push_back({C::CoverageKind::Retained,n});}
  }
  for(auto& v:r.variables)if(v.alias)v.target=r.variables[base(v.reference)].reference;
}
}
void CaptureParser::fail(ParserState& p,C::Status value,const std::string& message){
  if(status==C::Status::Complete){status=value;diagnostics.push_back({value,location(p),message});}
  p.hadError=true;
}
bool CaptureParser::array_size(ParserState& p,int n){
  if(n<0||static_cast<std::size_t>(n)>options.max_array_elements ||
     p.arrays.size()+p.floatvals.size()+p.setvals.size()+static_cast<std::size_t>(std::max(0,n))+1>static_cast<std::size_t>(std::numeric_limits<int>::max()))fail(p,C::Status::ResourceLimit,"declared array length exceeds capture limit");return !p.hadError;
}
bool CaptureParser::variable_count(ParserState& p,int n,bool initialized_array){
  const auto current=p.intvars.size()+p.boolvars.size()+p.setvars.size()+p.floatvars.size();
  // Initialized arrays may reuse existing slots; the exact total is checked
  // after declarations. The input limit already bounds their explicit list.
  if(n<0 || current>options.max_variables || (!initialized_array &&
      static_cast<std::size_t>(n)>options.max_variables-current))
    fail(p,C::Status::ResourceLimit,"variable count exceeds capture limit");
  return !p.hadError;
}
bool CaptureParser::annotations(ParserState& p,AST::Array* ann){
  guard(*this,p,[&]{for(const auto& v:values(p,ann))if(v.kind==C::ValueKind::Call&&v.text=="output_array"){
    if(v.elements.size()!=1 || v.elements[0].kind!=C::ValueKind::Array)bad("output_array requires one array of dimensions");
    for(const auto& dim:v.elements[0].elements)if(dim.kind!=C::ValueKind::Set)bad("output_array dimensions must be sets");
  }});return !p.hadError;
}
void CaptureParser::declaration(ParserState& p,const std::string& name,AST::Array* ann,int output_length){
  if(!annotations(p,ann))return;
  guard(*this,p,[&]{
    auto captured=values(p,ann);
    for(const auto& a:captured)if(a.kind==C::ValueKind::Call&&a.text=="output_array"){
      if(output_length<0)bad("output_array requires an array declaration");
      const auto& dims=a.elements[0].elements;if(dims.empty())bad("output_array requires at least one dimension");
      std::size_t total=1;
      for(const auto& d:dims){
        const auto width=d.set.interval ? (d.set.upper<d.set.lower?0:static_cast<std::size_t>(d.set.upper-d.set.lower+1)) : d.set.values.size();
        if(width && total>options.max_array_elements/width)limit("output dimensions exceed capture limit");
        total*=width;
      }
      if(total!=static_cast<std::size_t>(output_length))bad("output_array dimensions do not match declaration length");
    }
    records.declaration_annotations.push_back({name,std::move(captured),location(p,records.declaration_annotations.size())});
  });
}
void CaptureParser::begin(ParserState& p){guard(*this,p,[&]{
  records.source=options.source;records.raw_variables=variables(p);namespace_counts=counts(records.raw_variables);
  if(p.domainConstraints.size()>options.max_constraints)limit("declaration domain count exceeds capture limit");
  for(auto* c:p.domainConstraints)records.raw_domains.push_back(::Gecode::FlatZinc::constraint(p,*c,true));
  declarations_recorded=true;
});}
void CaptureParser::constraint(ParserState& p,const std::string& id,AST::Array* args,AST::Array* ann){guard(*this,p,[&]{
  if(records.raw_constraints.size()>=options.max_constraints-records.raw_domains.size())limit("constraint count exceeds capture limit");
  if(id.compare(0,18,"gecode_on_restart_")==0)throw CaptureFailure{C::Status::Unsupported,"native restart-state predicates are unsupported by capture"};
  C::Constraint out{id,values(p,args),values(p,ann),location(p,records.raw_constraints.size()),false};
  validate_signature(out);for(const auto& v:out.arguments)check_ref(v,namespace_counts);
  records.raw_constraints.push_back(std::move(out));
});}
void CaptureParser::objective(ParserState& p,AST::Node* n){guard(*this,p,[&]{records.solve.objective=value(p,n);records.solve.has_objective=true;});}
void CaptureParser::solve(ParserState& p,C::Method method,AST::Array* ann){guard(*this,p,[&]{
  records.solve.method=method;records.solve.annotations=values(p,ann);records.solve.location=location(p);
  if(method==C::Method::Satisfy){records.solve.has_objective=false;records.solve.objective={};}
  else {if(!records.solve.has_objective)bad("missing original objective");check_ref(records.solve.objective,namespace_counts);}
  solve_recorded=true;
});}
void CaptureParser::finish(ParserState& p){guard(*this,p,[&]{
  if(!declarations_recorded||!solve_recorded)bad("incomplete captured model");
  normalize(records);
  for(const auto& out:p._output){auto expression=value(p,out.second);check_ref(expression,namespace_counts);records.output.push_back({out.first,std::move(expression)});}
  std::sort(records.output.begin(),records.output.end(),[](const C::Output& a,const C::Output& b){return a.name<b.name;});
});}
void CaptureParser::cleanup(ParserState& p) noexcept {
  for(auto& v:p.intvars)delete v.second;for(auto& v:p.boolvars)delete v.second;
  for(auto& v:p.setvars)delete v.second;for(auto& v:p.floatvars)delete v.second;
  for(auto* c:p.domainConstraints)delete c;for(auto* c:p.constraints)delete c;
  for(auto& out:p._output)delete out.second;
  p.intvars.clear();p.boolvars.clear();p.setvars.clear();p.floatvars.clear();p.domainConstraints.clear();p.constraints.clear();p._output.clear();
}
namespace Capture {
namespace {
Result failure(Status s,const Options& o,const std::string& text){return {s,nullptr,{{s,{o.source,0,0},text}}};}
}
Result parse_string(const std::string& input,const Options& options){
  if(options.max_input_bytes>static_cast<std::size_t>(std::numeric_limits<int>::max())||
     options.max_variables>static_cast<std::size_t>(std::numeric_limits<int>::max())||
     options.max_array_elements>static_cast<std::size_t>(std::numeric_limits<int>::max())||
     options.max_value_depth>1024)return failure(Status::InvalidInput,options,"capture limits exceed parser address/index range");
  if(input.size()>options.max_input_bytes)return failure(Status::ResourceLimit,options,"input exceeds capture byte limit");
  if(input.find('\0')!=std::string::npos)return failure(Status::InvalidInput,options,"embedded NUL in FlatZinc input");
  for(std::size_t i=0;i<input.size();){
    const auto lead=static_cast<unsigned char>(input[i++]);if(lead<128)continue;
    unsigned n=lead>=0xc2&&lead<=0xdf?1:lead>=0xe0&&lead<=0xef?2:lead>=0xf0&&lead<=0xf4?3:0;
    if(!n || n>input.size()-i)return failure(Status::InvalidInput,options,"input is not valid UTF-8");
    std::uint32_t code=lead&((1u<<(6-n))-1u);
    for(unsigned k=0;k<n;++k){const auto c=static_cast<unsigned char>(input[i++]);if((c&0xc0)!=0x80)return failure(Status::InvalidInput,options,"input is not valid UTF-8");code=(code<<6)|(c&0x3f);}
    if((n==1&&code<0x80)||(n==2&&code<0x800)||(n==3&&code<0x10000)||code>0x10ffff||(code>=0xd800&&code<=0xdfff))return failure(Status::InvalidInput,options,"input is not valid UTF-8");
  }

  std::size_t depth=0;bool comment=false,quoted=false,escaped=false;
  for(char c:input){
    if(comment){if(c=='\n')comment=false;continue;}
    if(quoted){
      if(c=='\n' || c=='\r')return failure(Status::InvalidInput,options,"newline in string literal");
      if(escaped){
        escaped=false;continue;
      }
      if(c=='\\'){escaped=true;continue;}
      if(c=='"')quoted=false;
      continue;
    }
    if(c=='%'){comment=true;continue;}
    if(c=='"'){quoted=true;continue;}
    if(c=='('||c=='['||c=='{'){
      if(++depth>options.max_value_depth)return failure(Status::ResourceLimit,options,"source nesting exceeds capture limit");
    }else if((c==')'||c==']'||c=='}')&&depth) --depth;
  }
  if(quoted)return failure(Status::InvalidInput,options,"unterminated string literal");
  std::ostringstream errors;CaptureParser capture(options);ParserState p(input,errors,nullptr);p.capture=&capture;
  struct Cleanup{ParserState& p;~Cleanup(){if(p.yyscanner)yylex_destroy(p.yyscanner);CaptureParser::cleanup(p);}} cleanup{p};
  if(yylex_init(&p.yyscanner))throw std::bad_alloc();yyset_extra(&p,p.yyscanner);
  try{
    const int code=yyparse(&p);
    if(code==2)capture.fail(p,Status::ResourceLimit,"generated parser exhausted its storage");
    else if(code||p.hadError){if(capture.status==Status::Complete)capture.fail(p,Status::InvalidInput,errors.str().empty()?"invalid FlatZinc input":errors.str());}
    else capture.finish(p);
  }catch(const AST::TypeError& e){capture.fail(p,Status::InvalidInput,e.what());}
  catch(const std::out_of_range&){capture.fail(p,Status::InvalidInput,"invalid parser reference");}
  if(capture.status!=Status::Complete)return {capture.status,nullptr,std::move(capture.diagnostics)};
  return {Status::Complete,std::make_shared<const Records>(std::move(capture.records)),{}};
}
Result parse(std::istream& input,const Options& options){
  if(options.max_input_bytes>static_cast<std::size_t>(std::numeric_limits<int>::max()))return failure(Status::InvalidInput,options,"capture input limit exceeds parser index range");
  std::string text;char block[4096];
  while(input){
    const auto room=options.max_input_bytes-text.size();
    const auto n=static_cast<std::streamsize>(std::min<std::size_t>(sizeof(block),room+1));
    bool failed=false;
    try { input.read(block,n); } catch(const std::bad_alloc&) { throw; } catch(...) { failed=true; }
    const auto got=input.gcount();
    if(got<0 || static_cast<std::size_t>(got)>room)return failure(Status::ResourceLimit,options,"input exceeds capture byte limit");
    text.append(block,static_cast<std::size_t>(got));
    if(input.bad() || (failed&&!input.eof()))return failure(Status::InvalidInput,options,"input read failed");
  }
  if(input.bad() || (!input.eof()&&input.fail()))return failure(Status::InvalidInput,options,"input read failed");
  return parse_string(text,options);
}
}
}}
