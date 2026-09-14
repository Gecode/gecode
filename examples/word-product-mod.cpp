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
 * \brief %Example: Mathematical multiplication followed by modulo
 *
 * Finds factors between 10 and 30 whose product has remainder 16 modulo 17.
 * product_mod reduces the mathematical product before encoding the result,
 * even when the product exceeds the nine-bit Word range.
 * \ingroup Example
 */
class Model : public Script {
public:
  explicit Model(const Options& opt)
    : Script(opt), x(*this,9,WDT_UNSIGNED,10U,30U),
      y(*this,9,WDT_UNSIGNED,10U,30U), result(*this,9,WDT_UNSIGNED,16U,16U),
      modulus(*this,17,17) {
    product_mod(*this,x,y,modulus,result);
    branch(*this,WordVarArgs({x,y}),WORD_VAR_NONE(),WORD_VAL_SPLIT_MIN());
  }
  Model(Model& s) : Script(s) {
    x.update(*this,s.x);
    y.update(*this,s.y);
    result.update(*this,s.result);
    modulus.update(*this,s.modulus);
  }
  Space* copy(void) override { return new Model(*this); }
  void print(std::ostream& os) const override {
    os << "\tx = " << x.val() << ", y = " << y.val()
       << ", (x * y) mod " << modulus.val() << " = " << result.val()
       << std::endl;
  }
private:
  WordVar x, y, result;
  IntVar modulus;
};

int
main(int argc, char* argv[]) {
  Options opt("WordProductMod");
  opt.parse(argc,argv);
  Script::run<Model,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
