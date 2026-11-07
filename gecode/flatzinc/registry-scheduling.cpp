/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Contributing authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gmail.com>
 *
 *  Copyright:
 *     Guido Tack, 2007
 *     Mikael Zayenz Lagerkvist, 2009
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

#include <gecode/flatzinc/registry-internal.hh>
#include <gecode/minimodel.hh>

namespace Gecode::FlatZinc::RegistryDetail {

    void
    post_cumulatives(FlatZincSpace& s, const ConExpr& ce, AST::Node* ann) {
        if (ce.size() == 6 || ce.size() == 7) {
            // The seven-argument interface supplies end variables explicitly.
            int offset = ce.size() == 7 ? 1 : 0;
            IntVarArgs start = s.arg_to_int_var_args(ce[0]);
            IntVarArgs duration = s.arg_to_int_var_args(ce[1]);
            IntVarArgs resources = s.arg_to_int_var_args(ce[2 + offset]);
            IntVarArgs machine = s.arg_to_int_var_args(ce[3 + offset]);
            IntArgs bound = s.arg_to_int_args(ce[4 + offset]);
            bool upper = ce[5 + offset]->get_bool();
            int n = start.size();

            IntVarArgs end;
            if (offset) {
                end = s.arg_to_int_var_args(ce[2]);
            } else {
                end = IntVarArgs(n);
                for (int i = n; i--;)
                    end[i] = expr(s, start[i] + duration[i]);
            }

            if (duration.assigned()) {
                IntArgs duration_i(n);
                for (int i = n; (i--) != 0;) {
                    duration_i[i] = duration[i].val();
                }
                if (machine.assigned()) {
                    IntArgs machine_i(n);
                    for (int i = n; (i--) != 0;) {
                        machine_i[i] = machine[i].val();
                    }
                    if (resources.assigned()) {
                        IntArgs resources_i(n);
                        for (int i = n; (i--) != 0;) {
                            resources_i[i] = resources[i].val();
                        }
                        cumulatives(s, machine_i, start, duration_i, end, resources_i, bound, upper,
                                    s.annotation_to_int_prop_level(ann));
                    } else {
                        cumulatives(s, machine_i, start, duration_i, end, resources, bound, upper,
                                    s.annotation_to_int_prop_level(ann));
                    }
                } else if (resources.assigned()) {
                    IntArgs resources_i(n);
                    for (int i = n; (i--) != 0;) {
                        resources_i[i] = resources[i].val();
                    }
                    cumulatives(s, machine, start, duration_i, end, resources_i, bound, upper,
                                s.annotation_to_int_prop_level(ann));
                } else {
                    cumulatives(s, machine, start, duration_i, end, resources, bound, upper,
                                s.annotation_to_int_prop_level(ann));
                }
            } else {
                if (machine.assigned()) {
                    IntArgs machine_i(n);
                    for (int i = n; (i--) != 0;) {
                        machine_i[i] = machine[i].val();
                    }
                    if (resources.assigned()) {
                        IntArgs resources_i(n);
                        for (int i = n; (i--) != 0;) {
                            resources_i[i] = resources[i].val();
                        }
                        cumulatives(s, machine_i, start, duration, end, resources_i, bound, upper,
                                    s.annotation_to_int_prop_level(ann));
                    } else {
                        cumulatives(s, machine_i, start, duration, end, resources, bound, upper,
                                    s.annotation_to_int_prop_level(ann));
                    }
                } else if (resources.assigned()) {
                    IntArgs resources_i(n);
                    for (int i = n; (i--) != 0;) {
                        resources_i[i] = resources[i].val();
                    }
                    cumulatives(s, machine, start, duration, end, resources_i, bound, upper,
                                s.annotation_to_int_prop_level(ann));
                } else {
                    cumulatives(s, machine, start, duration, end, resources, bound, upper,
                                s.annotation_to_int_prop_level(ann));
                }
            }
            return;
        }

        IntVarArgs start = s.arg_to_int_var_args(ce[0]);
        IntVarArgs duration = s.arg_to_int_var_args(ce[1]);
        IntVarArgs height = s.arg_to_int_var_args(ce[2]);
        int n = start.size();
        IntVar bound = s.arg_to_int_var(ce[3]);

        if (n == 0)
            return;

        if (n == 1) {
            rel(s, (duration[0] > 0) >> (height[0] <= bound));
            return;
        }

        bool nonzero_duration = true;
        for (int i = 0; i < n; i++) {
            if (duration[i].min() <= 0) {
                nonzero_duration = false;
                break;
            }
        }

        int min_height = std::min(height[0].min(), height[1].min());
        int min_height2 = std::max(height[0].min(), height[1].min());
        for (int i = 2; i < n; i++) {
            if (height[i].min() < min_height) {
                min_height2 = min_height;
                min_height = height[i].min();
            } else if (height[i].min() < min_height2) {
                min_height2 = height[i].min();
            }
        }
        bool disjunctive =
            nonzero_duration && ((min_height > bound.max() / 2) ||
                                 (min_height2 > bound.max() / 2 && min_height + min_height2 > bound.max()));
        if (disjunctive) {
            rel(s, bound >= max(height));
            // Unary
            if (duration.assigned()) {
                IntArgs duration_i(n);
                for (int i = n; i--;)
                    duration_i[i] = duration[i].val();
                unshare(s, start);
                unary(s, start, duration_i, s.annotation_to_int_prop_level(ann));
            } else {
                IntVarArgs end(n);
                for (int i = n; i--;)
                    end[i] = expr(s, start[i] + duration[i]);
                unshare(s, start);
                unary(s, start, duration, end, s.annotation_to_int_prop_level(ann));
            }
        } else if (nonzero_duration && height.assigned()) {
            IntArgs height_i(n);
            for (int i = n; i--;)
                height_i[i] = height[i].val();
            if (duration.assigned()) {
                IntArgs duration_i(n);
                for (int i = n; i--;)
                    duration_i[i] = duration[i].val();
                cumulative(s, bound, start, duration_i, height_i, s.annotation_to_int_prop_level(ann));
            } else {
                IntVarArgs end(n);
                for (int i = n; i--;)
                    end[i] = expr(s, start[i] + duration[i]);
                cumulative(s, bound, start, duration, end, height_i, s.annotation_to_int_prop_level(ann));
            }
        } else if (nonzero_duration && bound.assigned()) {
            IntArgs machine = IntArgs::create(n, 0, 0);
            IntArgs limit({bound.val()});
            IntVarArgs end(n);
            for (int i = n; i--;)
                end[i] = expr(s, start[i] + duration[i]);
            cumulatives(s, machine, start, duration, end, height, limit, true, s.annotation_to_int_prop_level(ann));
        } else {
            int min = Gecode::Int::Limits::max;
            int max = Gecode::Int::Limits::min;
            IntVarArgs end(start.size());
            for (int i = start.size(); i--;) {
                min = std::min(min, start[i].min());
                max = std::max(max, start[i].max() + duration[i].max());
                end[i] = expr(s, start[i] + duration[i]);
            }
            for (int time = min; time < max; ++time) {
                IntVarArgs x(start.size());
                for (int i = start.size(); i--;) {
                    IntVar overlaps = channel(s, expr(s, (start[i] <= time) && (time < end[i])));
                    x[i] = expr(s, overlaps * height[i]);
                }
                linear(s, x, IRT_LQ, bound);
            }
        }
    }

} // namespace Gecode::FlatZinc::RegistryDetail

// STATISTICS: flatzinc-any
