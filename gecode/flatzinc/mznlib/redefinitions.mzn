% Sums over Boolean variables
predicate bool_lin_eq(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_ne(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_le(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_lt(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_ge(array[int] of int: a, array[int] of var bool: x,
                      var int: c);
predicate bool_lin_gt(array[int] of int: a, array[int] of var bool: x,
                      var int: c);

predicate bool_sum_eq(array[int] of var bool: x, var int: c) =
    bool_lin_eq([1 | i in index_set(x)],x,c);
predicate bool_sum_ne(array[int] of var bool: x, var int: c) =
    bool_lin_ne([1 | i in index_set(x)],x,c);
predicate bool_sum_le(array[int] of var bool: x, var int: c) =
    bool_lin_le([1 | i in index_set(x)],x,c);
predicate bool_sum_lt(array[int] of var bool: x, var int: c) =
    bool_lin_lt([1 | i in index_set(x)],x,c);
predicate bool_sum_ge(array[int] of var bool: x, var int: c) =
    bool_lin_ge([1 | i in index_set(x)],x,c);
predicate bool_sum_gt(array[int] of var bool: x, var int: c) =
    bool_lin_gt([1 | i in index_set(x)],x,c);
