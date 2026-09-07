/* Logical modeling helpers with explicit original-model semantics. */
#ifndef GECODE_OPTIMIZE_CONSTRAINTS_HPP
#define GECODE_OPTIMIZE_CONSTRAINTS_HPP

#include <gecode/optimize/model.hpp>

namespace Gecode { namespace Optimize {

struct IndicatorFormulation {
  Indicator indicator;
  std::optional<Variable> inactive_gate;
  std::vector<Constraint> rows;
  std::optional<double> lower_m;
  std::optional<double> upper_m;
};

/**
 * Add activator == active_value => lower <= sum(terms) <= upper.
 * The activator must have Binary type. Every required inactive activity bound
 * must be finite. M is derived conservatively; no guessed constant is used.
 * The returned gate keeps original coefficients and row bounds unchanged.
 * Original logical metadata is retained for independent solution validation.
 * Successful posting is one revision; errors leave the model unchanged.
 */
IndicatorFormulation add_indicator(Model& model, Variable activator,
                                  bool active_value,
                                  const std::vector<Term>& terms,
                                  double lower, double upper,
                                  std::string name = {});

/** Remove logical metadata and all its generated rows in one revision.
 * The auxiliary gate remains a model variable and can be removed once unused.
 */
void remove_indicator(Model& model, Indicator indicator);

/** result == AND(inputs); AND(empty) is true. All variables must be Binary. */
std::vector<Constraint> add_boolean_and(Model& model, Variable result,
                                      const std::vector<Variable>& inputs,
                                      std::string name = {});
/** result == OR(inputs); OR(empty) is false. All variables must be Binary. */
std::vector<Constraint> add_boolean_or(Model& model, Variable result,
                                     const std::vector<Variable>& inputs,
                                     std::string name = {});

namespace Detail {
/** Structural validation of untrusted original indicator metadata/lowerings. */
void validate_indicators(const ModelSnapshot& model);
}

}}
#endif
