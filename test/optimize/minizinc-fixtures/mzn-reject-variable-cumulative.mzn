include "cumulative.mzn"; var 1..2:d; var 0..2:x; constraint cumulative([x,1,2],[d,1,1],[1,1,1],2); solve satisfy; output [show([x,d]),"\n"];
