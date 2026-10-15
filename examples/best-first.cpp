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
#include <memory>

using namespace Gecode;

// Choose a route, then its final arc. All root alternatives need both decisions.
class Additive : public Space {
  static constexpr int prefix[2] = {0,9};
  static constexpr int tail[2][2] = {{5,6},{1,2}};
public:
  IntVar route, detail, objective;

  Additive(void)
    : route(*this,0,1), detail(*this,0,1), objective(*this,0,11) {
    TupleSet tuples(3);
    for (int r=0; r<2; r++)
      for (int d=0; d<2; d++)
        tuples.add(IntArgs({r,d,prefix[r]+tail[r][d]}));
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
  Space* copy(void) override {
    return new Additive(*this);
  }
  void constrain(const Space& incumbent) override {
    const Additive& best = static_cast<const Additive&>(incumbent);
    rel(*this,objective,IRT_LE,best.objective.val());
  }

  // Cost of the arcs already chosen.
  std::int64_t g(void) const {
    if (!route.assigned())
      return 0;
    return detail.assigned() ? objective.val() : prefix[route.val()];
  }

  // Lower bound on the cost of the arcs still to be chosen.
  std::int64_t h(void) const {
    if (!route.assigned())
      return std::min(prefix[0]+std::min(tail[0][0],tail[0][1]),
                      prefix[1]+std::min(tail[1][0],tail[1][1]));
    if (detail.assigned())
      return 0;
    return std::min(tail[route.val()][0],tail[route.val()][1]);
  }
};

static void
run(const char* name, Search::HeuristicGuidance::Evaluate rank) {
  Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
    [](const Space& space) -> std::int64_t {
      const Additive& s = static_cast<const Additive&>(space);
      return s.g()+s.h();
    },rank);
  std::unique_ptr<Search::Builder> builder(
    bestfirst<Additive>(guidance,Search::RANK_FIRST));
  Additive root;
  std::unique_ptr<Search::Engine> search((*builder)(&root));

  std::cout << name << " improvements:";
  while (std::unique_ptr<Space> solution{search->next()}) {
    const Additive& s = static_cast<const Additive&>(*solution);
    std::cout << ' ' << s.objective.val();
  }
  std::cout << (search->stopped() ? " (stopped)" : " (complete)") << '\n';
}

int
main(void) {
  run("Greedy h",[](const Space& space) -> std::int64_t {
    return static_cast<const Additive&>(space).h();
  });
  run("Weight 2: g+2*h",[](const Space& space) -> std::int64_t {
    const Additive& s = static_cast<const Additive&>(space);
    return s.g()+2*s.h();
  });
  return 0;
}

// STATISTICS: example-any
