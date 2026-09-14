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

/** \brief Options for the reflected CRC-16/X-25-style example */
class WordCRC16X25Options : public Options {
private:
  /// Number of input bits
  Driver::UnsignedIntOption _bits;
public:
  /// Initialize options
  WordCRC16X25Options(const char* n)
    : Options(n), _bits("bits","number of symbolic input bits",20) {
    add(_bits);
  }
  /// Return number of input bits
  unsigned int bits(void) const { return _bits.value(); }
};

/**
 * \brief %Example: Reflected CRC-16/X-25-style preimage
 *
 * A symbolic bit stream is processed from state 0xffff using reflected
 * polynomial 0x8408.  This models the reflected recurrence before the usual
 * X-25 final complement.  The low four final-state bits are fixed to zero.
 * Use \c -bits \c 28 for the larger profiling configuration.
 *
 * \ingroup Example
 */
class WordCRC16X25 : public Script {
private:
  /// Symbolic input bits packed into one word
  WordVar message;
  /// Final CRC state
  WordVar state;
public:
  /// Actual model
  WordCRC16X25(const WordCRC16X25Options& opt)
    : Script(opt), message(*this,opt.bits()), state(*this,16) {
    WordVarArray s(*this,opt.bits()+1,16,0,0xffffU);
    dom(*this,s[0],0xffffU);
    for (unsigned int i=0; i<opt.bits(); i++) {
      BoolVar input(*this,0,1), low(*this,0,1), feedback(*this,0,1);
      channel(*this,message,i,input);
      channel(*this,s[i],0,low);
      rel(*this,input,BOT_XOR,low,feedback);
      WordVar shifted(*this,16), polynomial(*this,16);
      WordVar zero(*this,16,0,0);
      logical_shift_right(*this,s[i],1,shifted);
      ite(*this,feedback,16,0x8408U,zero,polynomial);
      rel(*this,shifted,WOT_XOR,polynomial,s[i+1]);
    }
    rel(*this,state,WRT_EQ,s[opt.bits()]);
    dom(*this,state,0,0xfff0U);
    branch(*this,message,WORD_VAL_LSB());
  }
  /// Constructor for cloning \a s
  WordCRC16X25(WordCRC16X25& s) : Script(s) {
    message.update(*this,s.message);
    state.update(*this,s.state);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordCRC16X25(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
    os << "\tmessage = 0x" << std::hex << message.val()
       << ", state = 0x" << std::setw(4) << std::setfill('0') << state.val()
       << std::dec << std::setfill(' ') << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordCRC16X25
 */
int
main(int argc, char* argv[]) {
  WordCRC16X25Options opt("WordCRC16X25");
  opt.parse(argc,argv);
  if ((opt.bits() == 0) || (opt.bits() > 64)) {
    std::cerr << "-bits must be between 1 and 64" << std::endl;
    return 1;
  }
  Script::run<WordCRC16X25,DFS,WordCRC16X25Options>(opt);
  return 0;
}

// STATISTICS: example-any
