/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2017
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

#include <functional>

namespace Gecode {

  /// Function type for printing variable and value selection
  template<class Var, class Val>
  using VarValPrint = std::function<void(const Space& home, const Brancher& b,
                                         unsigned int a,
                                         Var x, int i, const Val& m,
                                         std::ostream& o)>;
 
  /// Class storing a print function
  template<class View, class Val>
  class BrancherPrint {
  public:
    /// The corresponding variable type
    typedef typename View::VarType Var;
  protected:
    SharedData<VarValPrint<Var,Val>> p;
  public:
    /// Initialize
    BrancherPrint(VarValPrint<Var,Val> vvp);
    /// Initialize during cloning
    BrancherPrint(BrancherPrint& bp);
    /// Whether printing is enabled
    operator bool(void) const;
    /// Invoke print function
    void operator ()(const Space& home, const Brancher& b,
                     unsigned int a,
                     View x, int i, const Val& m,
                     std::ostream& o) const;
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete
    void dispose(Space& home);
  };

  /// Class without print function
  template<class View, class Val>
  class BrancherNoPrint {
  public:
    /// The corresponding variable type
    typedef typename View::VarType Var;
  public:
    /// Initialize
    BrancherNoPrint(VarValPrint<Var,Val> vvp);
    /// Initialize during cloning
    BrancherNoPrint(BrancherNoPrint& bp);
    /// Whether printing is enabled
    operator bool(void) const;
    /// Invoke print function
    void operator ()(const Space& home, const Brancher& b,
                     unsigned int a,
                     View x, int i, const Val& m,
                     std::ostream& o) const;
    /// Whether dispose must always be called (that is, notice is needed)
    bool notice(void) const;
    /// Delete
    void dispose(Space& home);
  };



  template<class View, class Val>
  forceinline
  BrancherPrint<View,Val>::BrancherPrint(VarValPrint<Var,Val> vvp) : p(vvp) {
    if (!vvp)
      throw Gecode::InvalidFunction("BrancherPrint::BrancherPrint");
  }
  
  template<class View, class Val>
  forceinline
  BrancherPrint<View,Val>::BrancherPrint(BrancherPrint<View,Val>& bp)
    : p(bp.p) {
  }

  template<class View, class Val>
  forceinline
  BrancherPrint<View,Val>::operator bool(void) const {
    return true;
  }

  template<class View, class Val>
  forceinline void
  BrancherPrint<View,Val>::operator ()(const Space& home, const Brancher& b,
                                       unsigned int a,
                                       View x, int i, const Val& m,
                                       std::ostream& o) const {
    GECODE_VALID_FUNCTION(p());
    Var xv(x.varimp());
    p()(home,b,a,xv,i,m,o);
  }

  template<class View, class Val>
  forceinline bool
  BrancherPrint<View,Val>::notice(void) const {
    return true;
  }

  template<class View, class Val>
  forceinline void
  BrancherPrint<View,Val>::dispose(Space&) {
    p.~SharedData<VarValPrint<Var,Val>>();
  }


  template<class View, class Val>
  forceinline
  BrancherNoPrint<View,Val>::BrancherNoPrint(VarValPrint<Var,Val> vvp) {
    assert(!vvp);
    (void) vvp;
  }
  
  template<class View, class Val>
  forceinline
  BrancherNoPrint<View,Val>::BrancherNoPrint(BrancherNoPrint<View,Val>&) {}

  template<class View, class Val>
  forceinline
  BrancherNoPrint<View,Val>::operator bool(void) const {
    return false;
  }

  template<class View, class Val>
  forceinline void
  BrancherNoPrint<View,Val>::operator ()(const Space&, const Brancher&,
                                         unsigned int,
                                         View, int, const Val&,
                                         std::ostream&) const {}
  template<class View, class Val>
  forceinline bool
  BrancherNoPrint<View,Val>::notice(void) const {
    return false;
  }

  template<class View, class Val>
  forceinline void
  BrancherNoPrint<View,Val>::dispose(Space&) {}

}

// STATISTICS: kernel-branch
