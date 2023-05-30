/* Set `out` to be a the last assigned value of `input` on each restart */
predicate fzn_on_restart_last_val_float(var float: input, var float: out) = gecode_on_restart_last_val_float(input, out);
predicate gecode_on_restart_last_val_float(var float: input, var float: out)
