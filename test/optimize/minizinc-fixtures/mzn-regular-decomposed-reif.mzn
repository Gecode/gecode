include "regular.mzn"; array[1..2] of var 1..2:x; var bool:b; constraint b <-> regular(x,1,2,[|1,0|],1,{1}); solve maximize bool2int(b); output [show(x),"\n"];
