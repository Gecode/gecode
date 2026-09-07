include "table.mzn"; array[1..2] of var 1..2:x; constraint table(x,[|1,2|2,1|]); solve minimize x[1]; output [show(x),"\n"];
