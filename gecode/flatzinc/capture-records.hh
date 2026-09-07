/* Owning FlatZinc records, independent of native parser configuration. */
#ifndef GECODE_FLATZINC_CAPTURE_RECORDS_HH
#define GECODE_FLATZINC_CAPTURE_RECORDS_HH
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Gecode { namespace FlatZinc { namespace Capture {

enum class Type { Integer, Boolean, Float, Set };
struct Reference {
  Type type = Type::Integer;
  std::size_t index = 0;
};
/** Line is the parser reduction/end line, not a claimed exact start span. */
struct Location { std::string source; std::size_t line = 0, ordinal = 0; };
struct SetLiteral {
  bool interval = false;
  std::int64_t lower = 0, upper = -1;
  std::vector<std::int64_t> values;
};
enum class ValueKind { Integer, Boolean, Float, Set, Reference, Array, Atom, String, Call };
/** Value is an owning tagged tree. Only the member selected by kind is meaningful.
 * Array elements are in source order. Call uses text for its identifier and
 * elements for its positional arguments (one scalar argument remains one).
 */
struct Value {
  ValueKind kind = ValueKind::Integer;
  std::int64_t integer = 0;
  bool boolean = false;
  double floating = 0;
  SetLiteral set;
  Reference reference;
  std::string text;
  std::vector<Value> elements;
};
struct Domain {
  bool present = false;
  SetLiteral integers; // Integer/Boolean domain or set upper bound.
  double lower = 0, upper = 0; // Float domain, when present.
};
struct Variable {
  Reference reference;
  std::string name;
  bool alias = false;
  Reference target; // meaningful only when alias; same namespace.
  bool assigned = false; // meaningful only when !alias.
  Value value; // assigned literal, meaningful only when !alias && assigned.
  Domain domain; // meaningful only when !alias; alias restrictions are rows.
  bool introduced = false, functionally_defined = false;
};
struct Constraint {
  std::string id;
  std::vector<Value> arguments;
  std::vector<Value> annotations;
  Location location;
  bool synthesized = false;
};
struct DeclarationAnnotations {
  std::string name;
  std::vector<Value> annotations;
  Location location;
};
enum class CoverageKind { Retained, AliasEquality };
struct Coverage {
  CoverageKind kind = CoverageKind::Retained;
  std::size_t raw_constraint = 0;
};
enum class Method { Satisfy, Minimize, Maximize };
struct SolveGoal {
  Method method = Method::Satisfy;
  bool has_objective = false;
  Value objective; // original literal or typed variable reference, never dummy 0.
  std::vector<Value> annotations;
  Location location;
};
/** The original output expression is a typed value or an Array layout containing
 * String fragments and a typed value Array (legacy arrayNd representation).
 * DeclarationAnnotations retains the original output_array dimensions as sets.
 * No printer shrinking/renumbering has occurred.
 */
struct Output { std::string name; Value expression; };
struct Records {
  // Raw state after declarations, before constraint equality rewrites.
  // Each namespace is indexed by Variable.reference.index, never by name.
  std::vector<Variable> raw_variables;
  std::vector<Constraint> raw_domains;
  std::vector<Constraint> raw_constraints;
  // Normalized alias state and both complete pre-posting constraint vectors.
  std::vector<Variable> variables;
  std::vector<Constraint> domains;
  std::vector<Constraint> constraints;
  std::vector<Coverage> coverage;
  std::vector<DeclarationAnnotations> declaration_annotations;
  SolveGoal solve;
  std::vector<Output> output;
  std::string source;
};
enum class Status { Complete, InvalidInput, Unsupported, ResourceLimit };
struct Diagnostic { Status status = Status::InvalidInput; Location location; std::string message; };
struct Options {
  std::size_t max_input_bytes = 16 * 1024 * 1024;
  std::size_t max_variables = 1000000;
  std::size_t max_array_elements = 1000000;
  std::size_t max_constraints = 1000000;
  std::size_t max_value_depth = 64;
  std::string source = "<stream>";
};
struct Result {
  Status status = Status::InvalidInput;
  // Complete means parsing/capture completed, NOT that a solver supports it.
  // Unknown ordinary predicates are retained and require compiler rejection.
  // Non-Complete never publishes partial records. Shared immutable ownership
  // survives parser/input destruction and is safe for independent readers.
  std::shared_ptr<const Records> records;
  std::vector<Diagnostic> diagnostics;
};

}}}
#endif
