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
#include <iostream>
#include <memory>

using namespace Gecode;

// Option zero is a shallow goal; the other options leave a second decision.
class Alternative : public Space {
public:
  IntVar option, detail, objective;

  Alternative(void)
    : option(*this,0,3), detail(*this,0,1), objective(*this,-7,4) {
    element(*this,IntArgs({4,-2,-7,1}),option,objective);
    TupleSet tuples(2);
    for (int i=0; i<4; i++) {
      tuples.add(IntArgs({i,0}));
      if (i != 0)
        tuples.add(IntArgs({i,1}));
    }
    tuples.finalize();
    extensional(*this,IntVarArgs({option,detail}),tuples);
    branch(*this,option,INT_VALUES_MIN());
    branch(*this,detail,INT_VALUES_MIN());
  }
  Alternative(Alternative& s) : Space(s) {
    option.update(*this,s.option);
    detail.update(*this,s.detail);
    objective.update(*this,s.objective);
  }
  Space* copy(void) override {
    return new Alternative(*this);
  }
  void constrain(const Space& incumbent) override {
    const Alternative& best = static_cast<const Alternative&>(incumbent);
    rel(*this,objective,IRT_LE,best.objective.val());
  }
};

static void
run(const char* name, Search::Base<Alternative>& search) {
  std::cout << name << " improvements:";
  while (std::unique_ptr<Alternative> solution{search.next()})
    std::cout << ' ' << solution->objective.val();
  std::cout << (search.stopped() ? " (stopped)" : " (complete)") << '\n';
}

int
main(void) {
  Alternative root;
  Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
    [](const Space& space) -> std::int64_t {
      const Alternative& s = static_cast<const Alternative&>(space);
      return s.objective.min();
    },
    [](const Space& space) -> std::int64_t {
      const Alternative& s = static_cast<const Alternative&>(space);
      // Deliberately rank the optimal option last to show beam widening.
      return s.option.assigned() && s.option.val() == 2 ? 1000 : 0;
    });

  AStar<Alternative> astar(&root,guidance);
  run("A*",astar);

  BAB<Alternative> bab(&root);
  run("BAB",bab);

  CABS<Alternative> cabs(&root,guidance);
  run("CABS",cabs);

  // Width one finds improvements but cannot prove the best one optimal.
  CABS<Alternative> beam(&root,guidance,Search::Options::def,
                         Search::CABSOptions(1));
  run("CABS (maximum width 1)",beam);

  return 0;
}

// STATISTICS: example-any
