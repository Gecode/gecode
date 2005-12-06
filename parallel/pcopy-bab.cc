/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: pcopy-dfs.cc
 * Description:
 * File with the code for the parallel branch and bound engine using copying.
 *
 * History:
 * 2005-09-05: Morten Nielsen : File created from PReco-DFS and Copy-DFS.
 * 
 *
 */


#include "parallel.hh"
#include <pthread.h>
#include <iostream>
using namespace std;
namespace Gecode {
namespace Parallel {
	
  PCopyBAB::PCopyBAB(ShareStrategy* o) 
  : cur(NULL), s(SEARCH_DONE), mark(0), n_alt(0), constrained(false), options(o)  { //cout << propagate << "stat test" << endl;
		}

	
  PCopyBAB::~PCopyBAB(void) {
    delete cur;
    ds.reset();
  }
	
  void
  PCopyBAB::reset(Space* s) {
		//cerr << "reseting current" << endl;
    delete cur;
    //ds.reset();
    cur = s;
		//ParallelStatistics::current(cur);
  }
	
  bool
  PCopyBAB::searchStep(Space*& s1) {
  //cout << "searching one step." << std::endl;
		//while (cur) {
		if(cur != NULL) {
			if(constrained) {
				
				s1 = cur;
				constrained = false;
				return false;
			}
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
	  return false;
	  break;}
	case SS_SOLVED:{
			//cout << "solved" << ds.entries() << "entries..." <<endl;
			//s = SEARCH_DONE;
			s1 = cur->clone(true, propagate);
			delete cur;	
			cur = NULL;
			clone++;
			ParallelStatistics::current(NULL);
			mark = ds.entries();
	    return true;
	   break;}
	case SS_BRANCH: {
	  
				if (alt > 1) {
	  ds.push(cur,alt);
	  ParallelStatistics::push(ds.top().space());
	  clone++;
	}
	commit++;
	cur->commit(0,NULL,propagate);
	return false;
	   break;}
	}
	//cout << "done prop" << endl;
	} else  {
	if (ds.empty()) {
		s = SEARCH_DONE;
	  return false;
	}
	//cout << "test" << endl;
	
	unsigned int alt = ds.top().alt();
	if (ds.top().rightmost()) {
	  cur = ds.pop().space();
	  ParallelStatistics::pop(cur);
		cur->commit(alt,NULL,propagate);
		ParallelStatistics::current(cur);
		commit++;
		if(ds.entries()+1 <= mark) {
			mark--;
			s1 = cur;
			//delete cur;
			return false;
		}
		//cout << "just poped" << endl;
	} else {
		//cout << "commiting to alt: " << alt << endl;
	  clone++;
	  cur = ds.top().space()->clone(true,propagate);
	  ds.top().next();
		cur->commit(alt,NULL,propagate);
		ParallelStatistics::current(cur);
		if(ds.entries() <= mark) {
			s1 = cur;
			//delete cur;
			return false;
		}
	}

	}
		////cout << "test" << endl;
    //s = SEARCH_DONE;
		return false;
	}
	
	void
	PCopyBAB::newRoot(Space* t) {
		s = SEARCH_WORKING;
		unsigned int alt;
		long unsigned int p=0;
		//cerr << "new root: status and clone." << endl;
		Space* c = NULL;
		if(t->status(alt,p) == SS_FAILED) c= NULL;	
		else {//cerr << "not false cloning" << endl;
		c=t->clone(false);
		}
		//cerr << "new root: status check check." << endl;
		cur= c;
		//constrained = true;
		//delete t;
		//if(cur == NULL) s = SEARCH_DONE;
		propagate += p;
		//cout << "new root: done: status " << (s==SEARCH_DONE) << endl;
	}
	
	void PCopyBAB::newRoot(Space* t, unsigned int &alt, void* option) {
		s = SEARCH_WORKING;
		commit++;
		//t->commit(alt, static_cast<BranchingDesc*>(option), propagate);
		unsigned int a=alt;
		long unsigned int p=0;
		if(t->status(a,p) == SS_FAILED) {
			
			fail++;
			return;	
		}
		else if(t->status(a,p) == SS_SOLVED){// cerr << "share is a solution" << endl;
			cur=t->clone(false);
			clone++;
		} else {
			//cerr << "commiting to share: " << alt << endl;
			if(t != NULL) {
				t->commit(alt, NULL, propagate);
		
				newRoot(t);
			}
	}
	propagate += p;
	}

	bool
	PCopyBAB::share(Space*& ret, unsigned int &alt, void* option) {
		  //cout << "in share function" << endl;
			if(ds.entries() > 3) {
				//cout << "has enough to share..." << endl;

			  alt = ds.top().alt();
				Space* t = ds.pop().space();
				FullStatistics::pop(t);
				option =NULL;
				ret = t->clone(false);
			  
				delete t;
			
				clone++;
				if(ds.entries() +1 <= mark) {
					mark--;
					return true;
				} else {
					return false;
				}

				return false;
			}
			ret = NULL;
			return false;
	}
	
	SearchStatus
	PCopyBAB::status() const {
		return s;
	}

	ParallelStatistics
	PCopyBAB::statistics() const {
		return *this;
	}
	
	void
	PCopyBAB::clear() {
		cur = NULL;
	}

	void
	PCopyBAB::setMark(void) {
		mark = ds.entries();
		if(cur != NULL) constrained = true;
	}
	
	void
	PCopyBAB::setCurrent(Space* t) {
		cur = t;
		constrained = true;
		ParallelStatistics::current(cur);
	}
	
}
}
