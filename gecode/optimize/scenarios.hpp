/* Owning serial scenario batches; no shared search-tree semantics. */
#ifndef GECODE_OPTIMIZE_SCENARIOS_HPP
#define GECODE_OPTIMIZE_SCENARIOS_HPP
#include <gecode/optimize/session.hpp>
#include <gecode/optimize/validate.hpp>

namespace Gecode { namespace Optimize {
struct ScenarioVariableBounds {
  Variable variable;
  std::optional<double> lower, upper;
};
struct ScenarioRowBounds {
  Constraint row;
  std::optional<double> lower, upper;
};
struct ScenarioDefinition {
  std::string name;
  /** Absolute coefficients; zero removes a term. Missing entries inherit base. */
  std::vector<Term> objective_coefficients;
  std::optional<double> objective_offset;
  std::vector<ScenarioVariableBounds> variable_bounds;
  std::vector<ScenarioRowBounds> row_bounds;
};
struct ScenarioId {
  ModelId batch_id = 0;
  std::uint64_t index = 0;
};
enum class ScenarioReuse { Automatic, Cold };
struct ScenarioBatchOptions {
  SolveOptions solve;
  ScenarioReuse reuse = ScenarioReuse::Automatic;
  std::size_t max_scenarios = 1000;
  std::size_t max_patch_entries = 1000000;
  std::size_t max_saved_value_slots = 10000000;
  /** Metered coordinator element visits, excluding backend internals. */
  std::size_t max_work = 100000000;
  void validate() const;
};
enum class ScenarioRunState { NotStarted, Attempted };
enum class ScenarioBatchCompletion { Rejected, Interrupted, Complete };
namespace Detail { struct ScenarioBatchAccess; }
class ScenarioBatch {
public:
  ScenarioBatch(const ScenarioBatch&) = delete;
  ScenarioBatch& operator=(const ScenarioBatch&) = delete;
  ModelId id() const noexcept;
  const ModelSnapshot& base() const noexcept;
  std::size_t size() const noexcept;
  ScenarioId scenario(std::size_t index) const;
  const ScenarioDefinition& definition(ScenarioId) const;
  /** Owning private snapshot. Same private owner, distinct scenario revision. */
  ModelSnapshot materialize(ScenarioId) const;
  Variable map(Variable original) const;
  Constraint map(Constraint original) const;
private:
  friend struct Detail::ScenarioBatchAccess;
  ScenarioBatch() = default;
  ModelId owner_ = 0;
  ModelSnapshot base_;
  std::vector<ScenarioDefinition> definitions_;
};
struct ScenarioCheck {
  bool identity_valid = false;
  /** validation/objective/exact flags are meaningful only when true. */
  bool candidate_examined = false;
  bool objective_matches = false;
  bool exact_witness_validated = false;
  ValidationReport validation;
};
struct ScenarioOutcome {
  ScenarioId scenario;
  ScenarioRunState state = ScenarioRunState::NotStarted;
  std::optional<SolveResult> result;
  std::optional<ScenarioCheck> check;
  SessionStatistics reuse_delta;
  double elapsed_seconds = 0;
};
struct ScenarioBatchResult {
  ModelId model_id = 0;
  Revision revision = 0;
  std::shared_ptr<const ScenarioBatch> batch;
  ScenarioBatchCompletion completion = ScenarioBatchCompletion::Rejected;
  std::optional<Termination> stop_reason;
  std::optional<std::size_t> offending_scenario;
  std::string message;
  std::vector<ScenarioOutcome> outcomes;
  SessionStatistics reuse_statistics;
  std::size_t attempted = 0, resolved = 0, work = 0;
  double elapsed_seconds = 0;
  bool all_resolved() const noexcept;
  double value(ScenarioId, Variable original) const;
};
/**
 * Ordinary Continuous/Integer/Binary linear models only. All patches are
 * admitted before any solve; nonempty indicator/global metadata and primal
 * starts are Unsupported. Auto/HiGHS use private session reuse or Cold solves;
 * explicit Native uses its ordinary one-shot route and exact admission.
 * One whole-batch time/cancel allowance. Positive node limits with >1 scenario
 * are Unsupported until consumed-node accounting exists. Zero stops before any
 * solve; one scenario preserves the ordinary backend node-limit contract.
 * No caller model/session is changed. Materializations and results own history.
 */
ScenarioBatchResult solve_scenarios(const ModelSnapshot&,
    const std::vector<ScenarioDefinition>&, const ScenarioBatchOptions& = {});
ScenarioBatchResult solve_scenarios(const Model&,
    const std::vector<ScenarioDefinition>&, const ScenarioBatchOptions& = {});
}}
#endif
