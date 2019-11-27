predicate gecode_minimum_arg_bool_offset(array[int] of var bool: x, int: offset, var int: i);

predicate fzn_minimum_arg_bool(array[int] of var bool: x, var int: i) =
  gecode_minimum_arg_bool_offset(x,min(index_set(x)),i);
