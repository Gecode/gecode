predicate gecode_maximum_arg_int_offset(array[int] of var int: x, int: offset, var int: i);

predicate maximum_arg_int(array[int] of var int: x, var int: i) =
  gecode_maximum_arg_int_offset(x,min(index_set(x)),i);
