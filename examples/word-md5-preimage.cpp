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

  const std::uint32_t constants[64] = {
    0xd76aa478U,0xe8c7b756U,0x242070dbU,0xc1bdceeeU,
    0xf57c0fafU,0x4787c62aU,0xa8304613U,0xfd469501U,
    0x698098d8U,0x8b44f7afU,0xffff5bb1U,0x895cd7beU,
    0x6b901122U,0xfd987193U,0xa679438eU,0x49b40821U,
    0xf61e2562U,0xc040b340U,0x265e5a51U,0xe9b6c7aaU,
    0xd62f105dU,0x02441453U,0xd8a1e681U,0xe7d3fbc8U,
    0x21e1cde6U,0xc33707d6U,0xf4d50d87U,0x455a14edU,
    0xa9e3e905U,0xfcefa3f8U,0x676f02d9U,0x8d2a4c8aU,
    0xfffa3942U,0x8771f681U,0x6d9d6122U,0xfde5380cU,
    0xa4beea44U,0x4bdecfa9U,0xf6bb4b60U,0xbebfbc70U,
    0x289b7ec6U,0xeaa127faU,0xd4ef3085U,0x04881d05U,
    0xd9d4d039U,0xe6db99e5U,0x1fa27cf8U,0xc4ac5665U,
    0xf4292244U,0x432aff97U,0xab9423a7U,0xfc93a039U,
    0x655b59c3U,0x8f0ccc92U,0xffeff47dU,0x85845dd1U,
    0x6fa87e4fU,0xfe2ce6e0U,0xa3014314U,0x4e0811a1U,
    0xf7537e82U,0xbd3af235U,0x2ad7d2bbU,0xeb86d391U
  };

  const unsigned int shifts[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
  };

  unsigned int
  compute_message_index(unsigned int i) {
    if (i < 16)
      return i;
    if (i < 32)
      return (5*i+1) % 16;
    if (i < 48)
      return (3*i+5) % 16;
    return (7*i) % 16;
  }

  std::uint32_t
  rotate_left(std::uint32_t x, unsigned int s) {
    return (x << s) | (x >> (32-s));
  }

  std::uint32_t
  compute_round_function(unsigned int i, std::uint32_t b, std::uint32_t c,
               std::uint32_t d) {
    if (i < 16)
      return (b & c) | (~b & d);
    if (i < 32)
      return (d & b) | (~d & c);
    if (i < 48)
      return b ^ c ^ d;
    return c ^ (b | ~d);
  }

  /** Compute the reduced compression digest, including feed-forward.
   * MD5 words use little-endian byte order. See https://www.rfc-editor.org/rfc/rfc1321#section-3.4
   */
  std::array<std::uint32_t,4>
  compute_digest(unsigned int steps, const std::array<std::uint32_t,16>& message) {
    std::uint32_t a=0x67452301U, b=0xefcdab89U;
    std::uint32_t c=0x98badcfeU, d=0x10325476U;
    for (unsigned int i=0; i<steps; i++) {
      const std::uint32_t next = b + rotate_left(
        a + compute_round_function(i,b,c,d) + constants[i] +
        message[compute_message_index(i)],shifts[i]);
      a=d; d=c; c=b; b=next;
    }
    return {{a+0x67452301U,b+0xefcdab89U,c+0x98badcfeU,d+0x10325476U}};
  }

}

/** \brief Options for the reduced MD5 preimage example */
class WordMD5Options : public Options {
public:
  /// Initialize options
  explicit WordMD5Options(const char* name)
    : Options(name), _steps("steps","number of MD5 steps",16),
      _unknown("unknown","number of unknown message bits",132) {
    add(_steps); add(_unknown);
  }
  /// Return number of steps
  unsigned int get_steps(void) const { return _steps.value(); }
  /// Return number of unknown bits
  unsigned int get_unknown(void) const { return _unknown.value(); }
private:
  /// Number of MD5 steps
  Driver::UnsignedIntOption _steps;
  /// Number of low message bits left unknown
  Driver::UnsignedIntOption _unknown;
};

/**
 * \brief %Example: Reduced MD5 preimage
 *
 * This is the standard MD5 compression function applied to the padded
 * one-block message "abc".  The first \c unknown message bits are relaxed,
 * and the model recovers a message with the same digest after \c steps.
 * For example, \c -steps \c 16 \c -unknown \c 148 is a larger case.
 *
 * \ingroup Example
 */
class WordMD5Preimage : public Script {
public:
  /// Actual model
  explicit WordMD5Preimage(const WordMD5Options& opt)
    : Script(opt), message(*this,16,32,0,0xffffffffU),
      digest(*this,4,32,0,0xffffffffU),
      unknown_words((opt.get_unknown()+31)/32) {
    const std::array<std::uint32_t,16> known = {{
      0x80636261U,0,0,0,0,0,0,0,0,0,0,0,0,0,24,0
    }};
    const auto target=compute_digest(opt.get_steps(),known);

    const WordVarArgs decisions=restrict_message(known,opt.get_unknown());
    const auto final_state=post_rounds(opt.get_steps());
    bind_digest(final_state,target);
    branch(*this,decisions,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordMD5Preimage(WordMD5Preimage& s)
    : Script(s), unknown_words(s.unknown_words) {
    message.update(*this,s.message);
    digest.update(*this,s.digest);
  }
  /// Copy during cloning
  Space* copy(void) override {
    return new WordMD5Preimage(*this);
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
    for (int i=0; i<4; i++)
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
  std::array<WordVar,4> post_rounds(unsigned int steps) {
    // Each round consumes the preceding round's state.
    WordVarArray a(*this,steps+1,32,0,0xffffffffU);
    WordVarArray b(*this,steps+1,32,0,0xffffffffU);
    WordVarArray c(*this,steps+1,32,0,0xffffffffU);
    WordVarArray d(*this,steps+1,32,0,0xffffffffU);
    dom(*this,a[0],0x67452301U); dom(*this,b[0],0xefcdab89U);
    dom(*this,c[0],0x98badcfeU); dom(*this,d[0],0x10325476U);

    for (unsigned int i=0; i<steps; i++) {
      WordVar f(*this,32), x(*this,32), y(*this,32), z(*this,32);
      if (i < 16) {
        rel(*this,b[i],WOT_AND,c[i],x);
        complement(*this,b[i],y);
        rel(*this,y,WOT_AND,d[i],z);
        rel(*this,x,WOT_OR,z,f);
      } else if (i < 32) {
        rel(*this,d[i],WOT_AND,b[i],x);
        complement(*this,d[i],y);
        rel(*this,y,WOT_AND,c[i],z);
        rel(*this,x,WOT_OR,z,f);
      } else if (i < 48) {
        rel(*this,WOT_XOR,WordVarArgs({b[i],c[i],d[i]}),f);
      } else {
        complement(*this,d[i],x);
        rel(*this,b[i],WOT_OR,x,y);
        rel(*this,c[i],WOT_XOR,y,f);
      }
      WordVar round_constant(*this,32,constants[i],constants[i]);
      WordVar with_constant(*this,32), rotated(*this,32);
      add(*this,WordVarArgs({a[i],f,message[compute_message_index(i)],
                             round_constant}),with_constant);
      rotate_left(*this,with_constant,shifts[i],rotated);
      add(*this,b[i],rotated,b[i+1]);
      rel(*this,a[i+1],WRT_EQ,d[i]);
      rel(*this,c[i+1],WRT_EQ,b[i]);
      rel(*this,d[i+1],WRT_EQ,c[i]);
    }
    return {{a[steps],b[steps],c[steps],d[steps]}};
  }
  void bind_digest(const std::array<WordVar,4>& final_state,
                   const std::array<std::uint32_t,4>& target) {
    add(*this,final_state[0],32,0x67452301U,digest[0]);
    add(*this,final_state[1],32,0xefcdab89U,digest[1]);
    add(*this,final_state[2],32,0x98badcfeU,digest[2]);
    add(*this,final_state[3],32,0x10325476U,digest[3]);
    for (int i=0; i<4; i++)
      dom(*this,digest[i],target[i]);
  }
};

/** \brief Main-function
 *  \relates WordMD5Preimage
 */
int
main(int argc, char* argv[]) {
  WordMD5Options opt("WordMD5Preimage");
  opt.parse(argc,argv);
  if ((opt.get_steps() == 0) || (opt.get_steps() > 64)) {
    std::cerr << "--steps must be between 1 and 64" << std::endl;
    return 1;
  }
  const unsigned int used_bits = (opt.get_steps() < 16) ? 32*opt.get_steps() : 512;
  if (opt.get_unknown() > used_bits) {
    std::cerr << "--unknown exceeds the message bits used by this prefix"
              << std::endl;
    return 1;
  }
  Script::run<WordMD5Preimage,DFS,WordMD5Options>(opt);
  return 0;
}

// STATISTICS: example-any
