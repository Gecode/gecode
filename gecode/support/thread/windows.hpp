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
    if (CloseHandle(w_h) == 0)
      throw OperatingSystemError("Thread::~Thread[Windows::CloseHandle]");
  }

  forceinline Thread::Id
  Thread::id(void) const {
    Id i; i.w_id = GetThreadId(w_h);
    return i;
  } 


  /*
   * Equality checks for thread identifiers
   */
  forceinline bool 
  operator ==(const Thread::Id& ti1, const Thread::Id& ti2) {
    return ti1.w_id == ti2.w_id;
  }
  forceinline bool 
  operator !=(const Thread::Id& ti1, const Thread::Id& ti2) {
    return ti1.w_id != ti2.w_id;
  }


  /*
   * Mutex
   */
  forceinline void
  Mutex::acquire(void) {
    EnterCriticalSection(&w_cs);
  }
  forceinline bool
  Mutex::tryacquire(void) {
    return TryEnterCriticalSection(&w_cs) != 0;
  }
  forceinline void
  Mutex::release(void) {
    LeaveCriticalSection(&w_cs);
  }


  /*
   * Event
   */
  forceinline void
  Event::signal(void) {
    if (SetEvent(w_h) == 0)
      throw OperatingSystemError("Event::signal[Windows::SetEvent]");
  }
  forceinline void
  Event::wait(void) {
    if (WaitForSingleObject(w_h,INFINITE) != 0)
      throw OperatingSystemError("Event::wait[Windows::WaitForSingleObject]");
  }


}}

#endif

// STATISTICS: support-any
