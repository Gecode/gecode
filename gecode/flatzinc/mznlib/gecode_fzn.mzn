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
% Set relation codes: 1 =, 2 !=, 3 subset, 4 superset.
% Arrays correspond by position unless stated otherwise.
% _reif predicates equate b with the relation; _imp require it when b is true.

% Constrain every pair of values in x to differ by at least distance.
% The distance is nonnegative, including for empty and singleton arrays.
predicate gecode_inter_distance_fzn(array[int] of var int: x, var int: distance);

% Constrain values in x whose corresponding present entry is true to
% be pairwise distinct. The arrays have equal lengths.
predicate gecode_all_different_optional(
  array[int] of var int: x,
  array[int] of var bool: present,
);

% Constrain the values in x to be pairwise distinct, except that
% except_value may occur more than once.
predicate gecode_all_different_except(array[int] of var int: x, int: except_value);

% Constrain the sums of corresponding entries in x and offset to be
% pairwise distinct. The arrays must have equal lengths.
predicate gecode_all_different_offset_fzn(
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

% Compare the number of position-wise matches between x and values with count.
predicate gecode_count_matches_fzn(
  array[int] of var int: x,
  array[int] of int: values,
  int: relation,
  var int: count,
);

% Arrays have unequal lengths or at least one pair of corresponding entries differs.
predicate gecode_array_int_ne(array[int] of var int: x, array[int] of var int: y);

predicate gecode_array_bool_ne(array[int] of var bool: x, array[int] of var bool: y);

% At least two entries differ; empty and singleton arrays cannot satisfy this.
predicate gecode_not_all_equal_int(array[int] of var int: x);

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

% y is floor(root(x,n)); n is positive, and negative x requires odd n.
predicate gecode_int_nroot(var int: x, int: n, var int: y);

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

/** @group gecode.constraints
  Schedule tasks with explicit start, duration, and end variables on a unary
  resource. Durations are nonnegative. A zero-duration task cannot occur
  strictly inside another task. The arrays must have equal lengths.
  The caller must also constrain start[i] + duration[i] = end[i].
  Reification is not supported.
*/
predicate gecode_unary_flex(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
);

/** @group gecode.constraints
  Schedule tasks with explicit start, duration, and end variables on a
  cumulative resource. Durations, usage, and capacity are nonnegative.
  The arrays must have equal lengths. The caller must also constrain
  start[i] + duration[i] = end[i]. At each task start, its usage plus the usage
  of tasks strictly spanning that start must fit capacity, even for a
  zero-duration task. Use cumulative for MiniZinc's empty-task semantics.
  Reification is not supported.
*/
predicate gecode_cumulative_flex(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
  array[int] of int: usage,
  var int: capacity,
);

/** @group gecode.constraints
  Schedule tasks on machines numbered 0..length(capacity)-1 with explicit
  start, duration, and end variables. Usage may be negative. If upper is true,
  capacity bounds usage from above, otherwise from below. The bound applies
  at times when at least one task executes on the machine.
  Task arrays must have equal lengths. The caller must constrain
  start[i] + duration[i] = end[i], nonnegative durations, and machine domains.
  Reification is not supported. Only value propagation is available.
*/
predicate gecode_cumulatives_flex(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
  array[int] of var int: usage,
  array[int] of var int: machine,
  array[int] of int: capacity,
  bool: upper,
);

/** @group gecode.constraints
  Schedule the tasks whose present entry is true on a unary resource.
  Durations are nonnegative. Zero-duration tasks have the same semantics as
  gecode_unary_flex. The arrays must have equal lengths. The caller must
  constrain start[i] + duration[i] = end[i] for each present task.
  Reification is not supported.
*/
predicate gecode_unary_flex_optional(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
  array[int] of var bool: present,
);

/** @group gecode.constraints
  Schedule the tasks whose present entry is true on a cumulative resource.
  Durations, usage, and capacity are nonnegative. Zero-duration tasks have
  the same semantics as gecode_cumulative_flex. The arrays must have equal
  lengths. The caller must constrain start[i] + duration[i] = end[i] for each
  present task. Reification is not supported.
*/
predicate gecode_cumulative_flex_optional(
  array[int] of var int: start,
  array[int] of var int: duration,
  array[int] of var int: end,
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

% Constrain each set in x to contain cardinality elements and each pair of
% sets to share at most one element. The cardinality is nonnegative.
predicate gecode_set_atmost_one(array[int] of var set of int: x, int: cardinality);

% Constrain x to list the elements of y in strictly increasing order.
predicate gecode_set_channel_sorted(array[int] of var int: x, var set of int: y);

% Constrain y to be the intersection of all sets in x.
% The intersection of an empty array is the full Gecode set universe.
predicate gecode_array_set_intersect(
  array[int] of var set of int: x,
  var set of int: y,
);

% Constrain value to be the set at (column, row) in the row-major
% x matrix. Coordinates are zero-based; width and height are positive
% and their product equals the length of x.
predicate gecode_set_element2d(
  array[int] of var set of int: x,
  var int: column,
  int: width,
  var int: row,
  int: height,
  var set of int: value,
);

% Constrain y to be x raised to the nonnegative integer n.
predicate gecode_float_pow(var float: x, int: n, var float: y);

% Constrain y to be the minimum of the nonempty array x.
predicate gecode_float_array_min(array[int] of var float: x, var float: y);

% Constrain y to be the maximum of the nonempty array x.
predicate gecode_float_array_max(array[int] of var float: x, var float: y);

% Constrain y to be the positive fixed base raised to x. Requires MPFR.
predicate gecode_float_pow_base(float: base, var float: x, var float: y);

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

% Every element precedes all elements in later nonempty sets; sets are disjoint.
predicate gecode_array_set_seq(array[int] of var set of int: x);
predicate gecode_array_set_seq_union(array[int] of var set of int: x, var set of int: y);

predicate gecode_float_rel_imp_fzn(var float: x, int: relation, var float: y, var bool: b);
predicate gecode_set_rel_imp_fzn(var set of int: x, int: relation, var set of int: y, var bool: b);
