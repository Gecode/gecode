#ifndef GECODE_OPTIMIZE_GLOBALS_HPP
#define GECODE_OPTIMIZE_GLOBALS_HPP
#include <gecode/optimize/model.hpp>

namespace Gecode { namespace Optimize {
/// Retain pairwise inequality of the supplied integer-variable slots.
GlobalConstraint add_all_different(Model&, const std::vector<Variable>&, std::string name = {});
/// Retain result == elements[index-index_base], including repeated handles.
GlobalConstraint add_element(Model&, Variable index, const std::vector<Variable>& elements,
                             Variable result, std::int64_t index_base = 0, std::string name = {});
/** Retain membership in a positive integer tuple table with matching arity.
 * An empty tuple list is false. A zero-variable table containing an empty
 * tuple is true; the separate FlatZinc compiler requires positive arity.
 */
GlobalConstraint add_table(Model&, const std::vector<Variable>&,
                           const std::vector<std::vector<std::int64_t>>&, std::string name = {});
/** Mandatory fixed-duration tasks, half-open intervals; zero duration/height uses no resource. */
GlobalConstraint add_cumulative(Model&, const std::vector<Variable>& starts,
                                const std::vector<std::int64_t>& durations,
                                const std::vector<std::int64_t>& heights,
                                std::int64_t capacity, std::string name = {});
/** One cycle through a nonempty successor array, with explicit index base. */
GlobalConstraint add_circuit(Model&, const std::vector<Variable>& successors,
                             std::int64_t index_base = 0, std::string name = {});
/** Retain membership in a deterministic finite automaton's language.
 * States are in [0,state_count); missing transitions reject. Transition keys
 * (from,symbol) must be unique; repeated final states have set semantics.
 * Empty words accept exactly when initial_state is final. No epsilon edges.
 * Posting validates original metadata; native solving applies additional
 * integer, array-size and automaton-storage limits before compilation.
 */
GlobalConstraint add_regular(Model&, const std::vector<Variable>& variables,
                             std::uint64_t state_count, std::uint64_t initial_state,
                             const std::vector<RegularTransition>& transitions,
                             const std::vector<std::uint64_t>& final_states,
                             std::string name = {});

namespace Detail {
std::vector<Variable> global_variables(const GlobalPayload&);
void validate_global_payload(const GlobalPayload&, ModelId, const std::vector<VariableData>&,
                             bool require_active);
void validate_globals(const ModelSnapshot&);
bool global_satisfied(const GlobalPayload&, const std::vector<double>&, double integrality_tolerance,
                      std::string& reason);
}
}}
#endif
