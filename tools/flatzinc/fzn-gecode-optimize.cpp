/* Explicit, bounded FlatZinc capture/compiler frontend. Legacy driver is separate. */
#include <gecode/flatzinc/capture.hh>
#include <gecode/optimize/flatzinc.hpp>
#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/native_neighborhoods.hpp>
#include <gecode/optimize/validate.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>

namespace FznOptimizeDriver {
namespace O=Gecode::Optimize;
namespace F=Gecode::FlatZinc::Capture;
using Clock=std::chrono::steady_clock;
struct Options {
  std::string filename;
  bool minizinc=false;
  F::Options capture;
  O::SolveOptions solve;
  std::string native_mode="auto",lp="off",search="dfs",branching="default",neighborhood="off";
  O::NativeAutoSettings automatic;
  O::NativeRaceOptions race;
  O::NativeLpSettings relaxation;
  O::NativeBranchingSettings branching_settings;
  O::NativeNeighborhoodSettings neighborhood_settings;
  std::size_t max_open_nodes=100000;
  bool root_cuts=false,diagnostics=false;
  Options() {
    solve.backend=O::Backend::Native;solve.guarantee=O::Guarantee::Exact;
    solve.relative_gap=0;solve.absolute_gap=0;
  }
};
const char* usage() {
  return "Usage: fzn-gecode-optimize MODEL.fzn|- [--backend native|highs] "
    "[--time-limit SECONDS] [--node-limit N] [--max-input-bytes N]\n"
    "One satisfaction solution or final optimization; bounded integer/Boolean subset.\n"
    "Native uses exact integer search; HiGHS is explicitly numerical.\n"
    "MiniZinc protocol: --minizinc [-t MILLISECONDS] [NATIVE OPTIONS] MODEL.fzn|-\n"
    "Native options (all take a value):\n"
    "  --native-mode auto|race|plain|configured (default auto)\n"
    "  --native-auto-presolve/--native-auto-components/--native-auto-symmetry/--native-auto-knapsack on|off\n"
    "  --native-race-seconds SECONDS --native-race-nodes N (race only)\n"
    "  --native-lp off|root|updated --native-root-cuts on|off\n"
    "  --native-bound-tightening on|off --native-lp-interval N\n"
    "  --native-search bab|dfs|best-bound --native-max-open-nodes N\n"
    "  --native-branching default|reliability --native-branching-probes N\n"
    "  --native-neighborhood off|hamming --native-neighborhood-radius N\n"
    "  --native-neighborhood-nodes N --native-neighborhood-seconds SECONDS\n"
    "  LP/search/neighborhood controls require configured mode.\n"
    "  --native-node-limit N --native-diagnostics on|off\n"
    "Racing uses sequential probes and restarts: it can increase total CPU and solve time.\n";
}
std::uint64_t count(const std::string& text) {
  if(text.empty())throw std::invalid_argument("Empty count");
  std::uint64_t value=0;
  for(unsigned char c:text) {
    if(c<'0'||c>'9'||value>(std::numeric_limits<std::uint64_t>::max()-(c-'0'))/10)
      throw std::invalid_argument("Count must be an unsigned decimal integer in range");
    value=value*10+(c-'0');
  }
  return value;
}
double seconds(const std::string& text) {
  std::istringstream input(text);input.imbue(std::locale::classic());double value;
  input>>std::noskipws>>value;
  if(input.fail()||!input.eof()||!std::isfinite(value)||value<0)
    throw std::invalid_argument("Time limit must be finite and nonnegative");
  return value;
}
bool on_off(const std::string& value) {
  if(value=="on")return true;if(value=="off")return false;
  throw std::invalid_argument("Boolean native options must be on or off");
}
std::size_t size_count(const std::string& value) {
  const auto n=count(value);
  if(n>std::numeric_limits<std::size_t>::max())throw std::invalid_argument("Count exceeds size_t range");
  return static_cast<std::size_t>(n);
}
std::string choice(const std::string& key,const std::string& value,std::initializer_list<const char*> choices) {
  for(const auto* accepted:choices)if(value==accepted)return value;
  throw std::invalid_argument("Invalid value for "+key+": "+value);
}
bool native_argument(Options& o,const std::string& key,const std::string& value) {
  if(key=="--native-mode")o.native_mode=choice(key,value,{"auto","race","plain","configured"});
  else if(key=="--native-auto-presolve")o.automatic.presolve=on_off(value);
  else if(key=="--native-auto-components")o.automatic.components=on_off(value);
  else if(key=="--native-auto-symmetry")o.automatic.symmetry=on_off(value);
  else if(key=="--native-auto-knapsack")o.automatic.knapsack=on_off(value);
  else if(key=="--native-race-seconds")o.race.exploration_seconds=seconds(value);
  else if(key=="--native-race-nodes") {
    o.race.probe_node_limit=count(value);
    if(!o.race.probe_node_limit)throw std::invalid_argument("Native race node budget must be positive");
  } else if(key=="--native-lp")o.lp=choice(key,value,{"off","root","updated"});
  else if(key=="--native-root-cuts")o.root_cuts=on_off(value);
  else if(key=="--native-bound-tightening")o.relaxation.bound_tightening=on_off(value);
  else if(key=="--native-lp-interval") {
    const auto n=count(value);
    if(!n||n>std::numeric_limits<unsigned int>::max())throw std::invalid_argument("LP interval must be a positive unsigned integer in range");
    o.relaxation.bound_change_interval=static_cast<unsigned int>(n);
  } else if(key=="--native-search")o.search=choice(key,value,{"bab","dfs","best-bound"});
  else if(key=="--native-branching")o.branching=choice(key,value,{"default","reliability"});
  else if(key=="--native-branching-probes")o.branching_settings.max_probe_status_calls=count(value);
  else if(key=="--native-max-open-nodes")o.max_open_nodes=size_count(value);
  else if(key=="--native-neighborhood")o.neighborhood=choice(key,value,{"off","hamming"});
  else if(key=="--native-neighborhood-radius")o.neighborhood_settings.radius=size_count(value);
  else if(key=="--native-neighborhood-nodes")o.neighborhood_settings.max_status_calls=count(value);
  else if(key=="--native-neighborhood-seconds")o.neighborhood_settings.time_limit_seconds=seconds(value);
  else if(key=="--native-node-limit")o.solve.node_limit=count(value);
  else if(key=="--native-diagnostics")o.diagnostics=on_off(value);
  else return false;
  return true;
}
O::NativeSearchOptions search_options(const Options& o,const O::SolveOptions& solve) {
  O::NativeSearchOptions result;result.solve=solve;
  result.order=o.search=="best-bound"?O::NativeSearchOrder::BestBound:O::NativeSearchOrder::DepthFirst;
  result.max_open_nodes=o.max_open_nodes;
  if(o.lp!="off")result.relaxation=o.relaxation;
  if(o.branching=="reliability")result.branching=o.branching_settings;
  return result;
}
void validate_arguments(Options& o,const std::set<std::string>& seen) {
  for(const auto& key:seen) {
    if(key.compare(0,9,"--native-")!=0)continue;
    if(o.solve.backend!=O::Backend::Native)throw std::invalid_argument("Native controls require the native backend");
    if(key=="--native-mode"||key=="--native-node-limit"||key=="--native-diagnostics")continue;
    if(key.compare(0,14,"--native-auto-")==0) {
      if(o.native_mode!="auto"&&o.native_mode!="race")throw std::invalid_argument(key+" requires auto or race mode");
    } else if(key.compare(0,14,"--native-race-")==0) {
      if(o.native_mode!="race")throw std::invalid_argument(key+" requires race mode");
    } else if(o.native_mode!="configured")throw std::invalid_argument(key+" requires configured mode");
  }
  const auto requires=[&](const std::string& key,bool valid,const std::string& reason) {
    if(seen.count(key)&&!valid)throw std::invalid_argument(key+" requires "+reason);
  };
  requires("--native-root-cuts",o.lp!="off","LP enabled");
  requires("--native-bound-tightening",o.lp!="off","LP enabled");
  requires("--native-lp-interval",o.lp=="updated","updated LP frequency");
  requires("--native-branching",o.search!="bab","frontier search (dfs or best-bound)");
  requires("--native-max-open-nodes",o.search!="bab","frontier search (dfs or best-bound)");
  requires("--native-neighborhood",o.search!="bab","frontier search (dfs or best-bound)");
  requires("--native-branching-probes",o.branching=="reliability","reliability branching");
  for(const auto* key:{"--native-neighborhood-radius","--native-neighborhood-nodes","--native-neighborhood-seconds"})
    requires(key,o.neighborhood=="hamming","Hamming neighborhoods");
  o.relaxation.frequency=o.lp=="updated"?O::NativeLpFrequency::AfterBoundChanges:O::NativeLpFrequency::Root;
  if(o.root_cuts)o.relaxation.root_cover_cuts=O::NativeRootCoverSettings{};
  o.solve.validate();
  if(o.native_mode=="race") {o.race.solve=o.solve;o.race.automatic=o.automatic;o.race.validate();}
  if(o.native_mode=="configured") {
    if(o.lp!="off")o.relaxation.validate();
    if(o.search!="bab")search_options(o,o.solve).validate();
    if(o.neighborhood=="hamming")o.neighborhood_settings.validate();
  }
}
Options arguments(const std::vector<std::string>& args) {
  if(!args.empty()&&args[0]=="--minizinc") {
    Options options;options.minizinc=true;bool timed=false,filename_only=false;std::set<std::string> seen;
    for(std::size_t i=1;i<args.size();++i) {
      const auto& arg=args[i];
      if(!filename_only&&arg=="--") {filename_only=true;continue;}
      if(!filename_only&&arg=="-t") {
        if(timed||i+1==args.size())throw std::invalid_argument("Missing or repeated -t time limit");
        timed=true;const auto milliseconds=count(args[++i]);
        // MiniZinc 2.10.1 treats a zero solver time limit as unlimited.
        if(milliseconds)options.solve.time_limit_seconds=static_cast<double>(milliseconds)/1000.0;
      } else if(!filename_only&&arg.compare(0,9,"--native-")==0) {
        if(i+1==args.size()||!seen.insert(arg).second)throw std::invalid_argument("Missing or repeated option: "+arg);
        if(!native_argument(options,arg,args[++i]))throw std::invalid_argument("Unsupported MiniZinc protocol option: "+arg);
      } else if(arg.empty()||(!filename_only&&arg[0]=='-'&&arg!="-"))
        throw std::invalid_argument("Unsupported MiniZinc protocol option: "+arg);
      else {
        if(!options.filename.empty())throw std::invalid_argument("MiniZinc protocol requires exactly one model");
        options.filename=arg;
      }
    }
    if(options.filename.empty())throw std::invalid_argument("MiniZinc protocol requires exactly one model");
    options.capture.source=options.filename;validate_arguments(options,seen);return options;
  }
  if(args.empty()||args[0].empty()||(args[0][0]=='-'&&args[0]!="-"))
    throw std::invalid_argument(usage());
  Options options;options.filename=args[0];options.capture.source=args[0];std::set<std::string> seen;
  for(std::size_t i=1;i<args.size();i+=2) {
    const auto& key=args[i];
    const auto canonical=key=="--node-limit"?"--native-node-limit":key;
    if(i+1==args.size()||!seen.insert(canonical).second)throw std::invalid_argument("Missing or repeated option: "+key);
    const auto& value=args[i+1];
    if(key=="--backend") {
      if(value=="native") {options.solve.backend=O::Backend::Native;options.solve.guarantee=O::Guarantee::Exact;}
      else if(value=="highs") {options.solve.backend=O::Backend::Highs;options.solve.guarantee=O::Guarantee::Numerical;}
      else throw std::invalid_argument("Backend must be native or highs");
    } else if(key=="--time-limit")options.solve.time_limit_seconds=seconds(value);
    else if(key=="--node-limit")options.solve.node_limit=count(value);
    else if(key=="--max-input-bytes") {
      const auto n=count(value);
      if(n>static_cast<std::uint64_t>(std::numeric_limits<int>::max()))throw std::invalid_argument("Input limit exceeds parser index range");
      options.capture.max_input_bytes=static_cast<std::size_t>(n);
    } else if(!native_argument(options,key,value))throw std::invalid_argument("Unsupported option: "+key);
  }
  // The historical unprefixed node limit is also valid with HiGHS.
  if(seen.count("--native-node-limit")&&std::find(args.begin(),args.end(),"--native-node-limit")==args.end())seen.erase("--native-node-limit");
  validate_arguments(options,seen);return options;
}
struct SolveOutput {O::SolveResult result;std::string work;};
std::string lp_work(const O::NativeLpStatistics& s) {
  return "lp-calls="+std::to_string(s.lp_calls)+" checked-bounds="+std::to_string(s.valid_bounds)+
    " root-cuts="+std::to_string(s.root_cover.cuts)+" variable-fixings="+std::to_string(s.variable_fixings)+
    " bound-tightenings="+std::to_string(s.variable_bound_tightenings);
}
SolveOutput search_output(O::NativeSearchResult result) {
  auto work=lp_work(result.relaxation)+" frontier-admitted="+std::to_string(result.frontier.admitted_nodes)+
    " branching-probes="+std::to_string(result.branching.probe_status_calls)+
    " budget-nodes="+std::to_string(result.branching.budget_nodes);
  return {std::move(result.result),std::move(work)};
}
const char* neighborhood_completion(O::NativeNeighborhoodCompletion value) {
  using C=O::NativeNeighborhoodCompletion;
  switch(value) {
    case C::NotStarted:return "not-started";
    case C::NoIncumbent:return "no-incumbent";
    case C::ProofCompletedBeforeAttempt:return "proof-completed-before-attempt";
    case C::NoEligibleBinary:return "no-eligible-binary";
    case C::NonrestrictingRadius:return "nonrestricting-radius";
    case C::FormulationLimit:return "formulation-limit";
    case C::SourceLimit:return "source-limit";
    case C::WorkLimit:return "work-limit";
    case C::StatusLimit:return "status-limit";
    case C::SharedNodeReserve:return "shared-node-reserve";
    case C::LocalStorageLimit:return "local-storage-limit";
    case C::LocalTimeLimit:return "local-time-limit";
    case C::NoImprovement:return "no-improvement";
    case C::Improved:return "improved";
    case C::GlobalStop:return "global-stop";
    case C::Error:return "error";
  }
  return "unknown";
}
SolveOutput dispatch(const O::ModelSnapshot& model,const Options& o,const O::SolveOptions& solve) {
  if(solve.backend!=O::Backend::Native)return {O::solve(model,solve),{}};
  if(o.native_mode=="plain")return {O::solve_native(model,solve),{}};
  if(o.native_mode=="auto") {
    O::NativeAutoOptions automatic;automatic.solve=solve;automatic.settings=o.automatic;
    return {O::solve_native_auto_configured(model,automatic),{}};
  }
  if(o.native_mode=="race") {
    auto race=o.race;race.solve=solve;race.automatic=o.automatic;
    return {O::solve_native_race(model,race),{}};
  }
  if(o.search=="bab") {
    if(o.lp=="off")return {O::solve_native(model,solve),{}};
    O::NativeLpOptions lp;static_cast<O::NativeLpSettings&>(lp)=o.relaxation;lp.solve=solve;
    auto result=O::solve_native_lp(model,lp);auto work=lp_work(result.relaxation);
    return {std::move(result.result),std::move(work)};
  }
  auto search=search_options(o,solve);
  if(o.neighborhood=="off")return search_output(O::solve_native_search(model,search));
  O::NativeNeighborhoodOptions neighborhood;neighborhood.search=search;neighborhood.neighborhood=o.neighborhood_settings;
  auto result=O::solve_native_neighborhoods(model,neighborhood);auto output=search_output(std::move(result.search));
  output.work+=" neighborhood-attempts="+std::to_string(result.neighborhood.attempts)+
    " neighborhood-improvements="+std::to_string(result.neighborhood.accepted_improvements)+
    " neighborhood-status-attempts="+std::to_string(result.neighborhood.status_attempts)+
    " neighborhood-eligible="+std::to_string(result.neighborhood.eligible_variables)+
    " neighborhood-completion="+neighborhood_completion(result.neighborhood.completion);
  return output;
}
#ifdef GECODE_FLATZINC_DRIVER_TEST
SolveOutput test_solve(const O::ModelSnapshot&,const Options&,const O::SolveOptions&);
#endif
std::string location(const F::Location& at) {
  return at.source+(at.line?":"+std::to_string(at.line):"");
}
struct Output {std::string text;int code=1;};
std::string comment_line(std::string value) {
  for(auto& c:value)if(static_cast<unsigned char>(c)<32||static_cast<unsigned char>(c)==127)c=' ';
  return value;
}
std::string configuration(const Options& o) {
  std::ostringstream text;text.imbue(std::locale::classic());
  const auto on=[](bool value){return value?"on":"off";};
  if(o.native_mode=="auto"||o.native_mode=="race") {
    text<<"auto-presolve="<<on(o.automatic.presolve)<<" auto-components="<<on(o.automatic.components)
      <<" auto-symmetry="<<on(o.automatic.symmetry)<<" auto-knapsack="<<on(o.automatic.knapsack);
    if(o.native_mode=="race")text<<" race-seconds="<<o.race.exploration_seconds<<" race-nodes="<<o.race.probe_node_limit;
  } else if(o.native_mode=="configured") {
    text<<"search="<<o.search<<" lp="<<o.lp<<" root-cuts="<<on(o.root_cuts)
      <<" bound-tightening="<<on(o.relaxation.bound_tightening)<<" lp-interval="<<o.relaxation.bound_change_interval
      <<" branching="<<o.branching<<" branching-probes="<<o.branching_settings.max_probe_status_calls
      <<" max-open-nodes="<<o.max_open_nodes<<" neighborhood="<<o.neighborhood
      <<" neighborhood-radius="<<o.neighborhood_settings.radius<<" neighborhood-nodes="<<o.neighborhood_settings.max_status_calls
      <<" neighborhood-seconds="<<o.neighborhood_settings.time_limit_seconds;
  } else text<<"ordinary-native";
  text<<" node-limit=";
  if(o.solve.node_limit)text<<*o.solve.node_limit;else text<<"unlimited";
  return text.str();
}
Output render(const O::CompiledFlatZinc& compiled,const O::SolveResult& result,const Options& options,const std::string& work={}) {
  switch(result.termination) {
    case O::Termination::Unsupported:case O::Termination::InvalidModel:
    case O::Termination::BackendError:case O::Termination::NumericalFailure:
      throw std::runtime_error(std::string(O::to_string(result.termination))+": "+result.message);
    default:break;
  }
  if(result.model_id!=compiled.model().model_id||result.revision!=compiled.model().revision)
    throw std::runtime_error("Backend returned a result for a different model");
  if(result.guarantee!=options.solve.guarantee)throw std::runtime_error("Backend returned a different guarantee");
  if(result.objective&&!std::isfinite(*result.objective))throw std::runtime_error("Backend returned a nonfinite objective");
  if(result.termination==O::Termination::Infeasible&&result.values.size()==compiled.model().variables.size()) {
    auto candidate=result;bool complete=true;
    candidate.active_variables.clear();
    for(const auto& v:compiled.model().variables) {
      candidate.active_variables.push_back(v.active);
      if(!v.active)continue;
      auto& x=candidate.values[v.variable.id];const auto rounded=std::round(x);
      if(!std::isfinite(x)||std::abs(x-rounded)>options.solve.integrality_tolerance) {complete=false;break;}
      x=rounded;
    }
    if(complete) {
      const auto checked=O::validate(compiled.model(),candidate.values,0,0);
      if(checked.valid&&checked.objective) {
        candidate.objective=checked.objective;candidate.solution_validated=true;
        if(O::validate_flatzinc(compiled,candidate,0).valid)
          throw std::runtime_error("Infeasible status contradicts the backend's original feasible assignment");
      }
    }
  }
  for(const auto& gap:{result.absolute_gap,result.relative_gap,result.native_backend_gap})
    if(gap&&(!std::isfinite(*gap)||*gap<0))throw std::runtime_error("Backend returned an invalid gap");
  if(result.best_bound&&!std::isfinite(*result.best_bound))throw std::runtime_error("Backend returned a nonfinite bound for the bounded model");
  if(result.objective&&result.best_bound) {
    auto checked=result;checked.update_gaps(compiled.model().objective.sense);
    if((result.absolute_gap&&result.absolute_gap!=checked.absolute_gap)||
       (result.relative_gap&&result.relative_gap!=checked.relative_gap))
      throw std::runtime_error("Backend returned inconsistent gap fields");
  } else if(result.absolute_gap||result.relative_gap)throw std::runtime_error("Backend returned gaps without an objective and bound");
  const bool point=result.has_solution();
  if((result.termination==O::Termination::Optimal&&!point)||
     (result.termination==O::Termination::Infeasible&&point))
    throw std::runtime_error("Backend returned inconsistent status and assignment");
  if(result.termination==O::Termination::Unbounded||result.termination==O::Termination::InfeasibleOrUnbounded)
    throw std::runtime_error("Unexpected unbounded status for the finite discrete model");
  // Do not silently hide a malformed claimed incumbent behind UNKNOWN or UNSAT.
  if(result.solution_validated&&!point)throw std::runtime_error("Backend returned a malformed incumbent");
  if(result.termination==O::Termination::Optimal&&compiled.source().solve.method!=F::Method::Satisfy) {
    if(!result.best_bound)throw std::runtime_error("Completed optimization has no bound");
    if(options.solve.guarantee==O::Guarantee::Exact&&result.best_bound!=result.objective)
      throw std::runtime_error("Exact optimal status has an open objective gap");
  }
  Output output;
  output.text=options.solve.backend==O::Backend::Native ? "% guarantee: exact integer search\n" : "% guarantee: numerical; requested MIP gaps: 0\n";
  if(options.diagnostics) {
    output.text+="% native-mode: "+comment_line(options.native_mode)+"\n% native-backend: "+comment_line(result.backend)+
      "\n% native-policy: "+comment_line(result.message)+"\n% native-configuration: "+comment_line(configuration(options))+"\n";
    if(!work.empty())output.text+="% native-work: "+comment_line(work)+"\n";
  }
  if(point) {
    output.text+=O::format_flatzinc_solution(compiled,result,options.solve.integrality_tolerance);
    if(result.termination==O::Termination::Optimal) {
      if(compiled.source().solve.method!=F::Method::Satisfy)output.text+="==========\n";
      output.code=0;
    }
  } else if(result.termination==O::Termination::Infeasible) {
    output.text+="=====UNSATISFIABLE=====\n";output.code=0;
  } else output.text+="=====UNKNOWN=====\n";
  return output;
}
int execute(const Options& options,std::istream& input,std::ostream& out,std::ostream& errors) {
  const auto begin=Clock::now();
  const auto remaining=[&] {return std::max(0.0,options.solve.time_limit_seconds-
    std::chrono::duration<double>(Clock::now()-begin).count());};
  const auto expired=[&] {return remaining()==0;};
  const auto unknown=[&] {out<<"% time limit reached\n=====UNKNOWN=====\n";return options.minizinc?0:1;};
  try {
    if(expired())return unknown();
    auto captured=F::parse(input,options.capture);
    if(expired())return unknown();
    if(captured.status!=F::Status::Complete||!captured.records) {
      std::string message="FlatZinc capture failed";
      if(!captured.diagnostics.empty())message=location(captured.diagnostics[0].location)+": "+captured.diagnostics[0].message;
      throw std::runtime_error(message);
    }
    O::FlatZincCompileOptions compiler;compiler.time_limit_seconds=remaining();
    auto compiled=O::compile_flatzinc(*captured.records,compiler);
    if(expired()||compiled.status==O::FlatZincCompileStatus::TimeLimit)return unknown();
    if(compiled.status!=O::FlatZincCompileStatus::Complete||!compiled.compiled)
      throw std::runtime_error(location(compiled.location)+": "+compiled.message);
    auto solve_options=options.solve;solve_options.time_limit_seconds=remaining();
#ifdef GECODE_FLATZINC_DRIVER_TEST
    auto solved=test_solve(compiled.compiled->model(),options,solve_options);
#else
    auto solved=dispatch(compiled.compiled->model(),options,solve_options);
#endif
    // The parser has cooperative stage boundaries, not token-level interruption.
    // Never publish a newly returned point after the overall frontend deadline.
    if(expired())return unknown();
    const auto& result=solved.result;
    auto output=render(*compiled.compiled,result,options,solved.work);
    if(expired())return unknown();
    out<<output.text;
    if(output.code)errors<<"Solve stopped: "<<O::to_string(result.termination)<<"; "<<result.message<<'\n';
    return options.minizinc&&output.code==1?0:output.code;
  } catch(const std::bad_alloc&) {errors<<"FlatZinc optimization: memory allocation failed\n";return 2;}
    catch(const std::exception& e) {errors<<"FlatZinc optimization: "<<e.what()<<'\n';return 2;}
}
}
#ifndef GECODE_FLATZINC_DRIVER_TEST
int main(int argc,char** argv) {
  using namespace FznOptimizeDriver;
  if(argc==2&&std::string(argv[1])=="--help") {std::cout<<usage();return 0;}
  try {
    const auto options=arguments(std::vector<std::string>(argv+1,argv+argc));
    if(options.filename=="-")return execute(options,std::cin,std::cout,std::cerr);
    std::ifstream input(options.filename,std::ios::binary);
    if(!input)throw std::runtime_error("Cannot open input: "+options.filename);
    return execute(options,input,std::cout,std::cerr);
  } catch(const std::exception& e) {std::cerr<<"FlatZinc optimization: "<<e.what()<<'\n';return 2;}
}
#endif
