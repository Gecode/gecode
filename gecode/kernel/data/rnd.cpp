/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *     Mikael Lagerkvist, 2008
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

  Support::Mutex Rnd::IMP::m;

  forceinline
  Rnd::IMP::IMP(unsigned int s)
    : rg(s) {}

  Rnd::IMP::~IMP(void) {}

  forceinline void
  Rnd::_seed(unsigned int s) {
    if (object() == NULL) {
      object(new IMP(s));
    } else {
      static_cast<IMP*>(object())->seed(s);
    }
  }

  Rnd::Rnd(void) {}
  Rnd::Rnd(unsigned int s) {
    object(new IMP(s));
  }
  Rnd::Rnd(const Rnd& r)
    : SharedHandle(r) {}
  Rnd&
  Rnd::operator =(const Rnd& r) {
    (void) SharedHandle::operator =(r);
    return *this;
  }
  Rnd::~Rnd(void) {}

  void
  Rnd::seed(unsigned int s) {
    _seed(s);
  }
  void
  Rnd::time(void) {
    _seed(static_cast<unsigned int>(::time(NULL)));
  }
  void
  Rnd::hw(void) {
    _seed(Support::hwrnd());
  }
}

// STATISTICS: kernel-other
