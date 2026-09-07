/* Internal bounded, exact transformations in the automatic native pipeline. */
#ifndef GECODE_OPTIMIZE_NATIVE_PREPROCESS_HPP
#define GECODE_OPTIMIZE_NATIVE_PREPROCESS_HPP
#include <gecode/optimize/native.hpp>
#include <functional>
#include <optional>
namespace Gecode { namespace Optimize { namespace Detail {
using NativeSolveContinuation = std::function<SolveResult(
  const ModelSnapshot&, const SolveOptions&, SolveBudget&)>;
std::optional<SolveResult> native_presolve(const ModelSnapshot&, const SolveOptions&,
  SolveBudget&, const NativeSolveContinuation&);
/** Eliminate one singleton affine objective auxiliary after native admission. */
std::optional<SolveResult> native_objective_auxiliary(const ModelSnapshot&, const SolveOptions&,
  SolveBudget&, const NativeSolveContinuation&);
std::optional<SolveResult> native_components(const ModelSnapshot&, const SolveOptions&,
  SolveBudget&, const NativeSolveContinuation&);
std::optional<SolveResult> native_symmetry(const ModelSnapshot&, const SolveOptions&,
  SolveBudget&, const NativeSolveContinuation&);
}}}
#endif
