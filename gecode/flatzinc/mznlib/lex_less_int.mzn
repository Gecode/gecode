%-----------------------------------------------------------------------------%
% Requires that the array 'x' is lexicographically less than array 'y'.
% Compares them from first to last element, regardless of indices
%-----------------------------------------------------------------------------%
predicate array_int_lt(array[int] of var int: x,
                       array[int] of var int: y);

predicate lex_less_int(array[int] of var int: x,
                       array[int] of var int: y) =
    array_int_lt(x,y);
