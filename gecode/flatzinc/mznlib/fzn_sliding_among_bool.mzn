% Every subsequence of x of length l has at least m and at most occurrences
% of the value b
predicate gecode_among_seq_bool(array[int] of var bool: x, bool: b,
    int: l, int: m, int: n);

predicate fzn_sliding_among_bool(int: low, int: up, int: l, array [int] of var bool: x, bool: b) =
    gecode_among_seq_bool(x, b, l, low, up);
