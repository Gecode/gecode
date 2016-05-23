/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *    Vincent Barichard <Vincent.Barichard@univ-angers.fr>
 *
 *  Copyright:
 *    Vincent Barichard, 2013
 *
 *  Last modified:
 *     $Date$ by $Author$
 *     $Revision$
 *
 *  This file is part of Quacode:
 *     http://quacode.barichard.com
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

#ifndef __QCSP_HH__
#define __QCSP_HH__

#include <gecode/kernel.hh>

/*
 * Configure linking
 *
 */
#if !defined(QUACODE_STATIC_LIB) && \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef BUILD_QUACODE_LIB
#define QUACODE_EXPORT __declspec( dllexport )
#else
#define QUACODE_EXPORT __declspec( dllimport )
#endif

#else

#ifdef QUACODE_GCC_HAS_CLASS_VISIBILITY
#define QUACODE_EXPORT __attribute__ ((visibility("default")))
#else
#define QUACODE_EXPORT
#endif

#endif

namespace Gecode {
  #define EXISTS 0
  #define FORALL 1
  typedef unsigned int TQuantifier;
}

#include <gecode/int.hh>

namespace Gecode {
  /// Quantifier variable
  template <class VarType>
  struct QVar {
    TQuantifier q;
    VarType     x;
    int         r;
    QVar() : q(EXISTS), r(Int::Limits::max) { }
    QVar(VarType& _x) : q(EXISTS), x(_x), r(Int::Limits::max) { }
    QVar(TQuantifier _q, VarType& _x, int _r) : q(_q), x(_x), r(_r) { }
    void* varimp() const { return x.varimp(); }
  };

  typedef QVar<BoolVar> QBoolVar;
  typedef QVar<IntVar> QIntVar;

  /** \brief Passing Quantified variables
   */
  template <class QVarType>
  class QVarArgs : public VarArgArray<QVarType> {
    typedef typename VarArgArray<QVarType>::VarLess VarLess;
  public:
    using VarArgArray<QVarType>::n;
    using VarArgArray<QVarType>::a;

    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    QVarArgs(void) {}
    /// Allocate array with \a n elements
    explicit QVarArgs(int n) : VarArgArray<QVarType>(n) {}
    /// Initialize from variable argument array \a a (copy elements)
    QVarArgs(const QVarArgs& a) : VarArgArray<QVarType>(a) {}
    /// Initialize from variable array \a a (copy elements)
    QVarArgs(const VarArray<QVarType>& a)
     : VarArgArray<QVarType>(a) {}
    /// Remove all duplicate variables from array (changes element order)
    void unique(void) {
      if (n < 2)
        return ;
      VarLess vl;
      Support::quicksort<QVarType,VarLess>(a,n,vl);
      int j = 0;
      for (int i = 1; i<n; i++)
        if (a[j].varimp() != a[i].varimp())
          a[++j] = a[i];
      n = j+1;
    }
    //@}
  };

  typedef QVarArgs<QBoolVar> QBoolVarArgs;
  typedef QVarArgs<QIntVar>  QIntVarArgs;

  // TRAITS
  class QuantArgs;

  /// Traits of %QBoolVarArgs
  template<>
  class ArrayTraits<VarArgArray<QBoolVar> > {
  public:
    typedef QBoolVarArgs StorageType;
    typedef QBoolVar     ValueType;
    typedef QBoolVarArgs ArgsType;
  };

  /// Traits of %QIntVarArgs
  template<>
  class ArrayTraits<VarArgArray<QIntVar> > {
  public:
    typedef QIntVarArgs StorageType;
    typedef QIntVar     ValueType;
    typedef QIntVarArgs ArgsType;
  };

  /// Traits of %QuantArgs
  template<>
  class ArrayTraits<PrimArgArray<TQuantifier> > {
  public:
    typedef QuantArgs   StorageType;
    typedef TQuantifier ValueType;
    typedef QuantArgs   ArgsType;
  };

  /// Passing TQuantifier arguments
  class QuantArgs : public PrimArgArray<TQuantifier> {
  public:
    /// \name Constructors and initialization
    //@{
    /// Allocate empty array
    QuantArgs(void) {}
    /// Allocate array with \a n elements
    explicit QuantArgs(int n) : PrimArgArray<TQuantifier>(n) {}
    /// Allocate array and copy elements from \a x
    QuantArgs(const SharedArray<TQuantifier>& x)
       : PrimArgArray<TQuantifier>(x.size()) {
       for (int i=x.size(); i--;)
         a[i] = x[i];
     }
    /// Allocate array and copy elements from \a x
    QuantArgs(const std::vector<TQuantifier>& x)
      : PrimArgArray<TQuantifier>(static_cast<int>(x.size())) {
      for (std::vector<TQuantifier>::size_type i=x.size(); i--;)
        a[i] = x[i];
    }
    /// Allocate array with \a n elements and initialize with \a e0, ...
    QUACODE_EXPORT
    QuantArgs(int n, TQuantifier e0, ...) : PrimArgArray<TQuantifier>(n) {
      va_list args;
      va_start(args, e0);
      a[0] = e0;
      for (int i = 1; i < n; i++)
        a[i] = va_arg(args,TQuantifier);
      va_end(args);
    }
    /// Allocate array with \a n elements and initialize with elements from array \a e
    QuantArgs(int n, const TQuantifier* e)  : PrimArgArray<TQuantifier>(n, e) {}
    /// Initialize from primitive argument array \a a (copy elements)
    QuantArgs(const PrimArgArray<TQuantifier>& a) : PrimArgArray<TQuantifier>(a) {}
    //@}
  };

  /** \brief Post domain consistent propagator for Quantified Boolean operation on \a x0 and \a x1
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} x_1 = x_2\f$
   * \ingroup TaskModelIntRelBool
   */
  QUACODE_EXPORT void
  qrel(Home home, QBoolVar qx0, BoolOpType o, QBoolVar qx1, BoolVar x2);

  /** \brief Post domain consistent propagator for Quantified Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= z\f$
   *
   * Throws an exception of type Int::IllegalOperation, if \a o is different
   * from BOT_AND or BOT_OR.
   * \ingroup TaskModelIntRelBool
   */
  QUACODE_EXPORT void
  qclause(Home home, BoolOpType o, QBoolVarArgs x, QBoolVarArgs y,
          BoolVar z);
  /** \brief Post domain consistent propagator for Quantified Boolean clause with positive variables \a x and negative variables \a y
   *
   * Posts propagator for \f$ x_0 \diamond_{\mathit{o}} \cdots
   * \diamond_{\mathit{o}} x_{|x|-1} \diamond_{\mathit{o}} \neg y_0
   * \diamond_{\mathit{o}} \cdots \diamond_{\mathit{o}} \neg y_{|y|-1}= n\f$
   *
   * Throws an exception of type Int::NotZeroOne, if \a n is neither
   * 0 or 1.
   *
   * Throws an exception of type Int::IllegalOperation, if \a o is different
   * from BOT_AND or BOT_OR.
   * \ingroup TaskModelIntRelBool
   */
  QUACODE_EXPORT void
  qclause(Home home, BoolOpType o, QBoolVarArgs x, QBoolVarArgs y,
          int n);
}

#include <gecode/search.hh>
namespace Gecode {
  /**
   * \brief Depth-first search engine for quantified variables
   *
   * This class supports depth-first search for quantified variables for subclasses \a T of
   * Space.
   * \ingroup TaskModelSearch
   */
  template<class T>
  class QDFS : public Search::Base<T> {
  private:
    /// The actual search engine
    Search::Engine* e;
  public:
    /// Initialize search engine for space \a s with options \a o
    QDFS(T* s, const Search::Options& o=Search::Options::def);
    /// Whether engine does best solution search
    static const bool best = false;
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Destructor
    ~QDFS(void);
  };
}
#include <quacode/search/qdfs.hpp>

#endif
