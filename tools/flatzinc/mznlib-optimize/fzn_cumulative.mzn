% The frontend accepts parameter values or original singleton domains only.
% Do not replace data by lb/ub inferred through propagation.
predicate gecode_cumulatives(array[int] of var int: s, array[int] of var int: d,
                             array[int] of var int: r, var int: b);
predicate fzn_cumulative(array[int] of var int: s, array[int] of var int: d,
                         array[int] of var int: r, var int: b) =
  gecode_cumulatives(s, d, r, b);
