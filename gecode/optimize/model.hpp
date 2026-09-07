/* Sparse numerical model for the additive optimization API. */
#ifndef GECODE_OPTIMIZE_MODEL_HPP
#define GECODE_OPTIMIZE_MODEL_HPP

#include <gecode/optimize/types.hpp>
#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace Gecode { namespace Optimize {

struct Term {
  Variable variable;
  double coefficient = 0.0;
};

struct VariableSpec {
  VariableType type = VariableType::Continuous;
  double lower = 0.0;
  double upper = std::numeric_limits<double>::infinity();
  std::string name;
};
struct RowSpec {
  std::vector<Term> terms;
  double lower = -std::numeric_limits<double>::infinity();
  double upper = std::numeric_limits<double>::infinity();
  std::string name;
};
/**
 * CSR row input with an explicit unique live-variable column mapping.
 * Row starts have rows+1 entries, begin at zero and end at coefficients.size().
 * Column indices may be unsorted/repeated within a row; normal coalescing applies.
 * Names are empty or have exactly one entry per row. No dense matrix is created.
 */
struct SparseRowBatch {
  std::vector<Variable> columns;
  std::vector<std::size_t> row_start{0}, column;
  std::vector<double> coefficient, lower, upper;
  std::vector<std::string> names;
};

struct Indicator {
  ModelId model_id = 0;
  std::uint64_t id = 0;
};

struct VariableData {
  Variable variable;
  VariableType type = VariableType::Continuous;
  double lower = 0.0;
  double upper = std::numeric_limits<double>::infinity();
  std::string name;
  bool active = true;
  std::optional<Indicator> indicator_origin;
};

struct RowData {
  Constraint constraint;
  std::vector<Term> terms;
  double lower = -std::numeric_limits<double>::infinity();
  double upper = std::numeric_limits<double>::infinity();
  std::string name;
  bool active = true;
  std::optional<Indicator> indicator_origin;
};

struct ObjectiveData {
  std::vector<Term> terms;
  double offset = 0.0;
  ObjectiveSense sense = ObjectiveSense::Minimize;
};

/** A domain used to justify a bounded indicator's inactive relaxation. */
struct IndicatorDomain {
  Variable variable;
  double lower = 0.0;
  double upper = 0.0;
};

/** Original logical meaning and the complete, guarded linear lowering. */
struct IndicatorData {
  Indicator indicator;
  Variable activator;
  bool active_value = true;
  std::vector<Term> terms;
  double lower = -std::numeric_limits<double>::infinity();
  double upper = std::numeric_limits<double>::infinity();
  std::optional<Variable> inactive_gate;
  std::optional<double> lower_m;
  std::optional<double> upper_m;
  std::vector<Constraint> generated_rows;
  std::vector<IndicatorDomain> domains;
  bool active = true;
};

struct GlobalConstraint { ModelId model_id = 0; std::uint64_t id = 0; };
struct AllDifferentData { std::vector<Variable> variables; };
struct ElementData {
  Variable index;
  std::vector<Variable> elements;
  Variable result;
  std::int64_t index_base = 0;
};
struct TableData {
  std::vector<Variable> variables;
  std::vector<std::vector<std::int64_t>> tuples;
};
struct CumulativeData {
  std::vector<Variable> starts;
  std::vector<std::int64_t> durations;
  std::vector<std::int64_t> heights;
  std::int64_t capacity = 0;
};
struct CircuitData {
  std::vector<Variable> successors;
  std::int64_t index_base = 0;
};
struct RegularTransition {
  std::uint64_t from = 0;
  std::int64_t symbol = 0;
  std::uint64_t to = 0;
};
struct RegularData {
  std::vector<Variable> variables;
  std::uint64_t state_count = 1;
  std::uint64_t initial_state = 0;
  std::vector<RegularTransition> transitions;
  std::vector<std::uint64_t> final_states;
};
using GlobalPayload = std::variant<AllDifferentData, ElementData, TableData,
                                   CumulativeData, CircuitData, RegularData>;
struct GlobalData {
  GlobalConstraint global;
  GlobalPayload payload;
  std::string name;
  bool active = true;
};

/** An owning historical snapshot. Public fields are untrusted at API boundaries. */
struct ModelSnapshot {
  ModelId model_id = 0;
  Revision revision = 0;
  std::vector<VariableData> variables;
  std::vector<RowData> rows;
  ObjectiveData objective;
  std::vector<IndicatorData> indicators;
  std::vector<GlobalData> globals;
};

namespace Detail { class ConstraintBatch; }

/**
 * An owning sparse model with a linear objective and retained logical and
 * global constraints.
 *
 * Handles identify a model and a never-reused slot. Successful mutations advance
 * the revision, including assignments of unchanged values. Failed mutations leave
 * the model unchanged. Snapshots own their data and remain usable after edits.
 *
 * Copying is disabled. Moving preserves the identity and all existing handles;
 * a moved-from model can only be destroyed, assigned another model, or queried
 * for its zero id/revision. Mutations and snapshot() reject a moved-from object.
 * Mutation is not thread-safe; independent models can be created concurrently.
 */
class Model {
public:
  Model();
  Model(const Model&) = delete;
  Model& operator=(const Model&) = delete;
  Model(Model&& other) noexcept;
  Model& operator=(Model&& other) noexcept;

  ModelId id() const noexcept { return model_id_; }
  Revision revision() const noexcept { return revision_; }
  ModelSnapshot snapshot() const;

  Variable add_variable(VariableType type, double lower, double upper,
                        std::string name = {});
  Variable add_continuous(double lower = 0.0,
                          double upper = std::numeric_limits<double>::infinity(),
                          std::string name = {});
  Variable add_integer(double lower = 0.0,
                       double upper = std::numeric_limits<double>::infinity(),
                       std::string name = {});
  Variable add_binary(std::string name = {});
  Constraint add_row(const std::vector<Term>& terms, double lower, double upper,
                     std::string name = {});

  /** Atomic bulk additions; one revision per nonempty batch, no reused slots.
   * Validation/allocation failure leaves entities, revision and existing views
   * unchanged. Empty valid batches are no-ops. Rows refer to existing variables.
   */
  std::vector<Variable> add_variables(const std::vector<VariableSpec>& variables);
  std::vector<Constraint> add_rows(const std::vector<RowSpec>& rows);
  std::vector<Constraint> add_rows_sparse(const SparseRowBatch& rows);

  void set_objective(const std::vector<Term>& terms, ObjectiveSense sense,
                     double offset = 0.0);
  void minimize(const std::vector<Term>& terms, double offset = 0.0);
  void maximize(const std::vector<Term>& terms, double offset = 0.0);

  void set_bounds(Variable variable, double lower, double upper);
  void set_bounds(Constraint constraint, double lower, double upper);
  void set_coefficient(Constraint constraint, Variable variable,
                       double coefficient);
  void set_objective_coefficient(Variable variable, double coefficient);
  void set_objective_offset(double offset);
  void set_name(Variable variable, std::string name);
  void set_name(Constraint constraint, std::string name);

  /** Reject removal while an active row or the objective references variable. */
  void remove(Variable variable);
  /** Remove a row, retaining its slot as a tombstone. */
  void remove(Constraint constraint);

  const VariableData& variable(Variable variable) const;
  const RowData& row(Constraint constraint) const;
  GlobalConstraint add_global(GlobalPayload payload, std::string name = {});
  const GlobalData& global(GlobalConstraint constraint) const;
  void remove(GlobalConstraint constraint);
  void set_name(GlobalConstraint constraint, std::string name);

private:
  friend class Detail::ConstraintBatch;
  ModelId model_id_;
  Revision revision_ = 0;
  std::vector<VariableData> variables_;
  std::vector<RowData> rows_;
  ObjectiveData objective_;
  std::vector<IndicatorData> indicators_;
  std::vector<GlobalData> globals_;

  void require_live() const;
  void require_revision_capacity() const;
  void require_unprotected(Constraint constraint) const;
  std::vector<Term> normalize(const std::vector<Term>& terms,
                              const std::vector<VariableData>* additions = nullptr) const;
};

}}
#endif
