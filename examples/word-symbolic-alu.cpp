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
public:
  enum Domain { DOMAIN_CUBE, DOMAIN_UNSIGNED, DOMAIN_SIGNED };
  /// Initialize options
  explicit WordSymbolicALUOptions(const char* name)
    : Options(name), _steps("steps","number of constructed ALU steps",8),
      _width("width","word width",18),
      _domain("word-domain","word domain representation",DOMAIN_CUBE) {
    add(_steps); add(_width); add(_domain);
    _domain.add(DOMAIN_CUBE,"cube","independently known bits");
    _domain.add(DOMAIN_UNSIGNED,"unsigned","unsigned ranked interval and cube");
    _domain.add(DOMAIN_SIGNED,"signed","signed ranked interval and cube");
  }
  /// Return number of steps
  unsigned int get_steps(void) const { return _steps.value(); }
  /// Return word width
  unsigned int get_width(void) const { return _width.value(); }
  /// Return selected Word domain representation
  WordDomainType get_domain_type(void) const {
    switch (_domain.value()) {
    case DOMAIN_UNSIGNED: return WDT_UNSIGNED;
    case DOMAIN_SIGNED: return WDT_SIGNED;
    default: return WDT_CUBE;
    }
  }
private:
  /// Number of ALU steps
  Driver::UnsignedIntOption _steps;
  /// Word width
  Driver::UnsignedIntOption _width;
  /// Word domain representation
  Driver::StringOption _domain;
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
public:
  /// Actual model
  explicit WordSymbolicALU(const WordSymbolicALUOptions& opt)
    : Script(opt), input(*this,opt.get_width(),opt.get_domain_type()),
      output(*this,opt.get_width(),opt.get_domain_type()) {
    const WordValue mask = (opt.get_width() == 64) ? ~WordValue(0) :
                           (WordValue(1) << opt.get_width())-1;
    WordVarArray states(*this,opt.get_steps()+1,opt.get_width(),opt.get_domain_type());
    dom(*this,states[0],WordValue(0x2a55aU)&mask);
    // Each instruction consumes the preceding register state.
    for (unsigned int i=0; i<opt.get_steps(); i++) {
      rel(*this,states[i+1],WRT_EQ,post_step(opt,states[i],i));
    }
    rel(*this,output,WRT_EQ,states[opt.get_steps()]);
    dom(*this,output,0,mask & ~WordValue(3));
    branch(*this,input,(opt.get_domain_type() == WDT_CUBE)
           ? WORD_VAL_MSB() : WORD_VAL_SPLIT_MIN());
  }
  /// Constructor for cloning \a s
  WordSymbolicALU(WordSymbolicALU& s) : Script(s) {
    input.update(*this,s.input);
    output.update(*this,s.output);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordSymbolicALU(*this);
  }
  /// Print solution
  void print(std::ostream& os) const override {
    os << "\tinput = 0x" << std::hex << input.val()
       << ", output = 0x" << output.val() << std::dec << std::endl;
  }
private:
  /// Symbolic input register
  WordVar input;
  /// Final output register
  WordVar output;
  WordVar post_step(const WordSymbolicALUOptions& opt,
                    WordVar current, unsigned int i) {
    const WordValue mask=Word::width_mask(opt.get_width());
    WordVar rotated(*this,opt.get_width(),opt.get_domain_type());
    WordVar next(*this,opt.get_width(),opt.get_domain_type());
    rotate_left(*this,current,i%(opt.get_width()-1)+1,rotated);
    if (i%3 == 0) {
      add(*this,rotated,input,next);
    } else if (i%3 == 1) {
      rel(*this,rotated,WOT_XOR,input,next);
    } else {
      WordVar selected(*this,opt.get_width(),opt.get_domain_type());
      rel(*this,rotated,WOT_AND,input,selected);
      add(*this,selected,opt.get_width(),WordValue(0x12345U+i)&mask,next);
    }
    return next;
  }
};

/** \brief Main-function
 *  \relates WordSymbolicALU
 */
int
main(int argc, char* argv[]) {
  WordSymbolicALUOptions opt("WordSymbolicALU");
  opt.parse(argc,argv);
  if ((opt.get_steps() == 0) || (opt.get_steps() > 64)) {
    std::cerr << "-steps must be between 1 and 64" << std::endl;
    return 1;
  }
  if ((opt.get_width() < 2) || (opt.get_width() > 64)) {
    std::cerr << "-width must be between 2 and 64" << std::endl;
    return 1;
  }
  Script::run<WordSymbolicALU,DFS,WordSymbolicALUOptions>(opt);
  return 0;
}

// STATISTICS: example-any
