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
public:
  /// Initialize options
  explicit WordCRC16X25Options(const char* name)
    : Options(name), _bits("bits","number of symbolic input bits",20) {
    add(_bits);
  }
  /// Return number of input bits
  unsigned int get_bits(void) const { return _bits.value(); }
private:
  /// Number of input bits
  Driver::UnsignedIntOption _bits;
};

/**
 * \brief %Example: Reflected CRC-16/X-25-style preimage
 *
 * Packed message bit zero is processed first, from state 0xffff using
 * reflected polynomial 0x8408.  This models the reflected recurrence before the usual
 * X-25 final complement.  The low four final-state bits are fixed to zero.
 * Use \c -bits \c 28 for the larger profiling configuration.
 *
 * \ingroup Example
 */
class WordCRC16X25 : public Script {
public:
  /// Actual model
  explicit WordCRC16X25(const WordCRC16X25Options& opt)
    : Script(opt), message(*this,opt.get_bits()), state(*this,16) {
    WordVarArray states(*this,opt.get_bits()+1,16,0,0xffffU);
    dom(*this,states[0],0xffffU);
    // Each input bit advances the previous CRC state.
    for (unsigned int i=0; i<opt.get_bits(); i++) {
      BoolVar input_bit(*this,0,1);
      channel(*this,message,i,input_bit);
      post_step(states[i],input_bit,states[i+1]);
    }
    rel(*this,state,WRT_EQ,states[opt.get_bits()]);
    dom(*this,state,0,0xfff0U);
    branch(*this,message,WORD_VAL_LSB());
  }
  /// Constructor for cloning \a s
  WordCRC16X25(WordCRC16X25& s) : Script(s) {
    message.update(*this,s.message);
    state.update(*this,s.state);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordCRC16X25(*this);
  }
  /// Print solution
  void print(std::ostream& os) const override {
    os << "\tmessage = 0x" << std::hex << message.val()
       << ", state = 0x" << std::setw(4) << std::setfill('0') << state.val()
       << std::dec << std::setfill(' ') << std::endl;
  }
private:
  /// Symbolic input bits packed into one word
  WordVar message;
  /// Final CRC state
  WordVar state;
  void post_step(WordVar current, BoolVar input_bit, WordVar next) {
    BoolVar state_bit(*this,0,1), has_feedback(*this,0,1);
    channel(*this,current,0,state_bit);
    rel(*this,input_bit,BOT_XOR,state_bit,has_feedback);
    WordVar shifted(*this,16), polynomial(*this,16);
    WordVar zero(*this,16,0,0);
    logical_shift_right(*this,current,1,shifted);
    ite(*this,has_feedback,16,0x8408U,zero,polynomial);
    rel(*this,shifted,WOT_XOR,polynomial,next);
  }
};

/** \brief Main-function
 *  \relates WordCRC16X25
 */
int
main(int argc, char* argv[]) {
  WordCRC16X25Options opt("WordCRC16X25");
  opt.parse(argc,argv);
  if ((opt.get_bits() == 0) || (opt.get_bits() > 64)) {
    std::cerr << "-bits must be between 1 and 64" << std::endl;
    return 1;
  }
  Script::run<WordCRC16X25,DFS,WordCRC16X25Options>(opt);
  return 0;
}

// STATISTICS: example-any
