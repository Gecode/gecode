predicate gecode_table_int(array[int] of var int: x, array[int] of int: tuples);
predicate fzn_table_int(array[int] of var int: x, array[int,int] of int: t) =
  assert(length(x) > 0, "Gecode Optimize: zero-arity flat tables are unsupported",
         gecode_table_int(x, array1d(t)));
