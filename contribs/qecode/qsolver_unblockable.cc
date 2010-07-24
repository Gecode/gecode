/****   , [ QSolverUnblockable.cc ], 
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

#include "qsolver_unblockable.hh"
#include <climits>

QSolverUnblockable::QSolverUnblockable(QcspUnblockable* sp) {
    this->sp = sp;
    nbRanges=new int;
}

Strategy QSolverUnblockable::solve(unsigned long int& nodes) {
    vector<int> plop;
    plop.clear();
    return rSolve(sp,0,plop,nodes);
}

Strategy QSolverUnblockable::rSolve(QcspUnblockable* qs,int scope, vector<int> assignments, unsigned long int& nodes) {
    nodes++;
    //cout<<"rSolve for scope "<<scope<<" with assignments ";
    //    for (int i=0;i<assignments.size();i++) cout<<assignments[i]<<" ";
    //    cout<<endl;
    //////////////////////////////////////////////////////////////////////////////////////////
    // First case : Unblockable QCSP+ : Goal checking.                                      //
    //  If the goal is failed, even before the end on f the search, we fail.                //
    //////////////////////////////////////////////////////////////////////////////////////////
    MySpace* g = qs->getGoal();
    if (g == NULL) {return Strategy(StrategyNode::SFalse());}
    for (int i=0;i<assignments.size();i++) {
        switch (g->type_of_v[i]) {
            case VTYPE_INT : 
                rel(*g,*(static_cast<IntVar*>(g->v[i])) == assignments[i]);
                break;
            case VTYPE_BOOL :
                rel(*g,*(static_cast<BoolVar*>(g->v[i])) == assignments[i]);
                break;
            default :
                cout<<"1Unknown variable type"<<endl;
                abort();
        }
    }
    if (g->status() == SS_FAILED) {
        delete g;
        return Strategy(StrategyNode::SFalse());
    }
    delete g;
    
    if (scope == qs->spaces()) {
        return Strategy(StrategyNode::STrue());
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    // Second case : we are in the middle of the problem...                                //
    /////////////////////////////////////////////////////////////////////////////////////////
    else {
        MySpace* espace = qs->getSpace(scope);
        if (espace == NULL) cout<<"I caught a NULL for scope "<<scope<<". I will crash..."<<endl;
        for (int i=0;i<assignments.size();i++) {
            switch (espace->type_of_v[i]) {
                case VTYPE_INT : 
                    rel(*espace,*(static_cast<IntVar*>(espace->v[i])) == assignments[i]);
                    break;
                case VTYPE_BOOL :
                    rel(*espace,*(static_cast<BoolVar*>(espace->v[i])) == assignments[i]);
                    break;
                default :
                    cout<<"2Unknown variable type"<<endl;
                    abort();
            }
        }
        
        // Second case, first subcase : current scope is universal
        /////////////////////////////////////////////////////////
        if (qs->quantification(scope)) {
            if (espace->status() == SS_FAILED) {
                delete espace;
                return Strategy(StrategyNode::STrue());
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol = solutions.next();
            if (sol == NULL) {
                delete espace;
                return Strategy(StrategyNode::STrue());
            }
            
            Strategy retour = StrategyNode::Dummy();
            while (sol != NULL) {
                vector<int> assign;
                for (int i = 0; i<sp->nbVarInScope(scope);i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            assign.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            assign.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"3Unknown variable type"<<endl;
                            abort();
                    }
                }
                
                int vmin = ( (scope==0)? 0 : (qs->nbVarInScope(scope-1)) );
                int vmax = (qs->nbVarInScope(scope))-1;
                vector<int> zevalues;
                for (int i = vmin; i<=vmax;i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            zevalues.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            zevalues.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"4Unknown variable type"<<endl;
                            abort();
                    }
                }
                Strategy toAttach(true,vmin,vmax,scope,zevalues);
                
                Strategy son = rSolve(qs,scope+1,assign,nodes);
                if (son.isFalse()) {
                    delete sol;
                    delete espace;
                    return Strategy(StrategyNode::SFalse());
                }
                toAttach.attach(son);
                retour.attach(toAttach);
                delete sol;
                sol = solutions.next();
            } // end of while
            delete espace;
            return retour;
        } // end of if(universal)
        
        // Second case, second subcase : current scope is existential
        ////////////////////////////////////////////////////////////
        else {
            if ((espace->status()) == SS_FAILED) {
                delete espace;
                return Strategy(StrategyNode::SFalse());
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol =solutions.next();
            if (sol == NULL) {
                delete espace;
                return Strategy(StrategyNode::SFalse());
            }
            while (sol != NULL) {
                vector<int> assign;
                for (int i = 0; i<sp->nbVarInScope(scope);i++) {
		  //                    cout << "i = "<<i<<endl;
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            assign.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            assign.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"5Unknown variable type"<<endl;
                            abort();
                    }
                } // end for
                
                int vmin = ( (scope==0)?0 : qs->nbVarInScope(scope-1) );
                int vmax = qs->nbVarInScope(scope)-1;
                vector<int> zevalues;
                for (int i = vmin; i<=vmax;i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            zevalues.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            zevalues.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default : 
                            cout<<"6unknown Variable type"<<endl;
                            abort();
                    }
                }
                Strategy candidate(false,vmin,vmax,scope,zevalues);
                
                Strategy son_of_candidate = rSolve(qs,scope+1,assign,nodes);
                if (son_of_candidate.isFalse()) candidate = Strategy::SFalse();
                else candidate.attach(son_of_candidate);
                
                if (!candidate.isFalse()) {
                    delete sol;
                    delete espace;
                    return candidate;
                }
                delete sol;
                sol=solutions.next();
            } // end while sol != null
            delete espace;
            return Strategy(StrategyNode::SFalse());
        } // end if..else (existential)
    }// end "Second case"
}


//////////////////////
//////////////////////
//////////////////////

QSolverUnblockable2::QSolverUnblockable2(Qcop* sp) {
    this->sp = sp;
    nbRanges=new int;
}

Strategy QSolverUnblockable2::solve(unsigned long int& nodes) {
    vector<int> plop;
    plop.clear();
    return rSolve(sp,0,plop,nodes);
}

Strategy QSolverUnblockable2::rSolve(Qcop* qs,int scope, vector<int> assignments, unsigned long int& nodes) {
    nodes++;
    //cout<<"rSolve for scope "<<scope<<" with assignments ";
    //    for (int i=0;i<assignments.size();i++) cout<<assignments[i]<<" ";
    //    cout<<endl;
    //////////////////////////////////////////////////////////////////////////////////////////
    // First case : Unblockable QCSP+ : Goal checking.                                      //
    //  If the goal is failed, even before the end on f the search, we fail.                //
    //////////////////////////////////////////////////////////////////////////////////////////
    MySpace* g = qs->getGoal();
    if (g == NULL) {return Strategy(StrategyNode::SFalse());}
    for (int i=0;i<assignments.size();i++) {
        switch (g->type_of_v[i]) {
            case VTYPE_INT : 
                rel(*g,*(static_cast<IntVar*>(g->v[i])) == assignments[i]);
                break;
            case VTYPE_BOOL :
                rel(*g,*(static_cast<BoolVar*>(g->v[i])) == assignments[i]);
                break;
            default :
                cout<<"1Unknown variable type"<<endl;
                abort();
        }
    }
    if (g->status() == SS_FAILED) {
        delete g;
        return Strategy(StrategyNode::SFalse());
    }
    delete g;
    
    if (scope == qs->spaces()) {
        return Strategy(StrategyNode::STrue());
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    // Second case : we are in the middle of the problem...                                //
    /////////////////////////////////////////////////////////////////////////////////////////
    else {
        MySpace* espace = qs->getSpace(scope);
        if (espace == NULL) cout<<"I caught a NULL for scope "<<scope<<". I will crash..."<<endl;
        for (int i=0;i<assignments.size();i++) {
            switch (espace->type_of_v[i]) {
                case VTYPE_INT : 
                    rel(*espace,*(static_cast<IntVar*>(espace->v[i])) == assignments[i]);
                    break;
                case VTYPE_BOOL :
                    rel(*espace,*(static_cast<BoolVar*>(espace->v[i])) == assignments[i]);
                    break;
                default :
                    cout<<"2Unknown variable type"<<endl;
                    abort();
            }
        }
        
        // Second case, first subcase : current scope is universal
        /////////////////////////////////////////////////////////
        if (qs->quantification(scope)) {
            if (espace->status() == SS_FAILED) {
                delete espace;
                return Strategy(StrategyNode::STrue());
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol = solutions.next();
            if (sol == NULL) {
                delete espace;
                return Strategy(StrategyNode::STrue());
            }
            
            Strategy retour = StrategyNode::Dummy();
            while (sol != NULL) {
                vector<int> assign;
                for (int i = 0; i<sp->nbVarInScope(scope);i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            assign.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            assign.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"3Unknown variable type"<<endl;
                            abort();
                    }
                }
                
                int vmin = ( (scope==0)? 0 : (qs->nbVarInScope(scope-1)) );
                int vmax = (qs->nbVarInScope(scope))-1;
                vector<int> zevalues;
                for (int i = vmin; i<=vmax;i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            zevalues.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            zevalues.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"4Unknown variable type"<<endl;
                            abort();
                    }
                }
                Strategy toAttach(true,vmin,vmax,scope,zevalues);
                
                Strategy son = rSolve(qs,scope+1,assign,nodes);
                if (son.isFalse()) {
                    delete sol;
                    delete espace;
                    return Strategy(StrategyNode::SFalse());
                }
                toAttach.attach(son);
                retour.attach(toAttach);
                delete sol;
                sol = solutions.next();
            } // end of while
            delete espace;
            return retour;
        } // end of if(universal)
        
        // Second case, second subcase : current scope is existential
        ////////////////////////////////////////////////////////////
        else {
            if ((espace->status()) == SS_FAILED) {
                delete espace;
                return Strategy(StrategyNode::SFalse());
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol =solutions.next();
            if (sol == NULL) {
                delete espace;
                return Strategy(StrategyNode::SFalse());
            }
            while (sol != NULL) {
                vector<int> assign;
                for (int i = 0; i<sp->nbVarInScope(scope);i++) {
		  //                    cout << "i = "<<i<<endl;
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            assign.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            assign.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default :
                            cout<<"5Unknown variable type"<<endl;
                            abort();
                    }
                } // end for
                
                int vmin = ( (scope==0)?0 : qs->nbVarInScope(scope-1) );
                int vmax = qs->nbVarInScope(scope)-1;
                vector<int> zevalues;
                for (int i = vmin; i<=vmax;i++) {
                    switch (sol->type_of_v[i]) {
                        case VTYPE_INT : 
                            zevalues.push_back( (static_cast<IntVar*>(sol->v[i]))->val() );
                            break;
                        case VTYPE_BOOL : 
                            zevalues.push_back( (static_cast<BoolVar*>(sol->v[i]))->val() );
                            break;
                        default : 
                            cout<<"6unknown Variable type"<<endl;
                            abort();
                    }
                }
                Strategy candidate(false,vmin,vmax,scope,zevalues);
                
                Strategy son_of_candidate = rSolve(qs,scope+1,assign,nodes);
                if (son_of_candidate.isFalse()) candidate = Strategy::SFalse();
                else candidate.attach(son_of_candidate);
                
                if (!candidate.isFalse()) {
                    delete sol;
                    delete espace;
                    return candidate;
                }
                delete sol;
                sol=solutions.next();
            } // end while sol != null
            delete espace;
            return Strategy(StrategyNode::SFalse());
        } // end if..else (existential)
    }// end "Second case"
}
