/* Private QP compilation/checking helpers. Not a supported public model API. */
#ifndef GECODE_OPTIMIZE_QUADRATIC_BOUND_HPP
#define GECODE_OPTIMIZE_QUADRATIC_BOUND_HPP
#include <gecode/optimize/quadratic.hpp>
namespace Gecode { namespace Optimize { namespace Detail {
struct QuadraticAccess {
  static const ModelSnapshot& core(const QuadraticSnapshot&);
};
struct QpInterval { double lower, upper; };
bool quadratic_arithmetic_supported() noexcept;
QpInterval qp_add(QpInterval, QpInterval);
QpInterval qp_multiply(QpInterval, QpInterval);
QpInterval qp_negate(QpInterval);
QpInterval qp_point(double);
struct QuadraticBound {
  std::optional<double> normalized_lower;
  std::optional<double> gap_upper;
};
QuadraticBound quadratic_bound(const QuadraticSnapshot&, const std::vector<double>& values,
                              const std::vector<double>& tangents,
                              const std::vector<double>& row_duals,
                              const SolveBudget* = nullptr);
struct QuadraticRaw {
  ModelId model_id = 0;
  Revision revision = 0;
  Termination termination = Termination::Unknown;
  bool value_valid = false, dual_valid = false;
  std::vector<double> values, residual_values, row_duals, column_duals;
  std::vector<bool> active_variables;
  std::optional<double> objective, dual_estimate;
  std::uint64_t iterations = 0;
  double regularization = 0;
  std::string message;
};
#ifdef GECODE_QUADRATIC_TEST_HOOKS
bool quadratic_test_oracle(const QuadraticSnapshot&, const QuadraticOptions&, QuadraticRaw&);
void quadratic_test_event(const char*);
extern double quadratic_test_regularization;
#endif
}}}
#endif
