include "all_different.mzn"; array[1..2] of var 1..2:x; constraint all_different(x); solve minimize x[1]; output [show(x),"\n"];
