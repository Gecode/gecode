include "regular.mzn"; array[1..2] of var 1..2:x; constraint regular(x,3,2,[|2,3|0,3|2,0|],1,{3}); solve minimize x[1]; output [show(x),"\n"];
