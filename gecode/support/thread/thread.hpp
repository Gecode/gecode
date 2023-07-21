/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2009
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

  /*
   * Runnable objects
   */
  forceinline
  Runnable::Runnable(bool d0)
    : d(d0) {}
  forceinline void
  Runnable::todelete(bool d0) {
    d=d0;
  }
  forceinline bool
  Runnable::todelete(void) const {
    return d;
  }
  forceinline void
  Runnable::operator delete(void* p) {
    heap.rfree(p);
  }
  forceinline void*
  Runnable::operator new(size_t s) {
    return heap.ralloc(s);
  }

  /*
   * Mutexes
   *
   */
  forceinline
  Mutex::Mutex(void) {
#ifdef GECODE_USE_OSX_UNFAIR_MUTEX
    l = OS_UNFAIR_LOCK_INIT;
#endif
  }
  forceinline void
  Mutex::acquire(void) {
#ifdef GECODE_HAS_THREADS
#ifndef GECODE_USE_OSX_UNFAIR_MUTEX
    m.lock();
#else
    os_unfair_lock_lock(&l);
#endif
#endif
  }

  forceinline bool
  Mutex::tryacquire(void) {
#ifdef GECODE_HAS_THREADS
#ifndef GECODE_USE_OSX_UNFAIR_MUTEX
    return m.try_lock();
#else
    return os_unfair_lock_trylock(&l);
#endif
#else
    return true;
#endif
  }

  forceinline void
  Mutex::release(void) {
#ifdef GECODE_HAS_THREADS
#ifndef GECODE_USE_OSX_UNFAIR_MUTEX
    m.unlock();
#else
    os_unfair_lock_unlock(&l);
#endif
#endif
  }

  forceinline void*
  Mutex::operator new(size_t s) {
    return Gecode::heap.ralloc(s);
  }

  forceinline void
  Mutex::operator delete(void* p) {
    Gecode::heap.rfree(p);
  }


  /*
   * Locks
   */
  forceinline
  Lock::Lock(Mutex& m0) : m(m0) {
    m.acquire();
  }
  forceinline
  Lock::~Lock(void) {
    m.release();
  }

  /*
   * Events
   */
  forceinline
  Event::Event(void) : s(false) {}
  forceinline void
  Event::signal(void) {
#ifdef GECODE_HAS_THREADS
    std::unique_lock<std::mutex> l(m);
    if (!s) {
      s = true;
      c.notify_one();
    }
#endif
  }
  forceinline void
  Event::wait(void) {
#ifdef GECODE_HAS_THREADS
    std::unique_lock<std::mutex> l(m);
    c.wait(l, [this]{return this->s;});
    s = false;
#endif
  }


  /*
   * Threads
   */
  forceinline void
  Thread::sleep(unsigned int ms) {
#ifdef GECODE_HAS_THREADS
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#else
    (void) ms;
#endif
  }
  forceinline unsigned int
  Thread::npu(void) {
#ifdef GECODE_HAS_THREADS
    return std::thread::hardware_concurrency();
#else
    return 1;
#endif
  }
  inline void
  Thread::Run::run(Runnable* r0) {
    r.store(r0);
    e.signal();
  }
  inline void
  Thread::run(Runnable* r) {
    m()->acquire();
    if (idle != nullptr) {
      Run* i = idle;
      idle = idle->n;
      m()->release();
      i->run(r);
    } else {
      m()->release();
      (void) new Run(r);
    }
  }
  forceinline void
  Thread::Run::operator delete(void* p) {
    heap.rfree(p);
  }
  forceinline void*
  Thread::Run::operator new(size_t s) {
    return heap.ralloc(s);
  }

}}

// STATISTICS: support-any
