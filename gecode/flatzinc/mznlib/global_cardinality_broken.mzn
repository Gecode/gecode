%-----------------------------------------------------------------------------%
% Requires that 'c[i]' is the number of occurences of 'i' in 'x'.
%-----------------------------------------------------------------------------%
predicate global_cardinality_gecode(array[int] of var int: x,
                                    array[int] of var int: c, int: cmin);

predicate global_cardinality(array[int] of var int: x,
                             array[int] of var int: c) =
    global_cardinality_gecode(x,c,min(index_set(c)));
