%-----------------------------------------------------------------------------%
% Requires exactly 'n' variables in 'x' to take the value 'v'.
%-----------------------------------------------------------------------------%
predicate exactly_set(int: n, array[int] of var set of int: x, set of int: v) =
    bool_sum_eq([x[i] == v | i in index_set(x)], n);
