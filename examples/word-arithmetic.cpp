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

/** \brief Options for the direct word arithmetic example */
class WordArithmeticOptions : public Options {
private:
  /// Word domain representation
  Driver::StringOption _domain;
public:
  enum Domain { DOMAIN_CUBE, DOMAIN_UNSIGNED, DOMAIN_SIGNED };
  /// Initialize options
  WordArithmeticOptions(const char* n)
    : Options(n),
      _domain("word-domain","word domain representation",DOMAIN_CUBE) {
    add(_domain);
    _domain.add(DOMAIN_CUBE,"cube","independently known bits");
    _domain.add(DOMAIN_UNSIGNED,"unsigned","unsigned interval and cube");
    _domain.add(DOMAIN_SIGNED,"signed","signed interval and cube");
  }
  /// Return selected Word domain representation
  WordDomainType domain_type(void) const {
    switch (_domain.value()) {
    case DOMAIN_UNSIGNED: return WDT_UNSIGNED;
    case DOMAIN_SIGNED: return WDT_SIGNED;
    default: return WDT_CUBE;
    }
  }
};

/**
 * \brief %Example: Direct fixed-width word arithmetic
 *
 * Finds the four-bit input satisfying input + 1 = 6 modulo 16. Select
 * \c -word-domain \c cube, \c unsigned, or \c signed to compare the three
 * representations without changing the constraint. All modes print the same
 * solution: input 5 and result 6.
 *
 * \ingroup Example
 */
class WordArithmetic : public Script {
private:
  /// Construct the input domain for the selected representation
  static WordVar input_var(Space& home, WordDomainType kind) {
    return (kind == WDT_CUBE) ? WordVar(home,4) :
      WordVar(home,4,kind,0,6);
  }
  /// Construct the result domain for the selected representation
  static WordVar result_var(Space& home, WordDomainType kind) {
    return (kind == WDT_CUBE) ? WordVar(home,4) :
      WordVar(home,4,kind,1,7);
  }
  /// Unknown input and constrained result
  WordVar input;
  WordVar result;
public:
  /// Actual model
  WordArithmetic(const WordArithmeticOptions& opt)
    : Script(opt), input(input_var(*this,opt.domain_type())),
      result(result_var(*this,opt.domain_type())) {
    add(*this,input,4,1U,result);
    dom(*this,result,6U);
    branch(*this,input,(opt.domain_type() == WDT_CUBE) ?
           WORD_VAL_LSB() : WORD_VAL_SPLIT_MIN());
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
  WordArithmeticOptions opt("WordArithmetic");
  opt.parse(argc,argv);
  Script::run<WordArithmetic,DFS,WordArithmeticOptions>(opt);
  return 0;
}

// STATISTICS: example-any
