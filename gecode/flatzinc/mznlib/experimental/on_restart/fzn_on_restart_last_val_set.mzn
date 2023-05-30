/* Set `out` to be a the last assigned value of `input` on each restart */
predicate fzn_on_restart_last_val_set(var set of int: input, var set of int: out) = gecode_on_restart_last_val_set(input, out);
predicate gecode_on_restart_last_val_set(var set of int: input, var set of int: out)
