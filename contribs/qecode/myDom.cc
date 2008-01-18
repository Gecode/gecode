
#ifndef QECODE_MYDOM
#define QECODE_MYDOM
#include "gecode/minimodel.hh"
using namespace Gecode;
using namespace Gecode::Int;
using namespace std;

void myAntidom_int(Space* home, IntVar x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    IntView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(home,xv.minus_r(home,ris));
}

void myAntidom_bool(Space* home, BoolVar x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    BoolView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(home,xv.minus_r(home,ris));
}

/*
void myDom_int(Space* home, IntVar x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    IntView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(home,xv.inter_r(home,ris));
}

void myDom_bool(Space* home, BoolVar x, const IntSet& is, IntConLevel) {
    if (home->failed()) return;
    BoolView xv(x);
    IntSetRanges ris(is);
    GECODE_ME_FAIL(home,xv.inter_r(home,ris));
}
*/

#endif
