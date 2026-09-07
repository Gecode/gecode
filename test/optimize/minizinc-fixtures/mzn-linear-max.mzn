var -2..2:x; var -2..2:y; constraint x+y<=1; solve maximize 3*x-y+2; output [show([x,y,3*x-y+2]),"\n"];
