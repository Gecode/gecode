/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

namespace Gecode {

  /**
   * \brief Base-class for variables
   * \ingroup TaskVarView
   */
  template<class VarImp>
  class Var {
  protected:
    /// Pointer to variable implementation
    VarImp* x;
    /// Default constructor
    Var(void);
    /// Initialize with variable implementation \a y
    Var(VarImp* y);
  public:
    /// \name Generic variable information
    //@{
    /// Return variable implementation of variable
    VarImp* varimp(void) const;
    /// Return degree (number of subscribed propagators and advisors)
    unsigned int degree(void) const;
    /// Return accumulated failure count (plus degree)
    double afc(void) const;
    //@}

    /// \name Domain tests
    //@{
    /// Test whether view is assigned
    bool assigned(void) const;
    //@}

    /// \name Cloning
    //@{
    /// Update this variable to be a clone of variable \a y
    void update(Space& home, bool share, Var<VarImp>& y);
    //@}

    /// \name Variable comparison
    //@{
    /// Test whether variable is the same as \a y
    bool same(const Var<VarImp>& y) const;
    /// Test whether variable comes before \a y (arbitrary order)
    bool before(const Var<VarImp>& y) const;
    //@}
  };


  /*
   * Variable: contains a pointer to a variable implementation
   *
   */
  template<class VarImp>
  forceinline
  Var<VarImp>::Var(void)
    : x(NULL) {}
  template<class VarImp>
  forceinline
  Var<VarImp>::Var(VarImp* y)
    : x(y) {}
  template<class VarImp>
  forceinline VarImp*
  Var<VarImp>::varimp(void) const {
    return x;
  }
  template<class VarImp>
  forceinline unsigned int
  Var<VarImp>::degree(void) const {
    return x->degree();
  }
  template<class VarImp>
  forceinline double
  Var<VarImp>::afc(void) const {
    return x->afc();
  }
  template<class VarImp>
  forceinline bool
  Var<VarImp>::assigned(void) const {
    return x->assigned();
  }
  template<class VarImp>
  forceinline void
  Var<VarImp>::update(Space& home, bool share, Var<VarImp>& y) {
    x = y.x->copy(home,share);
  }
  template<class VarImp>
  forceinline bool
  Var<VarImp>::same(const Var<VarImp>& y) const {
    return varimp() == y.varimp();
  }
  template<class VarImp>
  forceinline bool
  Var<VarImp>::before(const Var<VarImp>& y) const {
    return varimp() < y.varimp();
  }

}

// STATISTICS: kernel-var
