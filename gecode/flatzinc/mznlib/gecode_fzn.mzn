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

% Typed FlatZinc declarations for the native Gecode library.
% Model code should include gecode.mzn for overloaded predicates.
% Relation and task codes follow the enums declared there.

predicate gecode_inter_distance(array[int] of var int: x, var int: distance);

predicate gecode_all_different_optional(array[int] of var int: x,
    array[int] of var bool: present);

predicate gecode_all_different_except(array[int] of var int: x, int: except_value);

predicate gecode_all_different_offset(array[int] of int: offset,
    array[int] of var int: x);

predicate gecode_bool_int_channel(array[int] of var bool: x,
    var int: value, int: offset);

predicate gecode_sort_permutation(array[int] of var int: x,
    array[int] of var int: y, array[int] of var int: permutation, int: offset);

predicate gecode_count_int_fzn(array[int] of var int: x, var int: value,
    int: relation, var int: count);

predicate gecode_count_in_fzn(array[int] of var int: x, set of int: values,
    int: relation, var int: count);

predicate gecode_count_matches_fzn(array[int] of var int: x, array[int] of int: values,
    int: relation, var int: count);

predicate gecode_global_cardinality_sets(array[int] of var int: x,
    array[int] of int: cover, array[int] of set of int: counts);

predicate gecode_nvalues_int_fzn(array[int] of var int: x,
    int: relation, var int: count);

predicate gecode_nvalues_bool_fzn(array[int] of var bool: x,
    int: relation, var int: count);

predicate gecode_array_int_ne(array[int] of var int: x, array[int] of var int: y);

predicate gecode_array_bool_ne(array[int] of var bool: x, array[int] of var bool: y);

predicate gecode_not_all_equal_int(array[int] of var int: x);

predicate gecode_not_all_equal_bool(array[int] of var bool: x);

predicate gecode_int_ite(var bool: b, var int: x, var int: y, var int: z);

predicate gecode_bool_ite(var bool: b, var bool: x, var bool: y, var bool: z);

predicate gecode_set_ite(var bool: b, var set of int: x,
    var set of int: y, var set of int: z);

predicate gecode_float_ite(var bool: b, var float: x, var float: y, var float: z);

predicate gecode_int_divmod(var int: x, var int: y,
    var int: quotient, var int: remainder);

predicate gecode_int_nroot(var int: x, int: n, var int: y);

predicate gecode_regular_bool(array[int] of var bool: x, int: q,
    array[int] of int: transitions, int: initial, set of int: final);

predicate gecode_path(int: offset, array[int] of var int: x,
    var int: start, var int: end);

predicate gecode_path_cost(array[int] of int: costs, int: offset,
    array[int] of var int: x, var int: start, var int: end, var int: cost);

predicate gecode_path_cost_array(array[int] of int: costs, int: offset,
    array[int] of var int: x, var int: start, var int: end,
    array[int] of var int: edge_costs, var int: cost);

predicate gecode_order(var int: start0, int: duration0,
    var int: start1, int: duration1, var bool: order);

predicate gecode_unary_flex_optional(array[int] of var int: start,
    array[int] of var int: duration, array[int] of var int: end,
    array[int] of var bool: present);

predicate gecode_cumulative_flex_optional(array[int] of var int: start,
    array[int] of var int: duration, array[int] of var int: end,
    array[int] of int: usage, var int: capacity, array[int] of var bool: present);

predicate gecode_unary_typed_fzn(array[int] of int: task_type,
    array[int] of var int: flex, array[int] of int: fix);

predicate gecode_unary_typed_optional_fzn(array[int] of int: task_type,
    array[int] of var int: flex, array[int] of int: fix,
    array[int] of var bool: present);

predicate gecode_cumulative_typed_fzn(array[int] of int: task_type,
    array[int] of var int: flex, array[int] of int: fix,
    array[int] of int: usage, var int: capacity);

predicate gecode_cumulative_typed_optional_fzn(array[int] of int: task_type,
    array[int] of var int: flex, array[int] of int: fix,
    array[int] of int: usage, var int: capacity, array[int] of var bool: present);

predicate gecode_nooverlap_optional(array[int] of var int: x,
    array[int] of int: width, array[int] of var int: y,
    array[int] of int: height, array[int] of var bool: present);

predicate gecode_nooverlap_flex_optional(array[int] of var int: x,
    array[int] of var int: width, array[int] of var int: right,
    array[int] of var int: y, array[int] of var int: height,
    array[int] of var int: top, array[int] of var bool: present);

predicate gecode_set_convex_hull(var set of int: x, var set of int: hull);

predicate gecode_set_convex(var set of int: x);

predicate gecode_set_atmost_one(array[int] of var set of int: x, int: cardinality);

predicate gecode_set_channel_sorted(array[int] of var int: x, var set of int: y);

predicate gecode_set_operation_fzn(var set of int: x, int: operation,
    var set of int: y, int: relation, var set of int: z);

predicate gecode_array_set_intersect(array[int] of var set of int: x,
    var set of int: y);

predicate gecode_array_int_set_operation_fzn(array[int] of var int: x,
    int: operation, set of int: initial, var set of int: y);

predicate gecode_set_element2d(array[int] of var set of int: x,
    var int: column, int: width, var int: row, int: height, var set of int: value);

predicate gecode_array_int_set_element_fzn(array[int] of var int: x,
    int: operation, var set of int: selected,
    var set of int: value, set of int: universe);

predicate gecode_float_pow(var float: x, int: n, var float: y);

predicate gecode_float_nroot(var float: x, int: n, var float: y);

predicate gecode_float_array_min(array[int] of var float: x, var float: y);

predicate gecode_float_array_max(array[int] of var float: x, var float: y);

predicate gecode_float_pow_base(float: base, var float: x, var float: y);

predicate gecode_float_log_base(float: base, var float: x, var float: y);

predicate gecode_arg_min_int(array[int] of var int: x, int: offset,
    var int: index, bool: break_ties);

predicate gecode_arg_max_int(array[int] of var int: x, int: offset,
    var int: index, bool: break_ties);

predicate gecode_arg_min_bool(array[int] of var bool: x, int: offset,
    var int: index, bool: break_ties);

predicate gecode_arg_max_bool(array[int] of var bool: x, int: offset,
    var int: index, bool: break_ties);

predicate gecode_set_min(var set of int: s, var int: x);

predicate gecode_set_min_reif(var set of int: s, var int: x, var bool: b);

predicate gecode_set_min_imp(var set of int: s, var int: x, var bool: b);

predicate gecode_set_max(var set of int: s, var int: x);

predicate gecode_set_max_reif(var set of int: s, var int: x, var bool: b);

predicate gecode_set_max_imp(var set of int: s, var int: x, var bool: b);

predicate gecode_set_singleton_rel_fzn(var set of int: s, int: relation,
    var int: x);

predicate gecode_set_singleton_rel_fzn_reif(var set of int: s, int: relation,
    var int: x, var bool: b);

predicate gecode_set_singleton_rel_fzn_imp(var set of int: s, int: relation,
    var int: x, var bool: b);

predicate gecode_array_set_seq(array[int] of var set of int: x);
predicate gecode_array_set_seq_union(array[int] of var set of int: x, var set of int: y);

predicate gecode_float_rel_imp_fzn(var float: x, int: relation, var float: y, var bool: b);
predicate gecode_set_rel_imp_fzn(var set of int: x, int: relation, var set of int: y, var bool: b);

predicate gecode_bool_float_channel(var bool: b, var float: x);
