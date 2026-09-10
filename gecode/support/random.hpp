/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.dev>
 *     Mikael Zayenz Lagerkvist <lagerkvist@gecode.dev>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Zayenz Lagerkvist, 2005
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
 *
 */

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace Gecode { namespace Support {

  /** \brief Template for linear congruential generators
   *
   * This class template defines a simple class for linear
   * congruential generators.
   *
   * \ingroup FuncSupport
   */
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  class LinearCongruentialGenerator {
  private:
    /// The maximum size of random numbers generated.
    static constexpr unsigned int max_value = 1UL<<31;
    /// Current seed value
    unsigned int s;
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    unsigned int u(unsigned int n);
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    unsigned long long int ull(unsigned long long int n);
  public:
    /// Set the current seed to \a s
    void seed(unsigned int s);
    /// Construct the generator instance with seed \a s
    LinearCongruentialGenerator(unsigned int s = 1);
    /// Return current seed
    unsigned int seed(void) const;
    /// Generate next number in series
    unsigned int next(void);
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    template<class Type>
    Type operator ()(Type n);
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    int operator ()(int n);
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    unsigned int operator ()(unsigned int n);
    /// Returns a random integer from the interval \f$[0\ldots n)\f$
    long long int operator ()(long long int n);
    /// Report size occupied
    size_t size(void) const;

    // Interface for conforming to C++ UniformRandomBitGenerator
    /// Type of the produced values
    typedef unsigned int result_type;
    /// Minimum value that may be produced when no bound is specified
    static constexpr result_type min() { return 0; }
    /// Maximum value that may be produced when no bound is specified
    static constexpr result_type max() { return max_value; }
    /// Returns a random integer from the interval \f$[0\ldots max()]\f$
    result_type operator()() { return next(); }
  };

  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline unsigned int
  LinearCongruentialGenerator<m,a,q,r>::next(void) {
    s = a*(s%q) - r*(s/q);
    unsigned int res = s;
    if (s==0) s = 1;
    return res;
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline void
  LinearCongruentialGenerator<m,a,q,r>::seed(unsigned int _s) {
    s = _s % m;
    if (s == 0) s = 1;
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline
  LinearCongruentialGenerator<m,a,q,r>::
  LinearCongruentialGenerator(unsigned int _s) {
    seed(_s);
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline unsigned int
  LinearCongruentialGenerator<m,a,q,r>::seed(void) const {
    return s;
  }

  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline unsigned int
  LinearCongruentialGenerator<m,a,q,r>::u(unsigned int n) {
    unsigned int x1 = next() & ((1U<<16)-1U);
    unsigned int x2 = next() & ((1U<<16)-1U);
    if (n < 2)
      return 0;
    double d = static_cast<double>(((x1<<16) | x2) % max_value) / max_value;
    unsigned int val = static_cast<unsigned int>(n * d);
    return (val < n) ? val : (n-1);
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline unsigned long long int
  LinearCongruentialGenerator<m,a,q,r>::ull(unsigned long long int n) {
    if (n <= UINT_MAX)
      return u(static_cast<unsigned int>(n));
    unsigned long long int x1 = next() & ((1LLU<<16)-1LLU);
    unsigned long long int x2 = next() & ((1LLU<<16)-1LLU);
    unsigned long long int x3 = next() & ((1LLU<<16)-1LLU);
    unsigned long long int x4 = next() & ((1LLU<<16)-1LLU);
    if (n < 2)
      return 0;
    return ((x1 << 48) | (x2 << 32) | (x3 << 16) | x4) % n;
  }

  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  template<class Type>
  forceinline Type
  LinearCongruentialGenerator<m,a,q,r>::operator ()(Type n) {
    return static_cast<Type>(ull(static_cast<unsigned long long int>(n)));
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline unsigned int
  LinearCongruentialGenerator<m,a,q,r>::operator ()(unsigned int n) {
    return u(n);
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline int
  LinearCongruentialGenerator<m,a,q,r>::operator ()(int n) {
    return (n < 0) ? 0 :
      static_cast<int>(u(static_cast<unsigned int>(n)));
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline long long int
  LinearCongruentialGenerator<m,a,q,r>::operator ()(long long int  n) {
    return (n < 0) ? 0 :
      static_cast<long long int>
      (ull(static_cast<unsigned long long int>(n)));
  }
  template<unsigned int m, unsigned int a, unsigned int q, unsigned int r>
  forceinline size_t
  LinearCongruentialGenerator<m,a,q,r>::size(void) const {
    return sizeof(LinearCongruentialGenerator<m,a,q,r>);
  }


  /** \brief Default values for linear congruential generator
   *
   * While this pseudo-random number generator is not a good source of
   * randomness, it is still an acceptable choice for many
   * applications. The choice of values is taken from D. E. Knuth,
   * The Art of Computer Programming, Vol 2, Seminumerical Algorithms,
   * 3rd edition.
   *
   * \ingroup FuncSupport
   */
  typedef LinearCongruentialGenerator<2147483647, 48271, 44488, 3399>
  LegacyRandomGenerator;

  /// Parse a decimal or hexadecimal 64-bit seed without truncation.
  inline uint64_t
  random_seed(const std::string& text) {
    const char* first = text.data();
    const char* last = first + text.size();
    int base = 10;
    if ((text.size() > 2) && (text[0] == '0') &&
        ((text[1] == 'x') || (text[1] == 'X'))) {
      first += 2;
      base = 16;
    }
    uint64_t value;
    auto r = std::from_chars(first,last,value,base);
    if ((r.ec != std::errc()) || (r.ptr != last))
      throw std::invalid_argument("Invalid 64-bit random seed");
    return value;
  }

  /** \brief Splittable SplitMix with two 64-bit state words
   *
   * Implements the SplitMix design of Steele, Lea, and Flood (OOPSLA 2014),
   * using Stafford's Mix13 output permutation and the MurmurHash3 finalizer
   * for gamma selection. Indexed splitting returns the child of the (a+1)th
   * successive split without changing the parent. For a 32-bit alternative
   * index, the inputs s + (2*a+1)*gamma are distinct: gamma is odd and the
   * offsets span less than 2^64. Mix13 is bijective, so child states differ.
   *
   * \ingroup FuncSupport
   */
  class SplitMix {
  public:
    using State = std::array<uint64_t,2>;
  private:
    State s;
    static uint64_t mix(uint64_t z) {
      z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
      z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
      return z ^ (z >> 31);
    }
    static uint64_t gamma(uint64_t z) {
      z = (z ^ (z >> 33)) * UINT64_C(0xff51afd7ed558ccd);
      z = (z ^ (z >> 33)) * UINT64_C(0xc4ceb9fe1a85ec53);
      z = (z ^ (z >> 33)) | 1;
      unsigned int n = 0;
      for (uint64_t bits = z ^ (z >> 1); bits; bits &= bits-1)
        ++n;
      return (n < 24) ? z ^ UINT64_C(0xaaaaaaaaaaaaaaaa) : z;
    }
  public:
    explicit SplitMix(uint64_t value=1) { seed(value); }
    static const char* name(void) { return "splitmix-v1"; }
    static constexpr uint64_t min(void) { return 0; }
    static constexpr uint64_t max(void) { return UINT64_MAX; }
    void seed(uint64_t value) {
      s = {{value, UINT64_C(0x9e3779b97f4a7c15)}};
    }
    State state(void) const { return s; }
    void state(const State& value) {
      if (!(value[1] & 1))
        throw std::invalid_argument("SplitMix increment must be odd");
      s = value;
    }
    uint64_t next(void) { return mix(s[0] += s[1]); }
    SplitMix split(uint32_t alternative) const {
      uint64_t first = s[0] + (2*uint64_t(alternative)+1)*s[1];
      SplitMix child;
      child.s = {{mix(first),gamma(first+s[1])}};
      return child;
    }
  };

  /** \brief One-word xorshift64* engine for standalone use
   *
   * Uses shifts 12, 25, 27 and Vigna's multiplier. Zero seeds map to one;
   * restoring zero state is an error. This engine does not provide splitting.
   * \ingroup FuncSupport
   */
  class Xorshift64Star {
  public:
    using State = std::array<uint64_t,1>;
  private:
    uint64_t s;
  public:
    explicit Xorshift64Star(uint64_t value=1) { seed(value); }
    static const char* name(void) { return "xorshift64star-v1"; }
    static constexpr uint64_t min(void) { return 1; }
    static constexpr uint64_t max(void) { return UINT64_MAX; }
    void seed(uint64_t value) { s = value ? value : 1; }
    State state(void) const { return {{s}}; }
    void state(const State& value) {
      if (!value[0])
        throw std::invalid_argument("Xorshift64* state must be nonzero");
      s = value[0];
    }
    uint64_t next(void) {
      s ^= s >> 12;
      s ^= s << 25;
      s ^= s >> 27;
      return s * UINT64_C(2685821657736338717);
    }
  };

  /** \brief Value-type generator with reproducible bounded draws and state
   *
   * Engine supplies a State array of 64-bit words, name(), seed(), state()
   * getter/setter, and next() over [0,UINT64_MAX] or [1,UINT64_MAX]. Search
   * engines additionally supply split(uint32_t) const. No state-size limit
   * is imposed. Copying a generator preserves its exact state.
   * \ingroup FuncSupport
   */
  template<class Engine>
  class Random {
  private:
    Engine e;
  public:
    using State = typename Engine::State;
    using result_type = uint64_t;
    static_assert(Engine::max() == UINT64_MAX && Engine::min() <= 1,
                  "Random engine must generate full or nonzero 64-bit words");
    explicit Random(uint64_t seed=1) : e(seed) {}
    explicit Random(const Engine& engine) : e(engine) {}
    static constexpr result_type min(void) { return Engine::min(); }
    static constexpr result_type max(void) { return Engine::max(); }
    static const char* name(void) { return Engine::name(); }
    void seed(uint64_t value) { e.seed(value); }
    State state(void) const { return e.state(); }
    void state(const State& value) { e.state(value); }
    result_type next(void) { return e.next(); }
    result_type operator ()(void) { return next(); }
    size_t size(void) const { return sizeof(*this); }
    Random split(uint32_t alternative) const {
      return Random(e.split(alternative));
    }
    /// Bounds <= 1 return zero without consuming a draw.
    template<class Type>
    Type operator ()(Type bound) {
      static_assert(std::is_integral<Type>::value && sizeof(Type) <= 8,
                    "Random bound must be an integer of at most 64 bits");
      if (bound <= 1)
        return 0;
      uint64_t n = static_cast<uint64_t>(bound);
      uint64_t value;
      if constexpr (Engine::min() == 0) {
        // Accept an exact multiple of n values from the 2^64-value source.
        uint64_t threshold = (uint64_t(0)-n) % n;
        do { value = next(); } while (value < threshold);
      } else {
        // Nonzero engines have 2^64-1 values, not 2^64.
        uint64_t limit = UINT64_MAX - (UINT64_MAX % n);
        do { value = next()-1; } while (value >= limit);
      }
      return static_cast<Type>(value % n);
    }
    /// Canonical identifier followed by fixed-width hexadecimal state words.
    std::string state_string(void) const {
      std::string text(name());
      constexpr char digits[] = "0123456789abcdef";
      for (uint64_t word : state()) {
        text += ':';
        for (int shift=60; shift>=0; shift-=4)
          text += digits[(word >> shift) & 15];
      }
      return text;
    }
    /// Restore full state, rejecting incompatible identifiers or invalid words.
    void state(const std::string& text) {
      const std::string prefix = std::string(name()) + ':';
      State words{};
      if ((text.compare(0,prefix.size(),prefix) != 0) ||
          (text.size() != prefix.size()+17*words.size()-1))
        throw std::invalid_argument("Invalid or incompatible random state");
      size_t pos = prefix.size();
      for (size_t i=0; i<words.size(); ++i) {
        const char* first = text.data()+pos;
        auto r = std::from_chars(first,first+16,words[i],16);
        if ((r.ec != std::errc()) || (r.ptr != first+16) ||
            ((i+1<words.size()) && (text[pos+16] != ':')))
          throw std::invalid_argument("Invalid random state word");
        pos += 17;
      }
      state(words);
    }
  };

  /// Default generator; full state consists of two 64-bit words.
  using RandomGenerator = Random<SplitMix>;

}}

// STATISTICS: support-any
