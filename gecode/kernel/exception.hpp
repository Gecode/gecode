/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2004
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
  class GECODE_KERNEL_EXPORT SpaceFailed : public Exception {
  public:
    /// Initialize with location \a l
    SpaceFailed(const char* l);
  };

  /// %Exception: Operation on not stable space invoked
  class GECODE_KERNEL_EXPORT SpaceNotStable : public Exception {
  public:
    /// Initialize with location \a l
    SpaceNotStable(const char* l);
  };

  /// %Exception: Copy constructor did not call base class copy constructor
  class GECODE_KERNEL_EXPORT SpaceNotCloned : public Exception {
  public:
    /// Initialize with location \a l
    SpaceNotCloned(const char* l);
  };

  /// %Exception: Commit when no brancher present
  class GECODE_KERNEL_EXPORT SpaceNoBrancher : public Exception {
  public:
    /// Initialize with location \a l
    SpaceNoBrancher(const char* l);
  };

  /// %Exception: Commit with illegal alternative
  class GECODE_KERNEL_EXPORT SpaceIllegalAlternative : public Exception {
  public:
    /// Initialize with location \a l
    SpaceIllegalAlternative(const char* l);
  };

  /// %Exception: too many groups
  class GECODE_KERNEL_EXPORT TooManyGroups : public Exception {
  public:
    /// Initialize with location \a l
    TooManyGroups(const char* l);
  };

  /// %Exception: unknown propagator
  class GECODE_KERNEL_EXPORT UnknownPropagator : public Exception {
  public:
    /// Initialize with location \a l
    UnknownPropagator(const char* l);
  };

  /// %Exception: too many branchers
  class GECODE_KERNEL_EXPORT TooManyBranchers : public Exception {
  public:
    /// Initialize with location \a l
    TooManyBranchers(const char* l);
  };

  /// %Exception: unknown brancher
  class GECODE_KERNEL_EXPORT UnknownBrancher : public Exception {
  public:
    /// Initialize with location \a l
    UnknownBrancher(const char* l);
  };

  /// %Exception: illegal decay factor
  class GECODE_KERNEL_EXPORT IllegalDecay : public Exception {
  public:
    /// Initialize with location \a l
    IllegalDecay(const char* l);
  };

  /// %Exception: invalid function
  class GECODE_KERNEL_EXPORT InvalidFunction : public Exception {
  public:
    /// Initialize with location \a l
    InvalidFunction(const char* l);
  };

  /// %Exception: uninitialized AFC
  class GECODE_KERNEL_EXPORT UninitializedAFC : public Exception {
  public:
    /// Initialize with location \a l
    UninitializedAFC(const char* l);
  };

  /// %Exception: uninitialized action
  class GECODE_KERNEL_EXPORT UninitializedAction : public Exception {
  public:
    /// Initialize with location \a l
    UninitializedAction(const char* l);
  };

  /// %Exception: uninitialized CHB
  class GECODE_KERNEL_EXPORT UninitializedCHB : public Exception {
  public:
    /// Initialize with location \a l
    UninitializedCHB(const char* l);
  };

  /// %Exception: uninitialized random number generator
  class GECODE_KERNEL_EXPORT UninitializedRnd : public Exception {
  public:
    /// Initialize with location \a l
    UninitializedRnd(const char* l);
  };

  /// %Exception: action has wrong arity
  class GECODE_KERNEL_EXPORT MoreThanOneTracer : public Exception {
  public:
    /// Initialize with location \a l
    MoreThanOneTracer(const char* l);
  };

  //@}

}

// STATISTICS: kernel-other
