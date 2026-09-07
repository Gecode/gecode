/* Lossless, deliberately bounded LP/free-MPS I/O. No backend parser is used. */
#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <utility>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Gecode { namespace Optimize {
namespace {
constexpr double inf = std::numeric_limits<double>::infinity();
using Terms = std::vector<std::pair<std::size_t, double>>;

[[noreturn]] void fail(const std::string& message) {
  throw ModelError("LP/MPS I/O: " + message);
}
std::string lower(std::string text) {
  for (char& c : text) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  return text;
}
std::string trim(const std::string& s) {
  const auto first = s.find_first_not_of(" \t\r\n");
  return first == std::string::npos ? "" : s.substr(first, s.find_last_not_of(" \t\r\n")-first+1);
}
bool identifier(const std::string& s) {
  const auto alpha = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; };
  if (s.empty() || !alpha(s.front())) return false;
  return std::all_of(s.begin()+1, s.end(), [&](char c) { return alpha(c) || (c >= '0' && c <= '9'); });
}
std::string format(const std::string& filename) {
  const auto extension = lower(std::filesystem::path(filename).extension().string());
  if (extension != ".lp" && extension != ".mps") fail("only uncompressed .lp and .mps are supported");
  return extension;
}
std::vector<std::string> words(const std::string& line) {
  std::istringstream in(line); in.imbue(std::locale::classic());
  std::vector<std::string> result;
  for (std::string word; in >> word;) result.push_back(word);
  return result;
}
long double number(std::string text, bool infinity = false) {
  const auto lc = lower(text);
  if (infinity && (lc == "inf" || lc == "+inf" || lc == "infinity" || lc == "+infinity")) return inf;
  if (infinity && (lc == "-inf" || lc == "-infinity")) return -inf;
  if (text.empty()) fail("missing number");
  for (auto& c : text) if (c == 'd' || c == 'D') c = 'e';
  std::istringstream in(text); in.imbue(std::locale::classic());
  long double value = 0;
  in >> value;
  if (!in || !in.eof() || !std::isfinite(value)) fail("invalid finite number: " + text);
  return value;
}
double narrow(long double value) {
  if (!std::isfinite(value) || std::fabs(value) > std::numeric_limits<double>::max())
    fail("numeric value or arithmetic result exceeds double range");
  const double result = static_cast<double>(value);
  if (value != 0 && result == 0) fail("nonzero value underflows double range");
  return result;
}
double scalar(const std::string& value, bool infinity = false) {
  const auto n = number(value, infinity);
  return std::isinf(n) ? static_cast<double>(n) : narrow(n);
}
std::string hex(const std::string& s) {
  static constexpr char digits[] = "0123456789abcdef";
  if (s.empty()) return "-";
  std::string result;
  for (unsigned char c : s) { result += digits[c >> 4]; result += digits[c & 15]; }
  return result;
}
std::string unhex(const std::string& s) {
  if (s == "-") return "";
  if (s.empty() || s.size()%2) fail("malformed name metadata");
  const auto digit = [](char c) -> int {
    if (c >= '0' && c <= '9') return c-'0';
    if (c >= 'a' && c <= 'f') return c-'a'+10;
    fail("malformed name metadata");
  };
  std::string result;
  for (std::size_t i=0; i<s.size(); i+=2) result += static_cast<char>(16*digit(s[i])+digit(s[i+1]));
  return result;
}

struct DraftVar {
  std::string name;
  VariableType type = VariableType::Continuous;
  double lb = 0, ub = inf;
  bool lower_set = false, upper_set = false, marker = false, any_bound = false;
};
struct DraftRow { std::string name; Terms terms; double lb = -inf, ub = inf; };
Terms canonical(const Terms& terms) {
  std::map<std::size_t,std::pair<long double,long double>> sums;
  for(const auto& term:terms) {
    auto& pair=sums[term.first];const long double next=pair.first+term.second;
    pair.second+=std::fabs(pair.first)>=std::fabs(term.second)?(pair.first-next)+term.second:(term.second-next)+pair.first;
    pair.first=next;if(!std::isfinite(next)||!std::isfinite(pair.second))fail("coefficient sum overflow");
  }
  Terms result;for(const auto& sum:sums){const double value=narrow(sum.second.first+sum.second.second);if(value!=0)result.push_back({sum.first,value});}
  return result;
}
struct Draft {
  std::vector<DraftVar> vars;
  std::map<std::string, std::size_t> indices;
  std::vector<DraftRow> rows;
  Terms objective;
  double offset = 0;
  ObjectiveSense sense = ObjectiveSense::Minimize;
  std::map<std::pair<std::string,std::string>,std::string> names;
  std::map<std::string,std::string> ranges;
  std::size_t variable(const std::string& name) {
    if (!identifier(name)) fail("unsupported identifier: " + name);
    const auto found = indices.find(name);
    if (found != indices.end()) return found->second;
    const auto id = vars.size(); vars.push_back({name}); indices.emplace(name,id); return id;
  }
  void metadata(const std::string& text) {
    const auto w=words(text);
    if (w.empty() || w[0].rfind("GECODE_",0)!=0) return;
    if (w[0]=="GECODE_NAME" && w.size()==4 && (w[1]=="V" || w[1]=="R") && identifier(w[2])) {
      if (!names.emplace(std::make_pair(w[1],w[2]),unhex(w[3])).second) fail("duplicate name metadata");
    } else if (w[0]=="GECODE_RANGE" && w.size()==3 && identifier(w[1]) && identifier(w[2])) {
      if (!ranges.emplace(w[1],w[2]).second) fail("duplicate range metadata");
    } else fail("unsupported or malformed Gecode metadata");
  }
  Model build() {
    // LP ranged rows are serialized as two portable inequalities. Only a
    // matching pair with exactly equal canonical expressions is recombined.
    Model model;
    std::vector<Variable> handles;
    for (auto& var : vars) {
      const auto named=names.find({"V",var.name});
      handles.push_back(model.add_variable(var.type,var.lb,var.ub,named==names.end()?var.name:named->second));
      if (named!=names.end()) names.erase(named);
    }
    const auto terms = [&](const Terms& input) {
      std::vector<Term> output;
      for (const auto& t : input) output.push_back({handles.at(t.first),t.second});
      return output;
    };
    std::map<std::string,std::size_t> row_names;
    for (std::size_t i=0;i<rows.size();++i)if(!row_names.emplace(rows[i].name,i).second)fail("duplicate row name: "+rows[i].name);
    std::set<std::string> omitted;
    for (const auto& range : ranges) {
      const auto li=row_names.find(range.first), ui=row_names.find(range.second);
      if(li==row_names.end()||ui==row_names.end())fail("range metadata references absent row");
      auto lo=rows.begin()+static_cast<std::ptrdiff_t>(li->second),hi=rows.begin()+static_cast<std::ptrdiff_t>(ui->second);
      if (lo==hi || !std::isfinite(lo->lb) || lo->ub!=inf ||
          hi->lb!=-inf || !std::isfinite(hi->ub) || omitted.count(lo->name) ||
          !omitted.insert(hi->name).second) fail("invalid ranged-row metadata");
      if(canonical(lo->terms)!=canonical(hi->terms))fail("range metadata expressions differ");
      lo->ub=hi->ub;
    }
    for (const auto& row : rows) {
      if (omitted.count(row.name)) continue;
      const auto named=names.find({"R",row.name});
      model.add_row(terms(row.terms),row.lb,row.ub,named==names.end()?row.name:named->second);
      if (named!=names.end()) names.erase(named);
    }
    if (!names.empty()) fail("name metadata references absent entity");
    model.set_objective(terms(objective),sense,offset);
    validate_structure(model.snapshot());
    return model;
  }
};

std::vector<std::string> lex(const std::string& text) {
  std::vector<std::string> result;
  for (std::size_t i=0;i<text.size();) {
    const char c=text[i];
    if(c==' '||c=='\t'||c=='\r'||c=='\n'){++i;continue;}
    if(c=='+'||c=='-'||c==':'||c=='='){result.push_back(text.substr(i++,1));continue;}
    if(c=='<'||c=='>') {
      if(i+1>=text.size()||text[i+1]!='=') fail("strict or incomplete LP comparison");
      result.push_back(text.substr(i,2));i+=2;continue;
    }
    const auto begin=i;
    if((c>='0'&&c<='9')||c=='.') {
      while(i<text.size()&&((text[i]>='0'&&text[i]<='9')||text[i]=='.')) ++i;
      if(i<text.size()&&(text[i]=='e'||text[i]=='E')) {
        ++i; if(i<text.size()&&(text[i]=='+'||text[i]=='-'))++i;
        const auto exponent=i;
        while(i<text.size()&&text[i]>='0'&&text[i]<='9')++i;
        if(i==exponent)fail("invalid LP exponent");
      }
      if(i<text.size()&&((text[i]>='a'&&text[i]<='z')||(text[i]>='A'&&text[i]<='Z')||text[i]=='_'))
        fail("LP numeric coefficients need whitespace before variable names");
      auto value=text.substr(begin,i-begin); (void)number(value); result.push_back(value); continue;
    }
    if((c>='A'&&c<='Z')||(c>='a'&&c<='z')||c=='_') {
      ++i;while(i<text.size()&&((text[i]>='A'&&text[i]<='Z')||(text[i]>='a'&&text[i]<='z')||
          (text[i]>='0'&&text[i]<='9')||text[i]=='_'))++i;
      result.push_back(text.substr(begin,i-begin));continue;
    }
    fail("unsupported LP syntax (nonlinear/SOS/quoted names are not supported)");
  }
  return result;
}
double signed_number(const std::vector<std::string>& tokens,std::size_t& p,bool infinity=false) {
  std::string sign;
  if(p<tokens.size()&&(tokens[p]=="+"||tokens[p]=="-"))sign=tokens[p++];
  if(p==tokens.size())fail("missing LP number");
  return scalar(sign+tokens[p++],infinity);
}
struct Offset {
  long double sum=0,correction=0;
  void add(long double value) {
    const long double next=sum+value;
    correction+=std::fabs(sum)>=std::fabs(value)?(sum-next)+value:(value-next)+sum;
    sum=next;
    if(!std::isfinite(sum)||!std::isfinite(correction))fail("LP expression offset overflow");
  }
  double value() const {return narrow(sum+correction);}
};
std::pair<Terms,Offset> expression(Draft& draft,const std::vector<std::string>& tokens,std::size_t first,std::size_t last) {
  Terms terms; Offset offset;
  bool initial=true;
  while(first<last) {
    double sign=1;
    if(tokens[first]=="+"||tokens[first]=="-"){if(tokens[first]=="-")sign=-1;++first;}
    else if(!initial)fail("LP terms need explicit + or - separators");
    if(first==last)fail("incomplete LP expression");
    if(identifier(tokens[first]) && lower(tokens[first])!="inf" && lower(tokens[first])!="infinity")
      terms.push_back({draft.variable(tokens[first++]),sign});
    else {
      const double coefficient=narrow(sign*number(tokens[first++]));
      if(first<last&&identifier(tokens[first]))terms.push_back({draft.variable(tokens[first++]),coefficient});
      else offset.add(coefficient);
    }
    initial=false;
  }
  if(initial)fail("empty LP expression");
  return {terms,offset};
}

Model read_lp(std::istream& input) {
  Draft draft; enum class Section{None,Objective,Rows,Bounds,Binary,General,Semi,End};
  Section section=Section::None; bool objective_seen=false,objective_parsed=false,rows_seen=false,end_seen=false;
  std::string objective,pending;
  const auto parse_objective=[&](){auto t=lex(objective);const std::size_t start=t.size()>1&&t[1]==":"?2:0;
    auto e=expression(draft,t,start,t.size());draft.objective=std::move(e.first);draft.offset=e.second.value();objective_parsed=true;};
  const auto parse_row=[&](const std::string& text) {
    const auto t=lex(text);std::size_t first=0;
    std::string name="row_"+std::to_string(draft.rows.size());
    if(t.size()>1&&t[1]==":"){name=t[0];first=2;}
    auto relation=std::find_if(t.begin()+static_cast<std::ptrdiff_t>(first),t.end(),[](const auto& s){return s=="<="||s==">="||s=="=";});
    if(relation==t.end())fail("LP row missing comparison");
    const auto at=static_cast<std::size_t>(relation-t.begin());
    auto e=expression(draft,t,first,at);std::size_t pos=at+1;const double rhs=signed_number(t,pos,true);
    if(pos!=t.size())fail("LP rows require one comparison and numeric RHS");
    // Keep the compensation through RHS subtraction: narrowing the LHS
    // constant first loses the residual in x + 1 - 1e16 >= -1e16.
    if(std::isfinite(rhs))e.second.add(-static_cast<long double>(rhs));
    const double side=std::isinf(rhs)?rhs:-e.second.value();
    DraftRow row{name,std::move(e.first),-inf,inf};
    if(*relation=="<="||*relation=="=")row.ub=side;
    if(*relation==">="||*relation=="=")row.lb=side;
    draft.rows.push_back(std::move(row));
  };
  for(std::string line;std::getline(input,line);) {
    line=trim(line);if(line.empty())continue;
    if(line.front()=='\\'){draft.metadata(trim(line.substr(1)));continue;}
    if(end_seen)fail("content follows LP End");
    if(line.find('\\')!=std::string::npos)line=trim(line.substr(0,line.find('\\')));
    const auto keyword=lower(line);Section next=Section::None;
    if(keyword=="minimize"||keyword=="minimum"||keyword=="min"||keyword=="maximize"||keyword=="maximum"||keyword=="max") {
      if(objective_seen)fail("multiple LP objectives"); objective_seen=true;
      draft.sense=keyword.rfind("max",0)==0?ObjectiveSense::Maximize:ObjectiveSense::Minimize;next=Section::Objective;
    } else if(keyword=="subject to"||keyword=="such that"||keyword=="st"||keyword=="s.t."||keyword=="s.t") {
      if(rows_seen)fail("duplicate LP constraint section");rows_seen=true;next=Section::Rows;
    } else if(keyword=="bounds"||keyword=="bound")next=Section::Bounds;
    else if(keyword=="binary"||keyword=="binaries"||keyword=="bin")next=Section::Binary;
    else if(keyword=="general"||keyword=="generals"||keyword=="gen"||keyword=="integer"||keyword=="integers")next=Section::General;
    else if(keyword=="semi"||keyword=="semis"||keyword=="semi-continuous")next=Section::Semi;
    else if(keyword=="end"){next=Section::End;end_seen=true;}
    else if(keyword=="sos"||keyword=="pwl"||keyword=="indicators")fail("unsupported LP section: "+line);
    if(next!=Section::None){if(!pending.empty())fail("incomplete LP row");
      if(!objective_seen)fail("LP objective must be first");
      if(section==Section::Objective&&!objective_parsed)parse_objective();section=next;continue;}
    if(section==Section::Objective){objective+=" "+line;continue;}
    if(section==Section::Rows) {
      pending+=" "+line;
      const auto t=lex(pending);auto rel=std::find_if(t.begin(),t.end(),[](const auto& s){return s=="<="||s==">="||s=="=";});
      if(rel!=t.end()&&rel+1!=t.end()&&t.back()!="+"&&t.back()!="-"){parse_row(pending);pending.clear();}
      continue;
    }
    const auto t=lex(line);
    if(section==Section::Bounds) {
      if(t.size()==2&&identifier(t[0])&&lower(t[1])=="free") {
        auto& v=draft.vars[draft.variable(t[0])];if(v.lower_set||v.upper_set)fail("duplicate LP bound side");
        v.lb=-inf;v.ub=inf;v.lower_set=v.upper_set=true;continue;
      }
      std::size_t p=0;std::string name;double lb=-inf,ub=inf;bool set_lb=false,set_ub=false;
      if(!t.empty()&&identifier(t[0])&&lower(t[0])!="inf"&&lower(t[0])!="infinity") {
        name=t[p++];if(p>=t.size())fail("incomplete LP bound");const auto op=t[p++];
        const double n=signed_number(t,p,true);
        if(op=="<="||op=="="){ub=n;set_ub=true;}if(op==">="||op=="="){lb=n;set_lb=true;}
        if(!set_lb&&!set_ub)fail("invalid LP bound comparison");
      } else {
        lb=signed_number(t,p,true);if(p>=t.size()||t[p++]!="<=")fail("unsupported LP bound direction");
        if(p>=t.size())fail("missing LP bound variable");name=t[p++];set_lb=true;
        if(p<t.size()){if(t[p++]!="<=")fail("unsupported LP range direction");ub=signed_number(t,p,true);set_ub=true;}
      }
      if(p!=t.size())fail("trailing LP bound syntax");auto& v=draft.vars[draft.variable(name)];
      if((set_lb&&v.lower_set)||(set_ub&&v.upper_set))fail("duplicate LP bound side");
      if(set_lb){v.lb=lb;v.lower_set=true;}if(set_ub){v.ub=ub;v.upper_set=true;}continue;
    }
    if(section==Section::Binary||section==Section::General||section==Section::Semi) {
      for(const auto& name:t) {
        auto& v=draft.vars[draft.variable(name)];
        if(section==Section::Binary){if(v.type!=VariableType::Continuous)fail("conflicting LP variable declarations");v.type=VariableType::Binary;if(!v.upper_set)v.ub=1;}
        else if(section==Section::General){if(v.type==VariableType::Binary||v.type==VariableType::Integer||v.type==VariableType::SemiInteger)fail("duplicate/conflicting LP type");v.type=v.type==VariableType::SemiContinuous?VariableType::SemiInteger:VariableType::Integer;}
        else {if(v.type!=VariableType::Continuous&&v.type!=VariableType::Integer)fail("duplicate/conflicting LP semi type");v.type=v.type==VariableType::Integer?VariableType::SemiInteger:VariableType::SemiContinuous;if(!v.lower_set)v.lb=1;}
      }continue;
    }
    fail("missing or unsupported LP section");
  }
  if(!objective_seen||!end_seen||!pending.empty())fail("LP needs one objective and End");
  if(!objective_parsed)parse_objective();
  return draft.build();
}

Model read_mps(std::istream& input) {
  Draft d;enum class Section{None,Name,Sense,Rows,Columns,Rhs,Ranges,Bounds,End};Section sec=Section::None;
  std::map<std::string,std::size_t> rows;std::vector<char> senses;std::map<std::size_t,long double> ranges;
  std::set<std::size_t> rhs_seen;std::set<std::pair<std::size_t,std::string>> bound_seen;
  std::string objective,rhs_name,range_name,bound_name;bool integer=false,ended=false,columns=false;
  std::set<Section> sections;
  for(std::string line;std::getline(input,line);) {
    line=trim(line);if(line.empty())continue;if(line.front()=='*'){d.metadata(trim(line.substr(1)));continue;}
    if(ended)fail("content follows MPS ENDATA");const auto w=words(line);const auto key=lower(w[0]);Section next=Section::None;
    if(key=="name"&&w.size()<=2)next=Section::Name;
    else if(key=="objsense"&&w.size()<=2)next=Section::Sense;
    else if(w.size()==1) {
      if(key=="rows")next=Section::Rows;else if(key=="columns")next=Section::Columns;
      else if(key=="rhs")next=Section::Rhs;else if(key=="ranges")next=Section::Ranges;
      else if(key=="bounds")next=Section::Bounds;else if(key=="endata")next=Section::End;
      else if(sec!=Section::Sense)fail("unsupported MPS section: "+w[0]);
    }
    if(next!=Section::None) {
      if(!sections.insert(next).second)fail("duplicate MPS section");
      if(next==Section::Columns){if(objective.empty())fail("MPS requires objective N row before COLUMNS");columns=true;}
      if(next==Section::Rows&&columns)fail("MPS ROWS out of order");
      if((next==Section::Rhs||next==Section::Ranges||next==Section::Bounds)&&!columns)fail("MPS sections out of order");
      if(sec==Section::Columns&&integer)fail("unclosed MPS integer marker");sec=next;
      if(next==Section::End){ended=true;continue;}
      if(next!=Section::Sense||w.size()==1)continue;
    }
    if(sec==Section::Sense) {
      const auto value=lower(w.back());if(value!="min"&&value!="max")fail("invalid MPS objective sense");
      d.sense=value=="min"?ObjectiveSense::Minimize:ObjectiveSense::Maximize;sec=Section::Name;continue;
    }
    if(sec==Section::Rows) {
      if(w.size()!=2||w[0].size()!=1||!identifier(w[1]))fail("invalid MPS ROWS record");
      const char kind=static_cast<char>(std::toupper(static_cast<unsigned char>(w[0][0])));
      if(kind!='N'&&kind!='L'&&kind!='G'&&kind!='E')fail("unsupported MPS row type");
      if(rows.count(w[1])||w[1]==objective)fail("duplicate MPS row name");
      if(kind=='N'&&objective.empty()){objective=w[1];continue;}
      rows.emplace(w[1],d.rows.size());d.rows.push_back({w[1],{},kind=='G'||kind=='E'?0:-inf,kind=='L'||kind=='E'?0:inf});senses.push_back(kind);continue;
    }
    if(sec==Section::Columns) {
      if(w.size()==3&&w[1]=="'MARKER'") {
        if(w[2]=="'INTORG'"&&!integer)integer=true;else if(w[2]=="'INTEND'"&&integer)integer=false;else fail("invalid integer marker");continue;
      }
      if(w.size()!=3&&w.size()!=5)fail("invalid MPS COLUMNS record");
      const bool existed=d.indices.count(w[0])!=0;const auto id=d.variable(w[0]);auto& v=d.vars[id];
      if(existed&&v.marker!=integer)fail("inconsistent column integrality markers");
      if(integer){v.marker=true;v.type=VariableType::Integer;}
      for(std::size_t p=1;p<w.size();p+=2) {
        const double value=scalar(w[p+1]);
        if(w[p]==objective)d.objective.push_back({id,value});
        else {const auto row=rows.find(w[p]);if(row==rows.end())fail("COLUMNS references unknown row");d.rows[row->second].terms.push_back({id,value});}
      }continue;
    }
    if(sec==Section::Rhs||sec==Section::Ranges) {
      if(w.size()!=3&&w.size()!=5)fail("invalid RHS/RANGES record");
      auto& vector_name=sec==Section::Rhs?rhs_name:range_name;
      if(vector_name.empty())vector_name=w[0];if(vector_name!=w[0])fail("multiple RHS/RANGES vectors are unsupported");
      for(std::size_t p=1;p<w.size();p+=2) {
        if(w[p]==objective){if(sec!=Section::Rhs||!rhs_seen.insert(std::numeric_limits<std::size_t>::max()).second)fail("duplicate/invalid objective RHS");d.offset=-scalar(w[p+1]);continue;}
        const auto found=rows.find(w[p]);if(found==rows.end())fail("RHS/RANGES references unknown row");const auto id=found->second;
        if(sec==Section::Ranges){if(senses[id]=='N'||!ranges.emplace(id,number(w[p+1])).second)fail("duplicate/invalid row range");}
        else {if(!rhs_seen.insert(id).second)fail("duplicate RHS row");const double value=scalar(w[p+1]);
          if(senses[id]=='N')fail("RHS for nonobjective free row unsupported");
          if(senses[id]=='L'||senses[id]=='E')d.rows[id].ub=value;if(senses[id]=='G'||senses[id]=='E')d.rows[id].lb=value;}
      }continue;
    }
    if(sec==Section::Bounds) {
      if(w.size()!=3&&w.size()!=4)fail("invalid MPS BOUNDS record");
      if(bound_name.empty())bound_name=w[1];if(bound_name!=w[1])fail("multiple bound vectors unsupported");
      const auto found=d.indices.find(w[2]);if(found==d.indices.end())fail("BOUNDS references unknown column");
      const auto id=found->second;auto& v=d.vars[id];const auto code=lower(w[0]);
      if(!bound_seen.insert({id,code}).second)fail("duplicate MPS bound record");v.any_bound=true;
      const bool valued=code=="lo"||code=="up"||code=="fx"||code=="li"||code=="ui"||code=="sc"||code=="si";
      if((valued&&w.size()!=4)||(!valued&&w.size()!=3))fail("wrong MPS bound arity");
      const double value=valued?scalar(w[3]):0;
      const bool writes_lower=code=="lo"||code=="li"||code=="fx"||code=="fr"||code=="mi";
      const bool writes_upper=code=="up"||code=="ui"||code=="fx"||code=="fr"||code=="pl"||code=="sc"||code=="si";
      if((writes_lower&&v.lower_set)||(writes_upper&&v.upper_set)||
          (code=="bv"&&(v.lower_set||v.upper_set)))fail("overlapping MPS bound definitions");
      if((code=="li"||code=="ui")&&(v.type==VariableType::Binary||v.type==VariableType::SemiContinuous||v.type==VariableType::SemiInteger))
        fail("conflicting MPS variable type definitions");
      if((code=="sc"&&v.type!=VariableType::Continuous)||
          (code=="si"&&v.type!=VariableType::Continuous&&v.type!=VariableType::Integer)||
          (code=="bv"&&(v.type==VariableType::SemiContinuous||v.type==VariableType::SemiInteger)))
        fail("conflicting MPS variable type definitions (use SI for semi-integer columns)");
      if(code=="lo"||code=="li"){v.lb=value;v.lower_set=true;if(code=="li")v.type=VariableType::Integer;}
      else if(code=="up"||code=="ui"){v.ub=value;v.upper_set=true;if(code=="ui")v.type=VariableType::Integer;}
      else if(code=="fx"){v.lb=v.ub=value;v.lower_set=v.upper_set=true;}
      else if(code=="fr"){v.lb=-inf;v.ub=inf;v.lower_set=v.upper_set=true;}
      else if(code=="mi"){v.lb=-inf;v.lower_set=true;}
      else if(code=="pl"){v.ub=inf;v.upper_set=true;}
      else if(code=="bv"){v.type=VariableType::Binary;v.lb=0;v.ub=1;v.lower_set=v.upper_set=true;}
      else if(code=="sc"||code=="si"){v.type=code=="si"?VariableType::SemiInteger:VariableType::SemiContinuous;v.ub=value;v.upper_set=true;if(!v.lower_set)v.lb=1;}
      else fail("unsupported MPS bound type: "+w[0]);continue;
    }
    fail("unexpected MPS record or unsupported section");
  }
  if(!ended||!columns||objective.empty())fail("MPS requires ROWS, COLUMNS and ENDATA");
  for(auto& v:d.vars)if(v.marker&&!v.any_bound)v.ub=1;
  for(const auto& range:ranges) {
    const auto id=range.first;const auto value=range.second;auto& row=d.rows[id];
    if(senses[id]=='L'||(senses[id]=='E'&&value<0))row.lb=narrow(static_cast<long double>(row.ub)-std::fabs(value));
    else row.ub=narrow(static_cast<long double>(row.lb)+std::fabs(value));
  }
  return d.build();
}

std::string real(double value) {
  if(std::isinf(value))return value<0?"-inf":"+inf";
  std::ostringstream out;out.imbue(std::locale::classic());out<<std::setprecision(std::numeric_limits<double>::max_digits10)<<value;return out.str();
}
ModelSnapshot compact(const ModelSnapshot& original) {
  validate_structure(original);Model m;std::vector<Variable> map(original.variables.size());
  for(const auto& v:original.variables)if(v.active)map[v.variable.id]=m.add_variable(v.type,v.lower,v.upper,v.name);
  const auto terms=[&](const std::vector<Term>& src){std::vector<Term> out;for(const auto& t:src)out.push_back({map[t.variable.id],t.coefficient});return out;};
  for(const auto& r:original.rows)if(r.active)m.add_row(terms(r.terms),r.lower,r.upper,r.name);
  m.set_objective(terms(original.objective.terms),original.objective.sense,original.objective.offset);return m.snapshot();
}
void equivalent(const ModelSnapshot& a,const ModelSnapshot& b) {
  if(a.variables.size()!=b.variables.size()||a.rows.size()!=b.rows.size()||a.objective.offset!=b.objective.offset||a.objective.sense!=b.objective.sense)
    fail("export round trip changed dimensions/objective");
  const auto terms=[](const auto& x,const auto& y){if(x.size()!=y.size())fail("export changed expression size");for(std::size_t i=0;i<x.size();++i)
    if(x[i].variable.id!=y[i].variable.id||x[i].coefficient!=y[i].coefficient)fail("export changed coefficient or variable mapping");};
  for(std::size_t i=0;i<a.variables.size();++i){const auto& x=a.variables[i];const auto& y=b.variables[i];
    if(x.type!=y.type||x.lower!=y.lower||x.upper!=y.upper||x.name!=y.name)fail("export changed variable type, bounds or name");}
  for(std::size_t i=0;i<a.rows.size();++i){const auto& x=a.rows[i];const auto& y=b.rows[i];
    if(x.lower!=y.lower||x.upper!=y.upper||x.name!=y.name)fail("export changed row bounds or name");terms(x.terms,y.terms);}
  terms(a.objective.terms,b.objective.terms);
}

std::string serialize(const ModelSnapshot& m,bool mps) {
  std::ostringstream out;out.imbue(std::locale::classic());const char* comment=mps?"* ":"\\ ";
  for(const auto& v:m.variables)out<<comment<<"GECODE_NAME V x"<<v.variable.id<<' '<<hex(v.name)<<'\n';
  for(const auto& r:m.rows)out<<comment<<"GECODE_NAME R r"<<r.constraint.id<<' '<<hex(r.name)<<'\n';
  std::vector<double> costs(m.variables.size(),0);for(const auto& t:m.objective.terms)costs[t.variable.id]=t.coefficient;
  const auto expression_out=[&](const std::vector<Term>& terms){if(terms.empty())out<<" 0";for(const auto& t:terms)out<<(t.coefficient<0?" - ":" + ")<<real(std::fabs(t.coefficient))<<" x"<<t.variable.id;};
  if(!mps) {
    out<<(m.objective.sense==ObjectiveSense::Minimize?"Minimize\n":"Maximize\n")<<" obj:";
    // Explicit zero terms declare every column, including cost-free columns.
    for(std::size_t i=0;i<costs.size();++i)out<<(costs[i]<0?" - ":" + ")<<real(std::fabs(costs[i]))<<" x"<<i;
    out<<(m.objective.offset<0?" - ":" + ")<<real(std::fabs(m.objective.offset))<<"\nSubject To\n";
    for(const auto& r:m.rows) {
      const auto id=r.constraint.id;out<<" r"<<id<<':';expression_out(r.terms);
      if(r.lower==r.upper)out<<" = "<<real(r.lower)<<'\n';
      else if(std::isfinite(r.lower)) {
        out<<" >= "<<real(r.lower)<<'\n';
        if(std::isfinite(r.upper)){out<<comment<<"GECODE_RANGE r"<<id<<" u"<<id<<"\n u"<<id<<':';expression_out(r.terms);out<<" <= "<<real(r.upper)<<'\n';}
      } else out<<" <= "<<real(r.upper)<<'\n';
    }
    out<<"Bounds\n";for(const auto& v:m.variables)out<<' '<<real(v.lower)<<" <= x"<<v.variable.id<<" <= "<<real(v.upper)<<'\n';
    for(const auto type:{VariableType::Binary,VariableType::Integer,VariableType::SemiContinuous}) {
      bool wrote=false;for(const auto& v:m.variables) {
        const bool selected=type==VariableType::Binary?v.type==type:type==VariableType::Integer?
          (v.type==type||v.type==VariableType::SemiInteger):(v.type==type||v.type==VariableType::SemiInteger);
        if(selected){if(!wrote){out<<(type==VariableType::Binary?"Binary\n":type==VariableType::Integer?"General\n":"Semi\n");wrote=true;}out<<" x"<<v.variable.id<<'\n';}
      }
    }out<<"End\n";return out.str();
  }
  out<<"NAME Gecode\nOBJSENSE\n "<<(m.objective.sense==ObjectiveSense::Minimize?"MIN":"MAX")<<"\nROWS\n N obj\n";
  for(const auto& r:m.rows)out<<' '<<(r.lower==r.upper?'E':std::isfinite(r.upper)?'L':std::isfinite(r.lower)?'G':'N')<<" r"<<r.constraint.id<<'\n';
  std::vector<std::vector<std::pair<std::uint64_t,double>>> by_column(m.variables.size());
  for(const auto& r:m.rows)for(const auto& t:r.terms)by_column[t.variable.id].push_back({r.constraint.id,t.coefficient});
  out<<"COLUMNS\n";bool integer=false;std::size_t marker=0;
  for(const auto& v:m.variables) {
    const bool integral=v.type==VariableType::Integer||v.type==VariableType::Binary||v.type==VariableType::SemiInteger;
    if(integral!=integer){out<<" mark"<<marker++<<" 'MARKER' '"<<(integral?"INTORG":"INTEND")<<"'\n";integer=integral;}
    out<<" x"<<v.variable.id<<" obj "<<real(costs[v.variable.id])<<'\n';
    for(const auto& entry:by_column[v.variable.id])out<<" x"<<v.variable.id<<" r"<<entry.first<<' '<<real(entry.second)<<'\n';
  }
  if(integer)out<<" mark"<<marker<<" 'MARKER' 'INTEND'\n";
  out<<"RHS\n rhs obj "<<real(-m.objective.offset)<<'\n';
  for(const auto& r:m.rows)if(std::isfinite(r.lower)||std::isfinite(r.upper))out<<" rhs r"<<r.constraint.id<<' '<<real(std::isfinite(r.upper)?r.upper:r.lower)<<'\n';
  bool range_written=false;for(const auto& r:m.rows)if(std::isfinite(r.lower)&&std::isfinite(r.upper)&&r.lower!=r.upper){
    if(!range_written){out<<"RANGES\n";range_written=true;}out<<" rng r"<<r.constraint.id<<' '
      <<real(narrow(static_cast<long double>(r.upper)-r.lower))<<'\n';}
  out<<"BOUNDS\n";
  for(const auto& v:m.variables) {
    if(v.type==VariableType::Binary) {
      if(v.lower!=0||v.upper!=1)fail("MPS binary export requires bounds [0,1]; use LP for tighter binary bounds");
      out<<" BV bnd x"<<v.variable.id<<'\n';continue;
    }
    if(std::isfinite(v.lower))out<<" LO bnd x"<<v.variable.id<<' '<<real(v.lower)<<'\n';else out<<" MI bnd x"<<v.variable.id<<'\n';
    if(v.type==VariableType::SemiContinuous||v.type==VariableType::SemiInteger) {
      if(!std::isfinite(v.upper))fail("MPS semi-variable export requires finite upper bound");
      out<<(v.type==VariableType::SemiInteger?" SI":" SC")<<" bnd x"<<v.variable.id<<' '<<real(v.upper)<<'\n';
    } else if(std::isfinite(v.upper))out<<" UP bnd x"<<v.variable.id<<' '<<real(v.upper)<<'\n';else out<<" PL bnd x"<<v.variable.id<<'\n';
  }
  out<<"ENDATA\n";return out.str();
}

class Temporary {
public:
  std::filesystem::path path;
  FILE* file=nullptr;
  explicit Temporary(const std::filesystem::path& destination) {
    static std::atomic<unsigned long long> serial{0};std::random_device random;
    for(int attempt=0;attempt<64;++attempt) {
      path=destination.parent_path()/("."+destination.filename().string()+".tmp-"+std::to_string(random())+"-"+std::to_string(serial++));
#ifdef _WIN32
      const int fd=_wopen(path.c_str(),_O_CREAT|_O_EXCL|_O_WRONLY|_O_BINARY,_S_IREAD|_S_IWRITE);
      if(fd>=0){file=_fdopen(fd,"wb");if(!file)_close(fd);}
#else
      const int fd=::open(path.c_str(),O_CREAT|O_EXCL|O_WRONLY,0600);
      if(fd>=0){file=fdopen(fd,"wb");if(!file)::close(fd);}
#endif
      if(file)return;
      if(fd>=0){std::error_code ec;std::filesystem::remove(path,ec);fail("cannot open temporary output stream");}
      if(errno!=EEXIST)fail("cannot create same-directory temporary output");
    }fail("cannot allocate unique temporary output");
  }
  ~Temporary(){if(file)std::fclose(file);if(!path.empty()){std::error_code ec;std::filesystem::remove(path,ec);}}
  void write(const std::string& bytes) {
    if(std::fwrite(bytes.data(),1,bytes.size(),file)!=bytes.size()||std::fflush(file)!=0)fail("output write failed");
    FILE* closing=file;file=nullptr;if(std::fclose(closing)!=0)fail("output close failed");
  }
  void replace(const std::filesystem::path& destination) {
#ifdef _WIN32
    if(!MoveFileExW(path.c_str(),destination.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))fail("atomic destination replacement failed");
#else
    if(std::rename(path.c_str(),destination.c_str())!=0)fail("atomic destination replacement failed");
#endif
    path.clear();
  }
};
Model parse_file(const std::filesystem::path& path,bool mps) {
  std::ifstream input(path);if(!input)fail("cannot open input: "+path.string());input.imbue(std::locale::classic());
  Model model=mps?read_mps(input):read_lp(input);if(input.bad())fail("input read failed");return model;
}
}

Model read_model(const std::string& filename) {
  return parse_file(std::filesystem::path(filename),format(filename)==".mps");
}
void write_model(const ModelSnapshot& model,const std::string& filename) {
  const bool mps=format(filename)==".mps";
  if(std::any_of(model.globals.begin(),model.globals.end(),[](const auto& item){return item.active;}))
    fail("native globals cannot be preserved by linear LP/MPS export");
  if(std::any_of(model.indicators.begin(),model.indicators.end(),[](const auto& item){return item.active;}))
    fail("active original indicators require retained logical metadata and domain guards; linear LP/MPS export is unsupported for this model");
  const auto original=compact(model);const auto bytes=serialize(original,mps);
  const auto destination=std::filesystem::absolute(std::filesystem::path(filename));
  Temporary output(destination);output.write(bytes);
  auto restored=parse_file(output.path,mps);equivalent(original,restored.snapshot());output.replace(destination);
}
}}
