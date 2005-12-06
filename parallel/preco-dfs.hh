/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: parallel-dfs.cc
 * Description:
 * This is the header file for a script used to run recomputation DFS in parallel with the
 * parallel prototype.
 *
 * History:
 * 2005-03-09: Morten Nielsen
 * This file is created, largely based on Christian Schultes search engine in
 * search/dfs-reco.hh
 * 2005-03-11: Morten : Added status member-function
 * 2005-05-07: Morten Nielsen: Added ShareStrategy as an option on creation.
 */


#ifndef __GECODE_SEARCH_PARALLEL_DFS_RECO_HH__
#define __GECODE_SEARCH_PARALLEL_DFS_RECO_HH__

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
  class PReCoDFS : public ParallelEngine {
  private:
		Space* cur;
    Search::ReCoStack          ds;
	  SearchStatus s;
    const unsigned int c_d;
    unsigned int       d;

	  ShareStrategy* options;
  public:
		void clear();
    PReCoDFS(unsigned int,unsigned int, ShareStrategy* o=NULL);
	   SearchStatus status() const;
	   void newRoot(Space*);
	   void newRoot(Space*, unsigned int&, void*);
		Space* share(unsigned int&, void*);
  	void reset(Space*);
    ~PReCoDFS(void);
    Space* searchStep(void);
	  ParallelStatistics statistics() const;
		void setCurrent(Space*);
  };

#include "parallel/preco-dfs.icc"
}


}
#endif
