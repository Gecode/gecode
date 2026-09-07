% This library intentionally does not inherit Gecode's broad native registry.
% Remaining standard decompositions must pass the frontend's whole-model check.
predicate int_eq_imp(var int: x, var int: y, var bool: b);
predicate int_le_imp(var int: x, var int: y, var bool: b);
predicate int_lin_le_imp(array[int] of int: a, array[int] of var int: x, int: b, var bool: r);
% Fail early for selected primitive families that cannot be compiled faithfully.
predicate int_times(var int: x, var int: y, var int: z) =
  abort("Gecode Optimize: variable multiplication is unsupported");
predicate int_div(var int: x, var int: y, var int: z) =
  abort("Gecode Optimize: variable division is unsupported");
predicate int_mod(var int: x, var int: y, var int: z) =
  abort("Gecode Optimize: variable modulo is unsupported");
predicate int_eq_reif(var int: x, var int: y, var bool: b) =
  abort("Gecode Optimize: reified integer equality is unsupported");
predicate int_lin_eq_reif(array[int] of int: a, array[int] of var int: x, int: b, var bool: r) =
  abort("Gecode Optimize: reified linear equality is unsupported");
