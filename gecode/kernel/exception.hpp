/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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
   * \defgroup FuncThrowKernel Kernel exceptions
   * \ingroup FuncThrow
   */

  //@{

  /// %Exception: Operation on failed space invoked
  class GECODE_VTABLE_EXPORT SpaceFailed : public Exception {
  public:
    /// Initialize with location \a l
    SpaceFailed(const char* l);
  };

  /// %Exception: Operation on not stable space invoked
  class GECODE_VTABLE_EXPORT SpaceNotStable : public Exception {
  public:
    /// Initialize with location \a l
    SpaceNotStable(const char* l);
  };

  /// %Exception: Commit when no brancher present
  class GECODE_VTABLE_EXPORT SpaceNoBrancher : public Exception {
  public:
    /// Initialize
    SpaceNoBrancher(void);
  };

  /// %Exception: Commit with illegal alternative
  class GECODE_VTABLE_EXPORT SpaceIllegalAlternative : public Exception {
  public:
    /// Initialize
    SpaceIllegalAlternative(void);
  };

  /// %Exception: no constrain member function defined
  class GECODE_VTABLE_EXPORT SpaceConstrainUndefined : public Exception {
  public:
    /// Initialize
    SpaceConstrainUndefined(void);
  };

  /// %Exception: too many branchers
  class GECODE_VTABLE_EXPORT TooManyBranchers : public Exception {
  public:
    /// Initialize with location \a l
    TooManyBranchers(const char* l);
  };

  //@}

  /*
   * Classes for exceptions raised by kernel
   *
   */
  inline
  SpaceFailed::SpaceFailed(const char* l)
    : Exception(l,"Attempt to invoke operation on failed space") {}

  inline
  SpaceNotStable::SpaceNotStable(const char* l)
    : Exception(l,"Attempt to invoke operation on not stable space") {}

  inline
  SpaceNoBrancher::SpaceNoBrancher(void)
    : Exception("Space::commit",
                "Attempt to commit with no brancher") {}

  inline
  SpaceIllegalAlternative::SpaceIllegalAlternative(void)
    : Exception("Space::commit",
                "Attempt to commit with illegal alternative") {}

  inline
  SpaceConstrainUndefined::SpaceConstrainUndefined(void)
    : Exception("Space::constrain",
                "Attempt to use undefined constrain function") {}

  inline
  TooManyBranchers::TooManyBranchers(const char* l)
    : Exception(l,"Too many branchers created") {}

}

// STATISTICS: kernel-other
