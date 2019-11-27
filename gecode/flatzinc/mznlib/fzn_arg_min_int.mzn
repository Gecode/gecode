predicate gecode_minimum_arg_int_offset(array[int] of var int: x, int: offset, var int: i);

predicate fzn_minimum_arg_int(array[int] of var int: x, var int: i) =
  gecode_minimum_arg_int_offset(x,min(index_set(x)),i);
