%
%  Main authors:
%     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
%
%  Copyright:
%     Mikael Zayenz Lagerkvist, 2026
%
%  This file is part of Gecode, the generic constraint
%  development environment:
%     http://www.gecode.dev
%
%  Permission is hereby granted, free of charge, to any person obtaining
%  a copy of this software and associated documentation files (the
%  "Software"), to deal in the Software without restriction, including
%  without limitation the rights to use, copy, modify, merge, publish,
%  distribute, sublicense, and/or sell copies of the Software, and to
%  permit persons to whom the Software is furnished to do so, subject to
%  the following conditions:
%
%  The above copyright notice and this permission notice shall be
%  included in all copies or substantial portions of the Software.
%
%  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
%  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
%  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
%  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
%  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
%  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
%  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
%
%

% Integer relation codes: 1 =, 2 !=, 3 <=, 4 <, 5 >=, 6 >.
% Set relation, operation, and task codes follow the enums in gecode.mzn.
% Arrays correspond by position unless stated otherwise.
% _reif predicates equate b with the relation; _imp require it when b is true.

/** @group gecode.constraints
  Constrain every pair of values in \a x to differ by at least \a distance.
  The distance is nonnegative, including for empty and singleton arrays.
*/
predicate gecode_inter_distance(array[int] of var int: x, var int: distance);

/** @group gecode.constraints
  Constrain values in \a x whose corresponding \a present entry is true to
  be pairwise distinct. The arrays have equal lengths.
*/
predicate gecode_all_different_optional(
  array[int] of var int: x,
  array[int] of var bool: present,
);

/** @group gecode.constraints
  Constrain the values in \a x to be pairwise distinct, except that
  \a except_value may occur more than once.
*/
predicate gecode_all_different_except(array[int] of var int: x, int: except_value);

/** @group gecode.constraints
  Constrain the sums of corresponding entries in \a x and \a offset to be
  pairwise distinct. The arrays must have equal lengths.
*/
predicate gecode_all_different_offset(
  array[int] of int: offset,
  array[int] of var int: x,
);

% Exactly one entry is true; its zero-based position plus offset equals value.
predicate gecode_bool_int_channel(
  array[int] of var bool: x,
  var int: value,
  int: offset,
);

% Sort x into y; permutation[i]-offset is the zero-based position of x[i] in y.
predicate gecode_sort_permutation(
  array[int] of var int: x,
  array[int] of var int: y,
  array[int] of var int: permutation,
  int: offset,
);

% Compare the number of occurrences of value in x with count.
predicate gecode_count_int_fzn(
  array[int] of var int: x,
  var int: value,
  int: relation,
  var int: count,
);

% Compare the number of entries of x that belong to values with count.
predicate gecode_count_in_fzn(
  array[int] of var int: x,
  set of int: values,
  int: relation,
  var int: count,
);

% Compare the number of position-wise matches between x and values with count.
predicate gecode_count_matches_fzn(
  array[int] of var int: x,
  array[int] of int: values,
  int: relation,
  var int: count,
);

/** @group gecode.constraints
  Constrain the number of occurrences of each value in \a cover to belong to
  the corresponding set in \a counts. Every value in \a x must occur in
  \a cover. Cover values are distinct; \a cover and \a counts have equal lengths.
*/
predicate gecode_global_cardinality_sets(
  array[int] of var int: x,
  array[int] of int: cover,
  array[int] of set of int: counts,
);

% Compare the number of distinct values in x with count; an empty array has none.
predicate gecode_nvalues_int_fzn(
  array[int] of var int: x,
  int: relation,
  var int: count,
);

predicate gecode_nvalues_bool_fzn(
  array[int] of var bool: x,
  int: relation,
  var int: count,
);

% Arrays have unequal lengths or at least one pair of corresponding entries differs.
predicate gecode_array_int_ne(array[int] of var int: x, array[int] of var int: y);

predicate gecode_array_bool_ne(array[int] of var bool: x, array[int] of var bool: y);

% At least two entries differ; empty and singleton arrays cannot satisfy this.
predicate gecode_not_all_equal_int(array[int] of var int: x);

predicate gecode_not_all_equal_bool(array[int] of var bool: x);

% z equals x when b is true, and y otherwise.
predicate gecode_int_ite(var bool: b, var int: x, var int: y, var int: z);

predicate gecode_bool_ite(var bool: b, var bool: x, var bool: y, var bool: z);

predicate gecode_set_ite(
  var bool: b,
  var set of int: x,
  var set of int: y,
  var set of int: z,
);

predicate gecode_float_ite(var bool: b, var float: x, var float: y, var float: z);

% result is the nonnegative gcd of x and y; gcd(0,0) is zero.
predicate gecode_gcd_int(var int: x, var int: y, var int: result);
predicate gecode_gcd_int_reif(var int: x, var int: y, var int: result, var bool: b);
predicate gecode_gcd_int_imp(var int: x, var int: y, var int: result, var bool: b);

% divisor divides dividend; zero divides only zero.
predicate gecode_divides_int(var int: divisor, var int: dividend);
predicate gecode_divides_int_reif(var int: divisor, var int: dividend, var bool: b);
predicate gecode_divides_int_imp(var int: divisor, var int: dividend, var bool: b);

% result is the product of x; an empty product is one.
predicate gecode_product_int(array[int] of var int: x, var int: result);
predicate gecode_product_int_reif(array[int] of var int: x, var int: result, var bool: b);
predicate gecode_product_int_imp(array[int] of var int: x, var int: result, var bool: b);

% The product has Euclidean residue result: modulus > 0 and 0 <= result < modulus.
predicate gecode_product_mod_fixed_fzn(array[int] of var int: x, int: modulus, var int: result);
predicate gecode_product_mod_fixed_fzn_reif(array[int] of var int: x, int: modulus, var int: result, var bool: b);
predicate gecode_product_mod_fixed_fzn_imp(array[int] of var int: x, int: modulus, var int: result, var bool: b);
predicate gecode_product_mod_var_fzn(array[int] of var int: x, var int: modulus, var int: result);
predicate gecode_product_mod_var_fzn_reif(array[int] of var int: x, var int: modulus, var int: result, var bool: b);
predicate gecode_product_mod_var_fzn_imp(array[int] of var int: x, var int: modulus, var int: result, var bool: b);

% sites use zero-based labels; distances is a row-major size-by-size matrix.
predicate gecode_minimum_distance_fzn(
  array[int] of var int: sites,
  int: size,
  array[int] of int: distances,
  var int: result,
);

% requirements is a row-major square matrix of pairwise lower bounds,
% with one row per entry in sites.
predicate gecode_minimum_distance_required_fzn(
  array[int] of var int: sites,
  int: size,
  array[int] of int: distances,
  var int: result,
  array[int] of int: requirements,
);

% x = y*quotient+remainder, with quotient truncated toward zero and y != 0.
predicate gecode_int_divmod(
  var int: x,
  var int: y,
  var int: quotient,
  var int: remainder,
);

% y is floor(root(x,n)); n is positive, and negative x requires odd n.
predicate gecode_int_nroot(var int: x, int: n, var int: y);

/** @group gecode.constraints
  Constrain \a x to be accepted by a deterministic automaton with states 1..\a q.
  The row-major \a transitions array has \a q rows and two columns, for false
  and true. A zero transition rejects; \a initial and \a final specify the
  initial state and accepting states.
*/
predicate gecode_regular_bool(
  array[int] of var bool: x,
  int: q,
  array[int] of int: transitions,
  int: initial,
  set of int: final,
);

% NFA rows are states 1..q, columns are first_symbol..first_symbol+symbols-1.
% Each entry gives the set of target states; an empty set rejects.
predicate gecode_regular_nfa_int_fzn(
  array[int] of var int: x,
  int: q,
  int: first_symbol,
  int: symbols,
  array[int] of set of int: transitions,
  int: initial,
  set of int: final,
);
predicate gecode_regular_nfa_bool_fzn(
  array[int] of var bool: x,
  int: q,
  int: first_symbol,
  int: symbols,
  array[int] of set of int: transitions,
  int: initial,
  set of int: final,
);

% Node labels start at offset; the final successor is offset+length(x).
predicate gecode_path(
  int: offset,
  array[int] of var int: x,
  var int: start,
  var int: end,
);

% costs is a row-major square matrix; the final successor edge costs zero.
predicate gecode_path_cost(
  array[int] of int: costs,
  int: offset,
  array[int] of var int: x,
  var int: start,
  var int: end,
  var int: cost,
);

predicate gecode_path_cost_array(
  array[int] of int: costs,
  int: offset,
  array[int] of var int: x,
  var int: start,
  var int: end,
  array[int] of var int: edge_costs,
  var int: cost,
);

/** @group gecode.constraints
  Constrain two tasks not to overlap. If \a order is false,
  \a start0 + \a duration0 <= \a start1; otherwise,
  \a start1 + \a duration1 <= \a start0.
  Durations are nonnegative.
*/
predicate gecode_order(
  var int: start0,
  int: duration0,
  var int: start1,
  int: duration1,
  var bool: order,
);

/** @group gecode.constraints
  Constrain tasks whose \a present entry is true not to overlap. Durations are
  nonnegative. The arrays have equal lengths. The caller must also constrain
  \a start[i] + \a duration[i] = \a end[i] for each present task.
*/
predicate gecode_unary_flex_optional(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
  array[int] of var bool: present,
);

/** @group gecode.constraints
  Constrain the total \a usage of tasks whose \a present entry is true not to
  exceed \a capacity. Durations, usage, and capacity are nonnegative.
  The arrays have equal lengths. The caller must also constrain
  \a start[i] + \a duration[i] = \a end[i] for each present task.
*/
predicate gecode_cumulative_flex_optional(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
  array[int] of int: usage,
  var int: capacity,
  array[int] of var bool: present,
);

% Task codes: 1 has fixed duration, 2 fixed start, 3 fixed end.
% flex/fix mean start/duration, end/start, and start/end, respectively.
predicate gecode_unary_typed_fzn(
  array[int] of int: task_type,
  array[int] of var int: flex,
  array[int] of int: fix,
);

predicate gecode_unary_typed_optional_fzn(
  array[int] of int: task_type,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of var bool: present,
);

predicate gecode_cumulative_typed_fzn(
  array[int] of int: task_type,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of int: usage,
  var int: capacity,
);

predicate gecode_cumulative_typed_optional_fzn(
  array[int] of int: task_type,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of int: usage,
  var int: capacity,
  array[int] of var bool: present,
);

% Present rectangles have nonnegative fixed widths and heights and cannot overlap.
predicate gecode_nooverlap_optional(
  array[int] of var int: x,
  array[int] of int: width,
  array[int] of var int: y,
  array[int] of int: height,
  array[int] of var bool: present,
);

% Present rectangles cannot overlap. The caller must also constrain
% x[i]+width[i]=right[i] and y[i]+height[i]=top[i] for present rectangles.
predicate gecode_nooverlap_flex_optional(
  array[int] of var int: x,
  array[int] of var int: width,
  array[int] of var int: right,
  array[int] of var int: y,
  array[int] of var int: height,
  array[int] of var int: top,
  array[int] of var bool: present,
);

% hull is empty when x is empty, and min(x)..max(x) otherwise.
predicate gecode_set_convex_hull(var set of int: x, var set of int: hull);

% x is empty or contains every integer between its minimum and maximum.
predicate gecode_set_convex(var set of int: x);

/** @group gecode.constraints
  Constrain each set in \a x to have \a cardinality elements and each pair of
  sets to intersect in at most one element. The cardinality is nonnegative.
*/
predicate gecode_set_atmost_one(array[int] of var set of int: x, int: cardinality);

/** @group gecode.constraints
  Constrain \a x to list the elements of \a y in strictly increasing order.
*/
predicate gecode_set_channel_sorted(array[int] of var int: x, var set of int: y);

predicate gecode_set_operation_fzn(
  var set of int: x,
  int: operation,
  var set of int: y,
  int: relation,
  var set of int: z,
);

/** @group gecode.constraints
  Constrain \a y to be the intersection of all sets in \a x.
  The intersection of an empty array is the full Gecode set universe.
*/
predicate gecode_array_set_intersect(
  array[int] of var set of int: x,
  var set of int: y,
);

predicate gecode_array_int_set_operation_fzn(
  array[int] of var int: x,
  int: operation,
  set of int: initial,
  var set of int: y,
);

/** @group gecode.constraints
  Constrain \a value to be the set at (\a column, \a row) in the row-major
  \a x matrix. Coordinates are zero-based; \a width and \a height are positive
  and their product equals the length of \a x.
*/
predicate gecode_set_element2d(
  array[int] of var set of int: x,
  var int: column,
  int: width,
  var int: row,
  int: height,
  var set of int: value,
);

% selected contains zero-based positions; combine the singleton sets {x[i]}.
% An empty intersection equals universe; an empty union is empty.
predicate gecode_array_int_set_element_fzn(
  array[int] of var int: x,
  int: operation,
  var set of int: selected,
  var set of int: value,
  set of int: universe,
);

/** @group gecode.constraints
  Constrain \a y to be \a x raised to the nonnegative integer \a n.
*/
predicate gecode_float_pow(var float: x, int: n, var float: y);

/** @group gecode.constraints
  Constrain \a y to be the n-th root of \a x. The degree \a n is positive.
  Both \a x and \a y are nonnegative, including for odd degrees.
*/
predicate gecode_float_nroot(var float: x, int: n, var float: y);

/** @group gecode.constraints
  Constrain \a y to be the minimum of the nonempty array \a x.
*/
predicate gecode_float_array_min(array[int] of var float: x, var float: y);

/** @group gecode.constraints
  Constrain \a y to be the maximum of the nonempty array \a x.
*/
predicate gecode_float_array_max(array[int] of var float: x, var float: y);

/** @group gecode.constraints
  Constrain \a y to be the positive fixed \a base raised to \a x. Requires MPFR.
*/
predicate gecode_float_pow_base(float: base, var float: x, var float: y);

/** @group gecode.constraints
  Constrain \a y to be the logarithm of \a x to the fixed \a base.
  The base is positive and differs from one; \a x is positive. Requires MPFR.
*/
predicate gecode_float_log_base(float: base, var float: x, var float: y);

% index is the zero-based position plus offset of an extremum.
% If break_ties is true, choose the first such position. Arrays are nonempty.
predicate gecode_arg_min_int(
  array[int] of var int: x,
  int: offset,
  var int: index,
  bool: break_ties,
);

predicate gecode_arg_max_int(
  array[int] of var int: x,
  int: offset,
  var int: index,
  bool: break_ties,
);

predicate gecode_arg_min_bool(
  array[int] of var bool: x,
  int: offset,
  var int: index,
  bool: break_ties,
);

predicate gecode_arg_max_bool(
  array[int] of var bool: x,
  int: offset,
  var int: index,
  bool: break_ties,
);

% x is the minimum of the nonempty set s; reified forms are false for empty s.
predicate gecode_set_min(var set of int: s, var int: x);

predicate gecode_set_min_reif(var set of int: s, var int: x, var bool: b);

predicate gecode_set_min_imp(var set of int: s, var int: x, var bool: b);

% x is the maximum of the nonempty set s; reified forms are false for empty s.
predicate gecode_set_max(var set of int: s, var int: x);

predicate gecode_set_max_reif(var set of int: s, var int: x, var bool: b);

predicate gecode_set_max_imp(var set of int: s, var int: x, var bool: b);

% Compare s with the singleton set {x}.
predicate gecode_set_singleton_rel_fzn(
  var set of int: s,
  int: relation,
  var int: x,
);

predicate gecode_set_singleton_rel_fzn_reif(
  var set of int: s,
  int: relation,
  var int: x,
  var bool: b,
);

predicate gecode_set_singleton_rel_fzn_imp(
  var set of int: s,
  int: relation,
  var int: x,
  var bool: b,
);

% Every element precedes all elements in later nonempty sets; sets are disjoint.
predicate gecode_array_set_seq(array[int] of var set of int: x);
predicate gecode_array_set_seq_union(array[int] of var set of int: x, var set of int: y);

predicate gecode_float_rel_imp_fzn(var float: x, int: relation, var float: y, var bool: b);
predicate gecode_set_rel_imp_fzn(var set of int: x, int: relation, var set of int: y, var bool: b);

% x equals 1.0 when b is true, and 0.0 otherwise.
predicate gecode_bool_float_channel(var bool: b, var float: x);
