/* Set `out` to be a the last solution value of `input` on each restart */
predicate fzn_on_restart_sol_int(var int: input, var int: out) = gecode_on_restart_sol_int(input, out);
predicate gecode_on_restart_sol_int(var int: input, var int: out);
