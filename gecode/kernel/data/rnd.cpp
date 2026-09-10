/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2008
 *     Mikael Zayenz Lagerkvist, 2008
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
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
  Rnd::Rnd(Space& home, const Rnd& source)
    : SharedHandle(home.random(source)) {}

  void Rnd::seed(uint64_t value) {
    if (!object())
      *this = Rnd(value);
    else
      imp().seed(value);
  }

  void Rnd::state(const std::string& text) {
    if (!object()) {
      Rnd candidate(1);
      candidate.state(text);
      *this = candidate;
    } else {
      imp().state(text);
    }
  }

  void Rnd::time(void) {
    seed(static_cast<uint64_t>(::time(nullptr)));
  }

  void Rnd::hw(void) {
    seed((uint64_t(Support::hwrnd()) << 32) | Support::hwrnd());
  }

  Rnd Space::random(const Rnd& source) {
    if (!source)
      throw UninitializedRnd("Space::random");
    // During actor/model copying, map source-local handles by their position.
    if (is_partial_clone() && pc.c.source && pc.c.source->randoms) {
      size_t i = pc.c.source->randoms->find(source);
      if (i < pc.c.source->randoms->size())
        return randoms->at(i);
    }
    if (!randoms)
      randoms = new RandomContext;
    return randoms->bind(source);
  }
}

// STATISTICS: kernel-other
