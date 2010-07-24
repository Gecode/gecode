/****   , [ Worker.cc ], 
 Copyright (c) 2010 Universite de Caen Basse Normandie- Jeremie Vautard 
 
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

#include "Worker.hh" 


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


QWorker::~QWorker() {
  //  cout<<"Destroying "<<this<<endl;
}

void QWorker::stopAndReturn() {
  access.acquire();
  if (stopandforget < 1) stopandforget=1;
  access.release();
}

void QWorker::stopAndForget() {
  access.acquire();
  if (stopandforget < 2) stopandforget=2;
  access.release();
}

bool QWorker::mustStop() {
  return (stopandforget > 0);
}

void QWorker::wake() {
  //	cout<<"Awaking "<<this<<endl;
  goToWork.signal();
}

vector<int> QWorker::workPosition() {
  //  access.acquire();
  //  vector<int> ret = currentWork.root();
  //  access.release();
  return currentWork.root();
}

void QWorker::run() {
	
  while(true) {
    access.acquire();
    stopandforget=0;
    finished=false;
    todo.clear();
    access.release();
    //		cout<<"QWorker "<<this<<" : One turn !"<<endl;
    QWork cur= wm->getWork(this);
    //		cout<<"QWorker "<<this<<" got work"<<endl;
    if (cur.isStop()) {
      return;
    }
    else if (cur.isWait()) {
      //      clock_t start = clock();
      //      cout<<this<<" WAIT goToWork "<<start<<endl;
      goToWork.wait();
      //      clock_t stop = clock();
      //      cout<<this<<" CONT goToWork "<<stop<<" waited "<<(stop-start)<<endl;
    }
    else {
      access.acquire();
      currentWork = cur;
      access.release();			
      Strategy ret=solve();
      access.acquire();
      bool forget= (stopandforget == 2);
      access.release();
      if (!forget) {
				//				cout<<"QWorker "<<this<<" returning work..."<<ret.nbTodos()<<" todo in strategy and "<<todo.size()<<" works to do."<<endl;
        wm->returnWork(this,ret,todo,cur.root());
      }
      else {
				//				cout<<"QWorker "<<this<<" forgetting work..."<<endl;
				
				for(list<QWork>::iterator i(todo.begin());i != todo.end();i++) {
					(*i).clean();
				}
				todo.clear();
      }
    }
  }
}

Strategy QWorker::solve() {
  return rsolve(currentWork.getScope(),currentWork.getRemaining());
}


Strategy QWorker::rsolve(int scope,/*vector<int> assignments,*/Engine* L) {
  access.acquire();
  bool forget = (stopandforget==2);
  access.release();
  if (forget) {
    delete L;
    return Strategy::Dummy();
  }	
	
  MySpace* sol = static_cast<MySpace*>(L->next());
  Strategy ret=Strategy::Dummy();
  bool LwasEmpty = true;
	
	
  while ((sol != NULL) ) {
    LwasEmpty=false;
    vector<int> assignments = getTheValues(sol,0,sol->nbVars()-1);
    Strategy result;
		
    if (scope == (problem->spaces() - 1) ) { // last scope reached. Verify the goal...
      MySpace* g = problem->getGoal();
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
				delete L;
				result = Strategy::SFalse();
      }
      else {
				int vmin = ( (scope==0)? 0 : (problem->nbVarInScope(scope-1)) );
				int vmax = (problem->nbVarInScope(scope))-1;
				vector<int> zevalues=getTheValues(sol,vmin,vmax);		
				result=Strategy(problem->quantification(scope),vmin,vmax,scope,zevalues);
				result.attach(Strategy::STrue());
				delete g;
				//	delete sol;
				delete goalsol;
      }
    }
		
    else { // This is not the last scope...
      MySpace* espace = problem->getSpace(scope+1);
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
			
      access.acquire();
      forget=(stopandforget == 2);
      bool stop=(stopandforget==1);
      access.release();
      if (forget) {
				delete sol;
				delete solutions;
				return Strategy::Dummy();
      }
      if (stop) {
				vector<int> root1;
				if (scope!=0) {root1= getTheValues(sol,0,problem->nbVarInScope(scope-1)-1);}
				QWork current(scope,root1,L);
				vector<int> root2 = getTheValues(sol,0,problem->nbVarInScope(scope)-1);
				QWork onemore(scope+1,root2,solutions);
				todo.push_back(current);
				todo.push_back(onemore);
				int vmin = ( (scope==0)? 0 : (problem->nbVarInScope(scope-1)) );
				int vmax = (problem->nbVarInScope(scope))-1;
				vector<int> zevalues=getTheValues(sol,vmin,vmax);
				Strategy toAttach(problem->quantification(scope),vmin,vmax,scope,zevalues);
				toAttach.attach(Strategy::Stodo());
				ret.attach(toAttach);
				ret.attach(Strategy::Stodo());
				return ret;
      }
			
      result=rsolve(scope+1,solutions);
    }
		
    int vmin = ( (scope == 0) ? 0 : (problem->nbVarInScope(scope-1)) );
    int vmax = (problem->nbVarInScope(scope))-1;
    vector<int> zevalues=getTheValues(sol,vmin,vmax);		
    delete sol;
    access.acquire();
    forget=(stopandforget == 2);
    access.release();
    if (forget) {
      delete L;
      return Strategy::Dummy();
    }
    if (problem->quantification(scope)) { // current scope is universal
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
				if (result.isComplete()) { // ...and has no todo nodes. So...
					ret = Strategy(problem->quantification(scope),vmin,vmax,scope,zevalues);
					ret.attach(result);
					delete L;
					return ret; // we return it immediately
				}
				else { // If result is not false, but still has todo nodes...
					Strategy toAttach(problem->quantification(scope),vmin,vmax,scope,zevalues);
					toAttach.attach(result); // the node corresponding to the current scope iis added...
					ret.attach(toAttach); // and the corresponding strategy is attached to a Dummy node. Next loop in the outer while will need it...
				}
      }
    }
    sol = static_cast<MySpace*>(L->next());
  }
  delete L;
  if (problem->quantification(scope))  //universal scope
    return (LwasEmpty ? Strategy::STrue() : ret);
  else {
    if (ret.isComplete())
      return Strategy::SFalse();
    else
      return ret;
  }
}
