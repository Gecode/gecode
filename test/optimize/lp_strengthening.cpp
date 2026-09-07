// Standalone exhaustive correctness tests for binary linear strengthening.
#include <gecode/minimodel/lp-strengthening.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace LP=Gecode::Experimental::LpRelaxation;
namespace S=LP::Strengthening;
using I=std::int64_t;
using V=std::vector<I>;

static void require(bool condition,const char* message) {
  if (!condition) throw std::runtime_error(message);
}

static std::set<unsigned int> feasible(const LP::LinearModel& model) {
  const std::size_t n=model.c.size();
  std::set<unsigned int> result;
  for (unsigned int mask=0; mask<(1U<<n); ++mask) {
    bool valid=true;
    for (std::size_t i=0; i<model.b.size() && valid; ++i) {
      I activity=0;
      for (std::size_t j=0; j<n; ++j)
        activity+=model.a[i*n+j]*static_cast<I>((mask>>j)&1U);
      valid=activity>=model.b[i];
    }
    if (valid) result.insert(mask);
  }
  return result;
}

static bool contains(const LP::LinearModel& model,const V& row,I rhs) {
  const auto n=model.c.size();
  for (std::size_t i=0; i<model.b.size(); ++i)
    if (model.b[i]==rhs &&
        std::equal(row.begin(),row.end(),model.a.begin()+i*n))
      return true;
  return false;
}

static bool rational_feasible(const LP::LinearModel& model,
                              const V& numerator,I denominator) {
  const auto n=model.c.size();
  for (I value : numerator)
    require(value>=0 && value<=denominator,"fractional witness outside binary box");
  for (std::size_t i=0; i<model.b.size(); ++i) {
    I activity=0;
    for (std::size_t j=0; j<n; ++j)
      activity+=model.a[i*n+j]*numerator[j];
    if (activity<model.b[i]*denominator) return false;
  }
  return true;
}

static S::Result equivalent(const LP::LinearModel& model,
                            const S::Options& options=S::Options()) {
  const auto strengthened=S::strengthen(model,options);
  LP::validate_model(strengthened.model);
  require(strengthened.model.c==model.c,"objective or variable mapping changed");
  require(feasible(strengthened.model)==feasible(model),"feasible binary set changed");
  require(strengthened.stats.rows_before==model.b.size(),"incorrect original row count");
  require(strengthened.stats.rows_after==strengthened.model.b.size(),"incorrect final row count");
  require(strengthened.stats.cuts_added()<=options.max_cuts,"cut cap exceeded");
  if (strengthened.stats.infeasible)
    require(feasible(model).empty(),"false infeasibility declaration");
  return strengthened;
}

static void cuts_off(const LP::LinearModel& original,const S::Result& strengthened,
                     const V& numerator,I denominator) {
  require(rational_feasible(original,numerator,denominator),
          "claimed fractional witness violates original rows");
  require(!rational_feasible(strengthened.model,numerator,denominator),
          "strengthening did not remove the fractional witness");
}

int main(void) {
  try {
    I out=17;
    require(!S::Detail::add(std::numeric_limits<I>::max(),1,out) && out==17,
            "positive addition overflow not rejected");
    require(!S::Detail::add(std::numeric_limits<I>::min(),-1,out) && out==17,
            "negative addition overflow not rejected");
    require(!S::Detail::subtract(std::numeric_limits<I>::min(),1,out) && out==17,
            "negative subtraction overflow not rejected");
    require(!S::Detail::subtract(std::numeric_limits<I>::max(),-1,out) && out==17,
            "positive subtraction overflow not rejected");
    require(S::Detail::add(std::numeric_limits<I>::min(),
                           std::numeric_limits<I>::max(),out) && out==-1,
            "valid extreme addition was rejected");

    S::Options normalize;
    normalize.max_cuts=0;
    auto result=equivalent({{6,10},{7},{-2,3}},normalize);
    require(contains(result.model,{3,5},4),"positive gcd ceiling is incorrect");
    result=equivalent({{-6,-10},{-7},{-2,3}},normalize);
    require(contains(result.model,{-3,-5},-3),"negative gcd ceiling is incorrect");
    result=equivalent({{2,4, 1,2, 1,2},{2,1,2},{-2,3}},normalize);
    require(result.model.b.size()==1 && contains(result.model,{1,2},2),
            "normalized duplicate/dominated rows not combined");
    require(result.stats.duplicates_removed==2,"wrong duplicate count");
    result=equivalent({{1,-2, 0,0},{-2,0},{-2,3}});
    require(result.model.b.empty(),"tautologies not removed");
    result=equivalent({{1,-2},{2},{-2,3}});
    require(result.stats.infeasible,"impossible row not recognized");
    equivalent({{}, {}, {}});

    S::Options pairs;
    pairs.fixings=false; pairs.cliques=false; pairs.covers=false;
    const LP::LinearModel pair_model{{-5,-5,-1},{-9},{-3,4,2}};
    result=equivalent(pair_model,pairs);
    require(contains(result.model,{-1,-1,0},-1),"binary pair conflict not posted");
    require(result.stats.pair_cuts>0,"pair cut counter did not increment");
    cuts_off(pair_model,result,{9,9,0},10);

    const LP::LinearModel triangle{
      {-1,-1,0, -1,0,-1, 0,-1,-1},{-1,-1,-1},{-3,4,2}};
    result=equivalent(triangle);
    require(contains(result.model,{-1,-1,-1},-1),"clique cut not posted");
    require(result.stats.clique_cuts>0,"clique cut counter did not increment");
    cuts_off(triangle,result,{1,1,1},2);

    const LP::LinearModel complemented_triangle{
      {1,-1,0, 1,0,1, 0,-1,1},{0,1,0},{4,-3,-2}};
    result=equivalent(complemented_triangle);
    require(contains(result.model,{1,-1,1},1),"complemented clique translation failed");
    cuts_off(complemented_triangle,result,{1,1,1},2);

    const LP::LinearModel cover{{-4,-4,-3},{-10},{2,-3,5}};
    result=equivalent(cover);
    require(contains(result.model,{-1,-1,-1},-2),"minimal cover cut not posted");
    require(result.stats.cover_cuts>0,"cover cut counter did not increment");
    cuts_off(cover,result,{3,3,2},3);

    const LP::LinearModel complemented_cover{{4,-4,3},{-3},{-4,3,2}};
    result=equivalent(complemented_cover);
    require(contains(result.model,{1,-1,1},0),"complemented cover translation failed");
    cuts_off(complemented_cover,result,{0,3,1},3);

    result=equivalent({{-2,-3},{-1},{-1,3}});
    require(contains(result.model,{-1,0},0) && contains(result.model,{0,-1},0),
            "overweight literals were not fixed");
    require(result.stats.fixing_cuts==2,"wrong fixing cut count");
    result=equivalent({{2,-3},{1,-1},{-1}});
    require(result.stats.infeasible,"opposite forced literals not recognized");

    bool rejected=false;
    try { S::strengthen({{1000000001LL},{1},{0}}); }
    catch (const std::invalid_argument&) { rejected=true; }
    require(rejected,"out-of-range input coefficient not rejected");

    std::mt19937 random(973321);
    std::size_t configurations=0, feasible_models=0, infeasible_models=0;
    std::size_t pair_cuts=0, clique_cuts=0, cover_cuts=0;
    constexpr unsigned int trials=2000;
    for (unsigned int trial=0; trial<trials; ++trial) {
      const std::size_t n=1+random()%8;
      const std::size_t m=random()%9;
      LP::LinearModel model;
      model.a.resize(n*m); model.b.resize(m); model.c.resize(n);
      const unsigned int witness=random()%(1U<<n);
      for (I& cost : model.c) cost=static_cast<int>(random()%21)-10;
      for (std::size_t i=0; i<m; ++i) {
        I activity=0;
        const I divisor=1+random()%4;
        for (std::size_t j=0; j<n; ++j) {
          model.a[i*n+j]=(static_cast<int>(random()%19)-9)*divisor;
          activity+=model.a[i*n+j]*static_cast<I>((witness>>j)&1U);
        }
        model.b[i]=(trial%2==0) ? activity-static_cast<I>(random()%8)
                                 : static_cast<int>(random()%41)-20;
      }
      if (feasible(model).empty()) ++infeasible_models;
      else ++feasible_models;
      auto full=equivalent(model);
      pair_cuts+=full.stats.pair_cuts;
      clique_cuts+=full.stats.clique_cuts;
      cover_cuts+=full.stats.cover_cuts;
      ++configurations;
      S::Options limited;
      limited.gcd=trial%3!=0;
      limited.max_cuts=trial%4;
      limited.max_graph_variables=trial%2 ? 0 : 128;
      limited.max_pair_checks=trial%5;
      limited.max_clique_size=2+trial%5;
      limited.max_cover_cuts_per_row=trial%3;
      limited.max_cover_terms=trial%2 ? 4 : 512;
      limited.max_cover_starts=trial%3;
      equivalent(model,limited);
      ++configurations;
      if (trial<20) {
        const auto again=S::strengthen(model);
        require(full.model.a==again.model.a && full.model.b==again.model.b,
                "strengthening is nondeterministic");
      }
    }
    require(feasible_models>=trials/2 && infeasible_models>0,
            "random suite lacks feasible/infeasible coverage");
    require(pair_cuts>0 && clique_cuts>0 && cover_cuts>0,
            "random suite failed to exercise all cut families");
    std::cout << "PASS strengthening edge cases, fractional witnesses, and "
              << configurations << " exhaustive configurations over " << trials
              << " models (" << feasible_models << " feasible, "
              << infeasible_models << " infeasible); "
              << pair_cuts << " pair, " << clique_cuts << " clique, "
              << cover_cuts << " cover cuts\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
