/* Set `out` to be a the last solution value of `input` on each restart */
predicate fzn_on_restart_sol_set(var set of int: input, var set of int: out) = gecode_on_restart_sol_set(input, out);
predicate gecode_on_restart_sol_set(var set of int: input, var set of int: out);
