/****   , [ NimFibo.cpp ], 
Copyright (c) 2007 Universite d'Orleans - Jeremie Vautard 

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

#include "qsolver.hh"
#include "implicative.hh"
#include "SDFVariableHeuristic.hh"
#include "NaiveValueHeuristics.hh"
#include "FirstFailValueHeuristic.hh"

///////////////////////////////////////////////////////////////////////////////////////////
// This is a model of the nim-fibonacci game. We have a N matches set. First player may  //
// take between 1 and N-1 matches. Then, each player may take at most twice the number of//
// matches taken by the last player. Take the last match to win !                        //
///////////////////////////////////////////////////////////////////////////////////////////

int main() {
    int N = 20; // Initial number of matches
    
    int* scopeSize = new int[N+2];
    for (int i=0;i<N+2;i++) 
        scopeSize[i] = 2;
    
    Implicative p(N+2,QECODE_EXISTENTIAL,scopeSize);
    
    p.QIntVar(0,1,N-1);
    p.QIntVar(1,0,N);
    post(p.space(),p.var(0) == p.var(1));
    p.nextScope();
    
    for (int i=1;i<N+2;i++) {
        p.QIntVar(2*i , 1, N-1);
        p.QIntVar(2*i + 1, 0, N);
        post(p.space(), p.var(2*i) <= 2*p.var(2*(i-1)));
        post(p.space(), p.var(2*i + 1) == p.var(2*i) + p.var(2*i - 1));
        p.nextScope();
    }
    
    p.makeStructure();
    
    SmallestDomainFirst heur;
//    SmallestValueFirst v_heur;
    FirstFailValue v_heur;
    QSolver s(&p,&heur,&v_heur);
    
    unsigned long int nodes=0;
    unsigned long int propsteps=0;
    
    bool outcome  = s.solve(nodes,propsteps);
    
    cout << "  outcome: " << ( outcome ? "TRUE" : "FALSE") << endl;
    cout << "  nodes visited: " << nodes << " " << propsteps << endl;
    
    return outcome ? 10:20;
}
