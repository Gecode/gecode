/****   , [ qsolver.hh ], 
Copyright (c) 2008 Universite d'Orleans - Jeremie Vautard 

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
#ifndef __QECODE_QSOLVER_OPT__
#define __QECODE_QSOLVER_OPT__

#include "QCOPPlus.hh"
#include <iostream>
#include <cstdlib>
#include "gecode/minimodel.hh"
#include "gecode/search.hh"
#include "Strategy.hh"
#include "qecode.hh"

using namespace Gecode;
/** General QCSP+ / QCOP+ Solver.
  * This class is the search engine for Qcop objects. 
*/
class QECODE_VTABLE_EXPORT QCOP_solver {
    
private:
    int n;
    Qcop* sp;
    int* nbRanges;
    Strategy rSolve(Qcop* qs,int scope,vector<int> assignments,unsigned long int& nodes);
public:
        /** Public constructor.
        @param sp The problem to solve
        */
    QECODE_EXPORT QCOP_solver(Qcop* sp); 
    
    /** Solves the problem and returns a corresponding winning strategy. 
        @param nodes A reference that is increased by the number of nodes encountered in the search tree.
        */
    QECODE_EXPORT Strategy solve(unsigned long int& nodes);
};

#endif
