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
 *  This file must not compile. Each configuration declares one of the
 *  meta-search hooks removed in Gecode 7. A successful compilation means
 *  that a compatibility hook has returned.
 *
 */

#include <gecode/kernel.hh>

class HubSpokeApiFail : public Gecode::Space {
public:
  HubSpokeApiFail(void) {}
  HubSpokeApiFail(HubSpokeApiFail& s) : Gecode::Space(s) {}
  Gecode::Space*
  copy(void) override {
    return new HubSpokeApiFail(*this);
  }
#if defined(GECODE_TEST_REMOVED_MASTER)
  bool
  master(const Gecode::MetaInfo&) override {
    return true;
  }
#elif defined(GECODE_TEST_REMOVED_SLAVE)
  bool
  slave(const Gecode::MetaInfo&) override {
    return true;
  }
#else
#error Select one removed meta-search hook
#endif
};

// STATISTICS: test-search
