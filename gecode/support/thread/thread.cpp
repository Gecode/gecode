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

#include <gecode/support.hh>

namespace Gecode { namespace Support {

  /*
   * Threads
   */

  Mutex* Thread::m(void) {
    static Mutex* m = new Mutex;
    return m;
  }

  Thread::Run* Thread::idle = nullptr;

  void
  Thread::Run::exec(void) {
    while (true) {
      // Execute runnable
      {
        GECODE_ASSUME(r != nullptr);
        Runnable* e = r.exchange(nullptr);
        assert(e != nullptr);
        e->run();
        if (e->todelete()) {
          Terminator* t = e->terminator();
          delete e;
          if (t)
            t->terminated();
        }
      }
      // Put into idle stack
      Thread::m()->acquire();
      n=Thread::idle; Thread::idle=this;
      Thread::m()->release();
      // Wait for next runnable
      e.wait();
    }
  }

  Thread::Run::Run(Runnable* r0) {
#ifdef GECODE_HAS_THREADS
    r.store(r0);
    std::thread t([](Thread::Run* r){r->exec();}, this);
    t.detach();
#else
    throw OperatingSystemError("Thread::run[Threads not supported]");
#endif
  }


  namespace {

    class GlobalMutexRunnable : public Runnable {
    protected:
      Mutex _m;
      Mutex _m_write;
      Event _e;
      Event _e_done;
      Mutex* _to_acquire;
      Mutex* _to_release;
    public:
      GlobalMutexRunnable(void) : _to_acquire(nullptr), _to_release(nullptr) {}
      virtual void run(void) {
        while (true) {
          _m.acquire();
          if (_to_acquire) {
            _to_acquire->acquire();
            _to_acquire = nullptr;
            _e_done.signal();
          } else if (_to_release) {
            _to_release->release();
            _to_release = nullptr;
            _e_done.signal();
          }
          _m.release();
          _e.wait();
        }
      }
      void acquire(Mutex* m) {
        _m_write.acquire();
        _m.acquire();
        _to_acquire = m;
        _m.release();
        _e.signal();
        _e_done.wait();
        _m_write.release();
      }
      void release(Mutex* m) {
        _m_write.acquire();
        _m.acquire();
        _to_release = m;
        _m.release();
        _e.signal();
        _e_done.wait();
        _m_write.release();
      }
    };
    
    class GlobalMutexRunnableInit {
    public:
      GlobalMutexRunnable* gmr;
      GlobalMutexRunnableInit(void) : gmr(new GlobalMutexRunnable) {
        Thread::run(gmr);
      }
    };
    
    GlobalMutexRunnable&
    globalMutexRunnable(void) {
      static GlobalMutexRunnableInit gmri;
      return *gmri.gmr;
    }
  }

  void
  Thread::acquireGlobalMutex(Mutex* m) {
    globalMutexRunnable().acquire(m);
  }

  void
  Thread::releaseGlobalMutex(Mutex* m) {
    globalMutexRunnable().release(m);
  }

}}

// STATISTICS: support-any
