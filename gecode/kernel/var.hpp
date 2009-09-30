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
  class VarBase {
  protected:
    /// Pointer to variable implementation
    VarImp* varimp;
    /// Default constructor
    VarBase(void);
    /// Initialize with variable implementation \a x
    VarBase(VarImp* x);
  public:
    /// \name Generic variable information
    //@{
    /// Return variable implementation of variable
    VarImp* var(void) const;
    /// Return degree (number of subscribed propagators and advisors)
    unsigned int degree(void) const;
    //@}
    /// \name Variable comparison
    //@{
    /// Test whether view is the same as \a y
    bool same(const VarBase<VarImp>& y) const;
    /// Test whether view comes before \a y (arbitrary order)
    bool before(const VarBase<VarImp>& y) const;
    //@}
  };


  /*
   * Variable: contains a pointer to a variable implementation
   *
   */
  template<class VarImp>
  forceinline
  VarBase<VarImp>::VarBase(void)
    : varimp(NULL) {}
  template<class VarImp>
  forceinline
  VarBase<VarImp>::VarBase(VarImp* x)
    : varimp(x) {}
  template<class VarImp>
  forceinline VarImp*
  VarBase<VarImp>::var(void) const {
    return varimp;
  }
  template<class VarImp>
  forceinline unsigned int
  VarBase<VarImp>::degree(void) const {
    return varimp->degree();
  }
  template<class VarImp>
  forceinline bool
  VarBase<VarImp>::same(const VarBase<VarImp>& y) const {
    return var() == y.var();
  }
  template<class VarImp>
  forceinline bool
  VarBase<VarImp>::before(const VarBase<VarImp>& y) const {
    return var() < y.var();
  }

}

// STATISTICS: kernel-var
