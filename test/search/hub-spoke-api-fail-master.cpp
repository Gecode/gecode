/*
 *  Main authors:
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Mikael Zayenz Lagerkvist, 2026
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
 *  This file must not compile: it is hub-spoke-api-pass.cpp with the
 *  hub() override replaced by a Gecode 6 master() override without
 *  override. Space deletes master(), and a non-deleted function cannot
 *  override a deleted one. Successful compilation means that an old
 *  override would again compile and silently never be called.
 *
 */

#include <gecode/kernel.hh>

namespace Test { namespace Search {

  /// Space overriding the meta-search hooks
  class HookSpace : public Gecode::Space {
  public:
    HookSpace(void) {}
    HookSpace(HookSpace& s) : Gecode::Space(s) {}
    Gecode::Space*
    copy(void) override {
      return new HookSpace(*this);
    }
    bool
    master(const Gecode::MetaInfo&) {
      return true;
    }
    bool
    spoke(const Gecode::MetaInfo&) override {
      return true;
    }
  };

}} // namespace Test::Search

// STATISTICS: test-search
