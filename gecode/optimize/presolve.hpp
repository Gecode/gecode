/* Explicit exact bounded-integer reductions and owning reconstruction. */
#ifndef GECODE_OPTIMIZE_PRESOLVE_HPP
#define GECODE_OPTIMIZE_PRESOLVE_HPP
#include <gecode/optimize/model.hpp>
#include <gecode/optimize/result.hpp>

namespace Gecode { namespace Optimize {
enum class PresolveStatus { Fixpoint, Incomplete, Infeasible, Unsupported, InvalidModel, Error };
enum class PresolveBoundSide { Lower, Upper };
struct PresolveOptions {
  double time_limit_seconds = std::numeric_limits<double>::infinity();
  std::shared_ptr<CancellationToken> cancellation;
  std::size_t max_passes = 100;
  // Counts propagation row visits, excluding structural checks/finalization.
  std::optional<std::size_t> max_row_visits;
};
struct PresolveBoundChange {
  Variable variable;
  Constraint row;
  PresolveBoundSide side = PresolveBoundSide::Lower;
  std::int64_t before = 0, after = 0;
  std::size_t pass = 0;
};
struct PresolveVariableMap {
  Variable original;
  bool active = false;
  std::optional<Variable> reduced;
  std::optional<std::int64_t> fixed_value;
  std::int64_t lower = 0, upper = 0;
};
struct PresolveRowMap {
  Constraint original;
  bool active = false;
  std::optional<Constraint> reduced;
  bool redundant = false;
  // Exact fixed-variable contribution subtracted from retained row bounds.
  std::optional<std::int64_t> substituted_constant;
};
struct PostsolveResult {
  SolveResult solution;
  bool exact_witness_validated = false;
};
namespace Detail { struct PresolveBuilder; }
class PresolvedModel {
public:
  const ModelSnapshot& original() const noexcept { return original_; }
  const ModelSnapshot& reduced() const noexcept { return reduced_; }
  const std::vector<PresolveVariableMap>& variables() const noexcept { return variables_; }
  const std::vector<PresolveRowMap>& rows() const noexcept { return rows_; }
  // An original historical feasible solution only: scalar optimum status,
  // global bounds and gaps are not transferred. Integer values may be rounded
  // within tolerance, then both models are checked with exact integer arithmetic.
  PostsolveResult postsolve(const SolveResult& candidate,
                           double integrality_tolerance = 1e-6) const;
private:
  friend struct Detail::PresolveBuilder;
  PresolvedModel(ModelSnapshot original, ModelSnapshot reduced,
                 std::vector<PresolveVariableMap> variables,
                 std::vector<PresolveRowMap> rows);
  ModelSnapshot original_, reduced_;
  std::vector<PresolveVariableMap> variables_;
  std::vector<PresolveRowMap> rows_;
};
struct PresolveResult {
  ModelId model_id = 0;
  Revision revision = 0;
  PresolveStatus status = PresolveStatus::Incomplete;
  // Optimal means propagation reached its fixpoint, not a solved optimization.
  Termination termination = Termination::Unknown;
  Guarantee guarantee = Guarantee::Exact;
  std::string message;
  std::shared_ptr<const PresolvedModel> model;
  std::vector<PresolveBoundChange> changes;
  std::optional<Constraint> infeasible_row;
  std::optional<Variable> infeasible_variable;
  std::size_t passes = 0, row_visits = 0, fixed_variables = 0, removed_rows = 0;
  double elapsed_seconds = 0;
};

// Finite Integer/Binary variables and integral ordinary linear data only.
// Checked int64 arithmetic; exported integer doubles have magnitude <=2^53.
// Unsupported arithmetic/data never authorize a reduction or infeasibility.
// Explicit utility; automatic Native solving uses a separate bounded exact
// composition. The postsolve() feasible-witness-only contract is unchanged.
PresolveResult presolve_integer(const ModelSnapshot& model, const PresolveOptions& options = {});
PresolveResult presolve_integer(const Model& model, const PresolveOptions& options = {});
}}
#endif
