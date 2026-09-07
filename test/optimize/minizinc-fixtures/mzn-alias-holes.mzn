var {1,3,7}:x; var 0..9:y=x; constraint y>=2; solve minimize y; output [show([x,y]),"\n"];
