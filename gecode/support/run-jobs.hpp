/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */


namespace Gecode { namespace Support {

  /// Baseclass for jobs with return type \a RetType
  template<class RetType>
  class Job {
  public:
    /// Run a job with iterator index \a i
    virtual RetType run(int i) = 0;
    /// Destructor
    virtual ~Job(void) {}
  };

  /// Class to throw an exception to stop new jobs from being started
  template<class RetType>
  class JobStop {
  protected:
    /// The result stored
    RetType r;
  public:
    /// Constructor
    JobStop(RetType r0);
    /// Return the passed result
    RetType result(void) const;
  };

  template<class Jobs, class RetType>
  /**
   * \brief Parallel iterator that runs jobs with a given number of threads
   *
   * It takes an iterator over jobs as input and acts as an iterator
   * over the jobs' results. The order of iteration is not kept. The
   * iterator runs several jobs as defined by the input iterator in
   * parallel using a maximal number of threads.
   *
   * The iterator can be stopped creating new jobs if one of the running
   * jobs throws an exception of type \a JobStop. In that case the result
   * of the stopped job is defined by the default constructor of the
   * return type \a RetType. Already running jobs are not stopped and
   * deliver their results as usual.
   *
   */
  class RunJobs {
  protected:
    class Master;
    /// The actual worker using a thread to run a job
    class Worker : public Runnable {
    protected:
      /// The job to run
      Job<RetType>* job;
      /// The master to communicate with
      Master* master;
      /// Original iterator index of job
      int idx;
    public:
      /// Initialize worker
      Worker(Job<RetType>* j, Master* m, int i);
      /// Run jobs
      virtual void run(void);
      /// Nothing to delete (done in run)
      virtual ~Worker(void);
    };
    class Master {
    protected:
      /// Mutex for synchronizing access
      Mutex m;
      /// Event is triggered if a the first job is added to queue
      Event e;
      /// Number of threads currently not in use
      unsigned int n_threads;
      /// Input jobs
      Jobs& jobs;
      /// Index of next job to be created
      int idx;
      /// Result from a the first stopped job (passed in exception)
      RetType sres; 
      /// Index of the first stop that has been stopped (-1 if none)
      int sidx;
      /// Queue of not yet requested results
      DynamicQueue<RetType,Heap> rs;
    public:
      /// Get next job witth index \a i, if possible
      Job<RetType>* next(int& i);
      /// Report result \a r by a worker
      void report(RetType r);
      /// Report that a job with index \a i has been stopped
      void stop(RetType r, int i);
      /// Test whether all jobs are done
      bool done(void) const;
      /// Initialize with job iterator \a j and maximal number of threads \a m
      Master(Jobs& j, unsigned int m);
      /// Run next job and return true if succesful and assign \a r to its result
      bool run(RetType& r);
      /// Whether a job has thrown a \a JobStop exception
      bool stopped(void) const;
      /// Return index of first job that has thrown a \a JobStop exception (-1 if none) with its result
      int stoppedjob(RetType& r) const;
      /// Whether a new thread is needed for deletion
      bool needthread(void);
      /// Destructor
      ~Master(void);
    };
    /// A class to delete the master (running in parallel)
    class Deleter : public Runnable {
    protected:
      /// The master to be deleted
      Master* master;
    public:
      /// Initialize with master \a m
      Deleter(Master* m);
      /// Perform deletion
      virtual void run(void);
    };
    /// The actual master
    Master* master;
  public:
    /// Initialize with job iterator \a j and maximal number of threads \a m
    RunJobs(Jobs& j, unsigned int m);
    /// Run next job and return true if succesful and assign \a r to its result
    bool run(RetType& r);
    /// Whether a job has thrown a \a JobStop exception with index \a i and result \a r
    bool stopped(int& i, RetType& r) const;
    /// Destructor
    ~RunJobs(void);
  };



  template<class RetType>
  forceinline
  JobStop<RetType>::JobStop(RetType r0) : r(r0) {}

  template<class RetType>
  forceinline RetType
  JobStop<RetType>::result(void) const {
    return r;
  }



  template<class Jobs, class RetType>
  forceinline
  RunJobs<Jobs,RetType>::Worker::Worker(Job<RetType>* j, 
                                        Master* m,
                                        int i) 
    : Runnable(true), job(j), master(m), idx(i) {}
  
  template<class Jobs, class RetType>
  RunJobs<Jobs,RetType>::Worker::~Worker(void) {}


  template<class Jobs, class RetType>
  inline int
  RunJobs<Jobs,RetType>::Master::stoppedjob(RetType& r) const {
    r = sres;
    return sidx;
  }

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::Master::stopped(void) const {
    return sidx >= 0;
  }

  template<class Jobs, class RetType>
  forceinline Job<RetType>*
  RunJobs<Jobs,RetType>::Master::next(int& i) {
    m.acquire();
    Job<RetType>* j;
    if (jobs() && !stopped()) {
      j = jobs.job(); i=idx++;
    } else {
      j = NULL;
      n_threads--;
      e.signal();
    }
    m.release();
    return j;
  }

  template<class Jobs, class RetType>
  forceinline void
  RunJobs<Jobs,RetType>::Master::report(RetType r) {
    m.acquire();
    rs.push(r); 
    e.signal();
    m.release();
  }

  template<class Jobs, class RetType>
  forceinline void
  RunJobs<Jobs,RetType>::Master::stop(RetType r, int i) {
    m.acquire();
    if (!stopped()) {
      sres=r; sidx = i;
    }
    rs.push(r); 
    e.signal();
    m.release();
  }

  template<class Jobs, class RetType>
  void
  RunJobs<Jobs,RetType>::Worker::run(void) {
    do {
      try {
        RetType r = job->run(idx);
        master->report(r);
      } catch (JobStop<RetType>& js) {
        master->stop(js.result(),idx);
      }
      delete job;
      job = master->next(idx);
    } while (job != NULL);
  }

  template<class Jobs, class RetType>
  forceinline bool
  RunJobs<Jobs,RetType>::Master::done(void) const {
    return (n_threads == 0) && (!jobs() || stopped()) && rs.empty();
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::Master::Master(Jobs& j, unsigned int m_threads) 
    : n_threads(0), jobs(j), idx(0), sidx(-1), rs(heap) {
    m.acquire();
    while ((n_threads < m_threads) && jobs()) {
      if (stopped())
        break;
      Thread::run(new Worker(jobs.job(),this,idx));
      n_threads++; idx++;
    }
    m.release();
  }

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::Master::run(RetType& r) {
    m.acquire();
    if (done()) {
      m.release();
      return false;
    }
    if (!rs.empty()) {
      r = rs.pop();
      m.release();
      return true;
    }
    m.release();
    while (true) {
      e.wait();
      m.acquire();
      if (done()) {
        m.release();
        return false;
      }
      if (!rs.empty()) {
        r = rs.pop();
        m.release();
        return true;
      }
      m.release();
    }
    GECODE_NEVER;
  }

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::Master::needthread(void) {
    bool n;
    m.acquire();
    while (!rs.empty())
      rs.pop().~RetType();
    sidx = 0;
    n = !done();
    m.release();
    return n;
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::Master::~Master(void) {
    sidx = 0;
    RetType r;
    while (run(r))
      r.~RetType();
  }

  template<class Jobs, class RetType>
  forceinline
  RunJobs<Jobs,RetType>::Deleter::Deleter(Master* m) 
    : Runnable(true), master(m) {}

  template<class Jobs, class RetType>
  void
  RunJobs<Jobs,RetType>::Deleter::run(void) {
    delete master;
  }





  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::stopped(int& i, RetType& r) const {
    i = master->stoppedjob(r);
    return i >= 0;
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::RunJobs(Jobs& j, unsigned int m) 
    : master(new Master(j,m)) {}

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::run(RetType& r) {
    return master->run(r);
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::~RunJobs(void) {
    if (!master->needthread())
      delete master;
    else
      Thread::run(new Deleter(master));
  }


}}

// STATISTICS: support-any

