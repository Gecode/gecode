/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: pcopy-dfs.hh
 * Description:
 * This is the header file for a script used to run DFS in parallel with the
 * parallel prototype.
 *
 * History:
 * 2005-09-03: Morten Nielsen
 * This file is created, largely based on preco-dfs.hh
 */


#ifndef __GECODE_SEARCH_PARALLEL_BAB_RECO_HH__
#define __GECODE_SEARCH_PARALLEL_BAB_RECO_HH__

#include "parallel.hh"	
#include "search.hh"
#include "search/stack.hh"

//using namespace Search;
namespace Gecode {
namespace Parallel{

  /*
   * The DFS engine for recomputation
   *
   */
  class PReCoBAB : public ParallelBABEngine {
  private:
		Space* cur;
    Search::ReCoStack          ds;
	  SearchStatus s;
		const unsigned int c_d;
		unsigned int       d;
	  ShareStrategy* options;
	  unsigned int mark;
		bool constrained;

  public:
		void clear();
    PReCoBAB(unsigned int, unsigned int,ShareStrategy* o=NULL);
	   SearchStatus status() const;
	   void newRoot(Space*);
	   void newRoot(Space*, unsigned int&, void*);
				bool share(Space*&, unsigned int&, void*);
  	void reset(Space*);
    ~PReCoBAB(void);
    bool searchStep(Space*&);
	  ParallelStatistics statistics() const;
	  	  void setMark(void);
		void setCurrent(Space*);
  };

}


}
#endif
