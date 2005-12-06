#ifndef __GECODE_PARALLEL_BAB_HH_
#define __GECODE_PARALLEL_BAB_HH_
namespace Gecode {
namespace Parallel {
	template<class T>
		class PBAB {
			private:
				T* best;
				ParallelBABEngine* s;
			public:
		 Worker<T, PBAB<T> >** init(int,pthread_barrier_t*, Controller<T, PBAB<T> >*, pthread_cond_t &w, T* r, ShareStrategy* o, int, int);
		 PBAB(unsigned int, unsigned int, ShareStrategy* o=NULL);
	   void newRoot(T*);
	   void newRoot(T*, unsigned int&, void*);
	   void solution(T*);
		 T* share(unsigned int&, void*);
  	 void reset(T*);
     T* searchStep(void);
	   SearchStatus status() const ;
	   void clear();
		 ParallelStatistics statistics();
		 bool bab();
			void setCurrent(T*);
		};
		
 template <class T>
	Worker<T, PBAB<T> >** PBAB<T>::init(int n_workers,pthread_barrier_t* start, Controller<T, PBAB<T> >* cnt, pthread_cond_t &cond_w, T* root, ShareStrategy* o, int a_d, int c_d) {
	
  Worker<T, PBAB<T> >** workers = new Worker<T, PBAB<T> >* [n_workers];
 
		for(int i=1; i < n_workers; i++) {
     workers[i] = new Worker<T, PBAB<T> >(cnt, start, cond_w, c_d, a_d,o);
   }
	
	 //We give the initial space to the first worker.
   workers[0] = new Worker<T, PBAB<T> >(cnt, start, cond_w, c_d, a_d, root, o);
	
	 return workers;
	};
	
		template<class T>
			PBAB<T>::PBAB(unsigned int c_d, unsigned int a_d, ShareStrategy* o) : best(NULL) {
				if(c_d == 1) {
					s = new PCopyBAB(o);
				}else {
					s = new PReCoBAB(c_d, a_d, o);
				}
			};
	
		template<class T>
			void PBAB<T>::newRoot(T* r) {
				if(r == NULL) return;
				if(best) r->constrain(best);
				s->newRoot(r);
				//s->setMark();
			}
		
		template<class T>
			void PBAB<T>::newRoot(T* r, unsigned int &alt, void* p) {
				if(r == NULL) return;
				//cerr << "new root" << endl;
				if(best) r->constrain(best);
				s->newRoot(r, alt, p);
				//s->setMark();
			}
						
			
		//template<class T>
		//	PBAB::~PBAB() {
		//		delete s;
		//		delete best;
		//}
		
		template<class T>
			SearchStatus PBAB<T>::status() const {
				return s->status();
			}
			
		template<class T>
			void PBAB<T>::clear() {
				s->clear();
			}
		template<class T>
			T* PBAB<T>::searchStep() {
				Space* s1=NULL;
				//cerr << "search step pbab"<< endl;
				if(!s->searchStep(s1)){
					if(s1 != NULL && best != NULL) {
						
						static_cast<T*>(s1)->constrain(best);
						//cerr << "constrining" << endl;
						//r->constrain(best);
						//s->reset(r);
						//delete r;
						//delete s1;
					}
				}else if(s1 != NULL){
					//delete best;
					//best = static_cast<T*>(s1->clone(false));
					//s->reset(NULL);
					T* r = static_cast<T*>(s1);
					if(best != NULL) r->constrain(best);
					return r;
				}
				//cout << "search step pbab done" << endl;
				return NULL;
			}
		template<class T>
			T* PBAB<T>::share(unsigned int &alt, void* option) {
				
				T* result = NULL;
				Space* r=NULL;
				if(s->share(r,alt, option)) {
				//cerr << "sharing" << endl;
				if (r != NULL) {
					//cerr << "sharing constrain" << endl;
					result = static_cast<T*>(r);
					if(best != NULL) result->constrain(best);
				}} else {
					result = static_cast<T*>(r);
				}
				return result;
			}
		template<class T>
			ParallelStatistics PBAB<T>::statistics() {
				return s->statistics();
			}
		template<class T>
			void PBAB<T>::reset(T* t) {
				return s->reset(t);
			}
		template<class T>
			void PBAB<T>::solution(T* t) {
				if(t != NULL) {
					//cerr << "new best" << endl;
					delete best;
					best = static_cast<T*>(t->clone(false));
					s->setMark();
				}
			}
		template<class T>
			bool PBAB<T>::bab() {
				return true;
			}

			template<class T>
			void PBAB<T>::setCurrent(T* t) {
				s->setCurrent(t);
			}
			
		}
};
#endif
