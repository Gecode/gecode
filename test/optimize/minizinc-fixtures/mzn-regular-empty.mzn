include "regular.mzn"; constraint regular([],1,1,[|1|],1,{1}); solve satisfy; output ["[]\n"];
