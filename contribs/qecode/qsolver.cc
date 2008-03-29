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

void QSolver::attachLeafObserver(LeafObserver* o) {
    obs_leaf.push_back(o);
}

void QSolver::attachBTObserver(BacktrackObserver* o) {
    obs_bt.push_back(o);
}

void QSolver::attachCPObserver(ChoicePointObserver* o) {
    obs_cp.push_back(o);
}

void QSolver::attachSubsumedObserver(SubsumedObserver* o) {
    obs_sub.push_back(o);
}

bool QSolver::solve(unsigned long int& nodes, unsigned long int& propsteps) {
    int ret = sp->status(0,propsteps);
    return rSolve(sp,nodes,propsteps,0,0);
}


bool QSolver::rSolve(QSpace* qs, unsigned long int& nodes, unsigned long int& propsteps, int curvar, int depth) {
    unsigned int rien=0;
    nodes++;
    ImplicativeState st(static_cast<Implicative*>(qs));   
        
    // Step 0 : we ask to the branching heuristic /////////////////////////////////
    // for the next variable to branch on. ////////////////////////////////////////
    int branchon = bh->nextVar(qs);                                              //
    ///////////////////////////////////////////////////////////////////////////////
    
    // Step 1 : if there is no more variables to branch on, we ask for the ////////
    // final status of the QCSP, and return its result. ///////////////////////////
    if (branchon == n) {                                                         //
        int ret=qs->status(curvar,propsteps);
        int fret=qs->finalStatus(propsteps);                                     //
        
//   LEAF OBSERVER (fret==1)
        {
            for ( vector<LeafObserver*>::reverse_iterator i = obs_leaf.rbegin();
                  i != obs_leaf.rend();
                  ++i )
            {
                (*i)->observeLeaf(st,fret==1);
            }
        }
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
        for ( vector<BacktrackObserver*>::reverse_iterator i = obs_bt.rbegin();
            i != obs_bt.rend();
            ++i )
        {
            (*i)->observeBacktrack(st,depth);
        }

//   BACKTRACK OBSERVER
        return (fret==1)?true:false;                                             //
    }                                                                            //
    ///////////////////////////////////////////////////////////////////////////////
    
    // Step 2 : if we must branch on a variable, first we check the status of  ////
    // the problem (it could be given at this moment) /////////////////////////////
    int ret = qs->status(curvar,propsteps);                                      //

    if (ret==1) { // If the answer is TRUE...                                    //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //

//   LEAF OBSERVER (true)
        for ( vector<LeafObserver*>::reverse_iterator i = obs_leaf.rbegin();
            i != obs_leaf.rend();
            ++i )
        {
            (*i)->observeLeaf(st,true);
        }

   //   BACKTRACK OBSERVER ( depth)
        for ( vector<BacktrackObserver*>::reverse_iterator i = obs_bt.rbegin();
            i != obs_bt.rend();
            ++i )
        {
            (*i)->observeBacktrack(st,depth);
        }
        return true;                                                             //
    }                                                                            //
                                                                                 //
    if (ret==0) { // If the answer is FALSE...                                   //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //

//   LEAF OBSERVER (false)
        for ( vector<LeafObserver*>::reverse_iterator i = obs_leaf.rbegin();
            i != obs_leaf.rend();
            ++i )
        {
            (*i)->observeLeaf(st,false);
        }

    //   BACKTRACK OBSERVER ( depth)
        for ( vector<BacktrackObserver*>::reverse_iterator i = obs_bt.rbegin();
            i != obs_bt.rend();
            ++i )
        {
            (*i)->observeBacktrack(st,depth);
        }
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
// CHOICE POINT OBSERVER ( depth, curvar, branchon, true, tab, *nbRanges)
       for ( vector<ChoicePointObserver*>::reverse_iterator i = obs_cp.rbegin();
            i != obs_cp.rend();
            ++i )
        {
            (*i)->observeChoicePoint(st,depth, curvar, branchon, true, tab, *nbRanges);
        }

    if ( rSolve(subspace,nodes, propsteps,branchon,depth+1) ^ qs->quantification(branchon)) {
        delete subspace;                                                         //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //

        delete [] tab[0];
        delete [] tab[1];
        delete [] tab;
// BACKTRACK OBSERVER (depth)
        for ( vector<BacktrackObserver*>::reverse_iterator i = obs_bt.rbegin();
            i != obs_bt.rend();
            ++i )
        {
            (*i)->observeBacktrack(st,depth);
        }
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

    // CHOICE POINT OBSERVER ( depth, curvar, branchon, false, tab, *nbRanges)
       for ( vector<ChoicePointObserver*>::reverse_iterator i = obs_cp.rbegin();
            i != obs_cp.rend();
            ++i )
        {
            (*i)->observeChoicePoint(st,depth, curvar, branchon, false, tab, *nbRanges);
        }

    if ( rSolve(subspace,nodes, propsteps, branchon,depth+1) ^ qs->quantification(branchon)) {
        delete subspace;                                                         //
        qs->backtrack();                                                         //
        bh->backtrack(qs);                                                       //
        delete [] tab[0];
        delete [] tab[1];
        delete [] tab;
// BACKTRACK OBSERVER (depth)
        for ( vector<BacktrackObserver*>::reverse_iterator i = obs_bt.rbegin();
            i != obs_bt.rend();
            ++i )
        {
            (*i)->observeBacktrack(st,depth);
        }
        return (!qs->quantification(branchon));                                  //
    }                                                                            //
    delete subspace;                                                             //
    qs->backtrack();                                                             //
    bh->backtrack(qs);                                                           //
    delete [] tab[0];
    delete [] tab[1];
    delete [] tab;
    return (qs->quantification(branchon));                                       //
  /////////////////////////////////////////////////////////////////////////////////
}

