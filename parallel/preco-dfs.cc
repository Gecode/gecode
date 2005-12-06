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

  PReCoDFS::PReCoDFS(unsigned int c_d0, unsigned int a_d0, ShareStrategy* o) 
  : cur(NULL),ds(a_d0), s(SEARCH_DONE), c_d(c_d0), d(0), options(o)  { //cout << propagate << "stat test" << endl;
		}

		
  PReCoDFS::~PReCoDFS(void) {
    delete cur;
    ds.reset();
  }

  void
  PReCoDFS::reset(Space* s) {
    delete cur;
    ds.reset();
    cur = s;
    d   = 0;
  }

  Space* 
  PReCoDFS::searchStep(void) {
  //cout << "searching one step." << std::endl;
		//while (cur) {
		if(cur != NULL) {
			//cout << "entries in the stack: " << ds.entries() << endl;
			//cout << "propagate" << endl;
	unsigned int alt;
	switch (cur->status(alt,propagate)) {
	case SS_FAILED:
		//cout << "failed" << endl;
	  fail++;
	  delete cur; 
	  cur = NULL;
	  ParallelStatistics::current(NULL);
	  return NULL;
	  break;
	case SS_SOLVED:
	  {
			//cout << "solved" << ds.entries() << "entries..." <<endl;
			//s = SEARCH_DONE;
			Space* t = cur;
			cur = NULL;
			ParallelStatistics::current(NULL);
	    return t;
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
			return NULL;
	    break;
	  }
	}
	//cout << "done prop" << endl;
	} else  {
			//cout << "nothing to work on" << endl;
      if (!ds.next(*dynamic_cast<Search::FullStatistics*>(this))){
				//cout << "search done" << endl;
				s = SEARCH_DONE;
				return NULL;
			}
				//cout << "recomputing" << endl;
				cur = ds.recompute(d,*dynamic_cast<Search::FullStatistics*>(this));
			  ParallelStatistics::current(cur);
			
			//cout << "done recomputing " << endl;
			//Solution finiding should be done in text step.
			return NULL;
    }
		////cout << "test" << endl;
    //s = SEARCH_DONE;
		return NULL;
	}
	
	void
	PReCoDFS::newRoot(Space* t) {
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
	
	void PReCoDFS::newRoot(Space* t, unsigned int &alt, void* option) {
		commit++;
		//t->commit(alt, static_cast<BranchingDesc*>(option), propagate);
		t->commit(alt, NULL, propagate);
		//cout << "commiting to share" << endl;
		newRoot(t);
	}

	Space*
	PReCoDFS::share(unsigned int &alt, void* option) {
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
					//Need to recompute the space. This is done by searching for a space
					//earlier in the stack than pos, which has a space, clone that space.
					//Then recompute based on that space.
					while(!ds[pos].space()) pos--;
					//After this loop finishes we should have a pos where there is a space.
					//cout << "space found at " << pos << " is NULL " << (ds[pos].space() == NULL) << endl;
					q = ds[pos].space()->clone(false);
					//q->commit(n->alt(), n->desc(), propagate);
					//Need to recompute the space. recompute the space up to point p (the node we want to share).
					for(unsigned int i=pos; i<p; i++) {
						commit++;
						q->commit(ds[i].alt(), ds[i].desc(), propagate);
					}

				}
				else q = n->space()->clone(false);
				Space* t = NULL;
				if(!n->rightmost()) {
				clone++;
				t = q; //Clones the space, without sharing variables.
			  //if(n->space() == NULL) delete q;

				//cout << "commiting" << endl;

				alt = n->share();

				//Commiting to the alternative we share.
				option = n->desc();
				//cout << "commiting to " << salt << endl;
				}
				//if(n->space()) n->space(NULL);

				//cout << "done com" << endl;

				//cout << "sharing a space." << endl;

				/*if(ds.next()) {
				Space* t = ds.recompute(d, *this);
				return t;
				}lse return NULL;*/
				
				return q;
			}
			return NULL;
	}
	
	SearchStatus
	PReCoDFS::status() const {
		return s;
	}

	ParallelStatistics
	PReCoDFS::statistics() const {
		////cout <<  this->propagate << "stat test" << endl;
		return *this;
	}
	
	void
	PReCoDFS::clear() {
		cur = NULL;
	}
	
	void
	PReCoDFS::setCurrent(Space* t) {
		cur = t;
		ParallelStatistics::current(cur);
	}
}
}
