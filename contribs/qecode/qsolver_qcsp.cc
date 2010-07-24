/****   , [ QCSP_Solver.cc ], 
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

#include "qsolver_qcsp.hh"
#include <climits>

inline vector<int> getTheValues(MySpace* sol,int vmin,int vmax){
  vector<int> zevalues;
	//	cout<<sol<<" "<<vmin<<" "<<vmax<<endl;
  if (vmax > (sol->nbVars())) cout<<"getTheValues mal appele"<<endl;
  for (int i = vmin; i<=vmax;i++) {
		//	cout<<i<<" ";
		//	cout.flush();
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
	//	cout<<endl;
	
  return zevalues;
}

QCSP_Solver::QCSP_Solver(Qcop* sp) {
	this->sp = sp;
	nbRanges=new int;
}

Strategy QCSP_Solver::solve(unsigned long int& nodes, unsigned int limit) {
	this->limit=limit;
	MySpace* espace=sp->getSpace(0);
  Options o;
  Engine* solutions = new WorkerToEngine<Gecode::Search::Sequential::DFS>(espace,sizeof(MySpace),o);	
	return rSolve(sp,0,solutions,nodes);
}



Strategy QCSP_Solver::rSolve(Qcop* qs,int scope,Engine* L, unsigned long int& nodes) {
  nodes++;
  MySpace* sol = static_cast<MySpace*>(L->next());
  Strategy ret=Strategy::Dummy();
  bool LwasEmpty = true;
  while ((sol != NULL) ) {
    LwasEmpty=false;
    vector<int> assignments = getTheValues(sol,0,sol->nbVars()-1);
    Strategy result;
		
    if (scope == (qs->spaces() - 1) ) { // last scope reached. Verify the goal...
      MySpace* g = qs->getGoal();
      for (int i=0;i<g->nbVars();i++) {
				switch (g->type_of_v[i]) {
					case VTYPE_INT : 
						rel(*g,*(static_cast<IntVar*>(g->v[i])) == assignments[i]);
						break;
					case VTYPE_BOOL :
						rel(*g,*(static_cast<BoolVar*>(g->v[i])) == assignments[i]);
						break;
					default :
						cout<<"Unknown variable type"<<endl;
						abort();
				}
      }
      Gecode::DFS<MySpace> solutions(g);
      MySpace* goalsol = solutions.next();
      if (goalsol == NULL) {
				delete g;
				result = Strategy::SFalse();
      }
      else {
				int vmin = ( (scope==0)? 0 : (qs->nbVarInScope(scope-1)) );
				int vmax = (qs->nbVarInScope(scope))-1;
				vector<int> zevalues=getTheValues(sol,vmin,vmax);		
				result=Strategy(qs->quantification(scope),vmin,vmax,scope,zevalues);
				result.attach(Strategy::STrue());
				delete g;
				//	delete sol;
				delete goalsol;
      }
		}
		
    else { // This is not the last scope...
      MySpace* espace = qs->getSpace(scope+1);
      for (int i=0;i<assignments.size();i++) {
				switch (espace->type_of_v[i]) {
					case VTYPE_INT : 
						rel(*espace,*(static_cast<IntVar*>(espace->v[i])) == assignments[i]);
						break;
					case VTYPE_BOOL :
						rel(*espace,*(static_cast<BoolVar*>(espace->v[i])) == assignments[i]);
						break;
					default :
						cout<<"Unknown variable type"<<endl;
						abort();
				}
      }			
      Options o;
      Engine* solutions = new WorkerToEngine<Gecode::Search::Sequential::DFS>(espace,sizeof(MySpace),o);
      delete espace;
      result=rSolve(qs,scope+1,solutions,nodes);
    }
		
    int vmin = ( (scope == 0) ? 0 : (qs->nbVarInScope(scope-1)) );
    int vmax = (qs->nbVarInScope(scope))-1;
    vector<int> zevalues=getTheValues(sol,vmin,vmax);		
    delete sol;
    if (qs->quantification(scope)) { // current scope is universal
      if (result.isFalse()) // one branch fails
			{
				delete L;
				return Strategy::SFalse();
			}
      else {
				Strategy toAttach(true,vmin,vmax,scope,zevalues);
				toAttach.attach(result);
				ret.attach(toAttach);
      }
    }
    else { //current scope is existential
      if (!result.isFalse()) { // result not the truivilally false strategy...
				delete L;
				if (scope >= limit) return Strategy::STrue();
				ret = Strategy(qs->quantification(scope),vmin,vmax,scope,zevalues);
				ret.attach(result);
				return ret; // we return it immediately
      }
    }
    sol = static_cast<MySpace*>(L->next());
  }
  delete L;
	if (scope>limit)
		ret = Strategy::STrue();
  if (qs->quantification(scope))  //universal scope
    return (LwasEmpty ? Strategy::STrue() : ret);
  else 
		return Strategy::SFalse();
}
