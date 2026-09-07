include "circuit.mzn"; array[-2..0] of var -2..0:x; constraint circuit(x); solve minimize x[-2]; output [show(x),"\n"];
