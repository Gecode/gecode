include "all_different.mzn"; array[1..3] of var 1..2:x; constraint all_different(x); solve satisfy; output [show(x),"\n"];
