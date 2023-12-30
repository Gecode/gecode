/* Set `out` to be a the last solution value of `input` on each restart */
predicate fzn_on_restart_sol_float(var float: input, var float: out) = gecode_on_restart_sol_float(input, out);
predicate gecode_on_restart_sol_float(var float: input, var float: out)
