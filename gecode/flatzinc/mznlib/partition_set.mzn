%-----------------------------------------------------------------------------%
% Partitions a 'universe' into disjoint sets.
%-----------------------------------------------------------------------------%
predicate array_set_partition(array[int] of var set of int: S,
                              set of int: universe);

predicate partition_set(array[int] of var set of int: S,
                        set of int: universe) =
    array_set_partition(S, universe);
