/************************************************************ Work.hh
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
#ifndef __QECODE_QWORK__
#define __QECODE_QWORK__
#include<vector>
#include "Strategy.hh"
#include "qecode.hh"
#include "gecode/support.hh"
#include "gecode/search.hh"
#include "gecode/search/support.hh"
#include "gecode/search/sequential/dfs.hh"

using namespace std;
using namespace Gecode;
using namespace Gecode::Support;

class QECODE_VTABLE_EXPORT QWork {
private:
	bool wait;
  bool stop;
	vector<int> theRoot;
  Gecode::Search::Engine* remaining;
	int scope;
public: 
  QECODE_EXPORT QWork(){wait=stop=0; remaining=NULL; scope=-1;} ///< creates a dummy work. Don't try to solve it.
  QECODE_EXPORT QWork(int scope,vector<int> root,Gecode::Search::Engine* todo);
  QECODE_EXPORT ~QWork();
	
  QECODE_EXPORT static QWork Wait();
  QECODE_EXPORT static QWork Stop();
	
  QECODE_EXPORT forceinline bool isWait() {return wait;}
  QECODE_EXPORT forceinline bool isStop() {return stop;}
	
  QECODE_EXPORT forceinline Gecode::Search::Engine* getRemaining() {return this->remaining;}
  QECODE_EXPORT vector<int> root() {
    vector<int> ret=this->theRoot;
    return ret;
  }
  QECODE_EXPORT forceinline int getScope() {return this->scope;}
  QECODE_EXPORT forceinline void clean() {if (!wait && !stop) delete remaining;}
};


#endif
