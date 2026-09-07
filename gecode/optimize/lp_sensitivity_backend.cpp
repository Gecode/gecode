#include <gecode/optimize/lp_sensitivity_backend.hpp>
#include <gecode/optimize/lp_basis_detail.hpp>
#include <cmath>
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
#include <gecode/optimize/lp_sensitivity_highs_detail.hpp>
#endif
namespace Gecode { namespace Optimize { namespace Detail {
namespace {
[[noreturn]] void fail(LpSensitivityReason reason,const char* text) {
  throw LpSensitivityBackendError(reason,text);
}
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
void check(const SolveBudget& budget) {
  if(budget.expired())fail(LpSensitivityReason::Stopped,"LP factorization budget stopped");
}
void ok(HighsStatus status,const char* text) {
  if(status!=HighsStatus::kOk)fail(LpSensitivityReason::BackendFailure,text);
}
HighsBasisStatus status(LpBasisStatus input) {
  switch(input) {
  case LpBasisStatus::Lower:return HighsBasisStatus::kLower;
  case LpBasisStatus::Upper:return HighsBasisStatus::kUpper;
  case LpBasisStatus::Basic:return HighsBasisStatus::kBasic;
  case LpBasisStatus::Zero:return HighsBasisStatus::kZero;
  case LpBasisStatus::NonbasicUnspecified:break;
  }
  fail(LpSensitivityReason::InvalidBasis,"Sensitivity requires explicit basis statuses");
}
struct Factor final : LpSensitivityFactor {
  Highs backend;
  std::vector<double> solve(const std::vector<double>& rhs,bool transpose) override {
    if(rhs.size()!=rows.size()||rhs.empty())
      fail(LpSensitivityReason::InvalidBasis,"Invalid basis-system RHS size");
    for(double x:rhs)if(!std::isfinite(x))
      fail(LpSensitivityReason::FailedLinearSolveChecks,"Nonfinite basis-system RHS");
    std::vector<double> values(rows.size(),0);
    ok(transpose?backend.getBasisTransposeSolve(rhs.data(),values.data()):
                 backend.getBasisSolve(rhs.data(),values.data()),"HiGHS basis-system solve failed");
    for(double x:values)if(!std::isfinite(x))
      fail(LpSensitivityReason::FailedLinearSolveChecks,"Nonfinite basis-system solution");
    return values;
  }
};
#endif
}
std::unique_ptr<LpSensitivityFactor> make_lp_sensitivity_factor(
  const ModelSnapshot& source,const LpBasis& basis,double tolerance,const SolveBudget& budget) {
#ifndef GECODE_OPTIMIZE_WITH_HIGHS
  (void)source;(void)basis;(void)tolerance;(void)budget;
  fail(LpSensitivityReason::Unsupported,"This build has no HiGHS LP factorization backend");
#else
  check(budget);
  LpBasisAccess::compatible(basis,source,budget);
  auto compiled=compile_lp_sensitivity(source,tolerance);
  check(budget);
  if(compiled.rows.empty())fail(LpSensitivityReason::NoBasis,"Zero-row sensitivity is not implemented");
  auto out=std::make_unique<Factor>();
  out->columns=std::move(compiled.columns);out->rows=std::move(compiled.rows);
  out->version=out->backend.version();
  ok(out->backend.setOptionValue("output_flag",false),"LP factorization log setup failed");
  ok(out->backend.setOptionValue("threads",1),"LP factorization thread setup failed");
  ok(out->backend.setOptionValue("presolve","off"),"LP factorization presolve setup failed");
  ok(out->backend.setOptionValue("time_limit",budget.remaining_seconds()),"LP factorization time setup failed");
  ok(out->backend.passModel(compiled.lp),"LP factorization model load failed or changed input");
  check(budget);
  HighsBasis requested;requested.alien=false;requested.valid=true;requested.useful=true;
  for(auto slot:out->columns)requested.col_status.push_back(status(*basis.columns()[slot]));
  for(auto slot:out->rows)requested.row_status.push_back(status(*basis.rows()[slot]));
  ok(out->backend.setBasis(requested,"Gecode original LP sensitivity basis"),"LP sensitivity basis rejected");
  check(budget);
  std::vector<HighsInt> order(out->rows.size());
  if(out->backend.getBasicVariables(order.data())!=HighsStatus::kOk||!out->backend.hasInvert())
    fail(LpSensitivityReason::InvalidBasis,"Selected LP basis is singular or cannot be factored without repair");
  check(budget);
  const auto& actual=out->backend.getBasis();
  if(!actual.valid||actual.col_status!=requested.col_status||actual.row_status!=requested.row_status)
    fail(LpSensitivityReason::ChangedBasis,"Backend changed selected sensitivity basis");
  std::vector<bool> seen_columns(out->columns.size()),seen_rows(out->rows.size());
  for(HighsInt index:order) {
    if(index>=0) {
      const auto k=static_cast<std::size_t>(index);
      if(k>=out->columns.size()||seen_columns[k]||requested.col_status[k]!=HighsBasisStatus::kBasic)
        fail(LpSensitivityReason::ChangedBasis,"Invalid structural factor basis mapping");
      seen_columns[k]=true;out->order.push_back(source.variables[out->columns[k]].variable);
    } else {
      const auto k=static_cast<std::size_t>(-(index+1));
      if(k>=out->rows.size()||seen_rows[k]||requested.row_status[k]!=HighsBasisStatus::kBasic)
        fail(LpSensitivityReason::ChangedBasis,"Invalid logical factor basis mapping");
      seen_rows[k]=true;out->order.push_back(source.rows[out->rows[k]].constraint);
    }
  }
  // The private model remains unsolved: no optimization run or status is invented.
  if(out->backend.getModelStatus()!=HighsModelStatus::kNotset||out->backend.getInfo().valid)
    fail(LpSensitivityReason::BackendFailure,"Unexpected optimization state during factor-only analysis");
  check(budget);return out;
#endif
}
}}}
