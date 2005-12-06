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


#ifndef __GECODE_SEARCH_PARALLEL_BAB_COPY_HH__
#define __GECODE_SEARCH_PARALLEL_BAB_COPY_HH__

#include "parallel.hh"	
#include "search.hh"
#include "search/stack.hh"

//using namespace Search;
namespace Gecode {
namespace Parallel{

  /*
   * Spacehe DFS engine for recomputation
   *
   */
  class PCopyBAB : public ParallelBABEngine {
  private:
		Space* cur;
    Search::CopyStack          ds;
	  SearchStatus s;
		unsigned int mark;
	  unsigned int n_alt;
	  bool constrained;
	  ShareStrategy* options;
  public:
		void clear();
    PCopyBAB(ShareStrategy* o=NULL);
	   SearchStatus status() const;
	   void newRoot(Space*);
	   void newRoot(Space*, unsigned int&, void*);
		bool share(Space*&, unsigned int&, void*);
  	void reset(Space*);
    ~PCopyBAB(void);
    bool searchStep(Space*&);
	  ParallelStatistics statistics() const;
	  void setMark(void);
		void setCurrent(Space*);
	 
  };

}


}
#endif
