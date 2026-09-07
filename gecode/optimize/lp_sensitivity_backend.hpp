/* Private factorization interface: never installed. No optimization operation. */
#ifndef GECODE_OPTIMIZE_LP_SENSITIVITY_BACKEND_HPP
#define GECODE_OPTIMIZE_LP_SENSITIVITY_BACKEND_HPP
#include <gecode/optimize/lp_sensitivity.hpp>
namespace Gecode { namespace Optimize { namespace Detail {
struct LpSensitivityFactor {
  std::vector<std::size_t> columns,rows;
  std::vector<LpSensitivityEntity> order;
  std::string version;
  virtual ~LpSensitivityFactor()=default;
  virtual std::vector<double> solve(const std::vector<double>&,bool transpose)=0;
};
class LpSensitivityBackendError : public std::runtime_error {
public:
  LpSensitivityReason reason;
  LpSensitivityBackendError(LpSensitivityReason r,const char* message)
    :std::runtime_error(message),reason(r){}
};
std::unique_ptr<LpSensitivityFactor> make_lp_sensitivity_factor(
  const ModelSnapshot&,const LpBasis&,double primal_tolerance,const SolveBudget&);
}}}
#endif
