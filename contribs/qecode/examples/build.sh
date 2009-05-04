programme=$1
source=$1.cpp
#PATH TO GECODE
gecode=../../.. # CHANGER CECI LES LAPINS
#RELATIVE PATH TO QECODE
qecode=$gecode/contribs/qecode
echo "COMPILATION :"
g++ $source -I$qecode -I$gecode -L$qecode -L$gecode -lgecodeqecode -lgecodeminimodel -lgecodeint -lgecodesearch -lgecodekernel -lgecodesupport   -ggdb -o $programme
echo "RESOLUTION :"
./$programme
