/****   , [ connect4.cpp ], 
 Copyright (c) 2009 Universite d'Orleans - Jeremie Vautard 
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 *************************************************************************/

/*
 This is a QCSP+ model of the complete 6*7 connect-4 game. Depth of search can be parametered. 

 The game is modeled as follows : for each turn i, the complete bard is represented by variables B_i_j (42 variables, each one representing a square of the board).
 A variable X_i represents the "decision" of curent player i.e. the column he will chose to place his counter. A set of constraints binds these variables so that the board representations are kept up to date 
 report to the decision of current player. 
 A Variable Lined_i is constrained to be set to 1 if the current player manags to align 4 counters, 0 otherwise. Lined_i-1 must be 0 for a player to b able to play. Otherwise, he looses the game, as its opponent just aligned 4 counters.
 
For each scope i, variables are : 
 - (1) X_i : the column played
 - (7) h_i_j : The height of column j
 - (1) h_i_x = The height of column X_i
 - (7) Ch_i_j : 1 if column j is played, 0 otherwise
 - (42) B_i_k = state of square (k/7,k%7) : 0 if empty, 1 if red, 2 if yellow
 - (42) C_i_k : 1 if square (k/7,k%7) is played at turn i, 0 otherwise.
 - (1) k_i : number of the square played at turn i
 - (6) LinedH_i_j : tells wether 4 pawns are alined on raw j or not.
 - (7) LinedV_i_j : tells wether 4 pawns are alined on column j or not. 
 - (6) LinedD1_i_j : tells wether 4 pawns are aligned on the j-th topleft-bottomright diagonal
 - (6) LinedD2_i_j : tells wether 4 pawns are aligned on the j-th bottomleft-topright diagonal
 - (1) Lined_i : tells wether 4 pawns are aligned anywhere in the board. (0 if not, something != 0 otherwise)
 - (1) Win_i : tells who is hte winner at this stage of the game (0 : no winner yet, 1 : red,2 : yellow).
 
 

 
 This example uses a custom branching heuristic MyB that gives priority to most critical movements (immediate win, avoiding immediate loose, etc.)
 */


#include <iostream>

#include "gecode/minimodel.hh"
#include "gecode/int/element.hh"
#include "UnblockableViewValBranching.hh"
#include "qsolver_general.hh"
#include "qsolver_unblockable.hh"
#include <time.h>
#define UNIVERSAL true
#define EXISTENTIAL false


using namespace MiniModel;


// This function gets an array of size 4 describing an alignment of 4 squares of the grid, and returns a mesure of the opportniyi for a player to place a counter in this alignment : 
// The returned score is as this : 
// counters from the two players are already present -> worth 0 (this alignment is unwinnable for either player)
// no counters are present -> worth 1 (it is possible to start filling this alignment, but playing there is far from granting an immediate victory)
// 1 counter is present : worth 30 if it belongs to the current player (who can continue filling this alignment), 10 if it belongs to the opponent (the player will cancel the possibility to win with this alignment)
// 2 counters are present : worth 400 if they belongs to the current player, 100 otherwise
// 3 counters are present : worth 20000 if they belongs to the current player (he will immediately win if he fills the last square), 2000 otherwise (the opponent will win next turn if this alignment is not blocked right now). 




int zescore(int* tab,int player) {
	int cpt1=0;
	int cpt2=0;
	for (int i=0;i<4;i++) {
		if (tab[i] == 1) cpt1++;
		else if (tab[i] == 2) cpt2++;
	}
	if (cpt1 !=0 && cpt2 !=0) {return 0;}
	int cpt=cpt1+cpt2;
	int qui= ((cpt1 != 0) ? 1 : 2);
	if (cpt == 0) return 1;
	if (cpt == 1) return ( (player == qui) ? 30 : 10);
	if (cpt == 2) return ( (player == qui) ? 400 : 100);
	return ( (player == qui) ? 20000 : 2000);
}



// Special brancher for the connect-4 game. This brancher only assigns decision variable X_i.
class MyB : Brancher {
	IntView x;
  ViewArray<Int::IntView> B;
	int player;
  mutable int val;
	
  class Description : public Choice {
  public:
    int val;
    Description(const Brancher& b, unsigned int a, int val0)
		: Choice(b,a), val(val0) {}
    virtual size_t size(void) const {
      return sizeof(Description);
    }
  };
	
  MyB(Space& home, ViewArray<Int::IntView>& Bv, IntView xv, int play) : Brancher(home), B(Bv), x(xv), val(-1), player(play) {}
	
  MyB(Space& home, bool share, MyB& b) : Brancher(home, share, b), val(b.val), player(b.player) {
		x.update(home, share, b.x);
		B.update(home, share, b.B);
  }



	public :
  virtual bool status(const Space&) const {
if (x.assigned()) return false;
		int* align = new int[4]; // buffer
    for (int i=0;i<B.size();i++) {
      if (!B[i].assigned()) {;return true;}
    }
		
		vector<int> s(7,0);
		for (int i=0;i<7;i++) {
			int j=0;
			while (j<6 && (B[i+7*j].val() != 0)) j++;
			if (j == 6 || !x.in(i)) s[i]=-1;
			else {
				for (int k=-3;k<1;k++) {
					if ( ((i+k)>=0) && ((i+k+3)<7) ) {
						align[0]=B[i+k+6*j].val(); // horizontal 
						align[1]=B[i+k+1+6*j].val();
						align[2]=B[i+k+2+6*j].val();
						align[3]=B[i+k+3+6*j].val();
						s[i] = s[i] + zescore(align,player);
						
						if ( ((j+k)>=0) && ((j+k+3)<6) ) {
							align[0]=B[i+k+6*(j+k)].val(); // diagonal up
							align[1]=B[i+k+1+6*(j+k+1)].val();
							align[2]=B[i+k+2+6*(j+k+2)].val();
							align[3]=B[i+k+3+6*(j+k+3)].val();
							s[i] += zescore(align,player);
						}

						if ( ((j-k)<6) && ((j-k-3)>=0) ) {
							align[0]=B[i+k+6*(j-k)].val(); // diagonal down
							align[1]=B[i+k+1+6*(j-k-1)].val();
							align[2]=B[i+k+2+6*(j-k-2)].val();
							align[3]=B[i+k+3+6*(j-k-3)].val();
							s[i] += zescore(align,player);
						}
					}
				}
				if (j>=3) {
					align[0]=B[i+6*(j)].val(); // vertical
					align[1]=B[i+6*(j-1)].val();
					align[2]=B[i+6*(j-2)].val();
					align[3]=B[i+6*(j-3)].val();
					s[i] += zescore(align,player);
				}
			}
		}
		int max=s[0];
		int argmax=0;
		for (int ii=0;ii<7;ii++) 
			if (max < s[ii]) {
				max = s[ii];
				argmax = ii;
			}
		val=argmax;
    delete[] align;
		if (max==-1) return false;
		return true;
	}
	
  virtual Choice* choice(Space&) {
    return new Description(*this,2,val);
  }
	
  virtual ExecStatus commit(Space& home, const Choice& d, unsigned int a) {
    const Description& desc = static_cast<const Description&>(d);
    val = -1;
    if (a)
      return me_failed(x.nq(home, desc.val))
			? ES_FAILED : ES_OK;
    else
      return me_failed(x.eq(home, desc.val))
			? ES_FAILED : ES_OK;
  }
  virtual Actor* copy(Space& home, bool share) {
    return new (home) MyB(home, share, *this);
  }
	
  static void post(Space& home, IntVarArgs B, IntVar x,int play) {
    ViewArray<Int::IntView> Bv(home, B);
		IntView xv(x);
    (void) new (home) MyB(home, Bv,xv,play);
  }
};






int main (int argc, char * const argv[]) {
	
  for (int nbScope = 3;nbScope<14;nbScope++) { // Depth of search
		clock_t start, finish;
		start=clock();

		cout<<"depth "<<nbScope<<" : ";
    bool* qtScopes = new bool[nbScope];
    for (int i=0;i<nbScope;i++) { 
			qtScopes[i] = ((i%2) != 0);
    }
    cout<<endl;
    
    int *scopesSize = new int[nbScope];
		
		
    int varByScope = 1 + 7 + 1 + 7 + 42 + 42 + 1 + 6 + 7 + 6 + 6 + 1 + 1;
    for (int i=0; i<nbScope; i++)
      scopesSize[i]=varByScope;
    
    Qcop p(nbScope, qtScopes, scopesSize);
		
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FIRST TURN ///////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		p.QIntVar(0, 0, 6); // X_i
		p.QIntVar(1+7,0,6); // h_ix
		p.QIntVar(1+7+1+7+42+42 ,0,41); // k_i
		p.QIntVar(1+7+1+7+42+42+1+6+7+6+6 ,0,25); // Lined_i
		p.QIntVar(1+7+1+7+42+42+1+6+7+6+6+1 ,0,2); // Win_i
		IntVar X_0(p.var(0));
		IntVar h_0x(p.var(1+7));
		IntVar k_0(p.var(1+7+1+7+42+42));
		IntVar Lined_0(p.var(1+7+1+7+42+42+1+6+7+6+6));
		IntVar Win_0(p.var(1+7+1+7+42+42+1+6+7+6+6+1));
		
		IntVarArgs LinedH_0(6);
		IntVarArgs LinedD1_0(6);
		IntVarArgs LinedD2_0(6);
		
		for (int j=0;j<6;j++) {
			p.QIntVar(1+7+1+7+42+42+1 +j , 0,1); // LinedH_i_j
			LinedH_0[j] = p.var(1+7+1+7+42+42+1+j);
			p.QIntVar(1+7+1+7+42+42+1+6+7 +j , 0,1); // LinedD1_i_j
			LinedD1_0[j] = p.var(1+7+1+7+42+42+1+6+7+j);
			p.QIntVar(1+7+1+7+42+42+1+6+7+6 +j , 0,1); // LinedD2_i_j
			LinedD2_0[j] = p.var(1+7+1+7+42+42+1+6+7+6+j);
		}
		
		IntVarArgs h_0(7);
		IntVarArgs Ch_0(7);
		IntVarArgs LinedV_0(7);
		for (int j=0;j<7;j++) {
			p.QIntVar(1 + j,0,6); // h_i_j
			h_0[j] = p.var(1+j);
			p.QIntVar(1+7+1 + j,0,1); // Ch_i_j
			Ch_0[j] = p.var(1+7+1+j);
			p.QIntVar(1+7+1+7+42+42+1+6 + j,0,1); // LinedV_i_j
			LinedV_0[j] = p.var(1+7+1+7+42+42+1+6+j);
		}
		
		IntVarArgs B_0(42);
		IntVarArgs C_0(42);
		for (int j=0;j<42;j++) {
			p.QIntVar(1+7+1+7 + j,0,2); // B_i_k
			B_0[j] = p.var(1+7+1+7+j);
			p.QIntVar(1+7+1+7+42 + j,0,1); // C_i_k
			C_0[j] = p.var(1+7+1+7+42+j);
		}
		
		// Constraints for first turn : 
		
		post(*(p.space()),Lined_0 == 0);
		element(*(p.space()),Ch_0,X_0,1,ICL_DOM);       // Ch_i[X_i] = 1
		linear(*(p.space()),Ch_0,IRT_EQ,1,ICL_DOM);     //  Sum(Ch_i[j]) over j  = 1
		element(*(p.space()),h_0,X_0,h_0x,ICL_DOM);     // h_i[X_i] = h_ix
		post(*(p.space()),k_0 == X_0 + 7*h_0x,ICL_DOM); // 
		element(*(p.space()),C_0,k_0,1,ICL_DOM);        // C_i[k_i] ! 1
		linear(*(p.space()),C_0,IRT_EQ,1,ICL_DOM);      // Sum(C_i[j]) over j  = 1
		for (int j=0;j<42;j++)
			post(*(p.space()),B_0[j] ==  C_0[j],ICL_DOM);
		for (int j=0;j<7;j++)
			post(*(p.space()),h_0[j] == Ch_0[j],ICL_DOM);
		
		for (int j=0;j<6;j++) {
			post(*(p.space()),LinedH_0[j] == 0);
			post(*(p.space()),LinedD1_0[j] == 0);
			post(*(p.space()),LinedD2_0[j] == 0);
		}
		
		for (int j=0;j<7;j++) 
			post(*(p.space()),LinedV_0[j] == 0);
		
		post(*(p.space()),Win_0 == 0);
		IntVarArgs firstBranch(1);
		firstBranch[0] = X_0;
		branch(*(p.space()),firstBranch,INT_VAR_SIZE_MIN,INT_VAL_MIN);
		branch(*(p.space()),p.space()->getIntVars(varByScope),INT_VAR_SIZE_MIN,INT_VAL_MIN);
		p.nextScope();
		
		
		
		
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OTHER TURNS //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i=1;i<nbScope;i++) {
      int player = ( (i%2 != 0)?2:1);
//      cout<<"scope "<<i<<" player "<<player<<endl;
      int offset = varByScope * i;
      p.QIntVar(offset + 0, 0, 6); // X_i
      p.QIntVar(offset + 1+7,0,6); // h_ix
      p.QIntVar(offset + 1+7+1+7+42+42 ,0,41); // k_i
      p.QIntVar(offset + 1+7+1+7+42+42+1+6+7+6+6 ,0,4); // Lined_i
      p.QIntVar(offset + 1+7+1+7+42+42+1+6+7+6+6+1 ,0,2); // Win_i
      IntVar X_i(p.var(offset+0));
      IntVar h_ix(p.var(offset+1+7));
      IntVar k_i(p.var(offset+1+7+1+7+42+42));
      IntVar Lined_i(p.var(offset+ 1+7+1+7+42+42+1+6+7+6+6));
      IntVar Win_i(p.var(offset+ 1+7+1+7+42+42+1+6+7+6+6+1));
			
      IntVarArgs LinedH_i(6);
      IntVarArgs LinedD1_i(6);
      IntVarArgs LinedD2_i(6);
			
      for (int j=0;j<6;j++) {
				p.QIntVar(offset + 1+7+1+7+42+42+1 +j , 0,1); // LinedH_i_j
				LinedH_i[j] = p.var(offset+ 1+7+1+7+42+42+1+j);
				p.QIntVar(offset + 1+7+1+7+42+42+1+6+7 +j , 0,1); // LinedD1_i_j
				LinedD1_i[j] = p.var(offset+ 1+7+1+7+42+42+1+6+7+j);
				p.QIntVar(offset + 1+7+1+7+42+42+1+6+7+6 +j , 0,1); // LinedD2_i_j
				LinedD2_i[j] = p.var(offset+ 1+7+1+7+42+42+1+6+7+6+j);
      }
			
      IntVarArgs h_i(7);
      IntVarArgs Ch_i(7);
      IntVarArgs LinedV_i(7);
      for (int j=0;j<7;j++) {
				p.QIntVar(offset + 1 + j,0,6); // h_i_j
				h_i[j] = p.var(offset+ 1+j);
				p.QIntVar(offset + 1+7+1 + j,0,1); // Ch_i_j
				Ch_i[j] = p.var(offset+ 1+7+1+j);
				p.QIntVar(offset + 1+7+1+7+42+42+1+6 + j,0,1); // LinedV_i_j
				LinedV_i[j] = p.var(offset+ 1+7+1+7+42+42+1+6+j);
      }
			
      IntVarArgs B_i(42);
      IntVarArgs C_i(42);
      for (int j=0;j<42;j++) {
				p.QIntVar(offset + 1+7+1+7 + j,0,2); // B_i_k
				B_i[j] = p.var(offset+ 1+7+1+7+j);
				p.QIntVar(offset + 1+7+1+7+42 + j,0,1); // C_i_k
				C_i[j] = p.var(offset+ 1+7+1+7+42+j);
      }
			
      IntVarArgs B_im1(42); // B_(i-1)_k
      IntVarArgs h_im1(7);  // B_(i-1)_j
      for (int j=0;j<42;j++)
				B_im1[j] = p.var((offset-varByScope) + 1+7+1+7+j);
      for (int j=0;j<7;j++)
				h_im1[j] = p.var((offset-varByScope) + 1+j);
      IntVar Lined_im1(p.var((offset-varByScope) + 1+7+1+7+42+42+1+6+7+6+6)); // Lined_(i-1)
			
			
      // CONSTRAINTS POSTING
			
      post(*(p.space()),Win_i == 0); // not used in this version
			
			
      element(*(p.space()),Ch_i,X_i,1,ICL_DOM);       // Ch_i[X_i] = 1
      linear(*(p.space()),Ch_i,IRT_EQ,1,ICL_DOM);     //  Sum(Ch_i[j]) over j  = 1
      element(*(p.space()),h_i,X_i,h_ix,ICL_DOM);     // h_i[X_i] = h_ix
      post(*(p.space()),k_i == X_i + 7*h_ix,ICL_DOM); // 
      element(*(p.space()),C_i,k_i,1,ICL_DOM);        // C_i[k_i] ! 1
      linear(*(p.space()),C_i,IRT_EQ,1,ICL_DOM);      // Sum(C_i[j]) over j  = 1
      for (int j=0;j<42;j++)
				post(*(p.space()),B_i[j] == B_im1[j] + player*C_i[j],ICL_DOM);
      for (int j=0;j<7;j++)
				post(*(p.space()),h_i[j] == h_im1[j] + Ch_i[j],ICL_DOM);
			
			
      /////////////////////////////////////
      // Horizontal lines detection...   //
      /////////////////////////////////////
      for (int j=0;j<6;j++)
				post(*(p.space()),tt(
														 eqv(
																 ~(LinedH_i[j] == 1),
																 (
																	~(B_i[7*j+3] != 0) &&
																	(
																	 (~(B_i[7*j+0] == B_i[7*j+1]) && ~(B_i[7*j+1] == B_i[7*j+2]) && ~(B_i[7*j+2] == B_i[7*j+3])) ||
																	 (~(B_i[7*j+1] == B_i[7*j+2]) && ~(B_i[7*j+2] == B_i[7*j+3]) && ~(B_i[7*j+3] == B_i[7*j+4])) ||
																	 (~(B_i[7*j+2] == B_i[7*j+3]) && ~(B_i[7*j+3] == B_i[7*j+4]) && ~(B_i[7*j+4] == B_i[7*j+5])) ||
																	 (~(B_i[7*j+3] == B_i[7*j+4]) && ~(B_i[7*j+4] == B_i[7*j+5]) && ~(B_i[7*j+5] == B_i[7*j+6])) 
																	 )
																	)
																 )
														 )
						 ,ICL_DOM);
			
      /////////////////////////////////////
      // Diagonal lines detection...     //
      /////////////////////////////////////
      // Topright-bottomleft diagonals
			
      post(*(p.space()),tt(eqv( ~(LinedD1_i[0] == 1),~(B_i[0 + 7*2] != 0) && 
															 ~(B_i[0+7*2] == B_i[1+7*3]) && ~(B_i[1+7*3] == B_i[2+7*4]) && ~(B_i[2+7*4] == B_i[3+7*5])
															 )),ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD1_i[1] == 1)
															 ,
															 (
																~(B_i[2 + 7*3] != 0) && 
																(
																 (~(B_i[0+7*1] == B_i[1+7*2]) && ~(B_i[1+7*2] == B_i[2+7*3]) && ~(B_i[2+7*3] == B_i[3+7*4])) ||
																 (~(B_i[1+7*2] == B_i[2+7*3]) && ~(B_i[2+7*3] == B_i[3+7*4]) && ~(B_i[3+7*4] == B_i[4+7*5]))
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD1_i[2] == 1)
															 ,
															 (
																~(B_i[3 + 7*3] != 0) && 
																(
																 (~(B_i[0+7*0] == B_i[1+7*1]) && ~(B_i[1+7*1] == B_i[2+7*2]) && ~(B_i[2+7*2] == B_i[3+7*3])) ||
																 (~(B_i[1+7*1] == B_i[2+7*2]) && ~(B_i[2+7*2] == B_i[3+7*3]) && ~(B_i[3+7*3] == B_i[4+7*4])) ||
																 (~(B_i[2+7*2] == B_i[3+7*3]) && ~(B_i[3+7*3] == B_i[4+7*4]) && ~(B_i[4+7*4] == B_i[5+7*5])) 
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD1_i[3] == 1)
															 ,
															 (
																~(B_i[3 + 7*2] != 0) && 
																(
																 (~(B_i[1+7*0] == B_i[2+7*1]) && ~(B_i[2+7*1] == B_i[3+7*2]) && ~(B_i[3+7*2] == B_i[4+7*3])) ||
																 (~(B_i[2+7*1] == B_i[3+7*2]) && ~(B_i[3+7*2] == B_i[4+7*3]) && ~(B_i[4+7*3] == B_i[5+7*4])) ||
																 (~(B_i[3+7*2] == B_i[4+7*3]) && ~(B_i[4+7*3] == B_i[5+7*4]) && ~(B_i[5+7*4] == B_i[6+7*5])) 
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD1_i[4] == 1)
															 ,
															 (
																~(B_i[5 + 7*3] != 0) && 
																(
																 (~(B_i[2+7*0] == B_i[3+7*1]) && ~(B_i[3+7*1] == B_i[4+7*2]) && ~(B_i[4+7*2] == B_i[5+7*3])) ||
																 (~(B_i[3+7*1] == B_i[4+7*2]) && ~(B_i[4+7*2] == B_i[5+7*3]) && ~(B_i[5+7*3] == B_i[6+7*4]))
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(eqv( ~(LinedD1_i[5] == 1),~(B_i[3 + 7*0] != 0) && 
															 ~(B_i[3+7*0] == B_i[4+7*1]) && ~(B_i[4+7*1] == B_i[5+7*2]) && ~(B_i[5+7*2] == B_i[6+7*3])
															 )),ICL_DOM);
			
      /////////////////////////////////
      // Bottomright-Topleft diagonals
			
      post(*(p.space()),tt(eqv( ~(LinedD2_i[0] == 1),~(B_i[3 + 7*0] != 0) && 
															 ~(B_i[3+7*0] == B_i[2+7*1]) && ~(B_i[2+7*1] == B_i[1+7*2]) && ~(B_i[1+7*2] == B_i[0+7*3])
															 )),ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD2_i[1] == 1)
															 ,
															 (
																~(B_i[2 + 7*2] != 0) && 
																(
																 (~(B_i[4+7*0] == B_i[3+7*1]) && ~(B_i[3+7*1] == B_i[2+7*2]) && ~(B_i[2+7*2] == B_i[1+7*3])) ||
																 (~(B_i[3+7*1] == B_i[2+7*2]) && ~(B_i[2+7*2] == B_i[1+7*3]) && ~(B_i[1+7*3] == B_i[0+7*4]))
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD2_i[2] == 1)
															 ,
															 (
																~(B_i[3 + 7*2] != 0) && 
																(
																 (~(B_i[5+7*0] == B_i[4+7*1]) && ~(B_i[4+7*1] == B_i[3+7*2]) && ~(B_i[3+7*2] == B_i[2+7*3])) ||
																 (~(B_i[4+7*1] == B_i[3+7*2]) && ~(B_i[3+7*2] == B_i[2+7*3]) && ~(B_i[2+7*3] == B_i[1+7*4])) ||
																 (~(B_i[3+7*2] == B_i[2+7*3]) && ~(B_i[2+7*3] == B_i[1+7*4]) && ~(B_i[1+7*4] == B_i[0+7*5])) 
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD2_i[3] == 1)
															 ,
															 (
																~(B_i[3 + 7*2] != 0) && 
																(
																 (~(B_i[6+7*0] == B_i[5+7*1]) && ~(B_i[5+7*1] == B_i[4+7*2]) && ~(B_i[4+7*2] == B_i[3+7*3])) ||
																 (~(B_i[5+7*1] == B_i[4+7*2]) && ~(B_i[4+7*2] == B_i[3+7*3]) && ~(B_i[3+7*3] == B_i[2+7*4])) ||
																 (~(B_i[4+7*2] == B_i[3+7*3]) && ~(B_i[3+7*3] == B_i[2+7*4]) && ~(B_i[2+7*4] == B_i[1+7*5])) 
																 )
																)
															 )
													 )
					 ,ICL_DOM );
			
      post(*(p.space()),tt(
													 eqv(
															 ~(LinedD2_i[4] == 1)
															 ,
															 (
																~(B_i[5 + 7*3] != 0) && 
																(
																 (~(B_i[6+7*1] == B_i[5+7*2]) && ~(B_i[5+7*2] == B_i[4+7*3]) && ~(B_i[4+7*3] == B_i[3+7*4])) ||
																 (~(B_i[5+7*2] == B_i[4+7*3]) && ~(B_i[4+7*3] == B_i[3+7*4]) && ~(B_i[3+7*4] == B_i[2+7*5]))
																 )
																)
															 )
													 )
					 ,ICL_DOM);
			
      post(*(p.space()),tt(eqv( ~(LinedD2_i[5] == 1),~(B_i[6 + 7*2] != 0) && 
															 ~(B_i[6+7*2] == B_i[5+7*3]) && ~(B_i[5+7*3] == B_i[4+7*4]) && ~(B_i[4+7*4] == B_i[3+7*5])
															 )),ICL_DOM);
			
			
			
      /////////////////////////////////
      // Vertical lines detection... //
      /////////////////////////////////
			
      for (int j=0;j<7;j++)
				post(*(p.space()),tt(
														 eqv(
																 ~(LinedV_i[j] == 1),
																 ~(B_i[3*7+j] != 0) &&
																 (
																	(~(B_i[7*0+j] == B_i[7*1+j]) && ~(B_i[7*1+j] == B_i[7*2+j]) && ~(B_i[7*2+j] == B_i[7*3+j])) ||
																	(~(B_i[7*1+j] == B_i[7*2+j]) && ~(B_i[7*2+j] == B_i[7*3+j]) && ~(B_i[7*3+j] == B_i[7*4+j])) ||
																	(~(B_i[7*2+j] == B_i[7*3+j]) && ~(B_i[7*3+j] == B_i[7*4+j]) && ~(B_i[7*4+j] == B_i[7*5+j])) 
																	)
																 )
														 )
						 ,ICL_DOM);
			
      post(*(p.space()), Lined_i == LinedH_i[0] + LinedH_i[1] + LinedH_i[2] + LinedH_i[3] + LinedH_i[4] + LinedH_i[5] + 
					 LinedV_i[0] + LinedV_i[1] + LinedV_i[2] + LinedV_i[3] + LinedV_i[4] + LinedV_i[5] + LinedV_i[6] +
					 LinedD1_i[0] + LinedD1_i[1] + LinedD1_i[2] + LinedD1_i[3] + LinedD1_i[4] + LinedD1_i[5] + 
					 LinedD2_i[0] + LinedD2_i[1] + LinedD2_i[2] + LinedD2_i[3] + LinedD2_i[4] + LinedD2_i[5]
					 ,ICL_DOM);
			
			
      // Winning condition : If the previous player had lined 4 pawns, current player loses and thus, can not move.
			post(*(p.space()), Lined_im1 == 0);
			

			// posts the brancher selecting a value for the decision variable X_i
			MyB::post(*(p.space()),B_im1,X_i,player); 
			// posts the brancher for the other variables. Though in the ideal case, all other variables re set by propagation after X_I have been assigned. 
      branch(*(p.space()),p.space()->getIntVars(varByScope*(i+1)),INT_VAR_SIZE_MIN,INT_VAL_MIN); 
			
			
			p.nextScope();
    }    
		
		
    p.makeStructure();
		
    QSolver solver(&p);    
    unsigned long int nodes=0;
    
    // then we solve the problem. Nodes and Steps will contain the number of nodes encountered and
    // of propagation steps achieved during the solving.
    Strategy outcome = solver.solve(nodes);
		finish=clock();
    cout<<(!outcome.isFalse()?"true":"false")<<" - Went through "<<nodes<<" nodes, taking "<<(finish-start)<<" microseconds."<<endl;
		
	}
	return 0;
}
