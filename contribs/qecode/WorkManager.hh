/************************************************************ WorkManager.hh
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

#ifndef __QECODE_WORKMANAGER__
#define __QECODE_WORKMANAGER__

#include <time.h>
#include <list>
#include <queue>
#include <vector>
#include "Strategy.hh"
#include "qecode.hh"
#include "AbstractWorker.hh"
#include "Work.hh"
#include "QCOPPlus.hh"
#include "gecode/minimodel.hh"
#include "gecode/support.hh"
#include "gecode/search.hh"
#include "gecode/search/support.hh"


using namespace Gecode;
using namespace std;
using namespace Gecode::Support;
using namespace Gecode::Search;

class QECODE_VTABLE_EXPORT WorkComparator {
	public : 
	virtual bool cmp(QWork a,QWork b)=0;
};

class QECODE_VTABLE_EXPORT WorkPool {
private:
	list<QWork> l;
	WorkComparator* cmp;
public:
	QECODE_EXPORT WorkPool(WorkComparator* a);
	QECODE_EXPORT void push(QWork q);
	QECODE_EXPORT QWork pop();
	QECODE_EXPORT bool empty() {return l.empty();}
	QECODE_EXPORT QWork top() {return l.front();}
	QECODE_EXPORT int size() {return l.size();}
	
	QECODE_EXPORT void trash(vector<int> prefix);
};


class QECODE_VTABLE_EXPORT WorkManager {
private:
	bool finished;
	Strategy S;
	WorkPool Todos;
	list<AQWorker*> actives;
	list<AQWorker*> idles;
	void getNewWorks();
	void updateTrue(Strategy s);
	void updateFalse(Strategy s);
	void trashWorks(vector<int> prefix);
	Strategy result;
	Gecode::Support::Mutex mex; // = PTHREAD_MUTEX_INITIALIZER;
public:
	Qcop* problem;
		
	QECODE_EXPORT WorkManager(Qcop* p,WorkComparator* c);
	
	QECODE_EXPORT QWork getWork(AQWorker* worker);

	QECODE_EXPORT void returnWork(AQWorker* worker,Strategy ret,list<QWork> todo,vector<int> position); 

	QECODE_EXPORT forceinline Strategy getResult() {return S;}
	
	QECODE_EXPORT forceinline bool isFinished() {mex.acquire();bool ret=finished;mex.release();return ret;}
	
	QECODE_EXPORT void printStatus();
};

#endif
