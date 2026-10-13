/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int.hh>
#include <gecode/search.hh>
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace Gecode;

// All costs and weights in this additive minimization example are nonnegative.
static std::int64_t add_cost(std::int64_t a, std::int64_t b) {
  if (a > std::numeric_limits<std::int64_t>::max()-b)
    throw std::overflow_error("additive cost overflow");
  return a+b;
}

static std::int64_t weighted_cost(std::int64_t g, std::int64_t h,
                                 std::int64_t weight) {
  if (h && weight > std::numeric_limits<std::int64_t>::max()/h)
    throw std::overflow_error("weighted remaining cost overflow");
  return add_cost(g,weight*h);
}

// Choose a route, then its final arc. Prefix costs are 0 and 9; remaining
// arc costs are [5,6] and [1,2]. All root alternatives remain nonterminal.
class Additive : public Space {
  static constexpr int prefix[2]={0,9};
  static constexpr int tail[2][2]={{5,6},{1,2}};
public:
  IntVar route, detail, objective;
  Additive(void)
    : route(*this,0,1), detail(*this,0,1), objective(*this,0,11) {
    TupleSet tuples(3);
    for (int r=0; r<2; r++)
      for (int d=0; d<2; d++)
        tuples.add(IntArgs({r,d,static_cast<int>(add_cost(prefix[r],tail[r][d]))}));
    tuples.finalize();
    extensional(*this,IntVarArgs({route,detail,objective}),tuples);
    branch(*this,route,INT_VALUES_MIN());
    branch(*this,detail,INT_VALUES_MIN());
  }
  Additive(Additive& s) : Space(s) {
    route.update(*this,s.route);
    detail.update(*this,s.detail);
    objective.update(*this,s.objective);
  }
  Space* copy(void) override { return new Additive(*this); }
  void constrain(const Space& s0) override {
    rel(*this,objective,IRT_LE,static_cast<const Additive&>(s0).objective.val());
  }
  // g counts determined arcs; after both decisions it is the full cost.
  std::int64_t g(void) const {
    if (!route.assigned()) return 0;
    return detail.assigned() ? objective.val() : prefix[route.val()];
  }
  // h bounds remaining arcs only. Restrictions may strengthen these bounds,
  // but the original cheapest arc remains a valid lower bound.
  std::int64_t h(void) const {
    if (!route.assigned())
      return std::min(add_cost(prefix[0],std::min(tail[0][0],tail[0][1])),
                      add_cost(prefix[1],std::min(tail[1][0],tail[1][1])));
    if (detail.assigned()) return 0;
    return std::min(tail[route.val()][0],tail[route.val()][1]);
  }
  static std::int64_t optimum(void) {
    std::int64_t best=std::numeric_limits<std::int64_t>::max();
    for (int r=0; r<2; r++)
      for (int d=0; d<2; d++)
        best=std::min(best,add_cost(prefix[r],tail[r][d]));
    return best;
  }
};

static bool run(const char* name, Search::HeuristicGuidance::Evaluate rank,
                const std::vector<int>& expected) {
  Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
    [](const Space& s0)->std::int64_t {
      const auto& s=static_cast<const Additive&>(s0);
      return add_cost(s.g(),s.h());
    },std::move(rank));
  Search::Options options; options.clone=false;
  std::unique_ptr<Search::Builder> builder(
    bestfirst<Additive>(guidance,Search::RANK_FIRST,options));
  std::unique_ptr<Search::Engine> engine((*builder)(new Additive));
  std::vector<int> costs;
  std::cout << name << " improvements:";
  while (std::unique_ptr<Additive> solution{
           static_cast<Additive*>(engine->next())}) {
    costs.push_back(solution->objective.val());
    std::cout << ' ' << costs.back();
  }
  std::cout << " status=" << (engine->stopped() ? "stopped" : "proven") << '\n';
  return costs==expected && costs.back()==Additive::optimum() && !engine->stopped();
}

int main(void) {
  const bool greedy=run("Greedy h",[](const Space& s)->std::int64_t {
    return static_cast<const Additive&>(s).h();
  },{10,5});
  const bool weighted=run("Fixed weight 2: g+2*h",[](const Space& s0)->std::int64_t {
    const auto& s=static_cast<const Additive&>(s0);
    return weighted_cost(s.g(),s.h(),2);
  },{5});
  return greedy && weighted ? 0 : 1;
}

// STATISTICS: example-any
