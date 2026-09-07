/* Private existing-adapter bridge. Deliberately excluded from installation. */
#ifndef GECODE_OPTIMIZE_LP_SENSITIVITY_HIGHS_DETAIL_HPP
#define GECODE_OPTIMIZE_LP_SENSITIVITY_HIGHS_DETAIL_HPP
#include <gecode/optimize/model.hpp>
#include <Highs.h>
namespace Gecode { namespace Optimize { namespace Detail {
struct LpSensitivityCompiled {
  HighsLp lp;
  std::vector<std::size_t> columns,rows;
};
LpSensitivityCompiled compile_lp_sensitivity(const ModelSnapshot&,double primal_tolerance);
}}}
#endif
