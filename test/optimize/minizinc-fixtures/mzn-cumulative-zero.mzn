include "cumulative.mzn"; var 0..2:x; constraint cumulative([x,1,1],[0,1,1],[2,1,1],2); solve minimize x; output [show([x]),"\n"];
