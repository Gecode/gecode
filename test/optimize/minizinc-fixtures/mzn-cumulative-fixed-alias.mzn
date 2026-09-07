include "cumulative.mzn"; var 1..1:d; var 1..1:r=d; var 0..2:x; constraint cumulative([x,1,1],[d,1,1],[r,1,1],2); solve minimize x; output [show([x,d,r]),"\n"];
