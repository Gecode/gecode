/* Set `out` to be a random value between `low` and `high` (inclusive) from a uniform distribution */
predicate fzn_on_restart_uniform_int(int: low, int: high, var int: out) = gecode_on_restart_uniform_int(low, high, out);
predicate gecode_on_restart_uniform_int(int: low, int: high, var int: out);
