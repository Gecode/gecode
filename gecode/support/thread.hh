/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Last modified:
 *     $Date: 2009-02-05 11:48:53 +0100 (Thu, 05 Feb 2009) $ by $Author: schulte $
 *     $Revision: 8155 $
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

#ifdef GECODE_HAS_THREADS

#ifndef __GECODE_SUPPORT_THREAD_HH__
#define __GECODE_SUPPORT_THREAD_HH__

#include <gecode/support.hh>

#ifdef GECODE_THREADS_WINDOWS
#include <windows.h>
#endif
#ifdef GECODE_THREADS_PTHREADS
#include <pthread.h>
#endif

namespace Gecode { namespace Support {

  /// An interface for objects that can be run by a thread
  class Runnable {
  public:
    /// The function that is executed when the thread starts
    virtual void run(void) = 0;
  };

  /// Simple threads
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
    /// Sleep for \a ms milliseconds
    void sleep(unsigned int ms);
    /// Stop executing the thread
    void yield(void);
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


  /// A mutex for mutual exclusion among several threads
  class Mutex {
  private:
#ifdef GECODE_THREADS_WINDOWS
    /// Use a simple but more efficient critical section on Windows
    CRITICAL_SECTION w_cs;
#endif
#ifdef GECODE_THREADS_PTHREADS
    /// The Pthread-mutex
    pthread_mutex_t p_mutex;
#endif
  public:
    /// Create a new mutex
    Mutex(void);
    /// Enter the mutex and possible block
    void enter(void);
    /// Try to enter the mutex, return if succesful
    bool tryenter(void);
    /// Leave the mutex
    void leave(void);
    /// Delete mutex object
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
    /// Enter mutex
    Lock(Mutex& m0);
    /// Leave mutex
    ~Lock(void);
  };

}}

#include <gecode/support/thread.hpp>

#endif

#endif

// STATISTICS: support-any
