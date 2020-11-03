predicate symmetry_breaking_constraint(var bool: b) = b;

predicate redundant_constraint(var bool: b) = b;

predicate gecode_bool_element(var int: idx, int: idxoffset, array[int] of var bool: x, var bool: c);

predicate gecode_int_element(var int: idx, int: idxoffset, array[int] of var int: x, var int: c);

predicate array_var_bool_element_nonshifted(var int: idx, array[int] of var bool: x, var bool: c) =
  gecode_bool_element(idx, min(index_set(x)), array1d(x), c);

predicate array_var_int_element_nonshifted(var int: idx, array[int] of var int: x, var int: c) =
  gecode_int_element(idx, min(index_set(x)), array1d(x), c);

predicate array_var_float_element_nonshifted(var int: idx, array[int] of var float: x, var float: c) =
  array_var_float_element((idx-(min(index_set(x))-1))::domain,array1d(x),c);

predicate array_var_set_element_nonshifted(var int: idx, array[int] of var set of int: x, var set of int: c) =
  array_var_set_element((idx-(min(index_set(x))-1))::domain,array1d(x),c);
