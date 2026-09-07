/* Explicit bounded convex/concave continuous quadratic optimization. */
#ifndef GECODE_OPTIMIZE_QUADRATIC_HPP
#define GECODE_OPTIMIZE_QUADRATIC_HPP
#include <gecode/optimize/solve.hpp>
#include <memory>
namespace Gecode { namespace Optimize {
namespace Detail { struct QuadraticAccess; }
struct WeightedSquare {
  std::vector<Term> terms;
  double offset = 0;
  double weight = 1;
  std::string name;
};
/** Immutable, owning, distinct snapshot; no conversion to a linear model. */
class QuadraticSnapshot {
public:
  QuadraticSnapshot(const QuadraticSnapshot&) noexcept = default;
  QuadraticSnapshot& operator=(const QuadraticSnapshot&) noexcept = default;
  // Moving a snapshot preserves a usable historical view in both objects.
  QuadraticSnapshot(QuadraticSnapshot&& other) noexcept : data_(other.data_) {}
  QuadraticSnapshot& operator=(QuadraticSnapshot&& other) noexcept { data_=other.data_; return *this; }
  ModelId id() const noexcept;
  Revision revision() const noexcept;
  const std::vector<VariableData>& variables() const noexcept;
  const std::vector<RowData>& rows() const noexcept;
  const ObjectiveData& linear_part() const noexcept;
  const std::vector<WeightedSquare>& squares() const noexcept;
private:
  struct Data;
  std::shared_ptr<const Data> data_;
  explicit QuadraticSnapshot(ModelSnapshot, std::vector<WeightedSquare>);
  friend class QuadraticModel;
  friend struct Detail::QuadraticAccess;
};
/** f = linear + offset + sum(w*r^2) for min, minus sum(w*r^2) for max. */
class QuadraticModel {
public:
  QuadraticModel() = default;
  QuadraticModel(QuadraticModel&&) noexcept = default;
  QuadraticModel& operator=(QuadraticModel&&) noexcept = default;
  QuadraticModel(const QuadraticModel&) = delete;
  QuadraticModel& operator=(const QuadraticModel&) = delete;
  ModelId id() const noexcept { return core_.id(); }
  Revision revision() const noexcept { return core_.revision(); }
  Variable add_continuous(double lower, double upper, std::string name = {});
  Constraint add_row(const std::vector<Term>&, double lower, double upper,
                     std::string name = {});
  void set_bounds(Variable, double lower, double upper);
  void set_bounds(Constraint, double lower, double upper);
  void set_coefficient(Constraint, Variable, double);
  void remove(Variable);
  void remove(Constraint);
  void minimize_squares(const std::vector<WeightedSquare>&,
                        const std::vector<Term>& linear = {}, double offset = 0);
  void maximize_concave_squares(const std::vector<WeightedSquare>&,
                        const std::vector<Term>& linear = {}, double offset = 0);
  QuadraticSnapshot snapshot() const;
private:
  Model core_;
  std::vector<WeightedSquare> squares_;
  void objective(const std::vector<WeightedSquare>&, const std::vector<Term>&,
                 double, ObjectiveSense);
};
struct QuadraticOptions {
  SolveOptions solve;
  std::uint64_t iteration_limit = 100000;
  std::size_t max_auxiliary_variables = 100000;
  std::size_t max_lifted_nonzeros = 2000000;
  double stationarity_tolerance = 1e-7;
  double complementarity_tolerance = 1e-7;
  /** Absolute original objective units; no relaxation by objective constants. */
  double optimality_tolerance = 1e-6;
  void validate() const;
};
struct QuadraticValidation {
  bool primal_valid = false, objective_valid = false;
  bool kkt_available = false, kkt_valid = false, bound_valid = false;
  double max_stationarity = 0, max_complementarity = 0;
  std::optional<double> original_objective, normalized_lower_bound;
  /** Outward upper bound on original normalized primal minus dual, offset free. */
  std::optional<double> gap_upper_bound;
  std::vector<double> square_values, original_gradient;
  std::string message;
};
struct QuadraticResult {
  SolveResult result;
  QuadraticValidation checks;
  std::optional<double> vendor_objective, vendor_dual_estimate;
  std::uint64_t qp_iterations = 0;
  double regularization = 0;
};
/** Independent original primal/objective/gradient check; no optimality claim. */
QuadraticValidation validate_quadratic(const QuadraticSnapshot&,
                                      const std::vector<double>&,
                                      double feasibility_tolerance = 1e-7);
QuadraticResult solve_quadratic(const QuadraticSnapshot&, const QuadraticOptions& = {});
QuadraticResult solve_quadratic(const QuadraticModel&, const QuadraticOptions& = {});
BackendCapabilities quadratic_capabilities();
}}
#endif
