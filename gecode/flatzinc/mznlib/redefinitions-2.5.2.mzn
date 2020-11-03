predicate gecode_int_element2d(var int: idx1, var int: idx2, array[int] of var int: x, set of int: idxset1, set of int: idxset2, var int: c);

predicate gecode_bool_element2d(var int: idx1, var int: idx2, array[int] of var bool: x, set of int: idxset1, set of int: idxset2, var bool: c);

predicate array_var_int_element2d_nonshifted(var int: idx1, var int: idx2, array[int,int] of var int: x, var int: c) =
  gecode_int_element2d(idx1,idx2,array1d(x),index_set_1of2(x),index_set_2of2(x),c);

predicate array_var_bool_element2d_nonshifted(var int: idx1, var int: idx2, array[int,int] of var bool: x, var bool: c) =
  gecode_bool_element2d(idx1,idx2,array1d(x),index_set_1of2(x),index_set_2of2(x),c);

predicate array_var_float_element2d_nonshifted(var int: idx1, var int: idx2, array[int,int] of var float: x, var float: c) =
  let {
    int: dim = card(index_set_2of2(x));
    int: min_flat = min(index_set_1of2(x))*dim+min(index_set_2of2(x))-1;
  } in array_var_float_element_nonshifted((idx1*dim+idx2-min_flat)::domain, array1d(x), c);

predicate array_var_set_element2d_nonshifted(var int: idx1, var int: idx2, array[int,int] of var set of int: x, var set of int: c) =
  let {
    int: dim = card(index_set_2of2(x));
    int: min_flat = min(index_set_1of2(x))*dim+min(index_set_2of2(x))-1;
  } in array_var_set_element_nonshifted((idx1*dim+idx2-min_flat)::domain, array1d(x), c);

