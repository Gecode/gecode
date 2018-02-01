/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

#include <gecode/kernel.hh>

namespace Gecode {

  const Action Action::def;

  Action::Action(const Action& a)
    : storage(a.storage) {
    if (storage != NULL) {
      acquire();
      storage->use_cnt++;
      release();
    }
  }

  Action&
  Action::operator =(const Action& a) {
    if (storage != a.storage) {
      if (storage != NULL) {
        bool done;
        acquire();
        done = (--storage->use_cnt == 0);
        release();
        if (done)
          delete storage;
      }
      storage = a.storage;
      if (storage != NULL) {
        acquire();
        storage->use_cnt++;
        release();
      }
    }
    return *this;
  }

  Action::~Action(void) {
    if (storage == NULL)
      return;
    bool done;
    acquire();
    done = (--storage->use_cnt == 0);
    release();
    if (done)
      delete storage;
  }

  void
  Action::update(Space&, bool, Action& a) {
    const_cast<Action&>(a).acquire();
    storage = a.storage;
    storage->use_cnt++;
    const_cast<Action&>(a).release();
  }

  void
  Action::decay(Space&, double d) {
    if ((d < 0.0) || (d > 1.0))
      throw IllegalDecay("Action");
    acquire();
    storage->invd = 1.0 / d;
    release();
  }

  double
  Action::decay(const Space&) const {
    double d;
    const_cast<Action*>(this)->acquire();
    d = 1.0 / storage->invd;
    const_cast<Action*>(this)->release();
    return d;
  }

}

// STATISTICS: kernel-branch
