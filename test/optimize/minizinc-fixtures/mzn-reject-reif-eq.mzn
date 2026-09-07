var 0..2:x; var 0..2:y; var bool:b; constraint b <-> (x=y); solve maximize bool2int(b); output [show([x,y]),"\n"];
