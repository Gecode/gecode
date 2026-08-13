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

#include <cstdint>
#include <iomanip>

using namespace Gecode;

namespace {

  const std::uint32_t md5_constant[64] = {
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

  const unsigned int md5_shift[64] = {
    7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
  };

  unsigned int
  message_index(unsigned int i) {
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
  md5_function(unsigned int i, std::uint32_t b, std::uint32_t c,
               std::uint32_t d) {
    if (i < 16)
      return (b & c) | (~b & d);
    if (i < 32)
      return (d & b) | (~d & c);
    if (i < 48)
      return b ^ c ^ d;
    return c ^ (b | ~d);
  }

  void
  md5_oracle(unsigned int steps, const std::uint32_t m[16],
             std::uint32_t digest[4]) {
    std::uint32_t a=0x67452301U, b=0xefcdab89U;
    std::uint32_t c=0x98badcfeU, d=0x10325476U;
    for (unsigned int i=0; i<steps; i++) {
      std::uint32_t next = b + rotate_left(a + md5_function(i,b,c,d) +
                                           md5_constant[i] +
                                           m[message_index(i)],
                                           md5_shift[i]);
      a=d; d=c; c=b; b=next;
    }
    digest[0]=a+0x67452301U; digest[1]=b+0xefcdab89U;
    digest[2]=c+0x98badcfeU; digest[3]=d+0x10325476U;
  }

}

/** \brief Options for the reduced MD5 preimage example */
class WordMD5Options : public Options {
private:
  /// Number of MD5 steps
  Driver::UnsignedIntOption _steps;
  /// Number of low message bits left unknown
  Driver::UnsignedIntOption _unknown;
public:
  /// Initialize options
  WordMD5Options(const char* n)
    : Options(n), _steps("steps","number of MD5 steps",16),
      _unknown("unknown","number of unknown message bits",132) {
    add(_steps); add(_unknown);
  }
  /// Return number of steps
  unsigned int steps(void) const { return _steps.value(); }
  /// Return number of unknown bits
  unsigned int unknown(void) const { return _unknown.value(); }
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
private:
  /// Message words
  WordVarArray message;
  /// Reduced digest
  WordVarArray digest;
  /// Number of message words containing decision bits
  unsigned int unknown_words;
public:
  /// Actual model
  WordMD5Preimage(const WordMD5Options& opt)
    : Script(opt), message(*this,16,32,0,0xffffffffU),
      digest(*this,4,32,0,0xffffffffU),
      unknown_words((opt.unknown()+31)/32) {
    const std::uint32_t known[16] = {
      0x80636261U,0,0,0,0,0,0,0,0,0,0,0,0,0,24,0
    };
    std::uint32_t target[4];
    md5_oracle(opt.steps(),known,target);

    unsigned int left=opt.unknown();
    WordVarArgs decision;
    for (unsigned int i=0; i<16; i++) {
      unsigned int n = (left < 32) ? left : 32;
      std::uint32_t mask = (n == 32) ? 0xffffffffU :
                           ((n == 0) ? 0U : ((1U << n)-1U));
      dom(*this,message[i],known[i] & ~mask,known[i] | mask);
      if (n != 0)
        decision << message[i];
      left -= n;
    }

    WordVarArray a(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray b(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray c(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray d(*this,opt.steps()+1,32,0,0xffffffffU);
    dom(*this,a[0],0x67452301U); dom(*this,b[0],0xefcdab89U);
    dom(*this,c[0],0x98badcfeU); dom(*this,d[0],0x10325476U);

    for (unsigned int i=0; i<opt.steps(); i++) {
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
      WordVar sum(*this,32), with_constant(*this,32), rotated(*this,32);
      add(*this,WordVarArgs({a[i],f,message[message_index(i)]}),sum);
      add(*this,sum,32,md5_constant[i],with_constant);
      rotate_left(*this,with_constant,md5_shift[i],rotated);
      add(*this,b[i],rotated,b[i+1]);
      rel(*this,a[i+1],WRT_EQ,d[i]);
      rel(*this,c[i+1],WRT_EQ,b[i]);
      rel(*this,d[i+1],WRT_EQ,c[i]);
    }

    add(*this,a[opt.steps()],32,0x67452301U,digest[0]);
    add(*this,b[opt.steps()],32,0xefcdab89U,digest[1]);
    add(*this,c[opt.steps()],32,0x98badcfeU,digest[2]);
    add(*this,d[opt.steps()],32,0x10325476U,digest[3]);
    for (int i=0; i<4; i++)
      dom(*this,digest[i],target[i]);

    branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordMD5Preimage(WordMD5Preimage& s)
    : Script(s), unknown_words(s.unknown_words) {
    message.update(*this,s.message);
    digest.update(*this,s.digest);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordMD5Preimage(*this);
  }
  /// Print solution
  virtual void print(std::ostream& os) const {
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
};

/** \brief Main-function
 *  \relates WordMD5Preimage
 */
int
main(int argc, char* argv[]) {
  WordMD5Options opt("WordMD5Preimage");
  opt.parse(argc,argv);
  if ((opt.steps() == 0) || (opt.steps() > 64)) {
    std::cerr << "--steps must be between 1 and 64" << std::endl;
    return 1;
  }
  unsigned int useful = (opt.steps() < 16) ? 32*opt.steps() : 512;
  if (opt.unknown() > useful) {
    std::cerr << "--unknown exceeds the message bits used by this prefix"
              << std::endl;
    return 1;
  }
  Script::run<WordMD5Preimage,DFS,WordMD5Options>(opt);
  return 0;
}

// STATISTICS: example-any
