// Standalone correctness checks for propagation-count checkpoints.
#include <gecode/int.hh>
#include <gecode/search.hh>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <vector>

using namespace Gecode;

static void require(bool condition, const char* message) {
  if (!condition)
    throw std::runtime_error(message);
}

class Queens : public Space {
public:
  IntVarArray q;
  Queens(int n, bool nary) : q(*this,n,0,n-1) {
    IntArgs up(n), down(n);
    for (int i=0; i<n; ++i) {
      up[i]=i;
      down[i]=-i;
    }
    distinct(*this,q);
    distinct(*this,up,q);
    distinct(*this,down,q);
    branch(*this,q,INT_VAR_NONE(),nary ? INT_VALUES_MIN() : INT_VAL_MIN());
  }
  Queens(Queens& s) : Space(s) { q.update(*this,s.q); }
  Space* copy(void) override { return new Queens(*this); }
  std::vector<int> values(void) const {
    std::vector<int> result(q.size());
    for (int i=0; i<q.size(); ++i)
      result[i]=q[i].val();
    return result;
  }
};

static std::set<std::vector<int>> queens_expected(int n) {
  std::set<std::vector<int>> expected;
  std::vector<int> permutation(n);
  for (int i=0; i<n; ++i)
    permutation[i]=i;
  do {
    bool valid=true;
    for (int i=0; i<n && valid; ++i)
      for (int j=i+1; j<n; ++j)
        if (std::abs(permutation[i]-permutation[j]) == j-i)
          valid=false;
    if (valid)
      expected.insert(permutation);
  } while (std::next_permutation(permutation.begin(),permutation.end()));
  return expected;
}

static const int item_count=12;
static const int item_weights[item_count]={2,3,5,7,9,4,6,8,11,13,10,12};
static const int item_values[item_count]={6,7,11,16,20,9,14,17,25,29,22,27};
static const int capacity=35;

class Knapsack : public Space {
public:
  IntVarArray x;
  IntVar objective;
  bool maximize;
  explicit Knapsack(bool maximum)
    : x(*this,item_count,0,1), objective(*this,0,300), maximize(maximum) {
    IntArgs w(item_count), v(item_count);
    for (int i=0; i<item_count; ++i) { w[i]=item_weights[i]; v[i]=item_values[i]; }
    linear(*this,w,x,maximize ? IRT_LQ : IRT_GQ,capacity);
    linear(*this,v,x,IRT_EQ,objective);
    // Small values first force BAB to improve multiple incumbents for max.
    branch(*this,x,INT_VAR_NONE(),INT_VAL_MIN());
  }
  Knapsack(Knapsack& s) : Space(s), maximize(s.maximize) {
    x.update(*this,s.x);
    objective.update(*this,s.objective);
  }
  Space* copy(void) override { return new Knapsack(*this); }
  void constrain(const Space& b) override {
    const Knapsack& best=static_cast<const Knapsack&>(b);
    rel(*this,objective,maximize ? IRT_GR : IRT_LE,best.objective.val());
  }
};

static int knapsack_expected(bool maximize) {
  int best=maximize ? -1 : std::numeric_limits<int>::max();
  for (unsigned int mask=0; mask<(1U<<item_count); ++mask) {
    int w=0, v=0;
    for (int i=0; i<item_count; ++i)
      if (mask & (1U<<i)) { w+=item_weights[i]; v+=item_values[i]; }
    if ((maximize && w<=capacity) || (!maximize && w>=capacity))
      best=maximize ? std::max(best,v) : std::min(best,v);
  }
  return best;
}

int main(void) {
  try {
    require(Search::Options().c_p == 0,"checkpointing must default off");
    const unsigned long thresholds[]={0,1,10};
    const unsigned int distances[]={1,8,32};
    const unsigned int adaptive[]={2,16};
    const int sizes[]={3,4,6,7};
    unsigned int runs=0;
    std::vector<unsigned long int> disabled_propagations;
    bool checkpoint_policy_exercised=false;
    for (unsigned long threshold : thresholds) {
      unsigned int profile_index=0;
      for (unsigned int distance : distances)
        for (unsigned int adapt : adaptive) {
          Search::Options options;
          options.threads=1;
          options.c_p=threshold;
          options.c_d=distance;
          options.a_d=adapt;
          for (int size : sizes)
            for (bool nary : {false,true}) {
              const auto expected=queens_expected(size);
              Queens* root=new Queens(size,nary);
              DFS<Queens> engine(root,options);
              delete root;
              std::set<std::vector<int>> actual;
              while (Queens* solution=engine.next()) {
                const auto assignment=solution->values();
                delete solution;
                require(actual.insert(assignment).second,"duplicate DFS solution");
              }
              require(!engine.stopped(),"DFS stopped before exhaustive completion");
              require(actual == expected,"DFS solution set differs from brute force");
              const unsigned long int propagations=engine.statistics().propagate;
              if (threshold == 0)
                disabled_propagations.push_back(propagations);
              else if (propagations != disabled_propagations[profile_index])
                checkpoint_policy_exercised=true;
              ++profile_index;
              ++runs;
            }
          for (bool maximum : {false,true}) {
            Knapsack* root=new Knapsack(maximum);
            BAB<Knapsack> engine(root,options);
            delete root;
            int best=maximum ? -1 : std::numeric_limits<int>::max();
            while (Knapsack* solution=engine.next()) {
              int w=0, v=0;
              for (int i=0; i<item_count; ++i) {
                w+=item_weights[i]*solution->x[i].val();
                v+=item_values[i]*solution->x[i].val();
              }
              const int candidate=solution->objective.val();
              delete solution;
              require(maximum ? w<=capacity : w>=capacity,"infeasible BAB solution");
              require(candidate == v,"incorrect BAB objective");
              require(maximum ? candidate>best : candidate<best,
                      "BAB did not improve its incumbent");
              best=candidate;
            }
            require(!engine.stopped(),"BAB stopped before proving optimality");
            require(best == knapsack_expected(maximum),"BAB optimum differs from brute force");
            ++runs;
          }
        }
    }
    require(checkpoint_policy_exercised,"checkpoint policy did not affect any replay profile");
    std::cout << "PASS " << runs << " exhaustive DFS/BAB configurations\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
