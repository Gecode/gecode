/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Matthias Balzer <matthias.balzer@itwm.fraunhofer.de>
 *
 *  Copyright:
 *     Fraunhofer ITWM, 2017
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

#include <gecode/minimodel.hh>

#include <cstddef>
#include <tuple>
#include <utility>

/// reproduce C++-14 std::index_sequence
namespace { namespace cxx14 {

  namespace detail {

    template<std::size_t...>
    struct sequence {};

    template<std::size_t N, std::size_t... I>
    struct make_sequence : make_sequence<N - 1, N - 1, I...> {};

    template<std::size_t... I>
    struct make_sequence<0, I...> {
      using type = sequence<I...>;
    };
  }

  template<std::size_t... I>
  using index_sequence = detail::sequence<I...>;

  template<typename... Ts>
  using index_sequence_for = typename detail::make_sequence<sizeof...(Ts)>::type;
}}


namespace Gecode {

  namespace {
    /// Buffer for domain constraint arguments
    template<typename... Args>
    class DomArgs {
    public:
      /// Constructor
      DomArgs(Args...);

    protected:
      /// Post reified domain constraint using the stored arguments
      template<std::size_t... I>
      void apply(Home, BoolVar, const IntPropLevels&,
                 cxx14::index_sequence<I...>);

    private:
      /// Storage for the arguments
      std::tuple<Args...> _args;
    };

    /// Buffer for domain constraint arguments - partial specialization for IntVar
    template<typename... Args>
    class DomArgs<IntVar, Args...> {
    public:
      /// Constructor
      DomArgs(IntVar, Args...);

    protected:
      /// Post reified domain constraint using stored arguments
      template<std::size_t... I>
      void apply(Home, BoolVar, const IntPropLevels&,
                 cxx14::index_sequence<I...>);

    private:
      /// Storage for the arguments
      std::tuple<IntVar, Args...> _args;
    };

    /*
     * Operations for argument buffer
     *
     */
    template<typename... Args>
    DomArgs<Args...>::DomArgs(Args... args)
      : _args(std::forward<Args>(args)...) {}

    template<typename... Args>
    template<std::size_t... I>
    void
    DomArgs<Args...>::apply(Home home, BoolVar b, const IntPropLevels&,
                            cxx14::index_sequence<I...>) {
      dom(home, std::get<I>(_args)..., b);
    }

    template<typename... Args>
    DomArgs<IntVar, Args...>::DomArgs(IntVar x, Args... args)
      : _args (x, std::forward<Args>(args)...) {}

    template<typename... Args>
    template<std::size_t... I>
    void
    DomArgs<IntVar, Args...>::apply(Home home, BoolVar b,
                                    const IntPropLevels&,
                                    cxx14::index_sequence<I...>) {
      dom(home, std::get<I>(_args)..., b);
    }


    /// Domain expression
    template<typename... Args>
    class DomExpr : public DomArgs<Args...>, public BoolExpr::Misc
    {
    public:
      using DomArgs<Args...>::DomArgs;

      /// Constrain \a b to be equivalent to the expression (negated if \a neg)
      virtual void post(Home, BoolVar b, bool neg,
                        const IntPropLevels&) override;
      /// Destructor
      virtual ~DomExpr() = default;
    };

    template<typename... Args>
    void
    DomExpr<Args...>::post(Home home, BoolVar b, bool neg,
                           const IntPropLevels& ipls)
    {
      DomArgs<Args...>::apply(home, neg ? (!b).expr (home, ipls) : b, ipls,
                              cxx14::index_sequence_for<Args...>{});
    }
  }


  /*
   * Domain constraints
   *
   */
  BoolExpr
  dom(const IntVar& x, int n) {
    return BoolExpr(new DomExpr<IntVar, int>(x, n));
  }

  BoolExpr
  dom(const IntVar& x, int l, int u) {
    return BoolExpr(new DomExpr<IntVar, int, int>(x, l, u));
  }

  BoolExpr
  dom(const IntVar& x, const IntSet& s) {
    return BoolExpr(new DomExpr<IntVar, IntSet>(x, s));
  }

#ifdef GECODE_HAS_SET_VARS
  BoolExpr
  dom(const SetVar& x, SetRelType rt, int i) {
    return BoolExpr(new DomExpr<SetVar, SetRelType, int>(x, rt, i));
  }

  BoolExpr
  dom(const SetVar& x, SetRelType rt, int i, int j) {
    return BoolExpr(new DomExpr<SetVar, SetRelType, int, int>(x, rt, i, j));
  }

  BoolExpr
  dom(const SetVar& x, SetRelType rt, const IntSet& s) {
    return BoolExpr(new DomExpr<SetVar, SetRelType, IntSet>(x, rt, s));
  }
#endif

#ifdef GECODE_HAS_FLOAT_VARS
  BoolExpr
  dom(const FloatVar& x, const FloatVal& n) {
    return BoolExpr(new DomExpr<FloatVar, FloatVal>(x, n));
  }

  BoolExpr
  dom(const FloatVar& x, FloatNum l, FloatNum u) {
    return BoolExpr(new DomExpr<FloatVar, FloatNum, FloatNum>(x, l, u));
  }
#endif
}

// STATISTICS: minimodel-any
