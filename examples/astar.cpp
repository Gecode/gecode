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
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>

using namespace Gecode;

// A bounded closed model: option zero is a shallow goal, while the other
// options retain a second decision. Rank deliberately disfavors the optimum.
class Alternative : public Space {
public:
  IntVar option, detail, objective;
  Alternative(void)
    : option(*this,0,3), detail(*this,0,1), objective(*this,-7,4) {
    element(*this,IntArgs({4,-2,-7,1}),option,objective);
    TupleSet tuples(2);
    for (int i=0;i<4;i++) {
      tuples.add(IntArgs({i,0}));
      if (i) tuples.add(IntArgs({i,1}));
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
  Space* copy(void) override { return new Alternative(*this); }
  SpaceComparison compare(const Space& s0) const override {
    const auto& s = dynamic_cast<const Alternative&>(s0);
    if (objective.val() == s.objective.val()) return SC_EQUIVALENT;
    return objective.val() < s.objective.val() ? SC_BETTER : SC_WORSE;
  }
  void constrain(const Space& s0) override {
    const auto& s = static_cast<const Alternative&>(s0);
    rel(*this,objective,IRT_LE,s.objective.val());
  }
};

// Use one model and one ordinary options value for all engines. The tiny
// elapsed times illustrate measurement output; they do not establish speedups.
static bool run(const char* name, Search::Engine* raw,
                unsigned long int width=0, unsigned long int frontier=0) {
  std::unique_ptr<Search::Engine> engine(raw);
  const auto start=std::chrono::steady_clock::now();
  int last=0;
  bool found=false;
  std::cout << name << " improvements:";
  while (std::unique_ptr<Alternative> s{
           static_cast<Alternative*>(engine->next())}) {
    last=s->objective.val();
    found=true;
    std::cout << ' ' << last;
  }
  const double milliseconds=std::chrono::duration<double,std::milli>(
    std::chrono::steady_clock::now()-start).count();
  const auto statistics=engine->statistics();
  std::cout << "\n" << name << " width=" << width << " frontier=" << frontier
            << " objective=";
  if (found) std::cout << last; else std::cout << "none";
  std::cout << " status=" << (engine->stopped() ? "stopped" : "proven")
            << " ms=" << std::fixed << std::setprecision(3) << milliseconds
            << " nodes=" << statistics.node << " restarts=" << statistics.restart
            << '\n';
  if (width==1) return found && last==-2 && engine->stopped();
  if (frontier==1) return found && last==4 && engine->stopped();
  return found && last==-7 && !engine->stopped();
}

int main(void) {
  Search::HeuristicGuidance guidance(Search::HeuristicGuidance::MINIMIZE,
    [](const Space& s) -> std::int64_t {
      return static_cast<const Alternative&>(s).objective.min();
    },
    [](const Space& s) -> std::int64_t {
      const auto& a=static_cast<const Alternative&>(s);
      return a.option.assigned() && a.option.val()==2 ? 1000 : 0;
    });
  Search::Options options;
  options.clone=false;
  bool valid=run("A*",Search::astarengine(new Alternative,guidance,options));
  for (const auto limits : {Search::ColumnOptions(), Search::ColumnOptions(1,0)})
    valid=run(limits.quota_growth ? "Column" : "CBFS",
              Search::columnengine(new Alternative,guidance,options,limits)) && valid;
  valid=run("BAB",Search::babengine(new Alternative,options)) && valid;
  for (const auto limits : {Search::CABSOptions(1,0), Search::CABSOptions(2,0),
                            Search::CABSOptions(4,0), Search::CABSOptions(),
                            Search::CABSOptions(0,1), Search::CABSOptions(0,4)})
    valid=run("CABS",Search::cabsengine(new Alternative,guidance,options,limits),
              limits.max_width,limits.frontier_limit) && valid;
  return valid ? 0 : 1;
}
