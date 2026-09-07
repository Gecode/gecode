/* Numerical conflicts over explicit original-model groups. */
#ifndef GECODE_OPTIMIZE_DIAGNOSTICS_HPP
#define GECODE_OPTIMIZE_DIAGNOSTICS_HPP

#include <gecode/optimize/solve.hpp>

namespace Gecode { namespace Optimize {

enum class ConflictStatus {
  Unknown, Feasible, Irreducible, Incomplete, Unsupported, InvalidModel, Error
};

enum class ConflictGroupKind {
  Row, LowerBound, UpperBound, Integrality, VariableDomain, IndicatorComponent
};

struct ConflictGroup {
  ConflictGroupKind kind = ConflictGroupKind::Row;
  std::string name;
  std::optional<Constraint> row;
  std::optional<Variable> variable;
  // An indicator component owns these logical records, their generated rows,
  // and the complete domains of all variables participating in the component.
  std::vector<Indicator> indicators;
  std::vector<Constraint> generated_rows;
  std::vector<Variable> grouped_variables;
  bool necessity_verified = false;
  // Optional feasible assignment after removing this group from the conflict.
  // Original slot order, with tombstones; it need not satisfy this group.
  std::vector<double> deletion_witness;
};

struct ConflictOptions {
  SolveOptions solve;
  bool retain_deletion_witnesses = false;
};

struct ConflictResult {
  ModelId model_id = 0;
  Revision revision = 0;
  ConflictStatus status = ConflictStatus::Unknown;
  // Completion: Optimal for Feasible, Infeasible for Irreducible. Otherwise
  // the stopping/error reason. This is separate from conflict completeness.
  Termination termination = Termination::Unknown;
  Guarantee guarantee = Guarantee::Numerical;
  std::string backend;
  std::string backend_version;
  std::string message;
  bool infeasibility_established = false;
  std::size_t oracle_calls = 0;
  double elapsed_seconds = 0.0;
  // Empty until original feasibility has been decided as infeasible. During
  // interruption this contains the last numerically established infeasible set.
  std::vector<ConflictGroup> groups;
  // Present only for a feasible original constraint system; original slots.
  std::vector<double> feasible_witness;
  bool irreducible() const noexcept { return status == ConflictStatus::Irreducible; }
};

/**
 * Deterministic deletion filtering with numerical LP/MILP feasibility oracles.
 * The original objective is replaced privately by zero. All calls share one
 * end-to-end time/cancellation budget. Starts are ignored and gaps forced zero.
 * Any node limit, Exact or Certified request is explicitly Unsupported.
 * See DIAGNOSTICS.md for group granularity and indicator domain ownership.
 * Irreducible means deletion-minimal at that granularity, not minimum size or
 * an exact infeasibility certificate. Interrupted or ambiguous oracles never
 * establish infeasibility or irreducibility.
 */
ConflictResult analyze_conflict(const ModelSnapshot& model,
                               const ConflictOptions& options = {});
ConflictResult analyze_conflict(const Model& model,
                               const ConflictOptions& options = {});

}}
#endif
