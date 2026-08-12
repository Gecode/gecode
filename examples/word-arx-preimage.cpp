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
 * \brief %Example: Recover an input through an ARX-style data path
 *
 * The model solves
 * \f$output=rotate_left(input+0x3d,3)\mathbin{xor}0xa5\f$ at width eight.
 * It branches from the observed output back towards the input. For larger
 * variants, Script copy and recomputation options can be worth comparing.
 *
 * \ingroup Example
 */
class WordArxPreimage : public Script {
private:
  /// Input and intermediate data-flow states
  WordVar input;
  WordVar added;
  WordVar rotated;
  WordVar output;
public:
  /// Actual model
  WordArxPreimage(const Options& opt)
    : Script(opt), input(*this,8), added(*this,8), rotated(*this,8),
      output(*this,8) {
    add(*this,input,8,0x3dU,added);
    rotate_left(*this,added,3,rotated);
    rel(*this,rotated,WOT_XOR,8,0xa5U,output);
    dom(*this,output,0x5eU);

    WordVarArgs decision={rotated,added,input};
    branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordArxPreimage(WordArxPreimage& s) : Script(s) {
    input.update(*this,s.input);
    added.update(*this,s.added);
    rotated.update(*this,s.rotated);
    output.update(*this,s.output);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordArxPreimage(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tinput = 0x" << std::hex << input.val()
       << ", output = 0x" << output.val() << std::dec << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordArxPreimage
 */
int
main(int argc, char* argv[]) {
  Options opt("WordArxPreimage");
  opt.parse(argc,argv);
  Script::run<WordArxPreimage,DFS,Options>(opt);
  return 0;
}

// STATISTICS: example-any
