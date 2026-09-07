predicate gecode_circuit(int: offset, array[int] of var int: x);
predicate fzn_circuit(array[int] of var int: x) =
  assert(length(x) > 0, "Gecode Optimize: empty circuits are unsupported",
    if min(index_set(x)) >= 0 then
      gecode_circuit(min(index_set(x)), x)
    else
      gecode_circuit(0, [x[i] - min(index_set(x)) | i in index_set(x)])
    endif);
