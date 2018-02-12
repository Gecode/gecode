include "cumulative.mzn";

predicate disjunctive_strict(array[int] of var int: s,
                             array[int] of var int: d) =
    assert(index_set(s) == index_set(d),
        "disjunctive: the array arguments must have identical index sets",
        forall (i in index_set(d)) (d[i] >= 0) /\
        if forall (i in index_set(d)) (is_fixed(d[i])) then
          cumulative(s,d,array1d(index_set(d),[1 | i in index_set(d)]),1)
        else
          forall (i,j in index_set(d) where i<j) (
            s[i]+d[i] <= s[j] \/ s[j]+d[j] <= s[i]
          )
        endif
    );
