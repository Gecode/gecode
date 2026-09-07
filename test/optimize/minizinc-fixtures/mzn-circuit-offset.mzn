include "circuit.mzn"; array[2..4] of var 2..4:x; constraint circuit(x); solve minimize x[2]; output [show(x),"\n"];
