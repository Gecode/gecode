#ifdef CHECK_NATIVE
#include <gecode/int.hh>
class NativeModel : public Gecode::Space {
public:
  Gecode::IntVar x;
  NativeModel() : x(*this,0,1) {}
  NativeModel(NativeModel& other) : Gecode::Space(other) { x.update(*this,other.x); }
  Gecode::Space* copy() override { return new NativeModel(*this); }
};
#endif
#ifndef NATIVE_ONLY
#include <gecode/optimize.hh>
#include <cmath>
#include <limits>
#endif

int main() {
#ifdef CHECK_NATIVE
  NativeModel native;
  Gecode::rel(native,native.x,Gecode::IRT_EQ,1);
  if (native.status()==Gecode::SS_FAILED || !native.x.assigned() || native.x.val()!=1) return 1;
#endif
#ifndef NATIVE_ONLY
  Gecode::Optimize::Model model;
  auto x=model.add_variables({{Gecode::Optimize::VariableType::Integer,2,5,"x"}}).front();
  model.add_rows({{{{x,1}},2,5,"bounds"}});
  Gecode::Optimize::SparseRowBatch sparse;
  sparse.columns={x};sparse.row_start={0,1};sparse.column={0};sparse.coefficient={1};sparse.lower={2};sparse.upper={5};
  model.add_rows_sparse(sparse);
  model.minimize({{x,3}});
  auto prepared=Gecode::Optimize::presolve_integer(model);
  if (!prepared.model || prepared.status!=Gecode::Optimize::PresolveStatus::Fixpoint
      || prepared.model->original().model_id!=model.id()) return 7;
  auto result=Gecode::Optimize::solve(model);
#ifdef EXPECT_BACKEND_DISABLED
  if (result.termination!=Gecode::Optimize::Termination::Unsupported || result.has_solution()) return 2;
#else
  if (!result.has_solution() || result.termination!=Gecode::Optimize::Termination::Optimal
      || result.value(x)!=2 || *result.objective!=6) return 2;
  Gecode::Optimize::SolveSession session;
  if (!session.solve(model).has_solution()) return 3;
  model.set_bounds(x,3,5);
  auto edited=session.solve(model);
  if (!edited.has_solution() || *edited.objective!=9 || session.statistics().model_loads!=1) return 4;
  auto conflict=Gecode::Optimize::analyze_conflict(model);
  if (conflict.status!=Gecode::Optimize::ConflictStatus::Feasible) return 5;
  auto reduced=Gecode::Optimize::solve(prepared.model->reduced());
  auto recovered=prepared.model->postsolve(reduced);
  if (!recovered.exact_witness_validated || recovered.solution.value(x)!=2
      || recovered.solution.termination!=Gecode::Optimize::Termination::Unknown) return 8;
  Gecode::Optimize::PoolOptions pool_options; pool_options.max_solutions=4;
  auto pool=Gecode::Optimize::solve_pool(model,pool_options);
  if (!pool.exhausted() || pool.entries.size()!=3 || pool.ranked_prefix!=3
      || pool.entries[0].solution.objective!=9 || pool.entries[2].solution.objective!=15) return 9;
#endif
#ifdef CHECK_NATIVE
  Gecode::Optimize::SolveOptions native_options;
  native_options.backend=Gecode::Optimize::Backend::Native;
  native_options.guarantee=Gecode::Optimize::Guarantee::Exact;
  native_options.primal_start={{x,5}};
  auto native_result=Gecode::Optimize::solve(model,native_options);
  if (!native_result.has_solution() || !native_result.start_submitted || native_result.guarantee!=Gecode::Optimize::Guarantee::Exact) return 6;
  Gecode::Optimize::Model regular;
  auto letter=regular.add_integer(-1,1);
  Gecode::Optimize::add_regular(regular,{letter},2,0,{{0,-1,1}},{1},"signed word");
  regular.minimize({{letter,1}});
  native_options.primal_start.clear();
  auto accepted=Gecode::Optimize::solve(regular,native_options);
  if (!accepted.has_solution() || accepted.objective!=-1 || accepted.best_bound!=accepted.objective) return 17;
#endif
  auto hybrid=Gecode::Optimize::solve_native_lp(model);
  if (Gecode::Optimize::native_lp_capabilities().available) {
    if (!hybrid.result.has_solution() || hybrid.result.termination!=Gecode::Optimize::Termination::Optimal) return 10;
  } else if (hybrid.result.termination!=Gecode::Optimize::Termination::Unsupported) return 11;
  auto frontier=Gecode::Optimize::solve_native_search(model);
#ifdef CHECK_NATIVE
  if (!frontier.result.has_solution() || frontier.result.termination!=Gecode::Optimize::Termination::Optimal
      || frontier.result.best_bound!=frontier.result.objective) return 12;
#else
  if (frontier.result.termination!=Gecode::Optimize::Termination::Unsupported) return 12;
#endif
  auto neighborhood=Gecode::Optimize::solve_native_neighborhoods(model);
  if (!neighborhood.neighborhood.requested) return 22;
#ifdef CHECK_NATIVE
  if (!neighborhood.search.result.has_solution()
      || neighborhood.search.result.termination!=Gecode::Optimize::Termination::Optimal
      || neighborhood.neighborhood.budget_nodes!=neighborhood.search.frontier.admitted_nodes
           +neighborhood.search.branching.probe_status_calls+neighborhood.neighborhood.status_attempts) return 22;
#else
  if (neighborhood.search.result.termination!=Gecode::Optimize::Termination::Unsupported) return 22;
#endif
  Gecode::Optimize::NativeSearchOptions branching_options;
  branching_options.branching=Gecode::Optimize::NativeBranchingSettings{};
  auto branching=Gecode::Optimize::solve_native_search(model,branching_options);
  if (!branching.branching.requested) return 14;
#ifdef CHECK_NATIVE
  if (!branching.result.has_solution() || branching.result.termination!=Gecode::Optimize::Termination::Optimal
      || branching.branching.budget_nodes!=branching.frontier.admitted_nodes+branching.branching.probe_status_calls) return 14;
#else
  if (branching.result.termination!=Gecode::Optimize::Termination::Unsupported) return 14;
#endif
  Gecode::Optimize::Model lp;
  auto lx=lp.add_continuous(),ly=lp.add_continuous();
  auto demand=lp.add_row({{lx,1},{ly,1}},4,std::numeric_limits<double>::infinity());
  lp.minimize({{lx,2},{ly,3}},7);
  auto observed=Gecode::Optimize::solve_lp_observed(lp);
#ifdef EXPECT_BACKEND_DISABLED
  if (Gecode::Optimize::lp_observation_capabilities().available
      || observed.result.termination!=Gecode::Optimize::Termination::Unsupported) return 15;
#else
  if (!observed.observations || observed.result.termination!=Gecode::Optimize::Termination::Optimal
      || observed.result.objective!=15 || !observed.observations->checks().accepted
      || observed.observations->row(demand).dual!=2
      || observed.observations->column(ly).reduced_cost!=1) return 15;
  Gecode::Optimize::LpBasisSolveOptions basis_options;
  basis_options.basis=Gecode::Optimize::make_lp_basis(*observed.observations);
  auto basis_result=Gecode::Optimize::solve_lp_with_basis(lp,basis_options);
  if (basis_result.observed.result.termination!=Gecode::Optimize::Termination::Optimal
      || basis_result.observed.result.objective!=15
      || !basis_result.submission.backend_attempted
      || (basis_result.submission.state!=Gecode::Optimize::LpBasisSubmissionState::Accepted
          && basis_result.submission.state!=Gecode::Optimize::LpBasisSubmissionState::Repaired)) return 16;
  Gecode::Optimize::LpSensitivityOptions sensitivity_options;
  sensitivity_options.parameters={Gecode::Optimize::LpObjectiveParameter{lx}};
  auto sensitivity=Gecode::Optimize::analyze_lp_sensitivity(observed,sensitivity_options);
  if (sensitivity.completion!=Gecode::Optimize::LpSensitivityCompletion::Complete
      || !sensitivity.sensitivity) return 21;
  const auto* range=sensitivity.sensitivity->objective(lx);
  if (!range || range->group.state!=Gecode::Optimize::LpSensitivityState::Available
      || !range->interval || range->interval->lower.value!=0
      || range->interval->upper.value!=3 || range->interval->objective_slope!=4
      || sensitivity.sensitivity->original().result.objective!=15) return 21;
#endif
  std::vector<Gecode::Optimize::ScenarioDefinition> scenarios(2);
  scenarios[1].objective_offset=-5;
  auto batch=Gecode::Optimize::solve_scenarios(lp,scenarios);
#ifdef EXPECT_BACKEND_DISABLED
  if (batch.stop_reason!=Gecode::Optimize::Termination::Unsupported || batch.all_resolved()) return 18;
#else
  if (!batch.all_resolved() || !batch.batch || batch.outcomes.size()!=2
      || !batch.outcomes[0].result || !batch.outcomes[1].result
      || batch.outcomes[0].result->objective!=15 || batch.outcomes[1].result->objective!=3
      || batch.value(batch.batch->scenario(1),lx)!=4) return 18;
#endif
  Gecode::Optimize::ScenarioBatchOptions certified_scenarios;
  certified_scenarios.solve.guarantee=Gecode::Optimize::Guarantee::Certified;
  auto unsupported_batch=Gecode::Optimize::solve_scenarios(lp,scenarios,certified_scenarios);
  if (unsupported_batch.stop_reason!=Gecode::Optimize::Termination::Unsupported
      || unsupported_batch.batch || unsupported_batch.attempted!=0) return 19;
  Gecode::Optimize::QuadraticModel quadratic;
  Gecode::Optimize::Model infeasible_lp;
  auto bounded=infeasible_lp.add_continuous(0,1);
  auto impossible=infeasible_lp.add_row({{bounded,1}},2,std::numeric_limits<double>::infinity());
  Gecode::Optimize::LpEvidenceOptions evidence_options;
  evidence_options.request=Gecode::Optimize::LpEvidenceRequest::Farkas;
  auto evidence=Gecode::Optimize::analyze_lp_evidence(infeasible_lp,evidence_options);
#ifdef EXPECT_BACKEND_DISABLED
  if (evidence.stop_reason!=Gecode::Optimize::Termination::Unsupported || evidence.attempted_calls) return 20;
#else
  if (evidence.completion!=Gecode::Optimize::LpEvidenceCompletion::Complete || !evidence.evidence
      || evidence.evidence->farkas().state!=Gecode::Optimize::LpEvidenceState::Available
      || evidence.evidence->row_multiplier(impossible).multiplier!=1
      || evidence.evidence->column_multiplier(bounded).multiplier!=-1
      || evidence.evidence->farkas_data().contradiction_margin!=1 || evidence.attempted_calls!=1) return 20;
#endif
  auto qx=quadratic.add_continuous(-2,3,"qx");
  quadratic.minimize_squares({{{{qx,1}},-1,2,"square"}}, {}, 3);
  auto qp=Gecode::Optimize::solve_quadratic(quadratic);
#ifdef EXPECT_BACKEND_DISABLED
  if (Gecode::Optimize::quadratic_capabilities().available
      || qp.result.termination!=Gecode::Optimize::Termination::Unsupported
      || qp.result.has_solution()) return 13;
#else
  if (!Gecode::Optimize::quadratic_capabilities().available || !qp.result.has_solution()
      || qp.result.termination!=Gecode::Optimize::Termination::Optimal
      || std::abs(qp.result.value(qx)-1)>1e-7 || !qp.result.objective
      || std::abs(*qp.result.objective-3)>1e-7 || !qp.checks.bound_valid
      || !qp.checks.gap_upper_bound || *qp.checks.gap_upper_bound>1e-6) return 13;
#endif
#endif
  return 0;
}
