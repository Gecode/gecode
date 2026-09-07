include "regular.mzn"; var 1..1:x; constraint regular([x],1,1,[|0|],1,{1}); solve satisfy; output [show([x]),"\n"];
