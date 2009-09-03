%-----------------------------------------------------------------------------%
% FlatZinc/Gecode builtin library.
%
% This library contains definitions of all the constraints supported by the
% FlatZinc/Gecode interpreter.
%
% You can use it as a reference that lists all constraints that are understood
% by the interpreter. You can also use it as a basis for translating FlatZinc
% that is specialized for Gecode into FlatZinc for your solver: Just add the
% necessary definitions to this file, include it from a FlatZinc source and
% run it through mzn2fzn.
%
%-----------------------------------------------------------------------------%

%-----------------------------------------------------------------------------%
% 'all_different' constrains an array of objects to be all different.
%-----------------------------------------------------------------------------%
predicate all_different(array[int] of var int:x);

%-----------------------------------------------------------------------------%
% 'all_different_offset' constrains all the x[i]+c[i] to be different.
%-----------------------------------------------------------------------------%
predicate all_different(array[int] of int: c, array[int] of var int:x);

% x = y
predicate int_eq(var int:x, var int:y);

% x != y
predicate int_ne(var int:x, var int:y);

% x >= y
predicate int_ge(var int:x, var int:y);

% x > y
predicate int_gt(var int:x, var int:y);

% x <= y
predicate int_le(var int:x, var int:y);

% x < y
predicate int_lt(var int:x, var int:y);

% (x = y) <=> b
predicate int_eq_reif(var int:x, var int:y, var bool:b);

% (x != y) <=> b
predicate int_ne_reif(var int:x, var int:y, var bool:b);

% (x >= y) <=> b
predicate int_ge_reif(var int:x, var int:y, var bool:b);

% (x > y) <=> b
predicate int_gt_reif(var int:x, var int:y, var bool:b);

% (x <= y) <=> b
predicate int_le_reif(var int:x, var int:y, var bool:b);

% (x < y) <=> b
predicate int_lt_reif(var int:x, var int:y, var bool:b);

% sum(c[i]*x[i]) = y
predicate int_lin_eq(array[int] of int:c,array[int] of var int:x,int y);

% ( sum(c[i]*x[i]) = y ) <=> b
predicate int_lin_eq_reif(array[int] of int:c,array[int] of var int:x,
                          int y,var bool:b);

% sum(c[i]*x[i]) <= y
predicate int_lin_le(array[int] of int:c,array[int] of var int:x,int y);

% ( sum(c[i]*x[i]) <= y ) <=> b
predicate int_lin_le_reif(array[int] of int:c,array[int] of var int:x,
                          int y,var bool:b);

% sum(c[i]*x[i]) >= y
predicate int_lin_ge(array[int] of int:c,array[int] of var int:x,int y);

% ( sum(c[i]*x[i]) >= y ) <=> b
predicate int_lin_ge_reif(array[int] of int:c,array[int] of var int:x,
                          int y,var bool:b);

% x+y=z
predicate int_plus(var int:x, var int:y, var int:z);

% x-y=z
predicate int_minus(var int:x, var int:y, var int:z);

% x*y=z
predicate int_times(var int:x, var int:y, var int:z);

% x div y = z
predicate int_div(var int:x, var int:y, var int:z);

% x mod y = z
predicate int_mod(var int:x, var int:y, var int:z)

% min(x,y) = z
predicate int_min(var int:x, var int:y, var int:z);

% max(x,y) = z
predicate int_max(var int:x, var int:y, var int:z);

% abs(x) = y
predicate int_abs(var int:x, var int:y);

% x = -y
predicate int_negate(var int:x, var int:y);

% x = y
predicate bool_eq(var bool:x, var bool:y);

% (x=y) <=> b
predicate bool_eq_reif(var bool:x, var bool:y, var bool:b);

% x != y
predicate bool_ne(var bool:x, var bool:y);

% x >= y
predicate bool_ge(var bool:x, var bool:y);

% x <= y
predicate bool_le(var bool:x, var bool:y);

% (x or y) <=> z
predicate bool_or(var bool:x, var bool:y, var bool:z);

% (x and y) <=> z
predicate bool_and(var bool:x, var bool:y, var bool:z);

% (x xor y) <=> z
predicate bool_xor(var bool:x, var bool:y, var bool:z);

% (x <= y) <=> z
predicate bool_left_imp(var bool:x, var bool:y, var bool:z);

% (x => y) <=> z
predicate bool_right_imp(var bool:x, var bool:y, var bool:z);

% x = !y
predicate bool_not(var bool:x, var bool:y);

% (x[0] and ... and x[n-1]) <=> y
predicate array_bool_and(array[int] of var bool:x, var bool:y);

% (x[0] or ... or x[n-1]) <=> y
predicate array_bool_or(array[int] of var bool:x, var bool:y);

% z=y[x]
predicate array_int_element(var int:x, array[int] of int:y, var int:z);

% z=y[x]
predicate array_var_int_element(var int:x, array[int] of var int:y,
                                var int:z);

% z=y[x]
predicate array_bool_element(var int:x, array[int] of bool:y, var bool:z);
% z=y[x]
predicate array_var_bool_element(var int:x, array[int] of var bool:y,
                                 var bool:z);

% x is lexicographically smaller than y
predicate array_int_lt(array[int] of var int:x, array[int] of var int:y);

% #{i | x[i]=y } = z
predicate count(array[int] of var int:x, var int:y, var int:z);

% min(x) = y
predicate minimum(array[int] of var int:x, var int y);

%
% Finite Set Constraints
%

% x = y
predicate set_eq(var set of int:x, var set of int:y);

% x != y
predicate set_ne(var set of int:x, var set of int:y);

% x union y = z
predicate set_union(var set of int:x, var set of int:y, var set of int:z);

% x intersect y = z
predicate set_intersect(var set of int:x, var set of int:y, var set of int:z);

% x-y=z
predicate set_diff(var set of int:x, var set of int:y, var set of int:z);

% x subset y
predicate set_subset(var set of int:x, var set of int:y);

% y subset x
predicate set_superset(var set of int:x, var set of int:y);

% #x = y
predicate set_card(var set of int:x, var int:y);

% x in y
predicate set_in(var int:x, var int:y);

% x disjoint y
predicate set_disjoint(var int:x, var int:y);

% (x=y) <=> b
predicate set_eq_reif(var set of int:x, var set of int:y, var bool:b);

% (x != y) <=> b
predicate set_ne_reif(var set of int:x, var set of int:y, var bool:b);

% (x subset y) <=> b
predicate set_subset_reif(var set of int:x, var set of int:y, var bool:b);

% (y subset x) <=> b
predicate set_superset_reif(var set of int:x, var set of int:y, var bool:b);

% (x in y) <=> b
predicate set_in_reif(var int:x, var set of int:y, var bool:b);

% z=y[x]
predicate array_set_element(var int:x, array[int] of set of int:y,
                            var set of int: z);

% z=y[x]
predicate array_var_set_element(var int:x, array[int] of var set of int:y,
                                var set of int: z);

% y = x[0] union x[1] union ... union x[n]
predicate array_set_union(array[int] of var set of int:x, var set of int:y);

% y = x[0] union x[1] union ... union x[n], all x[i] disjoint
predicate array_set_partition(array[int] of var set of int:x,
                              var set of int:y);

% x is convex
predicate set_convex(var set of int: x);

% all elements in x[i] are smaller than those in x[i+1]
predicate array_set_seq(array[int] of var set of int:x);

% all elements in x[i] are smaller than those in x[i+1],
% and y is the union of all x[i]
predicate array_set_seq_union(array[int] of var set of int:x,
                              var set of int:y);

% z = union(y[i]) where i in x
predicate array_set_element_union(var set of int:x,
                                  array[int] of set of int:y,
                                  var set of int: z);

% z = union(y[i]) where i in x
predicate array_var_set_element_union(var set of int:x,
                                      array[int] of var set of int:y,
                                      var set of int: z);
