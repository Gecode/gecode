#ifndef __GECODE_PARALLEL_DFS_HH_
#define __GECODE_PARALLEL_DFS_HH_
namespace Gecode {
namespace Parallel {
	template<class T>
		class PDFS {
			private:
				ParallelEngine* s;
			public:
		 Worker<T, PDFS<T> >** init(int,pthread_barrier_t*, Controller<T, PDFS<T> >*, pthread_cond_t &w, T* r, ShareStrategy* o, int, int);
		 PDFS(unsigned int, unsigned int, ShareStrategy* o=NULL);
	   void newRoot(T*);
	   void newRoot(T*, unsigned int&, void*);
	   void solution(T*) {};
		 T* share(unsigned int&, void*);
  	 void reset(T*);
     T* searchStep(void);
	   SearchStatus status() const ;
	   void clear();
		 ParallelStatistics statistics();
		 bool bab();
		 void setCurrent(Space*);
		};
		
 template <class T>
	Worker<T, PDFS<T> >** PDFS<T>::init(int n_workers,pthread_barrier_t* start, Controller<T, PDFS<T> >* cnt, pthread_cond_t &cond_w, T* root, ShareStrategy* o, int a_d, int c_d) {
	
  Worker<T, PDFS<T> >** workers = new Worker<T, PDFS<T> >* [n_workers];
 
		for(int i=1; i < n_workers; i++) {
     workers[i] = new Worker<T, PDFS<T> >(cnt, start, cond_w, c_d, a_d,o);
   }
	
	 //We give the initial space to the first worker.
   workers[0] = new Worker<T, PDFS<T> >(cnt, start, cond_w, c_d, a_d, root, o);
	
	 return workers;
	};
	
		template<class T>
			PDFS<T>::PDFS(unsigned int c_d, unsigned int a_d, ShareStrategy* o) {
				if(c_d == 1) {
					s = new PCopyDFS(o);
				}else {
					s = new PReCoDFS(c_d, a_d, o);
				}
			};
	
		template<class T>
			void PDFS<T>::newRoot(T* r) {
				s->newRoot(r);
			}
		
		template<class T>
			void PDFS<T>::newRoot(T* r, unsigned int &alt, void* p) {
				s->newRoot(r, alt, p);
			}
						
			
		//template<class T>
		//	PDFS::~PDFS() {
		//		delete s;
		//}
		
		template<class T>
			SearchStatus PDFS<T>::status() const {
				return s->status();
			}
			
		template<class T>
			void PDFS<T>::clear() {
				s->clear();
			}
		template<class T>
			T* PDFS<T>::searchStep() {
				return static_cast<T*>(s->searchStep());
			}
		template<class T>
			T* PDFS<T>::share(unsigned int &alt, void* option) {
				return static_cast<T*>(s->share(alt, option));
			}
		template<class T>
			ParallelStatistics PDFS<T>::statistics() {
				return s->statistics();
			}
		template<class T>
			void PDFS<T>::reset(T* t) {
				return s->reset(t);
			}
		template<class T>
			bool PDFS<T>::bab() {
				return false;
			}
		template<class T>
			void PDFS<T>::setCurrent(Space* t) {
				s->setCurrent(t);
			}

		}
};
#endif
