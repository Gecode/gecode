/*
 * Author: Morten Nielsen
 * Date: 2005-03-09
 * File: gecode-parallel.hh
 * Description:
 * This is the main header file for the parallel engine for Gecode.
 *
 * History:
 * 2005-02-10: Morten Nielsen : This file is created
 * 2005-02-15: Morten Nielsen
 * Added node, stack, and parallel-dfs to be included. Also fixed #idndef stuff.
 * 2005-03-09: Morten Nielsen
 * 				This file is created from the demonstration version. Slight changes in
 *				how files are combined, but the classes are the same. First step in
 *			  combining gecode with the parallel engine.
 * 2005-05-07: Morten Nielsen
 *             The stop function now takes the new solution as input, in
 *             preparation for bestsolution search. Best solution search
 *             now only needs the searchscript. Also ShareStrategy is added.
 */

#ifndef _GECODE_PARALLEL_HH_
#define _GECODE_PARALLEL_HH_

#include "search.hh"
#include <pthread.h>
namespace Gecode {
namespace Parallel {

	enum ShareOptions {
		SHARE_TOP,
		SHARE_BOTTOM,
		SHARE_RECOMP,
	};
	
 class ShareStrategy{
	 public:
	 //Shareoptions should tell the searchscript how to find nodes to share.
		 ShareOptions search;
	 //These values assist the search in when to share.
	 unsigned int StackSize;
	 unsigned int LDSIncrement;
 };

enum SearchStatus {
	SEARCH_DONE,
	SEARCH_WORKING
};

template <class T, class SearchScript>
class Worker;
	
class ParallelStatistics : public Search::FullStatistics {
	public:
	unsigned int shares;
	unsigned int searches;
	ParallelStatistics(size_t sz);
	ParallelStatistics();
};

	class ParallelEngine : public ParallelStatistics {
		public:
		 ParallelEngine(ShareStrategy* o=NULL) {};
	   virtual void newRoot(Space*) {};
	   virtual void newRoot(Space*, unsigned int&, void*) {};
	   void solution(Space*) {};
		 virtual Space* share(unsigned int&, void*)=0;
  	 virtual void reset(Space*) {};
     virtual Space* searchStep(void)=0;
	   virtual SearchStatus status() const =0;
	   virtual void clear()=0;
	 	 virtual ParallelStatistics statistics() const=0;
			 virtual void setCurrent(Space*)=0;
	};
	
	class ParallelBABEngine : public ParallelStatistics {
		public:
		 ParallelBABEngine(ShareStrategy* o=NULL) {};
	   virtual void newRoot(Space*) =0;
	   virtual void newRoot(Space*, unsigned int&, void*)=0;
		 virtual 		bool share(Space*&, unsigned int&, void*)=0;
  	 virtual void reset(Space*) =0;
     virtual bool searchStep(Space*&)=0;
	   virtual SearchStatus status() const =0;
	   virtual void clear()=0;
	 	 virtual ParallelStatistics statistics() const=0;
		 virtual void setMark(void)=0;
		 virtual void setCurrent(Space*)=0;
	};
	
	/*template<class T>
		class ParallelEngine : public ParallelEngine {
		public:
		 ParallelEngine(ShareStrategy* o=NULL) {};
	   virtual void newRoot(Space*)=0;
	   virtual void newRoot(Space*, unsigned int&, void*)=0;
	   void solution(Space*) {};
		 virtual Space* share(unsigned int&, void*)=0;
  	 virtual void reset(Space*)=0;
     virtual Space* searchStep(void)=0;
	   virtual SearchStatus status() const =0;
	   virtual void clear()=0;
	 	 virtual ParallelStatistics statistics() const=0;
	};*/

forceinline
ParallelStatistics::ParallelStatistics(size_t sz) : FullStatistics(sz), shares(0), searches(0) {
};

forceinline
ParallelStatistics::ParallelStatistics(): FullStatistics(0), shares(0), searches(0) {
};

template <class T, class SearchScript>
class Controller : public ParallelStatistics {
  private:
    int n_workers;
    Worker<T, SearchScript>** workers;
    T* cur_sol;

		//Pthread stuff.
	pthread_barrier_t start;
	  pthread_mutex_t control;
	  pthread_mutex_t solution;
		pthread_cond_t cond_sol;
    ShareStrategy options;
  public:
		pthread_cond_t cond_workers;
  //Extra constructors to take care of the options give to the search engines.
    Controller(T* root, int n=2, ShareStrategy* o=NULL);
	  Controller(T* root, int c_d, int a_d, int n=2, ShareStrategy* o=NULL);
	
	  ~Controller();
	
	  int collect(T*);
    T* find(unsigned int &alt, void*); //Finds work for workers.

    T* next(); //Returns next solution. mutex lock wait for set, when solution.
	
	  ParallelStatistics statistics(void);
};

//template class Controller<PDFS>;

#include "parallel/controller.icc"

enum WorkerStatus {
	 WORKER_IDLE,
	 WORKER_WORKING,
 };


 template <class T, class SearchScript>
 class Worker{
  private:
		Controller<T, SearchScript>* control;
		
    WorkerStatus status;
    int pause;
	  int searching;
    T* _cur;
    //Pthread variables.
		pthread_barrier_t* start;
    pthread_attr_t tattr;
    pthread_t tid;
    pthread_mutex_t search;
	  pthread_mutex_t pause_m;

	
	SearchScript* se;
		static void * thread_entry(void *);
	
    void run(void);
	  void checkpause(void);
	  void find(void);
	  void initialize(void);
	//Need too add mutex lock for SearchScript access.
  public:
		pthread_cond_t cond_workers;
		Worker(Controller<T, SearchScript>* cnt, pthread_barrier_t*s, pthread_cond_t &t, ShareStrategy* o = NULL);
	  Worker(Controller<T, SearchScript>* cnt, pthread_barrier_t*s, pthread_cond_t &t, int c_d, int a_d, ShareStrategy* o = NULL);
	  Worker(Controller<T, SearchScript>* cnt, pthread_barrier_t*s, pthread_cond_t &t, int c_d, int a_d, T*, ShareStrategy* o = NULL);
    Worker(Controller<T, SearchScript>* cnt, pthread_barrier_t*s, pthread_cond_t &t, T*, ShareStrategy* o = NULL);
    ~Worker();
	  ParallelStatistics stats() const;
    T* share(unsigned int &alt, void* option); //Shares work with other workes, if possible.
    void stop(T*); //Lets the worker finish its current work but then stops.
		void explore(); //Tries to find the next solution. answer sendt with message.
    WorkerStatus getStatus() const;
 };

 #include "parallel/worker.icc"
};
}
#include "parallel/preco-dfs.hh"
#include "parallel/pcopy-dfs.hh"
#include "parallel/preco-bab.hh"
#include "parallel/pcopy-bab.hh"

namespace Gecode {
namespace Parallel {
template<class T>
 class RecoDfs : PReCoDFS{
	 public:
		Worker<T, RecoDfs<T> >** init(int,pthread_barrier_t*, Controller<T, RecoDfs<T> >*, pthread_cond_t &w, T* r, ShareStrategy* o, int, int);
		RecoDfs(unsigned int, unsigned int, ShareStrategy* o=NULL);
	   void newRoot(T*);
	   void newRoot(T*, unsigned int&, void*);
	   void solution(T*) {};
		 T* share(unsigned int&, void*);
  	 void reset(T*);
     T* searchStep(void);
	   SearchStatus status() const ;
	   void clear();
	 	ParallelStatistics statistics();
 };
 
 template <class T>
	Worker<T, RecoDfs<T> >** RecoDfs<T>::init(int n_workers,pthread_barrier_t* start, Controller<T, RecoDfs<T> >* cnt, pthread_cond_t &cond_w, T* root, ShareStrategy* o, int a_d, int c_d) {
	
  Worker<T, RecoDfs<T> >** workers = new Worker<T, RecoDfs<T> >* [n_workers];
 
		for(int i=1; i < n_workers; i++) {
     workers[i] = new Worker<T, RecoDfs<T> >(cnt, start, cond_w, c_d, a_d,o);
   }
	
	 //We give the initial space to the first worker.
   workers[0] = new Worker<T, RecoDfs<T> >(cnt, start, cond_w, c_d, a_d, root, o);
	
	 return workers;
	};
	
 template <class T>
	T* RecoDfs<T>::share(unsigned int &alt, void* options) {
		return static_cast<T*>(PReCoDFS::share(alt, options));
 };

 template <class T>
	T* RecoDfs<T>::searchStep() {
		return static_cast<T*>(PReCoDFS::searchStep());
 };

 template <class T>
 void RecoDfs<T>::reset(T* s) {
	 //PReCoDFS::reset(s);
 };

 template <class T>
 void RecoDfs<T>::newRoot(T* s) {
	 PReCoDFS::newRoot(s);
 };

 template <class T>
 void RecoDfs<T>::newRoot(T* s, unsigned int &a, void* o) {
	 PReCoDFS::newRoot(s, a, o);
 };

 template <class T>
 SearchStatus RecoDfs<T>::status() const {
	 return PReCoDFS::status();
 };

 template <class T>
 RecoDfs<T>::RecoDfs(unsigned int c_d, unsigned int a_d, ShareStrategy* o) : Parallel::PReCoDFS(c_d,a_d,o) {
};

template <class T>
	void RecoDfs<T>::clear() {
		PReCoDFS::clear();
	}
template <class T>
	ParallelStatistics RecoDfs<T>::statistics() {
		return *this;
	}
		


};

 
}
#include "parallel/pdfs.hh"
#include "parallel/pbab.hh"
#endif
