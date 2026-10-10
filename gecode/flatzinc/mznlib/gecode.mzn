%
%  Main authors:
%     Guido Tack <tack@gecode.dev>
%
%  Copyright:
%     Guido Tack, 2007
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

/***
 @groupdef gecode Additional declarations for Gecode

 These annotations and predicates are available for the Gecode
 solver. In order to use them in a model, include the file "gecode.mzn".
*/

/***
 @groupdef gecode.annotations Additional Gecode annotations
*/

/** @group gecode.annotations Select variable with smallest accumulated failure count */
annotation afc_min;
/** @group gecode.annotations Select variable with smallest accumulated failure count divided by domain size */
annotation afc_size_min;
/** @group gecode.annotations Select variable with largest accumulated failure count */
annotation afc_max;
/** @group gecode.annotations Select variable with largest accumulated failure count divided by domain size */
annotation afc_size_max;
/** @group gecode.annotations Select variable with smallest action count */
annotation action_min;
/** @group gecode.annotations Select variable with smallest action count divided by domain size */
annotation action_size_min;
/** @group gecode.annotations Select variable with largest action count */
annotation action_max;
/** @group gecode.annotations Select variable with largest action count divided by domain size */
annotation action_size_max;
/** @group gecode.annotations Select random variable */
annotation random;

/** @group gecode.annotations Use the basic algorithm where supported. */
annotation gecode_basic_propagation;
/** @group gecode.annotations Use the advanced algorithm where supported. */
annotation gecode_advanced_propagation;
/** @group gecode.annotations Use both basic and advanced algorithms where supported.
  These may be combined with domain_propagation or bounds_propagation;
  constraints without algorithm choices ignore them. */
annotation gecode_full_propagation;
/** @group gecode.annotations
  Use a separate propagator for each pair in minimum-distance constraints.
  The default is a single propagator. The basic, advanced, and full propagation
  annotations select the algorithm in either case.
*/
annotation gecode_decomposed_propagation;

annotation int_assign(array[int] of var int: x, ann:a);

/** @group gecode.annotations Specify default search strategy for integer variables to use variable selection
  strategy \a varsel, and value choice strategy \a valsel. */
annotation int_default_search(ann: varsel, ann: valsel);
/** @group gecode.annotations Specify default search strategy for Boolean variables to use variable selection
  strategy \a varsel, and value choice strategy \a valsel. */
annotation bool_default_search(ann: varsel, ann: valsel);
/** @group gecode.annotations Specify default search strategy for set variables to use variable selection
  strategy \a varsel, and value choice strategy \a valsel. */
annotation set_default_search(ann: varsel, ann: valsel);
/** @group gecode.annotations Specify default search strategy for float variables to use variable selection
  strategy \a varsel, and value choice strategy \a valsel. */
annotation float_default_search(ann: varsel, ann: valsel);

/** @group gecode.annotations
  Simple large neighbourhood search strategy: upon restart, for each variable in \a x,
  the probability of it being fixed to the previous solution is \a percentage (out of 100).
*/
annotation relax_and_reconstruct(array[int] of var int: x, int: percentage);

/** @group gecode.annotations
  Simple large neighbourhood search strategy: upon restart, for each variable in \a x,
  the probability of it being fixed to the previous solution is \a percentage (out of 100).
  Start from an initial solution \a y.
*/
annotation relax_and_reconstruct(array[int] of var int: x, int: percentage, array[int] of int: y);

/***
 @groupdef gecode.constraints Additional Gecode constraints
*/

/** @group gecode.constraints
  Constrain \a z to be the intersection of all sets
  in \a y that are selected by \a x: \(i \in \a z \leftrightarrow \forall j \in \a x: (i \in \a y[j]) \)
*/
predicate gecode_array_set_element_intersect(var set of int: x,
    array[int] of var set of int: y, var set of int: z);

/** @group gecode.constraints
  Constrain \a z to be the disjoint union of all sets
  in \a y that are selected by \a x: \(i \in \a z \leftrightarrow \exists j \in \a x: (i \in \a y[j]) \)
  and \( i \in \a x \land j \in \a x \land i\neq j \rightarrow \a y[i] \cap \a y[j]=\emptyset \)
*/
predicate gecode_array_set_element_partition(var set of int: x,
    array[int] of var set of int: y, var set of int: z);

/** @group gecode.constraints
  Constrain \a z to be a subset of \a u, and \a z to be the intersection of all sets
  in \a y that are selected by \a x: \(i \in \a z \leftrightarrow \forall j \in \a x: (i \in \a y[j]) \)
*/
predicate gecode_array_set_element_intersect_in(var set of int: x,
    array[int] of var set of int: y,
    var set of int: z, set of int: u);

include "sliding_among.mzn";

/** @group gecode.constraints
 Every subsequence of \a x of length \a l has at least \a m and at most \a n occurrences
 of the values in \a S
*/
predicate among_seq(array[int] of var int: x, set of int: S,
    int: l, int: m, int: n) = sliding_among(m, n, l, x, S);

/** @group gecode.constraints
 Every subsequence of \a x of length \a l has at least \a m and at most \a n occurrences
 of the value \a b
*/
predicate among_seq(array[int] of var bool: x, bool: b,
    int: l, int: m, int: n) = sliding_among(m, n, l, x, b);

% Circuit constraints
predicate gecode_circuit_cost_array(array[int] of int: c,
    array[int] of var int: x, array[int] of var int: y, var int: z);
predicate gecode_circuit_cost(array[int] of int: c, array[int] of var int: x,
    var int: z);

/** @group gecode.constraints
  Constrains the elements of \a x to define a circuit where \a x[\p i] = \p j means
  that \p j is the successor of \p i. Additionally, constrain \a z to
  be the cost of the circuit. Each edge cost is defined by array \a c. The variables
  \a y[i] are constrained to be the edge cost of the node \a x[i].
*/
predicate circuit_cost_array(array[int] of int: c,
    array[int] of var int: x, array[int] of var int: y, var int: z) =
  gecode_circuit_cost_array(c,[x[i]-min(index_set(x)) | i in index_set(x)],
                            y,z);
/** @group gecode.constraints
  Constrains the elements of \a x to define a circuit where \a x[\p i] = \p j means
  that \p j is the successor of \p i. Additionally, constrain \a z to
  be the cost of the circuit. Each edge cost is defined by array \a c.
*/
predicate circuit_cost(array[int] of int: c, array[int] of var int: x,
    var int: z) =
  gecode_circuit_cost(c, [x[i]-min(index_set(x)) | i in index_set(x)], z);

% Scheduling constraints
predicate gecode_schedule_unary(array[int] of var int: x, array[int] of int: p);
predicate gecode_schedule_unary_optional(array[int] of var int: x,
    array[int] of int: p, array[int] of var bool: m);

predicate gecode_schedule_cumulative_optional(array[int] of var int: start,
  array[int] of int: duration, array[int] of int: usage,
  array[int] of var bool: m, var int: capacity);


include "gecode_fzn.mzn";
include "among.mzn";
include "nvalue.mzn";
include "range.mzn";
include "arg_min.mzn";
include "arg_max.mzn";

/** @group gecode.constraints
  Comparison operators for integer values and counts.
*/
enum GecodeIntRelation = {
  GecodeEq,
  GecodeNe,
  GecodeLe,
  GecodeLt,
  GecodeGe,
  GecodeGt,
};

/** @group gecode.constraints
  Comparison operators for sets. Complement uses Gecode's full set
  universe. Ordering compares membership from the smallest element, rather
  than comparing sorted lists of elements.
*/
enum GecodeSetRelation = {
  GecodeSetEq,
  GecodeSetNe,
  GecodeSubset,
  GecodeSuperset,
  GecodeDisjoint,
  GecodeComplement,
  GecodeSetLe,
  GecodeSetLt,
  GecodeSetGe,
  GecodeSetGt,
};
/** @group gecode.constraints
  Union, disjoint union, intersection, and difference of sets.
*/
enum GecodeSetOperation = {
  GecodeUnion,
  GecodeDisjointUnion,
  GecodeIntersection,
  GecodeDifference,
};

/** @group gecode.constraints
  Describes the fixed part of a task.
  GecodeFixedDuration uses flex as start and fix as duration.
  GecodeFixedStart uses flex as end and fix as start.
  GecodeFixedEnd uses flex as start and fix as end.
  Fixed durations must be nonnegative.
*/
enum GecodeTaskType = {
  GecodeFixedDuration,
  GecodeFixedStart,
  GecodeFixedEnd,
};

/** @group gecode.constraints
  Requires all values \a x[i] + \a offset[i] to be distinct.
  The arrays must have equal lengths and correspond by position.
*/
predicate gecode_all_different(
  array[int] of int: offset,
  array[int] of var int: x,
) =
  gecode_all_different_offset(offset, x);

/** @group gecode.constraints
  Constrains \a y to be the sorted values of \a x, with
  \a x[i] = \a y[\a permutation[i]]. Equal values may appear in either order.
  The index sets of \a x and \a permutation must agree, and all arrays must
  have equal lengths.
*/
predicate gecode_sort_permutation(
  array[int] of var int: x,
  array[int] of var int: y,
  array[int] of var int: permutation,
) =
  assert(
    index_set(x) = index_set(permutation) /\ length(x) = length(y),
    "gecode_sort_permutation: incompatible array indexes or lengths",
    if length(y) = 0 then
      true
    else
      gecode_sort_permutation(x, y, permutation, min(index_set(y)))
    endif,
  );

/** @group gecode.constraints
  Compares the number of elements of \a x that belong to \a values
  with \a n, using \a relation.
*/
predicate gecode_count(
  array[int] of var int: x,
  set of int: values,
  GecodeIntRelation: relation,
  var int: n,
) =
  if relation = GecodeEq then
    among(n, x, values)
  elseif relation = GecodeNe then
    let {
      var 0 .. length(x): c;
      constraint among(c, x, values);
    } in
      c != n
  else
    gecode_count_in_fzn(x, values, enum2int(relation), n)
  endif;
/** @group gecode.constraints
  Compares the number of positions where \a x matches \a values
  with \a n, using \a relation. The arrays must have equal lengths and
  correspond by position.
*/
predicate gecode_count(
  array[int] of var int: x,
  array[int] of int: values,
  GecodeIntRelation: relation,
  var int: n,
) =
  assert(
    length(x) = length(values),
    "gecode_count: unequal array lengths",
    gecode_count_matches_fzn(x, values, enum2int(relation), n),
  );

/** @group gecode.constraints
  Compares the number of distinct values in \a x with \a n, using
  \a relation. An empty array has zero distinct values.
*/
predicate gecode_nvalues(
  array[int] of var int: x,
  GecodeIntRelation: relation,
  var int: n,
) =
  if relation = GecodeEq then
    nvalue(n, x)
  elseif relation = GecodeNe then
    let {
      var 0 .. length(x): c;
      constraint nvalue(c, x);
    } in
      c != n
  else
    gecode_nvalues_int_fzn(x, enum2int(relation), n)
  endif;
/** @group gecode.constraints
  Compares the number of distinct values in \a x with \a n, using
  \a relation. An empty array has zero distinct values.
*/
predicate gecode_nvalues(
  array[int] of var bool: x,
  GecodeIntRelation: relation,
  var int: n,
) =
  if relation = GecodeNe then
    let {
      var 0 .. 2: c;
      constraint gecode_nvalues_bool_fzn(x, enum2int(GecodeEq), c);
    } in
      c != n
  else
    gecode_nvalues_bool_fzn(x, enum2int(relation), n)
  endif;

/** @group gecode.constraints
  Constrains \a result to be the nonnegative greatest common divisor
  of \a x and \a y. Negative operands use their absolute values, and
  gcd(0, 0) = 0.
*/
predicate gecode_gcd(var int: x, var int: y, var int: result) =
  gecode_gcd_int(x, y, result);

/** @group gecode.constraints
  Requires \a dividend to be an integer multiple of \a divisor.
  A zero divisor is permitted only when \a dividend is zero.
*/
predicate gecode_divides(var int: divisor, var int: dividend) =
  gecode_divides_int(divisor, dividend);

/** @group gecode.constraints
  Constrains \a b to be true if and only if the product of \a x
  equals \a result. The product of an empty array is one.
*/
predicate gecode_product_reif(
  array[int] of var int: x,
  var int: result,
  var bool: b,
) =
  gecode_product_int_reif(x, result, b);
/** @group gecode.constraints
  Requires the product of \a x to equal \a result when \a b is true.
  The product of an empty array is one.
*/
predicate gecode_product_imp(
  array[int] of var int: x,
  var int: result,
  var bool: b,
) =
  gecode_product_int_imp(x, result, b);

/** @group gecode.constraints
  Constrains \a result to be the Euclidean remainder of the product
  of \a x modulo \a modulus, with 0 <= \a result < \a modulus.
  A nonpositive modulus makes the proposition false.
  The product of an empty array is one.
*/
predicate gecode_product_mod(
  array[int] of var int: x,
  var int: modulus,
  var int: result,
) =
  if is_fixed(modulus) then
    if fix(modulus) > 0 then
      gecode_product_mod_fixed_fzn(x, fix(modulus), result)
    else
      gecode_product_mod_var_fzn(x, modulus, result)
    endif
  else
    gecode_product_mod_var_fzn(x, modulus, result)
  endif;

/** @group gecode.constraints
  Constrains \a result to be the smallest distance between any two
  positions in \a sites, using \a distances[\a sites[i], \a sites[j]].
  There must be at least two positions. Both matrix axes use the same site
  indexes, and the matrix must be symmetric and nonnegative with a zero
  diagonal. Repeated sites are permitted.
*/
predicate gecode_minimum_distance(
  array[int] of var int: sites,
  array[int, int] of int: distances,
  var int: result,
) =
  assert(
    length(sites) >= 2 /\
    length(distances) > 0 /\
    index_set_1of2(distances) = index_set_2of2(distances),
    "gecode_minimum_distance: need at least two positions and a square nonempty distance matrix",
    gecode_minimum_distance_fzn(
      [v - min(index_set_1of2(distances)) | v in sites],
      card(index_set_1of2(distances)),
      array1d(distances),
      result,
    ),
  );
/** @group gecode.constraints
  Constrains \a result to be the smallest distance between any two
  positions in \a sites. Each pair must also meet the lower bound in
  \a requirements[i,j]. There must be at least two positions.
  Both axes of \a distances use the same site indexes, and both axes of
  \a requirements use the index set of \a sites. Both matrices must be
  symmetric and nonnegative with a zero diagonal. Repeated sites are permitted.
*/
predicate gecode_minimum_distance(
  array[int] of var int: sites,
  array[int, int] of int: distances,
  var int: result,
  array[int, int] of int: requirements,
) =
  assert(
    length(sites) >= 2 /\
    length(distances) > 0 /\
    index_set_1of2(distances) = index_set_2of2(distances) /\
    index_set_1of2(requirements) = index_set(sites) /\
    index_set_2of2(requirements) = index_set(sites),
    "gecode_minimum_distance: incompatible distance/requirements matrix or too few positions",
    gecode_minimum_distance_required_fzn(
      [v - min(index_set_1of2(distances)) | v in sites],
      card(index_set_1of2(distances)),
      array1d(distances),
      result,
      array1d(requirements),
    ),
  );

/** @group gecode.constraints
  Constrains \a quotient = \a x div \a y and
  \a remainder = \a x mod \a y. The divisor \a y must be nonzero.
  Division truncates towards zero; a nonzero remainder has the sign of \a x.
*/
predicate gecode_divmod(
  var int: x,
  var int: y,
  var int: quotient,
  var int: remainder,
) =
  % Bound the results before dependent linear constraints are posted.
  let {
    var compute_div_bounds(x, y): q = quotient;
    var compute_mod_bounds(x, y): r = remainder;
  } in
    gecode_int_divmod(x, y, q, r);

/** @group gecode.constraints
  Constrains \a y to be the integer root of degree \a degree of \a x,
  rounded down. The degree must be positive, and negative \a x is permitted
  only for odd degrees.
*/
predicate gecode_nroot(var int: x, int: degree, var int: y) =
  assert(degree > 0, "gecode_nroot: degree must be positive", gecode_int_nroot(x, degree, y));
/** @group gecode.constraints
  Constrains \a y to be the nonnegative root of degree \a degree of \a x.
  The degree must be positive, and \a x must be nonnegative.
*/
predicate gecode_nroot(var float: x, int: degree, var float: y) =
  assert(
    degree > 0,
    "gecode_nroot: degree must be positive",
    if degree = 2 then
      float_sqrt(x, y)
    else
      gecode_float_nroot(x, degree, y)
    endif,
  );

/** @group gecode.constraints
  Requires \a x to describe a path from \a start to \a end that
  visits every node once, where \a x[i] is the successor of node i.
  The node array must be nonempty. Its indexes identify the nodes, and
  \a x[\a end] = max(index_set(\a x)) + 1 marks the end of the path.
*/
predicate gecode_path(array[int] of var int: x, var int: start, var int: end) =
  assert(
    length(x) > 0,
    "gecode_path: empty node array",
    gecode_path(
      0,
      [v - min(index_set(x)) | v in x],
      start - min(index_set(x)),
      end - min(index_set(x)),
    ),
  );
/** @group gecode.constraints
  Requires \a x to describe a path from \a start to \a end that
  visits every node once, with total edge cost \a cost given by \a costs.
  Both matrix axes must use the index set of the nonempty node array.
  The successor \a x[\a end] = max(index_set(\a x)) + 1 marks the end,
  and its edge costs zero.
*/
predicate gecode_path(
  array[int, int] of int: costs,
  array[int] of var int: x,
  var int: start,
  var int: end,
  var int: cost,
) =
  assert(
    length(x) > 0 /\
    index_set_1of2(costs) = index_set(x) /\
    index_set_2of2(costs) = index_set(x),
    "gecode_path: incompatible cost matrix",
    gecode_path_cost(
      array1d(costs),
      0,
      [v - min(index_set(x)) | v in x],
      start - min(index_set(x)),
      end - min(index_set(x)),
      cost,
    ),
  );
/** @group gecode.constraints
  Requires \a x to describe a path from \a start to \a end that
  visits every node once, with total cost \a cost. Each \a edge_costs[i]
  is the cost of the edge from i to \a x[i], as given by \a costs.
  Both matrix axes and \a edge_costs must use the index set of the nonempty
  node array. The successor \a x[\a end] = max(index_set(\a x)) + 1 marks
  the end, and its edge costs zero.
*/
predicate gecode_path(
  array[int, int] of int: costs,
  array[int] of var int: x,
  var int: start,
  var int: end,
  array[int] of var int: edge_costs,
  var int: cost,
) =
  assert(
    length(x) > 0 /\
    index_set_1of2(costs) = index_set(x) /\
    index_set_2of2(costs) = index_set(x) /\
    index_set(edge_costs) = index_set(x),
    "gecode_path: incompatible cost matrix or edge costs",
    gecode_path_cost_array(
      array1d(costs),
      0,
      [v - min(index_set(x)) | v in x],
      start - min(index_set(x)),
      end - min(index_set(x)),
      edge_costs,
      cost,
    ),
  );

/** @group gecode.constraints
  Requires the tasks described by \a types, \a flex, and \a fix
  not to overlap. GecodeTaskType specifies which start, end, or duration is
  fixed. The arrays must have equal lengths and correspond by position.
*/
predicate gecode_unary_typed(
  array[int] of GecodeTaskType: types,
  array[int] of var int: flex,
  array[int] of int: fix,
) =
  gecode_unary_typed_fzn(enum2int(types), flex, fix);
/** @group gecode.constraints
  Requires the present tasks described by \a types, \a flex, and
  \a fix not to overlap. A task is present when its entry in \a present
  is true. GecodeTaskType specifies which start, end, or duration is fixed.
  The arrays must have equal lengths and correspond by position.
*/
predicate gecode_unary_typed(
  array[int] of GecodeTaskType: types,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of var bool: present,
) =
  gecode_unary_typed_optional_fzn(enum2int(types), flex, fix, present);

/** @group gecode.constraints
  Requires the total \a usage of tasks executing at any time not
  to exceed \a capacity. GecodeTaskType specifies how \a types, \a flex,
  and \a fix describe each task. Usage and capacity must be nonnegative.
  The arrays must have equal lengths and correspond by position.
*/
predicate gecode_cumulative_typed(
  array[int] of GecodeTaskType: types,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of int: usage,
  var int: capacity,
) =
  gecode_cumulative_typed_fzn(enum2int(types), flex, fix, usage, capacity);
/** @group gecode.constraints
  Requires the total \a usage of present tasks executing at any
  time not to exceed \a capacity. A task is present when its entry in
  \a present is true. GecodeTaskType specifies how \a types, \a flex,
  and \a fix describe each task. Usage and capacity must be nonnegative.
  The arrays must have equal lengths and correspond by position.
*/
predicate gecode_cumulative_typed(
  array[int] of GecodeTaskType: types,
  array[int] of var int: flex,
  array[int] of int: fix,
  array[int] of int: usage,
  var int: capacity,
  array[int] of var bool: present,
) =
  gecode_cumulative_typed_optional_fzn(enum2int(types), flex, fix, usage, capacity, present);

/** @group gecode.constraints
  Requires the present rectangles with origins (\a x[i], \a y[i])
  and fixed sizes (\a width[i], \a height[i]) not to overlap.
  A rectangle is present when \a present[i] is true.
  Sizes must be nonnegative. All arrays must have identical index sets.
*/
predicate gecode_nooverlap(
  array[int] of var int: x,
  array[int] of int: width,
  array[int] of var int: y,
  array[int] of int: height,
  array[int] of var bool: present,
) =
  assert(
    index_set(x) = index_set(width) /\
    index_set(x) = index_set(y) /\
    index_set(x) = index_set(height) /\
    index_set(x) = index_set(present),
    "gecode_nooverlap: incompatible indexes",
    gecode_nooverlap_optional(x, width, y, height, present),
  );
/** @group gecode.constraints
  Requires the present rectangles not to overlap, with horizontal
  bounds \a x and \a right and vertical bounds \a y and \a top.
  A rectangle is present when \a present[i] is true.
  For each present rectangle, \a x + \a width = \a right and
  \a y + \a height = \a top. Sizes must be nonnegative.
  All arrays must have identical index sets.
*/
predicate gecode_nooverlap(
  array[int] of var int: x,
  array[int] of var int: width,
  array[int] of var int: right,
  array[int] of var int: y,
  array[int] of var int: height,
  array[int] of var int: top,
  array[int] of var bool: present,
) =
  assert(
    index_set(x) = index_set(width) /\
    index_set(x) = index_set(right) /\
    index_set(x) = index_set(y) /\
    index_set(x) = index_set(height) /\
    index_set(x) = index_set(top) /\
    index_set(x) = index_set(present),
    "gecode_nooverlap: incompatible indexes",
    forall (i in index_set(x)) (
      present[i] -> (x[i] + width[i] = right[i] /\ y[i] + height[i] = top[i])
    ) /\
    gecode_nooverlap_flex_optional(x, width, right, y, height, top, present),
  );

/** @group gecode.constraints
  Requires \a x to be empty or a consecutive interval of integers.
*/
predicate gecode_convex(var set of int: x) =
  gecode_set_convex(x);
/** @group gecode.constraints
  Constrains \a hull to be the smallest consecutive interval
  containing \a x. The hull of an empty set is empty.
*/
predicate gecode_convex(var set of int: x, var set of int: hull) =
  gecode_set_convex_hull(x, hull);

/** @group gecode.constraints
  Requires every element of each set in \a x to be smaller than
  every element of each later set. Empty sets are permitted.
*/
predicate gecode_set_sequence(array[int] of var set of int: x) =
  gecode_array_set_seq(x);
/** @group gecode.constraints
  Requires every element of each set in \a x to be smaller than
  every element of each later set, and constrains \a y to their union.
  Empty sets are permitted.
*/
predicate gecode_set_sequence(array[int] of var set of int: x, var set of int: y) =
  gecode_array_set_seq_union(x, y);

/** @group gecode.constraints
  Compares the result of applying \a operation to \a x and \a y
  with \a z, using \a relation. Complement and set ordering follow
  GecodeSetRelation.
*/
predicate gecode_set_operation(
  var set of int: x,
  GecodeSetOperation: operation,
  var set of int: y,
  GecodeSetRelation: relation,
  var set of int: z,
) =
  if relation = GecodeSetEq /\ operation = GecodeUnion then
    z = x union y
  elseif relation = GecodeSetEq /\ operation = GecodeIntersection then
    z = x intersect y
  elseif relation = GecodeSetEq /\ operation = GecodeDifference then
    z = x diff y
  else
    gecode_set_operation_fzn(x, enum2int(operation), y, enum2int(relation), z)
  endif;

/** @group gecode.constraints
  Combines \a initial and the singleton sets {\a x[i]} using
  \a operation, and constrains \a y to the result. Union, disjoint union,
  and intersection are supported.
*/
predicate gecode_array_int_set_operation(
  array[int] of var int: x,
  GecodeSetOperation: operation,
  set of int: initial,
  var set of int: y,
) =
  assert(
    operation != GecodeDifference,
    "gecode_array_int_set_operation: difference is unsupported",
    gecode_array_int_set_operation_fzn(x, enum2int(operation), initial, y),
  );

/** @group gecode.constraints
  Combines the singleton sets {\a x[i]} at the zero-based positions
  in \a selected, using \a operation, and constrains \a value to the result.
  Union, disjoint union, and intersection are supported. An empty selection
  gives \a universe for intersection and the empty set for either union.
*/
predicate gecode_array_int_set_element(
  array[int] of var int: x,
  GecodeSetOperation: operation,
  var set of int: selected,
  var set of int: value,
  set of int: universe,
) =
  assert(
    operation != GecodeDifference,
    "gecode_array_int_set_element: difference is unsupported",
    if operation = GecodeUnion then
      gecode_range(x, 0, selected, value)
    else
      gecode_array_int_set_element_fzn(x, enum2int(operation), selected, value, universe)
    endif,
  );

/** @group gecode.constraints
  Constrains \a b to be true if and only if \a s is nonempty and
  \a x is its smallest element.
*/
predicate gecode_min_reif(var set of int: s, var int: x, var bool: b) =
  gecode_set_min_reif(s, x, b);
/** @group gecode.constraints
  Constrains \a b to be true if and only if \a s is nonempty and
  \a x is its largest element.
*/
predicate gecode_max_reif(var set of int: s, var int: x, var bool: b) =
  gecode_set_max_reif(s, x, b);

/** @group gecode.constraints
  Compares \a s with the singleton set {\a x}, using \a relation.
  Complement and set ordering follow GecodeSetRelation.
*/
predicate gecode_set_singleton_rel(
  var set of int: s,
  GecodeSetRelation: relation,
  var int: x,
) =
  if relation = GecodeSuperset then
    x in s
  else
    gecode_set_singleton_rel_fzn(s, enum2int(relation), x)
  endif;
/** @group gecode.constraints
  Constrains \a b to be true if and only if \a s has \a relation
  to the singleton set {\a x}. Complement and set ordering follow
  GecodeSetRelation.
*/
predicate gecode_set_singleton_rel_reif(
  var set of int: s,
  GecodeSetRelation: relation,
  var int: x,
  var bool: b,
) =
  if relation = GecodeSuperset then
    b <-> x in s
  else
    gecode_set_singleton_rel_fzn_reif(s, enum2int(relation), x, b)
  endif;
/** @group gecode.constraints
  Requires \a s to have \a relation to the singleton set {\a x}
  when \a b is true. Complement and set ordering follow GecodeSetRelation.
*/
predicate gecode_set_singleton_rel_imp(
  var set of int: s,
  GecodeSetRelation: relation,
  var int: x,
  var bool: b,
) =
  if relation = GecodeSuperset then
    b -> x in s
  else
    gecode_set_singleton_rel_fzn_imp(s, enum2int(relation), x, b)
  endif;

/** @group gecode.constraints
  Requires \a x to have \a relation to \a y when \a b is true.
  Complement and set ordering follow GecodeSetRelation.
*/
predicate gecode_rel_imp(
  var set of int: x,
  GecodeSetRelation: relation,
  var set of int: y,
  var bool: b,
) =
  if relation = GecodeSetEq then
    b -> x = y
  elseif relation = GecodeSetNe then
    b -> x != y
  elseif relation = GecodeSubset then
    b -> x subset y
  elseif relation = GecodeSuperset then
    b -> x superset y
  else
    gecode_set_rel_imp_fzn(x, enum2int(relation), y, b)
  endif;

/** @group gecode.constraints
  Constrains \a y to be the logarithm of \a x to base \a base.
  The base must be positive and different from one. Requires MPFR support.
*/
predicate gecode_log(float: base, var float: x, var float: y) =
  assert(
    base > 0.0 /\ base != 1.0,
    "gecode_log: base must be positive and different from one",
    if base = 2.0 then
      float_log2(x, y)
    elseif base = 10.0 then
      float_log10(x, y)
    else
      gecode_float_log_base(base, x, y)
    endif,
  );

/** @group gecode.constraints
  Constrains \a index to be the index of a smallest value in \a x.
  If \a break_ties is true, the smallest qualifying index is required;
  otherwise any qualifying index is permitted. The array must be nonempty.
*/
predicate gecode_arg_min(
  array[int] of var int: x,
  var int: index,
  bool: break_ties,
) =
  assert(
    length(x) > 0,
    "gecode_arg_min: empty array",
    if break_ties then
      arg_min(array1d(x)) + min(index_set(x)) - 1 = index
    else
      gecode_arg_min_int(x, 0, index - min(index_set(x)), false)
    endif,
  );
/** @group gecode.constraints
  Constrains \a index to be the index of a smallest value in \a x.
  If \a break_ties is true, the smallest qualifying index is required;
  otherwise any qualifying index is permitted. The array must be nonempty.
*/
predicate gecode_arg_min(
  array[int] of var bool: x,
  var int: index,
  bool: break_ties,
) =
  assert(
    length(x) > 0,
    "gecode_arg_min: empty array",
    if break_ties then
      arg_min(array1d(x)) + min(index_set(x)) - 1 = index
    else
      gecode_arg_min_bool(x, 0, index - min(index_set(x)), false)
    endif,
  );
/** @group gecode.constraints
  Constrains \a index to be the index of a largest value in \a x.
  If \a break_ties is true, the smallest qualifying index is required;
  otherwise any qualifying index is permitted. The array must be nonempty.
*/
predicate gecode_arg_max(
  array[int] of var int: x,
  var int: index,
  bool: break_ties,
) =
  assert(
    length(x) > 0,
    "gecode_arg_max: empty array",
    if break_ties then
      arg_max(array1d(x)) + min(index_set(x)) - 1 = index
    else
      gecode_arg_max_int(x, 0, index - min(index_set(x)), false)
    endif,
  );
/** @group gecode.constraints
  Constrains \a index to be the index of a largest value in \a x.
  If \a break_ties is true, the smallest qualifying index is required;
  otherwise any qualifying index is permitted. The array must be nonempty.
*/
predicate gecode_arg_max(
  array[int] of var bool: x,
  var int: index,
  bool: break_ties,
) =
  assert(
    length(x) > 0,
    "gecode_arg_max: empty array",
    if break_ties then
      arg_max(array1d(x)) + min(index_set(x)) - 1 = index
    else
      gecode_arg_max_bool(x, 0, index - min(index_set(x)), false)
    endif,
  );

/** @group gecode.constraints
  Requires exactly one element of \a x to be true, at zero-based
  position \a value - \a offset.
*/
predicate gecode_channel(array[int] of var bool: x, var int: value, int: offset) =
  gecode_bool_int_channel(x, value, offset);
/** @group gecode.constraints
  Constrains \a x to be 1.0 when \a b is true and 0.0 otherwise.
*/
predicate gecode_channel(var bool: b, var float: x) =
  gecode_bool_float_channel(b, x);
