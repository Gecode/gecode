/* Certified lower bounds for integer linear minimization.
 *
 * Experimental, opt-in support for LP-guided Gecode propagation.
 */

#ifndef __GECODE_MINIMODEL_LP_CERTIFICATE_HPP__
#define __GECODE_MINIMODEL_LP_CERTIFICATE_HPP__

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace Gecode { namespace Experimental { namespace LpCertificate {

  /// Denominator used for the nonnegative rational multipliers.
  constexpr std::int64_t scale = 1048576; // 2^20

#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
  constexpr bool supported = true;
#else
  constexpr bool supported = false;
#endif

  /** Borrowed CSR view; arrays are only read during a checker call.
   * Row offsets include the terminal nnz offset. Each row has strictly
   * increasing column indices and nonzero values (no duplicate ambiguity).
   */
  struct SparseMatrixView {
    const std::vector<std::size_t>& row_start;
    const std::vector<std::size_t>& column;
    const std::vector<std::int64_t>& value;
    std::size_t columns;
  };

  inline bool valid_sparse(const SparseMatrixView& matrix, std::size_t rows,
                           std::size_t columns) {
    if (matrix.columns!=columns || rows==std::numeric_limits<std::size_t>::max() ||
        matrix.row_start.size()!=rows+1 || matrix.column.size()!=matrix.value.size() ||
        matrix.row_start.front()!=0 || matrix.row_start.back()!=matrix.value.size())
      return false;
    for (std::size_t i=0; i<rows; ++i) {
      const auto first=matrix.row_start[i], last=matrix.row_start[i+1];
      if (first>last || last>matrix.value.size()) return false;
      for (std::size_t k=first; k<last; ++k)
        if (matrix.column[k]>=columns || matrix.value[k]==0 ||
            (k!=first && matrix.column[k]<=matrix.column[k-1])) return false;
    }
    return true;
  }

  /// Deterministic work counts; no clocks or backend timings are involved.
  struct PreparationStats {
    std::size_t rows_visited=0;
    std::size_t nonzero_products=0;
    std::size_t residuals_initialized=0;
  };

  /**
   * Convert finite candidate multipliers to nonnegative rationals q/scale.
   *
   * Negative multipliers are clipped to zero. Every resulting q is valid
   * for the certificate, even if the LP solver's multiplier was inaccurate.
   * No proximity to an optimal dual solution is assumed. On failure, the
   * output vector is unchanged.
   */
  inline bool
  quantize(const std::vector<double>& duals,
           std::vector<std::int64_t>& result) {
    std::vector<std::int64_t> candidate;
    candidate.reserve(duals.size());
    for (double dual : duals) {
      if (!std::isfinite(dual))
        return false;
      if (dual <= 0.0) {
        candidate.push_back(0);
        continue;
      }
      const double scaled = std::ldexp(dual,20);
      // Comparing against 2^63 avoids rounding INT64_MAX up to 2^63.
      if (!std::isfinite(scaled) || (scaled >= std::ldexp(1.0,63)))
        return false;
      candidate.push_back(static_cast<std::int64_t>(std::floor(scaled)));
    }
    result.swap(candidate);
    return true;
  }

  /// Exact integer interval cuts, all derived from the same original box.
  struct IntegerFilterResult {
    std::int64_t lower_bound=0;
    std::vector<std::int64_t> lower,upper;
    bool infeasible=false;
  };

  /**
   * An exact affine lower bound valid for every integer box of one model.
   *
   * Its constant is q*b and its residual coefficients are scale*c-A^T*q.
   * A prepared certificate is independent of the box used to obtain its
   * candidate LP multipliers, so descendants and siblings may evaluate it
   * safely with their own bounds. No floating-point reduced cost is used.
   */
  class Certificate {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
    using Wide = __int128;
    Wide constant_ = 0;
    std::vector<Wide> residual_;
    bool valid_ = false;

    bool numerator(const std::vector<std::int64_t>& lower,
                   const std::vector<std::int64_t>& upper,
                   Wide& out, bool binary=true) const {
      if (!valid_ || lower.size()!=residual_.size() ||
          upper.size()!=residual_.size())
        return false;
      Wide value=constant_;
      for (std::size_t j=0; j<residual_.size(); ++j) {
        if (lower[j]>upper[j] || (binary && (lower[j]<0 || upper[j]>1)))
          return false;
        const std::int64_t endpoint=residual_[j]>=0 ? lower[j] : upper[j];
        Wide term;
        if (__builtin_mul_overflow(residual_[j],static_cast<Wide>(endpoint),&term) ||
            __builtin_add_overflow(value,term,&value))
          return false;
      }
      out=value;
      return true;
    }

    static bool ceiling(Wide numerator, std::int64_t& out) {
      Wide value=numerator/static_cast<Wide>(scale);
      if (numerator % static_cast<Wide>(scale)>0)
        if (__builtin_add_overflow(value,static_cast<Wide>(1),&value))
          return false;
      if (value<static_cast<Wide>(std::numeric_limits<std::int64_t>::min()) ||
          value>static_cast<Wide>(std::numeric_limits<std::int64_t>::max()))
        return false;
      out=static_cast<std::int64_t>(value);
      return true;
    }
#endif
    friend bool prepare(const SparseMatrixView&,
                        const std::vector<std::int64_t>&,
                        const std::vector<std::int64_t>&,
                        const std::vector<double>&, Certificate&, PreparationStats*);

  public:
    /// Evaluate the legacy binary-box integer bound; failure leaves out intact.
    bool lower_bound(const std::vector<std::int64_t>& lower,
                     const std::vector<std::int64_t>& upper,
                     std::int64_t& out) const {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
      Wide value;
      return numerator(lower,upper,value) && ceiling(value,out);
#else
      (void) lower; (void) upper; (void) out;
      return false;
#endif
    }

    /// Explicit bounded-integer evaluation; the legacy lower_bound stays binary.
    bool lower_bound_integer(const std::vector<std::int64_t>& lower,
                             const std::vector<std::int64_t>& upper,
                             std::int64_t& out) const {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
      Wide value;
      return numerator(lower,upper,value,false) && ceiling(value,out);
#else
      (void) lower; (void) upper; (void) out;
      return false;
#endif
    }

    /** Intersect a finite integer box with exact residual interval cuts.
     * For each j, remove its original box-minimum contribution, then solve
     * residual[j]*x[j] <= scale*objective_upper - remainder with directed
     * integer division. Every cut uses the unchanged original box. All
     * arithmetic and quotient narrowing are checked; false leaves out intact.
     * An infeasible result has no meaningful tightened-domain interpretation.
     */
    bool filter_integer(const std::vector<std::int64_t>& lower,
                        const std::vector<std::int64_t>& upper,
                        std::int64_t objective_upper, IntegerFilterResult& out) const {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
      Wide base,threshold;
      IntegerFilterResult candidate;
      if (!numerator(lower,upper,base,false) || !ceiling(base,candidate.lower_bound) ||
          __builtin_mul_overflow(static_cast<Wide>(scale),
                                 static_cast<Wide>(objective_upper),&threshold)) return false;
      candidate.lower=lower; candidate.upper=upper;
      candidate.infeasible=base>threshold;
      for (std::size_t j=0; !candidate.infeasible && j<residual_.size(); ++j) {
        const Wide residual=residual_[j];
        if (!residual || lower[j]==upper[j]) continue;
        Wide minimum,remainder,right;
        const auto endpoint=residual>0 ? lower[j] : upper[j];
        if (__builtin_mul_overflow(residual,static_cast<Wide>(endpoint),&minimum) ||
            __builtin_sub_overflow(base,minimum,&remainder) ||
            __builtin_sub_overflow(threshold,remainder,&right)) return false;
        // Signed minimum / -1 is the only nonzero-divisor division overflow.
        if (right==std::numeric_limits<Wide>::min() && residual==-1) return false;
        Wide quotient=right/residual;
        const Wide fraction=right%residual;
        if (residual>0) {
          if (fraction<0 && __builtin_sub_overflow(quotient,static_cast<Wide>(1),&quotient)) return false;
          if (quotient<static_cast<Wide>(lower[j])) candidate.infeasible=true;
          else if (quotient<static_cast<Wide>(upper[j])) candidate.upper[j]=static_cast<std::int64_t>(quotient);
        } else {
          if (fraction<0 && __builtin_add_overflow(quotient,static_cast<Wide>(1),&quotient)) return false;
          if (quotient>static_cast<Wide>(upper[j])) candidate.infeasible=true;
          else if (quotient>static_cast<Wide>(lower[j])) candidate.lower[j]=static_cast<std::int64_t>(quotient);
        }
      }
      out=std::move(candidate);
      return true;
#else
      (void) lower; (void) upper; (void) objective_upper; (void) out;
      return false;
#endif
    }

    /**
     * Evaluate the bound and both conditional bounds for each unfixed bit.
     *
     * forbidden[j]&1 proves x[j]=0 cannot have cost<=objective_upper;
     * forbidden[j]&2 proves the same for x[j]=1. Assigned variables have
     * mask zero. All arithmetic, including the conditional difference, is
     * checked. Failure leaves both outputs unchanged.
     */
    bool filter(const std::vector<std::int64_t>& lower,
                const std::vector<std::int64_t>& upper,
                std::int64_t objective_upper, std::int64_t& bound,
                std::vector<unsigned char>& forbidden) const {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
      Wide base,threshold;
      std::int64_t rounded;
      if (!numerator(lower,upper,base) || !ceiling(base,rounded) ||
          __builtin_mul_overflow(static_cast<Wide>(scale),
                                 static_cast<Wide>(objective_upper),&threshold))
        return false;
      std::vector<unsigned char> candidate(residual_.size(),0);
      for (std::size_t j=0; j<residual_.size(); ++j) {
        if (lower[j]==upper[j])
          continue;
        // An unfixed binary variable has [0,1]. Remove its contribution
        // from the box minimum, then substitute each candidate value.
        const Wide minimum=residual_[j]<0 ? residual_[j] : 0;
        Wide without;
        if (__builtin_sub_overflow(base,minimum,&without))
          return false;
        if (without>threshold)
          candidate[j]|=1;
        Wide with_one;
        if (__builtin_add_overflow(without,residual_[j],&with_one))
          return false;
        if (with_one>threshold)
          candidate[j]|=2;
      }
      bound=rounded;
      forbidden.swap(candidate);
      return true;
#else
      (void) lower; (void) upper; (void) objective_upper;
      (void) bound; (void) forbidden;
      return false;
#endif
    }
  };

  /** Prepare an immutable affine bound from canonical sparse rows.
   * Validation is O(rows+nnz), arithmetic is O(rows+columns+active-dual nnz).
   * No rows*columns allocation or scan occurs. Failure preserves both outputs.
   */
  inline bool
  prepare(const SparseMatrixView& A,
          const std::vector<std::int64_t>& b,
          const std::vector<std::int64_t>& c,
          const std::vector<double>& duals, Certificate& out,
          PreparationStats* work=nullptr) {
#if defined(__SIZEOF_INT128__) && (defined(__GNUC__) || defined(__clang__))
    using Wide = __int128;
    const std::size_t rows=b.size(),columns=c.size();
    if (duals.size()!=rows || !valid_sparse(A,rows,columns)) return false;
    std::vector<std::int64_t> q;
    if (!quantize(duals,q)) return false;
    Certificate candidate;
    PreparationStats measured;
    candidate.residual_.resize(columns);
    for (std::size_t j=0; j<columns; ++j) {
      ++measured.residuals_initialized;
      if (__builtin_mul_overflow(static_cast<Wide>(scale),static_cast<Wide>(c[j]),
                                 &candidate.residual_[j])) return false;
    }
    for (std::size_t i=0; i<rows; ++i) {
      ++measured.rows_visited;
      if (!q[i]) continue;
      Wide product;
      if (__builtin_mul_overflow(static_cast<Wide>(q[i]),static_cast<Wide>(b[i]),&product) ||
          __builtin_add_overflow(candidate.constant_,product,&candidate.constant_)) return false;
      for (std::size_t k=A.row_start[i]; k<A.row_start[i+1]; ++k) {
        ++measured.nonzero_products;
        const auto j=A.column[k];
        if (__builtin_mul_overflow(static_cast<Wide>(q[i]),static_cast<Wide>(A.value[k]),&product) ||
            __builtin_sub_overflow(candidate.residual_[j],product,&candidate.residual_[j])) return false;
      }
    }
    candidate.valid_=true;
    out=std::move(candidate);
    if (work) *work=measured;
    return true;
#else
    (void) A; (void) b; (void) c; (void) duals; (void) out; (void) work;
    return false;
#endif
  }

  /** Legacy dense row-major adapter; existing source calls remain valid.
   * Dense input is scanned once to create canonical sparse rows. Callers that
   * retain sparse storage should use the sparse overload directly.
   */
  inline bool
  prepare(const std::vector<std::int64_t>& A,
          const std::vector<std::int64_t>& b,
          const std::vector<std::int64_t>& c,
          const std::vector<double>& duals, Certificate& out) {
    if (!supported) return false;
    const auto rows=b.size(),columns=c.size();
    if (rows==std::numeric_limits<std::size_t>::max() || duals.size()!=rows ||
        (rows && columns>std::numeric_limits<std::size_t>::max()/rows) ||
        A.size()!=rows*columns) return false;
    std::vector<std::size_t> start,indices;
    std::vector<std::int64_t> values;
    start.reserve(rows+1); start.push_back(0);
    for (std::size_t i=0; i<rows; ++i) {
      for (std::size_t j=0; j<columns; ++j)
        if (A[i*columns+j]) { indices.push_back(j); values.push_back(A[i*columns+j]); }
      start.push_back(values.size());
    }
    return prepare(SparseMatrixView{start,indices,values,columns},b,c,duals,out);
  }

  /**
   * Certify min c*x, Ax>=b, over a nonempty binary box using weak duality:
   *
   * c*x >= (q*b + sum_j min((scale*c-A^T*q)[j]*lower[j],
   *                         (scale*c-A^T*q)[j]*upper[j])) / scale.
   *
   * Any nonnegative quantized q is valid. Checked signed 128-bit arithmetic
   * and mathematical ceiling are the only source of pruning bounds. Floating
   * LP objectives and infeasibility claims never certify a bound. False means
   * no bound was produced and leaves out intact; it does not mean infeasible.
   */
  inline bool
  lower_bound(const SparseMatrixView& A,
              const std::vector<std::int64_t>& b,
              const std::vector<std::int64_t>& c,
              const std::vector<std::int64_t>& lower,
              const std::vector<std::int64_t>& upper,
              const std::vector<double>& duals,
              std::int64_t& out) {
    Certificate certificate;
    return prepare(A,b,c,duals,certificate) && certificate.lower_bound(lower,upper,out);
  }

  /// Legacy dense adapter; identical binary box and failure semantics.
  inline bool
  lower_bound(const std::vector<std::int64_t>& A,
              const std::vector<std::int64_t>& b,
              const std::vector<std::int64_t>& c,
              const std::vector<std::int64_t>& lower,
              const std::vector<std::int64_t>& upper,
              const std::vector<double>& duals,
              std::int64_t& out) {
    Certificate certificate;
    return prepare(A,b,c,duals,certificate) && certificate.lower_bound(lower,upper,out);
  }

  /// Finite integer boxes with integer objective coefficients; never continuous.
  inline bool
  integer_lower_bound(const SparseMatrixView& A,
                      const std::vector<std::int64_t>& b,
                      const std::vector<std::int64_t>& c,
                      const std::vector<std::int64_t>& lower,
                      const std::vector<std::int64_t>& upper,
                      const std::vector<double>& duals, std::int64_t& out) {
    Certificate certificate;
    return prepare(A,b,c,duals,certificate) && certificate.lower_bound_integer(lower,upper,out);
  }
  inline bool
  integer_lower_bound(const std::vector<std::int64_t>& A,
                      const std::vector<std::int64_t>& b,
                      const std::vector<std::int64_t>& c,
                      const std::vector<std::int64_t>& lower,
                      const std::vector<std::int64_t>& upper,
                      const std::vector<double>& duals, std::int64_t& out) {
    Certificate certificate;
    return prepare(A,b,c,duals,certificate) && certificate.lower_bound_integer(lower,upper,out);
  }

}}}

#endif
