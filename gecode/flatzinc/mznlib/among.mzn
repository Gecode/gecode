%-----------------------------------------------------------------------------%
% Requires exactly 'n' variables in 'x' to take one of the values in 'v'.
%-----------------------------------------------------------------------------%
predicate among(var int: n, array[int] of var int: x, set of int: v) =
    bool_sum_eq([x[i] in v | i in index_set(x)], n);
