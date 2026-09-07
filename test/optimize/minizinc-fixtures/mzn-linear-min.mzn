var 0..3:x; var 0..3:y; constraint x+y>=3; solve minimize 2*x+y-4; output [show([x,y,2*x+y-4]),"\n"];
