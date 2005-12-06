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


#ifndef __GECODE_SEARCH_PARALLEL_DFS_COPY_HH__
#define __GECODE_SEARCH_PARALLEL_DFS_COPY_HH__

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
  class PCopyDFS : public ParallelEngine {
  private:
		Space* cur;
    Search::CopyStack          ds;
	  SearchStatus s;

	  ShareStrategy* options;
  public:
		void clear();
    PCopyDFS(ShareStrategy* o=NULL);
	   SearchStatus status() const;
	   void newRoot(Space*);
	   void newRoot(Space*, unsigned int&, void*);
		Space* share(unsigned int&, void*);
  	void reset(Space*);
    ~PCopyDFS(void);
    Space* searchStep(void);
	  ParallelStatistics statistics() const;
		void setCurrent(Space*);
  };

}


}
#endif
