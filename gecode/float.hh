/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Guido Tack <tack@gecode.org>
 *     Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Contributing authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *     David Rijsman <David.Rijsman@quintiq.com>
 *
 *  Copyright:
 *     David Rijsman, 2009
 *     Mikael Lagerkvist, 2006
 *     Christian Schulte, 2002
 *     Guido Tack, 2004
 *     Vincent Barichard, 2012
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
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

#ifndef __GECODE_FLOAT_HH__
#define __GECODE_FLOAT_HH__

#include <climits>
#include <cfloat>
#include <iostream>

#include <gecode/kernel.hh>
#include <gecode/int.hh>

/*
 * Configure linking
 *
 */
#if !defined(GECODE_STATIC_LIBS) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_FLOAT
#define GECODE_FLOAT_EXPORT __declspec( dllexport )
#else
#define GECODE_FLOAT_EXPORT __declspec( dllimport )
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_FLOAT_EXPORT __attribute__ ((visibility("default")))
#else
#define GECODE_FLOAT_EXPORT
#endif

#endif

// Configure auto-linking
#ifndef GECODE_BUILD_FLOAT
#define GECODE_LIBRARY_NAME "Float"
#include <gecode/support/auto-link.hpp>
#endif

// Include interval implementation
#include <gecode/third-party/boost/numeric/interval.hpp>

/**
 * \namespace Gecode::Float
 * \brief Floating point numbers
 *
 * The Gecode::Float namespace contains all functionality required
 * to program propagators and branchers for floating point numbers.
 * In addition, all propagators and branchers for floating point
 * numbers provided by %Gecode are contained as nested namespaces.
 *
 */

#include <gecode/float/exception.hpp>

namespace Gecode {

  /**
   * \brief Floating point number base type
   *
   * This type defines the interval bounds used for representing floating
   * point values.
   * \ingroup TaskModelFloatVars
   */
  typedef double FloatNum;

  /// Return lower bound of \f$\pi/2\f$
  FloatNum pi_half_lower(void);
  /// Return upper bound of \f$\pi/2\f$
  FloatNum pi_half_upper(void);
  /// Return lower bound of \f$\pi\f$
  FloatNum pi_lower(void);
  /// Return upper bound of \f$\pi\f$
  FloatNum pi_upper(void);
  /// Return lower bound of \f$2\pi\f$
  FloatNum pi_twice_lower(void);
  /// Return upper bound of \f$2\pi\f$
  FloatNum pi_twice_upper(void);

}

#include <gecode/float/num.hpp>

namespace Gecode { namespace Float {

  /**
   * \brief Floating point rounding policy
   *
   * \ingroup TaskModelFloatVars
   */
  class FullRounding : 
    public boost::numeric::interval_lib::rounded_arith_opp<FloatNum> {
#ifdef GECODE_HAS_MPFR
  public:
    /// Define generic mpfr function
#define GECODE_GENR_FUNC(name)      \
    GECODE_FLOAT_EXPORT double name##_down(FloatNum x); \
    GECODE_FLOAT_EXPORT double name##_up  (FloatNum x);
    GECODE_GENR_FUNC(exp)
    GECODE_GENR_FUNC(log)
    GECODE_GENR_FUNC(sin)
    GECODE_GENR_FUNC(cos)
    GECODE_GENR_FUNC(tan)
    GECODE_GENR_FUNC(asin)
    GECODE_GENR_FUNC(acos)
    GECODE_GENR_FUNC(atan)
    GECODE_GENR_FUNC(sinh)
    GECODE_GENR_FUNC(cosh)
    GECODE_GENR_FUNC(tanh)
    GECODE_GENR_FUNC(asinh)
    GECODE_GENR_FUNC(acosh)
    GECODE_GENR_FUNC(atanh)
#undef GECODE_GENR_FUNC
#endif
  };

}};

namespace Gecode {

  /**
   * \brief Float value type
   *
   * \ingroup TaskModelFloatVars
   */
  class FloatVal {
    friend FloatVal operator+(const FloatVal& x);
    friend FloatVal operator-(const FloatVal& x);
    friend FloatVal operator+(const FloatVal& x, const FloatVal& y);
    friend FloatVal operator+(const FloatVal& x, const FloatNum& y);
    friend FloatVal operator+(const FloatNum& x, const FloatVal& y);
    friend FloatVal operator-(const FloatVal& x, const FloatVal& y);
    friend FloatVal operator-(const FloatVal& x, const FloatNum& y);
    friend FloatVal operator-(const FloatNum& x, const FloatVal& y);
    friend FloatVal operator*(const FloatVal& x, const FloatVal& y);
    friend FloatVal operator*(const FloatVal& x, const FloatNum& y);
    friend FloatVal operator*(const FloatNum& x, const FloatVal& y);
    friend FloatVal operator/(const FloatVal& x, const FloatVal& y);
    friend FloatVal operator/(const FloatVal& x, const FloatNum& y);
    friend FloatVal operator/(const FloatNum& x, const FloatVal& y);
    friend FloatVal abs(const FloatVal& x);
    friend FloatVal sqrt(const FloatVal& x);
    friend FloatVal square(const FloatVal& x);
    friend FloatVal pow(const FloatVal& x, int n);
    friend FloatVal nth_root(const FloatVal& x, int n);
#ifdef GECODE_HAS_MPFR
    friend FloatVal exp(const FloatVal& x);
    friend FloatVal log(const FloatVal& x);
    friend FloatVal fmod(const FloatVal& x, const FloatVal& y);
    friend FloatVal fmod(const FloatVal& x, const FloatNum& y);
    friend FloatVal fmod(const FloatNum& x, const FloatVal& y);
    friend FloatVal sin(const FloatVal& x);
    friend FloatVal cos(const FloatVal& x);
    friend FloatVal tan(const FloatVal& x);
    friend FloatVal asin(const FloatVal& x);
    friend FloatVal acos(const FloatVal& x);
    friend FloatVal atan(const FloatVal& x);
    friend FloatVal sinh(const FloatVal& x);
    friend FloatVal cosh(const FloatVal& x);
    friend FloatVal tanh(const FloatVal& x);
    friend FloatVal asinh(const FloatVal& x);
    friend FloatVal acosh(const FloatVal& x);
    friend FloatVal atanh(const FloatVal& x);
#endif
    friend FloatVal max(const FloatVal& x, const FloatVal& y);
    friend FloatVal max(const FloatVal& x, const FloatNum& y);
    friend FloatVal max(const FloatNum& x, const FloatVal& y);
    friend FloatVal min(const FloatVal& x, const FloatVal& y);
    friend FloatVal min(const FloatVal& x, const FloatNum& y);
    friend FloatVal min(const FloatNum& x, const FloatVal& y);
    friend FloatNum lower(const FloatVal& x);
    friend FloatNum upper(const FloatVal& x);
    friend FloatNum width(const FloatVal& x);
    friend FloatNum median(const FloatVal& x);
    friend bool singleton(const FloatVal& x);
    friend bool in(const FloatNum& x, const FloatVal& y);
    friend bool zero_in(const FloatVal& x);
    friend bool subset(const FloatVal& x, const FloatVal& y);
    friend bool proper_subset(const FloatVal& x, const FloatVal& y);
    friend bool overlap(const FloatVal& x, const FloatVal& y);
    friend FloatVal intersect(const FloatVal& x, const FloatVal& y);
    friend FloatVal hull(const FloatVal& x, const FloatVal& y);
    friend FloatVal hull(const FloatVal& x, const FloatNum& y);
    friend FloatVal hull(const FloatNum& x, const FloatVal& y);
    friend FloatVal hull(const FloatNum& x, const FloatNum& y);
    friend bool operator <(const FloatVal& x, const FloatVal& y);
    friend bool operator <(const FloatVal& x, const FloatNum& y);
    friend bool operator <(const FloatNum& x, const FloatVal& y);
    friend bool operator <=(const FloatVal& x, const FloatVal& y);
    friend bool operator <=(const FloatVal& x, const FloatNum& y);
    friend bool operator <=(const FloatNum& x, const FloatVal& y);
    friend bool operator >(const FloatVal& x, const FloatVal& y);
    friend bool operator >(const FloatVal& x, const FloatNum& y);
    friend bool operator >(const FloatNum& x, const FloatVal& y);
    friend bool operator >=(const FloatVal& x, const FloatVal& y);
    friend bool operator >=(const FloatVal& x, const FloatNum& y);
    friend bool operator >=(const FloatNum& x, const FloatVal& y);
    friend bool operator ==(const FloatVal& x, const FloatVal& y);
    friend bool operator ==(const FloatVal& x, const FloatNum& y);
    friend bool operator ==(const FloatNum& x, const FloatVal& y);
    friend bool operator !=(const FloatVal& x, const FloatVal& y);
    friend bool operator !=(const FloatVal& x, const FloatNum& y);
    friend bool operator !=(const FloatNum& x, const FloatVal& y);
    template<class Char, class Traits>
    friend std::basic_ostream<Char,Traits>&
    operator <<(std::basic_ostream<Char,Traits>& os, const FloatVal& x);
  protected:
    /// Used rounding policies
    typedef boost::numeric::interval_lib::save_state<Float::FullRounding> R;
    /// Used checking policy
    typedef boost::numeric::interval_lib::checking_strict<FloatNum> P;
    /// Implementation type for float value
    typedef boost::numeric::interval
      <FloatNum,
       boost::numeric::interval_lib::policies<R, P> >
    FloatValImpType;
    /// Implementation of float value
    FloatValImpType x;
    /// Initialize from implementation \a i
    explicit FloatVal(const FloatValImpType& i);
  public:
    /// Rounding definition
    typedef FloatValImpType::traits_type::rounding Round;
    /// Default constructor
    FloatVal(void);
    /// Initialize with float number \a n
    FloatVal(const FloatNum& n);
    /// Initialize with lower bound \a l and upper bound \a u
    FloatVal(const FloatNum& l, const FloatNum& u);
    /// Copy constructor
    FloatVal(const FloatVal& v);

    /// Assignment operator
    FloatVal& operator =(const FloatNum& n);
    /// Assignment operator
    FloatVal& operator =(const FloatVal& v);
    
    /// Assign lower bound \a l and upper bound \a u
    void assign(FloatNum const &l, FloatNum const &u);

    /// Return lower bound
    FloatNum lower(void) const;
    /// Return upper bound
    FloatNum upper(void) const;
    
    /// Return hull of \a x and \a y
    static FloatVal hull(FloatNum x, FloatNum y);
    /// Return \f$\pi/2\f$
    static FloatVal pi_half(void);
    /// Return lower bound of \f$\pi\f$
    static FloatVal pi(void);
    /// Return \f$2\pi\f$
    static FloatVal pi_twice(void);
    
    /// Increment by \a n
    FloatVal& operator +=(const FloatNum& n);
    /// Subtract by \a n
    FloatVal& operator -=(const FloatNum& n);
    /// Multiply by \a n
    FloatVal& operator *=(const FloatNum& n);
    /// Divide by \a n
    FloatVal& operator /=(const FloatNum& n);
    /// Increment by \a v
    FloatVal& operator +=(const FloatVal& v);
    /// Subtract by \a v
    FloatVal& operator -=(const FloatVal& v);
    /// Multiply by \a v
    FloatVal& operator *=(const FloatVal& v);
    /// Divide by \a v
    FloatVal& operator /=(const FloatVal& v);
  };

  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator +(const FloatVal& x);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator -(const FloatVal& x);

  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator +(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator +(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator +(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator -(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator -(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator -(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator *(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator *(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator *(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator /(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator /(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Arithmetic operator
   * \relates Gecode::FloatVal
   */
  FloatVal operator /(const FloatNum& r, const FloatVal& x);

  /**
   * \brief Return absolute value of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal abs(const FloatVal& x);
  /**
   * \brief Return square root of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal sqrt(const FloatVal& x);
  /**
   * \brief Return square of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal square(const FloatVal& x);
  /**
   * \brief Return \a n -th power of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal pow(const FloatVal& x, int n);
  /**
   * \brief Return \a n -th root of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal nth_root(const FloatVal& x, int n);

#ifdef GECODE_HAS_MPFR
  /* transcendental functions: exp, log */
  /**
   * \brief Return exponential of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal exp(const FloatVal& x);
  /**
   * \brief Return logarithm of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal log(const FloatVal& x);

  /**
   * \brief Trigonometric function argument reduction
   * \relates Gecode::FloatVal
   */
  FloatVal fmod(const FloatVal& x, const FloatVal& y);
  /**
   * \brief  Trigonometric function argument reduction
   * \relates Gecode::FloatVal
   */
  FloatVal fmod(const FloatVal& x, const FloatNum& y);
  /**
   * \brief  Trigonometric function argument reduction
   * \relates Gecode::FloatVal
   */
  FloatVal fmod(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Return sine of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal sin(const FloatVal& x);
  /**
   * \brief  Return cosine of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal cos(const FloatVal& x);
  /**
   * \brief  Return tangent of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal tan(const FloatVal& x);
  /**
   * \brief  Return arcsine of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal asin(const FloatVal& x);
  /**
   * \brief  Return arccosine of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal acos(const FloatVal& x);
  /**
   * \brief  Return arctangent of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal atan(const FloatVal& x);

  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal sinh(const FloatVal& x);
  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal cosh(const FloatVal& x);
  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal tanh(const FloatVal& x);
  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal asinh(const FloatVal& x);
  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal acosh(const FloatVal& x);
  /**
   * \brief  Return hyperbolic of \a x
   * \relates Gecode::FloatVal
   */
  FloatVal atanh(const FloatVal& x);

#endif

  /**
   * \brief Return maximum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal max(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Return maximum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal max(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Return maximum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal max(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Return minimum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal min(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Return minimum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal min(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Return minimum of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal min(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Return lower bound of \a x
   * \relates Gecode::FloatVal
   */
  FloatNum lower(const FloatVal& x);
  /**
   * \brief Return upper bound of \a x
   * \relates Gecode::FloatVal
   */
  FloatNum upper(const FloatVal& x);
  /**
   * \brief Return width of \a x
   * \relates Gecode::FloatVal
   */
  FloatNum width(const FloatVal& x);
  /**
   * \brief Return median bound of \a x
   * \relates Gecode::FloatVal
   */
  FloatNum median(const FloatVal& x);

  /**
   * \brief Test whether \a x is a singleton
   * \relates Gecode::FloatVal
   */
  bool singleton(const FloatVal& x);
  /**
   * \brief Test whether \a x is included in \a y
   * \relates Gecode::FloatVal
   */
  bool in(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Test whether zero is included in \a x
   * \relates Gecode::FloatVal
   */
  bool zero_in(const FloatVal& b);
  /**
   * \brief Test whether \a x is a subset of \a y
   * \relates Gecode::FloatVal
   */
  bool subset(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Test whether \a x is a proper subset of \a y
   * \relates Gecode::FloatVal
   */
  bool proper_subset(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Test whether \a x and \a y overlap
   * \relates Gecode::FloatVal
   */
  bool overlap(const FloatVal& x, const FloatVal& y);

  /**
   * \brief Return intersection of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal intersect(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Return hull of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal hull(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Return hull of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal hull(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Return hull of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal hull(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Return hull of \a x and \a y
   * \relates Gecode::FloatVal
   */
  FloatVal hull(const FloatNum& x, const FloatNum& y);

  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <=(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <=(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator <=(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >=(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >=(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator >=(const FloatNum& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator ==(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator ==(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator ==(const FloatNum& x, const FloatVal& y);

  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator !=(const FloatVal& x, const FloatVal& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator !=(const FloatVal& x, const FloatNum& y);
  /**
   * \brief Comparison operator
   * \relates Gecode::FloatVal
   */
  bool operator !=(const FloatNum& x, const FloatVal& y);


  /**
   * \brief Print float value \a x
   * \relates Gecode::FloatVal
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const FloatVal& x);

  /**
   * \brief Float rounding mode
   *
   * \ingroup TaskModelFloatVars
   */
  static FloatVal::Round Round;
  
}

#include <gecode/float/val.hpp>

namespace Gecode { namespace Float {

  /**
   * \brief Numerical limits for floating point variables
   *
   * \ingroup TaskModelFloatVars
   */
  namespace Limits {
    /// Largest allowed float value
    const FloatNum max =  std::numeric_limits<FloatNum>::max();
    /// Smallest allowed float value
    const FloatNum min = -max;
    /// Return whether float \a n is a valid number
    bool valid(const FloatVal& n);
    /// Check whether float \a n is a valid number, otherwise throw out of limits exception with information \a l
    void check(const FloatVal& n, const char* l);
  }

}}

#include <gecode/float/limits.hpp>

#include <gecode/float/var-imp.hpp>

namespace Gecode {

  namespace Float {
    class FloatView;
  }

  /**
   * \brief Float variables
   *
   * \ingroup TaskModelFloatVars
   */
  class FloatVar : public VarImpVar<Float::FloatVarImp> {
    friend class FloatVarArray;
    friend class FloatVarArgs;
  private:
    using VarImpVar<Float::FloatVarImp>::x;
    /**
     * \brief Initialize variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. No exceptions are thrown.
     */
    void _init(Space& home, FloatNum min, FloatNum max);
  public:
    /// \name Constructors and initialization
    //@{
    /// Default constructor
    FloatVar(void);
    /// Initialize from float variable \a y
    FloatVar(const FloatVar& y);
    /// Initialize from float view \a y
    FloatVar(const Float::FloatView& y);
    /**
     * \brief Initialize variable with range domain
     *
     * The variable is created with a domain ranging from \a min
     * to \a max. The following exceptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Float::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for floats as defined
     *    in Gecode::Float::Limits, an exception of type
     *    Gecode::Float::OutOfLimits is thrown.
     */
    GECODE_FLOAT_EXPORT FloatVar(Space& home, FloatNum min, FloatNum max);
    //@}

    /// \name Value access
    //@{
    /// Return domain
    FloatVal domain(void) const;
    /// Return minimum of domain
    FloatNum min(void) const;
    /// Return maximum of domain
    FloatNum max(void) const;
    /// Return median of domain (interval)
    FloatVal med(void) const;
    /// Return median of domain (closest representation)
    FloatNum median(void) const;
    /**
     * \brief Return assigned value
     *
     * Throws an exception of type Float::ValOfUnassignedVar if variable
     * is not yet assigned.
     *
     */
    FloatVal val(void) const;

    /// Return width of domain (distance between maximum and minimum)
    FloatVal width(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether \a n is contained in domain
    bool in(const FloatVal& n) const;
    //@}
  };

  /**
   * \brief Print float variable \a x
   * \relates Gecode::FloatVar
   */
  template<class Char, class Traits>
  std::basic_ostream<Char,Traits>&
  operator <<(std::basic_ostream<Char,Traits>& os, const FloatVar& x);
}

#include <gecode/float/view.hpp>
#include <gecode/float/array-traits.hpp>

namespace Gecode {

  /// Passing float arguments
  class FloatArgs : public PrimArgArray<FloatNum> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    FloatArgs(void);
    /// Allocate array with \a n elements
    explicit FloatArgs(int n);
    /// Allocate array and copy elements from \a x
    FloatArgs(const SharedArray<FloatNum>& x);
    /// Allocate array and copy elements from \a x
    FloatArgs(const std::vector<FloatNum>& x);
    /// Allocate array and copy elements from \a first to \a last
    template<class InputIterator>
    FloatArgs(InputIterator first, InputIterator last);
    /// Allocate array with \a n elements and initialize with \a e0, ...
    GECODE_FLOAT_EXPORT
    FloatArgs(int n, int e0, ...);
    /// Allocate array with \a n elements and initialize with elements from array \a e
    FloatArgs(int n, const FloatNum* e);
    /// Initialize from primitive argument array \a a (copy elements)
    FloatArgs(const PrimArgArray<FloatNum>& a);

    /// Allocate array with \a n elements such that for all \f$0\leq i<n: x_i=\text{start}+i\cdot\text{inc}\f$
    static FloatArgs create(int n, FloatNum start, int inc=1);
    //@}
  };

  /// \brief Passing float variables
  class FloatVarArgs : public VarArgArray<FloatVar> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    FloatVarArgs(void) {}
    /// Allocate array with \a n elements
    explicit FloatVarArgs(int n) : VarArgArray<FloatVar>(n) {}
    /// Initialize from variable argument array \a a (copy elements)
    FloatVarArgs(const FloatVarArgs& a) : VarArgArray<FloatVar>(a) {}
    /// Initialize from variable array \a a (copy elements)
    FloatVarArgs(const VarArray<FloatVar>& a) : VarArgArray<FloatVar>(a) {}
    /// Initialize from vector \a a
    FloatVarArgs(const std::vector<FloatVar>& a) : VarArgArray<FloatVar>(a) {}
    /// Initialize from InputIterator \a first and \a last
    template<class InputIterator>
    FloatVarArgs(InputIterator first, InputIterator last)
    : VarArgArray<FloatVar>(first,last) {}
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Float::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for floats as defined
     *    in Gecode::Float::Limits, an exception of type
     *    Gecode::Float::OutOfLimits is thrown.
     */
    GECODE_FLOAT_EXPORT
    FloatVarArgs(Space& home, int n, FloatNum min, FloatNum max);
    //@}
  };
  //@}

  /**
   * \defgroup TaskModelFloatVarArrays Variable arrays
   *
   * Variable arrays can store variables. They are typically used
   * for storing the variables being part of a solution (script). However,
   * they can also be used for temporary purposes (even though
   * memory is not reclaimed until the space it is created for
   * is deleted).
   * \ingroup TaskModelFloat
   */

  /**
   * \brief Float variable array
   * \ingroup TaskModelFloatVarArrays
   */
  class FloatVarArray : public VarArray<FloatVar> {
  public:
    /// \name Creation and initialization
    //@{
    /// Default constructor (array of size 0)
    FloatVarArray(void);
    /// Allocate array for \a n float variables (variables are uninitialized)
    FloatVarArray(Space& home, int n);
    /// Initialize from float variable array \a a (share elements)
    FloatVarArray(const FloatVarArray& a);
    /// Initialize from float variable argument array \a a (copy elements)
    FloatVarArray(Space& home, const FloatVarArgs& a);
    /**
     * \brief Initialize array with \a n new variables
     *
     * The variables are created with a domain ranging from \a min
     * to \a max. The following execptions might be thrown:
     *  - If \a min is greater than \a max, an exception of type
     *    Gecode::Float::VariableEmptyDomain is thrown.
     *  - If \a min or \a max exceed the limits for floats as defined
     *    in Gecode::Float::Limits, an exception of type
     *    Gecode::Float::OutOfLimits is thrown.
     */
    GECODE_FLOAT_EXPORT
    FloatVarArray(Space& home, int n, FloatNum min, FloatNum max);
    //@}
  };

}

#include <gecode/float/array.hpp>

namespace Gecode {

  /**
   * \brief Relation types for floats
   * \ingroup TaskModelFloat
   */
  enum FloatRelType {
    FRT_EQ, ///< Equality (\f$=\f$)
    FRT_LQ, ///< Less or equal (\f$\leq\f$)
    FRT_GQ  ///< Greater or equal (\f$\geq\f$)
  };

  /**
   * \defgroup TaskModelFloatRelFloat Simple relation constraints over float variables
   * \ingroup TaskModelFloat
   */
  /** \brief Post propagator for \f$ x_0 \sim_{frt} x_1\f$
   *
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1);
  /** \brief Propagates \f$ x \sim_{frt} c\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x, FloatRelType frt, FloatVal c);
  /** \brief Post propagator for if (b) \f$(x \sim_{frt} c)\equiv r\f$ else \f$(x \sim_{frt} c)\equiv \neg r\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x, FloatRelType frt, FloatVal c, Reify r, bool b = true);
  /** \brief Post propagator for if (b) \f$(x_0 \sim_{frt} x_1)\equiv r\f$ else \f$(x_0 \sim_{frt} x_1)\equiv \neg r\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1, Reify r, bool b = true);
  /** \brief Propagates \f$ x_i \sim_{frt} c \f$ for all \f$0\leq i<|x|\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, const FloatVarArgs& x, FloatRelType frt, FloatVal c);

}


namespace Gecode {

  /**
   * \defgroup TaskModelFloatArith Arithmetic constraints
   * \ingroup TaskModelFloat
   */

  //@{
  /** \brief Post propagator for \f$ \min\{x_0,x_1\}=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  min(Home home, FloatVar x0, FloatVar x1, FloatVar x2);
  /** \brief Post propagator for \f$ \min x=y\f$
   * If \a x is empty, an exception of type Float::TooFewArguments is thrown.
   */
  GECODE_FLOAT_EXPORT void
  min(Home home, const FloatVarArgs& x, FloatVar y);
  /** \brief Post propagator for \f$ \max\{x_0,x_1\}=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  max(Home home, FloatVar x0, FloatVar x1, FloatVar x2);
  /** \brief Post propagator for \f$ \max x=y\f$
   * If \a x is empty, an exception of type Float::TooFewArguments is thrown.
   */
  GECODE_FLOAT_EXPORT void
  max(Home home, const FloatVarArgs& x, FloatVar y);

  /** \brief Post propagator for \f$ |x_0|=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  abs(Home home, FloatVar x0, FloatVar x1);

  /** \brief Post propagator for \f$x_0\cdot x_1=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  mult(Home home, FloatVar x0, FloatVar x1, FloatVar x2);

  /** \brief Post propagator for \f$x_0\cdot x_0=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sqr(Home home, FloatVar x0, FloatVar x1);

  /** \brief Post propagator for \f$\sqrt{x_0}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sqrt(Home home, FloatVar x0, FloatVar x1);

  /** \brief Post propagator for \f${x_0}^{exp}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  pow(Home home, FloatVar x0, int exp, FloatVar x1);

  /** \brief Post propagator for \f${x_0}^{1/exp}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  nroot(Home home, FloatVar x0, int exp, FloatVar x1);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  div(Home home, FloatVar x0, FloatVar x1, FloatVar x2);
  //@}

#ifdef GECODE_HAS_MPFR
  /**
   * \defgroup TaskModelFloatTrans Transcendental constraints
   * \ingroup TaskModelFloat
   */

  //@{
  /** \brief Post propagator for \f$ \mathrm{exp}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  exp(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{exp}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  log(Home home, FloatVar x0, FloatVar x1);
  //@}

  /**
   * \defgroup TaskModelFloatTrigo Trigonometric constraints
   * \ingroup TaskModelFloat
   */

  //@{
  /** \brief Post propagator for \f$ \mathrm{asin}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  asin(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{sin}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sin(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{acos}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  acos(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{cos}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  cos(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{atan}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  atan(Home home, FloatVar x0, FloatVar x1);
  /** \brief Post propagator for \f$ \mathrm{tan}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  tan(Home home, FloatVar x0, FloatVar x1);
  //@}
#endif

  /**
   * \defgroup TaskModelFloatLI Linear constraints over float variables
   * \ingroup TaskModelFloat
   */
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{frt} c\f$
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatVarArgs& x,
         FloatRelType frt, FloatNum c);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}x_i\sim_{frt} y\f$
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatVarArgs& x,
         FloatRelType frt, FloatVar y);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{frt} c\right)\equiv r\f$
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatVarArgs& x,
         FloatRelType frt, FloatNum c, Reify r);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}x_i\sim_{frt} y\right)\equiv r\f$
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatVarArgs& x,
         FloatRelType frt, FloatVar y, Reify r);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{frt} c\f$
   *
   *  Throws an exception of type Float::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatArgs& a, const FloatVarArgs& x,
         FloatRelType frt, FloatNum c);
  /** \brief Post propagator for \f$\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{frt} y\f$
   *
   *  Throws an exception of type Float::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatArgs& a, const FloatVarArgs& x,
         FloatRelType frt, FloatVar y);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{frt} c\right)\equiv r\f$
   *
   *  Throws an exception of type Float::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatArgs& a, const FloatVarArgs& x,
         FloatRelType frt, FloatNum c, Reify r);
  /** \brief Post propagator for \f$\left(\sum_{i=0}^{|x|-1}a_i\cdot x_i\sim_{frt} y\right)\equiv r\f$
   *
   *  Throws an exception of type Float::ArgumentSizeMismatch, if
   *  \a a and \a x are of different size.
   * \ingroup TaskModelFloatLI
   */
  GECODE_FLOAT_EXPORT void
  linear(Home home, const FloatArgs& a, const FloatVarArgs& x,
         FloatRelType frt, FloatVar y, Reify r);



  /**
   * \defgroup TaskModelFloatChannel Channel constraints
   * \ingroup TaskModelFloat
   */
  //@{
  /// Post domain consistent propagator for channeling a Float and an integer variable \f$ x_0 = x_1\f$
  GECODE_FLOAT_EXPORT void
  channel(Home home, FloatVar x0, IntVar x1);
  //@}

  /**
   * \defgroup TaskModelFloatBranch Branching
   * \ingroup TaskModelFloat
   */
  //@{
  /// Recording activities for float variables
  class FloatActivity : public Activity {
  public:
    /**
     * \brief Construct as not yet intialized
     *
     * The only member functions that can be used on a constructed but not
     * yet initialized activity storage is init or the assignment operator.
     *
     */
    FloatActivity(void);
    /// Copy constructor
    FloatActivity(const FloatActivity& a);
    /// Assignment operator
    FloatActivity& operator =(const FloatActivity& a);      
    /// Initialize for integer variables \a x with decay factor \a d
    GECODE_FLOAT_EXPORT 
    FloatActivity(Home home, const FloatVarArgs& x, double d);
    /**
     * \brief Initialize for integer variables \a x with decay factor \a d
     *
     * This member function can only be used once and only if the
     * activity storage has been constructed with the default constructor.
     *
     */
    GECODE_FLOAT_EXPORT void
    init(Home, const FloatVarArgs& x, double d);
  };

  /// Which variable to select for branching
  enum FloatVarBranch {
    FLOAT_VAR_NONE = 0,        ///< First unassigned
    FLOAT_VAR_RND,             ///< Random (uniform, for tie breaking)
    FLOAT_VAR_DEGREE_MIN,      ///< With smallest degree
    FLOAT_VAR_DEGREE_MAX,      ///< With largest degree
    FLOAT_VAR_AFC_MIN,         ///< With smallest accumulated failure count
    FLOAT_VAR_AFC_MAX,         ///< With largest accumulated failure count
    FLOAT_VAR_ACTIVITY_MIN,    ///< With lowest activity
    FLOAT_VAR_ACTIVITY_MAX,    ///< With highest activity
    FLOAT_VAR_MIN_MIN,         ///< With smallest min
    FLOAT_VAR_MIN_MAX,         ///< With largest min
    FLOAT_VAR_MAX_MIN,         ///< With smallest max
    FLOAT_VAR_MAX_MAX,         ///< With largest max
    FLOAT_VAR_WIDTH_MIN,        ///< With smallest domain size
    FLOAT_VAR_WIDTH_MAX,        ///< With largest domain size
    FLOAT_VAR_WIDTH_DEGREE_MIN, ///< With smallest domain size divided by degree
    FLOAT_VAR_WIDTH_DEGREE_MAX, ///< With largest domain size divided by degree
    FLOAT_VAR_WIDTH_AFC_MIN,    ///< With smallest domain size divided by accumulated failure count
    FLOAT_VAR_WIDTH_AFC_MAX,    ///< With largest domain size divided by accumulated failure count
    FLOAT_VAR_WIDTH_ACTIVITY_MIN, ///< With smallest domain size divided by activity
    FLOAT_VAR_WIDTH_ACTIVITY_MAX, ///< With largest domain size divided by activity
  };
  
  /// Which values to select first for branching
  enum FloatValBranch {
    FLOAT_VAL_SPLIT_MIN, ///< Select values not greater than mean of smallest and largest value
    FLOAT_VAL_SPLIT_MAX, ///< Select values not smaller than mean of largest and smallest value
    FLOAT_VAL_SPLIT_RND  ///< Select values randomly which are not greater or not smaller than mean of largest and smallest value
  };
  
  /// Branch over \a x with variable selection \a vars and value selection \a vals
  GECODE_FLOAT_EXPORT void
  branch(Home home, const FloatVarArgs& x,
         FloatVarBranch vars, FloatValBranch vals,
         const VarBranchOptions& o_vars = VarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with tie-breaking variable selection \a vars and value selection \a vals
  GECODE_FLOAT_EXPORT void
  branch(Home home, const FloatVarArgs& x,
         const TieBreakVarBranch<FloatVarBranch>& vars, FloatValBranch vals,
         const TieBreakVarBranchOptions& o_vars = TieBreakVarBranchOptions::def,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  /// Branch over \a x with value selection \a vals
  GECODE_FLOAT_EXPORT void
  branch(Home home, FloatVar x, FloatValBranch vals,
         const ValBranchOptions& o_vals = ValBranchOptions::def);
  
  //@}
}

#include <gecode/float/activity.hpp>

#endif

// IFDEF: GECODE_HAS_FLOAT_VARS
// STATISTICS: float-post

