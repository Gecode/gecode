#include <gecode/optimize/quadratic_bound.hpp>
#include <algorithm>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace Gecode { namespace Optimize { namespace Detail {
namespace {
void finite(QpInterval v) {
  if (!std::isfinite(v.lower) || !std::isfinite(v.upper) || v.lower > v.upper)
    throw std::overflow_error("Unavailable QP enclosure");
}
QpInterval widen(double lower, double upper) {
  QpInterval r{std::nextafter(lower,-std::numeric_limits<double>::infinity()),
               std::nextafter(upper,std::numeric_limits<double>::infinity())};
  finite(r); return r;
}
void checkpoint(const SolveBudget* b) {
  if (b && b->expired()) throw std::overflow_error("QP bound budget exhausted");
}
QpInterval affine(const std::vector<Term>& terms, double offset, const std::vector<double>& x) {
  auto r = qp_point(offset);
  for (const auto& t : terms) r = qp_add(r,qp_multiply(qp_point(t.coefficient),qp_point(x.at(t.variable.id))));
  return r;
}
}
bool quadratic_arithmetic_supported() noexcept {
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ != 0)
  return false;
#else
  if (!std::numeric_limits<double>::is_iec559 || FLT_RADIX != 2 || DBL_MANT_DIG != 53 ||
      sizeof(double) != 8 || std::fegetround() != FE_TONEAREST) return false;
  // Reject flush-to-zero/denormals-are-zero environments as well as other formats.
  volatile double tiny = std::numeric_limits<double>::denorm_min();
  volatile double two = 2.0;
  volatile double preserved = tiny * two;
  return preserved == std::numeric_limits<double>::denorm_min()*2 && preserved != 0;
#endif
}
QpInterval qp_point(double x) { QpInterval r{x,x}; finite(r); return r; }
QpInterval qp_negate(QpInterval x) { finite(x); return {-x.upper,-x.lower}; }
QpInterval qp_add(QpInterval a, QpInterval b) {
  finite(a); finite(b);
  if (a.lower == 0 && a.upper == 0) return b;
  if (b.lower == 0 && b.upper == 0) return a;
  volatile double lo = a.lower+b.lower, hi = a.upper+b.upper;
  return widen(lo,hi);
}
QpInterval qp_multiply(QpInterval a, QpInterval b) {
  finite(a); finite(b);
  if ((a.lower == 0 && a.upper == 0) || (b.lower == 0 && b.upper == 0)) return {0,0};
  if (a.lower == 1 && a.upper == 1) return b;
  if (b.lower == 1 && b.upper == 1) return a;
  if (a.lower == -1 && a.upper == -1) return qp_negate(b);
  if (b.lower == -1 && b.upper == -1) return qp_negate(a);
  volatile double p0 = a.lower*b.lower, p1 = a.lower*b.upper;
  volatile double p2 = a.upper*b.lower, p3 = a.upper*b.upper;
  const double lo = std::min(std::min(double(p0),double(p1)),std::min(double(p2),double(p3)));
  const double hi = std::max(std::max(double(p0),double(p1)),std::max(double(p2),double(p3)));
  return widen(lo,hi);
}
QuadraticBound quadratic_bound(const QuadraticSnapshot& q, const std::vector<double>& x,
                              const std::vector<double>& tangents,
                              const std::vector<double>& duals, const SolveBudget* budget) {
  QuadraticBound out;
  if (!quadratic_arithmetic_supported()) return out;
  try {
    checkpoint(budget);
    if (tangents.size() != q.squares().size() || duals.size() != q.rows().size()) return out;
    const double sign = q.linear_part().sense == ObjectiveSense::Minimize ? 1 : -1;
    std::vector<QpInterval> r(q.variables().size(),{0,0});
    for (const auto& t : q.linear_part().terms) r[t.variable.id] = qp_point(sign*t.coefficient);
    auto k = qp_point(0);
    for (std::size_t i=0; i<q.squares().size(); ++i) {
      checkpoint(budget);
      const auto& sq = q.squares()[i];
      const auto w = qp_point(sq.weight), t = qp_point(tangents[i]);
      const auto slope = qp_multiply(qp_point(2),qp_multiply(w,t));
      k = qp_add(k,qp_add(qp_multiply(slope,qp_point(sq.offset)),
                         qp_negate(qp_multiply(w,qp_multiply(t,t)))));
      for (const auto& a : sq.terms)
        r[a.variable.id] = qp_add(r[a.variable.id],qp_multiply(slope,qp_point(a.coefficient)));
    }
    for (const auto& row : q.rows()) if (row.active) {
      checkpoint(budget);
      double d = duals[row.constraint.id];
      if (!std::isfinite(d)) return out;
      const double side = d >= 0 ? row.lower : row.upper;
      if (!std::isfinite(side)) d = 0;
      if (!d) continue;
      k = qp_add(k,qp_multiply(qp_point(d),qp_point(side)));
      for (const auto& a : row.terms)
        r[a.variable.id] = qp_add(r[a.variable.id],qp_negate(qp_multiply(qp_point(d),qp_point(a.coefficient))));
    }
    auto centered = k;
    for (const auto& v : q.variables()) if (v.active) {
      checkpoint(budget);
      const auto product = qp_multiply(r[v.variable.id],{v.lower,v.upper});
      // Only this minimum's lower endpoint is used in the proof.
      centered = qp_add(centered,qp_point(product.lower));
    }
    out.normalized_lower = qp_add(centered,qp_point(sign*q.linear_part().offset)).lower;
    if (x.size() == q.variables().size()) {
      auto p = affine(q.linear_part().terms,0,x);
      if (sign < 0) p = qp_negate(p);
      for (const auto& sq : q.squares()) {
        checkpoint(budget);
        const auto residual = affine(sq.terms,sq.offset,x);
        p = qp_add(p,qp_multiply(qp_point(sq.weight),qp_multiply(residual,residual)));
      }
      // Offsets cancel algebraically before enclosure, even when huge.
      out.gap_upper = qp_add(p,qp_negate(qp_point(centered.lower))).upper;
    }
    checkpoint(budget);
  } catch (const std::overflow_error&) { return {}; }
    catch (const std::out_of_range&) { return {}; }
  return out;
}
}}}
