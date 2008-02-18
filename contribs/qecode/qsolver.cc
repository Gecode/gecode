/*****************************************************************[qsolver.cc]
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

#include "qsolver.hh"
#include "implicative.hh"


QSolver::QSolver(QSpace* qs, VariableHeuristic* ev, valueHeuristic* ve) {
    debug=false;
    eval=ev;
    valEval = ve;
    sp=qs;
    n=sp->nv();
    bh=new BranchingHeuristic(sp,eval);
    sp->indicateBranchingHeuristic(bh);
    nbRanges = new int;
}


bool QSolver::solve(unsigned long int& nodes, unsigned long int& propsteps) {
    int ret = sp->status(0,propsteps);
    return rSolve(sp,nodes,propsteps,0);
}


bool QSolver::rSolve(QSpace* qs, unsigned long int& nodes, unsigned long int& propsteps, int curvar) {
    unsigned int rien=0;
    nodes++;
    
//    if (debug) cout<<"Solver : Rsolve appelé..."<<endl;
    
    // Step 0 : we ask to the branching heuristic /////////////////////////////////
    // for the next variable to branch on. ////////////////////////////////////////
    int branchon = bh->nextVar(qs);                                              //
    ///////////////////////////////////////////////////////////////////////////////
    
    // Step 1 : if there is no more variables to branch on, we ask for the ////////
    // final status of the QCSP, and return its result. ///////////////////////////
    if (branchon == n) {                                                         //
        int ret=qs->status(curvar,propsteps);
        int fret=qs->finalStatus(propsteps);                                     //
        
#ifdef QDEBUG        
        cout<<"Solver : finalstatus appelé, résultat = "<<fret<<endl;
        qs->print();
#endif
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
#ifdef QDEBUG        
        cout<<"Solver : backtracking"<<endl;                          //
#endif
        return (fret==1)?true:false;                                             //
    }                                                                            //
    ///////////////////////////////////////////////////////////////////////////////
    
    // Step 2 : if we must branch on a variable, first we check the status of  ////
    // the problem (it could be given at this moment) /////////////////////////////
    int ret = qs->status(curvar,propsteps);                                      //
#ifdef QDEBUG        
    cout<<"Solver : status appelé, résultat = "<<ret<<endl;
    qs->print();
#endif
    if (ret==1) { // If the answer is TRUE...                                    //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
#ifdef QDEBUG        
        cout<<"Solver : backtracking"<<endl;
#endif
        return true;                                                             //
    }                                                                            //
                                                                                 //
    if (ret==0) { // If the answer is FALSE...                                   //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
#ifdef QDEBUG        
        cout<<"Solver : backtracking"<<endl;
#endif
        return false;                                                            //
    }                                                                            //
    ///////////////////////////////////////////////////////////////////////////////
    
    
    // Step 3 : the answer is "Don't know yet". We branch on the given variable. //
    int** tab = valEval->subSet(static_cast<Implicative*>(qs),branchon,nbRanges);//
    QSpace* subspace;                                                            //
    subspace=qs->clone();                                                        //
    switch (qs->type_of_v[branchon]) {                                           //
        case VTYPE_INT :                                                         //
            subspace->assign_int(branchon,tab,*nbRanges);                        //
            break;                                                               //
        case VTYPE_BOOL :                                                        //
            subspace->assign_bool(branchon,tab,*nbRanges);                       //
            break;
    }
#ifdef QDEBUG        
    cout<<"Solver : first branch : v"<<branchon<<" = "<<the_set<<endl;
#endif
    if ( rSolve(subspace,nodes, propsteps,branchon) ^ qs->quantification(branchon)) {
        delete subspace;                                                         //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
#ifdef QDEBUG        
        cout<<"Solver : Backtrack"<<endl;
#endif
        delete [] tab[0];
        delete [] tab[1];
        delete [] tab;
        return (!qs->quantification(branchon));                                  //
    }                                                                            //
    delete subspace;                                                             //
    subspace=qs->clone();                                                        //
    switch (subspace->type_of_v[branchon]) {
        case VTYPE_INT : 
            subspace->remove_int(branchon,tab,*nbRanges);
            break;
        case VTYPE_BOOL : 
            subspace->remove_bool(branchon,tab,*nbRanges);
            break;
    }

#ifdef QDEBUG        
    cout<<"Solver : second branch : v"<<branchon<<" != "<<the_set<<endl;
#endif
    if ( rSolve(subspace,nodes, propsteps, branchon) ^ qs->quantification(branchon)) {
        delete subspace;                                                         //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
#ifdef QDEBUG        
        cout<<"Solver : Backtrack"<<endl;
#endif
        delete [] tab[0];
        delete [] tab[1];
        delete [] tab;
        return (!qs->quantification(branchon));                                  //
    }                                                                            //
    delete subspace;                                                             //
    qs->backtrack();                                                             //
    bh->backtrack(qs);                                                           //
#ifdef QDEBUG        
    cout<<"Solver : Backtrack"<<endl;
#endif
    delete [] tab[0];
    delete [] tab[1];
    delete [] tab;
    return (qs->quantification(branchon));                                       //
  /////////////////////////////////////////////////////////////////////////////////
}

