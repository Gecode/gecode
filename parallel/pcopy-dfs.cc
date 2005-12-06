/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: pcopy-dfs.cc
 * Description:
 * File with the code for the parallel depth first search engine using copying.
 *
 * History:
 * 2005-09-04: Morten Nielsen : File created from PReco-DFS and Copy-DFS.
 * 
 *
 */


#include "parallel.hh"
#include <pthread.h>
#include <iostream>
using namespace std;
namespace Gecode {
namespace Parallel {

  PCopyDFS::PCopyDFS(ShareStrategy* o) 
  : cur(NULL), s(SEARCH_DONE), options(o)  { //cout << propagate << "stat test" << endl;
		}

		
  PCopyDFS::~PCopyDFS(void) {
    delete cur;
    ds.reset();
  }

  void
  PCopyDFS::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
  }

  Space* 
  PCopyDFS::searchStep(void) {
  //cout << "searching one step." << std::endl;
		//while (cur) {
		if(cur != NULL) {
			//cout << "entries in the stack: " << ds.entries() << endl;
		//	cout << "propagate" << endl;
	unsigned int alt;
	switch (cur->status(alt,propagate)) {
	case SS_FAILED: {
		//cout << "failed" << endl;
	  fail++;
	  delete cur; 
	  cur = NULL;
	  ParallelStatistics::current(NULL);
	  return NULL;
	  break;}
	case SS_SOLVED:{
	  
			//cout << "solved" << ds.entries() << "entries..." <<endl;
			//s = SEARCH_DONE;
			Space* t = cur;
			cur = NULL;
			ParallelStatistics::current(NULL);
	    return t;
	   break;}
	case SS_BRANCH: {
	  
				if (alt > 1) {
	  ds.push(cur,alt);
	  FullStatistics::push(ds.top().space());
	  clone++;
	}
	commit++;
	cur->commit(0,NULL,propagate);
	return NULL;
	   break;}
	}
	//cout << "done prop" << endl;
	} else  {
	if (ds.empty()) {
		s = SEARCH_DONE;
	  return NULL;
	}
	//cout << "test" << endl;
	
	unsigned int alt = ds.top().alt();
	if (ds.top().rightmost()) {
	  cur = ds.pop().space();
	  FullStatistics::pop(cur);
		//cout << "just poped" << endl;
	} else {
		//cout << "commiting to alt: " << alt << endl;
	  clone++;
	  cur = ds.top().space()->clone(true,propagate);
	  ds.top().next();

	}
			commit++;
	cur->commit(alt,NULL,propagate);
	FullStatistics::current(cur);
	}
		////cout << "test" << endl;
    //s = SEARCH_DONE;
		return NULL;
	}
	
	void
	PCopyDFS::newRoot(Space* t) {
		s = SEARCH_WORKING;
		unsigned int alt;
		long unsigned int p=0;
		//cout << "new root: status and clone." << endl;
		Space* c = (t->status(alt,p) == SS_FAILED) ? NULL : t->clone(false);
		//cout << "new root: status check check." << endl;
		cur= c;
		//delete t;
		//if(cur == NULL) s = SEARCH_DONE;
		propagate += p;
		//cout << "new root: done: status " << (s==SEARCH_DONE) << endl;
	}
	
	void PCopyDFS::newRoot(Space* t, unsigned int &alt, void* option) {
		commit++;
		//t->commit(alt, static_cast<BranchingDesc*>(option), propagate);
		t->commit(alt, NULL, propagate);
		//cout << "commiting to share" << endl;
		newRoot(t);
	}

	Space*
	PCopyDFS::share(unsigned int &alt, void* option) {
		  //cout << "in share function" << endl;
			if(ds.entries() > 3) {
				//cout << "has enough to share..." << endl;

				//Search::CopyNode* n = NULL;
				/*unsigned int pos = 0;
				int loop=1;
				//cout << "finding position to share." << endl;
				switch(options->search) {
					SHARE_BOTTOM:
					  pos = ds.entries();
						while(loop == 1 && pos > 0) {
							if(ds[pos].rightmost()){
						////cout << "right most at pos " << pos << endl;
								pos--;
							}	else{
								n = &ds[pos];
								loop = 0;
							}
						}
						break;
					SHARE_RECOMP:
					SHARE_TOP:
					default:
						while(loop == 1 && pos < (ds.entries()-1)) {
							if(ds[pos].rightmost()){
								//cout << "right most at pos " << pos << endl;
								pos++;
							}	else{
								//cout << "found share" << endl;
								n = &ds[pos];
								loop = 0;
							}
						}
				break;
			}

				if(loop == 1||n == NULL) return NULL;
			*/
			  alt = ds.top().alt();
				Space* t = ds.pop().space();
				
				Space* q = t->clone(false);
			
				delete t;
			
				clone++;


				//alt = n->share();

				//option = n->desc();

				//cout << "sharing a space." << endl;

				return q;
			}
			return NULL;
	}
	
	SearchStatus
	PCopyDFS::status() const {
		return s;
	}

	ParallelStatistics
	PCopyDFS::statistics() const {
		////cout <<  this->propagate << "stat test" << endl;
		return *this;
	}
	
	void
	PCopyDFS::clear() {
		cur = NULL;
	}
	
	void
	PCopyDFS::setCurrent(Space* t) {
		cur = t;
		ParallelStatistics::current(cur);
	}
}
}
