/*
 *  Main authors:
 *     Samuel Gagnon <samuel.gagnon92@gmail.com>
 *
 *  Copyright:
 *     Samuel Gagnon, 2018
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
     * Factors precomputed for every value in the domain of x. Those factors are
     * used to compute the Minc and Brégman upper bound for the permanent in
     * counting base search
     */
    const int MAX_MINC_FACTORS = 400;
    extern const double minc_factors[MAX_MINC_FACTORS];

    forceinline double
    get_minc_factor(int dom_size) {
        return minc_factors[dom_size - 1];
    }

    /**
     * \brief Liang and Bai factors
     *
     * Factors precomputed for every index and domain size in x. Those factors
     * are used to compute the Liang and Bai upper bound for the permanent in
     * counting base search
     */
    const int WIDTH_LIANG_BAI_FACTORS = 400;
    extern const double liang_bai_factors[WIDTH_LIANG_BAI_FACTORS * WIDTH_LIANG_BAI_FACTORS];

    forceinline double
    get_liang_bai_factor(int index, int dom_size) {
        return liang_bai_factors[index * WIDTH_LIANG_BAI_FACTORS + dom_size - 1];
    }

    /**
     * \brief Mapping of each value to its permanent update
     *
     */
    class ValToUpdate {
    private:
        /// Minimum value of the union of all variable domains in the propagator
        const int min_val;
        /// Minc and Brégman estimation update for each value
        double* minc_update;
        /// Liang and Bai estimation update for each value
        double* liang_update;

    public:
        template <class View>
        ValToUpdate(const ViewArray<View>& x, int min_dom_val, int max_dom_val, Region& r);
        /**
         * Gives the update we have to apply to the Minc and Brégman
         * estimation of the permanent if we fix a variable of cardinalty
         * \a varSize to the value \a val.
         */
        double get_minc_update(int val, unsigned int var_size) const;
        /**
         * Gives the update we have to apply to the Liang and Bai
         * estimation of the
         * permanent if we fix a variable of cardinalty \a varSize
         * to the value "val".
         */
        double get_liang_update(int val, unsigned int idx, unsigned int var_size) const;
    };

    template <class View>
    forceinline
    ValToUpdate::ValToUpdate(const ViewArray<View>& x, int min_dom_val, int max_dom_val, Region& r)
        : min_val(min_dom_val) {
        unsigned int width = max_dom_val - min_dom_val + 1;
        minc_update = r.alloc<double>(width);
        std::fill(minc_update, minc_update + width, 1);
        liang_update = r.alloc<double>(width);
        std::fill(liang_update, liang_update + width, 1);

        for (int i = 0; i < x.size(); i++) {
            if (x[i].assigned())
                continue;
            size_t s = x[i].size();
            for (ViewValues<View> val(x[i]); val(); ++val) {
                int idx = val.val() - min_val;
                minc_update[idx] *= get_minc_factor(s - 1) / get_minc_factor(s);
                liang_update[idx] *= get_liang_bai_factor(i, s - 1) / get_liang_bai_factor(i, s);
            }
        }
    }

    forceinline double
    ValToUpdate::get_minc_update(int val, unsigned int var_size) const {
        return minc_update[val - min_val] / get_minc_factor(var_size - 1);
    }

    forceinline double
    ValToUpdate::get_liang_update(int val, unsigned int idx, unsigned int var_size) const {
        return liang_update[val - min_val] / get_liang_bai_factor(idx, var_size - 1);
    }

    template <class View>
    void
    cbs_distinct(Space&, unsigned int prop_id, const ViewArray<View>& x, const Propagator::SendMarginal& send) {
        // Computation of Minc and Brégman and Liang and Bai upper bounds for
        // the permanent of the whole constraint
        struct UB {
            double minc;
            double liang_bai;
        };

        UB ub{1, 1};
        for (int i = 0; i < x.size(); i++) {
            unsigned int s = x[i].size();
            if (s >= std::min(MAX_MINC_FACTORS, WIDTH_LIANG_BAI_FACTORS))
                throw Gecode::Exception("Int::Distinct::cbsdistinct",
                                        "Variable cardinality too big for using counting-based"
                                        "search with distinct constraints");
            ub.minc *= get_minc_factor(s);
            ub.liang_bai *= get_liang_bai_factor(i, s);
        }

        // Minimum and maximum value of the union of all variable domains
        int min_val = std::numeric_limits<int>::max();
        int max_val = std::numeric_limits<int>::min();
        for (const auto& v : x) {
            if (v.assigned())
                continue;
            min_val = std::min(v.min(), min_val);
            max_val = std::max(v.max(), max_val);
        }

        // For each possible value, we compute the update we have to apply to the
        // permanent of the whole constraint to get the new solution count
        Region r;
        ValToUpdate val_to_update(x, min_val, max_val, r);

        // Preallocated memory for holding solution counts for all values of a
        // variable during computation
        double* sol_counts = r.alloc<double>(max_val - min_val + 1);

        for (int i = 0; i < x.size(); i++) {
            if (x[i].assigned())
                continue;

            // Normalization constant for keeping densities values between 0 and 1
            double normalization = 0;
            // We calculate the density for every possible value assignment
            for (ViewValues<View> val(x[i]); val(); ++val) {
                UB local_ub = ub;
                int v = val.val();
                unsigned int s = x[i].size();

                // We update both upper bounds according to the assigned value, yielding
                // two new estimations for the upper bound
                local_ub.minc *= val_to_update.get_minc_update(v, s);
                local_ub.liang_bai *= val_to_update.get_liang_update(v, i, s);

                // We take the lower upper bound as our estimation for the permanent
                double lower_ub = std::min(local_ub.minc, ::sqrt(local_ub.liang_bai));
                sol_counts[val.val() - min_val] = lower_ub;
                normalization += lower_ub;
            }

            // Because we approximate the permanent of each value for the variable, we
            // assign densities in a separate loop where we normalize solution densities.
            for (ViewValues<View> val(x[i]); val(); ++val) {
                // In practice, send is going to be a function provided by a brancher.
                // Thus, the brancher will receive each computed solution densities via
                // this call. For more details, please see Section 4 of the dissertation
                // "Improvement and Integration of Counting-Based Search Heuristics in
                // Constraint Programming" by Samuel Gagnon.
                send(prop_id, x[i].id(), x[i].base_value(val.val()), sol_counts[val.val() - min_val] / normalization);
            }
        }
    }

    template <class View>
    void
    cbs_size(const ViewArray<View>& x, const Propagator::InDecision& in, unsigned int& size, unsigned int& size_b) {
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

}}} // namespace Gecode::Int::Distinct

#endif

// STATISTICS: int-prop
