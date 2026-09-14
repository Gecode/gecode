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

#include <array>
#include <cstdint>
#include <iomanip>

using namespace Gecode;

namespace {

  std::uint32_t
  rotate_left(std::uint32_t x, unsigned int s) {
    return (x << s) | (x >> (32-s));
  }

  std::uint32_t
  compute_round_function(unsigned int i, std::uint32_t b, std::uint32_t c,
                std::uint32_t d) {
    if (i < 20)
      return (b & c) | (~b & d);
    if (i < 40)
      return b ^ c ^ d;
    if (i < 60)
      return (b & c) | (b & d) | (c & d);
    return b ^ c ^ d;
  }

  std::uint32_t
  get_round_constant(unsigned int i) {
    if (i < 20) return 0x5a827999U;
    if (i < 40) return 0x6ed9eba1U;
    if (i < 60) return 0x8f1bbcdcU;
    return 0xca62c1d6U;
  }

  /** Compute the reduced compression digest, including feed-forward.
   * SHA-1 words use big-endian byte order. See https://nvlpubs.nist.gov/nistpubs/FIPS/NIST.FIPS.180-4.pdf, section 6.1.2
   */
  std::array<std::uint32_t,5>
  compute_digest(unsigned int steps, const std::array<std::uint32_t,16>& message) {
    std::uint32_t w[80];
    for (int i=0; i<16; i++)
      w[i]=message[i];
    for (int i=16; i<80; i++)
      w[i]=rotate_left(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    std::uint32_t a=0x67452301U, b=0xefcdab89U;
    std::uint32_t c=0x98badcfeU, d=0x10325476U, e=0xc3d2e1f0U;
    for (unsigned int i=0; i<steps; i++) {
      const std::uint32_t next = rotate_left(a,5) + compute_round_function(i,b,c,d) +
                           e + get_round_constant(i) + w[i];
      e=d; d=c; c=rotate_left(b,30); b=a; a=next;
    }
    return {{a+0x67452301U,b+0xefcdab89U,c+0x98badcfeU,d+0x10325476U,e+0xc3d2e1f0U}};
  }

}

/** \brief Options for the reduced SHA-1 preimage example */
class WordSHA1Options : public Options {
public:
  /// Initialize options
  explicit WordSHA1Options(const char* name)
    : Options(name), _steps("steps","number of SHA-1 steps",16),
      _unknown("unknown","number of unknown message bits",164) {
    add(_steps); add(_unknown);
  }
  /// Return number of steps
  unsigned int get_steps(void) const { return _steps.value(); }
  /// Return number of unknown bits
  unsigned int get_unknown(void) const { return _unknown.value(); }
private:
  /// Number of SHA-1 steps
  Driver::UnsignedIntOption _steps;
  /// Number of low message bits left unknown
  Driver::UnsignedIntOption _unknown;
};

/**
 * \brief %Example: Reduced SHA-1 preimage
 *
 * This is the standard SHA-1 compression function applied to the padded
 * one-block message "abc".  The first \c unknown message bits are relaxed,
 * and the model recovers a message with the same digest after \c steps.
 * For example, \c -steps \c 16 \c -unknown \c 180 is a larger case.
 *
 * \ingroup Example
 */
class WordSHA1Preimage : public Script {
public:
  /// Actual model
  explicit WordSHA1Preimage(const WordSHA1Options& opt)
    : Script(opt), message(*this,16,32,0,0xffffffffU),
      digest(*this,5,32,0,0xffffffffU),
      unknown_words((opt.get_unknown()+31)/32) {
    const std::array<std::uint32_t,16> known = {{
      0x61626380U,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24
    }};
    const auto target=compute_digest(opt.get_steps(),known);

    const WordVarArgs decisions=restrict_message(known,opt.get_unknown());
    const WordVarArray schedule=build_schedule(opt.get_steps());
    const auto final_state=post_rounds(opt.get_steps(),schedule);
    bind_digest(final_state,target);
    branch(*this,decisions,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordSHA1Preimage(WordSHA1Preimage& s)
    : Script(s), unknown_words(s.unknown_words) {
    message.update(*this,s.message);
    digest.update(*this,s.digest);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordSHA1Preimage(*this);
  }
  /// Print solution
  void print(std::ostream& os) const override {
    os << "\tmessage = {" << std::hex << std::setfill('0');
    for (unsigned int i=0; i<unknown_words; i++) {
      if (i != 0)
        os << ", ";
      os << "0x" << std::setw(8) << message[i].val();
    }
    os << "}, digest =";
    for (int i=0; i<5; i++)
      os << " " << std::setw(8) << digest[i].val();
    os << std::dec << std::setfill(' ') << std::endl;
  }
private:
  /// Message words
  WordVarArray message;
  /// Reduced digest
  WordVarArray digest;
  /// Number of message words containing decision bits
  const unsigned int unknown_words;
  WordVarArgs restrict_message(const std::array<std::uint32_t,16>& known,
                               unsigned int unknown) {
    unsigned int remaining=unknown;
    WordVarArgs decisions;
    for (unsigned int i=0; i<16; i++) {
      const unsigned int bits = (remaining < 32) ? remaining : 32;
      const std::uint32_t mask = (bits == 32) ? 0xffffffffU :
                           ((bits == 0) ? 0U : ((1U << bits)-1U));
      dom(*this,message[i],known[i] & ~mask,known[i] | mask);
      if (bits != 0)
        decisions << message[i];
      remaining -= bits;
    }
    return decisions;
  }
  WordVarArray build_schedule(unsigned int steps) {
    WordVarArray w(*this,steps,32,0,0xffffffffU);
    for (unsigned int i=0; i<steps; i++) {
      if (i < 16) {
        rel(*this,w[i],WRT_EQ,message[i]);
      } else {
        WordVar x(*this,32);
        rel(*this,WOT_XOR,
            WordVarArgs({w[i-3],w[i-8],w[i-14],w[i-16]}),x);
        rotate_left(*this,x,1,w[i]);
      }
    }
    return w;
  }
  std::array<WordVar,5> post_rounds(unsigned int steps, const WordVarArray& w) {
    // Each round consumes the preceding round's state.
    WordVarArray a(*this,steps+1,32,0,0xffffffffU);
    WordVarArray b(*this,steps+1,32,0,0xffffffffU);
    WordVarArray c(*this,steps+1,32,0,0xffffffffU);
    WordVarArray d(*this,steps+1,32,0,0xffffffffU);
    WordVarArray e(*this,steps+1,32,0,0xffffffffU);
    dom(*this,a[0],0x67452301U); dom(*this,b[0],0xefcdab89U);
    dom(*this,c[0],0x98badcfeU); dom(*this,d[0],0x10325476U);
    dom(*this,e[0],0xc3d2e1f0U);

    for (unsigned int i=0; i<steps; i++) {
      WordVar f(*this,32), x(*this,32), y(*this,32), z(*this,32);
      if (i < 20) {
        rel(*this,b[i],WOT_AND,c[i],x);
        complement(*this,b[i],y);
        rel(*this,y,WOT_AND,d[i],z);
        rel(*this,x,WOT_OR,z,f);
      } else if ((i < 40) || (i >= 60)) {
        rel(*this,WOT_XOR,WordVarArgs({b[i],c[i],d[i]}),f);
      } else {
        rel(*this,b[i],WOT_AND,c[i],x);
        rel(*this,b[i],WOT_AND,d[i],y);
        rel(*this,c[i],WOT_AND,d[i],z);
        rel(*this,WOT_OR,WordVarArgs({x,y,z}),f);
      }
      WordVar round_constant(*this,32,get_round_constant(i),get_round_constant(i));
      WordVar rotated(*this,32);
      rotate_left(*this,a[i],5,rotated);
      add(*this,WordVarArgs({rotated,f,e[i],w[i],round_constant}),a[i+1]);
      rel(*this,b[i+1],WRT_EQ,a[i]);
      rotate_left(*this,b[i],30,c[i+1]);
      rel(*this,d[i+1],WRT_EQ,c[i]);
      rel(*this,e[i+1],WRT_EQ,d[i]);
    }
    return {{a[steps],b[steps],c[steps],d[steps],e[steps]}};
  }
  void bind_digest(const std::array<WordVar,5>& final_state,
                   const std::array<std::uint32_t,5>& target) {
    add(*this,final_state[0],32,0x67452301U,digest[0]);
    add(*this,final_state[1],32,0xefcdab89U,digest[1]);
    add(*this,final_state[2],32,0x98badcfeU,digest[2]);
    add(*this,final_state[3],32,0x10325476U,digest[3]);
    add(*this,final_state[4],32,0xc3d2e1f0U,digest[4]);
    for (int i=0; i<5; i++)
      dom(*this,digest[i],target[i]);
  }
};

/** \brief Main-function
 *  \relates WordSHA1Preimage
 */
int
main(int argc, char* argv[]) {
  WordSHA1Options opt("WordSHA1Preimage");
  opt.parse(argc,argv);
  if ((opt.get_steps() == 0) || (opt.get_steps() > 80)) {
    std::cerr << "--steps must be between 1 and 80" << std::endl;
    return 1;
  }
  const unsigned int used_bits = (opt.get_steps() < 16) ? 32*opt.get_steps() : 512;
  if (opt.get_unknown() > used_bits) {
    std::cerr << "--unknown exceeds the message bits used by this prefix"
              << std::endl;
    return 1;
  }
  Script::run<WordSHA1Preimage,DFS,WordSHA1Options>(opt);
  return 0;
}

// STATISTICS: example-any
