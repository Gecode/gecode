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
  template <class VarImp>
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
  };


  /** \name Variable comparison
   *  \relates VarBase
   */
  //@{
  /// Test whether views \a x and \a y are the same
  template <class VarImp>
  bool same(const VarBase<VarImp>& x, const VarBase<VarImp>& y);
  /// Test whether view \a x comes before \a y (arbitrary order)
  template <class VarImp>
  bool before(const VarBase<VarImp>& x, const VarBase<VarImp>& y);
  //@}



  /*
   * Variable: contains a pointer to a variable implementation
   *
   */
  template <class VarImp>
  forceinline
  VarBase<VarImp>::VarBase(void)
    : varimp(NULL) {}
  template <class VarImp>
  forceinline
  VarBase<VarImp>::VarBase(VarImp* x)
    : varimp(x) {}
  template <class VarImp>
  forceinline VarImp*
  VarBase<VarImp>::var(void) const {
    return varimp;
  }
  template <class VarImp>
  forceinline unsigned int
  VarBase<VarImp>::degree(void) const {
    return varimp->degree();
  }


  template <class VarImp>
  forceinline bool
  same(const VarBase<VarImp>& x, const VarBase<VarImp>& y) {
    return x.var() == y.var();
  }
  template <class VarImp>
  forceinline bool
  before(const VarBase<VarImp>& x, const VarBase<VarImp>& y) {
    return x.var() < y.var();
  }

}

// STATISTICS: kernel-var
