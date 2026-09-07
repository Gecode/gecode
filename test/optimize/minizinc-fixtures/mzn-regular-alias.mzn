include "regular.mzn"; var 1..2:x; constraint regular([x,x,x],2,2,[|2,1|1,2|],1,{1}); solve minimize x; output [show([x,x,x]),"\n"];
