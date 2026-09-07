#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/constraints.hpp>
#ifdef GECODE_OPTIMIZE_IO_TEST_HIGHS
#include <Highs.h>
#endif

#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>

namespace O=Gecode::Optimize;
namespace fs=std::filesystem;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
void require(bool value,const std::string& message){if(!value)throw std::runtime_error(message);}
void rejected(const std::function<void()>& action,const std::string& message){
  bool caught=false;try{action();}catch(const O::ModelError&){caught=true;}require(caught,message);
}
void put(const fs::path& path,const std::string& text){std::ofstream f(path);f<<text;require(bool(f),"fixture write failed");}
std::string get(const fs::path& path){std::ifstream f(path);return {std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>()};}
struct Directory {
  fs::path path=fs::temp_directory_path()/("gecode-io-tests-"+std::to_string(std::random_device{}()));
  Directory(){require(fs::create_directory(path),"temporary directory unavailable");}
  ~Directory(){std::error_code ec;fs::remove_all(path,ec);}
};
void equal(const O::ModelSnapshot& a,const O::ModelSnapshot& b){
  require(a.variables.size()==b.variables.size()&&a.rows.size()==b.rows.size(),"roundtrip dimensions");
  for(std::size_t i=0;i<a.variables.size();++i){const auto& x=a.variables[i];const auto& y=b.variables[i];
    require(x.type==y.type&&x.lower==y.lower&&x.upper==y.upper&&x.name==y.name,"roundtrip variable semantics/name");}
  const auto terms=[](const auto& x,const auto& y){require(x.size()==y.size(),"term count");for(std::size_t i=0;i<x.size();++i)
    require(x[i].variable.id==y[i].variable.id&&x[i].coefficient==y[i].coefficient,"term semantics");};
  for(std::size_t i=0;i<a.rows.size();++i){const auto& x=a.rows[i];const auto& y=b.rows[i];
    require(x.lower==y.lower&&x.upper==y.upper&&x.name==y.name,"row bounds/name");terms(x.terms,y.terms);}
  require(a.objective.sense==b.objective.sense&&a.objective.offset==b.objective.offset,"objective sense/offset");
  terms(a.objective.terms,b.objective.terms);
}
O::Model comprehensive(){
  O::Model m;
  const auto a=m.add_continuous(-inf,inf,"space and \"quotes\"\nname");
  const auto b=m.add_integer(-5,9,"duplicate");
  const auto c=m.add_binary("duplicate");
  const auto d=m.add_variable(O::VariableType::SemiContinuous,2.5,8.5,"semi-continuous");
  const auto e=m.add_variable(O::VariableType::SemiInteger,3,11,"semi-integer");
  m.add_continuous(0,inf,""); // Must retain an unreferenced zero-cost column.
  m.add_integer(0,1,"not_binary");
  m.add_row({{a,0.125},{b,3},{d,-2}},-0.25,17.25,"range\nrow");
  m.add_row({{c,1},{e,2}},4,4,"equality");
  m.add_row({{a,1}},-inf,inf,"free row");
  m.add_row({},-inf,3,"constant");
  m.maximize({{a,0.0625},{e,-0.25}},17.125);
  return m;
}
void roundtrips(const fs::path& dir){
  auto model=comprehensive();
  for(const auto& extension:{".lp",".mps"}){
    const auto path=dir/(std::string("all")+extension);
    O::write_model(model,path.string());auto reread=O::read_model(path.string());equal(model.snapshot(),reread.snapshot());
    require(model.id()!=reread.id(),"import retained original model identity");
    put(path,"existing destination\n");O::write_model(model,path.string());equal(model.snapshot(),O::read_model(path.string()).snapshot());
  }
  O::Model empty;empty.minimize({},-9.125);empty.add_row({},1,inf,"infeasible constant");
  for(const auto& extension:{".lp",".mps"}){auto path=dir/(std::string("empty")+extension);O::write_model(empty,path.string());equal(empty.snapshot(),O::read_model(path.string()).snapshot());}
  O::Model removed;const auto dead=removed.add_integer();removed.remove(dead);const auto live=removed.add_continuous(1,3,"live");
  auto gone=removed.add_row({{live,1}},0,inf,"gone");removed.remove(gone);removed.minimize({{live,1}});
  for(const auto& extension:{".lp",".mps"}){auto path=dir/(std::string("deleted")+extension);O::write_model(removed,path.string());auto r=O::read_model(path.string()).snapshot();
    require(r.variables.size()==1&&r.rows.empty()&&r.variables[0].name=="live"&&r.variables[0].variable.id==0,"tombstone compaction");}
}
void precision(const fs::path& dir){
  constexpr double a=100000000000000.125,b=100000000000000.25;
  require(a!=b&&std::ceil(b/a)==2,"precision regression precondition");
  O::Model m;const auto x=m.add_integer(0,10,"integer");m.add_row({{x,a}},b,inf,"precision");m.minimize({{x,1}});
  for(const auto& extension:{".lp",".mps"}){auto path=dir/(std::string("precision")+extension);O::write_model(m,path.string());
    auto restored=O::read_model(path.string()).snapshot();equal(m.snapshot(),restored);
    require(std::ceil(restored.rows[0].lower/restored.rows[0].terms[0].coefficient)==2,"writer changed minimum 2 to 1");
    require(!O::validate(restored,{1.0},0.0,0.0).valid&&O::validate(restored,{2.0},0.0,0.0).valid,"precision witness distinction lost");}
  // Tiny coefficients are preserved by I/O even if the numerical backend
  // later explicitly refuses their unsupported numerical range.
  O::Model tiny;auto t=tiny.add_continuous();tiny.add_row({{t,1e-14}},0,1);tiny.minimize({{t,1e-20}});
  for(const auto& extension:{".lp",".mps"}){auto path=dir/(std::string("tiny")+extension);O::write_model(tiny,path.string());equal(tiny.snapshot(),O::read_model(path.string()).snapshot());}
}
void lp_semantics(const fs::path& dir){
  const auto path=dir/"external.lp";
  put(path,"Minimize\n obj: 2 x + 3 x + 7\nSubject To\n demand: x + 2 >= 5\nBounds\n 0 <= x <= 10\nGeneral\n x\nEnd\n");
  auto m=O::read_model(path.string()).snapshot();require(m.objective.terms[0].coefficient==5&&m.objective.offset==7,"duplicate LP objective overwritten");
  require(m.rows[0].lower==3,"LP LHS constant ignored");
  put(path,"Maximize\n obj: 1e16 + 1 - 1e16\nSubject To\nEnd\n");
  require(O::read_model(path.string()).snapshot().objective.offset==1,"LP objective cancellation lost");
  for(const auto& relation:{">=","<=","="}){
    put(path,std::string("Minimize\n obj: x\nSubject To\n c: x + 1 - 1e16 ")+relation+" -1e16\nBounds\n x free\nEnd\n");
    const auto row=O::read_model(path.string()).snapshot().rows[0];
    require((std::string(relation)=="<="||row.lower==-1)&&(std::string(relation)==">="||row.upper==-1),"LP RHS cancellation lost residual");
  }
  put(path,"Minimize\n obj: + 0 x + 0 y\nSubject To\n c: 2 x\n + 3 y >= 7\nBounds\n x free\n 2 <= y <= 8\nGeneral\n y\nSemi\n y\nEnd\n");
  m=O::read_model(path.string()).snapshot();require(m.variables[0].lower==-inf&&m.variables[1].type==O::VariableType::SemiInteger,"LP free/semi-integer import");
  require(m.rows[0].terms.size()==2,"LP wrapped row");
  put(path,"Minimize\n obj: y\nBounds\n 2 <= y <= 8\nSemi\n y\nGeneral\n y\nEnd\n");
  require(O::read_model(path.string()).snapshot().variables[0].type==O::VariableType::SemiInteger,"LP Semi then General lost integrality");
}
void mps_semantics(const fs::path& dir){
  const auto path=dir/"external.mps";
  put(path,"NAME sample\nOBJSENSE MAX\nROWS\n N obj\n L upper\n G lower\n E eqpos\n E eqneg\nCOLUMNS\n x obj 2 upper 1\n x obj 3 lower 1\n x eqpos 1 eqneg 1\nRHS\n rhs obj -7 upper 10\n rhs lower 5 eqpos 8\n rhs eqneg 9\nRANGES\n range upper 4 lower 2\n range eqpos 3 eqneg -2\nBOUNDS\n FR bounds x\nENDATA\n");
  const auto m=O::read_model(path.string()).snapshot();require(m.objective.sense==O::ObjectiveSense::Maximize&&m.objective.offset==7&&m.objective.terms[0].coefficient==5,"MPS objective duplicates/sense/offset");
  require(m.rows[0].lower==6&&m.rows[0].upper==10&&m.rows[1].lower==5&&m.rows[1].upper==7&&m.rows[2].lower==8&&m.rows[2].upper==11&&m.rows[3].lower==7&&m.rows[3].upper==9,"MPS RANGES signs");
}
void type_declarations(const fs::path& dir){
  const auto path=dir/"types.mps";
  const std::string base="NAME t\nROWS\n N obj\nCOLUMNS\n";
  const std::string column=" x obj 1\n";
  const std::string marked=" mark0 'MARKER' 'INTORG'\n"+column+" mark1 'MARKER' 'INTEND'\n";
  const std::vector<std::string> declarations={" BV b x\n"," SC b x 2\n"," SI b x 2\n"," LI b x 1\n"," UI b x 2\n"};
  for(std::size_t i=0;i<declarations.size();++i)for(std::size_t j=0;j<declarations.size();++j){
    // LI+UI defines an ordinary integer interval; LI then SI refines it
    // to a semi-integer domain without dropping integrality.
    if((i==3&&j==4)||(i==4&&j==3)||(i==3&&j==2))continue;
    put(path,base+column+"BOUNDS\n"+declarations[i]+declarations[j]+"ENDATA\n");
    rejected([&]{O::read_model(path.string());},"conflicting MPS declaration order accepted: "+declarations[i]+declarations[j]);
  }
  put(path,base+marked+"BOUNDS\n SC b x 2\nENDATA\n");
  rejected([&]{O::read_model(path.string());},"SC discarded INTORG integrality");
  for(const auto& declarations:std::vector<std::string>{" SI b x 2\n"," LI b x 1\n SI b x 2\n"}){
    put(path,base+marked+"BOUNDS\n"+declarations+"ENDATA\n");
    const auto v=O::read_model(path.string()).snapshot().variables[0];
    require(v.type==O::VariableType::SemiInteger&&v.lower==1&&v.upper==2,"valid SI discarded integer type");
  }
  for(const auto& declarations:std::vector<std::string>{" LI b x 1\n UI b x 2\n"," UI b x 2\n LI b x 1\n"}){
    put(path,base+column+"BOUNDS\n"+declarations+"ENDATA\n");
    const auto v=O::read_model(path.string()).snapshot().variables[0];
    require(v.type==O::VariableType::Integer&&v.lower==1&&v.upper==2,"valid integer bound order rejected");
  }
  for(const auto& order:std::vector<std::string>{"Binary\n x\nSemi\n x\n","Semi\n x\nBinary\n x\n","Binary\n x\nGeneral\n x\n","General\n x\nBinary\n x\n"}){
    const auto lp=dir/"types.lp";put(lp,"Minimize\n obj: x\n"+order+"End\n");
    rejected([&]{O::read_model(lp.string());},"conflicting LP declaration order accepted");
  }
}
void failures(const fs::path& dir){
  const auto target=dir/"protected.lp";put(target,"original bytes");auto m=comprehensive();
  auto bad=m.snapshot();bad.objective.offset=inf;
  rejected([&]{O::write_model(bad,target.string());},"invalid export succeeded");require(get(target)=="original bytes","failed export destroyed destination");
  O::Model logical;auto active=logical.add_binary("active");auto quantity=logical.add_continuous(0,10,"quantity");
  const auto indicator=O::add_indicator(logical,active,true,{{quantity,1}},3,inf,"logical");
  rejected([&]{O::write_model(logical,target.string());},"original indicator semantics dropped on export");
  require(get(target)=="original bytes","indicator rejection changed destination");
  O::remove_indicator(logical,indicator.indicator);
  O::write_model(logical,(dir/"removed-indicator.lp").string());
  rejected([&]{O::write_model(m,(dir/"missing"/"out.lp").string());},"missing directory accepted");
  rejected([&]{O::read_model((dir/"missing.lp").string());},"missing input accepted");
  rejected([&]{O::write_model(m,(dir/"out.txt").string());},"unknown output format accepted");
  auto protected_dir=dir/"destination.lp";fs::create_directory(protected_dir);put(protected_dir/"marker","keep");
  const auto count=[&]{return static_cast<std::size_t>(std::distance(fs::directory_iterator(dir),fs::directory_iterator()));};
  const auto before=count();rejected([&]{O::write_model(m,protected_dir.string());},"directory replaced");
  require(get(protected_dir/"marker")=="keep"&&count()==before,"failed atomic replacement leaked or damaged files");
  O::Model nonrepresentable;auto x=nonrepresentable.add_continuous();nonrepresentable.add_row({{x,1}},std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::max());
  const auto mps=dir/"protected.mps";put(mps,"original MPS bytes");const auto files=count();
  rejected([&]{O::write_model(nonrepresentable,mps.string());},"unrepresentable MPS range changed semantics");
  require(get(mps)=="original MPS bytes"&&count()==files,"roundtrip rejection changed destination or leaked temp");
  for(const auto& bounds:std::vector<std::pair<double,double>>{{0.25,0.75},{0,0},{1,1}}){
    O::Model tighter;auto binary=tighter.add_binary();tighter.set_bounds(binary,bounds.first,bounds.second);
    rejected([&]{O::write_model(tighter,mps.string());},"nondefault MPS binary bounds exported nonportably");
    require(get(mps)=="original MPS bytes"&&count()==files,"binary export rejection changed destination or leaked temp");
    const auto lp=dir/"tighter.lp";O::write_model(tighter,lp.string());equal(tighter.snapshot(),O::read_model(lp.string()).snapshot());
    fs::remove(lp);
  }
  const std::vector<std::string> invalid_lp={
    "Minimize\n obj: [ x ^ 2 ] / 2\nEnd\n",
    "Minimize\n obj: x\nSOS\n s: S1 :: x : 1\nEnd\n",
    "Minimize\n obj: x\nSubject To\n c: z = 1 -> x <= 2\nEnd\n",
    "Minimize\n obj: nan x\nEnd\n",
    "Minimize\n obj: 1e-9999 x\nEnd\n",
    "Minimize\n obj: 1e+2e+3\nEnd\n",
    "Minimize\n obj: x\nBounds\n x <= 3\n x <= 2\nEnd\n",
    "Minimize\n obj: x\nBounds\n x <= 3\n x free\nEnd\n",
    "Minimize\n obj: x\nEnd\nSome ignored content\n",
    "Minimize\n obj: x\n",
    "Minimize\n obj: x\nSubject To\n c: x < 2\nEnd\n",
    "\\ GECODE_NAME V absent 41\nMinimize\n obj: x\nEnd\n"};
  for(const auto& input:invalid_lp){put(target,input);rejected([&]{O::read_model(target.string());},"malformed/unsupported LP accepted: "+input);}
  const std::string base="NAME t\nROWS\n N obj\n L row\nCOLUMNS\n x obj 1 row 1\n";
  for(const auto& tail:std::vector<std::string>{
      "SOS\n S1 set\nENDATA\n","QUADOBJ\n x x 1\nENDATA\n","INDICATORS\n IF row x 1\nENDATA\n",
      "RHS\n a row 1\n b row 2\nENDATA\n","RHS\n a unknown 1\nENDATA\n",
      "BOUNDS\n UP b unknown 2\nENDATA\n","BOUNDS\n XX b x\nENDATA\n",
      "BOUNDS\n LO b x 1\n LI b x 2\nENDATA\n","ENDATA\ntrailing\n",""}){
    put(mps,base+tail);rejected([&]{O::read_model(mps.string());},"malformed/unsupported MPS accepted: "+tail);}
}
#ifdef GECODE_OPTIMIZE_IO_TEST_HIGHS
// An independent parser checks exported mathematics, including every unused
// column. It intentionally ignores private display-name/type distinctions
// absent from HiGHS (Binary is represented as an integer with bounds [0,1]).
void highs_export(const O::ModelSnapshot& model,const fs::path& path){
  O::write_model(model,path.string());
  Highs highs;highs.setOptionValue("output_flag",false);
  require(highs.readModel(path.string())==HighsStatus::kOk,"HiGHS export import warned/failed: "+path.string());
  const auto& lp=highs.getLp();
  require(static_cast<std::size_t>(lp.num_col_)==model.variables.size(),"HiGHS lost unused column");
  require(lp.sense_==(model.objective.sense==O::ObjectiveSense::Minimize?ObjSense::kMinimize:ObjSense::kMaximize)&&lp.offset_==model.objective.offset,"HiGHS changed sense/offset");
  std::map<std::string,std::size_t> columns;
  for(std::size_t i=0;i<lp.col_names_.size();++i)columns.emplace(lp.col_names_[i],i);
  std::vector<double> costs(model.variables.size(),0);for(const auto& t:model.objective.terms)costs[t.variable.id]=t.coefficient;
  for(const auto& v:model.variables){const auto name="x"+std::to_string(v.variable.id);
    require(columns.count(name)!=0,"HiGHS lost column "+name+" in "+path.string());const auto col=columns.at(name);
    const auto type=lp.integrality_.empty()?HighsVarType::kContinuous:lp.integrality_[col];
    const auto expected=v.type==O::VariableType::Continuous?HighsVarType::kContinuous:v.type==O::VariableType::SemiContinuous?HighsVarType::kSemiContinuous:v.type==O::VariableType::SemiInteger?HighsVarType::kSemiInteger:HighsVarType::kInteger;
    require(type==expected&&lp.col_lower_[col]==v.lower&&lp.col_upper_[col]==v.upper&&lp.col_cost_[col]==costs[v.variable.id],"HiGHS changed exported variable domain/cost: "+std::to_string(v.variable.id));
  }
  auto matrix=lp.a_matrix_;matrix.ensureColwise();
  std::vector<std::map<std::string,double>> rows(static_cast<std::size_t>(lp.num_row_));
  for(HighsInt col=0;col<lp.num_col_;++col)for(HighsInt p=matrix.start_[col];p<matrix.start_[col+1];++p)
    rows[matrix.index_[p]][lp.col_names_[col]]=matrix.value_[p];
  std::map<std::string,std::size_t> indices;for(std::size_t i=0;i<lp.row_names_.size();++i)indices.emplace(lp.row_names_[i],i);
  std::size_t expected_rows=0;
  for(const auto& r:model.rows){std::map<std::string,double> terms;for(const auto& t:r.terms)terms.emplace("x"+std::to_string(t.variable.id),t.coefficient);
    // HiGHS' free-MPS parser discards later N rows; these unconstrained
    // expressions have no effect on the feasible set or objective.
    if(path.extension()==".mps"&&r.lower==-inf&&r.upper==inf&&indices.count("r"+std::to_string(r.constraint.id))==0)continue;
    const bool split=path.extension()==".lp"&&std::isfinite(r.lower)&&std::isfinite(r.upper)&&r.lower!=r.upper;
    const auto check=[&](const std::string& name,double lb,double ub){
      require(indices.count(name)!=0,"HiGHS lost row "+name+" in "+path.string());const auto row=indices.at(name);++expected_rows;
      require(lp.row_lower_[row]==lb&&lp.row_upper_[row]==ub&&rows[row]==terms,"HiGHS changed exported row: "+name);};
    check("r"+std::to_string(r.constraint.id),r.lower,split?inf:r.upper);
    if(split)check("u"+std::to_string(r.constraint.id),-inf,r.upper);
  }
  require(expected_rows==static_cast<std::size_t>(lp.num_row_),"HiGHS changed exported row count");
}
void highs_portability(const fs::path& dir){
  auto all=comprehensive();for(const auto& ext:{".lp",".mps"})highs_export(all.snapshot(),dir/(std::string("highs-all")+ext));
  for(const auto& bounds:std::vector<std::pair<double,double>>{{0.25,0.75},{0,0},{1,1}}){
    O::Model m;auto x=m.add_binary();m.set_bounds(x,bounds.first,bounds.second);m.minimize({{x,1}});
    highs_export(m.snapshot(),dir/"highs-tighter.lp");
  }
  O::Model m;auto x=m.add_integer(0,10);m.add_row({{x,100000000000000.125}},100000000000000.25,inf);m.minimize({{x,1}},0.125);
  for(const auto& ext:{".lp",".mps"})highs_export(m.snapshot(),dir/(std::string("highs-precision")+ext));
}
#endif
void public_mps(const fs::path& directory,const fs::path& output){
  struct Entry{const char* name;std::size_t variables,rows;};
  for(const auto& e:std::vector<Entry>{{"p0033",33,16},{"lseu",89,28},{"p0201",201,133},{"p0282",282,241},{"p0548",548,176}}){
    const auto path=directory/(std::string(e.name)+".mps");auto m=O::read_model(path.string()).snapshot();
    require(m.variables.size()==e.variables&&m.rows.size()==e.rows,std::string("public MPS dimensions: ")+e.name);
    for(const auto& v:m.variables)require((v.type==O::VariableType::Binary||v.type==O::VariableType::Integer)&&v.lower==0&&v.upper==1,"public binary bounds/types");
    O::validate_structure(m);
    for(const auto& extension:{".lp",".mps"}){const auto saved=output/(std::string(e.name)+extension);
      O::write_model(m,saved.string());equal(m,O::read_model(saved.string()).snapshot());}
#ifdef GECODE_OPTIMIZE_IO_TEST_HIGHS
    for(const auto& extension:{".lp",".mps"})highs_export(m,output/(std::string("highs-")+e.name+extension));
#endif
  }
}
}
int main(int argc,char** argv){
  try{Directory temp;roundtrips(temp.path);precision(temp.path);lp_semantics(temp.path);mps_semantics(temp.path);type_declarations(temp.path);failures(temp.path);
#ifdef GECODE_OPTIMIZE_IO_TEST_HIGHS
    highs_portability(temp.path);
#endif
    if(argc>1)public_mps(argv[1],temp.path);
    std::cout<<"PASS lossless numerical LP/MPS I/O"<<(argc>1?" and five public MPS imports":"")
#ifdef GECODE_OPTIMIZE_IO_TEST_HIGHS
      <<" with independent HiGHS export checks"
#endif
      <<'\n';return 0;
  }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
