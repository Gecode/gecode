/****   , [ Worker.hh ], 
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

#ifndef __QECODE_WORKER__
#define __QECODE_WORKER__
#include "qecode.hh"
#include<list>
#include<vector>
#include <time.h>
#include "QCOPPlus.hh"
#include "myspace.hh"
#include "Strategy.hh"
#include "AbstractWorker.hh"
#include "gecode/support.hh"
#include "gecode/search.hh"
#include "gecode/search/support.hh"
#include "gecode/search/sequential/dfs.hh"
#include "WorkManager.hh"

using namespace Gecode::Support;
using namespace Gecode::Search;
using namespace Gecode::Search::Sequential;


class QECODE_VTABLE_EXPORT QWorker : public AQWorker {

 private:
  WorkManager* wm;
  Gecode::Support::Event goToWork;
  Gecode::Support::Mutex access;
  int stopandforget; // 0 : continue, 1 : stop and return, 2 : stop and forget.
  Qcop* problem;
  QWork currentWork;
  list<QWork> todo;
  bool finished;
  Strategy rsolve(int scope,/*vector<int> assignments,*/Engine* L);
  

 public:
  QECODE_EXPORT QWorker(WorkManager* wm) {this->wm=wm; this->problem = wm->problem->clone();}
  QECODE_EXPORT ~QWorker();
  QECODE_EXPORT virtual void run();
  QECODE_EXPORT void stopAndReturn();
  QECODE_EXPORT void stopAndForget();
  QECODE_EXPORT vector<int> workPosition();
  QECODE_EXPORT bool mustStop();
  QECODE_EXPORT void wake();
  QECODE_EXPORT	Strategy solve();
};

#endif
