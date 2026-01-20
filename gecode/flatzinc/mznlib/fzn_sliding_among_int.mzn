% Every subsequence of x of length l has at least m and at most occurrences
% of the values in S
predicate gecode_among_seq_int(array[int] of var int: x, set of int: S,
    int: l, int: m, int: n);

predicate fzn_sliding_among_int(int: low, int: up, int: l, array [int] of var int: x, set of int: S) =
    gecode_among_seq_int(x, S, l, low, up);
