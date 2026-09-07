array[-1..0] of int:a=array1d(-1..0,[3,2]); var -1..0:i; var 2..3:x; constraint x=a[i]; solve minimize x; output [show([i,x]),"\n"];
