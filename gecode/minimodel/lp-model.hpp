/* Experimental binary and bounded-integer linear models. SPDX-License-Identifier: MIT */
#ifndef GECODE_MINIMODEL_LP_MODEL_HPP
#define GECODE_MINIMODEL_LP_MODEL_HPP

#include <gecode/int.hh>
#include <gecode/minimodel/lp-certificate.hpp>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <algorithm>
#include <utility>
#include <vector>

namespace Gecode { namespace Experimental { namespace LpRelaxation {

/// Binary minimization: min c*x, subject to a*x >= b. Dense row-major a.
/// Kept unchanged for source compatibility, including mutable public data.
struct LinearModel {
  std::vector<std::int64_t> a, b, c;
};

/** Binary minimization in canonical compressed sparse row (CSR) storage.
 * a[k] belongs to column[k]; row i occupies [row_start[i],row_start[i+1]).
 * Each row's columns must be strictly increasing and values must be nonzero.
 * The default value describes the empty model. No dense shadow is retained.
 */
struct SparseLinearModel {
  std::vector<std::size_t> row_start{0}, column;
  std::vector<std::int64_t> a, b, c;

  LpCertificate::SparseMatrixView matrix() const {
    return {row_start,column,a,c.size()};
  }
  std::size_t nonzeros() const { return a.size(); }
};

/// Convert the current dense value, never a cached copy of mutable input.
inline SparseLinearModel sparse_model(const LinearModel& model) {
  const auto n=model.c.size(),m=model.b.size();
  if (m==std::numeric_limits<std::size_t>::max() ||
      (n && m>std::numeric_limits<std::size_t>::max()/n) || model.a.size()!=m*n)
    throw std::invalid_argument("Binary linear model dimensions");
  SparseLinearModel result;
  result.b=model.b; result.c=model.c;
  result.row_start.reserve(m+1);
  for (std::size_t i=0;i<m;++i) {
    for (std::size_t j=0;j<n;++j)
      if (model.a[i*n+j]) {
        result.column.push_back(j);
        result.a.push_back(model.a[i*n+j]);
      }
    result.row_start.push_back(result.a.size());
  }
  return result;
}

// Constrained overloads preserve legacy calls using an untyped {} model.
template<class Model,
         typename std::enable_if<std::is_same<Model,SparseLinearModel>::value,int>::type=0>
inline void validate_model(const Model& model) {
  const auto n=model.c.size(),m=model.b.size();
  if (n>static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
      m>static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
      !LpCertificate::valid_sparse(model.matrix(),m,n))
    throw std::invalid_argument("Binary sparse model needs canonical CSR dimensions/indices");
  for (const auto* values : {&model.a,&model.b,&model.c})
    for (auto value : *values)
      if (value < -1000000000LL || value > 1000000000LL)
        throw std::invalid_argument("Binary linear coefficients must have magnitude <= 1e9");
  // n<=INT_MAX and abs(c)<=1e9 keep both sums within signed int64.
  std::int64_t lo=0,hi=0;
  for (auto value : model.c) { if (value<0) lo+=value; else hi+=value; }
  if (lo<Int::Limits::min || hi>Int::Limits::max)
    throw std::invalid_argument("Binary objective range exceeds Gecode integer limits");
}

inline void validate_model(const LinearModel& model) {
  const auto n=model.c.size(), m=model.b.size();
  if (n>static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
      m>static_cast<std::size_t>(std::numeric_limits<int>::max()) ||
      (n && m>std::numeric_limits<std::size_t>::max()/n) || model.a.size()!=m*n)
    throw std::invalid_argument("Binary linear model dimensions");
  for (const auto* values : {&model.a,&model.b,&model.c})
    for (auto value : *values)
      if (value < -1000000000LL || value > 1000000000LL)
        throw std::invalid_argument("Binary linear coefficients must have magnitude <= 1e9");
  std::int64_t lo=0,hi=0;
  for (auto value : model.c) { if (value<0) lo+=value; else hi+=value; }
  if (lo<Int::Limits::min || hi>Int::Limits::max)
    throw std::invalid_argument("Binary objective range exceeds Gecode integer limits");
}

/// Post original sparse constraints without allocating or scanning dense rows.
template<class Model,
         typename std::enable_if<std::is_same<Model,SparseLinearModel>::value,int>::type=0>
inline void post_native(Home home,const IntVarArgs& x,IntVar objective,
                        const Model& model) {
  validate_model(model);
  if (model.c.size()!=static_cast<std::size_t>(x.size()))
    throw Int::ArgumentSizeMismatch("LpRelaxation::post_native");
  if (home.failed()) return;
  dom(home,x,0,1);
  for (std::size_t i=0;i<model.b.size();++i) {
    IntArgs coefficients;
    IntVarArgs variables;
    for (std::size_t k=model.row_start[i];k<model.row_start[i+1];++k) {
      coefficients << static_cast<int>(model.a[k]);
      variables << x[static_cast<int>(model.column[k])];
    }
    linear(home,coefficients,variables,IRT_GQ,static_cast<int>(model.b[i]),IPL_BND);
  }
  IntArgs costs(x.size());
  for (int j=0;j<x.size();++j) costs[j]=static_cast<int>(model.c[j]);
  linear(home,costs,x,IRT_EQ,objective,IPL_BND);
}

/// Legacy dense posting reads the current input and converts exactly once.
inline void post_native(Home home,const IntVarArgs& x,IntVar objective,
                        const LinearModel& model) {
  post_native(home,x,objective,sparse_model(model));
}

/** Explicit finite integer domains over a sparse linear model. No objective
 * offset or fractional coefficient/lattice is implied by this integer API.
 */
struct BoundedIntegerModel {
  SparseLinearModel linear;
  std::vector<std::int64_t> lower,upper;
};

namespace Detail {
inline std::pair<int,int> validate_integer_native(const BoundedIntegerModel& model) {
  const auto& linear=model.linear;
  const auto n=linear.c.size(),m=linear.b.size();
  if (n>static_cast<std::size_t>(Int::Limits::max) ||
      m>static_cast<std::size_t>(Int::Limits::max) ||
      model.lower.size()!=n || model.upper.size()!=n ||
      !LpCertificate::valid_sparse(linear.matrix(),m,n))
    throw std::invalid_argument("Bounded integer model needs matching domains and canonical CSR");
  for (const auto* values : {&linear.a,&linear.b,&linear.c})
    for (auto value : *values)
      if (value < -1000000000LL || value > 1000000000LL)
        throw std::invalid_argument("Integer linear coefficients must have magnitude <= 1e9");
  for (std::size_t j=0;j<n;++j)
    if (model.lower[j]<Int::Limits::min || model.upper[j]>Int::Limits::max ||
        model.lower[j]>model.upper[j])
      throw std::invalid_argument("Integer domains must be nonempty and within native Gecode limits");
  // Each product fits int64 before comparison (1e9 * native_max). The sum
  // never crosses its checked native limit. This excludes floating-point
  // fallback propagation and protects every signed native row activity.
  const auto accumulate=[&](std::int64_t coefficient,std::size_t j,std::int64_t limit,
                             std::int64_t& magnitude,std::int64_t& lo,std::int64_t& hi) {
    const auto a=coefficient*model.lower[j],b=coefficient*model.upper[j];
    const auto absolute=std::max(std::abs(a),std::abs(b));
    if (absolute>limit-magnitude)
      throw std::invalid_argument("Integer linear activity exceeds exact native limits; tighten domains or rescale with exact integers");
    magnitude+=absolute; lo+=std::min(a,b); hi+=std::max(a,b);
  };
  for (std::size_t i=0;i<m;++i) {
    std::int64_t magnitude=0,lo=0,hi=0;
    for (auto k=linear.row_start[i];k<linear.row_start[i+1];++k)
      accumulate(linear.a[k],linear.column[k],Int::Limits::max,magnitude,lo,hi);
  }
  std::int64_t magnitude=0,lo=0,hi=0;
  // The equality also contains the bounded objective variable itself.
  for (std::size_t j=0;j<n;++j)
    accumulate(linear.c[j],j,Int::Limits::max/2,magnitude,lo,hi);
  return {static_cast<int>(lo),static_cast<int>(hi)};
}
}

inline void validate_integer_model(const BoundedIntegerModel& model) {
  (void) Detail::validate_integer_native(model);
}

/// Preserve original sparse integer rows/domains and the objective equality.
inline void post_native_integer(Home home,const IntVarArgs& x,IntVar objective,
                                const BoundedIntegerModel& model) {
  const auto objective_range=Detail::validate_integer_native(model);
  if (model.linear.c.size()!=static_cast<std::size_t>(x.size()))
    throw Int::ArgumentSizeMismatch("LpRelaxation::post_native_integer");
  if (home.failed()) return;
  for (int j=0;j<x.size();++j)
    dom(home,x[j],static_cast<int>(model.lower[j]),static_cast<int>(model.upper[j]));
  dom(home,objective,objective_range.first,objective_range.second);
  const auto& linear_model=model.linear;
  for (std::size_t i=0;i<linear_model.b.size();++i) {
    IntArgs coefficients; IntVarArgs variables;
    for (auto k=linear_model.row_start[i];k<linear_model.row_start[i+1];++k) {
      coefficients << static_cast<int>(linear_model.a[k]);
      variables << x[static_cast<int>(linear_model.column[k])];
    }
    linear(home,coefficients,variables,IRT_GQ,static_cast<int>(linear_model.b[i]),IPL_BND);
  }
  IntArgs costs(x.size());
  for (int j=0;j<x.size();++j) costs[j]=static_cast<int>(linear_model.c[j]);
  linear(home,costs,x,IRT_EQ,objective,IPL_BND);
}

}}}
#endif
