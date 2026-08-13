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

  std::uint32_t
  rotate_left(std::uint32_t x, unsigned int s) {
    return (x << s) | (x >> (32-s));
  }

  std::uint32_t
  sha1_function(unsigned int i, std::uint32_t b, std::uint32_t c,
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
  sha1_constant(unsigned int i) {
    if (i < 20) return 0x5a827999U;
    if (i < 40) return 0x6ed9eba1U;
    if (i < 60) return 0x8f1bbcdcU;
    return 0xca62c1d6U;
  }

  void
  sha1_oracle(unsigned int steps, const std::uint32_t m[16],
              std::uint32_t digest[5]) {
    std::uint32_t w[80];
    for (int i=0; i<16; i++)
      w[i]=m[i];
    for (int i=16; i<80; i++)
      w[i]=rotate_left(w[i-3]^w[i-8]^w[i-14]^w[i-16],1);
    std::uint32_t a=0x67452301U, b=0xefcdab89U;
    std::uint32_t c=0x98badcfeU, d=0x10325476U, e=0xc3d2e1f0U;
    for (unsigned int i=0; i<steps; i++) {
      std::uint32_t next = rotate_left(a,5) + sha1_function(i,b,c,d) +
                           e + sha1_constant(i) + w[i];
      e=d; d=c; c=rotate_left(b,30); b=a; a=next;
    }
    digest[0]=a+0x67452301U; digest[1]=b+0xefcdab89U;
    digest[2]=c+0x98badcfeU; digest[3]=d+0x10325476U;
    digest[4]=e+0xc3d2e1f0U;
  }

}

/** \brief Options for the reduced SHA-1 preimage example */
class WordSHA1Options : public Options {
private:
  /// Number of SHA-1 steps
  Driver::UnsignedIntOption _steps;
  /// Number of low message bits left unknown
  Driver::UnsignedIntOption _unknown;
public:
  /// Initialize options
  WordSHA1Options(const char* n)
    : Options(n), _steps("steps","number of SHA-1 steps",16),
      _unknown("unknown","number of unknown message bits",164) {
    add(_steps); add(_unknown);
  }
  /// Return number of steps
  unsigned int steps(void) const { return _steps.value(); }
  /// Return number of unknown bits
  unsigned int unknown(void) const { return _unknown.value(); }
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
private:
  /// Message words
  WordVarArray message;
  /// Reduced digest
  WordVarArray digest;
  /// Number of message words containing decision bits
  unsigned int unknown_words;
public:
  /// Actual model
  WordSHA1Preimage(const WordSHA1Options& opt)
    : Script(opt), message(*this,16,32,0,0xffffffffU),
      digest(*this,5,32,0,0xffffffffU),
      unknown_words((opt.unknown()+31)/32) {
    const std::uint32_t known[16] = {
      0x61626380U,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24
    };
    std::uint32_t target[5];
    sha1_oracle(opt.steps(),known,target);

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

    WordVarArray w(*this,opt.steps(),32,0,0xffffffffU);
    for (unsigned int i=0; i<opt.steps(); i++) {
      if (i < 16) {
        rel(*this,w[i],WRT_EQ,message[i]);
      } else {
        WordVar x(*this,32);
        rel(*this,WOT_XOR,
            WordVarArgs({w[i-3],w[i-8],w[i-14],w[i-16]}),x);
        rotate_left(*this,x,1,w[i]);
      }
    }

    WordVarArray a(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray b(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray c(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray d(*this,opt.steps()+1,32,0,0xffffffffU);
    WordVarArray e(*this,opt.steps()+1,32,0,0xffffffffU);
    dom(*this,a[0],0x67452301U); dom(*this,b[0],0xefcdab89U);
    dom(*this,c[0],0x98badcfeU); dom(*this,d[0],0x10325476U);
    dom(*this,e[0],0xc3d2e1f0U);

    for (unsigned int i=0; i<opt.steps(); i++) {
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
      WordVar rotated(*this,32), sum(*this,32);
      rotate_left(*this,a[i],5,rotated);
      add(*this,WordVarArgs({rotated,f,e[i],w[i]}),sum);
      add(*this,sum,32,sha1_constant(i),a[i+1]);
      rel(*this,b[i+1],WRT_EQ,a[i]);
      rotate_left(*this,b[i],30,c[i+1]);
      rel(*this,d[i+1],WRT_EQ,c[i]);
      rel(*this,e[i+1],WRT_EQ,d[i]);
    }

    add(*this,a[opt.steps()],32,0x67452301U,digest[0]);
    add(*this,b[opt.steps()],32,0xefcdab89U,digest[1]);
    add(*this,c[opt.steps()],32,0x98badcfeU,digest[2]);
    add(*this,d[opt.steps()],32,0x10325476U,digest[3]);
    add(*this,e[opt.steps()],32,0xc3d2e1f0U,digest[4]);
    for (int i=0; i<5; i++)
      dom(*this,digest[i],target[i]);

    branch(*this,decision,WORD_VAR_NONE(),WORD_VAL_MSB());
  }
  /// Constructor for cloning \a s
  WordSHA1Preimage(WordSHA1Preimage& s)
    : Script(s), unknown_words(s.unknown_words) {
    message.update(*this,s.message);
    digest.update(*this,s.digest);
  }
  /// Copy during cloning
  virtual Space* copy(void) {
    return new WordSHA1Preimage(*this);
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
    for (int i=0; i<5; i++)
      os << " " << std::setw(8) << digest[i].val();
    os << std::dec << std::setfill(' ') << std::endl;
  }
};

/** \brief Main-function
 *  \relates WordSHA1Preimage
 */
int
main(int argc, char* argv[]) {
  WordSHA1Options opt("WordSHA1Preimage");
  opt.parse(argc,argv);
  if ((opt.steps() == 0) || (opt.steps() > 80)) {
    std::cerr << "--steps must be between 1 and 80" << std::endl;
    return 1;
  }
  unsigned int useful = (opt.steps() < 16) ? 32*opt.steps() : 512;
  if (opt.unknown() > useful) {
    std::cerr << "--unknown exceeds the message bits used by this prefix"
              << std::endl;
    return 1;
  }
  Script::run<WordSHA1Preimage,DFS,WordSHA1Options>(opt);
  return 0;
}

// STATISTICS: example-any
