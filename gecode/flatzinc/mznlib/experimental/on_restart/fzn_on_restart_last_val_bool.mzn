/* Set `out` to be a the last assigned value of `input` on each restart */
predicate fzn_on_restart_last_val_bool(var bool: input, var bool: out) = gecode_on_restart_last_val_bool(input, out);
predicate gecode_on_restart_last_val_bool(var bool: input, var bool: out)
