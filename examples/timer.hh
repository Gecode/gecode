/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
 *
 *  Bugfixes provided by:
 *     Javier Andrés Mena Zapata <javimena@gmail.com>
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

#ifndef __GECODE_EXAMPLES_TIMER_HH__
#define __GECODE_EXAMPLES_TIMER_HH__

#include <ctime>
#include <cmath>

#include "gecode/config.icc"

/*
 * Support for measuring time
 *
 */

/// Timer interface used for examples
class Timer {
private:
  clock_t t0;
public:
  void start(void);
  double stop(void);
};

forceinline void
Timer::start(void) {
  t0 = clock();
}
forceinline double
Timer::stop(void) {
  return (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC) * 1000.0;
}


/**
 * \brief Compute arithmetic mean of \a n elements in \a t
 * \relates Timer
 */
double
am(double t[], int n);
/**
 * \brief Compute deviation of \a n elements in \a t
 * \relates Timer
 */
double
dev(double t[], int n);

#endif

// STATISTICS: example-any
