predicate gecode_regular(array[int] of var int: x, int: Q, int: S,
                         array[int] of int: d, int: q0, set of int: F);
predicate fzn_regular(array[int] of var int: x, int: Q, int: S,
                      array[int,int] of int: d, int: q0, set of int: F) =
  gecode_regular(x, Q, S, array1d(d), q0, F);
