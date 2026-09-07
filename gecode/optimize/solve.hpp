/* Numerical LP/MILP backend for the additive optimization API. */
#ifndef GECODE_OPTIMIZE_SOLVE_HPP
#define GECODE_OPTIMIZE_SOLVE_HPP

#include <gecode/optimize/model.hpp>
#include <gecode/optimize/result.hpp>
#include <string>
#include <vector>

namespace Gecode { namespace Optimize {

struct BackendCapabilities {
  std::string name;
  std::string version;
  bool available = false;
  bool linear_programming = false;
  bool mixed_integer_linear = false;
  bool exact_solving = false;
  bool proof_certificates = false;
  bool lazy_constraints = false;
  bool quadratic_programming = false;
  std::vector<std::string> limitations;
};

/** Query one backend without inspecting a model. Auto reports HiGHS here;
 * solve() separately routes models with active native globals to Native.
 * Other explicit operations have their own capability queries and contracts.
 */
BackendCapabilities capabilities(Backend backend = Backend::Auto);

/**
 * Solve an owning original-model snapshot. HiGHS is explicitly a numerical
 * backend, not the legacy certified Gecode binary-LP propagation engine.
 * Auto selects Native for models with active native globals, HiGHS otherwise.
 * HiGHS cannot meet Exact/Certified requests.
 * Explicit Native selects the bounded exact-integer Gecode bridge with
 * conservative structural algorithm selection; see solve_native_auto() in
 * native.hpp. The explicit solve_native() entry point retains ordinary BAB.
 * Returned incumbents pass the
 * independent original-model numerical checker. Time/cancel limits are
 * cooperative; elapsed_seconds includes compilation and checking. A late
 * candidate is excluded unless captured by an internal HiGHS observation
 * callback before the deadline. This does not expose a user callback API.
 */
SolveResult solve(const ModelSnapshot& model, const SolveOptions& options = {});
SolveResult solve(const Model& model, const SolveOptions& options = {});

/**
 * Backend-independent, strict numerical LP/free-MPS I/O. Unsupported dialects,
 * quadratic models, and exporting active original indicators/globals are rejected.
 * Writes preserve double values and atomically replace only after a checked
 * semantic round trip. See docs/optimize-io.md for the supported syntax.
 */
Model read_model(const std::string& filename);
void write_model(const ModelSnapshot& model, const std::string& filename);
inline void write_model(const Model& model, const std::string& filename) {
  write_model(model.snapshot(), filename);
}

}}
#endif
