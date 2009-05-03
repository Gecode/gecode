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

#include <gecode/support/thread.hh>

#ifdef GECODE_THREADS_PTHREADS

#ifdef GECODE_HAS_UNISTD_H
#include <unistd.h>
#endif

namespace Gecode { namespace Support {

  /*
   * Thread
   */
  /// Function to start execution
  void*
  bootstrap(void* p) {
    static_cast<Runnable*>(p)->run();
    delete static_cast<Runnable*>(p);
    pthread_exit(NULL);
    return NULL;
  }

  Thread::Thread(Runnable* r) {
    if (pthread_create(&p_t, NULL, bootstrap, r) != 0)
      throw OperatingSystemError("Thread::Thread[pthread_create]");
  }
  
  void
  Thread::sleep(unsigned int ms) {
#ifdef GECODE_HAS_UNISTD_H
    unsigned int s = ms / 1000;
    ms -= 1000 * s;
    if (s > 0) {
      // More than one million microseconds, use sleep
      sleep(s);
    }
    usleep(ms * 1000);
#endif
  }

  unsigned int
  Thread::npu(void) {
#ifdef GECODE_HAS_UNISTD_H
    int n=sysconf(_SC_NPROCESSORS_ONLN);
    return (n>1) ? n : 1;
#else
    return 1;
#endif
  }


  /*
   * Mutex
   */
  Mutex::Mutex(void) {
    if (pthread_mutex_init(&p_m,NULL) != 0)
      throw OperatingSystemError("Mutex::Mutex[pthread_mutex_init]");
  }

  Mutex::~Mutex(void) {
    if (pthread_mutex_destroy(&p_m) != 0)
      throw OperatingSystemError("Mutex::~Mutex[pthread_mutex_destroy]");
  }


  /*
   * Event
   */
  Event::Event(void) : p_s(false), p_w(false) {
    if (pthread_mutex_init(&p_m,NULL) != 0)
      throw OperatingSystemError("Event::Event[pthread_mutex_init]");
    if (pthread_cond_init(&p_c,NULL) != 0)
      throw OperatingSystemError("Event::Event[pthread_cond_init]");
  }

  Event::~Event(void) {
    if (pthread_cond_destroy(&p_c) != 0)
      throw OperatingSystemError("Event::~Event[pthread_cond_destroy]");
    if (pthread_mutex_destroy(&p_m) != 0)
      throw OperatingSystemError("Event::~Event[pthread_mutex_destroy]");
  }


}}

#endif

// STATISTICS: support-any
