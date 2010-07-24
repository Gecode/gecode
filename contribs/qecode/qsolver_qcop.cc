/****   , [ qsolver_qcop.cc ], 
Copyright (c) 2010 Universite de Caen Basse Normandie - Jeremie Vautard 

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

#include "qsolver_qcop.hh"
#include <climits>

QCOP_solver::QCOP_solver(Qcop* sp) {
    this->sp = sp;
    nbRanges=new int;
}

Strategy QCOP_solver::solve(unsigned long int& nodes) {
    vector<int> plop;
    plop.clear(); 
    return rSolve(sp,0,plop,nodes);
}

Strategy QCOP_solver::rSolve(Qcop* qs,int scope, vector<int> assignments, unsigned long int& nodes) {
    nodes++;
//    cout<<"rSolve for scope "<<scope<<" with assignments ";
//    for (int i=0;i<assignments.size();i++) cout<<assignments[i]<<" ";
//    cout<<endl;
    //////////////////////////////////////////////////////////////////////////////////////////
    // First case : we reached the goal, we just have to check it                           //
    //////////////////////////////////////////////////////////////////////////////////////////
    if (scope == qs->spaces()) {
//        cout<<"First case"<<endl;
        MySpace* g = qs->getGoal();
        if (g == NULL) {/*cout<<"the goal was null"<<endl;*/return Strategy::SFalse();}
        for (int i=0;i<g->nbVars();i++) {
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
//            cout<<"goal failed after assignments"<<endl;
            delete g;
            return Strategy::SFalse();
        }
//        cout<<"goal OK"<<endl;
        delete g;
        return Strategy(StrategyNode::STrue());
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////
    // Second case : we are in the middle of the problem...                                //
    /////////////////////////////////////////////////////////////////////////////////////////
    else {
//        cout<<"second case "<<endl;
        MySpace* espace = qs->getSpace(scope);
        if (espace == NULL) cout<<"I caught a NULL for scope "<<scope<<". I will crash..."<<endl;
//        cout<<"size of assignments "<<assignments.size()<<endl;
        for (int i=0;i<assignments.size();i++) {
//            cout<<"I assign variable "<<i<<" with value "<<assignments[i]<<endl; cout.flush();
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
//            cout<<"universal"<<endl;
            
            if (espace->status() == SS_FAILED) {
//                cout<<"the scope is failed"<<endl;
                delete espace;
                return Strategy::STrue();
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol = solutions.next();
            if (sol == NULL) {
//                cout<<"first sol is null"<<endl;
                delete espace;
                return Strategy::STrue();
            }
            
            Strategy retour = Strategy::Dummy();
            while (sol != NULL) {
//                cout<<"a solution"<<endl;
                vector<int> assign;
                for (int i = 0; i<sol->nbVars();i++) {
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
//                    cout<<"the son is false"<<endl;
                    delete sol;
                    delete espace;
                    return Strategy::SFalse();
                }
//                cout<<"the son is true"<<endl;
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
//            cout<<"existential"<<endl;
            if ((espace->status()) == SS_FAILED) {
//                cout<<"the Espace is failed"<<endl;
                delete espace;
                return Strategy::SFalse();
            }
            
            DFS<MySpace> solutions(espace);
            MySpace* sol =solutions.next();
            if (sol == NULL) {
//                cout<<"the first sol is null"<<endl;
                delete espace;
                return Strategy::SFalse();
            }
            
            OptVar* opt = qs->getOptVar(scope);
            int opttype = qs->getOptType(scope);
            Strategy retour(StrategyNode::SFalse());
            int score= ( (opttype == 1) ? INT_MAX : INT_MIN );
            while (sol != NULL) {
//                cout<<"une solution"<<endl;
                vector<int> assign;
                for (int i = 0; i<sol->nbVars();i++) {
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
                
                int score_of_candidate;
                if (candidate.isFalse()) score_of_candidate = ( (opttype == 1) ? INT_MAX : INT_MIN );
                else score_of_candidate = opt->getVal(candidate);
//                cout<<"score of candidate is "<<score_of_candidate<<endl;
//                cout<<"Opt type is";
                switch (opttype) {
                    case 0 : // ANY
//                        cout<<"ANY"<<endl;
                        if (!candidate.isFalse()) {
                            delete sol;
                            delete espace;
                            return candidate;
                        }
                        break;
                    case 1 : // MIN
//                        cout<<"MIN"<<endl;
                        if (score_of_candidate < score) {
                            retour=candidate;
                            score=score_of_candidate;
                        }
                        break;
                    case 2 : // MAX
//                        cout<<"MAX"<<endl;
                        if (score_of_candidate > score) {
                            retour=candidate;
                            score=score_of_candidate;
                        }
                        break;
                    default : 
                        cout<<"Unknown opt type : "<<opttype<<endl;
                        abort();
                        break;
                } // end switch
                delete sol;
                sol=solutions.next();
            } // end while 
            delete espace;
            return retour;
        } // end if..else (existential)
    }// end "Second case"
}
