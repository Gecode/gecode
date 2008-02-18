/*****************************************************************[qsolver.hh]
Copyright (c) 2007, Universite d'Orleans - Jeremie Vautard, Marco Benedetti,
Arnaud Lallouet.

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
*****************************************************************************/

#ifndef __QECODE_QSOLVER__
#define __QECODE_QSOLVER__

#include "qecode.hh"
#include <iostream>
#include <cstdlib>
#include "gecode/minimodel.hh"

#include "qecore.hh"
#include "valueHeuristic.hh"

using namespace Gecode;
using namespace std;
using namespace Gecode::Int;


/** \brief Depth-first search engine for quantified spaces.
* A quite naive depth-first search engine for quantified spaces. The answer of such an engine is true or false. The complete winning strategy, if existing, is not given. 
*/
class QECODE_VTABLE_EXPORT QSolver {
    
private : 
    bool debug;
    int n;
    QSpace* sp;
    int* nbRanges;
    BranchingHeuristic* bh;
    VariableHeuristic* eval;
    valueHeuristic* valEval;
    bool rSolve(QSpace* qs,unsigned long int& nodes, unsigned long int& propsteps, int curvar);
public :
        
        /** \brief  Constructor for a quantified space solver.
        * Builds, providing a quantified space and a score for this space, a solver for this space.
        *  @param sp The quantified space to be solved.
        *  @param ev The score attribuer ("evaluator") that will be used by the branching heuristic.
        */
        QECODE_EXPORT QSolver(QSpace* sp, VariableHeuristic* ev,valueHeuristic* ve);
    
    
    /** \brief  Solves the quantified space.
        * Solves the quantifies dpace, returning its solution. The parameters are used to provide statistics.
        *  @param nodes The integer referenced will be increased by the number of nodes of the search tree explored.
        *  @param propsteps The integer references will be increased by the total number of propagation steps (in the Gecode sense) performed during search.
        */
    QECODE_EXPORT bool solve(unsigned long int& nodes, unsigned long int& propsteps);
};


#endif
