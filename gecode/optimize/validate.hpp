/* Independent validation of numerical linear optimization models. */
#ifndef GECODE_OPTIMIZE_VALIDATE_HPP
#define GECODE_OPTIMIZE_VALIDATE_HPP

#include <gecode/optimize/model.hpp>

#include <optional>
#include <string>
#include <vector>

namespace Gecode { namespace Optimize {

struct ValidationReport {
  bool valid = false;
  // True iff the snapshot itself passed structural validation. This does
  // not establish that its domains contain a feasible solution.
  bool model_valid = false;
  std::string message;
  std::optional<double> objective;
  double max_bound_violation = 0.0;
  double max_row_violation = 0.0;
  double max_integrality_violation = 0.0;
  double max_indicator_violation = 0.0;
  std::size_t violated_globals = 0;
};

/** Check a complete original snapshot before a backend or workflow uses it.
 * Throws ModelError for malformed IDs, domains, expressions or references.
 * Expression terms must have ascending unique slot IDs and finite nonzero
 * coefficients, matching the canonical form produced by Model.
 * Semi-variable domains are {0} union [lower, upper], with finite lower > 0.
 * A mathematically infeasible integer interval is not a structural error.
 * Active original indicators require complete lowerings, safe M values,
 * intact generated rows/gates and valid captured-domain dependencies.
 * Typed global payloads and their referenced variable domains are checked too;
 * backend-specific numerical/native admission remains a separate operation.
 */
void validate_structure(const ModelSnapshot& model);

/** Independently evaluate original domains, rows, indicators, globals and cost.
 * Values use original slot indexing: exactly one entry per variable slot;
 * unreferenced deleted slots may contain NaN and are ignored. Tolerances must
 * be finite and nonnegative, in absolute original-model units.
 * Linear arithmetic uses compensated long double accumulation. This is
 * numerical validation, not a proof of optimality or infeasibility.
 * Original indicators are checked after rounding their binary activator;
 * passing their big-M rows alone never establishes logical feasibility.
 * Globals check tolerance-qualified integer values against original payloads.
 * Malformed models, options and assignments return an invalid report rather
 * than throwing ModelError. Allocation failures may still propagate.
 */
ValidationReport validate(const ModelSnapshot& model,
                          const std::vector<double>& slot_values,
                          double feasibility_tolerance = 1e-7,
                          double integrality_tolerance = 1e-6);

}}

#endif
