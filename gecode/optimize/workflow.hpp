/* Ordered linear objective workflows for the numerical optimization API. */
#ifndef GECODE_OPTIMIZE_WORKFLOW_HPP
#define GECODE_OPTIMIZE_WORKFLOW_HPP

#include <gecode/optimize/solve.hpp>

namespace Gecode { namespace Optimize {

/** The input vector establishes priority: first objective has highest priority. */
struct LexicographicObjective {
  ObjectiveData objective;
  double absolute_degradation = 0.0;
  double relative_degradation = 0.0;
  std::string name;
};

struct LexicographicStage {
  std::size_t index = 0;
  std::string name;
  SolveResult result;
  bool completed = false;
  // Threshold in this objective's original units, including its offset.
  // A minimization stage supplies an upper threshold; maximization a lower.
  std::optional<double> retention_bound;
};

struct LexicographicResult {
  ModelId model_id = 0;
  Revision revision = 0;
  Termination termination = Termination::Unknown;
  Guarantee guarantee = Guarantee::Numerical;
  std::string message;
  std::vector<LexicographicStage> stages;
  std::size_t completed_stages = 0;
  double elapsed_seconds = 0.0;
  // Feasible historical solution of the ORIGINAL model. Its scalar objective
  // evaluates the original model objective, which this workflow need not have
  // optimized; its termination remains Unknown and it has no objective bound.
  SolveResult final_solution;
  // All requested objectives at final_solution, in the original input order.
  std::vector<double> objective_values;

  bool has_solution() const noexcept { return final_solution.has_solution(); }
  bool completed_numerically() const noexcept {
    return termination == Termination::Optimal && !stages.empty() &&
           completed_stages == stages.size();
  }
};

/**
 * Optimize ordered linear objectives on private snapshot copies. Degradation
 * after stage i is absolute_degradation + relative_degradation * abs(value_i).
 * Both native MIP gap targets are forced to zero. Completion is numerical and
 * tolerance-qualified, never an exact certificate of lexicographic optimality.
 * All candidates are independently checked against original constraints,
 * accumulated lock rows and original objective retention thresholds.
 *
 * A shared outer clock/token covers copying, validation and every solve.
 * Time/cancellation remains cooperative; a stage returned after the outer
 * deadline is not promoted. Node-limited multi-stage requests are Unsupported.
 * Terms must use the canonical ascending unique slots of ModelSnapshot.
 */
LexicographicResult solve_lexicographic(
  const ModelSnapshot& model,
  const std::vector<LexicographicObjective>& objectives,
  const SolveOptions& options = {});

LexicographicResult solve_lexicographic(
  const Model& model,
  const std::vector<LexicographicObjective>& objectives,
  const SolveOptions& options = {});

}}
#endif
