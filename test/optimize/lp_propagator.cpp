// End-to-end correctness of the redundant LP bound actor.
// The oracle enumerates binary assignments independently of both solvers.
#include <gecode/minimodel/lp-relaxation.hpp>
#include <gecode/search.hh>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

namespace LP = Gecode::Experimental::LpRelaxation;
using I = std::int64_t;
using V = std::vector<I>;

static void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

enum class Mode { Native, Root, EveryNode, RootFix, EveryFix, ThrottledFix };

struct Instance {
  LP::LinearModel model;
  V lower, upper;
  bool sparse=false;
};

static I objective(const Instance& instance, unsigned int mask) {
  I result=0;
  for (std::size_t j=0; j<instance.model.c.size(); ++j)
    result+=instance.model.c[j]*static_cast<I>((mask>>j)&1U);
  return result;
}

static std::set<unsigned int> enumerate(const Instance& instance) {
  const auto& model=instance.model;
  const std::size_t n=model.c.size();
  std::set<unsigned int> result;
  for (unsigned int mask=0; mask<(1U<<n); ++mask) {
    bool feasible=true;
    for (std::size_t j=0; j<n; ++j) {
      const I x=(mask>>j)&1U;
      if (x<instance.lower[j] || x>instance.upper[j])
        feasible=false;
    }
    for (std::size_t i=0; feasible && i<model.b.size(); ++i) {
      I activity=0;
      for (std::size_t j=0; j<n; ++j)
        activity+=model.a[i*n+j]*static_cast<I>((mask>>j)&1U);
      if (activity<model.b[i])
        feasible=false;
    }
    if (feasible)
      result.insert(mask);
  }
  return result;
}

static int objective_endpoint(const LP::LinearModel& model, bool high) {
  I result=0;
  for (I cost : model.c)
    result+=high ? std::max<I>(0,cost) : std::min<I>(0,cost);
  return static_cast<int>(result);
}

class Problem : public Gecode::Space {
public:
  Gecode::IntVarArray x;
  Gecode::IntVar z;

  Problem(const Instance& instance, Mode mode,
          const std::shared_ptr<LP::SparseBackend>& backend)
    : x(*this,static_cast<int>(instance.model.c.size()),0,1),
      z(*this,objective_endpoint(instance.model,false),
              objective_endpoint(instance.model,true)) {
    for (int j=0; j<x.size(); ++j)
      Gecode::dom(*this,x[j],static_cast<int>(instance.lower[j]),
                             static_cast<int>(instance.upper[j]));
    if (mode==Mode::Native) {
      if (instance.sparse) LP::post_native(*this,x,z,backend->model);
      else LP::post_native(*this,x,z,instance.model);
    } else {
      LP::Options policy;
      policy.frequency=(mode==Mode::Root || mode==Mode::RootFix)
        ? LP::Frequency::Root : LP::Frequency::EveryNode;
      policy.reduced_cost_fixing=mode==Mode::RootFix || mode==Mode::EveryFix || mode==Mode::ThrottledFix;
      policy.assignment_interval=mode==Mode::ThrottledFix ? 4 : 1;
      LP::binary_linear_minimize(*this,x,z,backend,policy);
    }
    Gecode::branch(*this,x,Gecode::INT_VAR_NONE(),Gecode::INT_VAL_MIN());
  }
  Problem(Problem& other) : Gecode::Space(other) {
    x.update(*this,other.x);
    z.update(*this,other.z);
  }
  Gecode::Space* copy(void) override { return new Problem(*this); }
  void constrain(const Gecode::Space& best) override {
    const auto& other=static_cast<const Problem&>(best);
    Gecode::rel(*this,z,Gecode::IRT_LE,other.z.val());
  }
  unsigned int mask(void) const {
    unsigned int result=0;
    for (int j=0; j<x.size(); ++j) {
      require(x[j].assigned(),"unassigned variable in returned solution");
      result|=static_cast<unsigned int>(x[j].val())<<j;
    }
    return result;
  }
};

static void check_dfs(const Instance& instance, Mode mode,
                      const std::shared_ptr<LP::SparseBackend>& backend,
                      const Gecode::Search::Options& options,
                      const std::set<unsigned int>& expected) {
  {
    Problem* root=new Problem(instance,mode,backend);
    Gecode::DFS<Problem> engine(root,options);
    delete root;
    std::set<unsigned int> actual;
    while (Problem* raw=engine.next()) {
      std::unique_ptr<Problem> solution(raw);
      const unsigned int assignment=solution->mask();
      require(expected.count(assignment)==1,"DFS returned infeasible assignment");
      require(solution->z.assigned(),"DFS objective is not assigned");
      require(solution->z.val()==objective(instance,assignment),
              "DFS returned an incorrect objective");
      require(actual.insert(assignment).second,"DFS returned duplicate assignment");
    }
    require(!engine.stopped(),"DFS did not complete");
    require(actual==expected,"LP actor changed the complete solution set");
  }
  require(backend.use_count()==1,"backend retained after DFS destruction");
}

static void check_bab(const Instance& instance, Mode mode,
                      const std::shared_ptr<LP::SparseBackend>& backend,
                      const Gecode::Search::Options& options,
                      const std::set<unsigned int>& expected) {
  I optimum=std::numeric_limits<I>::max();
  for (unsigned int mask : expected)
    optimum=std::min(optimum,objective(instance,mask));
  {
    Problem* root=new Problem(instance,mode,backend);
    Gecode::BAB<Problem> engine(root,options);
    delete root;
    I best=std::numeric_limits<I>::max();
    bool found=false;
    while (Problem* raw=engine.next()) {
      std::unique_ptr<Problem> solution(raw);
      const unsigned int assignment=solution->mask();
      require(expected.count(assignment)==1,"BAB returned infeasible assignment");
      const I candidate=objective(instance,assignment);
      require(solution->z.assigned() && solution->z.val()==candidate,
              "BAB returned an incorrect objective");
      require(!found || candidate<best,"BAB incumbent did not strictly improve");
      best=candidate;
      found=true;
    }
    require(!engine.stopped(),"BAB did not complete");
    require(found==!expected.empty(),"BAB feasibility differs from brute force");
    require(!found || best==optimum,"BAB optimum differs from brute force");
  }
  require(backend.use_count()==1,"backend retained after BAB destruction");
}

static std::vector<Instance> instances(void) {
  std::vector<Instance> result;
  // A triangle cover needs two binary variables but has LP optimum 1.5.
  result.push_back({{{1,1,0, 0,1,1, 1,0,1},{1,1,1},{1,1,1}},
                    {0,0,0},{1,1,1}});
  // Signed objective: independent set on a triangle has LP -1.5, integer -1.
  result.push_back({{{-1,-1,0, 0,-1,-1, -1,0,-1},{-1,-1,-1},{-1,-1,-1}},
                    {0,0,0},{1,1,1}});
  // Contradictory equalities whose individual rows do not initially fix x.
  result.push_back({{{1,1, -1,-1, 1,-1, -1,1},{1,-1,0,0},{-2,3}},
                    {0,0},{1,1}});
  result.push_back({{{1},{2},{-1}},{0},{1}}); // Immediately infeasible.
  result.push_back({{{},{},{-3,0,2}},{0,0,0},{1,1,1}}); // No rows.
  result.push_back({{{1,-1,2},{0},{-3,4,-2}},{1,0,1},{1,0,1}}); // All fixed.
  result.push_back({{{0,0},{1},{-1,2}},{0,0},{1,1}}); // Empty impossible row.
  result.push_back({{{},{},{}},{},{}}); // Empty feasible model.

  std::mt19937 random(289417);
  while (result.size()<120) {
    const std::size_t id=result.size();
    const std::size_t n=3+random()%5;
    const std::size_t m=1+random()%6;
    Instance instance;
    auto& model=instance.model;
    model.a.resize(n*m);
    model.b.resize(m);
    model.c.resize(n);
    instance.lower.resize(n);
    instance.upper.resize(n);
    V witness(n);
    for (std::size_t j=0; j<n; ++j) {
      const unsigned int domain=random()%8;
      instance.lower[j]=(domain==0) ? 1 : 0;
      instance.upper[j]=(domain==1) ? 0 : 1;
      witness[j]=(instance.lower[j]==instance.upper[j])
        ? instance.lower[j] : random()%2;
      model.c[j]=static_cast<int>(random()%17)-8;
    }
    for (std::size_t i=0; i<m; ++i) {
      I activity=0;
      for (std::size_t j=0; j<n; ++j) {
        model.a[i*n+j]=static_cast<int>(random()%11)-5;
        activity+=model.a[i*n+j]*witness[j];
      }
      model.b[i]=(id%2==0) ? activity-static_cast<int>(random()%4)
                           : static_cast<int>(random()%17)-8;
    }
    result.push_back(std::move(instance));
  }
  return result;
}

static void check_lifecycle_and_root_bounds(const std::vector<Instance>& cases) {
  for (std::size_t id : {std::size_t(0),std::size_t(1)}) {
    std::shared_ptr<LP::SparseBackend> backend=std::make_shared<LP::Backend>(cases[id].model);
    std::weak_ptr<LP::SparseBackend> weak=backend;
    const int expected=id==0 ? 2 : -1;
    for (Mode mode : {Mode::Root,Mode::EveryNode}) {
      {
        std::unique_ptr<Problem> root(new Problem(cases[id],mode,backend));
        require(backend.use_count()>1,"LP actor did not retain its backend");
      } // Destruction before the first propagation call.
      require(backend.use_count()==1,"unpropagated actor leaked its backend");
      {
        std::unique_ptr<Problem> root(new Problem(cases[id],mode,backend));
        require(root->status()!=Gecode::SS_FAILED,"feasible root failed");
        require(root->z.min()==expected,"LP root certificate did not tighten objective");
        std::unique_ptr<Problem> clone(static_cast<Problem*>(root->clone()));
        require(clone->status()!=Gecode::SS_FAILED,"feasible cloned root failed");
      }
      require(backend.use_count()==1,"cloned actor leaked its backend");
      {
        Gecode::Search::Options options;
        options.threads=1;
        options.c_d=32;
        options.a_d=16;
        Problem* root=new Problem(cases[id],mode,backend);
        Gecode::DFS<Problem> engine(root,options);
        delete root;
        std::unique_ptr<Problem> first(engine.next());
        require(first!=nullptr,"early-stop search found no solution");
      } // Unexplored sibling spaces are destroyed here.
      require(backend.use_count()==1,"early-stop search leaked its backend");
    }
    require(backend->statistics().lp_calls>0,"no LP calls reached the backend");
    require(backend->statistics().valid_bounds>0,"no bound was certified");
    backend.reset();
    require(weak.expired(),"backend lifetime outlived all its owners");
  }
}

int main(void) {
  try {
    const auto cases=instances();
    check_lifecycle_and_root_bounds(cases);
    unsigned int runs=0, feasible=0, infeasible=0;
    std::uint64_t lp_calls=0, certified=0;
    for (const Instance& original : cases) for (bool sparse : {false,true}) {
      Instance instance=original; instance.sparse=sparse;
      const auto expected=enumerate(instance);
      if (expected.empty()) ++infeasible; else ++feasible;
      std::shared_ptr<LP::SparseBackend> backend;
      if (sparse) backend=std::make_shared<LP::SparseBackend>(LP::sparse_model(instance.model));
      else backend=std::make_shared<LP::Backend>(instance.model);
      std::weak_ptr<LP::SparseBackend> weak=backend;
      // Reuse one backend across independent searches, after traversing
      // siblings with opposite bound fixings and different recomputation.
      for (unsigned int distance : {1U,8U,32U})
        for (Mode mode : {Mode::Native,Mode::Root,Mode::EveryNode,
                          Mode::RootFix,Mode::EveryFix,Mode::ThrottledFix}) {
          Gecode::Search::Options options;
          options.threads=1;
          options.c_d=distance;
          options.a_d=16;
          check_dfs(instance,mode,backend,options,expected);
          check_bab(instance,mode,backend,options,expected);
          runs+=2;
        }
      lp_calls+=backend->statistics().lp_calls;
      certified+=backend->statistics().valid_bounds;
      backend.reset();
      require(weak.expired(),"backend leaked after completing an instance");
    }
    require(feasible>0 && infeasible>0,"both feasibility outcomes must be tested");
    require(lp_calls>0 && certified>0,"LP integration was not exercised");
    std::cout << "PASS " << cases.size() << " models in two storage forms (" << feasible
              << " feasible representations, " << infeasible << " infeasible representations), " << runs
              << " exhaustive dense/sparse native/bounds/fixing/throttled DFS/BAB configurations; "
              << lp_calls << " LP calls, " << certified
              << " certified bounds; backend lifecycle and root tightening\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
