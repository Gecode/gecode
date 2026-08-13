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

#include <iomanip>

using namespace Gecode;

/** \brief Options for the constructed symbolic ALU example */
class WordSymbolicALUOptions : public Options {
private:
  /// Number of ALU steps
  Driver::UnsignedIntOption _steps;
  /// Word width
  Driver::UnsignedIntOption _width;
public:
  /// Initialize options
  WordSymbolicALUOptions(const char* n)
    : Options(n), _steps("steps","number of constructed ALU steps",8),
      _width("width","word width",18) {
    add(_steps); add(_width);
  }
  /// Return number of steps
  unsigned int steps(void) const { return _steps.value(); }
  /// Return word width
  unsigned int width(void) const { return _width.value(); }
};

/**
 * \brief %Example: Constructed symbolic register and ALU path
 *
 * This deterministic model is inspired by symbolic-execution workloads; it
 * is not a copied benchmark instance.  A symbolic input passes through
 * rotate/add, rotate/XOR, and rotate/AND-plus-constant instructions.  The low
 * two output bits are fixed to zero.  Use \c -steps \c 12 \c -width \c 22
 * for the larger profiling configuration.
 *
 * \ingroup Example
 */
class WordSymbolicALU : public Script {
private:
  /// Symbolic input register
  WordVar input;
  /// Final output register
  WordVar output;
public:
  /// Actual model
  WordSymbolicALU(const WordSymbolicALUOptions& opt)
    : Script(opt), input(*this,opt.width()), output(*this,opt.width()) {
    const WordValue mask = (opt.width() == 64) ? ~WordValue(0) :
                           (WordValue(1) << opt.width())-1;
    WordVarArray reg(*this,opt.steps()+1,opt.width(),0,mask);
    dom(*this,reg[0],WordValue(0x2a55aU)&mask);
    for (unsigned int i=0; i<opt.steps(); i++) {
      WordVar rotated(*this,opt.width()), next(*this,opt.width());
      rotate_left(*this,reg[i],i%(opt.width()-1)+1,rotated);
      if (i%3 == 0) {
        add(*this,rotated,input,next);
      } else if (i%3 == 1) {
        rel(*this,rotated,WOT_XOR,input,next);
      } else {
        WordVar selected(*this,opt.width());
        rel(*this,rotated,WOT_AND,input,selected);
        add(*this,selected,opt.width(),WordValue(0x12345U+i)&mask,next);
      }
      rel(*this,reg[i+1],WRT_EQ,next);
    }
    rel(*this,output,WRT_EQ,reg[opt.steps()]);
    dom(*this,output,0,mask & ~WordValue(3));
    branch(*this,input,WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordSymbolicALU(WordSymbolicALU& s) : Script(s) {
    input.update(*this,s.input);
    output.update(*this,s.output);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordSymbolicALU(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tinput = 0x" << std::hex << input.val()
       << ", output = 0x" << output.val() << std::dec << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordSymbolicALU
 */
int
main(int argc, char* argv[]) {
  WordSymbolicALUOptions opt("WordSymbolicALU");
  opt.parse(argc,argv);
  if ((opt.steps() == 0) || (opt.steps() > 64)) {
    std::cerr << "-steps must be between 1 and 64" << std::endl;
    return 1;
  }
  if ((opt.width() < 2) || (opt.width() > 64)) {
    std::cerr << "-width must be between 2 and 64" << std::endl;
    return 1;
  }
  Script::run<WordSymbolicALU,DFS,WordSymbolicALUOptions>(opt);
  return 0;
}

// STATISTICS: example-any
