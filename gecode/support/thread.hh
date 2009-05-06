/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

#ifndef __GECODE_SUPPORT_THREAD_HH__
#define __GECODE_SUPPORT_THREAD_HH__

#include <gecode/support.hh>

#ifdef GECODE_THREADS_WINDOWS
#include <windows.h>
#endif

#ifdef GECODE_THREADS_PTHREADS
#include <pthread.h>
#endif

#ifdef GECODE_HAS_THREADS

/**
 * \defgroup FuncSupportThread Simple thread and synchronization support
 *
 * This is very simplistic, just enough for parallel search engines. Do
 * not mistake it for a full-fledged thread package.
 *
 * Requires \code #include <gecode/support/thread.hh> \endcode
 * 
 * If the platform supports threads, the macro GECODE_HAS_THREADS is defined.
 *
 * \ingroup FuncSupport
 */  

namespace Gecode { namespace Support {

  /**
   * \brief An interface for objects that can be run by a thread
   *
   * Requires \code #include <gecode/support/thread.hh> \endcode
   *
   * \ingroup FuncSupportThread
   */
  class Runnable {
  public:
    /// The function that is executed when the thread starts
    virtual void run(void) = 0;
    /// Destructor
    virtual ~Runnable(void) {}
    /// Allocate memory from heap
    static void* operator new(size_t s);
    /// Free memory allocated from heap
    static void  operator delete(void* p);
  };

  /**
   * \brief Simple threads
   *
   * Threads are assumed to properly terminate, the destructor will
   * only release the handle to a thread but will not terminate it.
   * The thread ceases when the call to run terminates, then the
   * runnable object passed will also be deleted.
   *
   * Requires \code #include <gecode/support/thread.hh> \endcode
   *
   * \ingroup FuncSupportThread
   */
  class Thread {
  private:
#ifdef GECODE_THREADS_WINDOWS
    /// The Windows specific handle to a thread
    HANDLE w_h;
#endif
#ifdef GECODE_THREADS_PTHREADS
    /// The Pthread specific thread datastructure
    pthread_t p_t;
#endif
  public:
    /**
     * \brief Construct a new thread and run \a r
     *
     * After \a r terminates, \a r is deleted. After that, the thread
     * terminates.
     */
    GECODE_SUPPORT_EXPORT Thread(Runnable* r);
    /// Destroy thread handle (does not terminate thread)
    ~Thread(void);
    /// Put current thread to sleep for \a ms milliseconds
    static void sleep(unsigned int ms);
    /// Return number of processing units (1 if information not available)
    static unsigned int npu(void);
  private:
    /// A thread cannot be copied
    Thread(const Thread&) {}
    /// A thread cannot be assigned
    void operator=(const Thread&) {}
  };


  /**
   * \brief A mutex for mutual exclausion among several threads
   * 
   * It is not specified whether the mutex is recursive or not.
   * Likewise, there is no guarantee of fairness among the
   * blocking threads.
   *
   * Requires \code #include <gecode/support/thread.hh> \endcode
   *
   * \ingroup FuncSupportThread
   */
  class Mutex {
  private:
#ifdef GECODE_THREADS_WINDOWS
    /// Use a simple but more efficient critical section on Windows
    CRITICAL_SECTION w_cs;
#endif
#ifdef GECODE_THREADS_PTHREADS
    /// The Pthread mutex
    pthread_mutex_t p_m;
#endif
  public:
    /// Initialize mutex
    Mutex(void);
    /// Acquire the mutex and possibly block
    void acquire(void);
    /// Try to acquire the mutex, return true if succesful
    bool tryacquire(void);
    /// Release the mutex
    void release(void);
    /// Delete mutex
    ~Mutex(void);
  private:
    /// A mutex cannot be copied
    Mutex(const Mutex&) {}
    /// A mutex cannot be assigned
    void operator=(const Mutex&) {}
  };

  /**
   * \brief A lock as a scoped frontend for a mutex
   *
   * Requires \code #include <gecode/support/thread.hh> \endcode
   *
   * \ingroup FuncSupportThread
   */
  class Lock {
  private:
    /// The mutex used for the lock
    Mutex& m;
  public:
    /// Enter lock
    Lock(Mutex& m0);
    /// Leave lock
    ~Lock(void);
  private:
    /// A lock cannot be copied
    Lock(const Lock& l) : m(l.m) {}
    /// A lock cannot be assigned
    void operator=(const Lock&) {}
  };

  /**
   * \brief An event for synchronization
   * 
   * An event can be waited on by a single thread until the event is
   * signalled.
   *
   * Requires \code #include <gecode/support/thread.hh> \endcode
   *
   * \ingroup FuncSupportThread
   */
  class Event {
  private:
#ifdef GECODE_THREADS_WINDOWS
    /// The Windows specific handle to an event
    HANDLE w_h;    
#endif
#ifdef GECODE_THREADS_PTHREADS
    /// The Pthread mutex
    pthread_mutex_t p_m;
    /// The Pthread condition variable
    pthread_cond_t p_c;
    /// Whether the event is signalled
    bool p_s;
    /// Whether a thread is waiting
    bool p_w;
#endif
  public:
    /// Initialize event
    Event(void);
    /// Signal the event
    void signal(void);
    /// Wait until the event becomes signalled
    void wait(void);
    /// Delete event
    ~Event(void);
  private:
    /// An event cannot be copied
    Event(const Event&) {}
    /// An event cannot be assigned
    void operator=(const Event&) {}
  };

}}

#ifdef GECODE_THREADS_WINDOWS
#include <gecode/support/thread/windows.hpp>
#endif
#ifdef GECODE_THREADS_PTHREADS
#include <gecode/support/thread/pthreads.hpp>
#endif

#include <gecode/support/thread/thread.hpp>

// Workaround for broken windows headers
#ifdef GECODE_THREADS_WINDOWS
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

#endif

#endif

// STATISTICS: support-any
