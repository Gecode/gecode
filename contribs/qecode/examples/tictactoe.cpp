/****   , [ tictactoe.cpp ], 
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


#include <iostream>

#include "gecode/minimodel.hh"
#include "gecode/int/element.hh"
#include "QCOPPlus.hh"
#include "qsolver_general.hh"
#include <time.h>
#define UNIVERSAL true
#define EXISTENTIAL false

//////////////////////////////////////////////////////////////////////////////
// This examples models the standard game of Tic-Tac-Toe on a 3*3 board.    //
// For each turn, the board is represented by nine Bi variables. Lined_i    //
// represents wether a preceding move have been winning or not
//////////////////////////////////////////////////////////////////////////////

using namespace MiniModel;

// Recursive function for strategy output. This version produces a graphviz script...
void printStr(Strategy s,int depth) {
  cout<<"A"<<s.id()<<" [ label=\"";
    StrategyNode plop = s.getTag();
    if (s.isTrue()) cout<<"TRUE";
    else if (s.isFalse()) cout<<"FALSE";
    else cout<<plop.scope<<( plop.quantifier ? "A" : "E" )<<"- "<<s.getTag().valeurs[0];
    //    for (int i=0;i<s.getTag().valeurs.size();i++) cout<<s.getTag().valeurs[i]<<" ";
    cout<<"\"];"<<endl;

    for (int i=0;i<s.degree();i++) {
      cout<<"A"<<s.id()<<" -> A"<<s.getChild(i).id()<<";"<<endl;
      printStr(s.getChild(i),depth+1);
    }
}



int main (int argc, char * const argv[]) {
  clock_t start, finish;
    int nbScope = 9;
    bool* qtScopes = new bool[nbScope];
    for (int i=0;i<nbScope;i++) { 
        qtScopes[i] = ((i%2) != 0);
        cout << (((i%2) != 0)?"true":"false")<<endl;
    }
    
    int *scopesSize = new int[nbScope];
    for (int i=0; i<nbScope; i++)
        scopesSize[i]=21;

    
    Qcop p(nbScope, qtScopes, scopesSize);
    

    p.QIntVar(0*21,0,8); // x_0
    IntVar X0(p.var(0*21));
    IntVarArgs B0(9);
    IntVarArgs C0(9);
    for(int j=0;j<9;j++) {
      p.QIntVar(0*21 + 1 + j,0,2); // b_0_j
      p.QIntVar(0*21 + 10 + j,0,1); // c_0_j
      B0[j] = p.var(1+j);
      C0[j] = p.var(10+j);
    }
    p.QIntVar(0*21 + 19 ,0,1); // Lined_0
    IntVar Lined0(p.var(19));
    p.QIntVar(0*21 + 20 ,0,0); // Win_0
    IntVar Win0(p.var(20));

    post(*(p.space()),Lined0 == 0);

    element(*(p.space()),C0,X0,1,ICL_DOM);
    post(*(p.space()), C0[0] + C0[1] + C0[2] + C0[3] + C0[4] + C0[5] + C0[6] + C0[7] + C0[8] == 1); 
    for (int j=0;j<9;j++) 
      post(*(p.space()),B0[j] == C0[j]);

    IntVarArgs b0 = p.space()->getIntVars(21);
    branch(*(p.space()),b0,INT_VAR_SIZE_MAX,INT_VAL_MIN);
    p.nextScope();

    for (int i=1; i<9; i++)
    {
      p.QIntVar(i*21,0,8);
      IntVar Xi(p.var(i*21));
      IntVarArgs Bi(9);
      IntVarArgs Ci(9);
      IntVarArgs Bim1(9);
      IntVarArgs Cim1(9);
      for(int j=0;j<9;j++) {
	p.QIntVar(i*21 + 1 + j,0,2); // b_i_j
	p.QIntVar(i*21 + 10 + j,0,1); // c_i_j
	Bi[j] = p.var(i*21 + 1+j);
	Ci[j] = p.var(i*21 + 10+j);
	Bim1[j] = p.var((i-1)*21 + 1+j);
	Cim1[j] = p.var((i-1)*21 + 10+j);
      }

      p.QIntVar(i*21 + 19 ,0,1); // Lined_0
      IntVar Lined_i(p.var(i*21+19));
      IntVar Lined_imi(p.var((i-1)*21+19));
      p.QIntVar(i*21 + 20 ,0,0); // Win_0 - unused

      IntVarArgs X(i+1);
      for (int j=0;j<=i;j++) {
	X[j] = p.var(j*21);
      }
      distinct(*(p.space()),X,ICL_DOM);

      for (int j=0;j<9;j++) {
	int toadd = 1 + (i % 2);
	post(*(p.space()),Bi[j] == Bim1[j] + toadd*Ci[j]);
      }

      element(*(p.space()),Ci,Xi,1);
      post(*(p.space()), Ci[0] + Ci[1] + Ci[2] + Ci[3] + Ci[4] + Ci[5] + Ci[6] + Ci[7] + Ci[8] == 1,ICL_DOM);

      post(*(p.space()),tt(
                          eqv(
                              ~(Lined_i == 1),
                              (
                               (~(Bi[0] == Bi[1]) && ~(Bi[1] == Bi[2]) && ~(Bi[1] != 0)) ||
                               (~(Bi[3] == Bi[4]) && ~(Bi[4] == Bi[5]) && ~(Bi[4] != 0)) ||
                               (~(Bi[6] == Bi[7]) && ~(Bi[7] == Bi[8]) && ~(Bi[7] != 0)) ||
                               
                               (~(Bi[0] == Bi[3]) && ~(Bi[3] == Bi[6]) && ~(Bi[0] != 0)) ||
                               (~(Bi[1] == Bi[4]) && ~(Bi[4] == Bi[7]) && ~(Bi[1] != 0)) ||
                               (~(Bi[2] == Bi[5]) && ~(Bi[8] == Bi[8]) && ~(Bi[2] != 0)) ||
                               
                               (~(Bi[0] == Bi[4]) && ~(Bi[4] == Bi[8]) && ~(Bi[0] != 0)) ||
                               (~(Bi[2] == Bi[4]) && ~(Bi[4] == Bi[6]) && ~(Bi[2] != 0))
                              )
                             )
                          )
	   );

      int toadd = 1 + (i % 2);
	
      post(*(p.space()),Lined_imi == 0);


      IntVarArgs b = p.space()->getIntVars(21*(i+1));
      branch(*(p.space()),b,INT_VAR_SIZE_MIN,INT_VAL_MIN);
      p.nextScope();
    }
	

    /* Goal */

    IntVarArgs b3 = p.space()->getIntVars(9+81+81+9+9);
    branch(*(p.space()),b3,INT_VAR_SIZE_MIN,INT_VAL_MIN);
    
    p.makeStructure();

    {
      start=clock();
      cout<<"General QCSP solver : "<<endl;

    QSolver solver(&p);
    
    unsigned long int nodes=0;
    
    Strategy outcome = solver.solve(nodes);
    
    cout << "  outcome: " << ( outcome.isFalse()? "FALSE" : "TRUE") << endl;
    cout << "  nodes visited: " << nodes << endl<<endl;
    finish=clock();
    cout<<"Time : "<<(finish-start)<<endl;
    }

    // Uncomment the following to have graphviz output
    //    cout<<" graph G {"<<endl;
    //    printStr(outcome,0);
    //    cout<<"}"<<endl;

    return 0;
}
