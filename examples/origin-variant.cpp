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
 *  Example: migrate meta-search callbacks to Gecode 7
 *
 *  Gecode 6 declaration                    Gecode 7 declaration
 *  bool master(const MetaInfo&) override;  bool origin(const MetaInfo&) override;
 *  bool slave(const MetaInfo&) override;   bool variant(const MetaInfo&) override;
 *
 *  Gecode 7 provides no compatibility aliases for the removed declarations.
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>

using namespace Gecode;

class OriginVariant : public Script {
protected:
  IntVar x;

public:
  OriginVariant(const Options& opt)
    : Script(opt), x(*this,0,1) {
    branch(*this,x,INT_VAL_MIN());
  }

  OriginVariant(OriginVariant& s)
    : Script(s) {
    x.update(*this,s.x);
  }

  Space* copy(void) override {
    return new OriginVariant(*this);
  }

  bool origin(const MetaInfo& mi) override {
    return Space::origin(mi);
  }

  bool variant(const MetaInfo& mi) override {
    return Space::variant(mi);
  }

  void print(std::ostream& os) const override {
    os << "x = " << x << std::endl;
  }
};

int
main(int argc, char* argv[]) {
  Options opt("OriginVariant");
  opt.parse(argc,argv);
  Script::run<OriginVariant,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
