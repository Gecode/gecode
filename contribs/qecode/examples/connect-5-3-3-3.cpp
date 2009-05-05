// Quantified/alternating formulation of the Connect4 game
// with a board of size 3x3 (widthxheight), a winning
// line of length 3, analyzed up to 5 moves for the initial/existential player

#include "gecode/minimodel.hh"
#include "gecode/int/element.hh"

#include "qsolver_general.hh"
#include "QCOPPlus.hh"

#define UNIVERSAL true
#define EXISTENTIAL false

#define WIDTH 3
#define HEIGHT 3
#define DEPTH 9

#define VARS_PER_BOARD (WIDTH*HEIGHT)
#define VARS_PER_STEP  (VARS_PER_BOARD + 1)

#define CELL_VCODE(I,J,TIME) \
	( (((I)-1)*WIDTH) + ((J)-1) + \
	(((TIME)==0)? 0 : (((TIME))*(VARS_PER_STEP))))
#define MOVE_VCODE(TIME) \
	( VARS_PER_BOARD + ((TIME)-1)*VARS_PER_STEP )
#define AUX_VCODE(N) ( (N) +  CELL_VCODE(HEIGHT,WIDTH,DEPTH) )

#define B(I,J,TIME) p.var(CELL_VCODE(I,J,TIME))
#define M(TIME)     p.var(MOVE_VCODE(TIME))
#define AUX(N)      p.bvar(AUX_VCODE(N))

using namespace Gecode;
using namespace Gecode::MiniModel;

int main () {
            
	int scopesSize[] = {19, 10, 10, 10, 10, 10, 10, 10, 18};
    bool qts[] = {EXISTENTIAL,UNIVERSAL,EXISTENTIAL,UNIVERSAL,EXISTENTIAL,UNIVERSAL,EXISTENTIAL,UNIVERSAL,EXISTENTIAL};
	Qcop p(9, qts, scopesSize);

	p.QIntVar(0, 0, 2);  // B(1,1,0)
	p.QIntVar(1, 0, 2);  // B(1,2,0)
	p.QIntVar(2, 0, 2);  // B(1,3,0)
	p.QIntVar(3, 0, 2);  // B(2,1,0)
	p.QIntVar(4, 0, 2);  // B(2,2,0)
	p.QIntVar(5, 0, 2);  // B(2,3,0)
	p.QIntVar(6, 0, 2);  // B(3,1,0)
	p.QIntVar(7, 0, 2);  // B(3,2,0)
	p.QIntVar(8, 0, 2);  // B(3,3,0)
	p.QIntVar(9, 1, 2);  // M(1)
	p.QIntVar(10, 0, 2);  // B(1,1,1)
	p.QIntVar(11, 0, 2);  // B(1,2,1)
	p.QIntVar(12, 0, 2);  // B(1,3,1)
	p.QIntVar(13, 0, 2);  // B(2,1,1)
	p.QIntVar(14, 0, 2);  // B(2,2,1)
	p.QIntVar(15, 0, 2);  // B(2,3,1)
	p.QIntVar(16, 0, 2);  // B(3,1,1)
	p.QIntVar(17, 0, 2);  // B(3,2,1)
	p.QIntVar(18, 0, 2);  // B(3,3,1)

	// The initial board is empty
	post(*(p.space()), B(1,1,0)==0 );
	post(*(p.space()), B(1,2,0)==0 );
	post(*(p.space()), B(1,3,0)==0 );
	post(*(p.space()), B(2,1,0)==0 );
	post(*(p.space()), B(2,2,0)==0 );
	post(*(p.space()), B(2,3,0)==0 );
	post(*(p.space()), B(3,1,0)==0 );
	post(*(p.space()), B(3,2,0)==0 );
	post(*(p.space()), B(3,3,0)==0 );

	// PA (Precondition Axiom) specifying legal moves at transition 0->1
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,0)==0) || ~(M(1)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,0)==0) || ~(M(1)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,0)==0) || ~(M(1)!=3)) ) );

	// SSA (Successor State Axiom) linking time 0 to time 1
	// The player who moves is "1"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,0)==1)) || ~(B(1,1,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,0)==1)) || ~(B(1,2,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,0)==1)) || ~(B(1,3,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,0)==1)) || ~(B(2,1,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,0)==1)) || ~(B(2,2,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,0)==1)) || ~(B(2,3,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,0)==1)) || ~(B(3,1,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,0)==1)) || ~(B(3,2,1)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,0)==1)) || ~(B(3,3,1)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,0)==2)) || ~(B(1,1,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,0)==2)) || ~(B(1,2,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,0)==2)) || ~(B(1,3,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,0)==2)) || ~(B(2,1,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,0)==2)) || ~(B(2,2,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,0)==2)) || ~(B(2,3,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,0)==2)) || ~(B(3,1,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,0)==2)) || ~(B(3,2,1)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,0)==2)) || ~(B(3,3,1)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,0)==0) && ~(B(1,1,0)==0)))) || ~(B(2,1,1)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,0)==0) && ~(B(1,2,0)==0)))) || ~(B(2,2,1)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,0)==0) && ~(B(1,3,0)==0)))) || ~(B(2,3,1)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,0)==0) && ~(B(2,1,0)==0)))) || ~(B(3,1,1)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,0)==0) && ~(B(2,2,0)==0)))) || ~(B(3,2,1)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,0)==0) && ~(B(2,3,0)==0)))) || ~(B(3,3,1)==0)) ) );

	// A cell that changes value can only turn from "0" to "1" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,0)==0) && ~(B(1,1,1)!=0))) || ((~(B(1,1,1)==1) && ~(M(1)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,0)==0) && ~(B(1,2,1)!=0))) || ((~(B(1,2,1)==1) && ~(M(1)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,0)==0) && ~(B(1,3,1)!=0))) || ((~(B(1,3,1)==1) && ~(M(1)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,0)==0) && ~(B(2,1,1)!=0))) || ((~(B(2,1,1)==1) && ~(M(1)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,0)==0) && ~(B(2,2,1)!=0))) || ((~(B(2,2,1)==1) && ~(M(1)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,0)==0) && ~(B(2,3,1)!=0))) || ((~(B(2,3,1)==1) && ~(M(1)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,0)==0) && ~(B(3,1,1)!=0))) || ((~(B(3,1,1)==1) && ~(M(1)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,0)==0) && ~(B(3,2,1)!=0))) || ((~(B(3,2,1)==1) && ~(M(1)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,0)==0) && ~(B(3,3,1)!=0))) || ((~(B(3,3,1)==1) && ~(M(1)==3)))) ) );
    
    IntVarArgs b1(19);
    for( int i=0;i<19;i++) {
        b1[i] = p.var(i);
    }
    branch(*(p.space()),b1,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(19, 1, 3);  // M(2)
	p.QIntVar(20, 0, 2);  // B(1,1,2)
	p.QIntVar(21, 0, 2);  // B(1,2,2)
	p.QIntVar(22, 0, 2);  // B(1,3,2)
	p.QIntVar(23, 0, 2);  // B(2,1,2)
	p.QIntVar(24, 0, 2);  // B(2,2,2)
	p.QIntVar(25, 0, 2);  // B(2,3,2)
	p.QIntVar(26, 0, 2);  // B(3,1,2)
	p.QIntVar(27, 0, 2);  // B(3,2,2)
	p.QIntVar(28, 0, 2);  // B(3,3,2)

	// CGA (Cut-Game Axiom) between time 1 and time 2
	// If player "1" won at time 1 then player "2" cannot play

	// Non-winning condition for player 1 at time 1
	post(*(p.space()), tt( (~(B(1,1,1)!=1) || ~(B(1,2,1)!=1) || ~(B(1,3,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,1)!=1) || ~(B(2,1,1)!=1) || ~(B(3,1,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,1)!=1) || ~(B(2,2,1)!=1) || ~(B(3,3,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,1)!=1) || ~(B(2,2,1)!=1) || ~(B(1,3,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,2,1)!=1) || ~(B(2,2,1)!=1) || ~(B(3,2,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,3,1)!=1) || ~(B(2,3,1)!=1) || ~(B(3,3,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(2,1,1)!=1) || ~(B(2,2,1)!=1) || ~(B(2,3,1)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,1)!=1) || ~(B(3,2,1)!=1) || ~(B(3,3,1)!=1)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 1->2
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,1)==0) || ~(M(2)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,1)==0) || ~(M(2)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,1)==0) || ~(M(2)!=3)) ) );

	// SSA (Successor State Axiom) linking time 1 to time 2
	// The player who moves is "2"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,1)==1)) || ~(B(1,1,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,1)==1)) || ~(B(1,2,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,1)==1)) || ~(B(1,3,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,1)==1)) || ~(B(2,1,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,1)==1)) || ~(B(2,2,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,1)==1)) || ~(B(2,3,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,1)==1)) || ~(B(3,1,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,1)==1)) || ~(B(3,2,2)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,1)==1)) || ~(B(3,3,2)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,1)==2)) || ~(B(1,1,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,1)==2)) || ~(B(1,2,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,1)==2)) || ~(B(1,3,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,1)==2)) || ~(B(2,1,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,1)==2)) || ~(B(2,2,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,1)==2)) || ~(B(2,3,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,1)==2)) || ~(B(3,1,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,1)==2)) || ~(B(3,2,2)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,1)==2)) || ~(B(3,3,2)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,1)==0) && ~(B(1,1,1)==0)))) || ~(B(2,1,2)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,1)==0) && ~(B(1,2,1)==0)))) || ~(B(2,2,2)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,1)==0) && ~(B(1,3,1)==0)))) || ~(B(2,3,2)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,1)==0) && ~(B(2,1,1)==0)))) || ~(B(3,1,2)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,1)==0) && ~(B(2,2,1)==0)))) || ~(B(3,2,2)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,1)==0) && ~(B(2,3,1)==0)))) || ~(B(3,3,2)==0)) ) );

	// A cell that changes value can only turn from "0" to "2" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,1)==0) && ~(B(1,1,2)!=0))) || ((~(B(1,1,2)==2) && ~(M(2)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,1)==0) && ~(B(1,2,2)!=0))) || ((~(B(1,2,2)==2) && ~(M(2)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,1)==0) && ~(B(1,3,2)!=0))) || ((~(B(1,3,2)==2) && ~(M(2)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,1)==0) && ~(B(2,1,2)!=0))) || ((~(B(2,1,2)==2) && ~(M(2)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,1)==0) && ~(B(2,2,2)!=0))) || ((~(B(2,2,2)==2) && ~(M(2)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,1)==0) && ~(B(2,3,2)!=0))) || ((~(B(2,3,2)==2) && ~(M(2)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,1)==0) && ~(B(3,1,2)!=0))) || ((~(B(3,1,2)==2) && ~(M(2)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,1)==0) && ~(B(3,2,2)!=0))) || ((~(B(3,2,2)==2) && ~(M(2)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,1)==0) && ~(B(3,3,2)!=0))) || ((~(B(3,3,2)==2) && ~(M(2)==3)))) ) );

    IntVarArgs b2(29);
    for( int i=0;i<29;i++) {
        b2[i] = p.var(i);
    }
    branch(*(p.space()),b2,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(29, 1, 3);  // M(3)
	p.QIntVar(30, 0, 2);  // B(1,1,3)
	p.QIntVar(31, 0, 2);  // B(1,2,3)
	p.QIntVar(32, 0, 2);  // B(1,3,3)
	p.QIntVar(33, 0, 2);  // B(2,1,3)
	p.QIntVar(34, 0, 2);  // B(2,2,3)
	p.QIntVar(35, 0, 2);  // B(2,3,3)
	p.QIntVar(36, 0, 2);  // B(3,1,3)
	p.QIntVar(37, 0, 2);  // B(3,2,3)
	p.QIntVar(38, 0, 2);  // B(3,3,3)

	// CGA (Cut-Game Axiom) between time 2 and time 3
	// If player "2" won at time 2 then player "1" cannot play

	// Non-winning condition for player 2 at time 2
	post(*(p.space()), tt( (~(B(1,1,2)!=2) || ~(B(1,2,2)!=2) || ~(B(1,3,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,2)!=2) || ~(B(2,1,2)!=2) || ~(B(3,1,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,2)!=2) || ~(B(2,2,2)!=2) || ~(B(3,3,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,2)!=2) || ~(B(2,2,2)!=2) || ~(B(1,3,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,2,2)!=2) || ~(B(2,2,2)!=2) || ~(B(3,2,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,3,2)!=2) || ~(B(2,3,2)!=2) || ~(B(3,3,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(2,1,2)!=2) || ~(B(2,2,2)!=2) || ~(B(2,3,2)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,2)!=2) || ~(B(3,2,2)!=2) || ~(B(3,3,2)!=2)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 2->3
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,2)==0) || ~(M(3)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,2)==0) || ~(M(3)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,2)==0) || ~(M(3)!=3)) ) );

	// SSA (Successor State Axiom) linking time 2 to time 3
	// The player who moves is "1"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,2)==1)) || ~(B(1,1,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,2)==1)) || ~(B(1,2,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,2)==1)) || ~(B(1,3,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,2)==1)) || ~(B(2,1,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,2)==1)) || ~(B(2,2,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,2)==1)) || ~(B(2,3,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,2)==1)) || ~(B(3,1,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,2)==1)) || ~(B(3,2,3)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,2)==1)) || ~(B(3,3,3)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,2)==2)) || ~(B(1,1,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,2)==2)) || ~(B(1,2,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,2)==2)) || ~(B(1,3,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,2)==2)) || ~(B(2,1,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,2)==2)) || ~(B(2,2,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,2)==2)) || ~(B(2,3,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,2)==2)) || ~(B(3,1,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,2)==2)) || ~(B(3,2,3)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,2)==2)) || ~(B(3,3,3)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,2)==0) && ~(B(1,1,2)==0)))) || ~(B(2,1,3)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,2)==0) && ~(B(1,2,2)==0)))) || ~(B(2,2,3)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,2)==0) && ~(B(1,3,2)==0)))) || ~(B(2,3,3)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,2)==0) && ~(B(2,1,2)==0)))) || ~(B(3,1,3)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,2)==0) && ~(B(2,2,2)==0)))) || ~(B(3,2,3)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,2)==0) && ~(B(2,3,2)==0)))) || ~(B(3,3,3)==0)) ) );

	// A cell that changes value can only turn from "0" to "1" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,2)==0) && ~(B(1,1,3)!=0))) || ((~(B(1,1,3)==1) && ~(M(3)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,2)==0) && ~(B(1,2,3)!=0))) || ((~(B(1,2,3)==1) && ~(M(3)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,2)==0) && ~(B(1,3,3)!=0))) || ((~(B(1,3,3)==1) && ~(M(3)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,2)==0) && ~(B(2,1,3)!=0))) || ((~(B(2,1,3)==1) && ~(M(3)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,2)==0) && ~(B(2,2,3)!=0))) || ((~(B(2,2,3)==1) && ~(M(3)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,2)==0) && ~(B(2,3,3)!=0))) || ((~(B(2,3,3)==1) && ~(M(3)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,2)==0) && ~(B(3,1,3)!=0))) || ((~(B(3,1,3)==1) && ~(M(3)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,2)==0) && ~(B(3,2,3)!=0))) || ((~(B(3,2,3)==1) && ~(M(3)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,2)==0) && ~(B(3,3,3)!=0))) || ((~(B(3,3,3)==1) && ~(M(3)==3)))) ) );

    IntVarArgs b3(39);
    for( int i=0;i<39;i++) {
        b3[i] = p.var(i);
    }
    branch(*(p.space()),b3,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(39, 1, 3);  // M(4)
	p.QIntVar(40, 0, 2);  // B(1,1,4)
	p.QIntVar(41, 0, 2);  // B(1,2,4)
	p.QIntVar(42, 0, 2);  // B(1,3,4)
	p.QIntVar(43, 0, 2);  // B(2,1,4)
	p.QIntVar(44, 0, 2);  // B(2,2,4)
	p.QIntVar(45, 0, 2);  // B(2,3,4)
	p.QIntVar(46, 0, 2);  // B(3,1,4)
	p.QIntVar(47, 0, 2);  // B(3,2,4)
	p.QIntVar(48, 0, 2);  // B(3,3,4)

	// CGA (Cut-Game Axiom) between time 3 and time 4
	// If player "1" won at time 3 then player "2" cannot play

	// Non-winning condition for player 1 at time 3
	post(*(p.space()), tt( (~(B(1,1,3)!=1) || ~(B(1,2,3)!=1) || ~(B(1,3,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,3)!=1) || ~(B(2,1,3)!=1) || ~(B(3,1,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,3)!=1) || ~(B(2,2,3)!=1) || ~(B(3,3,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,3)!=1) || ~(B(2,2,3)!=1) || ~(B(1,3,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,2,3)!=1) || ~(B(2,2,3)!=1) || ~(B(3,2,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,3,3)!=1) || ~(B(2,3,3)!=1) || ~(B(3,3,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(2,1,3)!=1) || ~(B(2,2,3)!=1) || ~(B(2,3,3)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,3)!=1) || ~(B(3,2,3)!=1) || ~(B(3,3,3)!=1)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 3->4
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,3)==0) || ~(M(4)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,3)==0) || ~(M(4)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,3)==0) || ~(M(4)!=3)) ) );

	// SSA (Successor State Axiom) linking time 3 to time 4
	// The player who moves is "2"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,3)==1)) || ~(B(1,1,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,3)==1)) || ~(B(1,2,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,3)==1)) || ~(B(1,3,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,3)==1)) || ~(B(2,1,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,3)==1)) || ~(B(2,2,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,3)==1)) || ~(B(2,3,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,3)==1)) || ~(B(3,1,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,3)==1)) || ~(B(3,2,4)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,3)==1)) || ~(B(3,3,4)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,3)==2)) || ~(B(1,1,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,3)==2)) || ~(B(1,2,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,3)==2)) || ~(B(1,3,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,3)==2)) || ~(B(2,1,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,3)==2)) || ~(B(2,2,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,3)==2)) || ~(B(2,3,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,3)==2)) || ~(B(3,1,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,3)==2)) || ~(B(3,2,4)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,3)==2)) || ~(B(3,3,4)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,3)==0) && ~(B(1,1,3)==0)))) || ~(B(2,1,4)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,3)==0) && ~(B(1,2,3)==0)))) || ~(B(2,2,4)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,3)==0) && ~(B(1,3,3)==0)))) || ~(B(2,3,4)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,3)==0) && ~(B(2,1,3)==0)))) || ~(B(3,1,4)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,3)==0) && ~(B(2,2,3)==0)))) || ~(B(3,2,4)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,3)==0) && ~(B(2,3,3)==0)))) || ~(B(3,3,4)==0)) ) );

	// A cell that changes value can only turn from "0" to "2" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,3)==0) && ~(B(1,1,4)!=0))) || ((~(B(1,1,4)==2) && ~(M(4)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,3)==0) && ~(B(1,2,4)!=0))) || ((~(B(1,2,4)==2) && ~(M(4)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,3)==0) && ~(B(1,3,4)!=0))) || ((~(B(1,3,4)==2) && ~(M(4)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,3)==0) && ~(B(2,1,4)!=0))) || ((~(B(2,1,4)==2) && ~(M(4)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,3)==0) && ~(B(2,2,4)!=0))) || ((~(B(2,2,4)==2) && ~(M(4)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,3)==0) && ~(B(2,3,4)!=0))) || ((~(B(2,3,4)==2) && ~(M(4)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,3)==0) && ~(B(3,1,4)!=0))) || ((~(B(3,1,4)==2) && ~(M(4)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,3)==0) && ~(B(3,2,4)!=0))) || ((~(B(3,2,4)==2) && ~(M(4)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,3)==0) && ~(B(3,3,4)!=0))) || ((~(B(3,3,4)==2) && ~(M(4)==3)))) ) );

    IntVarArgs b4(49);
    for( int i=0;i<49;i++) {
        b4[i] = p.var(i);
    }
    branch(*(p.space()),b4,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(49, 1, 3);  // M(5)
	p.QIntVar(50, 0, 2);  // B(1,1,5)
	p.QIntVar(51, 0, 2);  // B(1,2,5)
	p.QIntVar(52, 0, 2);  // B(1,3,5)
	p.QIntVar(53, 0, 2);  // B(2,1,5)
	p.QIntVar(54, 0, 2);  // B(2,2,5)
	p.QIntVar(55, 0, 2);  // B(2,3,5)
	p.QIntVar(56, 0, 2);  // B(3,1,5)
	p.QIntVar(57, 0, 2);  // B(3,2,5)
	p.QIntVar(58, 0, 2);  // B(3,3,5)

	// CGA (Cut-Game Axiom) between time 4 and time 5
	// If player "2" won at time 4 then player "1" cannot play

	// Non-winning condition for player 2 at time 4
	post(*(p.space()), tt( (~(B(1,1,4)!=2) || ~(B(1,2,4)!=2) || ~(B(1,3,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,4)!=2) || ~(B(2,1,4)!=2) || ~(B(3,1,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,4)!=2) || ~(B(2,2,4)!=2) || ~(B(3,3,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,4)!=2) || ~(B(2,2,4)!=2) || ~(B(1,3,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,2,4)!=2) || ~(B(2,2,4)!=2) || ~(B(3,2,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,3,4)!=2) || ~(B(2,3,4)!=2) || ~(B(3,3,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(2,1,4)!=2) || ~(B(2,2,4)!=2) || ~(B(2,3,4)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,4)!=2) || ~(B(3,2,4)!=2) || ~(B(3,3,4)!=2)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 4->5
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,4)==0) || ~(M(5)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,4)==0) || ~(M(5)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,4)==0) || ~(M(5)!=3)) ) );

	// SSA (Successor State Axiom) linking time 4 to time 5
	// The player who moves is "1"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,4)==1)) || ~(B(1,1,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,4)==1)) || ~(B(1,2,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,4)==1)) || ~(B(1,3,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,4)==1)) || ~(B(2,1,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,4)==1)) || ~(B(2,2,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,4)==1)) || ~(B(2,3,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,4)==1)) || ~(B(3,1,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,4)==1)) || ~(B(3,2,5)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,4)==1)) || ~(B(3,3,5)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,4)==2)) || ~(B(1,1,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,4)==2)) || ~(B(1,2,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,4)==2)) || ~(B(1,3,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,4)==2)) || ~(B(2,1,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,4)==2)) || ~(B(2,2,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,4)==2)) || ~(B(2,3,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,4)==2)) || ~(B(3,1,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,4)==2)) || ~(B(3,2,5)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,4)==2)) || ~(B(3,3,5)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,4)==0) && ~(B(1,1,4)==0)))) || ~(B(2,1,5)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,4)==0) && ~(B(1,2,4)==0)))) || ~(B(2,2,5)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,4)==0) && ~(B(1,3,4)==0)))) || ~(B(2,3,5)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,4)==0) && ~(B(2,1,4)==0)))) || ~(B(3,1,5)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,4)==0) && ~(B(2,2,4)==0)))) || ~(B(3,2,5)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,4)==0) && ~(B(2,3,4)==0)))) || ~(B(3,3,5)==0)) ) );

	// A cell that changes value can only turn from "0" to "1" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,4)==0) && ~(B(1,1,5)!=0))) || ((~(B(1,1,5)==1) && ~(M(5)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,4)==0) && ~(B(1,2,5)!=0))) || ((~(B(1,2,5)==1) && ~(M(5)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,4)==0) && ~(B(1,3,5)!=0))) || ((~(B(1,3,5)==1) && ~(M(5)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,4)==0) && ~(B(2,1,5)!=0))) || ((~(B(2,1,5)==1) && ~(M(5)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,4)==0) && ~(B(2,2,5)!=0))) || ((~(B(2,2,5)==1) && ~(M(5)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,4)==0) && ~(B(2,3,5)!=0))) || ((~(B(2,3,5)==1) && ~(M(5)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,4)==0) && ~(B(3,1,5)!=0))) || ((~(B(3,1,5)==1) && ~(M(5)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,4)==0) && ~(B(3,2,5)!=0))) || ((~(B(3,2,5)==1) && ~(M(5)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,4)==0) && ~(B(3,3,5)!=0))) || ((~(B(3,3,5)==1) && ~(M(5)==3)))) ) );

    IntVarArgs b5(59);
    for( int i=0;i<59;i++) {
        b5[i] = p.var(i);
    }
    branch(*(p.space()),b5,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(59, 1, 3);  // M(6)
	p.QIntVar(60, 0, 2);  // B(1,1,6)
	p.QIntVar(61, 0, 2);  // B(1,2,6)
	p.QIntVar(62, 0, 2);  // B(1,3,6)
	p.QIntVar(63, 0, 2);  // B(2,1,6)
	p.QIntVar(64, 0, 2);  // B(2,2,6)
	p.QIntVar(65, 0, 2);  // B(2,3,6)
	p.QIntVar(66, 0, 2);  // B(3,1,6)
	p.QIntVar(67, 0, 2);  // B(3,2,6)
	p.QIntVar(68, 0, 2);  // B(3,3,6)

	// CGA (Cut-Game Axiom) between time 5 and time 6
	// If player "1" won at time 5 then player "2" cannot play

	// Non-winning condition for player 1 at time 5
	post(*(p.space()), tt( (~(B(1,1,5)!=1) || ~(B(1,2,5)!=1) || ~(B(1,3,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,5)!=1) || ~(B(2,1,5)!=1) || ~(B(3,1,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,5)!=1) || ~(B(2,2,5)!=1) || ~(B(3,3,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,5)!=1) || ~(B(2,2,5)!=1) || ~(B(1,3,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,2,5)!=1) || ~(B(2,2,5)!=1) || ~(B(3,2,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,3,5)!=1) || ~(B(2,3,5)!=1) || ~(B(3,3,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(2,1,5)!=1) || ~(B(2,2,5)!=1) || ~(B(2,3,5)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,5)!=1) || ~(B(3,2,5)!=1) || ~(B(3,3,5)!=1)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 5->6
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,5)==0) || ~(M(6)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,5)==0) || ~(M(6)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,5)==0) || ~(M(6)!=3)) ) );

	// SSA (Successor State Axiom) linking time 5 to time 6
	// The player who moves is "2"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,5)==1)) || ~(B(1,1,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,5)==1)) || ~(B(1,2,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,5)==1)) || ~(B(1,3,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,5)==1)) || ~(B(2,1,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,5)==1)) || ~(B(2,2,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,5)==1)) || ~(B(2,3,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,5)==1)) || ~(B(3,1,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,5)==1)) || ~(B(3,2,6)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,5)==1)) || ~(B(3,3,6)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,5)==2)) || ~(B(1,1,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,5)==2)) || ~(B(1,2,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,5)==2)) || ~(B(1,3,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,5)==2)) || ~(B(2,1,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,5)==2)) || ~(B(2,2,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,5)==2)) || ~(B(2,3,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,5)==2)) || ~(B(3,1,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,5)==2)) || ~(B(3,2,6)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,5)==2)) || ~(B(3,3,6)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,5)==0) && ~(B(1,1,5)==0)))) || ~(B(2,1,6)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,5)==0) && ~(B(1,2,5)==0)))) || ~(B(2,2,6)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,5)==0) && ~(B(1,3,5)==0)))) || ~(B(2,3,6)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,5)==0) && ~(B(2,1,5)==0)))) || ~(B(3,1,6)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,5)==0) && ~(B(2,2,5)==0)))) || ~(B(3,2,6)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,5)==0) && ~(B(2,3,5)==0)))) || ~(B(3,3,6)==0)) ) );

	// A cell that changes value can only turn from "0" to "2" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,5)==0) && ~(B(1,1,6)!=0))) || ((~(B(1,1,6)==2) && ~(M(6)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,5)==0) && ~(B(1,2,6)!=0))) || ((~(B(1,2,6)==2) && ~(M(6)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,5)==0) && ~(B(1,3,6)!=0))) || ((~(B(1,3,6)==2) && ~(M(6)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,5)==0) && ~(B(2,1,6)!=0))) || ((~(B(2,1,6)==2) && ~(M(6)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,5)==0) && ~(B(2,2,6)!=0))) || ((~(B(2,2,6)==2) && ~(M(6)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,5)==0) && ~(B(2,3,6)!=0))) || ((~(B(2,3,6)==2) && ~(M(6)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,5)==0) && ~(B(3,1,6)!=0))) || ((~(B(3,1,6)==2) && ~(M(6)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,5)==0) && ~(B(3,2,6)!=0))) || ((~(B(3,2,6)==2) && ~(M(6)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,5)==0) && ~(B(3,3,6)!=0))) || ((~(B(3,3,6)==2) && ~(M(6)==3)))) ) );

    IntVarArgs b6(69);
    for( int i=0;i<69;i++) {
        b6[i] = p.var(i);
    }
    branch(*(p.space()),b6,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(69, 1, 3);  // M(7)
	p.QIntVar(70, 0, 2);  // B(1,1,7)
	p.QIntVar(71, 0, 2);  // B(1,2,7)
	p.QIntVar(72, 0, 2);  // B(1,3,7)
	p.QIntVar(73, 0, 2);  // B(2,1,7)
	p.QIntVar(74, 0, 2);  // B(2,2,7)
	p.QIntVar(75, 0, 2);  // B(2,3,7)
	p.QIntVar(76, 0, 2);  // B(3,1,7)
	p.QIntVar(77, 0, 2);  // B(3,2,7)
	p.QIntVar(78, 0, 2);  // B(3,3,7)

	// CGA (Cut-Game Axiom) between time 6 and time 7
	// If player "2" won at time 6 then player "1" cannot play

	// Non-winning condition for player 2 at time 6
	post(*(p.space()), tt( (~(B(1,1,6)!=2) || ~(B(1,2,6)!=2) || ~(B(1,3,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,6)!=2) || ~(B(2,1,6)!=2) || ~(B(3,1,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,6)!=2) || ~(B(2,2,6)!=2) || ~(B(3,3,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,6)!=2) || ~(B(2,2,6)!=2) || ~(B(1,3,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,2,6)!=2) || ~(B(2,2,6)!=2) || ~(B(3,2,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,3,6)!=2) || ~(B(2,3,6)!=2) || ~(B(3,3,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(2,1,6)!=2) || ~(B(2,2,6)!=2) || ~(B(2,3,6)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,6)!=2) || ~(B(3,2,6)!=2) || ~(B(3,3,6)!=2)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 6->7
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,6)==0) || ~(M(7)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,6)==0) || ~(M(7)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,6)==0) || ~(M(7)!=3)) ) );

	// SSA (Successor State Axiom) linking time 6 to time 7
	// The player who moves is "1"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,6)==1)) || ~(B(1,1,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,6)==1)) || ~(B(1,2,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,6)==1)) || ~(B(1,3,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,6)==1)) || ~(B(2,1,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,6)==1)) || ~(B(2,2,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,6)==1)) || ~(B(2,3,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,6)==1)) || ~(B(3,1,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,6)==1)) || ~(B(3,2,7)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,6)==1)) || ~(B(3,3,7)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,6)==2)) || ~(B(1,1,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,6)==2)) || ~(B(1,2,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,6)==2)) || ~(B(1,3,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,6)==2)) || ~(B(2,1,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,6)==2)) || ~(B(2,2,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,6)==2)) || ~(B(2,3,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,6)==2)) || ~(B(3,1,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,6)==2)) || ~(B(3,2,7)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,6)==2)) || ~(B(3,3,7)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,6)==0) && ~(B(1,1,6)==0)))) || ~(B(2,1,7)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,6)==0) && ~(B(1,2,6)==0)))) || ~(B(2,2,7)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,6)==0) && ~(B(1,3,6)==0)))) || ~(B(2,3,7)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,6)==0) && ~(B(2,1,6)==0)))) || ~(B(3,1,7)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,6)==0) && ~(B(2,2,6)==0)))) || ~(B(3,2,7)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,6)==0) && ~(B(2,3,6)==0)))) || ~(B(3,3,7)==0)) ) );

	// A cell that changes value can only turn from "0" to "1" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,6)==0) && ~(B(1,1,7)!=0))) || ((~(B(1,1,7)==1) && ~(M(7)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,6)==0) && ~(B(1,2,7)!=0))) || ((~(B(1,2,7)==1) && ~(M(7)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,6)==0) && ~(B(1,3,7)!=0))) || ((~(B(1,3,7)==1) && ~(M(7)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,6)==0) && ~(B(2,1,7)!=0))) || ((~(B(2,1,7)==1) && ~(M(7)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,6)==0) && ~(B(2,2,7)!=0))) || ((~(B(2,2,7)==1) && ~(M(7)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,6)==0) && ~(B(2,3,7)!=0))) || ((~(B(2,3,7)==1) && ~(M(7)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,6)==0) && ~(B(3,1,7)!=0))) || ((~(B(3,1,7)==1) && ~(M(7)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,6)==0) && ~(B(3,2,7)!=0))) || ((~(B(3,2,7)==1) && ~(M(7)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,6)==0) && ~(B(3,3,7)!=0))) || ((~(B(3,3,7)==1) && ~(M(7)==3)))) ) );

    IntVarArgs b7(79);
    for( int i=0;i<79;i++) {
        b7[i] = p.var(i);
    }
    branch(*(p.space()),b7,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(79, 1, 3);  // M(8)
	p.QIntVar(80, 0, 2);  // B(1,1,8)
	p.QIntVar(81, 0, 2);  // B(1,2,8)
	p.QIntVar(82, 0, 2);  // B(1,3,8)
	p.QIntVar(83, 0, 2);  // B(2,1,8)
	p.QIntVar(84, 0, 2);  // B(2,2,8)
	p.QIntVar(85, 0, 2);  // B(2,3,8)
	p.QIntVar(86, 0, 2);  // B(3,1,8)
	p.QIntVar(87, 0, 2);  // B(3,2,8)
	p.QIntVar(88, 0, 2);  // B(3,3,8)

	// CGA (Cut-Game Axiom) between time 7 and time 8
	// If player "1" won at time 7 then player "2" cannot play

	// Non-winning condition for player 1 at time 7
	post(*(p.space()), tt( (~(B(1,1,7)!=1) || ~(B(1,2,7)!=1) || ~(B(1,3,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,7)!=1) || ~(B(2,1,7)!=1) || ~(B(3,1,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,1,7)!=1) || ~(B(2,2,7)!=1) || ~(B(3,3,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,7)!=1) || ~(B(2,2,7)!=1) || ~(B(1,3,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,2,7)!=1) || ~(B(2,2,7)!=1) || ~(B(3,2,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(1,3,7)!=1) || ~(B(2,3,7)!=1) || ~(B(3,3,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(2,1,7)!=1) || ~(B(2,2,7)!=1) || ~(B(2,3,7)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,1,7)!=1) || ~(B(3,2,7)!=1) || ~(B(3,3,7)!=1)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 7->8
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,7)==0) || ~(M(8)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,7)==0) || ~(M(8)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,7)==0) || ~(M(8)!=3)) ) );

	// SSA (Successor State Axiom) linking time 7 to time 8
	// The player who moves is "2"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,7)==1)) || ~(B(1,1,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,7)==1)) || ~(B(1,2,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,7)==1)) || ~(B(1,3,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,7)==1)) || ~(B(2,1,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,7)==1)) || ~(B(2,2,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,7)==1)) || ~(B(2,3,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,7)==1)) || ~(B(3,1,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,7)==1)) || ~(B(3,2,8)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,7)==1)) || ~(B(3,3,8)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,7)==2)) || ~(B(1,1,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,7)==2)) || ~(B(1,2,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,7)==2)) || ~(B(1,3,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,7)==2)) || ~(B(2,1,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,7)==2)) || ~(B(2,2,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,7)==2)) || ~(B(2,3,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,7)==2)) || ~(B(3,1,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,7)==2)) || ~(B(3,2,8)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,7)==2)) || ~(B(3,3,8)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,7)==0) && ~(B(1,1,7)==0)))) || ~(B(2,1,8)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,7)==0) && ~(B(1,2,7)==0)))) || ~(B(2,2,8)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,7)==0) && ~(B(1,3,7)==0)))) || ~(B(2,3,8)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,7)==0) && ~(B(2,1,7)==0)))) || ~(B(3,1,8)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,7)==0) && ~(B(2,2,7)==0)))) || ~(B(3,2,8)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,7)==0) && ~(B(2,3,7)==0)))) || ~(B(3,3,8)==0)) ) );

	// A cell that changes value can only turn from "0" to "2" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,7)==0) && ~(B(1,1,8)!=0))) || ((~(B(1,1,8)==2) && ~(M(8)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,7)==0) && ~(B(1,2,8)!=0))) || ((~(B(1,2,8)==2) && ~(M(8)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,7)==0) && ~(B(1,3,8)!=0))) || ((~(B(1,3,8)==2) && ~(M(8)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,7)==0) && ~(B(2,1,8)!=0))) || ((~(B(2,1,8)==2) && ~(M(8)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,7)==0) && ~(B(2,2,8)!=0))) || ((~(B(2,2,8)==2) && ~(M(8)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,7)==0) && ~(B(2,3,8)!=0))) || ((~(B(2,3,8)==2) && ~(M(8)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,7)==0) && ~(B(3,1,8)!=0))) || ((~(B(3,1,8)==2) && ~(M(8)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,7)==0) && ~(B(3,2,8)!=0))) || ((~(B(3,2,8)==2) && ~(M(8)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,7)==0) && ~(B(3,3,8)!=0))) || ((~(B(3,3,8)==2) && ~(M(8)==3)))) ) );

    IntVarArgs b8(89);
    for( int i=0;i<89;i++) {
        b8[i] = p.var(i);
    }
    branch(*(p.space()),b8,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	p.nextScope();
	p.QIntVar(89, 1, 3);  // M(9)
	p.QIntVar(90, 0, 2);  // B(1,1,9)
	p.QIntVar(91, 0, 2);  // B(1,2,9)
	p.QIntVar(92, 0, 2);  // B(1,3,9)
	p.QIntVar(93, 0, 2);  // B(2,1,9)
	p.QIntVar(94, 0, 2);  // B(2,2,9)
	p.QIntVar(95, 0, 2);  // B(2,3,9)
	p.QIntVar(96, 0, 2);  // B(3,1,9)
	p.QIntVar(97, 0, 2);  // B(3,2,9)
	p.QIntVar(98, 0, 2);  // B(3,3,9)
	p.QBoolVar(99); // AUX(1)
	p.QBoolVar(100); // AUX(2)
	p.QBoolVar(101); // AUX(3)
	p.QBoolVar(102); // AUX(4)
	p.QBoolVar(103); // AUX(5)
	p.QBoolVar(104); // AUX(6)
	p.QBoolVar(105); // AUX(7)
	p.QBoolVar(106); // AUX(8)

	// CGA (Cut-Game Axiom) between time 8 and time 9
	// If player "2" won at time 8 then player "1" cannot play

	// Non-winning condition for player 2 at time 8
	post(*(p.space()), tt( (~(B(1,1,8)!=2) || ~(B(1,2,8)!=2) || ~(B(1,3,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,8)!=2) || ~(B(2,1,8)!=2) || ~(B(3,1,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,1,8)!=2) || ~(B(2,2,8)!=2) || ~(B(3,3,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,8)!=2) || ~(B(2,2,8)!=2) || ~(B(1,3,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,2,8)!=2) || ~(B(2,2,8)!=2) || ~(B(3,2,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(1,3,8)!=2) || ~(B(2,3,8)!=2) || ~(B(3,3,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(2,1,8)!=2) || ~(B(2,2,8)!=2) || ~(B(2,3,8)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,1,8)!=2) || ~(B(3,2,8)!=2) || ~(B(3,3,8)!=2)) ) );

	// PA (Precondition Axiom) specifying legal moves at transition 8->9
	// A move is valid only if the related colum is not full
	post(*(p.space()), tt( (~(B(3,1,8)==0) || ~(M(9)!=1)) ) );
	post(*(p.space()), tt( (~(B(3,2,8)==0) || ~(M(9)!=2)) ) );
	post(*(p.space()), tt( (~(B(3,3,8)==0) || ~(M(9)!=3)) ) );

	// SSA (Successor State Axiom) linking time 8 to time 9
	// The player who moves is "1"

	// If a cell is occupied by player 1, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,8)==1)) || ~(B(1,1,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,8)==1)) || ~(B(1,2,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,8)==1)) || ~(B(1,3,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,8)==1)) || ~(B(2,1,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,8)==1)) || ~(B(2,2,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,8)==1)) || ~(B(2,3,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,8)==1)) || ~(B(3,1,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,8)==1)) || ~(B(3,2,9)==1)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,8)==1)) || ~(B(3,3,9)==1)) ) );

	// If a cell is occupied by player 2, it stays so
	post(*(p.space()), tt( (!(~(B(1,1,8)==2)) || ~(B(1,1,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,2,8)==2)) || ~(B(1,2,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(1,3,8)==2)) || ~(B(1,3,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,1,8)==2)) || ~(B(2,1,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,2,8)==2)) || ~(B(2,2,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(2,3,8)==2)) || ~(B(2,3,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,1,8)==2)) || ~(B(3,1,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,2,8)==2)) || ~(B(3,2,9)==2)) ) );
	post(*(p.space()), tt( (!(~(B(3,3,8)==2)) || ~(B(3,3,9)==2)) ) );

	// A free cell on top of a free cell stays free
	post(*(p.space()), tt( (!(((~(B(2,1,8)==0) && ~(B(1,1,8)==0)))) || ~(B(2,1,9)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,2,8)==0) && ~(B(1,2,8)==0)))) || ~(B(2,2,9)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(2,3,8)==0) && ~(B(1,3,8)==0)))) || ~(B(2,3,9)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,1,8)==0) && ~(B(2,1,8)==0)))) || ~(B(3,1,9)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,2,8)==0) && ~(B(2,2,8)==0)))) || ~(B(3,2,9)==0)) ) );
	post(*(p.space()), tt( (!(((~(B(3,3,8)==0) && ~(B(2,3,8)==0)))) || ~(B(3,3,9)==0)) ) );

	// A cell that changes value can only turn from "0" to "1" (moving player code)
	// ..and at most one such switch can happen in the whole board
	post(*(p.space()), tt( (!((~(B(1,1,8)==0) && ~(B(1,1,9)!=0))) || ((~(B(1,1,9)==1) && ~(M(9)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(1,2,8)==0) && ~(B(1,2,9)!=0))) || ((~(B(1,2,9)==1) && ~(M(9)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(1,3,8)==0) && ~(B(1,3,9)!=0))) || ((~(B(1,3,9)==1) && ~(M(9)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(2,1,8)==0) && ~(B(2,1,9)!=0))) || ((~(B(2,1,9)==1) && ~(M(9)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(2,2,8)==0) && ~(B(2,2,9)!=0))) || ((~(B(2,2,9)==1) && ~(M(9)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(2,3,8)==0) && ~(B(2,3,9)!=0))) || ((~(B(2,3,9)==1) && ~(M(9)==3)))) ) );
	post(*(p.space()), tt( (!((~(B(3,1,8)==0) && ~(B(3,1,9)!=0))) || ((~(B(3,1,9)==1) && ~(M(9)==1)))) ) );
	post(*(p.space()), tt( (!((~(B(3,2,8)==0) && ~(B(3,2,9)!=0))) || ((~(B(3,2,9)==1) && ~(M(9)==2)))) ) );
	post(*(p.space()), tt( (!((~(B(3,3,8)==0) && ~(B(3,3,9)!=0))) || ((~(B(3,3,9)==1) && ~(M(9)==3)))) ) );

	// Winning condition for player 1 at time 9
	post(*(p.space()), tt( (!(BoolExpr(AUX(1))) || (~(B(1,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(1))) || (~(B(1,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(1))) || (~(B(1,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(2))) || (~(B(1,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(2))) || (~(B(2,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(2))) || (~(B(3,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(3))) || (~(B(1,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(3))) || (~(B(2,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(3))) || (~(B(3,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(4))) || (~(B(3,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(4))) || (~(B(2,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(4))) || (~(B(1,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(5))) || (~(B(1,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(5))) || (~(B(2,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(5))) || (~(B(3,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(6))) || (~(B(1,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(6))) || (~(B(2,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(6))) || (~(B(3,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(7))) || (~(B(2,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(7))) || (~(B(2,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(7))) || (~(B(2,3,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(8))) || (~(B(3,1,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(8))) || (~(B(3,2,9)==1))) ) );
	post(*(p.space()), tt( (!(BoolExpr(AUX(8))) || (~(B(3,3,9)==1))) ) );
	post(*(p.space()), tt( (BoolExpr(AUX(1)) || BoolExpr(AUX(2)) || BoolExpr(AUX(3)) || BoolExpr(AUX(4)) || BoolExpr(AUX(5)) || BoolExpr(AUX(6)) || BoolExpr(AUX(7)) || BoolExpr(AUX(8))) ) );

    IntVarArgs b9(99);
    for( int i=0;i<99;i++) {
        b9[i] = p.var(i);
    }
    branch(*(p.space()),b9,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
    BoolVarArgs b10(8);
    b10[0]=p.bvar(99);
    b10[1]=p.bvar(100);
    b10[2]=p.bvar(101);
    b10[3]=p.bvar(102);
    b10[4]=p.bvar(103);
    b10[5]=p.bvar(104);
    b10[6]=p.bvar(105);
    b10[7]=p.bvar(106);
    branch(*(p.space()),b10,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
	// Now we set-up the space, define the solver, and run it    
	p.makeStructure();
    
    QSolver solver(&p);

	unsigned long int nodes=0;

	Strategy outcome = solver.solve(nodes);
    cout<<nodes<<" nodes crossed."<<endl;

	return 0;
}
