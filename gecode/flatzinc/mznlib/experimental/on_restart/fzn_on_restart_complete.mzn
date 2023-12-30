/* Abandon and mark search as complete when argument variable is assigned `true'. */
predicate fzn_on_restart_complete(var bool: marker) = gecode_on_restart_complete(marker);
predicate gecode_on_restart_complete(var bool: marker);
