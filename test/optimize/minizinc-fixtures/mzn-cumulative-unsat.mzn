include "cumulative.mzn"; var 0..2:x; constraint cumulative([x,x,x],[1,1,1],[1,1,1],2); solve satisfy; output [show([x]),"\n"];
