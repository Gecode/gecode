%-----------------------------------------------------------------------------%
% Requires exactly 'n' variables in 'x' to take the value 'v'.
%-----------------------------------------------------------------------------%

include "count.mzn";

predicate exactly_int(int: n, array[int] of var int: x, int: v) =
    count(x, v, n);
