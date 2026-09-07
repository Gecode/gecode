#include <gecode/optimize/quadratic_bound.hpp>
#include <gecode/optimize/validate.hpp>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
namespace Gecode { namespace Optimize {
struct QuadraticSnapshot::Data {
  ModelSnapshot core;
  std::vector<WeightedSquare> squares;
};
QuadraticSnapshot::QuadraticSnapshot(ModelSnapshot core, std::vector<WeightedSquare> squares)
  : data_(std::make_shared<Data>(Data{std::move(core), std::move(squares)})) {}
ModelId QuadraticSnapshot::id() const noexcept { return data_->core.model_id; }
Revision QuadraticSnapshot::revision() const noexcept { return data_->core.revision; }
const std::vector<VariableData>& QuadraticSnapshot::variables() const noexcept { return data_->core.variables; }
const std::vector<RowData>& QuadraticSnapshot::rows() const noexcept { return data_->core.rows; }
const ObjectiveData& QuadraticSnapshot::linear_part() const noexcept { return data_->core.objective; }
const std::vector<WeightedSquare>& QuadraticSnapshot::squares() const noexcept { return data_->squares; }
const ModelSnapshot& Detail::QuadraticAccess::core(const QuadraticSnapshot& q) { return q.data_->core; }
namespace {
void finite_box(double l, double u) {
  if (!std::isfinite(l) || !std::isfinite(u) || l > u)
    throw ModelError("Quadratic variables require finite ordered bounds");
}
class Sum {
  long double sum_ = 0, correction_ = 0;
public:
  void add(long double x) {
    const auto next = sum_ + x;
    correction_ += std::abs(sum_) >= std::abs(x) ? (sum_-next)+x : (x-next)+sum_;
    sum_ = next;
  }
  long double value() const { return sum_ + correction_; }
};
double finite_value(long double x) {
  const double d = static_cast<double>(x);
  if (!std::isfinite(x) || !std::isfinite(d)) throw ModelError("Quadratic evaluation overflow");
  return d;
}
}
Variable QuadraticModel::add_continuous(double l, double u, std::string name) {
  finite_box(l,u); return core_.add_continuous(l,u,std::move(name));
}
Constraint QuadraticModel::add_row(const std::vector<Term>& terms, double l, double u, std::string name) {
  return core_.add_row(terms,l,u,std::move(name));
}
void QuadraticModel::set_bounds(Variable v, double l, double u) { finite_box(l,u); core_.set_bounds(v,l,u); }
void QuadraticModel::set_bounds(Constraint r, double l, double u) { core_.set_bounds(r,l,u); }
void QuadraticModel::set_coefficient(Constraint r, Variable v, double a) { core_.set_coefficient(r,v,a); }
void QuadraticModel::remove(Constraint r) { core_.remove(r); }
void QuadraticModel::remove(Variable v) {
  core_.variable(v);
  for (const auto& square : squares_)
    for (const auto& t : square.terms)
      if (t.variable == v) throw ModelError("Variable is referenced by a square");
  core_.remove(v);
}
void QuadraticModel::objective(const std::vector<WeightedSquare>& squares,
                              const std::vector<Term>& linear, double offset, ObjectiveSense sense) {
  if (!core_.id()) throw ModelError("Moved-from quadratic model");
  auto staged = squares;
  for (auto& square : staged) {
    if (!std::isfinite(square.weight) || square.weight <= 0 || !std::isfinite(square.offset))
      throw ModelError("Square requires finite positive weight and finite offset");
    for (const auto& t : square.terms) {
      core_.variable(t.variable);
      if (!std::isfinite(t.coefficient)) throw ModelError("Nonfinite square coefficient");
    }
    std::stable_sort(square.terms.begin(), square.terms.end(), [](const Term& a, const Term& b) {
      return a.variable.id < b.variable.id;
    });
    std::vector<Term> canonical;
    for (std::size_t i=0; i<square.terms.size();) {
      const auto v = square.terms[i].variable;
      Sum sum;
      do { sum.add(square.terms[i++].coefficient); }
      while (i<square.terms.size() && square.terms[i].variable == v);
      const double c = finite_value(sum.value());
      if (c) canonical.push_back({v,c});
    }
    square.terms.swap(canonical);
  }
  core_.set_objective(linear,sense,offset);
  squares_.swap(staged);
}
void QuadraticModel::minimize_squares(const std::vector<WeightedSquare>& s, const std::vector<Term>& l, double o) {
  objective(s,l,o,ObjectiveSense::Minimize);
}
void QuadraticModel::maximize_concave_squares(const std::vector<WeightedSquare>& s, const std::vector<Term>& l, double o) {
  objective(s,l,o,ObjectiveSense::Maximize);
}
QuadraticSnapshot QuadraticModel::snapshot() const { return QuadraticSnapshot(core_.snapshot(),squares_); }
void QuadraticOptions::validate() const {
  solve.validate();
  for (double t : {stationarity_tolerance,complementarity_tolerance,optimality_tolerance})
    if (!std::isfinite(t) || t < 0) throw ModelError("QP checking tolerances must be finite and nonnegative");
}
QuadraticValidation validate_quadratic(const QuadraticSnapshot& model,
                                     const std::vector<double>& values, double tolerance) {
  QuadraticValidation out;
  try {
    const auto& core = Detail::QuadraticAccess::core(model);
    const auto linear = validate(core,values,tolerance);
    out.primal_valid = linear.valid;
    if (!linear.valid) { out.message = linear.message; return out; }
    const int sign = core.objective.sense == ObjectiveSense::Minimize ? 1 : -1;
    Sum objective;
    objective.add(core.objective.offset);
    std::vector<Sum> gradient(core.variables.size());
    for (const auto& t : core.objective.terms) {
      objective.add(static_cast<long double>(t.coefficient)*values[t.variable.id]);
      gradient[t.variable.id].add(t.coefficient);
    }
    for (const auto& square : model.squares()) {
      Sum residual; residual.add(square.offset);
      for (const auto& t : square.terms)
        residual.add(static_cast<long double>(t.coefficient)*values[t.variable.id]);
      const auto r = residual.value();
      out.square_values.push_back(finite_value(r));
      objective.add(sign*static_cast<long double>(square.weight)*r*r);
      for (const auto& t : square.terms)
        gradient[t.variable.id].add(2*sign*static_cast<long double>(square.weight)*r*t.coefficient);
    }
    out.original_objective = finite_value(objective.value());
    out.original_gradient.resize(core.variables.size(),0);
    for (const auto& v : core.variables) if (v.active)
      out.original_gradient[v.variable.id] = finite_value(gradient[v.variable.id].value());
    out.objective_valid = true;
  } catch (const ModelError& e) { out.message = e.what(); }
  return out;
}
}}
