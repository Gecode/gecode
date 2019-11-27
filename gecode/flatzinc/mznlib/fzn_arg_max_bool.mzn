predicate gecode_maximum_arg_bool_offset(array[int] of var bool: x, int: offset, var int: i);

predicate fzn_maximum_arg_bool(array[int] of var bool: x, var int: i) =
  gecode_maximum_arg_bool_offset(x,min(index_set(x)),i);
