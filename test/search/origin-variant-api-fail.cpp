/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
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
 * This file is expected not to compile. Its two declarations use the
 * removed Gecode 6 meta-search callbacks. A successful compilation means
 * that a compatibility hook has accidentally returned.
 */

#include <gecode/kernel.hh>

class OriginVariantApiFail : public Gecode::Space {
public:
  OriginVariantApiFail(void) {}
  OriginVariantApiFail(OriginVariantApiFail& s) : Gecode::Space(s) {}

  Gecode::Space* copy(void) override {
    return new OriginVariantApiFail(*this);
  }

#if defined(GECODE_TEST_REMOVED_MASTER)
  bool master(const Gecode::MetaInfo&) override {
    return true;
  }
#elif defined(GECODE_TEST_REMOVED_SLAVE)
  bool slave(const Gecode::MetaInfo&) override {
    return true;
  }
#else
#error Select one removed meta-search hook
#endif
};
