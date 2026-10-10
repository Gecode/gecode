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
/** @group gecode.annotations Use separate pair propagators for minimum
  distance. The default uses one propagator; both implement the same filtering.
  Combine with basic/advanced/full annotations to select matching propagation. */
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

% Arrays correspond by position unless an index agreement is stated.
% Enum order is part of the typed FlatZinc interface.
enum GecodeIntRelation = {
  GecodeEq, GecodeNe, GecodeLe, GecodeLt, GecodeGe, GecodeGt
};
/** @group gecode.constraints Set comparison. Complement uses the full Gecode
  set universe. Ordering compares characteristic functions from the smallest
  element, rather than the lexicographic order of sorted element lists. */
enum GecodeSetRelation = {
  GecodeSetEq, GecodeSetNe, GecodeSubset, GecodeSuperset, GecodeDisjoint,
  GecodeComplement, GecodeSetLe, GecodeSetLt, GecodeSetGe, GecodeSetGt
};
enum GecodeSetOperation = {
  GecodeUnion, GecodeDisjointUnion, GecodeIntersection, GecodeDifference
};
/** @group gecode.constraints FixedDuration: flex is start, fix is duration.
  FixedStart: flex is end, fix is start. FixedEnd: flex is start, fix is end. */
enum GecodeTaskType = {
  GecodeFixedDuration, GecodeFixedStart, GecodeFixedEnd
};

/** @group gecode.constraints All x[i]+offset[i] values are distinct.
  Arrays must have equal lengths. Uses the existing native offset route. */
predicate gecode_all_different(array[int] of int: offset, array[int] of var int: x) =
  gecode_all_different_offset(offset, x);

/** @group gecode.constraints Sort x into y with x[i]=y[permutation[i]]. Ties
  may be permuted; standard arg_sort instead returns a stable inverse mapping.
  The explicit-offset typed form labels output positions starting at offset. */
predicate gecode_sort_permutation(array[int] of var int: x,
    array[int] of var int: y, array[int] of var int: permutation) =
  assert(index_set(x) = index_set(permutation) /\ length(x) = length(y),
    "gecode_sort_permutation: incompatible array indexes or lengths",
    if length(y) = 0 then true else
      gecode_sort_permutation(x, y, permutation, min(index_set(y))) endif);

/** @group gecode.constraints Compare an occurrence count with n. The second
  argument selects a value set, or one target per array position.
  Existing equality/disequality translations are reused where available. */
predicate gecode_count(array[int] of var int: x, set of int: values,
    GecodeIntRelation: relation, var int: n) =
  if relation = GecodeEq then among(n, x, values)
  elseif relation = GecodeNe then
    let {var 0..length(x): c; constraint among(c, x, values);} in c != n
  else gecode_count_in_fzn(x, values, enum2int(relation), n) endif;
predicate gecode_count(array[int] of var int: x, array[int] of int: values,
    GecodeIntRelation: relation, var int: n) =
  assert(length(x) = length(values), "gecode_count: unequal array lengths",
    gecode_count_matches_fzn(x, values, enum2int(relation), n));

/** @group gecode.constraints Compare the number of distinct values with n.
  The empty array has zero values. Equality and disequality on integers reuse
  nvalue; Boolean arrays use the native Boolean propagator. */
predicate gecode_nvalues(array[int] of var int: x,
    GecodeIntRelation: relation, var int: n) =
  if relation = GecodeEq then nvalue(n, x)
  elseif relation = GecodeNe then
    let {var 0..length(x): c; constraint nvalue(c, x);} in c != n
  else gecode_nvalues_int_fzn(x, enum2int(relation), n) endif;
predicate gecode_nvalues(array[int] of var bool: x,
    GecodeIntRelation: relation, var int: n) =
  if relation = GecodeNe then
    let {var 0..2: c;
      constraint gecode_nvalues_bool_fzn(x, enum2int(GecodeEq), c);} in c != n
  else gecode_nvalues_bool_fzn(x, enum2int(relation), n) endif;

/** @group gecode.constraints Nonnegative greatest common divisor of x and y.
  Negative operands use their absolute value; gcd(0,0)=0. Reification and
  implication of this predicate select the corresponding native actors. */
predicate gecode_gcd(var int: x, var int: y, var int: result) =
  gecode_gcd_int(x,y,result);

/** @group gecode.constraints Divisor divides dividend, including 0 divides 0.
  Zero does not divide nonzero values. Supports reification and implication. */
predicate gecode_divides(var int: divisor, var int: dividend) =
  gecode_divides_int(divisor,dividend);

/** @group gecode.constraints Reify or imply the exact nary integer product.
  Empty product is one. These specialized actors differ from reifying an
  equality against a separately computed standard product. */
predicate gecode_product_reif(array[int] of var int: x, var int: result, var bool: b) =
  gecode_product_int_reif(x,result,b);
predicate gecode_product_imp(array[int] of var int: x, var int: result, var bool: b) =
  gecode_product_int_imp(x,result,b);

/** @group gecode.constraints Nary product with canonical Euclidean residue
  0 <= result < modulus, including for negative products. Empty product is one.
  Modulus positivity/range is part of the reified proposition. An inactive
  implication imposes no restriction on modulus or result. Known positive
  moduli select the fixed-modulus native actor. */
predicate gecode_product_mod(array[int] of var int: x, var int: modulus, var int: result) =
  if is_fixed(modulus) then
    if fix(modulus) > 0 then gecode_product_mod_fixed_fzn(x,fix(modulus),result)
    else gecode_product_mod_var_fzn(x,modulus,result) endif
  else gecode_product_mod_var_fzn(x,modulus,result) endif;

/** @group gecode.constraints Minimum table distance among selected sites,
  with at least two positions. Matrix axes are equal site indexes; entries
  are symmetric/nonnegative with zero diagonal. Repeated sites are allowed.
  Optional requirements matrix uses the positions' index set and imposes
  inclusive pairwise lower bounds. Select native algorithm/organization
  through propagation annotations. */
predicate gecode_minimum_distance(array[int] of var int: sites,
    array[int,int] of int: distances, var int: result) =
  assert(length(sites) >= 2 /\ length(distances) > 0 /\
      index_set_1of2(distances) = index_set_2of2(distances),
    "gecode_minimum_distance: need at least two positions and a square nonempty distance matrix",
    gecode_minimum_distance_fzn([v-min(index_set_1of2(distances)) | v in sites],
      card(index_set_1of2(distances)),array1d(distances),result));
predicate gecode_minimum_distance(array[int] of var int: sites,
    array[int,int] of int: distances, var int: result,
    array[int,int] of int: requirements) =
  assert(length(sites) >= 2 /\ length(distances) > 0 /\
      index_set_1of2(distances) = index_set_2of2(distances) /\
      index_set_1of2(requirements) = index_set(sites) /\
      index_set_2of2(requirements) = index_set(sites),
    "gecode_minimum_distance: incompatible distance/requirements matrix or too few positions",
    gecode_minimum_distance_required_fzn([v-min(index_set_1of2(distances)) | v in sites],
      card(index_set_1of2(distances)),array1d(distances),result,array1d(requirements)));

/** @group gecode.constraints Joint truncating division and remainder; y is
  nonzero. */
predicate gecode_divmod(var int: x, var int: y, var int: quotient, var int: remainder) =
  gecode_int_divmod(x,y,quotient,remainder);
/** @group gecode.constraints Integer root rounds down; negative x is allowed
  for odd degrees. Float roots use interval propagation. Degree is positive. */
predicate gecode_nroot(var int: x, int: degree, var int: y) =
  assert(degree > 0, "gecode_nroot: degree must be positive", gecode_int_nroot(x,degree,y));
predicate gecode_nroot(var float: x, int: degree, var float: y) =
  assert(degree > 0, "gecode_nroot: degree must be positive",
    if degree = 2 then float_sqrt(x,y) else gecode_float_nroot(x,degree,y) endif);

/** @group gecode.constraints Hamiltonian successor path visits every node.
  x[end]=max(index_set(x))+1 is the terminal sentinel; x is nonempty. This
  differs from standard graph path. Costs are a row-major square matrix;
  the final sentinel edge costs zero. Typed forms accept an explicit
  nonnegative node offset and flat costs. */
predicate gecode_path(array[int] of var int: x, var int: start, var int: end) =
  assert(length(x) > 0, "gecode_path: empty node array",
    gecode_path(0, [v-min(index_set(x)) | v in x],
      start-min(index_set(x)), end-min(index_set(x))));
predicate gecode_path(array[int,int] of int: costs, array[int] of var int: x,
    var int: start, var int: end, var int: cost) =
  assert(length(x) > 0 /\ index_set_1of2(costs) = index_set(x) /\
      index_set_2of2(costs) = index_set(x), "gecode_path: incompatible cost matrix",
    gecode_path_cost(array1d(costs), 0, [v-min(index_set(x)) | v in x],
      start-min(index_set(x)), end-min(index_set(x)), cost));
predicate gecode_path(array[int,int] of int: costs, array[int] of var int: x,
    var int: start, var int: end, array[int] of var int: edge_costs, var int: cost) =
  assert(length(x) > 0 /\ index_set_1of2(costs) = index_set(x) /\
      index_set_2of2(costs) = index_set(x) /\ index_set(edge_costs) = index_set(x),
    "gecode_path: incompatible cost matrix or edge costs",
    gecode_path_cost_array(array1d(costs), 0, [v-min(index_set(x)) | v in x],
      start-min(index_set(x)), end-min(index_set(x)), edge_costs, cost));

/** @group gecode.constraints Native typed tasks may fix duration, start,
  or end. These task types are not part of standard disjunctive/cumulative. */
predicate gecode_unary_typed(array[int] of GecodeTaskType: types,
    array[int] of var int: flex, array[int] of int: fix) =
  gecode_unary_typed_fzn(enum2int(types),flex,fix);
predicate gecode_unary_typed(array[int] of GecodeTaskType: types,
    array[int] of var int: flex, array[int] of int: fix, array[int] of var bool: present) =
  gecode_unary_typed_optional_fzn(enum2int(types),flex,fix,present);

predicate gecode_cumulative_typed(array[int] of GecodeTaskType: types,
    array[int] of var int: flex, array[int] of int: fix,
    array[int] of int: usage, var int: capacity) =
  gecode_cumulative_typed_fzn(enum2int(types),flex,fix,usage,capacity);
predicate gecode_cumulative_typed(array[int] of GecodeTaskType: types,
    array[int] of var int: flex, array[int] of int: fix,
    array[int] of int: usage, var int: capacity, array[int] of var bool: present) =
  gecode_cumulative_typed_optional_fzn(enum2int(types),flex,fix,usage,capacity,present);

/** @group gecode.constraints Optional nonoverlapping rectangles. Fixed sizes
  are nonnegative. Variable-size forms enforce x+width=right and y+height=top
  when present. All arrays share their index sets. */
predicate gecode_nooverlap(array[int] of var int: x, array[int] of int: width,
    array[int] of var int: y, array[int] of int: height, array[int] of var bool: present) =
  assert(index_set(x) = index_set(width) /\ index_set(x) = index_set(y) /\
      index_set(x) = index_set(height) /\ index_set(x) = index_set(present),
    "gecode_nooverlap: incompatible indexes",
    gecode_nooverlap_optional(x,width,y,height,present));
predicate gecode_nooverlap(array[int] of var int: x, array[int] of var int: width,
    array[int] of var int: right, array[int] of var int: y,
    array[int] of var int: height, array[int] of var int: top,
    array[int] of var bool: present) =
  assert(index_set(x) = index_set(width) /\ index_set(x) = index_set(right) /\
      index_set(x) = index_set(y) /\ index_set(x) = index_set(height) /\
      index_set(x) = index_set(top) /\ index_set(x) = index_set(present),
    "gecode_nooverlap: incompatible indexes",
    forall(i in index_set(x))(present[i] ->
      (x[i]+width[i]=right[i] /\ y[i]+height[i]=top[i])) /\
      gecode_nooverlap_flex_optional(x,width,right,y,height,top,present));

/** @group gecode.constraints The set is empty or an interval; the two-argument
  form returns its convex hull (empty for an empty input). */
predicate gecode_convex(var set of int: x) = gecode_set_convex(x);
predicate gecode_convex(var set of int: x, var set of int: hull) = gecode_set_convex_hull(x,hull);
/** @group gecode.constraints Ordered disjoint sets: every element precedes
  every element of subsequent nonempty sets. Optional second result is union. */
predicate gecode_set_sequence(array[int] of var set of int: x) = gecode_array_set_seq(x);
predicate gecode_set_sequence(array[int] of var set of int: x, var set of int: y) =
  gecode_array_set_seq_union(x,y);
/** @group gecode.constraints Native combined (x operation y) relation z.
  Equality for ordinary union, intersection, and difference reuses existing
  MiniZinc operations. Complement is relative to Gecode's full set universe. */
predicate gecode_set_operation(var set of int: x, GecodeSetOperation: operation,
    var set of int: y, GecodeSetRelation: relation, var set of int: z) =
  if relation = GecodeSetEq /\ operation = GecodeUnion then z = x union y
  elseif relation = GecodeSetEq /\ operation = GecodeIntersection then z = x intersect y
  elseif relation = GecodeSetEq /\ operation = GecodeDifference then z = x diff y
  else gecode_set_operation_fzn(x,enum2int(operation),y,enum2int(relation),z) endif;
/** @group gecode.constraints Intersect singleton sets {x[i]} and initial, or
  form their union/disjoint union. Difference is unsupported. */
predicate gecode_array_int_set_operation(array[int] of var int: x,
    GecodeSetOperation: operation, set of int: initial, var set of int: y) =
  assert(operation != GecodeDifference, "gecode_array_int_set_operation: difference is unsupported",
    gecode_array_int_set_operation_fzn(x,enum2int(operation),initial,y));
/** @group gecode.constraints Selected zero-based positions of x are combined
  as singleton sets. Empty intersection is universe; empty union is empty.
  Union already has a standard range route; intersection and disjoint union
  select additional native variants. */
predicate gecode_array_int_set_element(array[int] of var int: x,
    GecodeSetOperation: operation, var set of int: selected,
    var set of int: value, set of int: universe) =
  assert(operation != GecodeDifference, "gecode_array_int_set_element: difference is unsupported",
    if operation = GecodeUnion then gecode_range(x,0,selected,value)
    else gecode_array_int_set_element_fzn(x,enum2int(operation),selected,value,universe) endif);
/** @group gecode.constraints Reified set extrema. False for an empty set.
  Unlike standard min/max expressions, these post the specialized native
  equivalence propagators. Negate by passing false as the final argument. */
predicate gecode_min_reif(var set of int: s, var int: x, var bool: b) =
  gecode_set_min_reif(s,x,b);
predicate gecode_max_reif(var set of int: s, var int: x, var bool: b) =
  gecode_set_max_reif(s,x,b);

/** @group gecode.constraints Compare s with singleton {x}. Superset is
  membership and reuses the existing membership translation. Other comparisons
  use native singleton views instead of introducing a set variable. */
predicate gecode_set_singleton_rel(var set of int: s, GecodeSetRelation: relation, var int: x) =
  if relation = GecodeSuperset then x in s else
    gecode_set_singleton_rel_fzn(s,enum2int(relation),x) endif;
predicate gecode_set_singleton_rel_reif(var set of int: s, GecodeSetRelation: relation,
    var int: x, var bool: b) =
  if relation = GecodeSuperset then b <-> x in s else
    gecode_set_singleton_rel_fzn_reif(s,enum2int(relation),x,b) endif;
predicate gecode_set_singleton_rel_imp(var set of int: s, GecodeSetRelation: relation,
    var int: x, var bool: b) =
  if relation = GecodeSuperset then b -> x in s else
    gecode_set_singleton_rel_fzn_imp(s,enum2int(relation),x,b) endif;
/** @group gecode.constraints Native set implication. Equality, disequality,
  subset, and superset use standard implication routes. Disjointness,
  complement, and characteristic-function ordering use native relations. */
predicate gecode_rel_imp(var set of int: x, GecodeSetRelation: relation,
    var set of int: y, var bool: b) =
  if relation = GecodeSetEq then b -> x = y
  elseif relation = GecodeSetNe then b -> x != y
  elseif relation = GecodeSubset then b -> x subset y
  elseif relation = GecodeSuperset then b -> x superset y
  else gecode_set_rel_imp_fzn(x,enum2int(relation),y,b) endif;

/** @group gecode.constraints Fixed-base logarithm requires MPFR; base must
  be positive and differ from one. Bases 2 and 10 reuse standard routes. */
predicate gecode_log(float: base, var float: x, var float: y) =
  assert(base > 0.0 /\ base != 1.0, "gecode_log: base must be positive and different from one",
    if base = 2.0 then float_log2(x,y)
    elseif base = 10.0 then float_log10(x,y)
    else gecode_float_log_base(base,x,y) endif);

/** @group gecode.constraints Index of a minimum/maximum. break_ties=true
  reuses standard arg_min/arg_max and selects the smallest qualifying index.
  Otherwise any qualifying index is permitted. Arrays must be nonempty. */
predicate gecode_arg_min(array[int] of var int: x, var int: index, bool: break_ties) =
  assert(length(x) > 0, "gecode_arg_min: empty array",
    if break_ties then arg_min(array1d(x)) + min(index_set(x)) - 1 = index
    else gecode_arg_min_int(x,0,index-min(index_set(x)),false) endif);
predicate gecode_arg_min(array[int] of var bool: x, var int: index, bool: break_ties) =
  assert(length(x) > 0, "gecode_arg_min: empty array",
    if break_ties then arg_min(array1d(x)) + min(index_set(x)) - 1 = index
    else gecode_arg_min_bool(x,0,index-min(index_set(x)),false) endif);
predicate gecode_arg_max(array[int] of var int: x, var int: index, bool: break_ties) =
  assert(length(x) > 0, "gecode_arg_max: empty array",
    if break_ties then arg_max(array1d(x)) + min(index_set(x)) - 1 = index
    else gecode_arg_max_int(x,0,index-min(index_set(x)),false) endif);
predicate gecode_arg_max(array[int] of var bool: x, var int: index, bool: break_ties) =
  assert(length(x) > 0, "gecode_arg_max: empty array",
    if break_ties then arg_max(array1d(x)) + min(index_set(x)) - 1 = index
    else gecode_arg_max_bool(x,0,index-min(index_set(x)),false) endif);

/** @group gecode.constraints Exactly one Boolean array entry is true and its
  position plus offset is value, or a float equals 1.0/0.0 for true/false.
  The Boolean/float overload uses the native channel directly. */
predicate gecode_channel(array[int] of var bool: x, var int: value, int: offset) =
  gecode_bool_int_channel(x,value,offset);
predicate gecode_channel(var bool: b, var float: x) = gecode_bool_float_channel(b,x);
