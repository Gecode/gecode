/************************************************************ WorkManager.cc
 Copyright (c) 2010 Universite d'Orleans - Jeremie Vautard 
 
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

#include "WorkManager.hh"
bool isPrefix(vector<int> prefix,vector<int> a) {
  if (a.size() < prefix.size())
    return false;
  else
    for (unsigned int i=0;i<prefix.size();i++) 
      if (prefix[i] != a[i])
	return false;
	
  return true;
}

WorkPool::WorkPool(WorkComparator* a) {
  cmp=a;
}

void WorkPool::push(QWork q) {
  list<QWork>::iterator i = l.begin();
  while (i != l.end() && cmp->cmp((*i),q)) i++;
  l.insert(i,q);
}

QWork WorkPool::pop()  {
  QWork ret=l.front();
  l.pop_front();
  return ret;
}

void WorkPool::trash(vector<int> prefix) {
  //    cout<<"      WPT called on ";
  //if (prefix.empty()) cout<<"empty";
  //for (int i=0;i< prefix.size();i++) cout<<prefix[i]<<" ";
  //cout<<endl;
  //  cout<<"debug "<<l.size()<<endl;  

    list<QWork>::iterator i=l.begin(); 
   while (!(i == l.end())) {
  //  cout<<"| ";
     //   cout.flush();
    bool avance=true;
    vector<int> root = (*i).root();
    if (isPrefix(prefix,root)) {
      (*i).clean();
      i=l.erase(i);
      avance = false;
      // cout<<"erased";
    }
    if (avance) i++;
  }
   //cout<<endl;

  //for (list<QWork>::iterator i=l.begin();i != l.end();i++) {
  //vector<int> root = (*i).root();
    //if (isPrefix(prefix,root)) {
      // cout<<"      trashing ";
      // if (root.empty()) cout<<"empty";
      // for (int j=0;j< root.size();j++) cout<<root[j]<<" ";
      // cout<<endl;
      //(*i).clean();
      //l.erase(i);
      //i--;
      //}
    // }
}


void WorkManager::getNewWorks() {
  // cout<<"WM asking for "<<actives.front()<<" to stopandreturn"<<endl;
  (actives.front())->stopAndReturn();
}

WorkManager::WorkManager(Qcop* p,WorkComparator* c) : Todos(c) {
  problem=p;
  vector<int> v;
  MySpace* espace=p->getSpace(0);
  Options o;
  Engine* solutions = new WorkerToEngine<Gecode::Search::Sequential::DFS>(espace,sizeof(MySpace),o);
  QWork first(0,v,solutions);
  Todos.push(first);
  finished=false;
  S = Strategy::Dummy();
  S.attach(Strategy::Stodo());
}

QWork WorkManager::getWork(AQWorker* worker) {
  //  clock_t start = clock();
  //  cout<<worker<<" WAIT getWork "<<start<<endl;
  mex.acquire();
  //  clock_t stop = clock();
  // cout<<worker<<" CONT getWork "<<stop<<" waited "<<(stop-start)<<endl;

  //  cout<<"G ";cout.flush();
  // cout<<"WM getwork acquired"<<endl;
  QWork ret;
  if (Todos.empty()) {
    // cout<<"WM : Todos was empty"<<endl;
    if (actives.empty()) {
      // cout<<"WM : Actives was empty, stopping worker."<<endl;
      ret = QWork::Stop();	
      finished=true;
    }
    else {
      getNewWorks();
      // cout<<"WM sleeping worker"<<endl;
      ret = QWork::Wait();
      idles.push_back(worker);
    }
  }
  else {
    // cout<<"WM : giving work ";
    ret = Todos.top();
    Todos.pop();
    // if (ret.root().empty()) cout<<"empty";
    // for (int i=0;i< ret.root().size();i++) cout<<ret.root()[i]<<" ";
    // cout<<endl;
		
    actives.push_back(worker); 
  }
  // cout<<"WM getwork release"<<endl;
  mex.release();
  return ret;
}

void WorkManager::returnWork(AQWorker* worker,Strategy ret,list<QWork> todo,vector<int> position) { 
  // If the worker is not among the actives ones, ignore his job. Else, withdraw it from the active workers
  // and process its result

  //  clock_t start = clock();
  //  cout<<worker<<" WAIT returnWork "<<start<<endl;
  mex.acquire();
  //  clock_t stop = clock();
  // cout<<worker<<" CONT returnWork "<<stop<<" waited "<<(stop-start)<<endl;
  // cout<<"RW returnWork acquired"<<endl;
  // cout<<"returning work at ";
  // if (position.empty()) cout<<"empty";
  // for (int i=0;i< position.size();i++) cout<<position[i]<<" ";
  // cout<<endl;

  bool ok=false;
  list<AQWorker*>::iterator i = actives.begin();
  while (!(i == actives.end()) && !ok) {
    bool avance=true;
    if ((*i) == worker) {
      // cout<<"RW Worker found in actives. Deleting"<<endl;
      i=actives.erase(i);
      avance = false;
      ok=true;
    }
    if (avance) ++i;
  }
  if (!ok) {
    // cout<<"RW Ignoring work"<<endl;
    // when ignoring the result, we have to delete every search engine from the eventual to-do works it did
    for (list<QWork>::iterator j = todo.begin();j != todo.end();j++) {
      (*j).clean();
    }
    // cout<<"RW release"<<endl;
    mex.release();
    return;
  }
	
  // processing results...
	
  // adding todo to Todos
	
  // going to father of substrategy
  bool acceptable = true;
  Strategy father = S.getSubStrategy(position);
  if (father.isDummy() && (father.id() != S.id())) {
    // cout<<"ERROR : RW Work was not in current strategy. Ignoring."<<endl;
    acceptable=false;
  }
  // remove the Todo node that marked this work
  if (acceptable) { 
    // cout<<" RW Father position is ";
    // vector<int> checkPosition = father.getPosition();
    // if (checkPosition.empty()) cout<<"empty";
    // for (int i=0;i< checkPosition.size();i++) cout<<checkPosition[i]<<" ";
    // cout<<endl;
		
    for (list<QWork>::iterator j = todo.begin();j != todo.end();j++) {
      //		cout<<"WM todo + 1"<<endl;
      if (!idles.empty()) {
	//			cout<<"WM waking an idle"<<endl;
	AQWorker* towake = idles.back();
	idles.pop_back();
	towake->wake();
      }
      // cout<<"RW Adding work in Todos : ";
      // if ((*j).root().empty()) cout<<"empty";
      // for (int i=0;i< (*j).root().size();i++) cout<<(*j).root()[i]<<" ";
      // cout<<endl;
			
      Todos.push(*j);
    }
		
    for (unsigned int i=0;i<father.degree();i++) {
      if (father.getChild(i).isTodo()) {
	// cout<<"RW deleting Todo node"<<endl;
	father.detach(i);
	i--;
      }
    }
		
		
    if (!ret.isComplete()) { // still work to do...
      // attach the (incomplete) substrategy
      // cout<<"RW returned work was incomplete. Attaching"<<endl;
      father.attach(ret);
    }
    else if (!ret.isFalse()) { // complete substrategy
      // attach the (complete) substrategy
      // recursively update node	
      // cout<<"RW returned work was true. Attaching and updating"<<endl;
      father.attach(ret);
      updateTrue(father);
    }
    else { //ret is false
      // cout<<"RW returned work was false. Attaching and updating"<<endl;
      int myscope = problem->qt_of_var(position.size() + 1);
      if (myscope) { // if ret was a forall node, the father itself is false
	// cout<<"RW work is A -> father is false"<<endl;
	updateFalse(father);
      }
      else if (father.degree() == 0) { // Here ret was an exists node. If the father has no other son, then the father itself is false
	// cout<<"RW work is E, father is without child"<<endl;
	updateFalse(father);
      }
      // else {
      // 	// cout<<"RW work is E, father still has children"<<endl;
      // }
    }
  }
  else { // work was not acceptable
    for (list<QWork>::iterator j = todo.begin();j != todo.end();j++) {
      (*j).clean();
    }
  }
	
  // cout<<"RW release"<<endl;
  mex.release();
}

void WorkManager::updateTrue(Strategy s) { //called when s has a son who is a complete strategy
  // cout<<"  UT on ";
  // vector<int> hihi = s.getPosition();
  // if (hihi.empty()) cout<<"empty";
  // for (int i=0;i< hihi.size();i++) cout<<hihi[i]<<" ";
  // cout<<endl;
	
	
  if (s.isComplete()) {
    // cout<<"  UT Complete ";
    if (s.quantifier()) {
      // cout<< " A ";
      if (s.hasFather()) {
	// cout<<" with father"<<endl;
	updateTrue(s.getFather());
      }
      else {
	// cout<<" without father"<<endl;
      }
    }
    else {
      // cout<<" E ";
      if (s.hasFather()) {
	// cout<<" with father"<<endl;
	Strategy father = s.getFather();
	while (father.degree() != 0) {
	  father.detach(father.degree()-1);
	}
	father.attach(s);
	trashWorks(father.getPosition());
	updateTrue(father);
      }
      else {
	// cout<<" without father"<<endl;
				
      }
    }
  }
  else {
    // cout<<"  UT incomplete"<<endl;
  }
}

void WorkManager::updateFalse(Strategy s) { // called when s is false
  // cout<<"  UF on ";
  // vector<int> hihi = s.getPosition();
  // if (hihi.empty()) cout<<"empty";
  // for (int i=0;i< hihi.size();i++) cout<<hihi[i]<<" ";
  // cout<<endl;
	
  trashWorks(s.getPosition());
  if (s.isDummy()) {
    return;
  }
  Strategy father=s.getFather();
	
  if (s.quantifier()) { // if s is universal, its father is false
    // cout<<"  UF A";
    if (father.isDummy()) { // if father is dummy, it is the root of the strategy. We must cut all its children and ad a false node.
      // cout<<" father root "<<endl;
      while (father.degree() != 0) {
	father.detach(father.degree()-1);
      }
      father.attach(Strategy::SFalse());
    }
    // cout<<endl;
    updateFalse(s.getFather());
  }
  else { // if s is existential, it it detached from its father
    // cout<<"  UF E ";
    father.detach(s);
    if (father.degree() == 0) {
      // cout<<" father false"<<endl;
      updateFalse(father);
    }
    else {
      // cout<<" father still not false"<<endl;
    }
  }
}


void WorkManager::trashWorks(vector<int> prefix) {
  // cout<<"    TW called on ";
  // if (prefix.empty()) cout<<"empty";
  // for (int i=0;i< prefix.size();i++) cout<<prefix[i]<<" ";
  // cout<<endl;
	
  Todos.trash(prefix);
  for (list<AQWorker*>::iterator i=actives.begin();i!=actives.end();i++) {
    if (isPrefix(prefix,(*i)->workPosition())) {
      // cout<<"    TW stopping worker on";
      // vector<int> plop = (*i)->workPosition();
      // if (plop.empty()) cout<<"empty";
      // for (int j=0;j< plop.size();j++) cout<<plop[j]<<" ";
      // cout<<endl;
      (*i)->stopAndForget();
      i=actives.erase(i);
      i--;
    }
  }
}

void WorkManager::printStatus() {
  mex.acquire();
  cout<<(finished?"Problem solved":"Problem not solved")<<endl;
  cout<<"Size of pool : "<<Todos.size()<<endl;
  cout<<"# idles : "<<idles.size()<<endl;
  cout<<"# actives : "<<actives.size()<<endl;
  mex.release();
}
