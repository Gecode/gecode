/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: parallel-dfs.cc
 * Description:
 * Spacehis is the source file for a script used to run recomputation DFS in parallel with the
 * parallel prototype. Spacehis is usually started using PDFS as a wrapper class.
 * But more advance uses are possible if used directly.
 *
 * History:
 * 2005-03-09: Morten Nielsen
 * Spacehis file is created, largely based on Christian Schultes search engine in
 * search/dfs-reco.cc
 * 2005-03-11: Morten Nielsen
 * Fixed some scope issues, added share() and status() member function.
 * 2005-04-18: Morten Nielsen
 * Fixed recomputation, some logs are missing, most work has been focused around
 * recomputation.
 * 2005-05-07: Morten Nielsen: Added ShareStrategy as an option on creation.
 *
 */


#include "parallel.hh"
#include <pthread.h>
#include <iostream>
using namespace std;
namespace Gecode {
namespace Parallel {
	
  PReCoBAB::PReCoBAB(unsigned int c_d0, unsigned int a_d0, ShareStrategy* o) 
  : cur(NULL),ds(a_d0), s(SEARCH_DONE), c_d(c_d0), d(0), options(o), mark(0), constrained(false)  { //cout << propagate << "stat test" << endl;
		}

	
  PReCoBAB::~PReCoBAB(void) {
    delete cur;
    ds.reset();
  }
	
  void
  PReCoBAB::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
    d   = 0;
  }
	
  bool 
  PReCoBAB::searchStep(Space* &s1) {
  //cerr << "searching one step." << std::endl;
		//while (cur) {
		if(cur != NULL) {
		  if(constrained) {
				
				s1 = cur;
				constrained = false;
				return false;
			}
			//cerr << "entries in the stack: " << ds.entries() << endl;
			//cout << "propagate" << endl;
	unsigned int alt;
	switch (cur->status(alt,propagate)) {
	case SS_FAILED:
		//cout << "failed" << endl;
	  fail++;
	  delete cur; 
	  cur = NULL;
	  ParallelStatistics::current(NULL);
	  return false;
	  break;
	case SS_SOLVED:
	  {
			//cout << "solved" << ds.entries() << "entries..." <<endl;
			//s = SEARCH_DONE;
			s1= cur->clone(true, propagate);
			delete cur;
			cur = NULL;
			ParallelStatistics::current(NULL);
	    return true;
	  }
	case SS_BRANCH: 
	  {
			//cout << "branching " << endl;
	    Space* c;
	    if ((d == 0) || (d >= c_d)) {
	      clone++;
				//cout << "cloning" << cur->status(alt, propagate) << SS_BRANCH<< endl;
	      c = cur->clone();
	      d = 1;
				//cout << "done cloning" << endl;
	    } else {
	      c = NULL;
	      if (alt > 1)
		    d++;
	    }
			//cout << "pushing" << endl;
	    BranchingDesc* desc = ds.push(cur,c,alt);
			ParallelStatistics::push(c, desc);
			//cout << "done pushing" << endl;
	    commit++;
	    cur->commit(0, desc);
			return false;
	    break;
	  }
	}
	//cout << "done prop" << endl;
	} else  {
			//cout << "nothing to work on" << endl;
      if (!ds.next(*dynamic_cast<Search::FullStatistics*>(this))){
				//cout << "search done" << endl;
				s = SEARCH_DONE;
				return false;
			}
				//cout << "recomputing" << endl;
				cur = ds.recompute(d,*dynamic_cast<Search::FullStatistics*>(this));
			 
				ParallelStatistics::current(cur);
			  	if (ds.entries() <= mark) {
	  				// Next space needs to be constrained
	  				mark = ds.entries();
	  				d  = 0; // Force copy!
	  				s1 = cur;
					}
			//cout << "done recomputing " << endl;
			//Solution finiding should be done in text step.
			return false;
    }
		////cout << "test" << endl;
    //s = SEARCH_DONE;
		return false;
	}
	
	void
	PReCoBAB::newRoot(Space* t) {
		s = SEARCH_WORKING;
		unsigned int alt;
		long unsigned int p=0;
		//cerr << "new root 2" << endl;
		Space* c = (t->status(alt,p) == SS_FAILED) ? NULL : t->clone(false);
		
		cur= c;
		//delete t;
		//if(cur == NULL) s = SEARCH_DONE;
		propagate += p;
		
	}
	
	void PReCoBAB::newRoot(Space* t, unsigned int &alt, void* option) {
		s = SEARCH_WORKING;
		commit++;
		//Space* tmp = t->clone(false);
		unsigned int a=alt;
		long unsigned int p=0;
		if(t->status(a,p) == SS_FAILED) {
			
			fail++;
			return;	
		}
		else if(t->status(a,p) == SS_SOLVED){ cerr << "share is a solution" << endl;
			cur=t->clone(false);
			clone++;
		} else {
			//cerr << "commiting to share: " << alt << endl;
			if(t != NULL) {
			//cerr << "t is null" << (t == NULL) << endl;
		
				t->commit(alt, NULL, propagate);
				//cerr << "commit done" << endl;
				newRoot(t);
			}
	}
	propagate += p;
	}
	
	bool
	PReCoBAB::share(Space*& ret, unsigned int &alt, void* option) {
		  //cout << "in share function" << endl;
			if(ds.entries() > 3) {
				//cout << "has enough to share..." << endl;

				Search::ReCoNode* n = NULL;
				unsigned int pos = 0;
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
						while(loop == 1 && pos < (ds.entries()-1)) {
							if(ds[pos].rightmost()){
						////cout << "right most at pos " << pos << endl;
								pos += c_d;
							}	else{
								n = &ds[pos];
								loop = 0;
							}
						}						
						break;
					SHARE_TOP:
					default:
						while(loop == 1 && pos < (ds.entries()-1)) {
							if(ds[pos].rightmost()){
						////cout << "right most at pos " << pos << endl;
								pos++;
							}	else{
								n = &ds[pos];
								loop = 0;
							}
						}
				break;
			}
				//cout << "done searching found at pos " << pos << "of" << ds.entries() << endl;
				if(loop == 1||n == NULL) return NULL;
				//got something to share.
				//cout << "finding space for recomputation" << endl;
				unsigned int p = pos;

				Space* q = NULL;
				if(n->space() == NULL) {
					//Need to recompute the space. Spacehis is done by searching for a space
					//earlier in the stack than pos, which has a space, clone that space.
					//Spacehen recompute based on that space.
					while(!ds[pos].space()) pos--;
					//After this loop finishes we should have a pos where there is a space.
					//cout << "space found at " << pos << " is NULL " << (ds[pos].space() == NULL) << endl;
					q = ds[pos].space()->clone(false);
					clone++;
					//q->commit(n->alt(), n->desc(), propagate);
					//Need to recompute the space. recompute the space up to point p (the node we want to share).
					for(unsigned int i=pos; i<p; i++) {
						commit++;
						q->commit(ds[i].alt(), ds[i].desc(), propagate);
					}

				}
				else {
					q = n->space()->clone(false);
					clone++;
				}
				//q->constrain(best);
				//if(n->space()) n->space(NULL);
				alt = n->share();

				//Commiting to the alternative we share.
				option = n->desc();
				
				ret = q;
				
				if(pos <= mark) return true;
					else return false;
				//cout << "done com" << endl;

				//cout << "sharing a space." << endl;

				/*if(ds.next()) {
				Space* t = ds.recompute(d, *this);
				return t;
				}lse return NULL;*/
				
				return false;
			}
			ret = NULL;
			return false;
	}
	
	SearchStatus
	PReCoBAB::status() const {
		return s;
	}
	
	ParallelStatistics
	PReCoBAB::statistics() const {
		////cout <<  this->propagate << "stat test" << endl;
		return *this;
	}
	
	void
	PReCoBAB::clear() {
		cur = NULL;
	}
		void
	PReCoBAB::setMark() {
		mark = ds.entries();
	}
		void
	PReCoBAB::setCurrent(Space* t) {
		cur = t;
		constrained = true;
		ParallelStatistics::current(cur);
	}
}
}
