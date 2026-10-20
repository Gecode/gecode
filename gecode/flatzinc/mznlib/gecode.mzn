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
  The basic, advanced, and full annotations may be combined with
  domain_propagation or bounds_propagation. Constraints without algorithm
  choices ignore these annotations. */
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


include "all_different.mzn";
include "gecode_fzn.mzn";

/** @group gecode.constraints
  Requires every pair of values in \a x to differ by at least \a distance.
  The distance must be nonnegative, including for empty and singleton arrays.
*/
predicate gecode_inter_distance(array[$$X] of var int: x, var int: distance) =
  if mzn_in_root_context() then
    gecode_inter_distance_fzn(array1d(x), distance)
  else
    distance >= 0 /\
    forall (i, j in index_set(x) where i < j) (
      abs(x[i] - x[j]) >= distance
    )
  endif;

/** @group gecode.constraints
  Requires the sums \a x[i] + \a offset[i] to be pairwise distinct.
  The arrays must have identical index sets.
  The offsets do not require additional variables for the sums.
*/
predicate gecode_all_different_offset(
  array[$$X] of int: offset,
  array[$$X] of var int: x,
) =
  assert(
    index_set(offset) = index_set(x),
    "gecode_all_different_offset: index set mismatch",
    if mzn_in_root_context() then
      gecode_all_different_offset_fzn(array1d(offset), array1d(x))
    else
      all_different([x[i] + offset[i] | i in index_set(x)])
    endif,
  );

/** @group gecode.constraints
  Constrains \a y to be the sorted values of \a x, with
  \a x[i] = \a y[\a permutation[i]]. Equal values may appear in either order.
  The index sets of \a x and \a permutation must agree, and all arrays
  must have equal lengths.
  Reification is not supported.
*/
predicate gecode_sort_permutation(
  array[$$X] of var $$V: x,
  array[$$Y] of var $$V: y,
  array[$$X] of var $$Y: permutation,
) =
  assert(
    index_set(x) = index_set(permutation) /\ length(x) = length(y),
    "gecode_sort_permutation: incompatible array indexes or lengths",
    if length(y) = 0 then
      true
    else
      gecode_sort_permutation(
        enum2int(array1d(x)),
        enum2int(array1d(y)),
        enum2int(array1d(permutation)),
        min(enum2int(index_set(y))),
      )
    endif,
  );

/** @group gecode.constraints
  Requires at least one corresponding pair of elements in \a x and \a y
  to differ. The arrays must have identical index sets.
*/
predicate gecode_array_ne(array[$$X] of var $$E: x, array[$$X] of var $$E: y) =
  assert(
    index_set(x) = index_set(y),
    "gecode_array_ne: index set mismatch",
    if mzn_in_root_context() then
      gecode_array_int_ne(enum2int(array1d(x)), enum2int(array1d(y)))
    else
      exists (i in index_set(x)) (x[i] != y[i])
    endif,
  );

/** @group gecode.constraints
  Requires at least one corresponding pair of Boolean elements in \a x
  and \a y to differ. The arrays must have identical index sets.
*/
predicate gecode_array_ne(array[$$X] of var bool: x, array[$$X] of var bool: y) =
  assert(
    index_set(x) = index_set(y),
    "gecode_array_ne: index set mismatch",
    if mzn_in_root_context() then
      gecode_array_bool_ne(array1d(x), array1d(y))
    else
      exists (i in index_set(x)) (x[i] != y[i])
    endif,
  );

/** @group gecode.constraints
  Requires at most \a n positions where \a x matches \a values.
  The arrays must have identical index sets.
  The fixed comparison values do not require additional difference variables.
*/
predicate gecode_count_matches_at_most(var int: n, array[$$X] of var $$E: x, array[$$X] of $$E: values) =
  assert(
    index_set(x) = index_set(values),
    "gecode_count_matches_at_most: index set mismatch",
    if mzn_in_root_context() then
      gecode_count_matches_fzn(enum2int(array1d(x)), enum2int(array1d(values)), 3, n)
    else
      sum (i in index_set(x)) (bool2int(x[i] = values[i])) <= n
    endif,
  );

/** @group gecode.constraints
  Requires at least \a n positions where \a x matches \a values.
  The arrays must have identical index sets.
  The fixed comparison values do not require additional difference variables.
*/
predicate gecode_count_matches_at_least(var int: n, array[$$X] of var $$E: x, array[$$X] of $$E: values) =
  assert(
    index_set(x) = index_set(values),
    "gecode_count_matches_at_least: index set mismatch",
    if mzn_in_root_context() then
      gecode_count_matches_fzn(enum2int(array1d(x)), enum2int(array1d(values)), 5, n)
    else
      sum (i in index_set(x)) (bool2int(x[i] = values[i])) >= n
    endif,
  );

/** @group gecode.constraints
  Constrains \a n to be the number of positions where \a x matches
  \a values. The arrays must have identical index sets.
  The fixed comparison values do not require additional difference variables.
*/
predicate gecode_count_matches(
  var int: n,
  array[$$X] of var $$E: x,
  array[$$X] of $$E: values,
) =
  assert(
    index_set(x) = index_set(values),
    "gecode_count_matches: index set mismatch",
    if mzn_in_root_context() then
      gecode_count_matches_fzn(enum2int(array1d(x)), enum2int(array1d(values)), 1, n)
    else
      sum (i in index_set(x)) (bool2int(x[i] = values[i])) = n
    endif,
  );

/** @group gecode.constraints
  Constrains \a result to be the nonnegative greatest common divisor
  of \a x and \a y. Negative operands use their absolute values, and
  gcd(0, 0) = 0.
*/
predicate gecode_gcd(var int: x, var int: y, var int: result) = gecode_gcd_int(x, y, result);

/** @group gecode.constraints
  Requires \a dividend to be an integer multiple of \a divisor.
  A zero divisor is permitted only when \a dividend is zero.
*/
predicate gecode_divides(var int: divisor, var int: dividend) = gecode_divides_int(divisor, dividend);

/** @group gecode.constraints
  Constrains \a result to be the product of \a x.
  The product of an empty array is one.
*/
predicate gecode_product(array[$$X] of var int: x, var int: result) = gecode_product_int(array1d(x), result);

/** @group gecode.constraints
  Constrains \a result to be the Euclidean remainder of the product
  of \a x modulo \a modulus, with 0 <= \a result < \a modulus.
  A nonpositive modulus makes the proposition false.
  The product of an empty array is one.
*/
predicate gecode_product_mod(
  array[$$X] of var int: x,
  var int: modulus,
  var int: result,
) =
  if is_fixed(modulus) then
    if fix(modulus) > 0 then
      gecode_product_mod_fixed_fzn(array1d(x), fix(modulus), result)
    else
      gecode_product_mod_var_fzn(array1d(x), modulus, result)
    endif
  else
    gecode_product_mod_var_fzn(array1d(x), modulus, result)
  endif;

/** @group gecode.constraints
  Constrains \a result to be the smallest distance between any two
  positions in \a sites, using \a distances[\a sites[i], \a sites[j]].
  There must be at least two positions. Both matrix axes use the same site
  indexes, and the matrix must be symmetric and nonnegative with a zero
  diagonal. Repeated sites are permitted.
  Reification is not supported.
*/
predicate gecode_minimum_distance(
  array[$$Position] of var $$Site: sites,
  array[$$Site, $$Site] of int: distances,
  var int: result,
) =
  assert(
    length(sites) >= 2 /\
    length(distances) > 0 /\
    index_set_1of2(distances) = index_set_2of2(distances),
    "gecode_minimum_distance: need at least two positions and a square nonempty distance matrix",
    gecode_minimum_distance_fzn(
      [enum2int(v) - min(enum2int(index_set_1of2(distances))) | v in sites],
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
  Reification is not supported.
*/
predicate gecode_minimum_distance(
  array[$$Position] of var $$Site: sites,
  array[$$Site, $$Site] of int: distances,
  var int: result,
  array[$$Position, $$Position] of int: requirements,
) =
  assert(
    length(sites) >= 2 /\
    length(distances) > 0 /\
    index_set_1of2(distances) = index_set_2of2(distances) /\
    index_set_1of2(requirements) = index_set(sites) /\
    index_set_2of2(requirements) = index_set(sites),
    "gecode_minimum_distance: incompatible distance/requirements matrix or too few positions",
    gecode_minimum_distance_required_fzn(
      [enum2int(v) - min(enum2int(index_set_1of2(distances))) | v in sites],
      card(index_set_1of2(distances)),
      array1d(distances),
      result,
      array1d(requirements),
    ),
  );

/** @group gecode.constraints
  Constrains \a y to be the integer root of \a x with degree \a degree,
  rounded down. The degree must be positive, and negative \a x is permitted
  only for odd degrees.
  Reification is not supported.
*/
predicate gecode_nroot(var int: x, int: degree, var int: y) =
  assert(degree > 0, "gecode_nroot: degree must be positive", gecode_int_nroot(x, degree, y));

/** @group gecode.constraints
  Requires the present rectangles with origins (\a x[i], \a y[i])
  and sizes (\a width[i], \a height[i]) not to overlap.
  Both coordinates of a rectangle must be present or absent together.
  Zero-sized rectangles still participate in non-overlap.
  Sizes must be nonnegative, and all arrays must have identical index sets.
  Reification is not supported.
*/
predicate gecode_nooverlap(
  array[$$X] of var opt int: x,
  array[$$X] of var int: width,
  array[$$X] of var opt int: y,
  array[$$X] of var int: height,
) =
  assert(
    index_set(x) = index_set(width) /\
    index_set(x) = index_set(y) /\
    index_set(x) = index_set(height),
    "gecode_nooverlap: index set mismatch",
    forall (i in index_set(x)) (
      occurs(x[i]) = occurs(y[i]) /\ width[i] >= 0 /\ height[i] >= 0
    ) /\
    let {
      array[int] of var bool: present = [occurs(xi) | xi in x];
      array[int] of var int: xx = [xi default 0 | xi in x];
      array[int] of var int: yy = [yi default 0 | yi in y];
    } in
    if forall (w in width) (is_fixed(w)) /\ forall (h in height) (is_fixed(h)) then
      gecode_nooverlap_optional(
        xx,
        [fix(w) | w in width],
        yy,
        [fix(h) | h in height],
        present,
      )
    else
      let {
        array[int] of var int: w = array1d(width);
        array[int] of var int: h = array1d(height);
        array[int] of var int: right = [if present[i] then xx[i] + w[i] else 0 endif | i in index_set(xx)];
        array[int] of var int: top = [if present[i] then yy[i] + h[i] else 0 endif | i in index_set(yy)];
      } in
      gecode_nooverlap_flex_optional(xx, w, right, yy, h, top, present)
    endif,
  );

/** @group gecode.constraints
  Requires \a x to be empty or a consecutive interval of values.
  Enum values follow their declaration order.
  Reification is not supported.
*/
predicate gecode_convex(var set of $$E: x) = gecode_set_convex(enum2int(x));

/** @group gecode.constraints
  Constrains \a hull to be the smallest consecutive interval
  containing \a x. The hull of an empty set is empty.
  Enum values follow their declaration order.
  Reification is not supported.
*/
predicate gecode_convex(var set of $$E: x, var set of $$E: hull) = gecode_set_convex_hull(enum2int(x), enum2int(hull));

/** @group gecode.constraints
  Requires every element of each set in \a x to be smaller than
  every element of each later set. Empty sets and arrays are permitted.
  Reification is not supported.
*/
predicate gecode_set_sequence(array[$$X] of var set of $$E: x) = gecode_array_set_seq(enum2int(array1d(x)));

/** @group gecode.constraints
  Requires every element of each set in \a x to be smaller than
  every element of each later set, and constrains \a y to their union.
  Empty sets and arrays are permitted. An empty array has an empty union.
  Reification is not supported.
*/
predicate gecode_set_sequence(array[$$X] of var set of $$E: x, var set of $$E: y) =
  gecode_array_set_seq_union(enum2int(array1d(x)), enum2int(y));

/** @group gecode.constraints
  Requires each pair of sets in \a x to share at most one element,
  and every set to have cardinality \a size.
  Reification is not supported.
*/
predicate gecode_at_most_one_intersection(array[$$X] of var set of $$E: x, int: size) =
  assert(
    size >= 0,
    "gecode_at_most_one_intersection: negative cardinality",
    gecode_set_atmost_one(enum2int(array1d(x)), size),
  );

/** @group gecode.constraints
  Constrains \a x to be the elements of \a values in strictly increasing order.
  Reification is not supported.
*/
predicate gecode_sorted_set(array[$$X] of var $$E: x, var set of $$E: values) =
  gecode_set_channel_sorted(enum2int(array1d(x)), enum2int(values));

/** @group gecode.constraints
  Constrains \a index to be the index of a smallest value in \a x.
  Any qualifying index is permitted. The array must be nonempty.
  Reification is not supported.
*/
predicate gecode_arg_min_any(array[$$X] of var $$E: x, var $$X: index) =
  assert(
    length(x) > 0,
    "gecode_arg_min_any: empty array",
    gecode_arg_min_int(
      enum2int(array1d(x)),
      0,
      enum2int(index) - min(enum2int(index_set(x))),
      false,
    ),
  );

/** @group gecode.constraints
  Constrains \a index to be the index of a smallest value in \a x.
  Any qualifying index is permitted. The array must be nonempty.
  Reification is not supported.
*/
predicate gecode_arg_min_any(array[$$X] of var bool: x, var $$X: index) =
  assert(
    length(x) > 0,
    "gecode_arg_min_any: empty array",
    gecode_arg_min_bool(
      array1d(x),
      0,
      enum2int(index) - min(enum2int(index_set(x))),
      false,
    ),
  );

/** @group gecode.constraints
  Constrains \a index to be the index of a largest value in \a x.
  Any qualifying index is permitted. The array must be nonempty.
  Reification is not supported.
*/
predicate gecode_arg_max_any(array[$$X] of var $$E: x, var $$X: index) =
  assert(
    length(x) > 0,
    "gecode_arg_max_any: empty array",
    gecode_arg_max_int(
      enum2int(array1d(x)),
      0,
      enum2int(index) - min(enum2int(index_set(x))),
      false,
    ),
  );

/** @group gecode.constraints
  Constrains \a index to be the index of a largest value in \a x.
  Any qualifying index is permitted. The array must be nonempty.
  Reification is not supported.
*/
predicate gecode_arg_max_any(array[$$X] of var bool: x, var $$X: index) =
  assert(
    length(x) > 0,
    "gecode_arg_max_any: empty array",
    gecode_arg_max_bool(
      array1d(x),
      0,
      enum2int(index) - min(enum2int(index_set(x))),
      false,
    ),
  );

/** @group gecode.constraints
  Requires exactly one element of \a x to be true, with \a value
  equal to its array index. The array must be nonempty.
  Reification is not supported.
*/
predicate gecode_one_hot(array[$$X] of var bool: x, var $$X: value) =
  assert(
    length(x) > 0,
    "gecode_one_hot: empty array",
    gecode_bool_int_channel(array1d(x), enum2int(value), min(enum2int(index_set(x)))),
  );
