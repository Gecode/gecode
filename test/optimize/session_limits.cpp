/* Persistent option reset and callback lifetime regression, HiGHS 1.15.1. */
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/validate.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>

namespace O=Gecode::Optimize;
namespace {
constexpr double inf=std::numeric_limits<double>::infinity();
void require(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
void near(double a,double b){require(std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<1e-6,"objective mismatch");}

O::Model node_fixture(){
  // Fixed mt19937 outputs (no implementation-dependent uniform distribution).
  // Five-dimensional 0/1 knapsack. With HiGHS 1.15.1, seed 219, one thread
  // and zero gap, node one leaves incumbent 1579 and upper bound 1590.
  // Both a cold run and a retained-incumbent run need more search to close
  // that gap. Do not replace it with a root-solved bound-only MIP.
  std::mt19937 random(219);
  O::Model m;std::vector<O::Variable> variables;std::vector<O::Term> objective;
  for(int i=0;i<48;++i){variables.push_back(m.add_binary());objective.push_back({variables.back(),double(1+random()%100)});}
  for(int row=0;row<5;++row){std::vector<O::Term> terms;int sum=0;
    for(auto v:variables){const int weight=1+random()%100;sum+=weight;terms.push_back({v,double(weight)});}
    m.add_row(terms,-inf,(45*sum)/100);
  }
  m.maximize(objective);return m;
}

O::SolveOptions unlimited_options(){
  O::SolveOptions options;options.threads=1;options.random_seed=219;
  options.relative_gap=options.absolute_gap=0;options.time_limit_seconds=120;
  return options;
}

void require_limited(const O::Model& model,const O::SolveResult& result){
  require(result.termination==O::Termination::NodeLimit,"fixture no longer exercises a post-registration node cutoff");
  require(result.has_solution(),"node-limited fixture lost its known incumbent");
  require(O::validate(model.snapshot(),result.values).valid,"limited incumbent fails original validation");
  require(result.best_bound&&*result.best_bound>*result.objective+1,"node-one bound already closes the gap; reset test is vacuous");
}
}

int main(){
  try{
    auto model=node_fixture();O::SolveSession session;
    if(!O::capabilities().available){
      require(session.solve(model).termination==O::Termination::Unsupported,"missing-backend session contract");
      std::cout<<"session limits backend-unavailable contract passed\n";return 0;
    }
    auto limited_options=unlimited_options();limited_options.node_limit=1;
    const auto first=session.solve(model,limited_options);require_limited(model,first);
    require(session.statistics().model_loads==1,"first limited model was not loaded");

    // Positive control: retained hints alone must not let node one solve the
    // fixture. Thus removing the persistent mip_max_nodes reset is detected.
    const auto again=session.solve(model,limited_options);require_limited(model,again);
    require(again.start_submitted&&session.statistics().incumbent_starts==1,"retained incumbent path was not exercised");

    // A NEW options object has no node limit. Neither the previous native
    // option nor its budget/callback captures may survive this call.
    const auto unlimited=unlimited_options();
    const auto warm=session.solve(model,unlimited);
    const auto cold=O::solve(model,unlimited);
    require(warm.termination==O::Termination::Optimal&&warm.has_solution(),"old node limit contaminated unlimited session solve");
    require(cold.termination==O::Termination::Optimal&&cold.has_solution(),"cold oracle did not complete");
    require(O::validate(model.snapshot(),warm.values).valid&&O::validate(model.snapshot(),cold.values).valid,"completed original-model validation failed");
    near(*warm.objective,*cold.objective);near(*warm.objective,1579);
    require(session.statistics().model_loads==1,"option reset unexpectedly reloaded the model");
    require(warm.start_submitted&&session.statistics().incumbent_starts==2,"unlimited solve did not exercise incumbent reuse");

    // NodeLimit with a positive limit is only obtained after highs.run() and
    // callback registration; the adapter's pre-run node counter is zero.
    // Follow an interruption with a changed model to exercise teardown
    // and replacement of callbacks that captured expired budgets/vectors.
    // This is deterministic post-registration LIMIT interruption, not a claim
    // to deterministically observe an asynchronous CancellationToken callback.
    // That narrower test needs a registration hook; sleep-based races are not
    // used here. Run this executable with the full library under ASan/UBSan.
    {
      require_limited(model,session.solve(model,limited_options));
      O::Model lp;const auto x=lp.add_continuous(2,5);lp.minimize({{x,3}},-1);
      const auto fresh=session.solve(lp,unlimited_options());
      require(fresh.termination==O::Termination::Optimal&&fresh.has_solution(),"post-interruption fresh model failed");
      near(fresh.value(x),2);near(*fresh.objective,5);
      // The original owning result must survive replacements and interrupts.
      near(*first.objective,1579);require(first.has_solution(),"historical limited result was invalidated");
    }
    std::cout<<"persistent node-limit reset and post-registration callback lifetime passed\n";return 0;
  }catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
