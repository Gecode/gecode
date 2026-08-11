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
 * \brief %Example: Direct fixed-width word arithmetic
 *
 * Finds the four-bit input satisfying input + 3 = 9 modulo 16.
 *
 * \ingroup Example
 */
class WordArithmetic : public Script {
private:
  /// Unknown input and constrained result
  WordVar input;
  WordVar result;
public:
  /// Actual model
  WordArithmetic(const Options& opt)
    : Script(opt), input(*this,4), result(*this,4) {
    add(*this,input,4,3U,result);
    dom(*this,result,9U);
    branch(*this,input,WORD_VAL_LSB());
  }
  /// Constructor for cloning \a s
  WordArithmetic(WordArithmetic& s) : Script(s) {
    input.update(*this,s.input);
    result.update(*this,s.result);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordArithmetic(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tinput = 0x" << std::hex << input.val()
       << ", result = 0x" << result.val() << std::dec << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordArithmetic
 */
int
main(int argc, char* argv[]) {
  Options opt("WordArithmetic");
  opt.parse(argc,argv);
  Script::run<WordArithmetic,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
