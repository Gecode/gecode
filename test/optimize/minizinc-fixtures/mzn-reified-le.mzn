var 0..2:x; var bool:b; constraint b <-> (x<=1); constraint b; solve maximize x; output [show([x,bool2int(b)]),"\n"];
