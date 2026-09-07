/* Optional continuous-LP backend for certified Gecode propagation.
 *
 * HiGHS proposes row multipliers. Only the separate exact certificate
 * checker decides whether a lower bound may enter the constraint solver.
 */

#ifndef __GECODE_MINIMODEL_LP_BACKEND_HPP__
#define __GECODE_MINIMODEL_LP_BACKEND_HPP__

#include <Highs.h>
#include <gecode/minimodel/lp-certificate.hpp>
#include <gecode/minimodel/lp-model.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Gecode { namespace Experimental { namespace LpRelaxation {

  struct BoundResult {
    bool valid = false;
    std::int64_t lower_bound = 0;
    /// Diagnostic only: never rounded or used to prune the CP search.
    double lp_objective = std::numeric_limits<double>::quiet_NaN();
    /// Optional exact residual data for binary or explicit integer interval bounds.
    std::shared_ptr<const LpCertificate::Certificate> certificate;
    /// Copied numerical selection hint only; never an integer feasible witness.
    /// Absent unless explicitly requested and HiGHS supplies finite valid values.
    std::optional<std::vector<double>> primal_suggestion;
  };

  struct Stats {
    std::uint64_t lp_calls = 0;
    double lp_ms = 0.0;
    std::uint64_t valid_bounds = 0;
    std::uint64_t rejected = 0;
    /// Floating-point infeasibility reports, ignored for CP pruning.
    std::uint64_t infeasible_status = 0;
    std::uint64_t certificate_evaluations = 0;
    std::uint64_t conditional_checks = 0;
    std::uint64_t variable_fixings = 0;
    /// Integer variables whose interval was tightened, including assignments.
    std::uint64_t variable_bound_tightenings = 0;
  };

  /**
   * Shared, serialized workspace for the descendants of one CP model.
   *
   * Each call replaces every variable bound, including bounds loosened
   * when search revisits a sibling. The previous simplex basis is merely
   * a hot start, and is never treated as a certificate for another node.
   * This object is deliberately not copied with Gecode spaces.
   */
  namespace Detail {
  class LpWorkspace {
    // Borrowed from the owning, nonmovable backend. Never shared independently.
    const SparseLinearModel& model;
    const std::vector<std::int64_t> original_lower_,original_upper_;
    const bool binary_;

  private:
    mutable std::mutex mutex_;
    Highs highs_;
    Stats stats_;
    std::vector<double> lower_, upper_;

    static void require_ok(HighsStatus status, const char* operation) {
      if (status != HighsStatus::kOk)
        throw std::runtime_error(std::string("LP backend: HiGHS ") + operation);
    }

    void validate_model() const {
      const std::size_t n = model.c.size(), m = model.b.size();
      const std::size_t limit =
        static_cast<std::size_t>(std::numeric_limits<HighsInt>::max());
      if ((n > limit) || (m > limit) ||
          (model.a.size() > limit) ||
          !LpCertificate::valid_sparse(model.matrix(),m,n))
        throw std::invalid_argument("LP backend: invalid canonical CSR dimensions/indices");
      // These integers are represented exactly as doubles in the LP.
      // Restrict the prototype's numerical range; the certificate still
      // checks every arithmetic operation independently.
      for (const auto* values : {&model.a, &model.b, &model.c})
        for (std::int64_t value : *values)
          if ((value < -1000000000LL) || (value > 1000000000LL))
            throw std::invalid_argument("LP backend: coefficient exceeds 1e9");
    }

  public:
    LpWorkspace(const SparseLinearModel& input,
                std::vector<std::int64_t> lower,std::vector<std::int64_t> upper,bool binary)
      : model(input),original_lower_(std::move(lower)),original_upper_(std::move(upper)),binary_(binary) {
      validate_model();
      if (original_lower_.size()!=model.c.size() || original_upper_.size()!=model.c.size())
        throw std::invalid_argument("LP backend: initial bound dimensions");
      for (std::size_t j=0;j<model.c.size();++j)
        if (original_lower_[j]>original_upper_[j] ||
            original_lower_[j]<Int::Limits::min || original_upper_[j]>Int::Limits::max ||
            (binary_ && (original_lower_[j]<0 || original_upper_[j]>1)))
          throw std::invalid_argument("LP backend: unsupported initial integer bounds");
      require_ok(highs_.setOptionValue("output_flag", false), "output option");
      require_ok(highs_.setOptionValue("threads", 1), "threads option");
      require_ok(highs_.setOptionValue("parallel", "off"), "parallel option");
      require_ok(highs_.setOptionValue("solver", "simplex"), "solver option");
      require_ok(highs_.setOptionValue("simplex_strategy", 1), "simplex strategy");
      require_ok(highs_.setOptionValue("presolve", "off"), "presolve option");
      require_ok(highs_.setOptionValue("simplex_iteration_limit", 10000),
                 "iteration limit");

      const std::size_t n = model.c.size(), m = model.b.size();
      lower_.assign(original_lower_.begin(),original_lower_.end());
      upper_.assign(original_upper_.begin(),original_upper_.end());
      if ((n == 0) || (m == 0) || model.a.empty())
        return; // Zero matrices use the checked box bound; native rows enforce feasibility.

      HighsLp lp;
      lp.num_col_ = static_cast<HighsInt>(n);
      lp.num_row_ = static_cast<HighsInt>(m);
      lp.sense_ = ObjSense::kMinimize;
      lp.col_cost_.assign(model.c.begin(), model.c.end());
      lp.col_lower_ = lower_;
      lp.col_upper_ = upper_;
      lp.row_lower_.assign(model.b.begin(), model.b.end());
      lp.row_upper_.assign(m, kHighsInf);
      lp.a_matrix_.format_ = MatrixFormat::kRowwise;
      lp.a_matrix_.num_col_ = lp.num_col_;
      lp.a_matrix_.num_row_ = lp.num_row_;
      // Copy CSR directly: O(rows+nnz), never a rows*columns scan.
      // Replace HiGHS' initial zero offset instead of appending another one.
      lp.a_matrix_.start_.assign(model.row_start.begin(),model.row_start.end());
      lp.a_matrix_.index_.assign(model.column.begin(),model.column.end());
      lp.a_matrix_.value_.assign(model.a.begin(),model.a.end());
      // integrality_ remains empty: HiGHS only solves continuous LPs.
      require_ok(highs_.passModel(std::move(lp)), "model construction");
    }

    LpWorkspace(const LpWorkspace&) = delete;
    LpWorkspace& operator=(const LpWorkspace&) = delete;

    BoundResult bound(const std::vector<std::int64_t>& lower,
                      const std::vector<std::int64_t>& upper,
                      bool retain_certificate=false,bool retain_primal=false) {
      std::lock_guard<std::mutex> lock(mutex_);
      BoundResult result;
      const std::size_t n = model.c.size(), m = model.b.size();
      if ((lower.size() != n) || (upper.size() != n)) {
        ++stats_.rejected;
        return result;
      }
      for (std::size_t j=0; j<n; ++j) {
        if (lower[j]<original_lower_[j] || upper[j]>original_upper_[j] || lower[j]>upper[j]) {
          ++stats_.rejected;
          return result;
        }
        lower_[j] = static_cast<double>(lower[j]);
        upper_[j] = static_cast<double>(upper[j]);
      }

      std::vector<double> duals(m, 0.0);
      if ((n != 0) && (m != 0) && !model.a.empty()) {
        using Clock = std::chrono::steady_clock;
        const auto start = Clock::now();
        // HiGHS accumulates run time across reoptimizations. Add this
        // call's allowance to the already consumed run time.
        HighsStatus status = highs_.setOptionValue(
          "time_limit", highs_.getRunTime()+0.2);
        bool ran = false;
        if (status == HighsStatus::kOk)
          status = highs_.changeColsBounds(
            0, static_cast<HighsInt>(n)-1, lower_.data(), upper_.data());
        if (status == HighsStatus::kOk) {
          ++stats_.lp_calls;
          ran = true;
          status = highs_.run();
        }
        stats_.lp_ms += std::chrono::duration<double,std::milli>(
          Clock::now()-start).count();
        if (ran && (highs_.getModelStatus() == HighsModelStatus::kInfeasible))
          ++stats_.infeasible_status;
        const HighsSolution& solution = highs_.getSolution();
        if (retain_primal && ran && status != HighsStatus::kError &&
            solution.value_valid && solution.col_value.size()==n &&
            std::all_of(solution.col_value.begin(),solution.col_value.end(),
                        [](double value){return std::isfinite(value);}))
          result.primal_suggestion=solution.col_value;
        if (!ran || (status == HighsStatus::kError) || !solution.dual_valid ||
            (solution.row_dual.size() != m)) {
          ++stats_.rejected;
          return result;
        }
        for (double dual : solution.row_dual)
          if (!std::isfinite(dual)) {
            ++stats_.rejected;
            return result;
          }
        duals = solution.row_dual;
        const HighsInfo& info = highs_.getInfo();
        if (info.valid && std::isfinite(info.objective_function_value))
          result.lp_objective = info.objective_function_value;
      }

      if (retain_certificate) {
        auto certificate=std::make_shared<LpCertificate::Certificate>();
        result.valid=LpCertificate::prepare(model.matrix(),model.b,model.c,duals,*certificate) &&
          (binary_ ? certificate->lower_bound(lower,upper,result.lower_bound)
                   : certificate->lower_bound_integer(lower,upper,result.lower_bound));
        if (result.valid)
          result.certificate=std::move(certificate);
      } else {
        // Preserve the original bound-only path unless explicitly requested.
        result.valid = binary_ ? LpCertificate::lower_bound(
          model.matrix(), model.b, model.c, lower, upper, duals, result.lower_bound)
          : LpCertificate::integer_lower_bound(
          model.matrix(), model.b, model.c, lower, upper, duals, result.lower_bound);
      }
      if (result.valid)
        ++stats_.valid_bounds;
      else
        ++stats_.rejected;
      return result;
    }

    void record_filtering(std::uint64_t conditional_checks,
                          std::uint64_t variable_fixings) {
      std::lock_guard<std::mutex> lock(mutex_);
      ++stats_.certificate_evaluations;
      stats_.conditional_checks+=conditional_checks;
      stats_.variable_fixings+=variable_fixings;
    }

    void record_integer_filtering(std::uint64_t checks,std::uint64_t tightened,std::uint64_t fixed) {
      std::lock_guard<std::mutex> lock(mutex_);
      ++stats_.certificate_evaluations;
      stats_.conditional_checks+=checks;
      stats_.variable_bound_tightenings+=tightened;
      stats_.variable_fixings+=fixed;
    }

    Stats statistics() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return stats_;
    }
  };

  } // namespace Detail

  /// Existing strict binary workspace; public sparse model remains immutable.
  class SparseBackend {
  public:
    const SparseLinearModel model;
  private:
    Detail::LpWorkspace workspace_;
  public:
    explicit SparseBackend(SparseLinearModel input)
      : model(std::move(input)),workspace_(model,
          std::vector<std::int64_t>(model.c.size(),0),std::vector<std::int64_t>(model.c.size(),1),true) {}
    virtual ~SparseBackend() = default;
    SparseBackend(const SparseBackend&) = delete;
    SparseBackend& operator=(const SparseBackend&) = delete;
    BoundResult bound(const std::vector<std::int64_t>& lower,const std::vector<std::int64_t>& upper,
                      bool retain_certificate=false) {
      return workspace_.bound(lower,upper,retain_certificate);
    }
    BoundResult bound(const std::vector<std::int64_t>& lower,const std::vector<std::int64_t>& upper,
                      bool retain_certificate,bool retain_primal) {
      return workspace_.bound(lower,upper,retain_certificate,retain_primal);
    }
    void record_filtering(std::uint64_t checks,std::uint64_t fixings) {workspace_.record_filtering(checks,fixings);}
    Stats statistics() const {return workspace_.statistics();}
  };

  /** Explicit integer backend, deliberately unrelated to SparseBackend so an
   * integer model cannot accidentally enter binary posting through an upcast.
   * Each bound call must stay within the immutable original integer domains.
   */
  class BoundedIntegerBackend {
  public:
    const BoundedIntegerModel model;
  private:
    static const SparseLinearModel& checked(const BoundedIntegerModel& input) {
      validate_integer_model(input);return input.linear;
    }
    Detail::LpWorkspace workspace_;
  public:
    explicit BoundedIntegerBackend(BoundedIntegerModel input)
      : model(std::move(input)),workspace_(checked(model),model.lower,model.upper,false) {}
    BoundedIntegerBackend(const BoundedIntegerBackend&) = delete;
    BoundedIntegerBackend& operator=(const BoundedIntegerBackend&) = delete;
    BoundResult bound(const std::vector<std::int64_t>& lower,const std::vector<std::int64_t>& upper,
                      bool retain_certificate=false) {
      return workspace_.bound(lower,upper,retain_certificate);
    }
    BoundResult bound(const std::vector<std::int64_t>& lower,const std::vector<std::int64_t>& upper,
                      bool retain_certificate,bool retain_primal) {
      return workspace_.bound(lower,upper,retain_certificate,retain_primal);
    }
    void record_filtering(std::uint64_t checks,std::uint64_t tightened,std::uint64_t fixings) {
      workspace_.record_integer_filtering(checks,tightened,fixings);
    }
    Stats statistics() const {return workspace_.statistics();}
  };

  /** Dense-compatible adapter. The public immutable dense model is preserved.
   * Its current constructor input is copied into CSR once; repeated bounds and
   * propagation use the base's immutable sparse model. Use SparseBackend to
   * avoid retaining dense storage altogether.
   */
  class Backend : public SparseBackend {
  public:
    const LinearModel model;
    explicit Backend(LinearModel input)
      : SparseBackend(sparse_model(input)), model(std::move(input)) {}
    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;
  };

}}}

#endif
