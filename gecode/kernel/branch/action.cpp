/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <gecode/kernel.hh>

namespace Gecode {

  Support::Mutex Action::Storage::m;

  Action::Storage::~Storage(void) {
    heap.free<double>(a,n);
  }

  const Action Action::def;

  Action::Action(const Action& a)
    : SharedHandle(a) {}

  Action&
  Action::operator =(const Action& a) {
    (void) SharedHandle::operator =(a);
    return *this;
  }

  Action::~Action(void) {}

  void
  Action::decay(Space&, double d) {
    if ((d < 0.0) || (d > 1.0))
      throw IllegalDecay("Action");
    acquire();
    object().invd = 1.0 / d;
    release();
  }

  double
  Action::decay(const Space&) const {
    double d;
    const_cast<Action*>(this)->acquire();
    d = 1.0 / object().invd;
    const_cast<Action*>(this)->release();
    return d;
  }

}

// STATISTICS: kernel-branch
