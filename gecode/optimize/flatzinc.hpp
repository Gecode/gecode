/* Explicit compiler for complete owning FlatZinc source records. */
#ifndef GECODE_OPTIMIZE_FLATZINC_HPP
#define GECODE_OPTIMIZE_FLATZINC_HPP

#include <gecode/flatzinc/capture-records.hh>
#include <gecode/optimize/model.hpp>
#include <gecode/optimize/result.hpp>

namespace Gecode { namespace Optimize {

enum class FlatZincCompileStatus {
  Complete, Unsupported, InvalidInput, ResourceLimit, TimeLimit, Cancelled, Error
};
struct FlatZincCompileOptions {
  std::size_t max_variables=1000000, max_constraints=1000000;
  std::size_t max_nonzeros=2000000, max_work=16000000, max_value_depth=64;
  double time_limit_seconds=std::numeric_limits<double>::infinity();
  std::shared_ptr<CancellationToken> cancellation;
};
struct FlatZincVariableMapping {
  FlatZinc::Capture::Reference source;
  Variable variable;
};
namespace Detail { struct FlatZincArtifact; struct FlatZincCompiler; }
/** Immutable original source and compiled model; no parser or Space lifetime. */
class CompiledFlatZinc {
  std::shared_ptr<const Detail::FlatZincArtifact> data_;
  explicit CompiledFlatZinc(std::shared_ptr<const Detail::FlatZincArtifact> data);
  friend struct Detail::FlatZincCompiler;
public:
  const ModelSnapshot& model() const;
  const FlatZinc::Capture::Records& source() const;
  const std::vector<FlatZincVariableMapping>& variables() const;
};
struct FlatZincCompileResult {
  FlatZincCompileStatus status=FlatZincCompileStatus::InvalidInput;
  std::optional<CompiledFlatZinc> compiled;
  FlatZinc::Capture::Location location;
  std::string message;
  std::size_t work=0;
};
/**
 * Compiles authoritative raw_variables/raw_domains/raw_constraints. Normalized
 * parser records are audit data, never a replacement for an original predicate.
 * Only a fully admitted model is published. No backend is called here.
 * Admits bounded integer/Boolean linear/Boolean relations, <= reification and
 * implication, integer equality implication, AllDifferent, 1-based integer
 * Element, positive-arity gecode_table_int and bounded finite integer domains
 * with holes; explicit-offset nonempty Circuit, fixed four-argument Cumulative,
 * and six-argument Regular with literal automaton parameters are also admitted.
 * Equality reification, other global/signature forms, floats/sets and unknown
 * controls remain Unsupported. Invalid literal schemas are InvalidInput.
 * Private gates and fixed literal slots are not source outputs.
 */
FlatZincCompileResult compile_flatzinc(const FlatZinc::Capture::Records& source,
                                     const FlatZincCompileOptions& options={});
struct FlatZincValidation {
  bool valid=false;
  std::optional<double> original_objective;
  /** Values in variables() order, including each repeated alias. */
  std::vector<std::int64_t> source_values;
  std::string message;
};
/** Recheck identity, all model slots and every original source relation exactly
 * after tolerance-qualified integer rounding. Does not certify the solve bound.
 */
FlatZincValidation validate_flatzinc(const CompiledFlatZinc&, const SolveResult&,
                                    double integrality_tolerance=1e-6);
/** Buffered assignments and a solution separator; never an optimality or
 * enumeration-completion marker. Rejects invalid full-model witnesses.
 */
std::string format_flatzinc_solution(const CompiledFlatZinc&, const SolveResult&,
                                     double integrality_tolerance=1e-6);

}}
#endif
