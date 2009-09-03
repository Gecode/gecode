%-----------------------------------------------------------------------------%
% Requires at most 'n' variables in 'x' to take the value 'v'.
%-----------------------------------------------------------------------------%
predicate at_most_set(int: n, array[int] of var set of int: x, set of int: v) =
    bool_sum_le([x[i] == v | i in index_set(x)], n);
