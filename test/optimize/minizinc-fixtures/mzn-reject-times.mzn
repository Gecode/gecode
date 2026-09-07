var 1..3:x; var 1..3:y; var 1..9:z; constraint z=x*y; solve minimize z; output [show([x,y,z]),"\n"];
