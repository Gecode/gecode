#include <gecode/optimize/quadratic_bound.hpp>
#include <gecode/optimize/validate.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
#include <Highs.h>
#endif
namespace Gecode { namespace Optimize {
namespace {
using Detail::QuadraticRaw;
struct Unsupported : std::runtime_error { using std::runtime_error::runtime_error; };
struct Stopped { Termination reason; };
void checkpoint(const SolveBudget& budget) {
  if (auto reason = budget.stop_reason()) throw Stopped{*reason};
}
void event(const char* name) {
#ifdef GECODE_QUADRATIC_TEST_HOOKS
  Detail::quadratic_test_event(name);
#else
  (void)name;
#endif
}
void support(const QuadraticOptions& o) {
  if (o.solve.backend != Backend::Auto && o.solve.backend != Backend::Highs)
    throw Unsupported("QP requires the HiGHS backend");
  if (o.solve.guarantee != Guarantee::Numerical)
    throw Unsupported("QP supports Numerical guarantees only");
  if (o.solve.threads != 1 || o.solve.random_seed != 0 || o.solve.node_limit || !o.solve.primal_start.empty())
    throw Unsupported("QP supports one worker, zero seed, no node quota or primal start");
  if (!Detail::quadratic_arithmetic_supported())
    throw Unsupported("QP requires IEEE binary64 nearest rounding without fast-math or flush-to-zero");
}
class Sum {
  long double s_=0,c_=0;
public:
  void add(long double x) { const auto n=s_+x; c_+=std::abs(s_)>=std::abs(x)?(s_-n)+x:(x-n)+s_; s_=n; }
  long double value() const { return s_+c_; }
};
long double activity(const std::vector<Term>& terms, const std::vector<double>& x, double offset=0) {
  Sum sum; sum.add(offset);
  for (const auto& t : terms) sum.add(static_cast<long double>(t.coefficient)*x[t.variable.id]);
  return sum.value();
}
void check_kkt(const QuadraticSnapshot& q, const QuadraticRaw& raw,
               const QuadraticOptions& o, QuadraticValidation& checked, const SolveBudget& budget) {
  if (!raw.dual_valid || raw.row_duals.size()!=q.rows().size() ||
      raw.column_duals.size()!=q.variables().size()) return;
  for (const auto& row : q.rows()) if (row.active && !std::isfinite(raw.row_duals[row.constraint.id])) return;
  for (const auto& v : q.variables()) if (v.active && !std::isfinite(raw.column_duals[v.variable.id])) return;
  checked.kkt_available = true;
  bool valid = true;
  const int sign = q.linear_part().sense == ObjectiveSense::Minimize ? 1 : -1;
  std::vector<Sum> residual(q.variables().size());
  std::vector<long double> scales(q.variables().size(),1);
  for (const auto& v : q.variables()) if (v.active) {
    const auto j=v.variable.id;
    residual[j].add(sign*checked.original_gradient[j]);
    residual[j].add(-raw.column_duals[j]);
    scales[j]+=std::abs(checked.original_gradient[j])+std::abs(raw.column_duals[j]);
  }
  auto complementary = [&](long double d, long double slack, bool side_finite) {
    if (d==0) return;
    if (!side_finite) { valid=false; return; }
    const auto magnitude=std::abs(d*slack);
    if (!std::isfinite(magnitude)) { valid=false; return; }
    checked.max_complementarity=std::max(checked.max_complementarity,static_cast<double>(magnitude));
    if (magnitude>o.complementarity_tolerance) valid=false;
  };
  for (const auto& row : q.rows()) if (row.active) {
    checkpoint(budget);
    const double d=raw.row_duals[row.constraint.id];
    const double side=d>=0?row.lower:row.upper;
    complementary(d,std::isfinite(side)?activity(row.terms,raw.values,-side):0,std::isfinite(side));
    for (const auto& t : row.terms) {
      const auto value=static_cast<long double>(t.coefficient)*d;
      residual[t.variable.id].add(-value);
      scales[t.variable.id]+=std::abs(value);
    }
  }
  for (const auto& v : q.variables()) if (v.active) {
    checkpoint(budget);
    const auto j=v.variable.id;
    const double d=raw.column_duals[j], side=d>=0?v.lower:v.upper;
    complementary(d,static_cast<long double>(raw.values[j])-side,true);
    const auto magnitude=std::abs(residual[j].value());
    if (!std::isfinite(magnitude) || !std::isfinite(scales[j])) { valid=false; continue; }
    checked.max_stationarity=std::max(checked.max_stationarity,static_cast<double>(magnitude));
    if (magnitude>o.stationarity_tolerance*scales[j]) valid=false;
  }
  checked.kkt_valid=valid;
}
void accept(const QuadraticSnapshot& q, const QuadraticOptions& o, const QuadraticRaw& raw,
            QuadraticResult& out, const SolveBudget& budget) {
  checkpoint(budget);
  auto& result=out.result;
  result.termination=raw.termination;
  result.message=raw.message;
  out.qp_iterations=raw.iterations;
  out.regularization=raw.regularization;
  if ((raw.objective && !std::isfinite(*raw.objective)) ||
      (raw.dual_estimate && !std::isfinite(*raw.dual_estimate))) {
    result.termination=Termination::NumericalFailure; result.message="Nonfinite vendor objective evidence"; return;
  }
  out.vendor_objective=raw.objective; out.vendor_dual_estimate=raw.dual_estimate;
  if (raw.model_id!=q.id() || raw.revision!=q.revision() || raw.active_variables!=result.active_variables) {
    result.termination=Termination::NumericalFailure; result.message="QP raw result identity or layout mismatch"; return;
  }
  // Even an oracle declining value_valid cannot conceal a contradictory feasible witness.
  if (raw.values.size()==q.variables().size()) out.checks=validate_quadratic(q,raw.values,o.solve.feasibility_tolerance);
  const bool original_valid=out.checks.primal_valid && out.checks.objective_valid;
  if (raw.termination==Termination::Infeasible) {
    if (original_valid) { result.termination=Termination::NumericalFailure; result.message="Infeasibility contradicted by original primal witness"; }
    else result.message="HiGHS numerical infeasibility conclusion; no independently checked certificate";
    return;
  }
  if (raw.termination==Termination::Unbounded || raw.termination==Termination::InfeasibleOrUnbounded) {
    result.termination=Termination::NumericalFailure; result.message="Finite-box QP cannot be unbounded"; return;
  }
  if (!raw.value_valid || !original_valid || raw.residual_values.size()!=q.squares().size() || !raw.objective) {
    if (raw.termination==Termination::Optimal) result.termination=Termination::NumericalFailure;
    result.message="No independently validated original QP candidate"; return;
  }
  const int sign=q.linear_part().sense==ObjectiveSense::Minimize?1:-1;
  for (std::size_t i=0;i<q.squares().size();++i) {
    checkpoint(budget);
    const auto& sq=q.squares()[i];
    // Subtract before collapsing the compensated residual (large affine offsets).
    Sum difference; difference.add(sq.offset); difference.add(-raw.residual_values[i]);
    for (const auto& t:sq.terms) difference.add(static_cast<long double>(t.coefficient)*raw.values[t.variable.id]);
    const auto mismatch=difference.value();
    if (!std::isfinite(raw.residual_values[i]) || !std::isfinite(mismatch) ||
        std::abs(mismatch)>o.solve.feasibility_tolerance) {
      result.termination=Termination::NumericalFailure; result.message="Lifted residual disagrees with original square"; return;
    }
  }
  // Vendor objective is normalized. Its agreement cannot establish optimality.
  const long double mismatch=static_cast<long double>(*raw.objective)-sign*(*out.checks.original_objective);
  if (std::abs(mismatch)>o.optimality_tolerance || !std::isfinite(mismatch)) {
    result.termination=Termination::NumericalFailure; result.message="Vendor objective disagrees with original quadratic objective"; return;
  }
  check_kkt(q,raw,o,out.checks,budget);
  std::vector<double> duals(q.rows().size(),0);
  if (raw.dual_valid && raw.row_duals.size()==duals.size()) {
    bool finite=true;
    for (const auto& row:q.rows()) if(row.active) finite &= std::isfinite(raw.row_duals[row.constraint.id]);
    if (finite) duals=raw.row_duals;
  }
  const auto bound=Detail::quadratic_bound(q,raw.values,out.checks.square_values,duals,&budget);
  out.checks.normalized_lower_bound=bound.normalized_lower;
  out.checks.gap_upper_bound=bound.gap_upper;
  out.checks.bound_valid=bound.normalized_lower.has_value();
  event("after_validation"); checkpoint(budget);
  result.values=raw.values; result.objective=out.checks.original_objective;
  result.solution_validated=true;
  if (bound.normalized_lower) {
    result.best_bound=sign*(*bound.normalized_lower);
    try { result.update_gaps(q.linear_part().sense); }
    catch (const ModelError&) {
      result.best_bound.reset(); out.checks.bound_valid=false;
      result.termination=Termination::NumericalFailure;
      result.message="Verified feasible-set bound contradicts numerical primal objective"; return;
    }
  }
  if (raw.termination==Termination::Optimal &&
      (!out.checks.kkt_valid || !out.checks.bound_valid || !bound.gap_upper ||
       *bound.gap_upper<0 || *bound.gap_upper>o.optimality_tolerance)) {
    result.termination=Termination::NumericalFailure;
    result.message="Original KKT or offset-independent verified gap did not close";
  }
}
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
void ok(HighsStatus s, const char* what) {
  if (s!=HighsStatus::kOk) throw Unsupported(std::string("HiGHS rejected/modified QP input: ")+what);
}
void range(double x, double maximum, bool matrix=false) {
  if (std::isfinite(x) && (std::abs(x)>=maximum || (matrix && x!=0 && std::abs(x)<=1e-12)))
    throw Unsupported("QP data exceeds supported HiGHS coefficient/bound range; rescale explicitly");
}
Termination termination(HighsModelStatus s) {
  switch(s) {
    case HighsModelStatus::kOptimal: case HighsModelStatus::kModelEmpty:return Termination::Optimal;
    case HighsModelStatus::kInfeasible:return Termination::Infeasible;
    case HighsModelStatus::kUnbounded:return Termination::Unbounded;
    case HighsModelStatus::kUnboundedOrInfeasible:return Termination::InfeasibleOrUnbounded;
    case HighsModelStatus::kTimeLimit:return Termination::TimeLimit;
    case HighsModelStatus::kIterationLimit:return Termination::IterationLimit;
    case HighsModelStatus::kMemoryLimit:return Termination::MemoryLimit;
    case HighsModelStatus::kInterrupt: case HighsModelStatus::kHighsInterrupt:return Termination::Cancelled;
    default:return Termination::BackendError;
  }
}
QuadraticRaw backend(const QuadraticSnapshot& q, const QuadraticOptions& o, const SolveBudget& budget) {
  QuadraticRaw raw; raw.model_id=q.id(); raw.revision=q.revision();
  for(const auto& v:q.variables()) raw.active_variables.push_back(v.active);
  const auto max=static_cast<std::size_t>(std::numeric_limits<HighsInt>::max());
  if (q.squares().size()>o.max_auxiliary_variables || q.squares().size()>max ||
      o.iteration_limit>static_cast<std::uint64_t>(std::numeric_limits<HighsInt>::max()))
    throw Unsupported("QP auxiliary/iteration capacity exceeds supported range");
  HighsModel model;
  auto& lp=model.lp_;
  lp.sense_=ObjSense::kMinimize;
  const int sign=q.linear_part().sense==ObjectiveSense::Minimize?1:-1;
  lp.offset_=sign*q.linear_part().offset; range(lp.offset_,1e20);
  std::vector<HighsInt> columns(q.variables().size(),-1);
  std::vector<std::size_t> slots, rows;
  for(const auto& v:q.variables()) if(v.active) {
    checkpoint(budget);
    if(slots.size()+q.squares().size()>=max) throw Unsupported("QP dimension exceeds HiGHS range");
    range(v.lower,1e20);range(v.upper,1e20);
    columns[v.variable.id]=static_cast<HighsInt>(slots.size());slots.push_back(v.variable.id);
    lp.col_lower_.push_back(v.lower);lp.col_upper_.push_back(v.upper);lp.col_cost_.push_back(0);
  }
  for(const auto& t:q.linear_part().terms) { range(t.coefficient,1e20);lp.col_cost_[columns[t.variable.id]]=sign*t.coefficient; }
  const auto originals=slots.size();
  for(std::size_t k=0;k<q.squares().size();++k) {
    lp.col_lower_.push_back(-kHighsInf);lp.col_upper_.push_back(kHighsInf);lp.col_cost_.push_back(0);
  }
  auto& a=lp.a_matrix_;a.format_=MatrixFormat::kRowwise;a.start_.assign(1,0);
  auto add_entry=[&](HighsInt column,double value) {
    if(a.value_.size()>=o.max_lifted_nonzeros || a.value_.size()>=max) throw Unsupported("QP nonzero capacity exceeded");
    range(value,1e15,true);a.index_.push_back(column);a.value_.push_back(value);
  };
  for(const auto& row:q.rows()) if(row.active) {
    checkpoint(budget);
    range(row.lower,1e20);range(row.upper,1e20);
    if(rows.size()+q.squares().size()>=max) throw Unsupported("QP row capacity exceeded");
    rows.push_back(row.constraint.id);lp.row_lower_.push_back(row.lower);lp.row_upper_.push_back(row.upper);
    for(const auto& t:row.terms) add_entry(columns[t.variable.id],t.coefficient);
    a.start_.push_back(static_cast<HighsInt>(a.value_.size()));
  }
  for(std::size_t k=0;k<q.squares().size();++k) {
    checkpoint(budget);const auto& square=q.squares()[k];range(square.offset,1e20);
    lp.row_lower_.push_back(square.offset);lp.row_upper_.push_back(square.offset);
    for(const auto& t:square.terms) add_entry(columns[t.variable.id],-t.coefficient);
    add_entry(static_cast<HighsInt>(originals+k),1);a.start_.push_back(static_cast<HighsInt>(a.value_.size()));
  }
  lp.num_col_=static_cast<HighsInt>(lp.col_cost_.size());lp.num_row_=static_cast<HighsInt>(lp.row_lower_.size());
  a.num_col_=lp.num_col_;a.num_row_=lp.num_row_;
  auto& h=model.hessian_;
  h.dim_=lp.num_col_;h.format_=HessianFormat::kTriangular;h.start_.assign(1,0);
  for(HighsInt j=0;j<lp.num_col_;++j) {
    double value=static_cast<std::size_t>(j)<originals?0:2*q.squares()[j-originals].weight;
    if(!std::isfinite(value)) throw Unsupported("QP Hessian weight overflow");
    range(value,1e15,true);h.index_.push_back(j);h.value_.push_back(value);h.start_.push_back(j+1);
  }
  // The zero-variable case still uses the same original checker and bound gates.
  if(lp.num_col_==0) {
    raw.values.assign(q.variables().size(),std::numeric_limits<double>::quiet_NaN());
    raw.value_valid=true;raw.dual_valid=true;raw.row_duals.assign(q.rows().size(),0);
    raw.column_duals.assign(q.variables().size(),0);raw.objective=lp.offset_;
    raw.termination=Termination::Optimal;
    for(const auto& row:q.rows()) if(row.active && (row.lower>0 || row.upper<0)) { raw.termination=Termination::Infeasible;raw.values.clear();raw.value_valid=false; }
    return raw;
  }
  Highs highs;
  ok(highs.setOptionValue("output_flag",false),"output");
  ok(highs.setOptionValue("threads",1),"threads");
  ok(highs.setOptionValue("random_seed",0),"seed");
  ok(highs.setOptionValue("solver","qpasm"),"solver");
  ok(highs.setOptionValue("small_matrix_value",1e-12),"matrix threshold");
  double regularization=0;
#ifdef GECODE_QUADRATIC_TEST_HOOKS
  regularization=Detail::quadratic_test_regularization;
#endif
  raw.regularization=regularization;
  ok(highs.setOptionValue("qp_regularization_value",regularization),"regularization");
  ok(highs.setOptionValue("qp_iteration_limit",static_cast<HighsInt>(o.iteration_limit)),"iterations");
  ok(highs.setOptionValue("primal_feasibility_tolerance",o.solve.feasibility_tolerance),"primal tolerance");
  ok(highs.setOptionValue("dual_feasibility_tolerance",std::max(1e-10,o.stationarity_tolerance)),"dual tolerance");
  ok(highs.passModel(model),"upload");
  const auto& loaded=highs.getModel();
  const auto& p=loaded.lp_;
  if(p.num_col_!=lp.num_col_ || p.num_row_!=lp.num_row_ || p.offset_!=lp.offset_ ||
     p.sense_!=lp.sense_ || p.col_cost_!=lp.col_cost_ || p.col_lower_!=lp.col_lower_ || p.col_upper_!=lp.col_upper_ ||
     p.row_lower_!=lp.row_lower_ || p.row_upper_!=lp.row_upper_ || p.a_matrix_.value_.size()!=a.value_.size())
    throw Unsupported("HiGHS changed lifted QP model on upload");
  for(HighsInt i=0;i<lp.num_row_;++i) for(HighsInt z=a.start_[i];z<a.start_[i+1];++z) {
    checkpoint(budget);double value=0;ok(highs.getCoeff(i,a.index_[z],value),"coefficient audit");
    if(value!=a.value_[z]) throw Unsupported("HiGHS changed lifted QP coefficient");
  }
  const auto& hh=loaded.hessian_;
  if(!q.squares().empty()) {
    if(hh.dim_!=h.dim_) throw Unsupported("HiGHS changed QP Hessian dimension");
    for(HighsInt j=0;j<hh.dim_;++j) {
      double diagonal=0;
      for(HighsInt z=hh.start_[j];z<hh.start_[j+1];++z) {
        if(hh.index_[z]!=j && hh.value_[z]!=0) throw Unsupported("Unexpected off-diagonal Hessian");
        if(hh.index_[z]==j) diagonal+=hh.value_[z];
      }
      if(diagonal!=h.value_[j]) throw Unsupported("HiGHS changed QP curvature");
    }
  }
  event("before_backend");checkpoint(budget);
  ok(highs.setOptionValue("time_limit",budget.remaining_seconds()),"deadline");
  const auto status=highs.run();
  event("after_backend");checkpoint(budget);
  raw.termination=termination(highs.getModelStatus());
  if(status==HighsStatus::kError && raw.termination==Termination::Optimal) raw.termination=Termination::BackendError;
  const auto& solution=highs.getSolution();
  raw.value_valid=solution.value_valid;raw.dual_valid=solution.dual_valid;
  raw.values.assign(q.variables().size(),std::numeric_limits<double>::quiet_NaN());
  if(solution.col_value.size()==lp.col_cost_.size()) {
    for(std::size_t j=0;j<originals;++j) raw.values[slots[j]]=solution.col_value[j];
    raw.residual_values.assign(solution.col_value.begin()+static_cast<std::ptrdiff_t>(originals),solution.col_value.end());
  } else { raw.values.clear();raw.value_valid=false; }
  if(solution.col_dual.size()==lp.col_cost_.size() && solution.row_dual.size()==lp.row_lower_.size()) {
    raw.column_duals.assign(q.variables().size(),0);raw.row_duals.assign(q.rows().size(),0);
    for(std::size_t j=0;j<originals;++j) raw.column_duals[slots[j]]=solution.col_dual[j];
    for(std::size_t i=0;i<rows.size();++i) raw.row_duals[rows[i]]=solution.row_dual[i];
  } else raw.dual_valid=false;
  const auto& info=highs.getInfo();
  if(info.valid) {
    if(solution.value_valid) raw.objective=info.objective_function_value;
    raw.iterations=static_cast<std::uint64_t>(std::max(HighsInt{0},info.qp_iteration_count));
  }
  double dual=0;
  if(solution.dual_valid && highs.getDualObjectiveValue(dual)==HighsStatus::kOk) raw.dual_estimate=dual;
  return raw;
}
#endif
QuadraticResult run(const QuadraticSnapshot& q,const QuadraticOptions& o,const SolveBudget& budget) {
  QuadraticResult out;
  auto& r=out.result;r.model_id=q.id();r.revision=q.revision();r.backend="HiGHS QP";r.guarantee=o.solve.guarantee;
  for(const auto& v:q.variables()) r.active_variables.push_back(v.active);
  try {
    o.validate();support(o);checkpoint(budget);
    validate_structure(Detail::QuadraticAccess::core(q));
    event("after_preflight");checkpoint(budget);
    QuadraticRaw raw;
    bool supplied=false;
#ifdef GECODE_QUADRATIC_TEST_HOOKS
    supplied=Detail::quadratic_test_oracle(q,o,raw);
#endif
    if(!supplied) {
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
      r.backend_version=Highs{}.version();raw=backend(q,o,budget);
#else
      throw Unsupported("HiGHS quadratic backend is not available in this build");
#endif
    }
    checkpoint(budget);accept(q,o,raw,out,budget);checkpoint(budget);
  } catch(const Stopped& s) {
    // Candidates checked/published after a deadline are excluded completely.
    r.termination=s.reason;r.solution_validated=false;r.values.clear();r.objective.reset();r.best_bound.reset();
    r.absolute_gap.reset();r.relative_gap.reset();out.checks={};r.message="QP shared budget exhausted before publication";
  } catch(const Unsupported& e) {r.termination=Termination::Unsupported;r.message=e.what();}
    catch(const ModelError& e) {r.termination=Termination::InvalidModel;r.message=e.what();}
    catch(const std::bad_alloc&) {r.termination=Termination::MemoryLimit;r.message="QP allocation failed";}
    catch(const std::exception& e) {r.termination=Termination::BackendError;r.message=e.what();}
  if (auto reason=budget.stop_reason(); reason && r.termination!=Termination::Unsupported && r.termination!=Termination::InvalidModel) {
    r.termination=*reason;r.solution_validated=false;r.values.clear();r.objective.reset();r.best_bound.reset();
    r.absolute_gap.reset();r.relative_gap.reset();out.checks={};
  }
  r.elapsed_seconds=budget.elapsed_seconds();return out;
}
QuadraticResult failure(ModelId id,Revision rev,const QuadraticOptions& o,Termination t,const char* msg) {
  QuadraticResult out;out.result.model_id=id;out.result.revision=rev;out.result.guarantee=o.solve.guarantee;
  out.result.termination=t;out.result.message=msg;return out;
}
}
QuadraticResult solve_quadratic(const QuadraticSnapshot& q,const QuadraticOptions& o) {
  try {o.validate();SolveBudget b(o.solve);return run(q,o,b);}
  catch(const ModelError& e) {return failure(q.id(),q.revision(),o,Termination::InvalidModel,e.what());}
  catch(const std::bad_alloc&) {return failure(q.id(),q.revision(),o,Termination::MemoryLimit,"QP allocation failed");}
}
QuadraticResult solve_quadratic(const QuadraticModel& q,const QuadraticOptions& o) {
  try {o.validate();support(o);SolveBudget b(o.solve);checkpoint(b);return run(q.snapshot(),o,b);}
  catch(const Unsupported& e) {return failure(q.id(),q.revision(),o,Termination::Unsupported,e.what());}
  catch(const Stopped& s) {return failure(q.id(),q.revision(),o,s.reason,"QP budget exhausted before snapshot");}
  catch(const ModelError& e) {return failure(q.id(),q.revision(),o,Termination::InvalidModel,e.what());}
  catch(const std::bad_alloc&) {return failure(q.id(),q.revision(),o,Termination::MemoryLimit,"QP allocation failed");}
}
BackendCapabilities quadratic_capabilities() {
  BackendCapabilities c;c.name="HiGHS QP";
#ifdef GECODE_OPTIMIZE_WITH_HIGHS
  c.available=Detail::quadratic_arithmetic_supported();c.quadratic_programming=c.available;c.version=Highs{}.version();
#endif
  c.limitations={"Explicit finite-box continuous quadratic wrapper only", "Positive affine squares: convex min or concave max", "Numerical guarantee; checked original KKT and outward finite-box bound", "Requires IEEE binary64 nearest rounding without fast-math or flush-to-zero", "No arbitrary Hessian, MIQP, QCP, starts, session, native or Exact support", "Cooperative time limit; active-set cancellation checked before/after backend"};
  return c;
}
}}
