% Preserve the exact predicate admitted by the optimization frontend.
predicate all_different_int(array[int] of var int: x);
predicate fzn_all_different_int(array[int] of var int: x) = all_different_int(x);
