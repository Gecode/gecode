include "table.mzn"; var 1..2:x; constraint table([x,x],[|1,1|2,2|]); solve minimize x; output [show([x,x]),"\n"];
