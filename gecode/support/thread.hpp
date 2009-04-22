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

#ifdef GECODE_THREADS_WINDOWS

namespace Gecode { namespace Support {

  /*
   * Thread
   */
  forceinline
  Thread::Thread(void) 
    : w_h(GetCurrentThread()) {} 
  forceinline
  Thread::~Thread(void) {
    CloseHandle(w_h);
  }

  forceinline void
  Thread::sleep(unsigned int ms) {
    Sleep(static_cast<DWORD>(ms));
  }
  forceinline void
  Thread::yield(void) {
    Sleep(0);
  }
  forceinline Thread::Id
  Thread::id(void) const {
    Id i; i.w_id = GetThreadId(w_h);
    return i;
  } 

  /*
   * Mutex
   */
  forceinline void
  Mutex::enter(void) {
    EnterCriticalSection(&w_cs);
  }
  forceinline bool
  Mutex::tryenter(void) {
    return TryEnterCriticalSection(&w_cs) != 0;
  }
  forceinline void
  Mutex::leave(void) {
    LeaveCriticalSection(&w_cs);
  }

}}

#endif

#ifdef GECODE_THREADS_PTHREADS

  /*
   * Thread
   */
  forceinline
  Thread::Thread(void) 
    : p_t(pthread_self()) {} 
  forceinline
  Thread::~Thread(void) {
  }

  forceinline void
  Thread::sleep(unsigned int ms) {
    Sleep(static_cast<DWORD>(ms));
  }
  forceinline void
  Thread::yield(void) {
    pthread
  }
  forceinline Thread::Id
  Thread::id(void) const {
    Id i; i.p_t = p_t;
    return i;
  } 


  /*
   * Mutex
   */
  forceinline void
  Mutex::enter(void) {
    EnterCriticalSection(&w_cs);
  }
  forceinline bool
  Mutex::tryenter(void) {
    return TryEnterCriticalSection(&w_cs) != 0;
  }
  forceinline void
  Mutex::leave(void) {
    LeaveCriticalSection(&w_cs);
  }

#endif

namespace Gecode { namespace Support {

  /*
   * Locks
   */
  forceinline
  Lock::Lock(Mutex& m0) : m(m0) {
    m.enter();
  }
  forceinline
  Lock::~Lock(void) {
    m.leave();
  }

}}

// STATISTICS: support-any
