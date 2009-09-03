%-----------------------------------------------------------------------------%
% Requires at least 'n' variables in 'x' to take the value 'v'.
%-----------------------------------------------------------------------------%
predicate at_least_set(int: n, array[int] of var set of int: x, set of int: v) =
    bool_sum_ge([x[i] == v | i in index_set(x)], n);
