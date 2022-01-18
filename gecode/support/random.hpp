/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2005
 *     Mikael Lagerkvist, 2005
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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
  RandomGenerator;

}}

// STATISTICS: support-any
