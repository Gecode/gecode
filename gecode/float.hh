/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *
 *  Copyright:
 *
 *  Last modified:
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
#include <boost/numeric/interval.hpp>

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

/**
 * \brief Floating point number base type
 *
 * This type defines the interval bounds used for representing floating
 * point values.
 * \ingroup TaskModelFloatVars
 */
typedef double FloatNum;

/**
 * \brief Floating point interval type
 *
 * \ingroup TaskModelFloatVars
 */

#ifdef GECODE_HAS_MPFR
  //extern "C" {
  #include <gmp.h>
  #include <mpfr.h>
  //}
#endif
namespace Gecode { namespace Float {
  using namespace boost;
  using namespace numeric;
  using namespace interval_lib;

  struct fullRounding : rounded_arith_opp<FloatNum> {
#ifdef GECODE_HAS_MPFR
    private:
      typedef int mpfr_func(mpfr_t, const __mpfr_struct*, mp_rnd_t);
      double invoke_mpfr(FloatNum x, mpfr_func f, mp_rnd_t r) {
        mpfr_t xx;
        mpfr_init_set_d(xx, x, GMP_RNDN);
        f(xx, xx, r);
        FloatNum res = mpfr_get_d(xx, r);
        mpfr_clear(xx);
        return res;
      }
    public:
    # define GENR_FUNC(name) \
      double name##_down(FloatNum x) { return invoke_mpfr(x, mpfr_##name, GMP_RNDD); } \
      double name##_up  (FloatNum x) { return invoke_mpfr(x, mpfr_##name, GMP_RNDU); }
      GENR_FUNC(exp)
      GENR_FUNC(log)
      GENR_FUNC(sin)
      GENR_FUNC(cos)
      GENR_FUNC(tan)
      GENR_FUNC(asin)
      GENR_FUNC(acos)
      GENR_FUNC(atan)
      GENR_FUNC(sinh)
      GENR_FUNC(cosh)
      GENR_FUNC(tanh)
      GENR_FUNC(asinh)
      GENR_FUNC(acosh)
      GENR_FUNC(atanh)
#endif
  };

  typedef save_state< fullRounding > R;
  //  typedef save_state< rounded_transc_opp<FloatNum> > R;
  typedef checking_strict<FloatNum> P;
  typedef interval<FloatNum, policies<R, P> > GECODE_FLOAT_FLOATINTERVAL_TYPE;
}};

typedef Gecode::Float::GECODE_FLOAT_FLOATINTERVAL_TYPE FloatInterval;

/**
 * \brief Floating rounding mode
 *
 * \ingroup TaskModelFloatVars
 */
static FloatInterval::traits_type::rounding RND;

/**
 * \brief Floating point value type
 *
 * This type defines floating point values, which use an interval
 * representation for better accuracy.
 * \ingroup TaskModelFloatVars
 */
typedef FloatInterval FloatVal;



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
    FloatInterval domain(void) const;
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
    bool in(const FloatInterval& n) const;
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
   * \brief Consistency levels for float propagators
   *
   * \ingroup TaskModelFloat
   */
  enum FloatConLevel {
    FCL_VAL, ///< Value propagation or consistency (naive)
    FCL_BND, ///< Bounds propagation or consistency
    FCL_DEF  ///< The default consistency for a constraint
  };

  /**
   * \defgroup TaskModelIntRelFloat Simple relation constraints over float variables
   * \ingroup TaskModelFloat
   */
  /** \brief Post propagator for \f$ x_0 \sim_{frt} x_1\f$
   *
   * \ingroup TaskModelIntRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Propagates \f$ x \sim_{frt} c\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x, FloatRelType frt, FloatVal c,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for if (b) \f$(x \sim_{frt} c)\equiv r\f$ else \f$(x \sim_{frt} c)\equiv \neg r\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x, FloatRelType frt, FloatVal c, Reify r, bool b = true,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for if (b) \f$(x_0 \sim_{frt} x_1)\equiv r\f$ else \f$(x_0 \sim_{frt} x_1)\equiv \neg r\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, FloatVar x0, FloatRelType frt, FloatVar x1, Reify r, bool b = true,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Propagates \f$ x_i \sim_{frt} c \f$ for all \f$0\leq i<|x|\f$
   * \ingroup TaskModelFloatRelFloat
   */
  GECODE_FLOAT_EXPORT void
  rel(Home home, const FloatVarArgs& x, FloatRelType frt, FloatVal c,
      FloatConLevel fcl=FCL_DEF);
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
  min(Home home, FloatVar x0, FloatVar x1, FloatVar x2,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \min x=y\f$
   * If \a x is empty, an exception of type Float::TooFewArguments is thrown.
   */
  GECODE_FLOAT_EXPORT void
  min(Home home, const FloatVarArgs& x, FloatVar y,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \max\{x_0,x_1\}=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  max(Home home, FloatVar x0, FloatVar x1, FloatVar x2,
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \max x=y\f$
   * If \a x is empty, an exception of type Float::TooFewArguments is thrown.
   */
  GECODE_FLOAT_EXPORT void
  max(Home home, const FloatVarArgs& x, FloatVar y,
      FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f$ |x_0|=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  abs(Home home, FloatVar x0, FloatVar x1,
      FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_1=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  mult(Home home, FloatVar x0, FloatVar x1, FloatVar x2,
       FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f$x_0\cdot x_0=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sqr(Home home, FloatVar x0, FloatVar x1,
      FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f$\sqrt{x_0}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sqrt(Home home, FloatVar x0, FloatVar x1,
       FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f${x_0}^{exp}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  pow(Home home, FloatVar x0, FloatVar x1, int exp,
      FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f${x_0}^{1/exp}=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  nroot(Home home, FloatVar x0, FloatVar x1, int exp,
       FloatConLevel fcl=FCL_DEF);

  /** \brief Post propagator for \f$x_0\ \mathrm{div}\ x_1=x_2\f$
   */
  GECODE_FLOAT_EXPORT void
  div(Home home, FloatVar x0, FloatVar x1, FloatVar x2,
      FloatConLevel fcl=FCL_DEF);
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
  exp(Home home, FloatVar x0, FloatVar x1, 
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{exp}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  log(Home home, FloatVar x0, FloatVar x1, 
      FloatConLevel fcl=FCL_DEF);
  //@}

  /**
   * \defgroup TaskModelFloatTrigo Trigonometric constraints
   * \ingroup TaskModelFloat
   */

  //@{
  /** \brief Post propagator for \f$ \mathrm{asin}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  asin(Home home, FloatVar x0, FloatVar x1, 
       FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{sin}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  sin(Home home, FloatVar x0, FloatVar x1, 
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{acos}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  acos(Home home, FloatVar x0, FloatVar x1, 
       FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{cos}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  cos(Home home, FloatVar x0, FloatVar x1, 
      FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{atan}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  atan(Home home, FloatVar x0, FloatVar x1, 
       FloatConLevel fcl=FCL_DEF);
  /** \brief Post propagator for \f$ \mathrm{tan}(x_0)=x_1\f$
   */
  GECODE_FLOAT_EXPORT void
  tan(Home home, FloatVar x0, FloatVar x1, 
      FloatConLevel fcl=FCL_DEF);
  //@}
#endif

  /**
   * \defgroup TaskModelFloatChannel Channel constraints
   * \ingroup TaskModelFloat
   */
  //@{
  /// Post domain consistent propagator for channeling a Float and an integer variable \f$ x_0 = x_1\f$
  GECODE_FLOAT_EXPORT void
  channel(Home home, FloatVar x0, IntVar x1,
          FloatConLevel fcl=FCL_DEF);
  //@}

  /**
   * \defgroup TaskModelFloatBranch Branching
   * \ingroup TaskModelFloat
   */
  //@{
  /// Which variable to select for branching
  enum FloatVarBranch {
    FLOAT_VAR_NONE = 0,         ///< First unassigned
    FLOAT_VAR_RND,              ///< Random (uniform, for tie breaking)
    FLOAT_VAR_DEGREE_MIN,       ///< With smallest degree
    FLOAT_VAR_DEGREE_MAX,       ///< With largest degree
    FLOAT_VAR_AFC_MIN,          ///< With smallest accumulated failure count
    FLOAT_VAR_AFC_MAX,          ///< With largest accumulated failure count
    FLOAT_VAR_MIN_MIN,          ///< With smallest min
    FLOAT_VAR_MIN_MAX,          ///< With largest min
    FLOAT_VAR_MAX_MIN,          ///< With smallest max
    FLOAT_VAR_MAX_MAX,          ///< With largest max
    FLOAT_VAR_WIDTH_MIN,        ///< With smallest domain width
    FLOAT_VAR_WIDTH_MAX,        ///< With largest domain width
    FLOAT_VAR_WIDTH_DEGREE_MIN, ///< With smallest domain width divided by degree
    FLOAT_VAR_WIDTH_DEGREE_MAX, ///< With largest domain width divided by degree
    FLOAT_VAR_WIDTH_AFC_MIN,    ///< With smallest domain width divided by accumulated failure count
    FLOAT_VAR_WIDTH_AFC_MAX,    ///< With largest domain width divided by accumulated failure count
  };
  
  /// Which values to select first for branching
  enum FloatValBranch {
    FLOAT_VAL_SPLIT_MIN, ///< Select values not greater than mean of smallest and largest value
    FLOAT_VAL_SPLIT_MAX, ///< Select values not smaller than mean of largest and smallest value
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

#endif

// IFDEF: GECODE_HAS_FLOAT_VARS
// STATISTICS: float-post

