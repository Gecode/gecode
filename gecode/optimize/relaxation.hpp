/* Explicit weighted L1 repairs on a distinct private model. */
#ifndef GECODE_OPTIMIZE_RELAXATION_HPP
#define GECODE_OPTIMIZE_RELAXATION_HPP

#include <gecode/optimize/workflow.hpp>
#include <gecode/optimize/validate.hpp>

namespace Gecode { namespace Optimize {

enum class RelaxationSide { Lower, Upper };

struct RowRelaxation {
  Constraint row;
  RelaxationSide side = RelaxationSide::Lower;
  double penalty = 1.0;
};
struct BoundRelaxation {
  Variable variable;
  RelaxationSide side = RelaxationSide::Lower;
  double penalty = 1.0;
};
struct RelaxationOptions {
  SolveOptions solve;
  std::vector<RowRelaxation> rows;
  std::vector<BoundRelaxation> bounds;
  bool optimize_original_objective = false;
};

struct RelaxationItem {
  // Exactly one source handle is set. Private slack/row handles have a distinct
  // model owner and cannot be passed back to the original model as its handles.
  std::optional<Constraint> source_row;
  std::optional<Variable> source_variable;
  RelaxationSide side = RelaxationSide::Lower;
  std::string name;
  double original_bound = 0.0;
  double penalty = 1.0;
  Variable slack;
  Constraint penalty_row;
  // Absent until a repaired candidate passes independent checks. Violations
  // are recomputed in original units, rather than inferred from slack values.
  std::optional<double> activity;
  std::optional<double> violation;
  std::optional<double> weighted_violation;
  std::optional<double> slack_value;
};

struct RelaxationResult {
  ModelId source_model_id = 0;
  Revision source_revision = 0;
  Termination termination = Termination::Unknown;
  Guarantee guarantee = Guarantee::Numerical;
  std::string message;
  double elapsed_seconds = 0.0;
  std::optional<ModelSnapshot> private_model;
  // One private handle per original variable slot, including tombstones.
  std::vector<Variable> private_variables;
  std::vector<RelaxationItem> items;
  // Present when phase two was requested and a minimum-violation lock was
  // established. The returned private model includes this additional hard row.
  std::optional<Constraint> violation_lock;
  // All workflow results belong to private_model, never the original model.
  LexicographicResult workflow;
  bool repair_validated = false;
  bool minimum_violation_established = false;
  // Optimal only among repairs with minimum weighted violation, not an
  // unconstrained optimum of the original objective or original feasible set.
  bool original_objective_optimized = false;
  std::optional<double> minimum_weighted_violation;
  std::optional<double> weighted_violation;
  std::optional<double> original_objective;
  std::vector<double> original_values;
  ValidationReport original_validation;

  bool has_repair() const noexcept { return repair_validated && workflow.has_solution(); }
};

/**
 * Minimize the sum of positive finite penalties times selected side violations.
 * Unselected sides, integrality and intrinsic binary/semi domains remain hard.
 * Binary bound selection relaxes a narrower stored bound only as far as [0,1].
 * Selected semi bounds, generated indicator rows, indicator-participating
 * variable bounds, active globals and primal starts are explicitly Unsupported.
 * Optional phase two optimizes the original objective with zero requested
 * degradation of minimum weighted violation. All completion is numerical and
 * tolerance-qualified; no original feasible SolveResult is manufactured.
 * The original model is not mutated. See RELAXATION.md for support/limits.
 */
RelaxationResult relax_feasibility(const ModelSnapshot& model,
                                  const RelaxationOptions& options = {});
RelaxationResult relax_feasibility(const Model& model,
                                  const RelaxationOptions& options = {});

}}
#endif
