/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main author:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
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

namespace Gecode {

  /**
   * \defgroup TaskBranchValSel Generic value selection for brancher based on view and value selection
   *
   * \ingroup TaskBranchViewVal
   */
  //@{
  /// Base class for value selection
  template<class View_, class Val_>
  class ValSel {
  public:
    /// View type
    typedef View_ View;
    /// Corresponding variable type
    typedef typename View::VarType Var;
    /// Value type
    typedef Val_ Val;
  public:
    /// Constructor for initialization
    ValSel(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSel(Space& home, ValSel<View,Val>& vs);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete value selection
    void dispose(Space& home);
  };

  /// User-defined value selection
  template<class View>
  class ValSelFunction :
    public ValSel<View,
                  typename BranchTraits<typename View::VarType>::ValType> {
  public:
    /// The corresponding variable type
    typedef typename ValSel<View,
                            typename BranchTraits<typename View::VarType>
                              ::ValType>::Var Var;
    /// The corresponding value type
    typedef typename ValSel<View,
                            typename BranchTraits<typename View::VarType>
                              ::ValType>::Val Val;
    /// The corresponding value function
    typedef typename BranchTraits<Var>::Val ValFunction;
  protected:
    /// The user-defined value function
    SharedData<ValFunction> v;
  public:
    /// Constructor for initialization
    ValSelFunction(Space& home, const ValBranch<Var>& vb);
    /// Constructor for cloning
    ValSelFunction(Space& home, ValSelFunction<View>& vs);
    /// Return user-defined value of view \a x at position \a i
    Val val(const Space& home, View x, int i);
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete value selection
    void dispose(Space& home);
  };
  //@}


  // Baseclass value selection
  template<class View, class Val>
  forceinline
  ValSel<View,Val>::ValSel(Space&, const ValBranch<Var>&) {}
  template<class View, class Val>
  forceinline
  ValSel<View,Val>::ValSel(Space&, ValSel<View,Val>&) {}
  template<class View, class Val>
  forceinline bool
  ValSel<View,Val>::notice(void) const {
    return false;
  }
  template<class View, class Val>
  forceinline void
  ValSel<View,Val>::dispose(Space&) {}


  // User-defined value selection
  template<class View>
  forceinline
  ValSelFunction<View>::ValSelFunction
    (Space& home, const ValBranch<Var>& vb)
    : ValSel<View,Val>(home,vb), v(vb.val()) {
    if (!v())
      throw InvalidFunction("ValSelFunction::ValSelFunction");
  }
  template<class View>
  forceinline
  ValSelFunction<View>::ValSelFunction(Space& home, ValSelFunction<View>& vs)
    : ValSel<View,Val>(home,vs), v(vs.v) {
  }
  template<class View>
  forceinline typename ValSelFunction<View>::Val
  ValSelFunction<View>::val(const Space& home, View x, int i) {
    typename View::VarType y(x.varimp());
    GECODE_VALID_FUNCTION(v());
    return v()(home,y,i);
  }
  template<class View>
  forceinline bool
  ValSelFunction<View>::notice(void) const {
    return true;
  }
  template<class View>
  forceinline void
  ValSelFunction<View>::dispose(Space&) {
    v.~SharedData<ValFunction>();
  }

}

// STATISTICS: kernel-branch
