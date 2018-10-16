/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Samuel Gagnon, 2018
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifdef GECODE_HAS_CBS

#include <limits>
#include <algorithm>

namespace Gecode { namespace Int { namespace Distinct {

  /**
   * \brief Minc and Brégman factors
   *
   * Factors precomputed for every value in the domain of x. Thoses factors are
   * used to compute the Minc and Brégman upper bound for the permanent in
   * counting base search
   */
  const int MAX_MINC_FACTORS = 400;
  extern const double mincFactors[MAX_MINC_FACTORS];

  forceinline double
  getMincFactor(int domSize) {
    return mincFactors[domSize - 1];
  }

  /**
   * \brief Liang and Bai factors
   *
   * Factors precomputed for every index and domain size in x. Thoses factors
   * are used to compute the Liang and Bai upper bound for the permanent in
   * counting base search
   */
  const int WIDTH_LIANG_BAI_FACTORS = 400;
  extern const double liangBaiFactors[WIDTH_LIANG_BAI_FACTORS * WIDTH_LIANG_BAI_FACTORS];

  forceinline double
  getLiangBaiFactor(int index, int domSize) {
    return liangBaiFactors[index*WIDTH_LIANG_BAI_FACTORS + domSize - 1];
  }

  /**
   * \brief Mapping of each value to its permanent update
   *
   */
  class ValToUpdate {
  private:
    /// Minimum value of the union of all variable domains in the propagator
    const int minVal;
    /// Minc and Brégman estimation update for each value
    double* mincUpdate;
    /// Liang and Bai estimation update for each value
    double* liangUpdate;
  public:
    template<class View>
    ValToUpdate(const ViewArray<View>& x,
                int minDomVal, int maxDomVal, Region& r);
    /**
     * Gives the update we have to apply to the Minc and Brégman
     * estimation of the permanent if we fix a variable of cardinalty
     * \a varSize to the value \a val.
     */
    double getMincUpdate(int val, unsigned int varSize) const;
    /**
     * Gives the update we have to apply to the Liang and Bai
     * estimation of the
     * permanent if we fix a variable of cardinalty \a varSize
     * to the value "val".
     */
    double getLiangUpdate(int val, unsigned int idx, unsigned int varSize) const;
  };

  template<class View>
  forceinline
  ValToUpdate::ValToUpdate(const ViewArray<View>& x,
                           int minDomVal, int maxDomVal, Region& r)
    : minVal(minDomVal) {
    unsigned int width = maxDomVal - minDomVal + 1;
    mincUpdate = r.alloc<double>(width);
    std::fill(mincUpdate, mincUpdate + width, 1);
    liangUpdate = r.alloc<double>(width);
    std::fill(liangUpdate, liangUpdate + width, 1);

    for (int i=0; i<x.size(); i++) {
      if (x[i].assigned()) continue;
      size_t s = x[i].size();
      for (ViewValues<View> val(x[i]); val(); ++val) {
        int idx = val.val() - minVal;
        mincUpdate[idx] *= getMincFactor(s-1) / getMincFactor(s);
        liangUpdate[idx] *= getLiangBaiFactor(i, s-1) / getLiangBaiFactor(i, s);
      }
    }
  }

  forceinline double
  ValToUpdate::getMincUpdate(int val, unsigned int varSize) const {
    return mincUpdate[val-minVal] / getMincFactor(varSize-1);
  }

  forceinline double
  ValToUpdate::getLiangUpdate(int val, unsigned int idx,
                              unsigned int varSize) const {
    return liangUpdate[val-minVal] / getLiangBaiFactor(idx, varSize-1);
  }


  template<class View>
  void cbsdistinct(Space&, unsigned int prop_id, const ViewArray<View>& x,
                   Propagator::SendMarginal send) {
    // Computation of Minc and Brégman and Liang and Bai upper bounds for
    // the permanent of the whole constraint
    struct UB {
      double minc;
      double liangBai;
    };

    UB ub{1,1};
    for (int i=0; i<x.size(); i++) {
      unsigned int s = x[i].size();
      if ((s >= MAX_MINC_FACTORS) || (s >= WIDTH_LIANG_BAI_FACTORS))
        throw Gecode::Exception("Int::Distinct::cbsdistinct",
                                "Variable cardinality too big for using counting-based"
                                "search with distinct constraints");
      ub.minc *= getMincFactor(s);
      ub.liangBai *= getLiangBaiFactor(i, s);
    }

    // Minimum and maximum value of the union of all variable domains
    int minVal = std::numeric_limits<int>::max();
    int maxVal = std::numeric_limits<int>::min();
    for (const auto& v : x) {
      if (v.assigned()) continue;
      minVal = std::min(v.min(), minVal);
      maxVal = std::max(v.max(), maxVal);
    }

    // For each possible value, we compute the update we have to apply to the
    // permanent of the whole constraint to get the new solution count
    Region r;
    ValToUpdate valToUpdate(x, minVal, maxVal, r);

    // Preallocated memory for holding solution counts for all values of a
    // variable during computation
    double* solCounts = r.alloc<double>(maxVal - minVal + 1);

    for (int i=0; i<x.size(); i++) {
      if (x[i].assigned()) continue;

      // Normalization constant for keeping densities values between 0 and 1
      double normalization = 0;
      // We calculate the density for every possible value assignment
      for (ViewValues<View> val(x[i]); val(); ++val) {
        UB localUB = ub;
        int v = val.val();
        unsigned int s = x[i].size();

        // We update both upper bounds according to the assigned value, yielding
        // two new estimations for the upper bound
        localUB.minc *= valToUpdate.getMincUpdate(v, s);
        localUB.liangBai *= valToUpdate.getLiangUpdate(v, i, s);

        // We take the lower upper bound as our estimation for the permanent
        double lowerUB = std::min(localUB.minc, ::sqrt(localUB.liangBai));
        solCounts[val.val() - minVal] = lowerUB;
        normalization += lowerUB;
      }

      // Because we approximate the permanent of each value for the variable, we
      // assign densities in a separate loop where we normalize solution densities.
      for (ViewValues<View> val(x[i]); val(); ++val) {
        // In practice, send is going to be a function provided by a brancher.
        // Thus, the brancher will receive each computed solution densities via
        // this call. For more details, please see Section 4 of the dissertation
        // "Improvement and Integration of Counting-Based Search Heuristics in
        // Constraint Programming" by Samuel Gagnon.
        send(prop_id,
             x[i].id(),
             x[i].baseval(val.val()),
             solCounts[val.val() - minVal] / normalization);
      }
    }
  }

  template<class View>
  void cbssize(const ViewArray<View>& x, Propagator::InDecision in,
               unsigned int& size, unsigned int& size_b) {
    size = 0;
    size_b = 0;
    for (const auto& v : x) {
      if (!v.assigned()) {
        size += v.size();
        if (in(v.id()))
          size_b += v.size();
      }
    }
  }

}}}

#endif

// STATISTICS: int-prop

