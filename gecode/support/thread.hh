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
#define GECODE_HAS_THREADS
#endif

#ifdef GECODE_THREADS_PTHREADS
#include <pthread.h>
#define GECODE_HAS_THREADS
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
 * If the platform supports threads, after inclusion the macro
 * GECODE_HAS_THREADS is defined.
 *
 * \ingroup FuncSupport
 */  

namespace Gecode { namespace Support {

  /// An interface for objects that can be run by a thread
  class Runnable {
  public:
    /// The function that is executed when the thread starts
    virtual void run(void) = 0;
  };

  /**
   * \brief Simple threads
   *
   * Threads are assumed to properly terminate, the destructor will
   * only release the handle to a thread but will not terminate it.
   */
  class GECODE_SUPPORT_EXPORT Thread {
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
    /// A class for thread identification
    class Id {
      friend class Thread;
      friend bool operator ==(const Thread::Id&, const Thread::Id&);
      friend bool operator !=(const Thread::Id&, const Thread::Id&);
    private:
#ifdef GECODE_THREADS_WINDOWS
      /// The Windows-specific id
      DWORD w_id;
#endif
#ifdef GECODE_THREADS_PTHREADS
      /// The Pthread-specific id
      pthread_t p_t;
#endif
    };
    /// Get a handle on the current thread
    Thread(void);
    /// Construct a new thread and run \a r (\a r must outlive the thread)
    Thread(Runnable& r);
    /// Destroy thread handle (does not terminate thread)
    ~Thread(void);
    /// Put current thread to sleep for \a ms milliseconds
    static void sleep(unsigned int ms);
    /// Return identifier for thread
    Id id(void) const;
    /// Return number of processing units (0 if information not available)
    static unsigned int npu(void);
  private:
    /// A thread cannot be copied
    Thread(const Thread&) {}
    /// A thread cannot be assigned
    void operator=(const Thread&) {}
  };

  /**
   * \brief Test whether thread identifiers are equal
   * \relates Thread::Id
   */
  bool operator ==(const Thread::Id& ti1, const Thread::Id& ti2);
  /**
   * \brief Test whether thread identifiers are not equal
   * \relates Thread::Id
   */
  bool operator !=(const Thread::Id& ti1, const Thread::Id& ti2);

  /**
   * \brief Test whether threads are equal
   * \relates Thread
   */
  bool operator ==(const Thread& t1, const Thread& t2);
  /**
   * \brief Test whether threads are not equal
   * \relates Thread::Id
   */
  bool operator !=(const Thread& t1, const Thread& t2);


  /**
   * \brief A mutex for mutual exclausion among several threads
   * 
   * The mutex is recursive: if a thread already holds the mutex,
   * then it can acquire it again. However, the number of acquire and
   * release operations must match.
   */
  class Mutex {
  private:
#ifdef GECODE_THREADS_WINDOWS
    /// Use a simple but more efficient critical section on Windows
    CRITICAL_SECTION w_cs;
#endif
#ifdef GECODE_THREADS_PTHREADS
    /// The Pthread-mutex
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

  /// A lock as a scoped frontend for a mutex
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

}}

#ifdef GECODE_THREADS_WINDOWS
#include <gecode/support/thread/windows.hpp>
#endif
#ifdef GECODE_THREADS_PTHREADS
#include <gecode/support/thread/pthreads.hpp>
#endif

#include <gecode/support/thread/thread.hpp>

#endif

#endif

// STATISTICS: support-any
