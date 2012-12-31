/****   , [ MatrixGame.cpp ],
Copyright (c) 2007 Universite d'Orleans - Arnaud Lallouet

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

#include <cstdlib> /* for srand, rand et RAND_MAX */
#include <ctime> /* for time */
#include <math.h> /* for pow */

#include <iostream>

#include <gecode/minimodel.hh>
#include <gecode/int/element.hh>

#include "qsolver_qcsp.hh"
#include "QCOPPlus.hh"

#define UNIVERSAL true
#define EXISTENTIAL false

// The Matrix game consists in a square boolean matrix of size 2^depth. First player cuts it vertically in two parts and removes one half,
// while secodn player do the same, but cutting the matrix horizontally. The game ends when there are only one cell left in the matrix.
// If this last cell has value 1, the first player wins. If it has value 0, the second player wins.

// The present model of this game is pure QBF, that QeCode can handle (though not as fast as QBF solvers...)

using namespace MiniModel;

int main (int argc, char * const argv[]) {

    int depth = 5;  // Size of the matrix is 2^depth. Larger values may take long to solve...
    int nbDecisionVar = 2*depth;
    int nbScope = nbDecisionVar+1;
    bool* qtScopes = new bool[nbScope];
    for (int i=0;i<nbScope;i++) {
        qtScopes[i] = ((i%2) != 0);
//        cout << (((i%2) != 0)?"true":"false")<<endl;
    }
    int boardSize = (int)pow((double)2,(double)depth);

    std::srand(std::time(NULL));

    IntArgs board(boardSize*boardSize);
    for (int i=0; i<boardSize; i++)
        for (int j=0; j<boardSize; j++)
            board[j*boardSize+i] = (int)( (double)rand()  /  ((double)RAND_MAX + 1) * 50 ) < 25 ? 0:1;

    IntArgs access(nbDecisionVar);
    access[nbDecisionVar-1]=1;
    access[nbDecisionVar-2]=boardSize;
    for (int i=nbDecisionVar-3; i>=0; i--)
        access[i]=access[i+2]*2;
	
    // debug
    for (int i=0; i<boardSize; i++)
    {
        for (int j=0; j<boardSize; j++)
            cout << board[j*boardSize+i] << " ";
        cout << endl;
	}
    cout  << endl;
    //    for (int i=0; i<nbDecisionVar; i++)
//        cout << access[i] << " ";
//    cout << endl;
    // end debug

    int * scopesSize = new int[nbScope];
    for (int i=0; i<nbScope-1; i++)
        scopesSize[i]=1;
    scopesSize[nbScope-1]=2;

    Qcop p(nbScope, qtScopes, scopesSize);

    // Defining the variable of the n first scopes ...
    for (int i=0; i<nbDecisionVar; i++)
    {
        p.QIntVar(i, 0, 1);
        IntVarArgs b(i+1);
        for (int plop=0;plop<(i+1);plop++)
            b[plop] = p.var(plop);
        branch(*(p.space()),b,INT_VAR_SIZE_MIN(),INT_VAL_MIN());
        p.nextScope();
    }
	
    // Declaring last scope variables ...

    p.QIntVar(nbDecisionVar, 0, 1);
    p.QIntVar(nbDecisionVar+1, 0, boardSize*boardSize);
    IntVarArgs b(nbDecisionVar+2);
    for (int plop=0;plop<(nbDecisionVar+2);plop++)
        b[plop] = p.var(plop);
    branch(*(p.space()),b,INT_VAR_SIZE_MIN(),INT_VAL_MIN());

    p.nextScope();
    // Body

    rel(*(p.space()), p.var(nbDecisionVar) == 1);

    IntVarArgs X(nbDecisionVar);
    for (int i=0; i<nbDecisionVar; i++)
        X[i]=p.var(i);

    linear(*(p.space()), access, X, IRT_EQ, p.var(nbDecisionVar+1));
    //  MiniModel::LinRel R(E, IRT_EQ, MiniModel::LinExpr(p.var(nbDecisionVar+1)));
    element(*(p.space()), board, p.var(nbDecisionVar+1), p.var(nbDecisionVar));

	  // Note : declaring a brancher for the goal is not mandatory, as the goal will be tested only when all variables are assigned.
	 
    // When every variables and constraints have been declared, the makeStructure method
    // must be called in order to lead the problem ready for solving.
    p.makeStructure();

    // So, we build a quantified solver for our problem p, using the heuristic we just created.
    QCSP_Solver solver(&p);

    unsigned long int nodes=0;

    // then we solve the problem. Nodes and Steps will contain the number of nodes encountered and
    // of propagation steps achieved during the solving.
    Strategy outcome = solver.solve(nodes);

    cout << "  outcome: " << ( outcome.isFalse()? "FALSE" : "TRUE") << endl;
    cout << "  nodes visited: " << nodes << endl;

}
