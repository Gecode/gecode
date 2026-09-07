#include <gecode/optimize/solve.hpp>
#include <gecode/optimize/validate.hpp>
#include <gecode/optimize/constraints.hpp>
#include <gecode/optimize/workflow.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>

namespace O = Gecode::Optimize;
constexpr double inf = std::numeric_limits<double>::infinity();
static void close(double a, double b) { assert(std::abs(a-b) < 1e-6); }

int main() {
  O::Model basic;
  auto x = basic.add_continuous(0, 100, "x");
  basic.add_row({{x, 1}}, 4, inf, "demand");
  basic.minimize({{x, 2}}, -3);
  O::SolveOptions options;
  options.guarantee = O::Guarantee::Certified;
  assert(O::solve(basic, options).termination == O::Termination::Unsupported);
  options.guarantee = O::Guarantee::Numerical;
  options.time_limit_seconds = 0;
  assert(O::solve(basic, options).termination == O::Termination::TimeLimit);
  options.time_limit_seconds = inf;
  options.cancellation = std::make_shared<O::CancellationToken>();
  options.cancellation->cancel();
  assert(O::solve(basic, options).termination == O::Termination::Cancelled);
  options = {};
  auto malformed = basic.snapshot();
  malformed.rows[0].terms[0].variable.model_id = 0;
  assert(O::solve(malformed).termination == O::Termination::InvalidModel);
  O::Model moved_from;
  O::Model moved_to(std::move(moved_from));
  assert(O::solve(moved_from).termination==O::Termination::InvalidModel);
  if (!O::capabilities().available) {
    assert(O::solve(basic).termination == O::Termination::Unsupported);
    std::cout << "backend-unavailable contract passed\n";
    return 0;
  }
  auto result = O::solve(basic);
  assert(result.termination == O::Termination::Optimal && result.has_solution());
  close(result.value(x), 4); close(*result.objective, 5);
  assert(result.backend == "HiGHS" && !result.backend_version.empty());
  assert(result.guarantee == O::Guarantee::Numerical);
  const auto old_revision = result.revision;
  basic.set_bounds(x, 5, 100);
  auto edited = O::solve(basic);
  close(*edited.objective, 7); close(result.value(x), 4);
  assert(edited.revision > old_revision);
  basic.maximize({{x, -2}}, 3);
  auto maximum = O::solve(basic);
  close(*maximum.objective, -7); close(maximum.value(x), 5);
  if (maximum.relative_gap) close(*maximum.relative_gap, 0);

  O::Model mixed;
  auto open = mixed.add_binary("open");
  auto q = mixed.add_continuous(0, 100, "quantity");
  auto count = mixed.add_integer(0, 4, "count");
  mixed.add_row({{q,1},{open,-100}}, -inf, 0, "capacity");
  mixed.add_row({{q,1},{count,3}}, 43, inf, "demand");
  mixed.minimize({{open,12},{q,0.5},{count,2}});
  auto mip = O::solve(mixed);
  assert(mip.termination == O::Termination::Optimal && mip.has_solution());
  close(*mip.objective, 33.5);
  assert(O::validate(mixed.snapshot(), mip.values).valid);

  // Sparse starts guide the solver without fixing the supplied variables.
  options.primal_start = {{open,1}};
  auto started = O::solve(mixed,options);
  assert(started.start_submitted && started.has_solution());
  close(*started.objective,33.5);
  options.primal_start = {{open,1},{q,50},{count,0}};
  started = O::solve(mixed,options);
  assert(started.start_submitted && started.has_solution());
  close(*started.objective,33.5);
  options.primal_start = {{open,0},{q,0},{count,0}};
  assert(O::solve(mixed,options).termination==O::Termination::InvalidModel);
  options.primal_start = {{open,1},{open,1}};
  assert(O::solve(mixed,options).termination==O::Termination::InvalidModel);
  options.primal_start = {{x,5}};
  assert(O::solve(mixed,options).termination==O::Termination::InvalidModel);
  options.primal_start = {{count,1.5}};
  assert(O::solve(mixed,options).termination==O::Termination::InvalidModel);
  options.primal_start = {{q,std::numeric_limits<double>::quiet_NaN()}};
  assert(O::solve(mixed,options).termination==O::Termination::InvalidModel);
  options = {};
  options.threads=2;
  assert(O::solve(mixed,options).termination==O::Termination::Unsupported);
  options = {};

  // Cross-feature conformance: typed indicators retain their original meaning
  // through native backend presolve, primal starts and multiobjective locks.
  O::Model logical;
  auto enabled=logical.add_binary("enabled");
  auto amount=logical.add_continuous(0,10,"amount");
  auto lower=O::add_indicator(logical,enabled,true,{{amount,1}},7,inf);
  O::add_indicator(logical,enabled,false,{{amount,1}},-inf,3);
  logical.minimize({{enabled,-20},{amount,1}});
  options.primal_start={{enabled,0},{amount,0}};
  auto lr=O::solve(logical,options);
  assert(lr.has_solution() && lr.start_submitted);
  close(lr.value(enabled),1); close(lr.value(amount),7); close(*lr.objective,-13);
  O::ObjectiveData first{{{enabled,1}},0,O::ObjectiveSense::Maximize};
  O::ObjectiveData second{{{amount,1}},0,O::ObjectiveSense::Minimize};
  auto lex=O::solve_lexicographic(logical,{{first,0,0,"activate"},{second,0,0,"quantity"}},options);
  assert(lex.completed_numerically() && lex.has_solution());
  close(lex.final_solution.value(amount),7);
  O::remove_indicator(logical,lower.indicator);
  options={};
  lr=O::solve(logical);
  assert(lr.has_solution()); close(*lr.objective,-20);

  O::Model infeasible;
  auto z = infeasible.add_integer(0, 0.5);
  infeasible.add_row({{z,1}}, 0.25, inf);
  assert(O::solve(infeasible).termination == O::Termination::Infeasible);
  O::Model unbounded;
  auto free = unbounded.add_continuous(-inf,inf);
  unbounded.minimize({{free,1}});
  auto unb = O::solve(unbounded);
  assert(unb.termination == O::Termination::Unbounded || unb.termination == O::Termination::InfeasibleOrUnbounded);
  O::Model empty;
  empty.minimize({},7);
  auto emp = O::solve(empty); assert(emp.has_solution()); close(*emp.objective,7);
  empty.add_row({},1,inf);
  assert(O::solve(empty).termination == O::Termination::Infeasible);
  options.time_limit_seconds=0;
  assert(O::solve(empty,options).termination==O::Termination::TimeLimit);
  options = {};

  O::Model deleted;
  auto dead = deleted.add_binary();
  deleted.remove(dead);
  auto live = deleted.add_integer(2,5);
  deleted.minimize({{live,1}});
  auto del = O::solve(deleted); close(del.value(live),2);
  bool threw = false; try { del.value(dead); } catch (const O::ModelError&) { threw=true; }
  assert(threw && std::isnan(del.values[dead.id]));

  O::Model semi;
  auto s = semi.add_variable(O::VariableType::SemiContinuous,3,8,"semi");
  semi.add_row({{s,1}},1,inf);
  semi.minimize({{s,1}});
  auto sr = O::solve(semi); assert(sr.has_solution()); close(sr.value(s),3);
  semi.set_bounds(semi.snapshot().rows[0].constraint,0,inf);
  sr=O::solve(semi); assert(sr.has_solution()); close(sr.value(s),0);
  options.primal_start={{s,0}};
  sr=O::solve(semi,options);
  assert(sr.has_solution() && sr.start_submitted); close(sr.value(s),0);
  auto semi_other=semi.add_binary("other");
  sr=O::solve(semi,options); // A zero-valued semi variable in a partial start.
  assert(sr.has_solution() && sr.start_submitted); close(sr.value(s),0);
  options.primal_start={{s,0},{semi_other,0}};
  sr=O::solve(semi,options); assert(sr.has_solution() && sr.start_submitted);
  options={};
  O::Model semi_integer;
  auto si=semi_integer.add_variable(O::VariableType::SemiInteger,3,8);
  semi_integer.minimize({{si,1}});
  options.primal_start={{si,0}};
  sr=O::solve(semi_integer,options);
  assert(sr.has_solution() && sr.start_submitted); close(sr.value(si),0);
  options={};

  O::Model tiny;
  auto t = tiny.add_continuous(0,1);
  tiny.add_row({{t,1e-14}},0,1);
  assert(O::solve(tiny).termination == O::Termination::Unsupported);

  // Differential tiny MILPs: exhaustive integer choice plus analytic recourse.
  std::mt19937 rng(1701);
  for (int iteration=0; iteration<30; ++iteration) {
    const int demand=1+static_cast<int>(rng()%12), fixed=1+static_cast<int>(rng()%7);
    const int rate=1+static_cast<int>(rng()%4);
    O::Model problem;
    auto n=problem.add_integer(0,5), y=problem.add_continuous(0,20);
    problem.add_row({{n,3},{y,1}},demand,inf);
    problem.minimize({{n,static_cast<double>(fixed)},{y,static_cast<double>(rate)}},-10);
    double oracle=inf;
    for (int i=0;i<=5;++i) oracle=std::min(oracle,-10.0+fixed*i+rate*std::max(0,demand-3*i));
    auto r=O::solve(problem); assert(r.termination==O::Termination::Optimal && r.has_solution());
    close(*r.objective,oracle);
  }

  const auto temp = std::filesystem::temp_directory_path()/
    ("gecode-optimize-test-"+std::to_string(std::random_device{}()));
  std::filesystem::create_directory(temp);
  for (const auto extension : {".lp",".mps"}) {
    auto path=(temp/(std::string("roundtrip")+extension)).string();
    O::write_model(mixed,path);
    auto restored=O::read_model(path);
    auto r=O::solve(restored); assert(r.has_solution()); close(*r.objective,*mip.objective);
    assert(restored.snapshot().rows.size()==mixed.snapshot().rows.size());
  }
  auto qp=(temp/"quadratic.lp").string();
  { std::ofstream file(qp); file << "Minimize\n obj: [ x ^ 2 ] / 2\nSubject To\n c: x >= 1\nEnd\n"; }
  threw=false; try { auto ignored=O::read_model(qp); } catch (const std::exception&) { threw=true; }
  assert(threw);
  std::filesystem::remove_all(temp);
  std::cout << "LP/MILP integration, original validation, limits and I/O passed\n";
}
