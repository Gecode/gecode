/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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
  };

  /// Class to throw an exception to stop new jobs from being started
  class JobStop {};

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
    /// The actual worker using a thread to run a job
    class Worker : public Runnable {
    protected:
      /// The job to run
      Job<RetType>* job;
      /// The master to communicate with
      RunJobs* master;
      /// Original iterator index of job
      int idx;
    public:
      /// Initialize worker
      Worker(Job<RetType>* j, RunJobs* m, int i);
      /// Run a job
      virtual void run(void);
      /// Nothing to delete (done in run)
      virtual ~Worker(void);
    };
    /// Mutex for synchronizing access
    Mutex m;
    /// Event is triggered if a the first job is added to queue
    Event e;
    /// Maximal number of threads to use
    unsigned int m_threads;
    /// Number of threads currently not in use
    unsigned int n_threads;
    /// Input jobs
    Jobs& jobs;
    /// Index of next job to be created
    int idx;
    /// Index of the first stop that has been stopped (-1 if none)
    int sidx;
    /// Queue of not yet requested results
    DynamicQueue<RetType,Heap> rs;
    /// Create new parallel workers if needed
    void workers(void);
    /// Report result \a r by a worker
    void report(RetType r);
    /// Report that a job with index \a i has been stopped
    void stop(int i);
    /// Test whether all jobs are done
    bool done(void) const;
  public:
    /// Initialize with job iterator \a j and maximal number of threads \a m
    RunJobs(Jobs& j, unsigned int m);
    /// Test whether there are more jobs to run
    bool operator ()(void);
    /// Return result (and possibly run) next job
    RetType run(void);
    /// Whether a job has thrown a \a JobStop exception
    bool stopped(void) const;
    /// Return index of first job that has thrown a \a JobStop exception (-1 if none)
    int stoppedjob(void) const;
    /// Destructor
    ~RunJobs(void);
  };



  template<class Jobs, class RetType>
  forceinline
  RunJobs<Jobs,RetType>::Worker::Worker(Job<RetType>* j, 
                                        RunJobs<Jobs,RetType>* m,
                                        int i) 
    : Runnable(true), job(j), master(m), idx(i) {}
  
  template<class Jobs, class RetType>
  RunJobs<Jobs,RetType>::Worker::~Worker(void) {}


  template<class Jobs, class RetType>
  inline int
  RunJobs<Jobs,RetType>::stoppedjob(void) const {
    return sidx;
  }

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::stopped(void) const {
    return sidx >= 0;
  }

  template<class Jobs, class RetType>
  inline void
  RunJobs<Jobs,RetType>::workers(void) {
    if (stopped())
      return;
    while ((n_threads < m_threads) && jobs()) {
      Thread::run(new Worker(jobs.job(),this,idx));
      n_threads++; idx++;
    }
  }

  template<class Jobs, class RetType>
  forceinline void
  RunJobs<Jobs,RetType>::report(RetType r) {
    m.acquire();
    rs.push(r); 
    n_threads--;
    e.signal();
    workers();
    m.release();
  }

  template<class Jobs, class RetType>
  forceinline void
  RunJobs<Jobs,RetType>::stop(int i) {
    m.acquire();
    sidx = i;
    RetType r;
    rs.push(r); 
    n_threads--;
    e.signal();
    m.release();
  }

  template<class Jobs, class RetType>
  void
  RunJobs<Jobs,RetType>::Worker::run(void) {
    try {
      RetType r = job->run(idx);
      master->report(r);
    } catch (JobStop&) {
      master->stop(idx);
    }
    delete job;
  }

  template<class Jobs, class RetType>
  forceinline bool
  RunJobs<Jobs,RetType>::done(void) const {
    return (n_threads == 0) && (!jobs() || stopped()) && rs.empty();
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::RunJobs(Jobs& j, unsigned int m) 
    : m_threads(m), n_threads(0), jobs(j), idx(0), sidx(-1), rs(heap) {
    this->m.acquire();
    workers();
    this->m.release();
  }

  template<class Jobs, class RetType>
  inline bool
  RunJobs<Jobs,RetType>::operator ()(void) {
    m.acquire();
    bool r = !done();
    m.release();
    return r;
  }

  template<class Jobs, class RetType>
  inline RetType
  RunJobs<Jobs,RetType>::run(void) {
    m.acquire();
    assert(!done());
    if (!rs.empty()) {
      RetType r = rs.pop();
      m.release();
      return r;
    }
    m.release();
    while (true) {
      e.wait();
      m.acquire();
      if (!rs.empty()) {
        RetType r = rs.pop();
        m.release();
        return r;
      }
      m.release();
    }
    GECODE_NEVER;
  }

  template<class Jobs, class RetType>
  inline
  RunJobs<Jobs,RetType>::~RunJobs(void) {
    sidx = 0;
    while ((*this)())
      this->run().~RetType();
  }

}}

// STATISTICS: support-any

