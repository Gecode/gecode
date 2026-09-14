/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
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
 */

#include <gecode/driver.hh>
#include <gecode/word.hh>

using namespace Gecode;

/**
 * \brief %Example: A scatter/gather transfer with a fixed total length
 *
 * Four, six, or eight aligned segment lengths sum to 160 times their count.
 * The total cannot wrap at this width. Nondecreasing lengths remove symmetric
 * permutations. Pass the count as the final positional size argument.
 * \ingroup Example
 */
class ScatterGather : public Script {
public:
  explicit ScatterGather(const SizeOptions& opt)
    : Script(opt), lengths(*this,opt.size(),12,WDT_UNSIGNED,64U,256U),
      total(*this,12,WDT_UNSIGNED,160U*opt.size(),160U*opt.size()) {
    for (int i=0; i<lengths.size(); i++)
      dom(*this,lengths[i],0U,0xff0U);
    for (int i=1; i<lengths.size(); i++)
      rel(*this,lengths[i-1],WRT_ULQ,lengths[i]);
    add(*this,WordVarArgs(lengths),total);
    branch(*this,lengths,WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
  }
  ScatterGather(ScatterGather& s) : Script(s) {
    lengths.update(*this,s.lengths);
    total.update(*this,s.total);
  }
  Space* copy(void) override { return new ScatterGather(*this); }
  void print(std::ostream& os) const override {
    os << "\tlengths = {";
    for (int i=0; i<lengths.size(); i++) {
      if (i != 0) os << ", ";
      os << lengths[i].val();
    }
    os << "}, total = " << total.val() << std::endl;
  }
private:
  WordVarArray lengths;
  WordVar total;
};

int
main(int argc, char* argv[]) {
  SizeOptions opt("WordNaryAdd");
  opt.size(4);
  opt.parse(argc,argv);
  const bool is_supported_count=(opt.size() == 4) || (opt.size() == 6) ||
                                (opt.size() == 8);
  if (!is_supported_count) {
    std::cerr << "size must be 4, 6, or 8" << std::endl;
    return 1;
  }
  Script::run<ScatterGather,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any
