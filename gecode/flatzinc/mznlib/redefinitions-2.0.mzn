% This file contains redefinitions of standard builtins that can be overridden
% by solvers.

predicate bool_clause_reif(array[int] of var bool: as,
                           array[int] of var bool: bs,
                           var bool: b);

predicate array_int_maximum(var int: m, array[int] of var int: x);

predicate gecode_float_array_max(array[int] of var float: x, var float: m);

predicate array_float_maximum(var float: m, array[int] of var float: x) =
    if length(x) = 0 then false else gecode_float_array_max(x, m) endif;

predicate array_int_minimum(var int: m, array[int] of var int: x);

predicate gecode_float_array_min(array[int] of var float: x, var float: m);

predicate array_float_minimum(var float: m, array[int] of var float: x) =
    if length(x) = 0 then false else gecode_float_array_min(x, m) endif;

predicate gecode_float_pow(var float: x, int: n, var float: y);
predicate gecode_float_pow_base(float: base, var float: x, var float: y);

% Nonnegative integer exponents permit zero or negative bases.
predicate float_pow(var float: x, var float: exponent, var float: y) =
    let {
        float: n = if is_fixed(exponent) then fix(exponent) else -1.0 endif;
        bool: integer_exponent = if n >= 0.0 /\ n <= 2147483646.0 then
            n = int2float(floor(n)) else false endif;
    } in
    if integer_exponent then
        gecode_float_pow(x, floor(n), y)
    elseif is_fixed(x) /\ fix(x) > 0.0 then
        gecode_float_pow_base(fix(x), exponent, y)
    else
        assert(false, "Gecode float_pow requires a nonnegative integer exponent or a fixed positive base")
    endif;
