/* Private validation seam; not installed. */
#ifndef GECODE_OPTIMIZE_LP_BASIS_DETAIL_HPP
#define GECODE_OPTIMIZE_LP_BASIS_DETAIL_HPP
#include <gecode/optimize/lp_basis.hpp>
namespace Gecode { namespace Optimize { namespace Detail {
struct LpBasisAccess {
  static std::shared_ptr<const LpBasis> create(LpBasisData, LpBasisOrigin);
  static void validate_statuses(const ModelSnapshot&,
    const std::vector<std::optional<LpBasisStatus>>& rows,
    const std::vector<std::optional<LpBasisStatus>>& columns,
    const SolveBudget* budget = nullptr);
  static void compatible(const LpBasis&, const ModelSnapshot&, const SolveBudget&);
};
}}}
#endif
